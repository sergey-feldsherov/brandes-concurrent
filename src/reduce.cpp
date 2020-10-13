#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <dirent.h>
#include <vector>
#include <unordered_map>

int main(int argc, char** argv) {
	assert(argc <= 2);

	std::string indir;
	if(argc == 1) {
		indir = "./";
	} else {
		indir = argv[1];
		if(indir.back() != '/') {
			indir.append("/");
		}
	}
	std::cout << "Looking for .txt files in \"" << indir << "\"\n";

	std::vector<std::string> filesVec;

	DIR *dir;
	struct dirent *ent;
	if((dir = opendir(indir.c_str())) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			std::string newFileName = ent->d_name;
			std::string suffix = ".txt";
			if(newFileName.length() >= 4 && newFileName.compare(newFileName.length() - suffix.length(), suffix.length(), suffix) == 0) {
				filesVec.push_back(indir + ent->d_name);
			}
		}
		closedir(dir);
	} else {
		std::cout << "Unable to open directory \"" << indir << "\"\n";
		return -1;
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



















