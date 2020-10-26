#include <cassert>
#include <stack>
#include <thread>
#include <unordered_set>
#include <chrono>
#include <cstdio>

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
    printf("Vertices: %lu, edges: %u\n", vertices.size(), edgeCount);

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

    fflush(stdout);
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
        //assert(S.empty());

        std::vector< std::vector< unsigned int > > P(vertices.size(), std::vector< unsigned int > ());

        std::vector< unsigned int > sigma(vertices.size(), 0);
        sigma[s] = 1;

        std::vector< int > d(vertices.size(), -1);
        d[s] = 0;

        std::deque< int > Q;
        //assert(Q.empty());
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
                if(d[neighbour] == (d[v] + 1)) {
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
                delta[v] += ((double) sigma[v] / (double) sigma[w]) * (1. + delta[w]);
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
    std::atomic<bool> shouldBeRunning(true);
    std::atomic<unsigned int> runningThreads(0);
    unsigned int fID = args->finishID;
    std::string saveFileName = "";

    printf("Starting threads\n");
    for(unsigned int i = 0; i < (unsigned int) args->thNum; i++) {
        if(args->debug) {
            printf("\tStarting thread %d\n", i);
        }
        workers.push_back(std::thread([this, i, fID, &counter, &shouldBeRunning, &runningThreads] (){this->threadFunction(i, fID, counter, shouldBeRunning, runningThreads);}));
    }
    printf("Threads started\n");
    fflush(stdout);

    std::chrono::time_point<std::chrono::high_resolution_clock> previousTime = std::chrono::high_resolution_clock::now();
    ProgressBar bar;
    bar.setMax(args->finishID - args->startID);
    bar.start();
    unsigned int cntrVal = counter.load();
    while(cntrVal < args->finishID) {
        cntrVal = counter.load();
        bar.setCurrent(cntrVal);

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(currentTime - previousTime).count();
        if(args->autosaveInterval > 0. && dt >= 60. * args->autosaveInterval) {
            //pause progress bar and threads
            bar.setMessage(std::string("Pausing threads"), true);
            shouldBeRunning.store(false);
            while(runningThreads.load() > 0) {}

            //reduce scores
            bar.setMessage(std::string("Reducing scores"), true);
            for(unsigned int i = 0; i < vertices.size(); i++) {
                for(unsigned int j = 0; j < (unsigned int) args->thNum; j++) {
                    scores[i] += threadScores[j][i];
                }
            }

            //remove older save file
            if(saveFileName != "") {
                if(remove(saveFileName.c_str()) == 0) {
                    bar.setMessage(std::string("Removed last save file at " + saveFileName), true);
                } else {
                    bar.setMessage(std::string("Error while removing last save file at " + saveFileName), true);
                }
            }

            //save
            saveFileName = args->saveFilePrefix + std::to_string(args->startID) + "-" + std::to_string(counter.load()) + ".ontheway";
            bar.setMessage(std::string("Saving to " + saveFileName), true);
            saveResult(saveFileName, true);

            //reset time, unpause
            bar.setMessage(std::string("Restarting threads"), true);
            previousTime = std::chrono::high_resolution_clock::now();
            shouldBeRunning.store(true);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bar.finish();

    for(auto& t: workers) {
        t.join();
    }
    printf("Threads joined\n");

    if(saveFileName != "") {
        if(remove(saveFileName.c_str()) == 0) {
            printf("Removed last save file at %s\n", saveFileName.c_str());
        } else {
            printf("Error while removing last save file at %s\n", saveFileName.c_str());
        }
    }

    printf("Reducing scores\n");
    for(unsigned int i = 0; i < vertices.size(); i++) {
        for(unsigned int j = 0; j < (unsigned int) args->thNum; j++) {
            scores[i] += threadScores[j][i];
        }
    }
    printf("Scores array reduced\n");

    unsigned long long t = currTimeNano() - t0;
    printf("Total work time: %.4lf seconds\n", t * 1e-9);
    fflush(stdout);
}


void FastGraph::threadedBrandes_noAutosave() {
    assert(args->startID < args->finishID);
    printf("\nRunning threaded Brandes for vertices in interval [%u, %u), no autosave.\n", args->startID, args->finishID);
    unsigned long long t0 = currTimeNano();

    scores.resize(vertices.size(), 0.);
    threadScores.resize(args->thNum, std::vector< double >(vertices.size(), 0.));
    std::vector<unsigned int> progressVector(args->thNum, 0);
    std::vector<std::thread> workers;

    int threadCount = args->thNum;
    assert(((double) vertices.size() / (double) threadCount) >= 1. );

    unsigned int fullThreads, rest, verticesPerThread;
    rest = vertices.size() % threadCount;
    verticesPerThread = vertices.size() / threadCount;
    if(rest != 0) {
        fullThreads = rest;
    } else {
        fullThreads = threadCount;
    }
    if(args->debug) {
        printf("full threads: %u\nvertices per thread: %u\n", fullThreads, verticesPerThread);
    }

    unsigned int a = 0, b = 0;
    printf("Starting threads\n");
    for(unsigned int i = 0; i < (unsigned int) args->thNum; i++) {
        a = b;
        if(i < fullThreads && rest != 0) {
            b += verticesPerThread + 1;
        } else {
            b += verticesPerThread;
        }

        if(args->debug) {
            printf("\tStarting thread %d, [%u, %u) (%u)\n", i, a, b, b-a);
        }
        workers.push_back(std::thread([this, i, a, b, &progressVector] (){this->threadFunction_noAutosave(i, a, b, progressVector);}));
    }
    printf("Threads started\n");
    fflush(stdout);

    ProgressBar bar;
    bar.setMax(args->finishID - args->startID);
    bar.start();
    unsigned int counter = 0;
    while(counter < (args->finishID - args->startID)) {
        counter = 0;
        for(auto p: progressVector) {
            counter += p;
        }
        bar.setCurrent(counter);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    fflush(stdout);
}


void FastGraph::threadFunction(unsigned int id, unsigned int endID, std::atomic< unsigned int >& counter, std::atomic<bool>& shouldBeRunning, std::atomic<unsigned int>& runningThreads) {
    runningThreads.fetch_add(1);

    while(true) {
        unsigned int s = counter.fetch_add(1);
        if(s >= vertices.size() || s >= endID) {
            break;
        }

        std::stack< int > S;
        //assert(S.empty());

        std::vector< std::vector< unsigned int > > P(vertices.size(), std::vector< unsigned int >());

        std::vector< unsigned int > sigma(vertices.size(), 0);
        sigma[s] = 1;

        std::vector< int > d(vertices.size(), -1);
        d[s] = 0;

        std::deque< int > Q;
        //assert(Q.empty());
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
                if(d[neighbour] == (d[v] + 1)) {
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
                delta[v] += ((double) sigma[v] / (double) sigma[w]) * (1. + delta[w]);
            }
            if( w != s ) {
                threadScores[id][w] += delta[w];
            }
        }

        if(not shouldBeRunning.load()) {
            runningThreads.fetch_sub(1);
            while(not shouldBeRunning.load()) {}
            runningThreads.fetch_add(1);
        }

    }

    runningThreads.fetch_sub(1);
}


void FastGraph::threadFunction_noAutosave(unsigned int id, unsigned int m_startID, unsigned int m_finishID, std::vector<unsigned int>& progressVector) {
    for(unsigned int s = m_startID; s < m_finishID; s++) {
        std::stack< int > S;
        //assert(S.empty());

        std::vector< std::vector< unsigned int > > P(vertices.size(), std::vector< unsigned int >());

        std::vector< unsigned int > sigma(vertices.size(), 0);
        sigma[s] = 1;

        std::vector< int > d(vertices.size(), -1);
        d[s] = 0;

        std::deque< int > Q;
        //assert(Q.empty());
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
                if(d[neighbour] == (d[v] + 1)) {
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
                delta[v] += ((double) sigma[v] / (double) sigma[w]) * (1. + delta[w]);
            }
            if( w != s ) {
                threadScores[id][w] += delta[w];
            }
        }
        progressVector[id] += 1;

    }
}


void FastGraph::saveResult(std::string str, bool noPrinting) {
    if(str == "") {
        str = args->outputDir + "/result.txt";
    }

    if(not noPrinting) {
        printf("\nSaving data to %s\n", str.c_str());
    }

    FILE *output = fopen(str.c_str(), "w");
    if(output == NULL) {
        if(not noPrinting) {
            printf("Unable to open output file: %s.\n", str.c_str());
        }
        return;
    }

    unsigned long long t = currTimeNano();
    for(unsigned int i = 0; i < vertices.size(); i++) {
        fprintf(output, "%u %f\n", vertices[i], scores[i]);
    }
    fclose(output);

    t = currTimeNano() - t;
    if(not noPrinting) {
        printf("Done (%.5lf seconds)\n", t * 1e-9);
    }
    fflush(stdout);
}
