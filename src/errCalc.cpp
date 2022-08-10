#include <cstdio>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <string>
#include <cmath> //fabs(), fmax(), sqrt()
#include <algorithm>
#include <argp.h>
#include <vector>
#include <time.h>
#include <unistd.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

void readTruthFile(std::unordered_map<unsigned int, double>&, std::unordered_set<unsigned int>&, std::string, std::string);
bool comp(std::pair<unsigned int, double>, std::pair<unsigned int, double>);
std::vector<unsigned int> topk(std::unordered_map<unsigned int, double>&, unsigned int);
std::vector<unsigned int> intersect(std::vector<unsigned int>&, std::vector<unsigned int>&);
void printSome(std::vector<unsigned int>&, std::unordered_map<unsigned int, double>&, int);

struct Args {
    std::string file1 = "./file1.txt";
    std::string file2 = "./file2.txt";
    std::string split1 = " ";
    std::string split2 = " ";
};

Args my_args;

int main(int argc, char** argv) {
    auto t0 = currTimeNano();

    struct argp_option options[] = {
        {  "file1", -1, "FILE", 0,    "Path to first file" },
        {  "file2", -2, "FILE", 0,   "Path to second file" },
        { "split1", -3,  "STR", 0,  "Split for first file" },
        { "split2", -4,  "STR", 0, "Split for second file" },
        { 0 }
    };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    std::unordered_map<unsigned int, double> data1;
    std::unordered_set<unsigned int> vertices1;
    readTruthFile(data1, vertices1, my_args.file1, my_args.split1);

    std::unordered_map<unsigned int, double> data2;
    std::unordered_set<unsigned int> vertices2;
    readTruthFile(data2, vertices2, my_args.file2, my_args.split2);

    //assert(data1.size() == data2.size());

    double sumErr = 0.;
    double sumSquareErr = 0.;
    double maxErr = 0.;
    double data1size = static_cast<double>(data1.size());
    double data2size = static_cast<double>(data2.size());
    //int cntPos = 0;
    //int cntNeg = 0;
    for(auto p: data1) {
        auto v1 = p.second /** (data1size-1.) * (data1size-2.)*/;
        auto v2 = data2[p.first] /*/ ((data2size-1)*(data2size-2))*/;
        auto err = v1 - v2;//fabs()!!!
        maxErr = fmax(fabs(err), fabs(maxErr));
        sumErr += fabs(err);
        sumSquareErr += err * err;
        //if(err > 1e-7) {
        //    cntPos++;
        //} else if(err < -1e-7) {
        //    cntNeg++;
        //}
    }
    //printf("cntPos: %d\ncntNeg: %d\n\n", cntPos, cntNeg);

    int printCounter = 0;
    int maxPrint = 0;
    std::string green = "\u001b[32m";
    std::string red = "\u001b[31m";
    std::string reset = "\u001b[0m";
    std::string col;
    while(printCounter < maxPrint) {
        double thisError = data1[printCounter] - data2[printCounter];
        if(fabs(thisError) > 1e-7) {
            col = red;
        } else {
            col = green;
        }
        printf("ID: %3d, \tscore1: %.5e, \tscore2: %.5e, \terror: %s%.3e%s\n", printCounter, data1[printCounter], data2[printCounter], col.c_str(), thisError, reset.c_str());
        printCounter++;
    }

    int topSize = 10000;
    printf("\n");
    printf("Calculating top-%d nodes\n", topSize);
    std::vector<unsigned int> data1topk = topk(data1, topSize);
    std::vector<unsigned int> data2topk = topk(data2, topSize);
    printf("Calculating intersection\n");
    std::vector<unsigned int> tkIntersection = intersect(data1topk, data2topk);
    printf("Precision: %.4lf (%lu nodes in intersection out of %d)\n", ((double) tkIntersection.size()) / (double) topSize, tkIntersection.size(), topSize);

    printf("\n");
    printSome(data1topk, data1, 7);
    printSome(data2topk, data2, 7);

    printf("\n");
    printf("L1:   %.15e (sum of absolute errors)\n", sumErr);
    printf("L2:   %.15e (Euclidean norm of error vector)\n", sqrt(sumSquareErr));
    printf("Linf: %.15e (max absolute error)\n", maxErr);
    printf("E:    %.15e (average error)\n", sumErr / fmax(data1.size(), data2.size()));
    printf("RMSE: %.15e (root mean square error)\n", sqrt(sumSquareErr / fmax(data1.size(), data2.size())));

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - t0)*1e-9);
    return 0;
}


