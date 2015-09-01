#include "quoter.hpp"

int main() {
	Quoter quoterObj;
	quoterObj.feed_string("This is a test stream.");
	quoterObj.feed_file("test.txt");

	return 0;
}
