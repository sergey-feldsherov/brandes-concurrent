#include <iostream>
#include <string>
#include "utils.h"

class ProgressBar {
public:
    char p_begin = '[';
    char p_fill = '=';
    char p_lead = '>';
    char p_rest = '.';
    char p_end = ']';

    double p_min = 0.;
    double p_max = 100.;
    double p_current = 0.;
    double p_step = 1.0;

    unsigned int p_width = 60;
    unsigned long long p_t0 = 0;

    ProgressBar() {
        p_t0 = currTimeNano();
    }

    void update() {
        std::string progressLine = "\r";

        double completedPercentage = (p_current - p_min) / (p_max - p_min);
        char str[256];
        sprintf(str, "%d%%", (int) (completedPercentage * 100.));
        progressLine += str;

        progressLine += p_begin;
        for(unsigned int i = 0; i < (unsigned int) p_width * completedPercentage; i++) {
            progressLine += p_fill;
        }
        if(completedPercentage < 1.0) {
            progressLine += p_lead;
            for(unsigned int i = ((unsigned int) (p_width * completedPercentage)) + 1; i < p_width; i++) {
                progressLine += p_rest;
            }
        }
        progressLine += p_end;

        double timeSpentSeconds = (double) (currTimeNano() - p_t0) * (double) 1e-9;
        double etaSeconds = timeSpentSeconds / completedPercentage;
        double iterationsPerSecond = (p_current - p_min) / timeSpentSeconds;
        sprintf(str, " %d/%d [%.2fs<%.2fs, %.1fit/s]", (int)(p_current-p_min), (int)(p_max-p_min), timeSpentSeconds, etaSeconds, iterationsPerSecond);
        progressLine += str;

        //printf("%s", progressLine.c_str());
        //std::cout << progressLine;
        fprintf(stderr, "%s", progressLine.c_str());
    }

    void tick() {
        setCurrent(p_current + p_step);
        update();
    }

    void finish() {
        update();
        printf("\n");
    }

    void setCurrent(double newCurrent) {
        if(newCurrent  >= p_min && newCurrent <= p_max) {
            p_current = newCurrent;
        }
    }

    void setMin(double newMin) {
        if(newMin < p_max) {
            p_min = newMin;
        }
    }

    void setMax(double newMax) {
        if(newMax > p_min) {
            p_max = newMax;
        }
    }

};
