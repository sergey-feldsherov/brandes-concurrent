#include <time.h>
#include "timing.h"

unsigned long long currTimeNano(void) {
	struct timespec t;
	clock_gettime (CLOCK_MONOTONIC, &t);
	return t.tv_sec*1000000000 + t.tv_nsec;
}

unsigned long long currTimeNanoThread(void) {
	struct timespec t;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
	return t.tv_sec*1000000000 + t.tv_nsec;
}

