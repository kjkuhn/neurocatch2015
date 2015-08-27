#include "stdio.h"
#include "string.h"
#include "iostream"
#include "math.h"
#include "sphero/bluetooth/bluez_adaptor.h"
#include "sphero/Sphero.hpp"
#include "sphero/packets/Constants.hpp"

#define SPHERO_MAC                  	"00:06:66:44:64:F7"
#define USE_SPHERO			0
#define PI 				3.14159265359
#define DEG(a) 				(a * 180.0 / PI)
#define RANDOM_FILE			"/dev/random"


Sphero *s;


void random_t()
{
	FILE *f;
	int i;
	double d;
	f = fopen(RANDOM_FILE, "rb");
	printf("file opened\n");
	fread(&i, 4,1,f);
	printf("int read\n");
	fread(&d,4,1,f);
	printf("double read\n");
	fclose(f);
	printf("file closed\n");
	printf("int: %d\tdouble: %d\n", i, (int)d);
}


int main(int argc, char** argv)
{
#if USE_SPHERO
	s = new Sphero(SPHERO_MAC, new bluez_adaptor());
	s->onConnect([](){std::cout << "connection established" << std::endl;});
	if(s->connect())
		std::cout << "all went fine" << std::endl;
	s->roll(0x2f, 0);
	sleep(2);
	s->roll(0,0);
	sleep(2);
	s->roll(0x2f, 339);
	sleep(2);
	s->roll(0,0);
#endif
//	printf("atan: %f\natan2: %f\n",DEG(atan(13.0/20.0)),DEG(atan2(30.0-43.0,65.0-85.0)));
	printf("-13, -20: %f\n-13, 20: %f\n13, 20: %f\n13, -20: %f\n",
		DEG(atan2(-13.0, -20.0)), DEG(atan2(-13.0,20.0)),
		DEG(atan2(13.0,20.0)), DEG(atan2(13.0,-20.0)));
//	getc(stdin);
	random_t();
	return 0;
}
