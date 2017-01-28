#include <iostream>
#include <PegFinder.hpp>
#include <string>

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 

	//TODO: Implement a better file saving system! Unordered maps with only ints cannot handle the serial numbers of the cameras
	char serial[11] = "2391000767"; //It's 10 chars long, but there's also the null char
	PegFinder* finder = new PegFinder(serial, argv[1], &std::cout);

	while (true) {
		finder->ProcessFrame();
	}

}
