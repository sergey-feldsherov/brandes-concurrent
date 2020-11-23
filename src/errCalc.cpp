#include <cstdio>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <string>
#include <cmath> //fabs(), fmax(), sqrt()
#include <argp.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

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

    auto timeRead1Start = currTimeNano();
	printf("Reading file \"%s\".\n", my_args.file1.c_str());
	std::unordered_map<unsigned int, double> data1;
    std::unordered_set<unsigned int> vertices1;
    FILE* input = fopen(my_args.file1.c_str(), "r");
    if(input == NULL) {
        printf("Error while opening file \"%s\", aborting\n", my_args.file1.c_str());
        abort();
    }
    char* line = NULL;
    size_t len = 0;
    unsigned int vertex;
    double score;
    unsigned int lineCount = 0;
    while(getline(&line, &len, input) != -1) {
        std::string tmp = line;
        size_t splitPos = tmp.find(my_args.split1);
        if(std::string::npos == splitPos) {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.file1.c_str());
            abort();
        }
        vertex = std::stoi(tmp.substr(0, splitPos));
        score = std::stod(tmp.substr(splitPos + 1, tmp.length()));
        //printf("From string \"%s\", %u %lf\n", tmp.c_str(), vertex, score);
        data1[vertex] = score;
        vertices1.insert(vertex);
        lineCount++;
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\nVertices: %lu\nLines in file: %u\n\n", (currTimeNano() - timeRead1Start)*1e-9, vertices1.size(), lineCount);

    auto timeRead2Start = currTimeNano();
	printf("Reading file \"%s\".\n", my_args.file2.c_str());
	std::unordered_map<unsigned int, double> data2;
    std::unordered_set<unsigned int> vertices2;
    input = fopen(my_args.file2.c_str(), "r");
    if(input == NULL) {
        printf("Error while opening file \"%s\", aborting\n", my_args.file2.c_str());
        abort();
    }
    line = NULL;
    len = 0;
    lineCount = 0;
    while(getline(&line, &len, input) != -1) {
        std::string tmp = line;
        size_t splitPos = tmp.find(my_args.split2);
        if(std::string::npos == splitPos) {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.file2.c_str());
            abort();
        }
        vertex = std::stoi(tmp.substr(0, splitPos));
        score = std::stod(tmp.substr(splitPos + 1, tmp.length()));
        data2[vertex] = score;
        lineCount++;
        vertices2.insert(vertex);
        //printf("From string \"%s\", %u %lf\n", tmp.c_str(), vertex, score);
        /*if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            data2[vertex] = score;
        } else {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.file2.c_str());
            abort();
        }*/
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\nVertices: %lu\nLines in file: %u\n\n", (currTimeNano() - timeRead2Start)*1e-9, vertices2.size(), lineCount);

    if(data1.size() != data2.size()) {
        printf("Warning: scores may be for different graphs (vertice set sizes are not equal)\n");
    }
    //assert(data1.size() == data2.size());

    double sumErr = 0.;
    double sumSquareErr = 0.;
    double maxErr = 0.;
    for(auto p: data1) {
        auto v1 = p.second;
        auto v2 = data2[p.first];
        auto err = fabs(v1 - v2);
        maxErr = fmax(err, maxErr);
		sumErr += err;
		sumSquareErr += err * err;
    }
    int printCounter = 0;
    int maxPrint = 30;
    while(printCounter < maxPrint) {
        printf("ID: %d, score1: %.10lf, score2: %.10lf\n", printCounter, data1[printCounter], data2[printCounter]);
        printCounter++;
    }

    printf("\n");
	printf("L1:   %lf (sum of absolute errors)\n", sumErr);
	printf("L2:   %lf (Euclidean norm of error vector)\n", sqrt(sumSquareErr));
	printf("Linf: %lf (max absolute error)\n", maxErr);
    printf("E:    %lf (average error)\n", sumErr / fmax(data1.size(), data2.size()));
    printf("RMSE: %lf (root mean square error)\n", sqrt(sumSquareErr / fmax(data1.size(), data2.size())));

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - t0)*1e-9);
    return 0;
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
