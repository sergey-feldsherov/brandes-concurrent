#include <cassert>
#include <cmath> //for floor() and max()
#include <cstdio>

#include "ProgressBar.h"

unsigned long long threadCurrTimeNano() {
  struct timespec t;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
  return t.tv_sec*1000000000 + t.tv_nsec;
}

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

void ProgressBar::setMessage(std::string msg, bool forceRedraw) {
    message = msg;
    if(forceRedraw) {
        forceUpdate();
    }
}

void ProgressBar::start() {
    assert(not threadRunning);

    t0 = std::chrono::high_resolution_clock::now();
    timeToStop.store(false);
    timeToUpdate.store(true);
    finished = false;
    fprintf(stdout, "\n");

    printingThread = std::thread([this] (){this->loop();});

    threadRunning = true;
}

void ProgressBar::forceUpdate() {
    timeToUpdate.store(true);
    //std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
            forceUpdate();
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
    auto t_0 = threadCurrTimeNano();

    while(true) {
        if(timeToUpdate.load() || ((double)(threadCurrTimeNano() - t_0))*1e-7 >= updateIntervalMilliseconds) {
            timeToUpdate.store(false);
            update();
            t_0 = threadCurrTimeNano();
        }
        if(timeToStop.load() || finished) {
            statusMessage = "finished";
            update();
            fprintf(stdout, "\n");
            break;
        }
    }
}


void ProgressBar::update() {
    /* It looks like this:
    Some previous message
    Some latest message
    [Status: running]
    27%|==>       | 493/1821 [01m38s<06m11s, 5.03it/s]
    */
    std::string progressLine = "\033[1F";//Move the cursor to the beginning of previous line

    if(message != "") {
        progressLine += message + "\033[K\n";
        message = "";
    }

    progressLine += "[Status: ";
    progressLine += statusMessage + "]";
    progressLine += "\033[K\n";//Erase remainder of line and move cursor to the beginning of next line

    double completedPercentage = (current - min) / (max - min);
    char str[512];
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
    double iterationsPerSecond = 0.;
    if(finished) {
        iterationsPerSecond = (current - min) / totalTimeSpentSeconds;
    } else {
        iterationsPerSecond = (current - previous) / dt;
    }
    double etaSeconds = 0.;
    if(iterationsPerSecond > 0.) {
        etaSeconds = totalTimeSpentSeconds + (max - current) / iterationsPerSecond;
    } else {
        etaSeconds = totalTimeSpentSeconds / ((completedPercentage > 0.) ? completedPercentage : 1.);
    }
    double filteredEta = 0.;
    etaFilter.push_back(etaSeconds);
    if(etaFilter.size() > filterSize) {
        etaFilter.pop_front();
        for(double x: etaFilter) {
            filteredEta += x;
        }
        filteredEta = filteredEta / ((double) etaFilter.size());
    }
    //printf("it/s = %lf, total = %lf, comp = %lf%%\n\n\n", iterationsPerSecond, totalTimeSpentSeconds, completedPercentage);

    char spentTime[128];
    char estimatedTime[128];
    timeConvert(totalTimeSpentSeconds, spentTime);
    timeConvert(filteredEta, estimatedTime);

    if(finished) {
        sprintf(str, " %u/%u [in %s, avg %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, iterationsPerSecond);
    } else {
        sprintf(str, " %u/%u [%s<%s, %.2fit/s]", (unsigned int)(current-min), (unsigned int)(max-min), spentTime, estimatedTime, iterationsPerSecond);
    }
    progressLine += str;

    progressLine += "\033[K";//This control sequence erases part of current line from cursor position until the end of the line, \e is the GNU shortcut for \033.
    //progressLine += "\n";
    fprintf(stdout, "%s", progressLine.c_str());//Printing to stderr to evade cursor blinking and potential mess if IO is redirected to file.
    fflush(stdout);

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




