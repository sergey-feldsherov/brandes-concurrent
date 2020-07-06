#include <iostream>
#include <stdio.h>
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <string>
#include <stdlib.h>
#include <thread>
#include <assert.h>

#include "Graph.h"
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
                if(not args->directed) {
                    addEdge(v1, v0);
                }
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

    vertices = std::vector< vertex >(tmp.begin(), tmp.end());

    t = currTimeNano() - t;
    printf("Done (%.2lf seconds)\n", t * 1e-9);
    printf("Vertices: %'lu, edges: %'u\n", vertices.size(), edgeCount);
}


void Graph::threadFuncBrandes(unsigned int id, unsigned int begin, unsigned int end) {
    for(auto v: vertices) {
        concurrentRanking[id][v] = 0;
    }

    for(auto i = begin; i < end; i++) {
        auto s = vertices[i];

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
                concurrentRanking[id][w] += delta[w];
            }
        }

    }
}


void Graph::concurrentBrandes() {
    for(auto v: vertices) {
        ranking[v] = 0.;
    }

    int threadCount = args->thNum;
    assert(threadCount > 0);
    assert((int) (vertices.size() / threadCount) >= 1 );

    unsigned int fullThreads, rest, verticesPerThread;
    rest = vertices.size() % threadCount;
    verticesPerThread = vertices.size() / threadCount;
    if(rest != 0) {
        fullThreads = rest;
    } else {
        fullThreads = threadCount;
    }

    printf("Starting threads\n");
    unsigned int a = 0, b = 0;
    concurrentRanking.resize(threadCount);
    auto t = currTimeNano();
    for(unsigned int i = 0; i < (unsigned int) threadCount; i++) {
        a = b;
        if(i < fullThreads) {
            b += verticesPerThread + 1;
        } else {
            b += verticesPerThread;
        }

        workers.push_back(std::thread([this, i, a, b] (){this->threadFuncBrandes(i, a, b);}));
    }
    printf("Threads started, waiting for join\n");

    for(auto& t: workers) {
        t.join();
    }
    printf("Threads joined\n");

    for(auto v: vertices) {
        for(unsigned int i = 0; i < (unsigned int) threadCount; i++) {
            ranking[v] += concurrentRanking[i][v];
        }
    }
    printf("Rankings reduced\n");

    t = currTimeNano() - t;
    printf("Total computation time: %.3lf seconds\n", t * 1e-9);
}


void Graph::computeBrandes() {
    printf("Running serial Brandes\n");
    auto t = currTimeNano();

    for(auto p: edges) {
        ranking[p.first] = 0;
    }

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
    }

    t = currTimeNano() - t;
    printf("Done (%.3lf seconds)\n", t * 1e-9);
}


void Graph::saveBrandesData() {
    printf("Saving data to %s\n", args->outputFile.c_str());

    FILE *output = fopen(args->outputFile.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", args->outputFile.c_str());
        return;
    }

    unsigned long long t = currTimeNano();
    for(auto p: ranking) {
        fprintf(output, "%u %f\n", p.first, p.second);
    }
    fclose(output);

    t = currTimeNano() - t;
    printf("File written in %.2lf seconds.\n", t * 1e-9);
}



