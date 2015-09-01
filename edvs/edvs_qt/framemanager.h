#ifndef FRAMEMANAGER_H
#define FRAMEMANAGER_H

#include <QObject>
#include "QImage"

#include "settings.h"
#include "semaphore.h"
#include "thread"
#include "deque"
#include "stdint.h"
#include "mutex"
#include "atomic"
#include "tracker.h"




namespace neurocatch
{

class FrameManager:public QObject
{
    Q_OBJECT

public:
    FrameManager(Tracker *tracker = 0);
    ~FrameManager();

#if !USE_DYNAMIC_ORB
    void push_evt(uint32_t evt);
    uint8_t* next();

#else
    void push_evt(uint32_t evt, bool parity);
#endif /*!USE_DYNAMIC_ORB*/

signals:
    void update_img(QImage *img);

private:
    std::thread mgr;
    std::mutex mtx;
    sem_t sema;
    std::deque<uint8_t*> frames;
    uint8_t *active;
    std::atomic<bool> run;
    uint32_t count;
    Tracker *tracker;


    void manage();
};



}/*neurocatch*/

#endif // FRAMEMANAGER_H
