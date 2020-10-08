#include <cstdio>
#include <string>

#include "utils.h"

void initArgs(struct globalArgs_t *args) {
    args->debug            = false;
    args->directed         = true;
    args->inputFile        = "";
    args->outputDir        = "./output";
    args->thNum            = 0;
    args->startID          = 0;
    args->finishID         = 0;
    args->autosaveInterval = 0;
    args->norenumeration   = false;

    return;
}

void printArgs(struct globalArgs_t *args) {
    printf("Global arguments:\n");

    if(args->debug) {
        printf("\tdebug = true\n");
    } else {
        printf("\tdebug = false\n");
    }

    if(args->directed) {
        printf("\tdirected = true\n");
    } else {
        printf("\tdirected = false\n");
    }

    printf("\tinputFile = %s\n", (args->inputFile).c_str());
    printf("\toutputDir = %s\n", (args->outputDir).c_str());

    printf("\tthNum = %d\n", args->thNum);

    printf("\tstartID = %u\n", args->startID);
    printf("\tfinishID = %u\n", args->finishID);

    printf("\tautosaveInterval = %u\n", args->autosaveInterval);
    if(args->norenumeration) {
        printf("\tnorenumeration = true\n");
    } else {
        printf("\tnorenumeration = false\n");
    }

    return;
}

unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}
