#include <stdio.h>
#include <stack>
#include <deque>
#include <unordered_map>
#include <list>
#include <vector>

#include "Graph.h"
#include "ProgressBar.h"
#include "utils.h"


void Graph::loadGraph() {
    FILE *input = fopen(args->inputFile.c_str(), "r");
    if(input == NULL) {
        printf("Unable to open input file: %s.\n", args->inputFile.c_str());
        return;
    }

    unsigned int edgeCount = 0;
    unsigned long long t = currTimeNano();
    vertex v0, v1;
    while(fscanf(input, "%u %u", &v0, &v1) == 2) {
        addEdge(v0, v1);
        edgeCount++;
    }
    fclose(input);
    t = currTimeNano() - t;

    printf("Graph read in %.2lf seconds.\n", t * 1e-9);
    printf("Vertices: %lu, edges: %u\n", edges.size(), edgeCount);
}


void Graph::computeBrandes() {
    printf("Beginning Brandes\n");
    auto t = currTimeNano();

    for(auto p: edges) {
        ranking[p.first] = 0;
    }

    for(auto p: edges) {
        vertex s = p.first;

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
    }

    t = currTimeNano() - t;
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





