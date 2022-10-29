#include <cstdio>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <argp.h>
#include <cstdint>
#include <cinttypes>
#include <limits>
#include <time.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string inputFile = "./input/input.txt";
	std::string outputFile = "./output/output.txt";
	int france = 0;//see options
};

Args my_args;

int main(int argc, char **argv) {
    auto t0 = currTimeNano();

    struct argp_option options[] = {
        {  "input", 'i', "FILE", 0,                                                  "Path to input file", 0 },
        { "output", 'o', "FILE", 0,                                                 "Path to output file", 0 },
	{ "french", 'f',      0, 0, "If included, graph vertices will be enumerated from 1 instead of 0.", 0 },
        {        0,   0,      0, 0,                                                                     0, 0 }
    };
    struct argp argp = { options, parse_opt, 0, 0, 0, 0, 0 };
    argp_parse(&argp, argc, argv, 0, 0, 0);

    auto timeReadingStart = currTimeNano();
    printf("Reading graph from \"%s\"\n", my_args.inputFile.c_str());
    FILE *input = fopen(my_args.inputFile.c_str(), "r");
    if(input == NULL) {
        printf("\tUnable to open input file \"%s\", aborting\n", my_args.inputFile.c_str());
        abort();
    }
    std::unordered_set< uint64_t > allVerticesSet;
    std::unordered_map< uint64_t, std::unordered_set< uint64_t > > allEdgesMap;
    uint64_t edgeCount = 0;
    char* line = NULL;
    size_t len = 0;
    uint64_t v0, v1;
    while(getline(&line, &len, input) != -1) {
        if(line[0] == '#') {
            printf("\tEncountered a commentary line\n");
        } else {
            if(sscanf(line, "%" SCNu64 " %" SCNu64, &v0, &v1) == 2) {
                allEdgesMap[v0].insert(v1);

                allVerticesSet.insert(v0);
                allVerticesSet.insert(v1);
                edgeCount++;
            } else {
                printf("\tInvalid line: \"%s\", aborting\n", line);
                abort();
            }
        }
    }
    if(line) {
        free(line);
    }
    fclose(input);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeReadingStart)*1e-9);
    printf("Vertices: %lu, edges: %" PRIu64 "\n", allVerticesSet.size(), edgeCount);

    auto timeConstructionStart = currTimeNano();
    printf("\nConstructing renumeration structures\n");
    std::vector<uint64_t> vertices = std::vector<uint64_t>(allVerticesSet.begin(), allVerticesSet.end());
    std::unordered_map<uint64_t, uint64_t> renumerationTable;
    for(uint64_t i = 0; i < vertices.size(); i++) {
        renumerationTable[vertices[i]] = i;
    }
    printf("Done (%.4lf seconds)\n", (currTimeNano() - timeConstructionStart)*1e-9);

    auto timeGraphWriteStart = currTimeNano();
    std::string renumeratedPath = my_args.outputFile + ".renumerated";
    printf("\nWriting renumerated graph to \"%s\"\n", renumeratedPath.c_str());
    FILE *output = fopen(renumeratedPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file \"%s\", aborting\n", renumeratedPath.c_str());
        abort();
    }
    for(uint64_t i = 0; i < vertices.size(); i++) {
        for(auto vrtx: allEdgesMap[vertices[i]]) {
            fprintf(output, "%" PRIu64 " %" PRIu64 "\n", i + my_args.france, renumerationTable[vrtx] + my_args.france);
        }
    }
    fclose(output);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeGraphWriteStart)*1e-9);


    auto timeOld2newWriteStart = currTimeNano();
    std::string old2newPath = my_args.outputFile + ".old2new";
    printf("Writing old-to-new conversion table to \"%s\"\n", old2newPath.c_str());
    output = fopen(old2newPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file \"%s\", aborting\n", old2newPath.c_str());
        abort();
    }
    for(uint64_t i = 0; i < vertices.size(); i++) {
        fprintf(output, "%" PRIu64 " %" PRIu64 "\n", vertices[i], i + my_args.france);
    }
    fclose(output);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeOld2newWriteStart)*1e-9);

    auto timeNew2oldWriteStart = currTimeNano();
    std::string new2oldPath = my_args.outputFile + ".new2old";
    printf("Writing new-to-old conversion table to \"%s\"\n", new2oldPath.c_str());
    output = fopen(new2oldPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file \"%s\",\n", new2oldPath.c_str());
        abort();
    }
    for(uint64_t i = 0; i < vertices.size(); i++) {
        fprintf(output, "%" PRIu64 " %" PRIu64 "\n", i + my_args.france, vertices[i]);
    }
    fclose(output);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeNew2oldWriteStart)*1e-9);

    printf("\nMaximal value for <unsigned int> for this system: %u (+1: %u)\n", std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max() + 1);

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - t0)*1e-9);
    return 0;
}


static int parse_opt(int key, char *arg, struct argp_state *) {
    if(key == 'i') {
	    my_args.inputFile = arg;
    } else if(key == 'o') {
	    my_args.outputFile = arg;
    } else if(key == 'f') {
	    my_args.france = 1;
    }

    return 0;
}


unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}
