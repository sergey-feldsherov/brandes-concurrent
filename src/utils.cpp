#include <stdio.h>
#include <string>

#include "utils.h"

void initArgs(struct globalArgs_t *args) {
    args->debug      = false;
    args->inputFile  = "";
    args->outputFile = "output/output.ranking";
    args->thNum      = 0;

    return;
}

void printArgs(struct globalArgs_t *args) {
    printf("Global arguments:\n");

    if(args->debug) {
        printf("\tdebug = true\n");
    } else {
        printf("\tdebug = false\n");
    }

    printf("\tinputFile = %s\n", (args->inputFile).c_str());
    printf("\toutputFile = %s\n", (args->outputFile).c_str());

    printf("\tthNum = %d\n", args->thNum);

    return;
}

unsigned long long currTimeNano()   {
    struct timespec t;
    clock_gettime ( CLOCK_MONOTONIC, &t );
    return t.tv_sec*1000000000 + t.tv_nsec;
}
