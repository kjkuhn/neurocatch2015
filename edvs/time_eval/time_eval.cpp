#include "stdio.h"
#include "stdlib.h"
#include "time.h"

typedef long double uldouble;



int main(int argc, char** argv)
{
	FILE *f;
	int i, count;
	struct timespec ts;
	time_t sec;
	long nsec;
	uldouble result;
	if(argc != 3)
	{
		printf("please specify as first argument the file to evaluate and as second the number of samples\n");
		return 0;
	}
	sscanf(argv[2], "%d", &count);
	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		printf("invalid file\n");
		return 0;
	}
	nsec = 0;
	sec = 0;
	for(i = 0; i < count && !feof(f); i++)
	{
		fread(&ts, sizeof(ts), 1, f);
		nsec += ts.tv_nsec/1000;
		sec += ts.tv_sec;
	}
	result = ((uldouble)sec)/((uldouble)count *1000000.0) +
		((uldouble)(nsec/count));
	printf("%Lfus\n", result);
	return 0;
}
