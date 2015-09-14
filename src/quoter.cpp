/* TODO
 *  - Seperate filtering from feed_stream.o
 *  - Add compatibility for []'s, ()'s, "'s and 's that surround text.
 */

#include <fstream>
#include <sstream>
#include "quoter.hpp"

enum struct Markers: std::uint32_t {
	START,
	PERIOD,
	EXCLAIM,
	QUESTION,
	NUM_ITEMS
};
enum struct ParserItemTypes: std::uint32_t {
	MARKER,
        WORD
};
struct ParserItem {
        ParserItemTypes type;
};
struct ParserItem_marker: ParserItem {
	Markers marker;
	ParserItem_marker(Markers m) {
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
		     std::vector<std::uint32_t>((int)Markers::NUM_ITEMS, 0)),
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
	bool start_of_sentence=true, end_of_sentence=false;
	Markers end_marker;
	while (in >> word) {
		// Check for end of sentence.
		end_of_sentence=true;
		if (word.find('.') != std::string::npos)
			end_marker=Markers::PERIOD;
		else if (word.find('!') != std::string::npos)
			end_marker=Markers::EXCLAIM;
		else if (word.find('?') != std::string::npos)
			end_marker=Markers::QUESTION;
		else
			end_of_sentence=false;

		// Filter out unwanted characters.
		word = filterWord(word);

		if (start_of_sentence) {
			ParserItem_marker* newStart;
			newStart=new ParserItem_marker(Markers::START);
			parserItems.push_back(newStart);
			start_of_sentence=false;
		}
		if (!word.empty()) {
			ParserItem_word* newWord;
			newWord=new ParserItem_word(word);
			parserItems.push_back(newWord);
		}
		// Make sure at least one word is in the current sentence
		// if it is being ended.
		ParserItem& last=*parserItems.back();
		if (end_of_sentence &&
		    !(last.type==ParserItemTypes::MARKER &&
		      ((ParserItem_marker&)last).marker==Markers::START)) {
			ParserItem_marker* newEnd;
			newEnd=new ParserItem_marker(end_marker);
			parserItems.push_back(newEnd);
			start_of_sentence=true;
		}
	}

	// Mend end of stack if needed.
        ParserItem& last=*parserItems.back();
	if (last.type==ParserItemTypes::MARKER &&
	    ((ParserItem_marker&)last).marker==Markers::START) {
		parserItems.pop_back();
	} else if (last.type==ParserItemTypes::WORD) {
		ParserItem_marker* newEnd;
		newEnd=new ParserItem_marker(Markers::PERIOD);
		parserItems.push_back(newEnd);
	}

	/// SECOND PARSING RUN ///
	//TODO: Reduce indentations. Add helper functions and/or restructure.
        std::uint32_t lastCol, row;
	// Initialize lastCol with assumption that
	// the first parser item is a START marker.
	lastCol=(std::uint32_t)Markers::START;
	// First parser item has been accounted for,
	// so start iterator on second element.
	for (std::vector<ParserItem*>::iterator token=parserItems.begin()+1;
	     token!=parserItems.end(); ++token) {
		switch ((*token)->type) {
		case ParserItemTypes::MARKER:
			{
				ParserItem_marker& mark_pi=
					*((ParserItem_marker*)(*token));
				row=(unsigned int)mark_pi.marker;
				break;
			}
		case ParserItemTypes::WORD:
			{
				ParserItem_word& word_pi=
					*((ParserItem_word*)(*token));
				// Find row index of word in bigram array.
				row=0;
				std::vector<std::string>::iterator w;
				for (w=bigram_words.begin();
				     w!=bigram_words.end(); ++w) {
					if (word_pi.word == *w)
						break;
					row++;
				}
				// If word does not yet exist in bigram array,
				// add it.
				if (row >= bigram_words.size()) {
					bigram_words.push_back(word_pi.word);

					// Extend all rows.
					std::vector<std::vector<std::uint32_t>>::iterator r;
					for (r=bigram_array.begin();
					     r!=bigram_array.end(); ++r)
						r->push_back(0);

					// Add rows.
					std::vector<std::uint32_t> newRow(bigram_words.size(), 0);
					bigram_array.push_back(newRow);
					bigram_rowSums.push_back(0);
				}
				break;
			}
		default:
			std::cerr << "Error in Quoter::feed_stream: "
				  << "Unexpected error in second parsing run."
				  << std::endl;
		}

		bigram_array[lastCol][row]++;
		bigram_rowSums[lastCol]++;

		lastCol=row;
	}

	//Delete parser items.
	for (std::vector<ParserItem*>::iterator token=parserItems.begin();
	     token!=parserItems.end(); ++token)
		delete *token;
}

