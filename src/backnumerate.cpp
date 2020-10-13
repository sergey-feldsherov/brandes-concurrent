#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <argp.h>

static int parse_opt(int key, char *arg, struct argp_state *state);

struct Args {
	std::string dataFile;
	std::string renumerationFile;
    std::string outputFile;
};

Args my_args;

int main(int argc, char **argv) {

    struct argp_option options[] = {
        {         "data",  -1, "FILE", 0,       "Path to file with scores" },
        { "renumeration",  -2, "FILE", 0, "Path to file with renumeration" },
        {       "output", 'o', "FILE", 0,            "Path to output file" },
        { 0 }
    };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

	std::cout << "Reading scores from \"" << my_args.dataFile << "\"\n";
	std::unordered_map<unsigned int, double> renumeratedScores;
	std::ifstream ifs;
	ifs.open(my_args.dataFile);
	while(ifs.peek() != EOF) {
		char line[256];
		ifs.getline(line, 256);
		unsigned int vertex;
		double score;
		if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            renumeratedScores[vertex] = score;
        } else {
            std::cout << "\tInvalid line: " << line << "\n";
			ifs.close();
            abort();
        }
	}
	ifs.close();
    std::cout << "Vertices: " << renumeratedScores.size() << "\n";

	std::cout << "Reading renumeration array from \"" << my_args.renumerationFile << "\"\n";
	std::unordered_map<unsigned int, unsigned int> renumerationMap;//Converts new to old
	ifs.open(my_args.renumerationFile);
	while(ifs.peek() != EOF) {
		char line[256];
		ifs.getline(line, 256);
		unsigned int v0;
		unsigned int v1;
		if(sscanf(line, "%u %u", &v0, &v1) == 2) {
            renumerationMap[v0] = v1;
        } else {
            std::cout << "\tInvalid line: " << line << "\n";
			ifs.close();
            abort();
        }
	}
	ifs.close();
    std::cout << "Renumeration data size: " << renumerationMap.size() << "\n";

	std::cout << "Writing result to \"" << my_args.outputFile << "\"\n";
	std::ofstream ofs;
	ofs.open(my_args.outputFile);
	for(auto p: renumeratedScores) {
		ofs << renumerationMap[p.first] << " " << p.second << "\n";
	}
	ofs.close();
	std::cout << "Finished\n";
	
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
