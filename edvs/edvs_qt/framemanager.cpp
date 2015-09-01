#include "framemanager.h"
#include "unistd.h"
#include "string.h"
#include "time.h"

#define BB_SIZE         32
#define BB_THRESHOLD    5
#define WAITING_SEMA    0


#if USE_DYNAMIC_ORB
static const __useconds_t time_to_sleep = UPDATE_INTERVAL * 1000;
static QImage qimg;
#else
static const __useconds_t time_to_sleep = (UPDATE_INTERVAL-FRAME_OVERLAPPING)*1000;
#endif /*USE_DYNAMIC_ORB*/

namespace neurocatch
{


FrameManager::FrameManager(Tracker *tracker)
{
    run.store(true);
#if USE_DYNAMIC_ORB
    sem_init(&sema, 0, 0);
#else
    sem_init(&sema, 0, 1);
#endif /*USE_DYNAMIC_ORB*/
    this->tracker = tracker;
    count = 0;
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


#if !USE_DYNAMIC_ORB
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


#else
void FrameManager::manage()
{
    uint8_t buffer[2][DATA_LEN];
    active = buffer[0];
    uint8_t num = 0;
    int i, sem_result;
    struct timespec tspec;

    sem_result = 0;
    tspec.tv_sec = 0;
    tspec.tv_nsec = time_to_sleep * 1000;
    memset(buffer[0], 0, DATA_LEN);
    memset(buffer[1], 0, DATA_LEN);
    while(run)
    {
        mtx.lock();
        active = buffer[num];
        mtx.unlock();
        num = (num + 1) % 2;
        qimg = QImage(128,128, QImage::Format_RGB32);
        for(i = 0; i < DATA_LEN; i++)
        {
            if((char)buffer[num][i] > 0)
                buffer[num][i] = 250;
            else if((char)buffer[num][i] < 0)
                buffer[num][i] = 100;
            qimg.setPixel(i%128, i/128, qRgb(buffer[num][i] == 100 ? 255 : 0, buffer[num][i] == 250 ? 255 : 0, 0));
        }
        emit update_img(&qimg);
        if(sem_result == 0)
            tracker->add_to_wl(buffer[num]);
        FILE *f = fopen("data.dat", "ab");
        fwrite(buffer[num],1, DATA_LEN, f);
        fclose(f);
        memset(buffer[num], 0, DATA_LEN);
        if(tracker->static_frame())
            usleep(time_to_sleep);
        else
        {
#if WAITING_SEMA
            sem_result = sem_timedwait(&sema, &tspec);
#else
            sem_wait(&sema);
#endif /*WAITING_SEMA*/
        }
    }
}


void FrameManager::push_evt(uint32_t evt, bool parity)
{
    int x1,x2,y1,y2;
    mtx.lock();
    active[evt] += parity ? 1 : -1;
    mtx.unlock();
    if(!tracker->static_frame())
    {
        x1 = tracker->getTrackingPoint();
        y1 = x1 / 128;
        x1 = x1 % 128;
        x2 = evt % 128;
        y2 = evt / 128;
        if(x2 > x1-BB_SIZE && x2 < x1 +BB_SIZE && y2 > y1-BB_SIZE && y2 < y1+BB_SIZE)
            count++;
        if(count >= BB_THRESHOLD)
        {
            sem_post(&sema);
            count = 0;
        }
    }
}

#endif /*!USE_DYNAMIC_ORB*/


}/*neurocatch*/
