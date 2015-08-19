#ifndef SPHEROCONTROLLER_H
#define SPHEROCONTROLLER_H

#include "sphero/bluetooth/bluez_adaptor.h"
#include "sphero/Sphero.hpp"
#include "sphero/packets/Constants.hpp"
#include <atomic>
#include "semaphore.h"
#include "thread"

#include "settings.h"

namespace neurocatch
{

class SpheroController
{
public:
    SpheroController();
    ~SpheroController();
    bool connected();
    void setXY(double x, double y);
    void setX(double x);
    void setY(double y);
    void signal_obj_present();

private:
    Sphero *sphero;
    std::atomic<double> x, y;
    //sem_t obj_present;
    std::atomic<bool> run, object_present;
    std::thread ctrl;

    void controller_loop();
};

} /*neurocatch2015*/

#endif // SPHEROCONTROLLER_H
