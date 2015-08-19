#include "stdio.h"
#include "string.h"
#include "iostream"

#include "sphero/bluetooth/bluez_adaptor.h"
#include "sphero/Sphero.hpp"
#include "sphero/packets/Constants.hpp"

#define SPHERO_MAC                  "00:06:66:44:64:F7"



Sphero *s;

int main(int argc, char** argv)
{
	s = new Sphero(SPHERO_MAC, new bluez_adaptor());
	s->onConnect([](){std::cout << "connection established" << std::endl;});
	if(s->connect())
		std::cout << "all went fine" << std::endl;
	getc(stdin);
	return 0;
}
