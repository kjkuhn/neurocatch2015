#include "framemanager.h"
#include "unistd.h"
#include "string.h"


namespace neurocatch
{


static const __useconds_t time_to_sleep = (UPDATE_INTERVAL-FRAME_OVERLAPPING)*1000;


FrameManager::FrameManager()
{
    run.store(true);
    sem_init(&sema, 0, 1);
    mgr = std::thread(&FrameManager::manage, this);
}


FrameManager::~FrameManager()
{
    std::deque<uint8_t*>::iterator it;
    run.store(false);
    sem_post(&sema);
    mgr.join();
    for(it = frames.begin(); it != frames.end(); it++)
        free(*it);
}


void FrameManager::push_evt(uint32_t evt)
{
    std::deque<uint8_t*>::iterator it;
    mtx.lock();
    for(it = frames.begin(); it != frames.end(); it++)
        (*it)[evt]++;
    mtx.unlock();
}


/*
 *
 * reactivates timer and returns next frame
 *
*/
uint8_t* FrameManager::next()
{
    uint8_t *ret;
    mtx.lock();
    ret = frames.front();
    frames.pop_front();
    mtx.unlock();
    sem_post(&sema);
    return ret;
}


void FrameManager::manage()
{
    uint8_t *buffer;
    while(run.load())
    {
        sem_wait(&sema);
        if(!run.load())
            break;
        usleep(time_to_sleep);
        buffer = (uint8_t*)malloc(DATA_LEN);
        memset(buffer, 0, DATA_LEN);
        mtx.lock();
        frames.push_back(buffer);
        mtx.unlock();
    }
}

}/*neurocatch*/
