#include <argp.h>
#include <stdlib.h> // atoi, atof

#include "FastGraph.h"
#include "utils.h"
#include "timing.h"


globalArgs_t globalArgs;

static int parse_opt(int key, char *arg, struct argp_state *);

int main(int argc, char **argv) {
	unsigned long long t0 = currTimeNano();

    //init globalArgs (fill default values)
    initArgs(&globalArgs);

    //create stuff for argp
    struct argp_option options[] = {
        {      "debug", 'd',      0, 0,                                                         "Debugging information display", 0 },
        { "undirected", 'u',      0, 0,                                                             "Treat graph as undirected", 0 },
        {      "input", 'i', "FILE", 0,                                                                            "Input file", 0 },
        {     "output", 'o',  "DIR", 0,                                                                      "Output directory", 0 },
        {    "threads", 't',  "NUM", 0,           "Number of threads: <=0 - serial, >= 1 -> concurrent with threads allocation", 0 },
        {      "start", 's',   "ID", 0,                      "Starting vertex id for calculating contributions, first included", 0 },
        {     "finish", 'f',   "ID", 0,                  "Finishing vertex id for calculating contributions, last not included", 0 },
        {   "autosave", 'a',  "MIN", 0,                 "Time interval in minutes between part-way saves of calculation result", 0 },
        {    "norenum", 'n',      0, 0, "Do not renumerate graph vertices (graph vertices are numbered 0..N-1 and are present)", 0 },
        {            0,   0,      0, 0,                                                                                       0, 0 }
    };
    struct argp argp = { options, parse_opt, 0, 0, 0, 0, 0 };

    //parse args and put stuff into globalArgs
    argp_parse(&argp, argc, argv, 0, 0, 0);
    if(globalArgs.outputDir.back() != '/') {
        globalArgs.outputDir.append("/");
    }
    globalArgs.saveFilePrefix = globalArgs.inputFile;
    size_t last_bslash_idx = globalArgs.saveFilePrefix.find_last_of("/");
    if(std::string::npos != last_bslash_idx) {
        globalArgs.saveFilePrefix.erase(0, last_bslash_idx + 1);
    }
    globalArgs.saveFilePrefix = globalArgs.outputDir + globalArgs.saveFilePrefix;
    if(globalArgs.debug)
        printArgs(&globalArgs);

    FastGraph fg(&globalArgs);
    fg.loadGraph();
    if(globalArgs.thNum <= 0) {
        fg.serialBrandes();
    } else {
        //fg.threadedBrandes_noAutosave();
        fg.threadedBrandes();
    }

    std::string suffix = "." + std::to_string(globalArgs.startID) + "-" + std::to_string(globalArgs.finishID) + ".final";
    fg.saveResult(globalArgs.saveFilePrefix + suffix);

    unsigned long long t1 = currTimeNano();
    printf("Total end-to-end time: %lf s\n", (t1 - t0) * 1e-9);

    return 0;
}

static int parse_opt(int key, char *arg, struct argp_state *) {
    if(key == 'd') {
        globalArgs.debug = true;
    } else if(key == 'u') {
        globalArgs.directed = false;
    } else if(key == 'i') {
        globalArgs.inputFile = arg;
    } else if(key == 'o') {
        globalArgs.outputDir = arg;
    } else if(key == 't') {
        globalArgs.thNum = atoi(arg);
    } else if(key == 's') {
        globalArgs.startID = atoi(arg);
    } else if(key == 'f') {
        globalArgs.finishID = atoi(arg);
    } else if(key == 'a') {
        globalArgs.autosaveInterval = atoi(arg);
    } else if(key == 'n') {
        globalArgs.norenumeration = true;
    }

    return 0;
}

