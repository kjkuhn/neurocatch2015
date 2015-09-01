#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include "opencv2/features2d.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "vector"
#include "queue"
#include "deque"
#include "thread"
#include "mutex"
#include "semaphore.h"
#include <atomic>
#include "spherocontroller.h"
#include "settings.h"


namespace neurocatch{

class Tracker:public QObject
{
    Q_OBJECT

public:
    Tracker();
    ~Tracker();

    void add_to_wl(uint8_t *buf, size_t rows=128, size_t cols=128);
    void getKeyPoints(std::vector<cv::KeyPoint>&);
    neurocatch::SpheroController *sphero;
    bool static_frame(){return !object_present;}
    int getTrackingPoint(){return tracking_point;}

signals:
    void sendFrame(QImage *img);
    void send_info(const char *str);


private:
#if USE_ORB || USE_DYNAMIC_ORB
    cv::Ptr<cv::ORB> orb;
#elif USE_SIFT
    cv::Ptr<cv::xfeatures2d::SIFT> sift;
#elif USE_SURF
    cv::Ptr<cv::xfeatures2d::SURF> surf;
#elif USE_BRIEF_ONLY
    cv::Ptr<cv::xfeatures2d::BriefDescriptorExtractor> brief;
#endif /*ALGORITHM*/

    std::thread worker;
    std::queue<uint8_t*> wl;     //working list
    sem_t wl_count;
    std::mutex wl_mtx;
    std::atomic<bool> run, object_present;
    std::deque<cv::Mat> descriptors;
    std::deque<std::vector<cv::KeyPoint>> keypoints;
    std::deque<uint8_t*> images; 
    std::atomic<int> tracking_point;


    void process();
    void calculate(uint8_t*);
};

} /*neurocatch*/

#endif // TRACKER_H
