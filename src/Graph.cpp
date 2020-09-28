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
#include <atomic>
#include <chrono>

#include "Graph.h"
#include "utils.h"
#include "ProgressBar.h"


void FastGraph::loadGraph() {
    auto t0 = currTimeNano();
    printf("Reading graph from %s\n", args->inputFile.c_str());

    FILE *input = fopen(args->inputFile.c_str(), "r");
    if(input == NULL) {
        printf("\tUnable to open input file: %s.\n", args->inputFile.c_str());
        abort();
    }

    std::unordered_set< vertex > allVerticesSet;
    std::unordered_map< vertex, std::unordered_set< vertex > > allEdgesMap;
    unsigned int edgeCount = 0;
    unsigned long long t = currTimeNano();

    char* line = NULL;
    size_t len = 0;
    vertex v0, v1;
    while(getline(&line, &len, input) != -1) {
        if(line[0] == '#') {
            if(args->debug) {
                printf("\tEncountered a commentary line\n");
            }
        } else {
            if(sscanf(line, "%u %u", &v0, &v1) == 2) {
                allEdgesMap[v0].insert(v1);
                if(not args->directed) {
                    allEdgesMap[v1].insert(v0);
                }
                allVerticesSet.insert(v0);
                allVerticesSet.insert(v1);
                edgeCount++;
            } else {
                printf("\tInvalid line: %s\n", line);
                abort();
            }
        }
    }
    if(line) {
        free(line);
    }
    fclose(input);

    vertices = std::vector< vertex >(allVerticesSet.begin(), allVerticesSet.end());

    t = currTimeNano() - t;
    printf("Done (%.4lf seconds)\n", t * 1e-9);
    printf("Vertices: %'lu, edges: %'u\n", vertices.size(), edgeCount);

    printf("Renumerating graph\n");
    unsigned long long t1 = currTimeNano();

    indices.resize(vertices.size() + 1);
    csr.reserve(vertices.size());
    for(unsigned int idx = 0; idx < vertices.size(); idx++) {
        renumerationTable[vertices[idx]] = idx;
    }
    int currPosition = 0;
    for(unsigned int idx = 0; idx < vertices.size(); idx++) {
        indices[idx] = currPosition;
	for(auto vrtx: allEdgesMap[vertices[idx]]) {
            csr.push_back(renumerationTable[vrtx]);
        }
        currPosition += allEdgesMap[vertices[idx]].size();
    }
    indices[vertices.size()] = csr.size();

    printf("Done (%.4lf seconds)\n", (currTimeNano() - t1) * 1e-9);
    printf("Total loading time: %.4lf seconds\n", (currTimeNano() - t0) * 1e-9);

    /*
    printf("Forward renumeration:\n");
    for(auto p: renumerationTable) {
        printf("%d => %d\n", p.first, p.second);
    }

    printf("Edges for source graph:\n");
    for(auto p: allEdgesMap) {
        printf("%d - [", p.first);
        for(auto trgt: p.second) {
            printf("%d, ", trgt);
        }
        printf("]\n");
    }

    printf("Edges for renumerated graph(source enumeration - to double check):\n");
    for(int idx = 0; idx < vertices.size(); idx++) {
        int start = indices[idx];
        int end = indices[idx + 1];
        printf("%d -> [", vertices[idx]);
        for(int subIdx = start; subIdx < end; subIdx++) {
            printf("%d, ", vertices[csr[subIdx]]);
        }
        printf("]\n");
    }

    printf("Edges for renumerated graph:\n");
    for(int idx = 0; idx < vertices.size(); idx++) {
        int start = indices[idx];
        int end = indices[idx + 1];
        printf("%d -> [", idx);
        for(int subIdx = start; subIdx < end; subIdx++) {
            printf("%d, ", csr[subIdx]);
        }
        printf("]\n");
    }
    */
    
}

