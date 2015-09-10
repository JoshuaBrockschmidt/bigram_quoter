#ifndef QUOTER_H
#define QUOTER_H

#include <cstdint>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <vector>

class QuoterError: public std::exception {
public:
	QuoterError(std::string m);
	virtual const char* what() const throw();
private:
	std::string msg;
};

class Quoter {
public:
	Quoter();

	/* Feed a stream of coherent text into quoter for it to mimic.
	 *
	 * @param in Stream of coherent text.
	 */
	void feed_stream(std::istream& in);

	/* Feeds a file containing coherent text into a quoter for it to mimic.
	 *
	 * @param filePath Path to file containing coherent text.
	 */
	void feed_file(std::string filePath);

	/* Feed a string of coherent text into a quoter for it to mimic.
	 *
	 * @param text String of coherent text.
	 */
	void feed_string(std::string text);

	/* Builds a sentence based on the text fed into a quoter.
	 *
	 * @return A single sentence.
	 */
	std::string buildSentence();

	/* Writes quoter data to a file.
	 *
	 * @param filename Name of file to write to.
	 */
	void writeData(std::string filename);

	/* Read quoter data from file. This will overwrite data
	 * in a quoter if successful.
	 *
	 * @param filename Name of file to read from.
	 */
	void readData(std::string filename);

	/* Prints out a character representation of a quoter's bigram array.
	 */
	void emitArray();
private:
	const std::int16_t SAVEFORMAT_MAJOR=1;
	const std::int16_t SAVEFORMAT_MINOR=0;
	std::default_random_engine randGen;
	std::vector<std::vector<std::uint32_t>> bigram_array;
	std::vector<std::uint32_t> bigram_rowSums;
	std::vector<std::string> bigram_words;

	std::string filterWord(std::string& word);
};

#endif //QUOTER_H
