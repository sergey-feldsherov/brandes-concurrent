#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <vector>
#include <unordered_map>
#include <argp.h>

static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string inputDirectory;
    std::string suffix;
    std::string outputFile;
};

Args my_args;

int main(int argc, char** argv) {

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

	std::cout << "Looking for .txt files in \"" << my_args.inputDirectory << "\"\n";

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
		std::cout << "Unable to open directory \"" << my_args.inputDirectory << "\"\n";
		abort();
	}

	if(filesVec.size() == 0) {
		std::cout << "Found 0 .txt files\n";
		return 0;
	}
	std::cout << "Found " << filesVec.size() << " .txt files:\n";
	for(auto name: filesVec) {
		std::cout << "\t" << name << "\n";
	}

	std::unordered_map<unsigned int, double> reducedScores;
	for(auto name: filesVec) {
		std::cout << "Reading file " << name << "\n";
		std::ifstream ifs;
		ifs.open(name);
		while(ifs.peek() != EOF) {
			char line[256];
			ifs.getline(line, 256);
			unsigned int vertex;
			double score;
			if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
                reducedScores[vertex] += score;
            } else {
                std::cout << "\tInvalid line: " << line << "\n";
				ifs.close();
                abort();
            }
		}
		ifs.close();
	}
	std::cout << "Reading done\n";

	std::cout << "Writing to \"reduced.txt\"\n";
	std::ofstream ofs;
	ofs.open("reduced.txt");
	for(auto p: reducedScores) {
		ofs << p.first << " " << p.second << "\n";
	}
	ofs.close();
	std::cout << "Finished\n";

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



















