#include <argp.h>
#include <stdlib.h> // atoi, atof
#include <locale.h> // setlocale to enable thousands delimeter in output

#include "Graph.h"
#include "FastGraph.h"
#include "utils.h"

globalArgs_t globalArgs;

static int parse_opt(int key, char *arg, struct argp_state *state);

int main(int argc, char **argv) {
    //setlocale(LC_ALL, "");

    //init globalArgs (fill default values)
    initArgs(&globalArgs);

    //create stuff for argp
    struct argp_option options[] = {
        { "debug",      'd',      0, 0,                                                         "Debugging information display" },
        { "undirected", 'u',      0, 0,                                                             "Treat graph as undirected" },
        { "input",      'i', "FILE", 0,                                                                            "Input file" },
        { "output",     'o', "FILE", 0,                                                                           "Output file" },
        { "threads",    't',  "NUM", 0,           "Number of threads: <=0 - serial, >= 1 -> concurrent with threads allocation" },
        { "start",      's',   "ID", 0,                      "Starting vertex id for calculating contributions, first included" },
        { "finish",     'f',   "ID", 0,                  "Finishing vertex id for calculating contributions, last not included" },
        { "autosave",   'a',  "MIN", 0,                 "Time interval in minutes between part-way saves of calculation result" },
        { "norenum",    'n',      0, 0, "Do not renumerate graph vertices (graph vertices are numbered 0..N-1 and are present)" },
        { 0 }
    };
    struct argp argp = { options, parse_opt, 0, 0 };

    //parse args and put stuff into globalArgs
    argp_parse(&argp, argc, argv, 0, 0, 0);
    if(globalArgs.debug)
        printArgs(&globalArgs);

    FastGraph fg(&globalArgs);
    fg.loadGraph();
    if(globalArgs.thNum <= 0) {
        fg.serialBrandes();
    } else {
        fg.threadedBrandes();
    }

    std::string name = "output/" + std::to_string(globalArgs.startID) + "-" + std::to_string(globalArgs.finishID) + ".final" + ".txt";
    fg.saveResult(name);

    /*
    Graph g(&globalArgs);
    g.loadGraph();
    if(globalArgs.thNum <= 0) {
        g.computeBrandes();
    } else {
        g.concurrentBrandes();
    }
    g.saveBrandesData();
    */

    return 0;
}

static int parse_opt(int key, char *arg, struct argp_state *state) {
    if(key == 'd') {
        globalArgs.debug = true;
    } else if(key == 'u') {
        globalArgs.directed = false;
    } else if(key == 'i') {
        globalArgs.inputFile = arg;
    } else if(key == 'o') {
        globalArgs.outputFile = arg;
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




