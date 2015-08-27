#include "spherocontroller.h"
#include "math.h"
#include "stdio.h"
#include "stdint.h"


#define SPHERO_SET_COLOR(a) sphero->setColor((uint8_t)((a >> 16) & 0xff), (uint8_t)((a >> 8)& 0xff), (uint8_t)(a & 0xff))
#ifndef PI
#define PI 3.14159265
#endif /*PI*/
#define DEG(a) (((double)a) * 180.0 / PI)
#define DIRECTION(a, phi) ((uint16_t)((((int)a) < 0 ? 360-(int)a: (int)a) + (int)(phi)))
#define XTRANSFORM(x,y,phi) ((x * cos(phi)) + (y * sin(phi)))
#define YTRANSFORM(x,y,phi) (((-x) * sin(phi)) + (y * cos(phi)))
#define TO_COORDINATE(a) ((double)(abs((int)a) % 100))


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
    double angle;
    uint32_t rgb;
    FILE *rf;
    auto getRandom = [&]()->void {
            rf = fopen("/dev/random", "rb");
            if(rf == 0) return;
            fread(&xtarget, 4, 1, rf);
            fread(&ytarget, 4, 1, rf);
            fread(&rgb, 4, 1, rf);
            fclose(rf);
            TO_COORDINATE(xtarget);
            TO_COORDINATE(ytarget);
    };

    //move sphero to track it
    rgb = 0;
    while(!object_present.load())
    {
        sphero->roll(0x2f,(uint16_t)rgb);
        rgb = (rgb + 90) % 360;
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
        sleep(2);
    }
    sphero->roll(0, 0);
    sleep(2);
    //TODO: angle setup
    ytarget = y.load()-ytarget;
    xtarget = x.load()-xtarget;
    angle = DEG(atan2(ytarget,xtarget));
    if(ytarget < 0 && xtarget < 0)
        angle = 360.0 + angle;
    else if(ytarget < 0 && xtarget > 0)
        angle = 180.0 + angle;
    else if(ytarget > 0 && xtarget > 0)
        //angle = angle;
        __asm("nop");
    else if(ytarget > 0 && xtarget < 0)
        angle = 180.0 + angle;
    sphero->setHeading((uint16_t)angle);
    //sphero->roll(0xff, 0);
    //sleep(2);
    //sphero->roll(0,0);
    //getRandom();
    //SPHERO_SET_COLOR(rgb);
    //hypot()
    while(run.load())
    {
        getRandom();
        SPHERO_SET_COLOR(rgb);
        angle = DEG(atan2(ytarget-y.load(), xtarget-x.load()));
        if(ytarget < 0 && xtarget < 0)
            angle = -angle;
        else if(ytarget < 0 && xtarget > 0)
            angle = 180.0 - angle;
        else if(ytarget > 0 && xtarget > 0)
            angle = 360.0 - angle;
        else if(ytarget > 0 && xtarget < 0)
            angle = 180.0 - angle;
        sphero->roll(0x2f, (uint16_t)angle);
        while(abs((int)(x.load()-xtarget)) > 10 || abs((int)(y.load()-ytarget)) > 10);
        sphero->roll(0,0);
        sleep(5);
    }
}


} /*neurocatch2015*/
