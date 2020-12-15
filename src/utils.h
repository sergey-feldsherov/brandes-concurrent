#pragma once
#include <list>
#include <vector>
#include <string>

struct globalArgs_t {
    bool debug;
    bool directed;
    std::string inputFile;
    std::string outputDir;
    std::string saveFilePrefix;
    int thNum;
    unsigned long long startID;
    unsigned long long finishID;
    unsigned int autosaveInterval;
    bool norenumeration;
};

void initArgs(struct globalArgs_t *args);
void printArgs(struct globalArgs_t *args);

unsigned long long currTimeNano();
