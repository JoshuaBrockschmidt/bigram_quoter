#include <unistd.h>
#include "quoter.hpp"

const char* savefile="save.dat";

int main() {
	Quoter quoterObj;
	if (access(savefile, F_OK)!=-1) {
		std::cout << "Loading data..." << std::endl;
		quoterObj.readData(std::string(savefile));
	} else {
		quoterObj.feed_file("test.txt");
		std::cout << "Saving data..." << std::endl;
		quoterObj.writeData(std::string(savefile));
	}
	std::cout << quoterObj.buildSentence() << std::endl;

	return 0;
}
