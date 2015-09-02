#include "quoter.hpp"

int main() {
	Quoter quoterObj;
	quoterObj.feed_file("test.txt");
	std::cout << quoterObj.buildSentence() << std::endl;

	return 0;
}
