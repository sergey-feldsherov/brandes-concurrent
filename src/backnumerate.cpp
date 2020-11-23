#include <cstdio>
#include <string>
#include <unordered_map>
#include <argp.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string dataFile = "./output/scores.txt";
	std::string renumerationFile = "./output/new2old.txt";
    std::string outputFile = "./output/reduced-scores.txt";
};

Args my_args;

int main(int argc, char **argv) {
    auto t0 = currTimeNano();

    struct argp_option options[] = {
        {         "data",  -1, "FILE", 0,       "Path to file with scores" },
        { "renumeration",  -2, "FILE", 0, "Path to file with renumeration" },
        {       "output", 'o', "FILE", 0,            "Path to output file" },
        { 0 }
    };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    auto timeDataReadingStart = currTimeNano();
	printf("Reading scores from \"%s\"\n", my_args.dataFile.c_str());
	std::unordered_map<unsigned int, double> renumeratedScores;
    FILE *input = fopen(my_args.dataFile.c_str(), "r");
    if(input == NULL) {
        printf("\tError while opening file \"%s\", aborting\n", my_args.dataFile.c_str());
        abort();
    }
    char* line = NULL;
    size_t len = 0;
    unsigned int vertex;
    double score;
    unsigned int numLines = 0;
    while(getline(&line, &len, input) != -1) {
        if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            renumeratedScores[vertex] = score;
        } else {
            printf("\tInvalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.dataFile.c_str());
            abort();
        }
        numLines++;
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\n", (currTimeNano() - timeDataReadingStart)*1e-9);
    printf("Lines in file: %u\n", numLines);
    printf("Vertices: %lu\n", renumeratedScores.size());

    auto timeRenumerationReadingStart = currTimeNano();
	printf("Reading renumeration data from \"%s\"\n", my_args.renumerationFile.c_str());
	std::unordered_map<unsigned int, unsigned int> renumerationMap;//Converts new to old - backwards renumeration
    input = fopen(my_args.renumerationFile.c_str(), "r");
    if(input == NULL) {
        printf("\tError while opening file \"%s\", aborting\n", my_args.renumerationFile.c_str());
        abort();
    }
    line = NULL;
    len = 0;
    unsigned int v0, v1;
    numLines = 0;
    while(getline(&line, &len, input) != -1) {
        if(sscanf(line, "%u %u", &v0, &v1) == 2) {
            renumerationMap[v0] = v1;
        } else {
            printf("\tInvalid line: \"%s\" in file \"%s\", aborting\n", line, my_args.renumerationFile.c_str());
            abort();
        }
        numLines++;
    }
    if(line) {
        free(line);
    }
    fclose(input);
	printf("Done (%.2lf s)\n", (currTimeNano() - timeRenumerationReadingStart)*1e-9);
    printf("Lines in file: %u\n", numLines);

    auto timeWritingStart = currTimeNano();
	printf("Writing result to \"%s\"\n", my_args.outputFile.c_str());
	FILE* output = fopen(my_args.outputFile.c_str(), "w");
    if(output == NULL) {
        printf("\tError while opening file \"%s\", aborting\n", my_args.outputFile.c_str());
        abort();
    }
	for(auto p: renumeratedScores) {
		fprintf(output, "%u %.10lf\n", renumerationMap[p.first], p.second);
	}
	fclose(output);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeWritingStart)*1e-9);

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - t0)*1e-9);
    return 0;
}


static int parse_opt(int key, char *arg, struct argp_state *state) {
    if(key == -1) {
        my_args.dataFile = arg;
    } else if(key == -2) {
        my_args.renumerationFile = arg;
    } else if(key == 'o') {
        my_args.outputFile = arg;
    }

    return 0;
}


unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}