void FastGraph::serialBrandes() {
    printf("\nRunning serial Brandes\n");
    auto t = currTimeNano();
    ProgressBar bar;
    bar.setMax(vertices.size() - 1);
    bar.start();

    scores.resize(vertices.size(), 0.0);

    for(unsigned int s = 0; s < vertices.size(); s++) {

        std::stack< int > S;
        assert(S.empty());

        std::vector< std::vector< int > > P(vertices.size());

        std::vector< unsigned int > sigma(vertices.size(), 0);
        sigma[s] = 1;

        std::vector< int > d(vertices.size(), -1);
        d[s] = 0;

        std::deque< int > Q;
        assert(Q.empty());
        Q.push_back(s);

        while(not Q.empty()) {
            int v = Q.front();
            Q.pop_front();
            S.push(v);
            int st = indices[v];
            int fn = indices[v + 1];
            for(int w = st; w < fn; w++) {
                int neighbour = csr[w];
                if(d[neighbour] < 0) {
                    Q.push_back(neighbour);
                    d[neighbour] = d[v] + 1;
                }
                if(d[neighbour] == d[v] + 1) {
                    sigma[neighbour] += sigma[v];
                    P[neighbour].push_back(v);
                }
            }
        }

        std::vector< double > delta(vertices.size(), 0.);
        while(not S.empty()) {
            vertex w = S.top();
            S.pop();
            for(auto v: P[w]) {
                delta[v] += (sigma[v] / sigma[w]) * (1. + delta[w]);
            }
            if( w != s ) {
                scores[w] += delta[w];
            }
        }

        bar.tick();
    }
    bar.finish();

    t = currTimeNano() - t;
    printf("Done (%.4lf seconds)\n", t * 1e-9);
}


void FastGraph::threadedBrandes() {
    printf("\nRunning threaded Brandes\n");
    unsigned long long t0 = currTimeNano();

    std::atomic<unsigned int> counter(0);

    scores.resize(vertices.size(), 0.);
    threadScores.resize(args->thNum, std::vector< double >(vertices.size(), 0.));
    std::vector< std::thread > workers;

    printf("Starting threads\n");
    for(unsigned int i = 0; i < (unsigned int) args->thNum; i++) {
        if(args->debug) {
            printf("\tStarting thread %d\n", i);
        }
        workers.push_back(std::thread([this, i, &counter] (){this->threadFunction(i, counter);}));
    }
    printf("Threads started\n");

    ProgressBar bar;
    bar.setMax(vertices.size() - 1);
    bar.start();
    while(not bar.isFinished()) {
        bar.setCurrent(counter.load());
        bar.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bar.finish();

    for(auto& t: workers) {
        t.join();
    }
    printf("Threads joined\n");

    printf("Reducing scores\n");
    for(unsigned int i = 0; i < vertices.size(); i++) {
        for(unsigned int j = 0; j < (unsigned int) args->thNum; j++) {
            scores[i] += threadScores[j][i];
        }
    }
    printf("Scores array reduced\n");

    unsigned long long t = currTimeNano() - t0;
    printf("Total work time: %.4lf seconds\n", t * 1e-9);
}


void FastGraph::threadFunction(unsigned int id, std::atomic< unsigned int >& counter) {

    while(true) {
        unsigned int s = counter.fetch_add(1);
        if(s >= vertices.size()) {
            break;
        }

        std::stack< int > S;
        assert(S.empty());

        std::vector< std::vector< int > > P(vertices.size(), std::vector< int >());

        std::vector< unsigned int > sigma(vertices.size(), 0);
        sigma[s] = 1;

        std::vector< int > d(vertices.size(), -1);
        d[s] = 0;

        std::deque< int > Q;
        assert(Q.empty());
        Q.push_back(s);

        while(not Q.empty()) {
            int v = Q.front();
            Q.pop_front();
            S.push(v);
            int st = indices[v];
            int fn = indices[v + 1];
            for(int w = st; w < fn; w++) {
                int neighbour = csr[w];
                if(d[neighbour] < 0) {
                    Q.push_back(neighbour);
                    d[neighbour] = d[v] + 1;
                }
                if(d[neighbour] == d[v] + 1) {
                    sigma[neighbour] += sigma[v];
                    P[neighbour].push_back(v);
                }
            }
        }

        std::vector< double > delta(vertices.size(), 0.);
        while(not S.empty()) {
            vertex w = S.top();
            S.pop();
            for(auto v: P[w]) {
                delta[v] += (sigma[v] / sigma[w]) * (1. + delta[w]);
            }
            if( w != s ) {
                threadScores[id][w] += delta[w];
            }
        }

    }
}


void FastGraph::saveResult() {
    printf("\nSaving data to %s\n", args->outputFile.c_str());

    FILE *output = fopen(args->outputFile.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", args->outputFile.c_str());
        return;
    }

    unsigned long long t = currTimeNano();
    for(unsigned int i = 0; i < vertices.size(); i++) {
        fprintf(output, "%u %f\n", vertices[i], scores[i]);
    }
    fclose(output);

    t = currTimeNano() - t;
    printf("Done (%.5lf seconds)\n", t * 1e-9);
}


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
            auto it = edges.find(v);
            if(it == edges.end()) {
                continue;
            }
            for(auto w: it->second) {
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
    printf("\nRunning serial Brandes\n");
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
    printf("\nSaving data to %s\n", args->outputFile.c_str());

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
    printf("Done (%.5lf seconds)\n", t * 1e-9);
}



