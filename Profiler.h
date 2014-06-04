#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#endif

typedef enum{
	PRECISION_NS,
	PRECISION_US,
	PRECISION_MS,
	PRECISION_S,
} timer_precision;

typedef enum {
	TIMER_REALTIME,
	TIMER_MONOTONIC,
} timer_mode;

typedef struct {
	void* data;
	timer_precision precision;
	timer_mode mode;
} Timer;

Timer* TimerStart();
double TimerLap(Timer* t);
double TimerEnd(Timer* t);
char* GetMemoryRepr(char* buffer, unsigned int i);
unsigned int MemoryInfo();