#include "tracker.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "opencv2/highgui.hpp"
#include "qimage.h"



#ifndef T_WINDOW_NAME
#define T_WINDOW_NAME "tracker window"
#endif

#ifndef T_NUM_OBJ_DESC
#define T_NUM_OBJ_DESC  3
#endif

#ifndef T_MIN_MATCHES
#define T_MIN_MATCHES   50
#endif




namespace neurocatch
{


#if DEBUG
    QImage qimg;// = QImage(128,128,QImage::Format_RGB32);
#endif

char str_info[1024];
uint32_t img_count;


Tracker::Tracker()
{
    this->orb = cv::ORB::create(500, 2, 8, 2, 0,4,cv::ORB::FAST_SCORE, 2, 20);
    run.store(true);
    object_present.store(false);
    //error checking?
    sem_init(&this->wl_count, 0, 0);
    this->worker = std::thread(&Tracker::process, this);
    img_count = 0;
#if USE_SPHERO
    sphero = new neurocatch::SpheroController();
#endif /*USE_SPHERO*/
}


Tracker::~Tracker()
{
    run.store(false);
    sem_post(&wl_count);
    worker.join();
}


void Tracker::add_to_wl(uint8_t *buf, size_t rows, size_t cols)
{
    uint8_t *store;
    int size = rows*cols;
    //cv::Mat img(rows,cols,CV_8UC1, buf);
    store = (uint8_t*)malloc(size);
    memcpy(store, buf, size);
    wl_mtx.lock();
    wl.push(store);
    wl_mtx.unlock();
    sem_post(&wl_count);
}


void Tracker::getKeyPoints(std::vector<cv::KeyPoint> &kp)
{
    if(object_present.load())
        kp = keypoints[0];
}


void Tracker::process()
{
    uint8_t *raw_img;

    while(1)
    {    
        sem_wait(&wl_count);
        if(!run.load())
            break;
        wl_mtx.lock();
        raw_img = wl.front();
        wl.pop();
        wl_mtx.unlock();
        img_count++;
        calculate(raw_img);
    }

}


void Tracker::calculate(uint8_t *raw_img)
{
    cv::Mat img, desc, unfiltered;
    std::vector<cv::KeyPoint> kp;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    std::vector<cv::DMatch>matches, good_matches;
    unsigned int it, i;
    double max_dist, min_dist;
    std::vector<cv::KeyPoint> good_old, good_new;

    unfiltered = cv::Mat(128,128,CV_8UC1, raw_img);
    cv::blur(unfiltered, img, cv::Size(5,5));
    orb.get()->detect(img, kp);
    orb.get()->compute(img, kp, desc);
    if(descriptors.size() < T_NUM_OBJ_DESC)
    {
        images.push_back(raw_img);
        keypoints.push_back(kp);
        descriptors.push_back(desc);
        if(images.size() <= 1)
            return;
        for(it = 0; it < descriptors.size()-1; it++)
        {
            if(descriptors[it].cols != descriptors[descriptors.size()-1].cols)
            {
                descriptors.pop_back();
                images.pop_back();
                keypoints.pop_back();
                free(raw_img);
                break;
            }
            matcher.get()->match(descriptors[descriptors.size()-1], descriptors[it], matches);
            sprintf(str_info, "#keypoints:\t%lu\n#descriptors:\t%d\n#matches:\t%lu\n#images:\t%u",
                    kp.size(), desc.rows, matches.size(), img_count);
            emit send_info((const char*)str_info);
            if(matches.size() < T_MIN_MATCHES)
            {
                //no object ?
                while(!images.empty())
                {
                    free(images.front());
                    images.pop_front();
                }
                keypoints.clear();
                descriptors.clear();
                break;
            }

            max_dist = 0;
            min_dist = 100;
            //get min/max distance
            for(i = 0; i < (unsigned int)matches.size(); i++)
            {
                if(matches[i].distance < min_dist) min_dist = matches[i].distance;
                if(matches[i].distance > max_dist) max_dist = matches[i].distance;
            }
            //get good matches
            for(i = 0; i < (unsigned int)matches.size()/*descriptors[it].rows*/; i++)
            {
                if(matches[i].distance <= 3 * min_dist) //good_matches.push_back(matches[i]);
                {
                    good_new.push_back(kp[matches[i].queryIdx]);
                    good_old.push_back(keypoints[it][matches[i].trainIdx]);
                }
            }
/*
#if DEBUG
            cv::Mat img2(128,128,CV_8UC1, images[it]);
            cv::Mat outImg;
            try
            {
            cv::drawMatches(img, kp, img2, keypoints[it], matches, outImg);
            qimg = QImage(outImg.cols, outImg.rows, QImage::Format_RGB32);
            for(int y = 0; y < outImg.rows; y++)
                for(int x = 0; x < outImg.cols; x++)
                    qimg.setPixel(x,y, qRgb(outImg.at<uint8_t>(y,x),outImg.at<uint8_t>(y,x),outImg.at<uint8_t>(y,x)));
            emit sendFrame(&qimg);
            }
            catch(int err)
            {

            }
#endif
*/

#if DEBUG

            qimg = QImage(128,128, QImage::Format_RGB32);
            for(int x = 0; x < 128; x++)
                for(int y = 0; y < 128;y++)
                {
                    qimg.setPixel(x,y,0);
                }

            for(unsigned int x = 0; x < good_old.size(); x++)
            {
                qimg.setPixel(good_old[x].pt.x, good_old[x].pt.y, qRgb(217,255,0));
            }
            for(unsigned int x = 0; x < good_new.size(); x++)
            {
                qimg.setPixel(good_new[x].pt.x, good_new[x].pt.y, qRgb(0,255,217));
            }
            emit sendFrame(&qimg);

#endif
/*
            keypoints[it] = good_old;
            keypoints[it+1] = good_new;
            orb.get()->compute(img, keypoints[it+1], desc);
            descriptors[it+1] = desc;
            //img = cv::Mat(128,128,CV_8UC1, images[it]);
            orb.get()->compute(cv::Mat(128,128,CV_8UC1, images[it]), keypoints[it], desc);
            descriptors[it] = desc;
*/
            good_new.clear();
            good_old.clear();
            matches.clear();
        }
        if(descriptors.size() == T_NUM_OBJ_DESC)
        {
            object_present.store(true);
            sprintf(&str_info[strlen(str_info)], "\nobject detected!");
            emit send_info(str_info);
#if DEBUG

            qimg = QImage(128,128, QImage::Format_RGB32);
            for(int x = 0; x < 128; x++)
                for(int y = 0; y < 128;y++)
                    qimg.setPixel(x,y,0);
            for(unsigned int i = 0; i < keypoints.size(); i++)
                for(unsigned int j = 0; j < keypoints[i].size(); j++)
                {
                    qimg.setPixel(keypoints[i][j].pt.x, keypoints[i][j].pt.y,
                                      qRgb((30*i+70)%255,(60*i+40)%255,(90*i+50)%255));
                }
            emit sendFrame(&qimg);

#endif /*DEBUG*/
        }
    }
    else
    {
        for(it = images.size()-1; (int)it >= 0 ; it--)
        {
            if(desc.cols == descriptors[it].cols)
                matcher.get()->match(desc, descriptors[it], matches);
            else continue;
            if(matches.size() >= T_MIN_MATCHES)
                break;
        }
        if(matches.size() >= T_MIN_MATCHES)
        {
            //object present??
            max_dist = 0;
            min_dist = 100;
            //get min/max distance
            for(i = 0; i < (unsigned int)descriptors[it].rows; i++)
            {
                if(matches[i].distance < min_dist) min_dist = matches[i].distance;
                if(matches[i].distance > max_dist) max_dist = matches[i].distance;
            }
            //get good matches
            for(i = 0; i < matches.size(); i++)
            {
                if(matches[i].distance <= 3 * min_dist)
                    good_matches.push_back(matches[i]);
            }
            min_dist = 0;
            max_dist = 0;
            for(i = 0; i < (unsigned int)good_matches.size(); i++)
            {
                //if(kp[good_matches[i].queryIdx].pt.x != 0)
                    max_dist += (double)(kp[good_matches[i].queryIdx].pt.x);
                //if(kp[good_matches[i].queryIdx].pt.y != 0)
                    min_dist += (double)(kp[good_matches[i].queryIdx].pt.y);
            }
            max_dist /= (double)i > 0 ? i : 1;
            min_dist /= (double)i > 0 ? i : 1;
#if USE_SPHERO
            sphero->setXY(max_dist, min_dist);
#endif /*USE_SPHERO*/
            sprintf(str_info, "xdirection: %f\tydirection: %f\n\nnext: %hhu | %hhu",
                    max_dist, min_dist, (uint8_t)(sphero->get_next() >> 8)&0xff, (uint8_t)(sphero->get_next()&0xff));
            emit send_info(str_info);
#if DEBUG
            qimg = QImage(128,128, QImage::Format_RGB32);
            for(int x = 0; x < 128; x++)
                for(int y = 0; y < 128;y++)
                    qimg.setPixel(x,y,0);
            for(i = 0; i < (unsigned int)good_matches.size(); i++)
                qimg.setPixel(kp[good_matches[i].queryIdx].pt.x, kp[good_matches[i].queryIdx].pt.y, qRgb(217,100,50));
            emit sendFrame(&qimg);
#endif /*DEBUG*/
        }
        free(raw_img);
    }
}


} /*neurocatch*/
