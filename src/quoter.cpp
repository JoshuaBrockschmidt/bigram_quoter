//TODO: Seperate filtering from feed_stream.
//TODO: Add compatibility for []'s, ()'s, "'s and 's that surround text.
//TODO: Add possibility for multiple types of punctuation.

#include <fstream>
#include <sstream>
#include "quoter.hpp"

QuoterError::QuoterError(std::string m): msg(m) {}
const char* QuoterError::what() const throw() {
	return msg.c_str();
}

Quoter::Quoter():
	randGen(),
	// First two rows/columns of bigram array are START and END markers.
	bigram_array((int)Markers::NUM_ITEMS,
		     std::vector<int>((int)Markers::NUM_ITEMS,0)),
	bigram_rowSums((int)Markers::NUM_ITEMS, 0),
	// START and END markers don't require associated words.
	// Just give them empty strings.
	bigram_words((int)Markers::NUM_ITEMS, std::string()) {
	std::random_device rd;
	randGen.seed(rd());
}

void Quoter::feed_stream(std::istream& in) {
	// Read words in one by one.
	std::string word;
	bool sos=true, eos=false;
	unsigned int lastCol=(unsigned int)Markers::START;
	while (in >> word) {
		// Check for end of sentence.
		if (word.find('.') != std::string::npos ||
		    word.find('!') != std::string::npos ||
		    word.find('?') != std::string::npos) {
			if (sos) continue;
			else eos=true;
		}

		// Filter out unwanted characters.
		word = filterWord(word);

		// Check if filtered word is blank.
		if (word.empty()) {
			if (eos) {
				bigram_array[lastCol][(unsigned int)Markers::END] += 1;
				bigram_rowSums[lastCol] += 1;
				lastCol = (unsigned int)Markers::START;
				sos = true;
			}
			continue;
		}

		unsigned int row=0;
		// Find row/column index of word in bigram array.
	        for (std::vector<std::string>::iterator it=bigram_words.begin();
		     it!=bigram_words.end(); ++it) {
			if (word == *it)
				break;
			row++;
		}

		// If word does not yet exist in bigram array, add it.
		if (row >= bigram_words.size()) {
			bigram_words.push_back(word);

			// Extend all rows.
			std::vector<std::vector<int>>::iterator it;
			for (it=bigram_array.begin();
			     it!=bigram_array.end(); ++it)
				(*it).push_back(0);

			// Add rows.
			std::vector<int> newRow(bigram_words.size(), 0);
			bigram_array.push_back(newRow);
			bigram_rowSums.push_back(0);
		}

		// Update bigram array.
		bigram_array[lastCol][row] += 1;
		bigram_rowSums[lastCol] += 1;
		if (eos) {
			bigram_array[row][(unsigned int)Markers::END] += 1;
			bigram_rowSums[row] += 1;
			lastCol = (unsigned int)Markers::START;
			eos = false;
			sos = true;
		} else {
			lastCol = row;
			if (sos) sos=false;
		}
	}
}

void Quoter::feed_file(std::string filePath) {
	std::ifstream ifs(filePath.c_str());

	if (!ifs.is_open()) {
		std::string m = "Quoter::feed: Could not open ";
		m += filePath;
		throw QuoterError(m);
	}

        std::istream& is = ifs;
	feed_stream(is);
	ifs.close();
}

void Quoter::feed_string(std::string text) {
	std::istringstream iss(text);
	std::istream& is = iss;
	feed_stream(is);
}

std::string Quoter::buildSentence() {
	std::string sentence;

	int row = (unsigned int)Markers::START;
	unsigned int goal, sum, col;
	while (true) {
	        goal=((unsigned int)randGen()%(unsigned int)bigram_rowSums[row])+1;
		sum=0, col=row;
		while (sum<goal) {
			col = ++col%bigram_array.size();
			sum += bigram_array[row][col];
		}

		if (col != (unsigned int)Markers::END) {
			sentence += bigram_words[col];
			sentence += ' ';
			row=col;
		} else {
			// Replace trailing space with a period.
			sentence.back() = '.';
			break;
		}
	}

	return sentence;
}

void Quoter::emitArray() {
	std::vector<std::vector<int>>::iterator row;
	std::vector<int>::iterator col;
	for (row=bigram_array.begin(); row!=bigram_array.end(); ++row) {
		for (col=row->begin(); col!=row->end(); ++col) {
			std::cout << *col << ' ';
		}
		std::cout << std::endl;
	}
}

std::string Quoter::filterWord(std::string& word) {
	char filtered[word.size()];
        unsigned int i = 0;
	for (std::string::iterator it=word.begin(); it!=word.end(); it++) {
		if ((*it >= '#' && *it <='\'') ||
		    (*it == ',') ||
		    (*it == '-') ||
		    (*it >= '0' && *it <= '9') ||
		    (*it >= '@' && *it <= 'Z') ||
		    (*it >= 'a' && *it <= 'z') ||
		    (*it >= 128 && *it <= 167) ||
		    (*it == 171) ||
		    (*it == 172) ||
		    (*it >= 224 && *it <= 239) ||
		    (*it >= 251 && *it <= 253)) {
			filtered[i++] = *it;
		}
	}
	filtered[i] = '\0';

	return std::string(filtered);
}
