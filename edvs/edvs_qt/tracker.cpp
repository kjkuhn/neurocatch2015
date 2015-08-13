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



Tracker::Tracker()
{
    this->orb = cv::ORB::create(500, 2, 8, 2, 0,4,cv::ORB::FAST_SCORE, 2, 20);
    run.store(true);
    object_present.store(false);
    //error checking?
    sem_init(&this->wl_count, 0, 0);
    this->worker = std::thread(&Tracker::process, this);
    //cv::namedWindow(T_WINDOW_NAME);
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
    //cv::Mat img(rows,cols,CV_8UC1, buf);
    store = (uint8_t*)malloc(128*128);
    memcpy(store, buf, 128*128);
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
        calculate(raw_img);
    }

}


void Tracker::calculate(uint8_t *raw_img)
{
    cv::Mat img, desc;
    //std::vector<cv::Mat>  closest_matches;
    std::vector<cv::KeyPoint> kp;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    //std::deque<cv::Mat>::iterator it1, it0;
    std::vector<cv::DMatch>matches, good_matches;
    unsigned int it;
    double max_dist, min_dist;
    std::vector<cv::KeyPoint> good_old, good_new;
#if DEBUG
    QImage qimg = QImage(128,128,QImage::Format_RGB32);
#endif

    img = cv::Mat(128,128,CV_8UC1, raw_img);
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
            matcher.get()->match(descriptors[descriptors.size()-1], descriptors[it], matches);
            if(matches.size() < T_MIN_MATCHES)
            {
                //no object ?
                //images.clear();
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
            for(int i = 0; i < descriptors[it].rows; i++)
            {
                if(matches[i].distance < min_dist) min_dist = matches[i].distance;
                if(matches[i].distance > max_dist) max_dist = matches[i].distance;
            }
            //get good matches
            for(int i = 0; i < descriptors[it].rows; i++)
            {
                if(matches[i].distance <= 3 * min_dist) good_matches.push_back(matches[i]);
            }
            for(unsigned int i = 0; i < good_matches.size(); i++)
            {
                good_old.push_back(keypoints[it][good_matches[i].trainIdx]);
                good_new.push_back(kp[good_matches[i].queryIdx]);
            }
#if DEBUG
            cv::Mat img2(128,128,CV_8UC1, images[it]);
            cv::Mat outImg;
            try
            {
            cv::drawMatches(img, kp, img2, keypoints[it], matches, outImg);
            qimg = QImage(outImg.rows, outImg.cols, QImage::Format_RGB32);
            for(int y = 0; y < outImg.cols; y++)
                for(int x = 0; x < outImg.rows; x++)
                    qimg.setPixel(x,y, qRgb(outImg.at<uint8_t>(y,x),outImg.at<uint8_t>(y,x),outImg.at<uint8_t>(y,x)));
            emit sendFrame(&qimg);
            }
            catch(int err)
            {

            }
#endif
            keypoints[it] = good_old;
            keypoints[it+1] = good_new;
            orb.get()->compute(img, keypoints[it], desc);
            descriptors[it+1] = desc;
            //img = cv::Mat(128,128,CV_8UC1, images[it]);
            orb.get()->compute(cv::Mat(128,128,CV_8UC1, images[it]), keypoints[it+1], desc);
            descriptors[it] = desc;
            good_new.clear();
            good_old.clear();
            matches.clear();
        }
        if(descriptors.size() == T_NUM_OBJ_DESC)
        {
            object_present.store(true);
        }
    }
    else
    {
        free(raw_img);
    }
}


} /*neurocatch*/
