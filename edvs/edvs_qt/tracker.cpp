#include "tracker.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"


#ifndef T_WINDOW_NAME
#define T_WINDOW_NAME "tracker window"
#endif

#ifndef T_OBJ_DESC
#define T_OBJ_DESC  3
#endif

namespace neurocatch
{



Tracker::Tracker()
{
    this->orb = cv::ORB::create(500, 2, 8, 2, 0,4,cv::ORB::FAST_SCORE, 2, 20);
    run.store(true);
    //error checking?
    sem_init(&this->wl_count, 0, 0);
    this->worker = std::thread(&Tracker::process, this);
    cv::namedWindow(T_WINDOW_NAME);
}


Tracker::~Tracker()
{
    run.store(false);
    sem_post(&wl_count);
    worker.join();
}


void Tracker::add_to_wl(const uint8_t *buf, size_t rows, size_t cols)
{
    cv::Mat img(rows,cols,CV_8UC1, buf);
    wl_mtx.lock();
    wl.push(img);
    wl_mtx.unlock();
    sem_post(&wl_count);
}


void Tracker::process()
{
    cv::Mat img, desc;
    std::vector<cv::KeyPoint> kp;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    std::vector<cv::Mat>::iterator it;
    std::vector<cv::DMatch>matches, closest_matches;
    int num_matches;
    while(1)
    {
        sem_wait(&wl_count);
        if(!run.load())
            break;
        wl_mtx.lock();
        img = wl.front();
        wl.pop();
        wl_mtx.unlock();
        orb.get()->detect(img, kp);
        orb.get()->compute(img, kp, desc);
        if(descriptor.size() < T_OBJ_DESC)
            descriptor.push_back(desc);
        else
        {
            matcher.get()->match(desc, *descriptors.begin(), matches);
            num_matches = matches.size();
            Mat table(num_matches, 1, CV_32F);
            for(int i = 0; i < num_matches; i++)
                table.at<float>(i,0) = matches[i].distance();
            cv::Mat index;
            cv::sortIdx(table, index, cv::SORT_EVERY_COLUMN + cv::SORT_ASCENDING);
            for(int i = 0; i < 30; i++)
                closest_matches.push_back(matches[index.at<int>(i,0)]);

            for(it = descriptors.begin()++; it != descriptors.end(); it++)
            {
                matcher.get()->match(desc, *it, matches);

            }
        }
    }

}


} /*neurocatch*/
