#include "spherocontroller.h"
#include "math.h"
#include "stdio.h"


#define SPHERO_SET_COLOR(a) sphero->setColor((uint8_t)((a >> 16) & 0xff), (uint8_t)((a >> 8)& 0xff), (uint8_t)(a & 0xff))

namespace neurocatch
{


SpheroController::SpheroController()
{
    run.store(true);
    sphero = new Sphero(SPHERO_MAC, new bluez_adaptor());
    sphero->onConnect([&](){ctrl = std::thread(&SpheroController::controller_loop, this);});
    //sem_init(&obj_present, 0, 0);
    object_present.store(false);
    //ctrl = std::thread(&SpheroController::controller_loop, this);
    sphero->connect();
}


SpheroController::~SpheroController()
{

}


bool SpheroController::connected(){return sphero->isConnected();}


void SpheroController::setX(double x){this->x.store(x);}


void SpheroController::setY(double y){this->y.store(y);}


void SpheroController::setXY(double x, double y)
{
    this->x.store(x);
    this->y.store(y);
    if(object_present == false)
        object_present = true;
}


void SpheroController::signal_obj_present(){object_present.store(true);/*sem_post(&obj_present);*/}


void SpheroController::controller_loop()
{
    double xtarget, ytarget;
    uint32_t rgb;
    FILE *rf;
    auto getRandom = [&]()->void {
            rf = fopen("/dev/random", "rb");
            if(rf == 0) return;
            fread(&xtarget, sizeof(double), 1, rf);
            fread(&ytarget, sizeof(double), 1, rf);
            fread(&rgb, sizeof(uint32_t), 1, rf);
            fclose(rf);
    };
    //move sphero to track it
    rgb = 0;
    while(!object_present.load())
    {
        sphero->roll(0x2f,(uint16_t)rgb);
        rgb = (rgb + 180) % 360;
        sleep(1);
    }
    sphero->roll(0x00, 0);
    getRandom();
    SPHERO_SET_COLOR(rgb);
    xtarget = x.load();
    ytarget = y.load();
    while(xtarget == x.load() && ytarget == y.load())
    {
        sphero->roll(0x2f,0);
    }

    while(run.load())
    {

        getRandom();
        SPHERO_SET_COLOR(rgb);

    }
}


} /*neurocatch2015*/
