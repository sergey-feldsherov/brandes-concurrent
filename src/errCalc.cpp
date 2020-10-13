#include <cstdio>
#include <unordered_map>
#include <cassert>
#include <string>
#include <cmath> //fabs(), fmax(), sqrt()
#include <argp.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string file1;
	std::string file2;
};

Args my_args;

int main(int argc, char** argv) {
    auto t0 = currTimeNano();

    struct argp_option options[] = {
        {  "file1",  -1, "FILE", 0,  "Path to first file" },
        {  "file2",  -2, "FILE", 0, "Path to second file" },
        { 0 }
    };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    auto timeRead1Start = currTimeNano();
	printf("Reading file \"%s\".\n", my_args.file1.c_str());
	std::unordered_map<unsigned int, double> data1;
    FILE* input = fopen(my_args.file1.c_str(), "r");
    if(input == NULL) {
        printf("Error while opening file \"%s\", aborting\n", my_args.file1.c_str());
        abort();
    }
    char* line = NULL;
    size_t len = 0;
    unsigned int vertex;
    double score;
    while(getline(&line, &len, input) != -1) {
        if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            data1[vertex] = score;
        } else {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.file1.c_str());
            abort();
        }
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\nVertices: %lu\n", (currTimeNano() - timeRead1Start)*1e-9, data1.size());

    auto timeRead2Start = currTimeNano();
	printf("Reading file \"%s\".\n", my_args.file2.c_str());
	std::unordered_map<unsigned int, double> data2;
    input = fopen(my_args.file2.c_str(), "r");
    if(input == NULL) {
        printf("Error while opening file \"%s\", aborting\n", my_args.file2.c_str());
        abort();
    }
    line = NULL;
    len = 0;
    while(getline(&line, &len, input) != -1) {
        if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            data2[vertex] = score;
        } else {
            printf("Invalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.file2.c_str());
            abort();
        }
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\nVertices: %lu\n", (currTimeNano() - timeRead2Start)*1e-9, data2.size());

    assert(data1.size() == data2.size());

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

    printf("\n");
	printf("L1:   \t%lf (sum of absolute errors)\n", sumErr);
	printf("L2:   \t%lf (Euclidean norm of error vector)\n", sqrt(sumSquareErr));
	printf("Linf: \t%lf (max absolute error)\n", maxErr);

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - timeRead1Start)*1e-9);
    return 0;
}


static int parse_opt(int key, char *arg, struct argp_state *state) {
    if(key == -1) {
        my_args.file1 = arg;
    } else if(key == -2) {
        my_args.file2 = arg;
    }

    return 0;
}


unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}
