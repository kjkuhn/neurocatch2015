#include "Edvs.h"
#include "edvs_pos.h"
//#include <pthread.h>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string.h>
#include <iostream>


#define DATA1       	data_buffer[0]
#define DATA2       	data_buffer[1]
#define DATA_LEN    	128*128
#define LED_FREQUENCY   1000    /*Hz*/
#define FILTER_DIFF 	10
#define TOLERANCE	20



const int cDecay = (2 * 256) / 60;
const int cDisplaySize = 512;
const int cUpdateInterval = FILTER_DIFF*2;
const Edvs::Baudrate cBaudrate = Edvs::B4000k;

short data_buffer[2][DATA_LEN];
short *active;

Edvs::Device device;
Edvs::EventCapture capture;

std::thread thd;

int run;


void stop()
{
	run = 0;
}


short get_direction()
{
	short result = 0;
	if(current_adjustment[0] < -TOLERANCE)
		result |= ((short)'L') << 8;
	else if(current_adjustment[0] > TOLERANCE)
 		result |= ((short)'R') << 8;
	if(current_adjustment[1] < -TOLERANCE)
		result |= ((short)'U');
	else if(current_adjustment[1] > TOLERANCE)
 		result |= ((short)'D');
	return result;	
}


void OnEvent(const std::vector<Edvs::Event>& events)
{
    for(std::vector<Edvs::Event>::const_iterator it=events.begin(); it!=events.end(); it++) {
 
       active[it->y*128+it->x] += 1;
    }
}



short* adjust(short last, short first)
{
    short top, bottom, left, right;
    short *ret = (short*)malloc(4);
    top = first / 128;
    left = first % 128;
    right = 128 - last % 128;
    bottom = 128 - last / 128;
//    printf("first: %hd\tlast: %hd\n", first, last);
//    printf("top: %hd\tbottom: %hd\tleft: %hd\tright: %hd\n",top,bottom,left,right);
    ret[0] = (left-right) / 2;
    ret[1] = (top - bottom) / 2;
    return ret;
}


void update()
{
	int i;
    	short *nactive;
    	short first, last;
    	first = 0;
    	last = 0;

	if(active == DATA1)
    	{
        	active = DATA2;
        	nactive = DATA1;
   	 }
    	else
    	{
        	active = DATA1;
        	nactive = DATA2;
   	 }

    	for(i = 0; i < DATA_LEN; i++)
    	{
        	nactive[i] -= FILTER_DIFF;
        	if(nactive[i] > 0)
        	{
            		if(first == 0)
                		first = i;
            		last = i;
        	}
		nactive[i] = 0;
    	}
	//memset(nactive, 0, DATA_LEN);
	nactive = adjust(last, first);
	memcpy(current_adjustment, nactive, 4);
#if PRINT_EDVS_POS_INFO
	printf("x: %d\ty: %d\n", current_adjustment[0],current_adjustment[1]);
	first = get_direction();
	printf("horizontal: %c\tvertical: %c\n", ((char)(first>>8)), (char) first & 0xff);
#endif
	free(nactive);
}


void thread_entry()
{
	printf("Thread initialized\n");
	while(run)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(cUpdateInterval));
		update(); 
	}
}


void edvs_init()
{
	memset(DATA1, 0, DATA_LEN);
    	memset(DATA2, 0, DATA_LEN);

    	active = DATA1;

	run = 1;

	device = Edvs::Device(cBaudrate);
    	capture = Edvs::EventCapture(device, boost::bind(OnEvent, _1));
	thd = std::thread(thread_entry);	
}

#if STANDALONE
int main(int argc, char** argv)
{
	edvs_init();
	while(1);
	return 0;
}
#endif
