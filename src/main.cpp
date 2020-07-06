#include <argp.h>
#include <stdlib.h> // atoi, atof
#include <locale.h> // setlocale to enable thousands delimeter in output

#include "Graph.h"
#include "utils.h"

globalArgs_t globalArgs;

static int parse_opt(int key, char *arg, struct argp_state *state);

int main(int argc, char **argv) {
    //setlocale(LC_ALL, "");

    //init globalArgs (fill default values)
    initArgs(&globalArgs);

    //create stuff for argp
    struct argp_option options[] = {
        { "debug",      'd',      0, 0, "Debugging information display" },
        { "undirected", 'u',      0, 0, "Treat graph as undirected" },
        { "input",      'i', "FILE", 0, "Input file" },
        { "output",     'o', "FILE", 0, "Output file" },
        { "threads",    't',  "NUM", 0, "Number of threads: non-positive -> serial, >= 1 -> concurrent with threads allocation" },
        { 0 }
    };
    struct argp argp = { options, parse_opt, 0, 0 };

    //parse args and put stuff into globalArgs
    argp_parse(&argp, argc, argv, 0, 0, 0);
    if(globalArgs.debug)
        printArgs(&globalArgs);

    Graph g(&globalArgs);
    g.loadGraph();
    if(globalArgs.thNum <= 0) {
        g.computeBrandes();
    } else {
        g.concurrentBrandes();
    }
    g.saveBrandesData();

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
    }
    return 0;
}




