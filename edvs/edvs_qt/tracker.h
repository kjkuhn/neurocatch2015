#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include "opencv2/features2d.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
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

signals:
    void sendFrame(QImage *img);
    void send_info(const char *str);

private:
    cv::Ptr<cv::ORB> orb;
    std::thread worker;
    std::queue<uint8_t*> wl;     //working list
    sem_t wl_count;
    std::mutex wl_mtx;
    std::atomic<bool> run, object_present;
    std::deque<cv::Mat> descriptors;
    std::deque<std::vector<cv::KeyPoint>> keypoints;
    std::deque<uint8_t*> images;
    neurocatch::SpheroController *sphero;



    void process();
    void calculate(uint8_t*);
};

} /*neurocatch*/

#endif // TRACKER_H
