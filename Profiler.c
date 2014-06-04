#include "Profiler.h"

Timer* TimerStart() {
	Timer* t = (Timer*)malloc(sizeof(Timer));
	t->mode = TIMER_REALTIME;
	t->precision = PRECISION_MS;
#ifdef __linux__
	struct timespec *start = (struct timespec*)malloc(sizeof(struct timespec));
	clock_gettime(CLOCK_MONOTONIC,start);
	t->data=start;
#endif
#ifdef _WIN32
	LARGE_INTEGER *start = (LARGE_INTEGER*)malloc(sizeof(LARGE_INTEGER));
	QueryPerformanceCounter(start);
	t->data = start;
#endif
	return t;
}

double TimerLap(Timer* t) {
	double val;
#ifdef __linux__
	struct timespec *end = (struct timespec*)malloc(sizeof(struct timespec));
	clock_gettime(CLOCK_MONOTONIC, end);
	struct timespec *start = (struct timespec*)t->data;
	double ndiff = (double)(end->tv_nsec-start->tv_nsec)/1000000000;
	val = ndiff + end->tv_sec - start->tv_sec;
	free(start);
	t->data = end;
#endif
#ifdef _WIN32
	LARGE_INTEGER *end = (LARGE_INTEGER*)malloc(sizeof(LARGE_INTEGER));
	QueryPerformanceCounter(end);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER* start = (LARGE_INTEGER*)t->data;
	val = ((double)(end->QuadPart - start->QuadPart) / freq.QuadPart);
	free(start);
	t->data = end;
#endif
	return val;
}

double TimerEnd(Timer* t) {
	double val;
#ifdef __linux__
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	struct timespec start = *(struct timespec*)t->data;
	double ndiff = (double)(end.tv_nsec-start.tv_nsec)/1000000000;
	val=ndiff+end.tv_sec-start.tv_sec;
#endif
#ifdef _WIN32
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER start = *(LARGE_INTEGER*)t->data;
	val = ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart);
#endif
	free(t->data);
	free(t);
	return val;
}

char* GetMemoryRepr(char* buffer, unsigned int i) {
	if (i>=10000000) {
		sprintf(buffer,"%4uMB",i/1000000);
	} else if (i>=10000) {
		sprintf(buffer,"%4uKB",i/1000);
	} else {
		sprintf(buffer,"%4uB",i);
	}
	return buffer;
}

unsigned int MemoryInfo() {
	unsigned int info=-1;
#ifdef __linux__	
	FILE *f = fopen("/proc/self/statm","r");
	unsigned int vmsize,vmrss,vmshared;
	if(f){
		if (fscanf(f,"%u %u %u",&vmsize,&vmrss,&vmshared)==3) {
			int pagesize = getpagesize();
			info=(vmrss-vmshared)*pagesize;
		}
		fclose(f);
	}
#endif
	return info;
}

	