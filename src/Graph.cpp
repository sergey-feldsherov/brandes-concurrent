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
#include <future>
#include <assert.h>

#include "Graph.h"
#include "utils.h"
#include <indicators/indeterminate_progress_bar.hpp>
#include <indicators/block_progress_bar.hpp>
#include <indicators/dynamic_progress.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/termcolor.hpp>


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
    indicators::IndeterminateProgressBar bar{
        indicators::option::BarWidth{30},
        indicators::option::Start{"["},
        indicators::option::Fill{"."},
        indicators::option::Lead{"<>"},
        indicators::option::End{"]"},
        indicators::option::PostfixText{"Reading..."},
        indicators::option::ForegroundColor{indicators::Color::yellow},
        indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
    };
    indicators::show_console_cursor(false);

    char* line;
    size_t len = 0;
    vertex v0, v1;
    auto t_tmp = currTimeNano();
    while(getline(&line, &len, input) != -1) {
        if(line[0] == '#') {
            printf("Encountered a commentary line\n");
        } else {
            if(sscanf(line, "%u %u", &v0, &v1) == 2) {
                addEdge(v0, v1);
                tmp.insert(v0);
                tmp.insert(v1);
                edgeCount++;
                if((currTimeNano() - t_tmp) * 1e-9 > 0.05) {
                    bar.tick();
                    t_tmp = currTimeNano();
                }
            } else {
                printf("Invalid line: %s\n", line);
                abort();
            }
        }
    }
    if(line) {
        free(line);
    }
    bar.mark_as_completed();
    indicators::show_console_cursor(true);

    fclose(input);
    t = currTimeNano() - t;

    vertices = std::vector< vertex >(tmp.begin(), tmp.end());

    printf("Done ( %.2lf seconds)\n", t * 1e-9);
    printf("Vertices: %'lu, edges: %'u\n", vertices.size(), edgeCount);
}


void Graph::threadFuncBrandes(unsigned int id, unsigned int begin, unsigned int end, size_t barID) {
    //TODO: run Brandes for vertices [begin, ..., end), put result into thread's map in vector of rankings
    printf("Thread %u: barID = %lu\n", id, barID);  
    for(auto i = begin; i < end; i++) {
        concurrentRanking[id][vertices[i]] = 0;
    }

    bars[barID].set_option(indicators::option::PostfixText{"Thread " + std::to_string(id) + ": calculation"});
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

        bars[barID].tick();
    }

    bars[barID].mark_as_completed();
}


void Graph::concurrentBrandes() {
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

    printf("Starting threads...\n");
    indicators::show_console_cursor(false);
    bars.set_option(indicators::option::HideBarWhenComplete{false});
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

        indicators::BlockProgressBar bar{
            indicators::option::BarWidth{60},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::MaxProgress{b - a},
            indicators::option::PostfixText{"Thread " + std::to_string(i) + ": initialization"}
        };
        if(i % 8 == 0) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::grey});
        } else if(i % 8 == 1) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::red});
        } else if(i % 8 == 2) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::green});
        } else if(i % 8 == 3) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::yellow});
        } else if(i % 8 == 4) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::blue});
        } else if(i % 8 == 5) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::magenta});
        } else if(i % 8 == 6) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::cyan});
        } else if(i % 8 == 7) {
            bar.set_option(indicators::option::ForegroundColor{indicators::Color::white});
        }
        auto barID = bars.push_back(bar);
        workers.push_back(std::thread([this, i, a, b, barID] (){this->threadFuncBrandes(i, a, b, barID);}));
    }

    for(auto& t: workers) {
        t.join();
    }

    t = currTimeNano() - t;
    std::cout << termcolor::bold << termcolor::green << "Threads joined, user time: " << t * 1e-9 << std::endl;
    std::cout << termcolor::reset;
    indicators::show_console_cursor(true);

    printf("\n");
}


void Graph::computeBrandes() {
    printf("Running Brandes...\n");
    indicators::show_console_cursor(false);
    indicators::BlockProgressBar bar{
            indicators::option::BarWidth{60},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::ForegroundColor{indicators::Color::yellow},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::MaxProgress{vertices.size()},
            indicators::option::PrefixText{"Progress:"}
    };
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
        bar.tick();
    }

    t = currTimeNano() - t;
    bar.mark_as_completed();
    indicators::show_console_cursor(false);
    printf("Done (%.3lf seconds)\n", t * 1e-9);
}


void Graph::saveBrandesData() {
    printf("Saving data to %s\n", args->outputFile.c_str());
    indicators::show_console_cursor(false);
    indicators::BlockProgressBar bar{
            indicators::option::BarWidth{60},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::ForegroundColor{indicators::Color::yellow},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::MaxProgress{ranking.size()},
            indicators::option::PrefixText{"Save progress:"}
    };

    FILE *output = fopen(args->outputFile.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", args->outputFile.c_str());
        return;
    }

    unsigned long long t = currTimeNano();
    for(auto p: ranking) {
        fprintf(output, "%u %f\n", p.first, p.second);
        bar.tick();
    }
    fclose(output);
    t = currTimeNano() - t;

    bar.mark_as_completed();
    indicators::show_console_cursor(true);

    printf("File written in %.2lf seconds.\n", t * 1e-9);
}


void Graph::saveConcurrentBrandesData() {
    ProgressBar saveProgress(concurrentRanking.size(), "Saving graph data to " + args->outputFile);

    FILE *output = fopen(args->outputFile.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", args->outputFile.c_str());
        return;
    }

    unsigned long long t = currTimeNano();
    for(auto m: concurrentRanking) {
        for(auto p: m) {
            fprintf(output, "%u %f\n", p.first, p.second);
        }
        ++saveProgress;
    }
    fclose(output);
    t = currTimeNano() - t;

    saveProgress.endProgressBar();

    printf("File written in %.2lf seconds.\n", t * 1e-9);
}



