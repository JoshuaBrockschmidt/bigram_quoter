/* TODO
 *  - Seperate filtering from feed_stream.
 *  - Add compatibility for []'s, ()'s, "'s and 's that surround text.
 *  - Add possibility for multiple types of punctuation.
 *  - Add saving and loading.
 */

#include <fstream>
#include <sstream>
#include "quoter.hpp"

enum struct ParserItemTypes: unsigned int {
	MARKER,
        WORD
};
struct ParserItem {
        ParserItemTypes type;
};
struct ParserItem_marker: ParserItem {
	Quoter::Markers marker;
	ParserItem_marker(Quoter::Markers m) {
		type=ParserItemTypes::MARKER;
		marker=m;
	}
};
struct ParserItem_word: ParserItem {
	std::string word;
	ParserItem_word(std::string w) {
		type=ParserItemTypes::WORD;
		word=w;
	}
};

QuoterError::QuoterError(std::string m): msg(m) {}
const char* QuoterError::what() const throw() {
	return msg.c_str();
}

Quoter::Quoter():
	randGen(),
	// First two rows/columns of bigram array are START and END markers.
	bigram_array((int)Markers::NUM_ITEMS,
		     std::vector<int>((int)Markers::NUM_ITEMS, 0)),
	bigram_rowSums((int)Markers::NUM_ITEMS, 0),
	// START and END markers don't require associated words.
	// Just give them empty strings.
	bigram_words((int)Markers::NUM_ITEMS, std::string()) {
	std::random_device rd;
	randGen.seed(rd());
}

void Quoter::feed_stream(std::istream& in) {
	// Initialize pre-parser FIFO stack.
	std::vector<ParserItem*> parserItems;

	/// FIRST PARSING RUN ///
	// Read words in one by one.
	std::string word;
	bool sos=true, eos=false;
	Markers eos_marker;
	while (in >> word) {
		// Check for end of sentence.
		eos=true;
		if (word.find('.') != std::string::npos)
			eos_marker=Markers::PERIOD;
		else if (word.find('!') != std::string::npos)
			eos_marker=Markers::EXCLAIM;
		else if (word.find('?') != std::string::npos)
			eos_marker=Markers::QUESTION;
		else
			eos=false;

		// Filter out unwanted characters.
		word = filterWord(word);

		if (sos) {
			ParserItem_marker* newStart;
			newStart=new ParserItem_marker(Markers::START);
			parserItems.push_back(newStart);
			sos=false;
		}
		if (!word.empty()) {
			ParserItem_word* newWord;
			newWord=new ParserItem_word(word);
			parserItems.push_back(newWord);
		}
		// Make sure at least one word is in the current sentence
		// if it is being ended.
		ParserItem& last=*parserItems.back();
		if (eos &&
		    !(last.type==ParserItemTypes::MARKER &&
		      ((ParserItem_marker&)last).marker==Markers::START)) {
			ParserItem_marker* newEnd;
			newEnd=new ParserItem_marker(eos_marker);
			parserItems.push_back(newEnd);
			sos=true;
		}
	}

	// Mend end of stack if needed.
        ParserItem& lastParItm=*parserItems.back();
	if (lastParItm.type==ParserItemTypes::MARKER &&
	    ((ParserItem_marker&)lastParItm).marker==Markers::START) {
		parserItems.pop_back();
	} else if (lastParItm.type==ParserItemTypes::WORD) {
		ParserItem_marker* newEnd;
		newEnd=new ParserItem_marker(Markers::PERIOD);
		parserItems.push_back(newEnd);
	}

	/// SECOND PARSING RUN ///
	//TODO: Reduce indentations. Add helper functions and/or restructure.
	unsigned int lastCol, row;
	// Initialize lastCol with assumption that
	// the first parser item is a START marker.
	lastCol=(unsigned int)Markers::START;
	// First parser item has been accounted for,
	// so start iterator on second element.
	for (std::vector<ParserItem*>::iterator pItm=parserItems.begin()+1;
	     pItm!=parserItems.end(); ++pItm) {
		switch ((*pItm)->type) {
		case ParserItemTypes::MARKER:
			{
				ParserItem_marker& mark=
					*((ParserItem_marker*)(*pItm));
				row=(unsigned int)mark.marker;
				break;
			}
		case ParserItemTypes::WORD:
			{
				ParserItem_word& word=
					*((ParserItem_word*)(*pItm));

				// Find row index of word in bigram array.
				row=0;
				std::vector<std::string>::iterator w;
				for (w=bigram_words.begin();
				     w!=bigram_words.end(); ++w) {
					if (word.word == *w)
						break;
					row++;
				}
				// If word does not yet exist in bigram array,
				// add it.
				if (row >= bigram_words.size()) {
					bigram_words.push_back(word.word);

					// Extend all rows.
					std::vector<std::vector<int>>::iterator r;
					for (r=bigram_array.begin();
					     r!=bigram_array.end(); ++r)
						r->push_back(0);

					// Add rows.
					std::vector<int> newRow(bigram_words.size(), 0);
					bigram_array.push_back(newRow);
					bigram_rowSums.push_back(0);
				}
				break;
			}
		default:
			std::cerr << "Quoter::feed_stream: "
				  << "Unexpected error in second parsing run."
				  << std::endl;
		}

		bigram_array[lastCol][row]++;
		bigram_rowSums[lastCol]++;

		lastCol=row;
	}

	//Delete parser items.
	for (std::vector<ParserItem*>::iterator pItm=parserItems.begin();
	     pItm!=parserItems.end(); ++pItm)
		delete *pItm;
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

		if (col == (unsigned int)Markers::PERIOD) {
			sentence.back() = '.';
			break;
		} else if (col == (unsigned int)Markers::EXCLAIM) {
			sentence.back() = '!';
			break;
		} else if (col == (unsigned int)Markers::QUESTION) {
			sentence.back() = '?';
			break;
		} else {
			sentence += bigram_words[col];
			sentence += ' ';
			row=col;
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
