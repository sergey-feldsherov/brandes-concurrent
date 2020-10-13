#include <fstream>
#include <iostream>
#include <cassert>
#include <string>
#include <unordered_map>

int main(int argc, char **argv) {
	assert(argc == 4);

	std::string dataFile = argv[1];
	std::string renumerationFile = argv[2];
	std::string outputFile = argv[3];

	std::cout << "Reading scores from \"" << dataFile << "\"\n";
	std::unordered_map<unsigned int, double> renumeratedScores;
	std::ifstream ifs;
	ifs.open(dataFile);
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

	std::cout << "Reading renumeration array from \"" << renumerationFile << "\"\n";
	std::unordered_map<unsigned int, unsigned int> renumerationMap;//Converts new to old
	ifs.open(renumerationFile);
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

	std::cout << "Writing result to \"" << outputFile << "\"\n";
	std::ofstream ofs;
	ofs.open("backnumerated.txt");
	for(auto p: renumeratedScores) {
		ofs << renumerationMap[p.first] << " " << p.second << "\n";
	}
	ofs.close();
	std::cout << "Finished\n";
	
}
