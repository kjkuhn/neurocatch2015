#ifndef FRAMEMANAGER_H
#define FRAMEMANAGER_H

#include "settings.h"
#include "semaphore.h"
#include "thread"
#include "deque"
#include "stdint.h"
#include "mutex"
#include "atomic"



namespace neurocatch
{

class FrameManager
{
public:
    FrameManager();
    ~FrameManager();
    void push_evt(uint32_t evt);
    uint8_t* next();

private:
    std::thread mgr;
    std::mutex mtx;
    sem_t sema;
    std::deque<uint8_t*> frames;
    std::atomic<bool> run;


    void manage();
};



}/*neurocatch*/

#endif // FRAMEMANAGER_H
