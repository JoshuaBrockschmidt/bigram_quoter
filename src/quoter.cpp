#include <fstream>
#include <sstream>
#include "quoter.hpp"

QuoterError::QuoterError(std::string m): msg(m) {}
const char* QuoterError::what() const throw() {
	return msg.c_str();
}

Quoter::Quoter():
	// First two rows/columns of bigram array are START and STOP markers.
	bigram_array(2, std::vector<int>(2,0)),
	bigram_rowSums(2, 0),
	// START and STOP markers don't require associated words.
	// Just give them empty strings.
	bigram_words(2, std::string()) {}

void Quoter::feed_stream(const std::istream& in) {
	//TODO: Index text into bigram model.
	//DEBUG
	std::cout << in.rdbuf() << std::endl;
	//EOF DEBUG
}

void Quoter::feed_file(std::string filePath) {
	std::ifstream ifs(filePath.c_str());

	if (!ifs.is_open()) {
		std::string m = "Quoter::feed: Could not open ";
		m += filePath;
		throw QuoterError(m);
	}

	const std::istream& is = ifs;
	feed_stream(is);
	ifs.close();
}

void Quoter::feed_string(std::string text) {
	std::istringstream iss(text);
	const std::istream& is = iss;
	feed_stream(is);
}
