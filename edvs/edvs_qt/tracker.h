#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include "opencv2/features2d.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "vector"
#include "queue"
#include "thread"
#include "mutex"
#include "semaphore.h"
#include <atomic>


namespace neurocatch{

class Tracker:public QObject
{
    Q_OBJECT

public:
    Tracker();
    ~Tracker();

    void add_to_wl(const uint8_t *buf, size_t rows=128, size_t cols=128);

private:
    cv::Ptr<cv::ORB> orb;
    std::thread worker;
    std::queue<cv::Mat> wl;     //working list
    sem_t wl_count;
    std::mutex wl_mtx;
    std::atomic<bool> run;
    std::vector<cv::Mat> descriptors;

    void process();
};

} /*neurocatch*/

#endif // TRACKER_H
