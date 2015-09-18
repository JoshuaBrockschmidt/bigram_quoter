#include "argparser.hpp"
#include "quoter.hpp"

int main(int argc, char** argv) {
	ArgParser::parseArgs(argc, argv);

	return 0;
}
