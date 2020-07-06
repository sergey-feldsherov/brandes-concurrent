#pragma once
#include <list>
#include <vector>
#include <string>

struct globalArgs_t {
    bool debug;
    bool directed;
    std::string inputFile;
    std::string outputFile;
    int thNum;
};

void initArgs(struct globalArgs_t *args);
void printArgs(struct globalArgs_t *args);

unsigned long long currTimeNano();
