#include <iostream>
#include <string>

#include "utils.h"


void convert(double total, char* str) {
    sprintf(str, "%.2lfs", total);
}

class ProgressBar {
public:
    char begin = '[';
    char fill = '=';
    char lead = '>';
    char rest = '.';
    char end = ']';

    double min = 0.;
    double max = 100.;
    double current = 0.;
    double step = 1.0;

    bool finished = false;

    unsigned int width = 35;
    unsigned long long t0 = 0;


    void start() {
        t0 = currTimeNano();
    }

    void update() {
        std::string progressLine = "\r";

        double completedPercentage = (current - min) / (max - min);
        char str[256];
        sprintf(str, "%d%%", (int) (completedPercentage * 100.));
        progressLine += str;

        progressLine += begin;
        for(unsigned int i = 0; i < (unsigned int) width * completedPercentage; i++) {
            progressLine += fill;
        }
        if(completedPercentage < 1.0) {
            progressLine += lead;
            for(unsigned int i = ((unsigned int) (width * completedPercentage)) + 1; i < width; i++) {
                progressLine += rest;
            }
        }
        progressLine += end;

        double timeSpentSeconds = (double) (currTimeNano() - t0) * (double) 1e-9;
        double etaSeconds = timeSpentSeconds / completedPercentage;
        double iterationsPerSecond = (current - min) / timeSpentSeconds;
        char spentTime[256];
        char estimatedTime[256];
        convert(timeSpentSeconds, spentTime);
        convert(etaSeconds, estimatedTime);
        sprintf(str, " %u/%u [%s<%s, %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, estimatedTime, iterationsPerSecond);
        progressLine += str;

        //printf("%s", progressLine.c_str());
        //std::cout << progressLine;
        fprintf(stderr, "%s", progressLine.c_str());

        if(current >= max) {
            finished = true;
        }
    }

    void tick() {
        setCurrent(current + step);
        update();
    }

    void finish() {
        update();
        printf("\n");
    }

    void setCurrent(double newCurrent) {
        if(newCurrent  >= min && newCurrent <= max) {
            current = newCurrent;
        } else if(newCurrent > max) {
            current = max;
        } else if(newCurrent < min) {
            current = min;
        }
    }

    void setMin(double newMin) {
        if(newMin < max) {
            min = newMin;
        }
    }

    void setMax(double newMax) {
        if(newMax > min) {
            max = newMax;
        }
    }

    bool isFinished() {
        return finished;
    }

};
