#ifndef EDVS_POS
#define EDVS_POS

#define STANDALONE 		1
#define PRINT_EDVS_POS_INFO	1

void stop();
void edvs_init();
short get_direction();

short current_adjustment[2]; 

#endif
