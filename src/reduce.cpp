#include <iostream>
#include <string>
#include <cassert>

int main(int argc, char** argv) {
	assert(argc <= 2);

	std::string indir;
	if(argc == 1) {
		indir = "./";
		std::cout << "Looking for .txt files in current directory\n";
	} else {
		indir = argv[1];
		std::cout << "Looking for .txt files in \"" << indir << "\"\n";
	}
}