void readTruthFile(std::unordered_map<unsigned int, double>& data, std::unordered_set<unsigned int>& vertices, std::string fileName, std::string split) {
    auto timeReadStart = currTimeNano();
    printf("Reading file \"%s\".\n", fileName.c_str());
    FILE* input = fopen(fileName.c_str(), "r");
    if(input == NULL) {
        printf("Error while opening file \"%s\", aborting\n", fileName.c_str());
        abort();
    }
    char* line = NULL;
    size_t len = 0;
    unsigned int vertex;
    double score;
    unsigned int lineCount = 0;
    while(getline(&line, &len, input) != -1) {
        std::string tmp = line;
        size_t splitPos = tmp.find(split);
        if(std::string::npos == splitPos) {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, fileName.c_str());
            abort();
        }
        vertex = std::stoi(tmp.substr(0, splitPos));
        score = std::stod(tmp.substr(splitPos + 1, tmp.length()));
        //printf("From string \"%s\", %u %lf\n", tmp.c_str(), vertex, score);
        data[vertex] = score;
        vertices.insert(vertex);
        lineCount++;
    }
    if(line) {
        free(line);
    }
    fclose(input);
    printf("Done (%.2lf s)\nVertices: %lu\nLines in file: %u\n\n", (currTimeNano() - timeReadStart)*1e-9, vertices.size(), lineCount);
}


static int parse_opt(int key, char *arg, struct argp_state *state) {
    if(key == -1) {
        my_args.file1 = arg;
    } else if(key == -2) {
        my_args.file2 = arg;
    } else if(key == -3) {
        my_args.split1 = arg;
    } else if(key == -4) {
        my_args.split2 = arg;
    }

    return 0;
}


unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}


bool comp(std::pair<unsigned int, double> p1, std::pair<unsigned int, double> p2) {
    return (p1.second > p2.second);
}


std::vector<unsigned int> topk(std::unordered_map<unsigned int, double>& data, unsigned int k) {
    std::vector<std::pair<unsigned int, double>> sortedVals(data.begin(), data.end());
    std::sort(sortedVals.begin(), sortedVals.end(), comp);
    if(sortedVals.size() > k) {
        sortedVals.erase(sortedVals.begin()+k, sortedVals.end());
    }

    std::vector<unsigned int> res;
    for(auto p: sortedVals) {
        res.push_back(p.first);
    }

    return res;
}


std::vector<unsigned int> intersect(std::vector<unsigned int>& v1, std::vector<unsigned int>& v2) {
    printf("Intersecting vectors of size %lu and %lu\n", v1.size(), v2.size());
    std::vector<unsigned int> res;
    std::unordered_set<unsigned int> tmp(v1.begin(), v1.end());

    for(auto a: v2) {
        if(tmp.count(a)) {
            res.push_back(a);
            tmp.erase(a);
        }
    }

    printf("Resulting in vector of size %lu\n", res.size());
    return res;
}

void printSome(std::vector<unsigned int>& data, std::unordered_map<unsigned int, double>& scores, int k) {
    assert(data.size() >= k);

    for(int i = 0; i < k-1; i++) {
        printf("%u(%lf), ", data[i], scores[data[i]]);
    }
    printf("%u(%lf)\n", data[k-1], scores[data[k-1]]);
}

