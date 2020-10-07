#include <thread>
#include <cassert>
#include <cmath> //for floor()

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
    if(newCurrent > current || newCurrent < current) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
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
    double iterationsPerSecond = (finished)?((current - min) / totalTimeSpentSeconds) : ((current - previous) / dt);
    //double etaSeconds = totalTimeSpentSeconds / completedPercentage;
    double etaSeconds = totalTimeSpentSeconds + (max - current) / iterationsPerSecond;
    char spentTime[256];
    char estimatedTime[256];
    timeConvert(totalTimeSpentSeconds, spentTime);
    timeConvert(etaSeconds, estimatedTime);
    sprintf(str, " %u/%u [%s<%s, %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, estimatedTime, iterationsPerSecond);
    progressLine += str;

    progressLine += "\033[K";//This control sequence erases part of current line from cursor position until the end of the line, \e is the GNU shortcut for \033.
    fprintf(stderr, "%s", progressLine.c_str());//Printing to stderr to evade cursor blinking and potential mess if IO is redirected to file.

    if(current >= max) {
        finished = true;
    }

    lastTime = std::chrono::high_resolution_clock::now();
    previous = current;
}


void ProgressBar::timeConvert(double totalSeconds, char* str) {
    double seconds = 0;
    int minutes = 0, hours = 0, days = 0;
    double rest = totalSeconds;

    days = floor(rest) / (86400);
    rest -= days * 86400;

    hours = floor(rest) / (3600);
    rest -= hours * 3600;

    minutes = floor(rest) / 60;
    rest -= minutes * 60;

    seconds = rest;

    if(days > 0) {
        sprintf(str, "%02dd%02dh%02dm%02ds", days, hours, minutes, (int) seconds);
    } else if(hours > 0) {
        sprintf(str, "%02dh%02dm%02ds", hours, minutes, (int) seconds);
    } else if(minutes > 0) {
        sprintf(str, "%02dm%02ds", minutes, (int) seconds);
    } else {
        sprintf(str, "%05.2lfs", seconds);
    }

}




