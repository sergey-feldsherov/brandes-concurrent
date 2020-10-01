#include <string>
#include <atomic>
#include <thread>
#include <chrono>


class ProgressBar {
public:
    ProgressBar();
    void setMin(double);
    void setMax(double);
    void start();
    void tick();
    void setCurrent(double);
    void finish();
    ~ProgressBar();

private:
    bool threadRunning = false;
    std::thread printingThread;

    std::atomic<bool> timeToUpdate;
    std::atomic<bool> timeToStop;

    char begin = '|';
    char fill = '=';
    char lead = '>';
    char rest = '.';
    char end = '|';

    double min = 0.;
    double max = 100.;
    double current = 0.;
    double previous = 0.;
    double step = 1.0;

    unsigned int width = 35;
    std::chrono::time_point<std::chrono::high_resolution_clock> t0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    bool finished;

    void update();
    void loop();
    void timeConvert(double, char*);
};
