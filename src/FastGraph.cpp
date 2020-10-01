#include <cassert>
#include <stack>
#include <thread>
#include <unordered_set>

#include "FastGraph.h"
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

    unsigned long long t1 = currTimeNano();
    if(args->norenumeration) {//TODO: this case can be optimized
        printf("Constructing CSR without renumeration\n");

        indices.resize(vertices.size() + 1);
        csr.reserve(vertices.size());
        for(unsigned int i = 0; i < vertices.size(); i++) {
            vertices[i] = i;
            renumerationTable[i] = i;
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

    } else {
        printf("Renumerating graph\n");

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

    }

    printf("Done (%.4lf seconds)\n", (currTimeNano() - t1) * 1e-9);
    printf("Total loading time: %.4lf seconds\n", (currTimeNano() - t0) * 1e-9);

    if(args->finishID == 0) {
        args->finishID = vertices.size();
        printf("Upper interval border not specified, Brandes will be run from %u to %lu\n", args->startID, vertices.size());
    }

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
    assert(args->startID < args->finishID);
    printf("\nRunning serial Brandes for vertices in interval [%u, %u).\n", args->startID, args->finishID);

    auto t = currTimeNano();
    ProgressBar bar;
    bar.setMax(args->finishID - args->startID);
    bar.start();

    scores.resize(vertices.size(), 0.0);

    for(unsigned int s = args->startID; s < args->finishID; s++) {

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
    assert(args->startID < args->finishID);
    printf("\nRunning threaded Brandes for vertices in interval [%u, %u).\n", args->startID, args->finishID);
    unsigned long long t0 = currTimeNano();

    std::atomic<unsigned int> counter(args->startID);

    scores.resize(vertices.size(), 0.);
    threadScores.resize(args->thNum, std::vector< double >(vertices.size(), 0.));
    std::vector< std::thread > workers;
    unsigned int sID = args->startID;
    unsigned int fID = args->finishID;

    printf("Starting threads\n");
    for(unsigned int i = 0; i < (unsigned int) args->thNum; i++) {
        if(args->debug) {
            printf("\tStarting thread %d\n", i);
        }
        workers.push_back(std::thread([this, i, sID, fID, &counter] (){this->threadFunction(i, sID, fID, counter);}));
    }
    printf("Threads started\n");

    ProgressBar bar;
    bar.setMax(args->finishID - args->startID);
    bar.start();
    unsigned int cntrVal = counter.load();
    while(cntrVal < args->finishID) {
        cntrVal = counter.load();
        bar.setCurrent(cntrVal);
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


void FastGraph::threadFunction(unsigned int id, unsigned int startID, unsigned int endID, std::atomic< unsigned int >& counter) {

    while(true) {
        unsigned int s = counter.fetch_add(1);
        if(s >= vertices.size() || s >= endID) {
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
