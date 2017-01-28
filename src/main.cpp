#include <iostream>
#include <PegFinder.hpp>
#include <Networking.hpp>
#include <string>

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 

	//TODO: Implement a better file saving system! Unordered maps with only ints cannot handle the serial numbers of the cameras
	//char serial[11] = "2391000767"; //It's 10 chars long, but there's also the null char
	//PegFinder* finder = new PegFinder(serial, argv[1], &std::cout);

	//while (true) {
	//	finder->ProcessFrame();
	//}

	//Trevor sends me either one or the other messages, so I just have one thread that waits for a message, and when it accepts the
	//message it interprets it, and then it uses either of the modules' process_frame functions to send back a message to trevor.

	Networking::Server* serv = new Networking::Server(5805);			

	while(true) {
		std::cout << "Waiting for client connection on port " << 5805 << std::endl;
		serv->WaitForClientConnection();
		while (true) {
			if (!serv->WaitForClientMessage(&std::cout)) {
				break;
			}
		}
		std::cout << "Connection stopped. Uh oh." << std::endl;
	}

}
