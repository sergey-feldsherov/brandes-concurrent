#include <stdio.h>
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <string>
#include <stdlib.h>

#include "Graph.h"
#include "ProgressBar.h"
#include "utils.h"


void Graph::loadGraph() {
    printf("Reading graph from %s\n", args->inputFile.c_str());
    FILE *input = fopen(args->inputFile.c_str(), "r");
    if(input == NULL) {
        printf("Unable to open input file: %s.\n", args->inputFile.c_str());
        return;
    }

    std::unordered_set< vertex > tmp;
    unsigned int edgeCount = 0;
    unsigned long long t = currTimeNano();

    char* line;
    size_t len = 0;
    vertex v0, v1;
    while(getline(&line, &len, input) != -1) {
        if(line[0] == '#') {
            printf("Encountered a commentary line\n");
        } else {
            if(sscanf(line, "%u %u", &v0, &v1) == 2) {
                addEdge(v0, v1);
                tmp.insert(v0);
                tmp.insert(v1);
                edgeCount++;
            } else {
                printf("Invalid line: %s\n", line);
                abort();
            }
        }
    }
    if(line) {
        free(line);
    }

    fclose(input);
    t = currTimeNano() - t;

    vertices = std::vector< vertex >(tmp.begin(), tmp.end());

    printf("Done ( %.2lf seconds)\n", t * 1e-9);
    printf("Vertices: %'lu, edges: %'u\n", vertices.size(), edgeCount);
}


void Graph::computeBrandes() {
    printf("Running Brandes...\n");
    ProgressBar computationProgress(vertices.size(), "Progress:");
    auto t = currTimeNano();

    for(auto p: edges) {
        ranking[p.first] = 0;
    }

    unsigned int it = 0;
    for(auto s: vertices) {

        std::stack< vertex > S;
        assert(S.empty());

        std::unordered_map< vertex, std::list< vertex > > P;

        std::unordered_map< vertex, unsigned int > sigma;
        for(auto p1: edges) {
            sigma[p1.first] = 0;
        }
        sigma[s] = 1;

        std::unordered_map< vertex, int > d;
        for(auto p1: edges) {
            d[p1.first] = -1;
        }
        d[s] = 0;

        std::deque< vertex > Q;
        assert(Q.empty());
        Q.push_back(s);

        while(not Q.empty()) {
            vertex v = Q.front();
            Q.pop_front();
            S.push(v);
            for(auto w: edges[v]) {
                if(d[w] < 0) {
                    Q.push_back(w);
                    d[w] = d[v] + 1;
                }
                if(d[w] == d[v] + 1) {
                    sigma[w] += sigma[v];
                    P[w].push_back(v);
                }
            }
        }

        std::unordered_map< vertex, double > delta;
        for(auto p1: edges) {
            delta[p1.first] = 0;
        }
        while(not S.empty()) {
            vertex w = S.top();
            S.pop();
            for(auto v: P[w]) {
                delta[v] += (sigma[v] / sigma[w]) * (1. + delta[w]);
            }
            if( w != s ) {
                ranking[w] += delta[w];
            }
        }
        ++computationProgress;
        ++it;
        double time = (currTimeNano() - t) * 1e-9;
        int total = int(time);
        int seconds = total % 60;
        int minutes = total / 60;
        int hours = minutes / 60;
        minutes %= 60;
        double avg_speed = it / time;
        int etotal = (int) (vertices.size() / avg_speed);
        int eseconds = etotal % 60;
        int eminutes = etotal / 60;
        int ehours = eminutes / 60;
        eminutes %= 60;
        char stats[256];
        sprintf(stats, "time: %dh %dm %ds, eta: %dh %dm %ds", hours, minutes, seconds, ehours, eminutes, eseconds);
        computationProgress.updateLastPrintedMessage(std::string(stats));
    }

    t = currTimeNano() - t;
    computationProgress.endProgressBar();
    printf("Done (%.3lf seconds)\n", t * 1e-9);
}


void Graph::saveBrandesData() {
    ProgressBar saveProgress(ranking.size(), "Saving graph data to " + args->outputFile);

    FILE *output = fopen(args->outputFile.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", args->outputFile.c_str());
        return;
    }

    unsigned long long t = currTimeNano();
    for(auto p: ranking) {
        fprintf(output, "%u %f\n", p.first, p.second);
        ++saveProgress;
    }
    fclose(output);
    t = currTimeNano() - t;

    saveProgress.endProgressBar();

    printf("File written in %.2lf seconds.\n", t * 1e-9);
}





