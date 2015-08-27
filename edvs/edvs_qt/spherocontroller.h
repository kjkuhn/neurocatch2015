#ifndef SPHEROCONTROLLER_H
#define SPHEROCONTROLLER_H

#include "QObject"
#include "sphero/bluetooth/bluez_adaptor.h"
#include "sphero/Sphero.hpp"
#include "sphero/packets/Constants.hpp"
#include <atomic>
#include "semaphore.h"
#include "thread"

#include "settings.h"



namespace neurocatch
{

class SpheroController:public QObject
{
    Q_OBJECT

public:
    SpheroController();
    ~SpheroController();
    bool connected();
    void setXY(double x, double y);
    void setX(double x);
    void setY(double y);
    void signal_obj_present();
    void signal_next_pos();
    uint16_t get_next(){return next_direction;}


signals:
    void position_reached(void);

private:
    Sphero *sphero;
    std::atomic<double> x, y;
    sem_t next_pos;
    std::atomic<bool> run, object_present;
    std::thread ctrl;
    std::atomic_char16_t next_direction;

    void controller_loop();
};

} /*neurocatch*/

#endif // SPHEROCONTROLLER_H
