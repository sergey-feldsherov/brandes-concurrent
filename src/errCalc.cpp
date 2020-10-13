#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cassert>
#include <string>
#include <cmath> //fabs(), fmax(), sqrt()

int main(int argc, char** argv) {
	assert(argc = 3);

	std::string file1 = argv[1];
	std::string file2 = argv[2];
	std::ifstream ifs;

	std::cout << "Reading file \"" << file1 << "\"\n";
	std::unordered_map<unsigned int, double> data1;
	ifs.open(file1);
	while(ifs.peek() != EOF) {
		char line[256];
		ifs.getline(line, 256);
		unsigned int vertex;
		double score;
		if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            data1[vertex] = score;
        } else {
            std::cout << "\tInvalid line: " << line << "\n";
			ifs.close();
            abort();
        }
	}
	ifs.close();
	std::cout << "Vertices: " << data1.size() << "\n";

	std::cout << "Reading file \"" << file2 << "\"\n";
	std::unordered_map<unsigned int, double> data2;
	ifs.open(file2);
	while(ifs.peek() != EOF) {
		char line[256];
		ifs.getline(line, 256);
		unsigned int vertex;
		double score;
		if(sscanf(line, "%u %lf", &vertex, &score) == 2) {
            data2[vertex] = score;
        } else {
            std::cout << "\tInvalid line: " << line << "\n";
			ifs.close();
            abort();
        }
	}
	ifs.close();
	std::cout << "Vertices: " << data2.size() << "\n";

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

	std::cout << "L1: \t" << sumErr << " (sum of absolute values)\n";
	std::cout << "L2: \t" << sqrt(sumSquareErr) << " (Euclidean)\n";
	std::cout << "L" << "\u221E" << ": \t" << maxErr << " (max absolute value)\n";

}
