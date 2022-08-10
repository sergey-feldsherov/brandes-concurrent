#include <cstdio>
#include <string>
#include <dirent.h>
#include <vector>
#include <unordered_map>
#include <argp.h>
#include <unistd.h>
#include <time.h>

unsigned long long currTimeNano();
static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string inputDirectory = "./input/";
    std::string suffix = ".txt";
    std::string outputFile = "./output/reduced.txt";
};

Args my_args;

int main(int argc, char** argv) {
    auto t0 = currTimeNano();

    struct argp_option options[] = {
        {  "indir", 'i',    "DIR", 0, "Path to directory with input files" },
        { "suffix", 's', "SUFFIX", 0,   "Suffix to look for in file names" },
        { "output", 'o',   "FILE", 0,                "Path to output file" },
        { 0 }
    };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    if(my_args.inputDirectory.back() != '/') {
		my_args.inputDirectory.append("/");
	}

    auto timeLookupStart = currTimeNano();
	printf("Looking for \"*%s\" files in \"%s\"\n", my_args.suffix.c_str(), my_args.inputDirectory.c_str());
	std::vector<std::string> filesVec;
	DIR *dir;
	struct dirent *ent;
	if((dir = opendir(my_args.inputDirectory.c_str())) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			std::string fname = ent->d_name;
			if(fname.length() >= my_args.suffix.length() && fname.compare(fname.length() - my_args.suffix.length(), my_args.suffix.length(), my_args.suffix) == 0) {
				filesVec.push_back(my_args.inputDirectory + ent->d_name);
			}
		}
		closedir(dir);
	} else {
		printf("Unable to open directory \"%s\", aborting\n", my_args.inputDirectory.c_str());
		abort();
	}

    printf("Done (%.2lf s)\n", (currTimeNano() - timeLookupStart)*1e-9);
	if(filesVec.size() == 0) {
		printf("Found 0 \"*%s\" files\n", my_args.suffix.c_str());
		return 0;
	}
	printf("Found %lu \"*%s\" files:\n", filesVec.size(), my_args.suffix.c_str());
	for(auto name: filesVec) {
		printf("\t%s\n", name.c_str());
	}

    auto timeReadingStart = currTimeNano();
    printf("Reading files:\n");
    std::unordered_map<unsigned int, double> reducedScores;
    for(auto name: filesVec) {
        auto timeFileReadingStart = currTimeNano();
        FILE *input = fopen(name.c_str(), "r");
        if(input == NULL) {
            printf("\tError while opening file \"%s\", aborting\n", name.c_str());
            abort();
        }
        char* line = NULL;
        size_t len = 0;
        unsigned int vertex;
        double score;
        unsigned int numLines = 0;
        while(getline(&line, &len, input) != -1) {
            if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
                reducedScores[vertex] += score;
            } else {
                printf("\tInvalid line: \"%s\" in file \"%s\", aborting\n", line, name.c_str());
                abort();
            }
            numLines++;
        }
        if(line) {
            free(line);
        }
        fclose(input);
        printf("\t%s - %u lines (%.2lf s)\n", name.c_str(), numLines, (currTimeNano() - timeFileReadingStart)*1e-9);
    }
    printf("Done (%.2lf s)\n", (currTimeNano() - timeReadingStart)*1e-9);

    auto timeWritingStart = currTimeNano();
    printf("Writing to \"%s\"\n", my_args.outputFile.c_str());
    FILE* output = fopen(my_args.outputFile.c_str(), "w");
    if(output == NULL) {
        printf("\tError while opening file \"%s\", aborting\n", my_args.outputFile.c_str());
        abort();
    }
    for(auto p: reducedScores) {
        fprintf(output, "%u %.15e\n", p.first, p.second);
    }
    fclose(output);
    printf("Done (%.2lf s)\n", (currTimeNano() - timeWritingStart)*1e-9);

    printf("\nFinished (%.2lf s)\n", (currTimeNano() - t0)*1e-9);
    return 0;
}


static int parse_opt(int key, char *arg, struct argp_state *state) {
    if(key == 'i') {
        my_args.inputDirectory = arg;
    } else if(key == 's') {
        my_args.suffix = arg;
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
