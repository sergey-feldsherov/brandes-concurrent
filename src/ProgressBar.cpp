#include <thread>
#include <cassert>
#include <time.h>

#include "ProgressBar.h"

ProgressBar::ProgressBar() {
    //
}

void ProgressBar::setMin(double newMin) {
    assert(not threadRunning);
    if(newMin < max) {
        min = newMin;
    }
}

void ProgressBar::setMax(double newMax) {
    assert(not threadRunning);
    if(newMax > min) {
        max = newMax;
    }
}

void ProgressBar::start() {
    assert(not threadRunning);

    timeToStop.store(false);
    timeToUpdate.store(true);
    finished = false;
    printingThread = std::thread([this] (){this->loop();});

    threadRunning = true;
}

void ProgressBar::tick() {
    setCurrent(current + step);
}

void ProgressBar::setCurrent(double newCurrent) {
    if(newCurrent != current) {
        if(newCurrent  >= min && newCurrent <= max) {
            current = newCurrent;
        } else if(newCurrent > max) {
            current = max;
        } else if(newCurrent < min) {
            current = min;
        }
        if(threadRunning) {
            timeToUpdate.store(true);
        }
    }
}

void ProgressBar::finish() {
    assert(threadRunning);

    timeToStop.store(true);
    printingThread.join();

    threadRunning = false;
}

ProgressBar::~ProgressBar() {
    if(threadRunning) {
        finish();
    }
}


void ProgressBar::loop() {
    t0 = std::chrono::high_resolution_clock::now();

    while(true) {
        if(timeToUpdate.load()) {
            update();
            timeToUpdate.store(false);
        }
        if(timeToStop.load() || finished) {
            update();
            fprintf(stderr, "\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void ProgressBar::update() {
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

    std::chrono::time_point<std::chrono::high_resolution_clock> thisTime = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(thisTime - lastTime).count();
    double totalTimeSpentSeconds = std::chrono::duration<double>(thisTime - t0).count();
    double etaSeconds = totalTimeSpentSeconds / completedPercentage;
    double iterationsPerSecond = (current - previous) / dt;
    char spentTime[256];
    char estimatedTime[256];
    timeConvert(totalTimeSpentSeconds, spentTime);
    timeConvert(etaSeconds, estimatedTime);
    sprintf(str, " %u/%u [%s<%s, %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, estimatedTime, iterationsPerSecond);
    progressLine += str;

    //printf("%s", progressLine.c_str());
    //std::cout << progressLine;
    fprintf(stderr, "%s", progressLine.c_str());

    if(current >= max) {
        finished = true;
    }

    lastTime = std::chrono::high_resolution_clock::now();
    previous = current;
}


void ProgressBar::timeConvert(double total, char* str) {
    sprintf(str, "%.2lfs", total);
}
/*
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
        m_TimeConverter(timeSpentSeconds, spentTime);
        m_TimeConverter(etaSeconds, estimatedTime);
        sprintf(str, " %u/%u [%s<%s, %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, estimatedTime, iterationsPerSecond);
        progressLine += str;

        //printf("%s", progressLine.c_str());
        //std::cout << progressLine;
        fprintf(stderr, "%s", progressLine.c_str());

        if(current >= max) {
            finished = true;
        }
    }


private:
    void m_TimeConverter(double total, char* str) {
        sprintf(str, "%.2lfs", total);
    }*/