void Quoter::feed_file(std::string filePath) {
	std::ifstream ifs(filePath.c_str());

	if (!ifs.is_open()) {
		std::string m = "Error in Quoter::feed: Could not open ";
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

        std::uint32_t row = (unsigned int)Markers::START;
	std::uint32_t goal, sum, col;
	while (true) {
	        goal=(randGen() % bigram_rowSums[row]) + 1;
		sum=0, col=row;
		while (sum<goal) {
			col=(col+1)%bigram_array.size();
			sum += bigram_array[row][col];
		}

		if (col == (std::uint32_t)Markers::PERIOD) {
			sentence.back() = '.';
			break;
		} else if (col == (std::uint32_t)Markers::EXCLAIM) {
			sentence.back() = '!';
			break;
		} else if (col == (std::uint32_t)Markers::QUESTION) {
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

void Quoter::writeData(std::string filename) {
	std::ofstream out(filename);
	if (!out.is_open()) {
		std::string m="Error in Quoter::writeData: Cannot open file '";
		m+=filename;
		m+="' for writing";
		throw QuoterError(m);
	}

	// Write major and minor version.
	out << save_format.major << ' ' << save_format.minor << '\n';

	// Write word count.
	std::uint64_t wordCnt=bigram_array.size();
	out << wordCnt << '\n';

	// Write words.
	std::vector<std::string>::iterator word_it;
	for (word_it=bigram_words.begin();
	     word_it!=bigram_words.end(); ++word_it)
		out << *word_it << '\n';

	// Write array data.
	std::vector<std::vector<std::uint32_t>>::iterator row_it;
	std::vector<std::uint32_t>::iterator col_it;
	for (row_it=bigram_array.begin();
	     row_it!=bigram_array.end(); ++row_it)
		for (col_it=row_it->begin();
		     col_it!=row_it->end(); ++col_it)
			out << *col_it << '\n';
}

void Quoter::readData(std::string filename) {
	std::ifstream in(filename);
	if (!in.is_open()) {
		std::string m="Error in Quoter::readData: Cannot open file '";
		m+=filename;
		m+="' for reading";
		throw QuoterError(m);
	}

	std::uint64_t wordCnt, row, col;
	std::vector<std::vector<std::uint32_t>> newArray;
	std::vector<std::string> newWords;

	try {
		std::string buf;
		std::uint16_t major, minor;
		int state=0;
		size_t pos = 0;
		while (std::getline(in, buf)) {
			switch(state) {
			case 0:
				// Get major version number.
				major=std::stoi(buf, &pos);
				// Get minor version number.
				minor=std::stoi(buf.substr(pos + 1));
				if (major!=save_format.major || minor!=save_format.minor) {
					std::string m="Error in Quoter::readData: "
						"File format version is ";
					m+=std::to_string(major);
					m+=".";
					m+=std::to_string(minor);
					m+="; it should be ";
					m+=std::to_string(save_format.major);
					m+=".";
					m+=std::to_string(save_format.minor);
					throw QuoterError(m);
				}
				state++;
				break;
			case 1:
				// Get word count.
				wordCnt=std::stoi(buf);
				state++;
				row=1;
				break;
			case 2:
				// Get words.
				newWords.push_back(buf);
				if (row++==wordCnt) {
					row=0, col=0;
					state++;
				}
				break;
			case 3:
				// Get array data.
				if (col==0)
					newArray.push_back(std::vector<std::uint32_t>());
				newArray[row].push_back(std::stoi(buf));
				col++;
				if (col==wordCnt) {
					col=0;
					row++;
					if (row==wordCnt)
						state++;
				}
				break;
			default:
				// Too many lines.
				std::string m="Error in Quoter::readData: ";
				m+="Save file '";
				m+=filename;
				m+="' is corrupt: Too many lines";
				throw QuoterError(m);
			}
		}

		if (state<=3) {
			// Too few lines.
			std::string m="Error in Quoter::readData: ";
			m+="Save file '";
			m+=filename;
			m+="' is corrupt: Too few lines";
			throw QuoterError(m);
		}
        } catch(const std::logic_error& e) {
	        // Errors thrown by std::stoi.
		std::string m="Error in Quoter::readData: ";
		m+="Save file '";
		m+=filename;
		m+="' is corrupt: ";
		m+=e.what();
		throw QuoterError(m);
	}

	bigram_words=newWords;
	bigram_array=newArray;
	bigram_rowSums=std::vector<std::uint32_t>(wordCnt, 0);
	for (row=0;row<wordCnt;row++) {
		for (col=0;col<wordCnt;col++)
			bigram_rowSums[row]+=bigram_array[row][col];
	}
}

void Quoter::emitArray() {
	std::vector<std::vector<std::uint32_t>>::iterator row;
	std::vector<std::uint32_t>::iterator col;
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
		if (isalnum(*it) || (*it >= '#' && *it <='\'') ||
		    (*it == ',') || (*it == '-') || (*it == '@')) {
			filtered[i++] = *it;
		}
	}
	filtered[i] = '\0';

	return std::string(filtered);
}
