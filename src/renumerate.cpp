#include <stdio.h>
#include <cassert>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

typedef unsigned int vertex;

unsigned long long currTimeNano();

int main(int argc, char **argv) {
    assert(argc == 2);

    auto t0 = currTimeNano();

    std::string inputPath = argv[1];
    printf("Reading graph from %s\n", inputPath.c_str());

    FILE *input = fopen(inputPath.c_str(), "r");
    if(input == NULL) {
        printf("\tUnable to open input file: %s.\n", inputPath.c_str());
        abort();
    }

    std::unordered_set< vertex > allVerticesSet;
    std::unordered_map< vertex, std::unordered_set< vertex > > allEdgesMap;
    unsigned int edgeCount = 0;
    auto t = currTimeNano();

    char* line = NULL;
    size_t len = 0;
    vertex v0, v1;
    while(getline(&line, &len, input) != -1) {
        if(line[0] == '#') {
            printf("\tEncountered a commentary line\n");
        } else {
            if(sscanf(line, "%u %u", &v0, &v1) == 2) {
                allEdgesMap[v0].insert(v1);

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

    std::vector< vertex > vertices = std::vector< vertex >(allVerticesSet.begin(), allVerticesSet.end());
	std::unordered_map< vertex, unsigned int > renumerationTable;
    for(unsigned int i = 0; i < vertices.size(); i++) {
        renumerationTable[vertices[i]] = i;
    }

    t = currTimeNano() - t;
    printf("Done (%.4lf seconds)\n", t * 1e-9);
    printf("Vertices: %lu, edges: %u\n", vertices.size(), edgeCount);

    std::string renumeratedPath = inputPath + ".renumerated";
    printf("\nSaving data to %s\n", renumeratedPath.c_str());

    FILE *output = fopen(renumeratedPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", renumeratedPath.c_str());
        abort();
    }

    t = currTimeNano();
    for(unsigned int i = 0; i < vertices.size(); i++) {
        for(auto vrtx: allEdgesMap[vertices[i]]) {
            fprintf(output, "%u %u\n", i, renumerationTable[vrtx]);
        }
    }
    fclose(output);

    t = currTimeNano() - t;
    printf("Done (%.5lf seconds)\n", t * 1e-9);


    std::string old2newPath = inputPath + ".old2new";
    printf("\nSaving data to %s\n", old2newPath.c_str());

    output = fopen(old2newPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", old2newPath.c_str());
        abort();
    }

    t = currTimeNano();
    for(unsigned int i = 0; i < vertices.size(); i++) {
        fprintf(output, "%u %u\n", vertices[i], i);
    }
    fclose(output);

    t = currTimeNano() - t;
    printf("Done (%.5lf seconds)\n", t * 1e-9);

    std::string new2oldPath = inputPath + ".new2old";
    printf("\nSaving data to %s\n", new2oldPath.c_str());

    output = fopen(new2oldPath.c_str(), "w");
    if(output == NULL) {
        printf("Unable to open output file: %s.\n", new2oldPath.c_str());
        abort();
    }

    t = currTimeNano();
    for(unsigned int i = 0; i < vertices.size(); i++) {
        fprintf(output, "%u %u\n", i, vertices[i]);
    }
    fclose(output);

    t = currTimeNano() - t;
    printf("Done (%.5lf seconds)\n", t * 1e-9);

    auto tTotal = currTimeNano() - t0;
    printf("\nTotal work time: %.5lf seconds\n", tTotal * 1e-9);

}

unsigned long long currTimeNano() {
    struct timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}
