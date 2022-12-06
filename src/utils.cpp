#include <cstdio>
#include <string>
#include <unistd.h>
#include <time.h>

#include "utils.h"

void initArgs(struct globalArgs_t *args) {
    args->debug            = false;
    args->directed         = true;
    args->inputFile        = "./input/example.txt";
    args->outputDir        = "./output/";
    args->saveFilePrefix   = "./output/example";
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
    printf("\tsaveFilePrefix = %s\n", (args->saveFilePrefix).c_str());

    printf("\tthNum = %d\n", args->thNum);

    printf("\tstartID = %llu\n", args->startID);
    printf("\tfinishID = %llu\n", args->finishID);

    printf("\tautosaveInterval = %u\n", args->autosaveInterval);
    if(args->norenumeration) {
        printf("\tnorenumeration = true\n");
    } else {
        printf("\tnorenumeration = false\n");
    }

    return;
}

