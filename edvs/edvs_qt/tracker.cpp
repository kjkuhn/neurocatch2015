#include "tracker.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "opencv2/highgui.hpp"
#include "qimage.h"
#include "time.h"



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

#if USE_KNN
static uint8_t l[DATA_LEN], r[DATA_LEN], b[DATA_LEN], t[DATA_LEN];
#endif /*USE_KNN*/


Tracker::Tracker()
{
#if USE_ORB || USE_DYNAMIC_ORB
    orb = cv::ORB::create(500, 2, 8, ORB_THRESHOLD, 0,4,cv::ORB::FAST_SCORE, 2, 20);
#elif USE_SIFT
    sift = cv::xfeatures2d::SIFT::create(100);
#elif USE_SURF
    surf = cv::xfeatures2d::SURF::create();
#elif USE_BRIEF_ONLY
    brief = cv::xfeatures2d::BriefDescriptorExtractor::create(BRIEF_DESCRIPTOR_LENGTH);
#elif USE_AKAZE
    akaze = cv::AKAZE::create(cv::AKAZE::DESCRIPTOR_MLDB);
#elif USE_KNN
    memset(l, 0, DATA_LEN);
    memset(r, 0, DATA_LEN);
    memset(t, 0, DATA_LEN);
    memset(b, 0, DATA_LEN);
#endif
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


#if USE_WEIGHTS

void Tracker::calculate(uint8_t *raw_img)
{
    int x,y,i,j, pos, max, max_pos;
    cv::Mat img, unfiltered;
    int sum[DATA_LEN];
    max = 0;
//#if FILTER_IMAGE
    unfiltered = cv::Mat(128,128,CV_8UC1, raw_img);
    cv::blur(unfiltered, img, cv::Size(5,5));
//#else
//    img = cv::Mat(128,128,CV_8UC1, raw_img);
//#endif /*FILTER_IMAGE*/
    for(y = 0; y < 128; y++)
    {
        for(x = 0; x < 128; x++)
        {
            pos = y*128+x;
            sum[pos] = -(int)img.at<uint8_t>(y,x);
            for(i = y-3 < 0? 0:y-3; i < y+3 && i < 128; i++)
                for(j = x-3 < 0? 0:x-3; j < x+3 && j < 128; j++)
                    sum[pos] = (int)img.at<uint8_t>(i,j);
            if(sum[pos] > max)
            {
                max = sum[pos];
                max_pos = pos;
            }
        }
    }
    free(raw_img);
#if DEBUG
    i = max_pos/128;    //y
    j = max_pos%128;    //x
    qimg = QImage(128,128, QImage::Format_RGB32);
    for(x = 0; x < 128; x++)
        for(y = 0; y < 128;y++)
        {
            if(x >= j-3 && x <= j+3 && y >= i-3 && y <= i+3 && max > 100)
                qimg.setPixel(x,y,qRgb(255,255,0));
            else
                qimg.setPixel(x,y,0);
        }
    emit sendFrame(&qimg);

#endif /*DEBUG*/
#if USE_SPHERO
    if(max > 100)
    {
        sphero->setXY((double)j, (double)i);
        sprintf(str_info, "xdirection: %d\tydirection: %d\n\nnext: %hhu | %hhu",
            j, i, (uint8_t)(sphero->get_next() >> 8)&0xff, (uint8_t)(sphero->get_next()&0xff));
    }
#else
    if(max > 100)
        sprintf(str_info, "xdirection: %d\tydirection: %d", j, i);
#endif /*USE_SPHERO*/

    emit send_info(str_info);
}


#elif USE_KNN


#define RDIST dist[0]
#define LDIST dist[1]
#define TDIST dist[2]
#define BDIST dist[3]

/*filter???!!!*/

void Tracker::calculate(uint8_t *raw_img)
{
    int x, y, pos, xtranslate, ytranslate;
    long double dist[4], min, last_min;
    uint8_t inspect[DATA_LEN];
    //uint8_t l2[DATA_LEN], r2[DATA_LEN], b2[DATA_LEN], t2[DATA_LEN];

    xtranslate = 0;
    ytranslate = 0;
    last_min = -1.0;

    if(!object_present)
    {
        for(y = 0; y < 128; y ++)
        {
            for(x = 0; x < 128; x++)
            {
                pos = y * 128 + x;
                l[pos] = x < 127 ? raw_img[pos+1] : 0;
                r[pos] = x > 0 ? raw_img[pos-1] : 0;
                b[pos] = y > 0 ? raw_img[(y-1)*128+x] : 0;
                t[pos] = x < 127 ? raw_img[(y+1)*128+x] : 0;
            }
        }
        object_present = true;
        return;
    }
    else
    {
knn_search_lbl:
        memset(dist, 0, sizeof(dist));
        for(y = 0; y < 128; y++)
        {
            for(x = 0; x < 128; x++)
            {
                pos = y * 128 + x;
                RDIST += pow((long double)(raw_img[pos] - r[pos]), 2.0);
                LDIST += pow((long double)(raw_img[pos] - l[pos]), 2.0);
                TDIST += pow((long double)(raw_img[pos] - t[pos]), 2.0);
                BDIST += pow((long double)(raw_img[pos] - b[pos]), 2.0);
            }
        }
        min = -1.0;
        for(pos = 0; pos < 4; pos++)
        {
            dist[pos] = sqrt(dist[pos]);
            if((min == -1.0 || dist[pos] < min) && dist[pos] < 31)
            {
                min = dist[pos];
                x = pos;
            }
        }

        if(min == -1.0)
        {
            //no match
        }
        else if(min < last_min)
        {

        }

        switch(x)
        {
        case 0:
            memcpy(inspect, r, DATA_LEN);
            xtranslate++;
            break;
        case 1:
            memcpy(inspect, l, DATA_LEN);
            xtranslate--;
            break;
        case 2:
            memcpy(inspect, t, DATA_LEN);
            ytranslate++;
            break;
        case 3:
            memcpy(inspect, d, DATA_LEN);
            ytranslate--;
            break;
        }

        for(y = 0; y < 128; y ++)
        {
            for(x = 0; x < 128; x++)
            {
                pos = y * 128 + x;
                l[pos] = x < 127 ? inspect[pos+1] : 0;
                r[pos] = x > 0 ? inspect[pos-1] : 0;
                b[pos] = y > 0 ? inspect[(y-1)*128+x] : 0;
                t[pos] = x < 127 ? inspect[(y+1)*128+x] : 0;
            }
        }
        goto knn_search_lbl;
    }
}


#undef RDIST
#undef LDIST
#undef TDIST
#undef BDIST

#else


void Tracker::calculate(uint8_t *raw_img)
{
    cv::Mat img, desc, unfiltered;
    std::vector<cv::KeyPoint> kp;
    cv::Ptr<cv::DescriptorMatcher> matcher;
#if MEASURE_TIME
    struct timespec _tstart, _tstop;
    FILE *_tfile;
#endif /*MEASURE_TIME*/
#if USE_ORB
    matcher = cv::DescriptorMatcher::create(MATCHER_ORB);
#elif USE_SURF
    matcher = cv::DescriptorMatcher::create(MATCHER_SURF);
#elif USE_SIFT
    matcher = cv::DescriptorMatcher::create(MATCHER_SIFT);
#elif USE_BRIEF_ONLY
    matcher = cv::DescriptorMatcher::create(MATCHER_BRIEF);
#elif USE_AKAZE
    matcher = cv::DescriptorMatcher::create(MATCHER_AKAZE);
#endif
    std::vector<cv::DMatch>matches, good_matches;
    unsigned int it, i;
    long double max_dist, min_dist, avg_x, avg_y;
    std::vector<cv::KeyPoint> good_old, good_new;

#if FILTER_IMAGE
    unfiltered = cv::Mat(128,128,CV_8UC1, raw_img);
    cv::blur(unfiltered, img, cv::Size(5,5));
#else
    img = cv::Mat(128,128,CV_8UC1, raw_img);
#endif /*FILTER_IMAGE*/
#if MEASURE_TIME
    clock_gettime(CLOCK_REALTIME, &_tstart);
#endif /*MEASURE_TIME*/
#if USE_ORB
    orb.get()->detect(img, kp);
    orb.get()->compute(img, kp, desc);
#elif USE_SIFT
    sift.get()->detect(img, kp);
    sift.get()->compute(img, kp, desc);
#elif USE_SURF
    surf.get()->detect(img, kp);
    surf.get()->compute(img, kp, desc);
#elif USE_BRIEF_ONLY
    for(it = 0; (int)it < img.rows; it++)
    {
        for(i = 0; (int)i < img.cols; i++)
        {
            if(img.at<uint8_t>(it, i) >= 64)
                kp.push_back(cv::KeyPoint((float)i, (float)it, 1));
        }
    }
    brief.get()->compute(img, kp, desc);
#elif USE_AKAZE
    akaze.get()->detect(img, kp);
    akaze.get()->compute(img, kp, desc);
#endif /*ALGORITHM*/
#if MEASURE_TIME
    clock_gettime(CLOCK_REALTIME, &_tstop);
    _tstart.tv_sec = _tstop.tv_sec - _tstart.tv_sec;
    _tstart.tv_nsec = _tstop.tv_nsec - _tstart.tv_nsec;
    _tfile = fopen(MEASURE_TIME_OF, "ab");
    if(_tfile != NULL)
    {
        fwrite(&_tstart, sizeof(_tstart), 1, _tfile);
        fclose(_tfile);
    }
#endif /*MEASURE_TIME*/

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
/*            if(matches.size() < T_MIN_MATCHES)
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
*/
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
/*#if USE_DYNAMIC_ORB
                    avg_x += good_new[x].pt.x;
                    avg_y += good_new[x].pt.y;
#endif /*USE_DYNAMIC_ORB*/
                }
            }
            if(matches.size() < T_MIN_MATCHES || good_new.size() < T_MIN_GOOD_MATCHES)
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
            //TODO: change to random kp
            tracking_point = good_new[0].pt.x + good_new[0].pt.y * 128;
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
        //if(matches.size() >= T_MIN_MATCHES)
        //{
            //object present??
            max_dist = 0;
            min_dist = 100;
            //get min/max distance
            for(i = 0; i < (unsigned int)matches.size()/*descriptors[it].rows*/; i++)
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
            if(good_matches.size() >= T_MIN_GOOD_MATCHES)
            {
                min_dist = 0;
                max_dist = 0;
                for(i = 0; i < (unsigned int)good_matches.size(); i++)
                {
                    //if(kp[good_matches[i].queryIdx].pt.x != 0)
                        max_dist += (double)(kp[good_matches[i].queryIdx].pt.x);
                    //if(kp[good_matches[i].queryIdx].pt.y != 0)
                        min_dist += (double)(kp[good_matches[i].queryIdx].pt.y);
                }
                max_dist /= (long double)i > 0 ? i : 1;
                min_dist /= (long double)i > 0 ? i : 1;
                tracking_point = (int)max_dist + (int)(min_dist * 128.0);
#if USE_SPHERO
                sphero->setXY(max_dist, min_dist);
                sprintf(str_info, "xdirection: %Lf\tydirection: %Lf\n\nnext: %hhu | %hhu",
                        max_dist, min_dist, (uint8_t)(sphero->get_next() >> 8)&0xff, (uint8_t)(sphero->get_next()&0xff));
#else
                sprintf(str_info, "xdirection: %Lf\tydirection: %Lf\n#images: %u",
                        max_dist, min_dist, img_count);
#endif /*USE_SPHERO*/

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
        //}
        free(raw_img);
    }
}

#endif /*USE_WEIGHTS*/


} /*neurocatch*/
