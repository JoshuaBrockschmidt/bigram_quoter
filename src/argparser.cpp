/* TODO
 *  - Check if bigram quoter has been fed before building a sentence.
 *  - Enforce a file extension (maybe .bq). If the file extension is not
 *    included with a savefile string, it will be appended automatically.
 *  - Add command for manually saving stashed bigram quoters.
 *    Don't save them automatically.
 *  - Finish merge command.
 */

#include <algorithm>
#include <exception>
#include <vector>
#include <unistd.h>
#include "argparser.hpp"

const char* opts_string="stn:o:l:m:f:b";

void ArgParser::parseArgs(int argc, char** argv) {
	if (argc == 1) {
		std::cerr << argv[0] << ": no arguments given\n" << std::endl;
		#include "showhelp.h"
		std::cerr <<
			"\nSee https://github.com/JoshuaBrockschmidt/bigram_quoter"
			"\n" << std::endl;
	        exit(1);
	}
	std::vector<std::pair<Quoter, std::string>> stash;
	bool strictMode=false, strictMode_exit=false;
	int o, argi=1;
	while ((o=getopt_long(argc, argv, opts_string, opts_long, &argi)) != -1) {
		switch(o) {
		case 's':
			// Switch to strict mode.
			strictMode=true;
			break;
		case 't':
			// Switch to tolerant mode.
			strictMode=false;
			break;
		case 'n':
			// Create and save a new bigram quoter
			// and add it to the queue.
			option_new(argc, argv, stash,
				   strictMode, strictMode_exit);
			break;
		case 'o':
			// Create a new bigram quoter with the same as an
			// existing bigram quoter.
			option_overwrite(argc, argv, stash,
					 strictMode, strictMode_exit);
			break;
		case 'l':
			// Load a bigram quoter and add it to the queue.
			option_load(argc, argv, stash,
				    strictMode, strictMode_exit);
			break;
		case 'm':
			// Merge bigram quoters in queue into one or more
			// new quoter and add them to the queue.
			option_merge(argc, argv, stash,
				     strictMode, strictMode_exit);
			break;
		case 'f':
			// Feed one or more text file into
			// the queued bigram quoters.
			option_feed(argc, argv, stash,
				    strictMode, strictMode_exit);
			break;
		case 'b':
			// Construct/build one sentence for each
			// queued bigram quoter.
			option_build(argc, argv, stash,
				     strictMode, strictMode_exit);
			break;
		default:
			break;
		}

		if (strictMode_exit) {
			std::cerr << argv[0]
				  << ": exiting as per strict mode"
				  << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// Write stashed bigram quoters to their respective save files.
	std::vector<std::pair<Quoter, std::string>>::iterator s_it;
        for (s_it=stash.begin(); s_it!=stash.end(); ++s_it)
		s_it->first.writeData(s_it->second);
}

void ArgParser::option_new(int argc, char** argv,
			   std::vector<std::pair<Quoter, std::string>>& stash,
			   bool strictMode, bool& strictMode_exit) {
	std::string filename(optarg);
	if (access(optarg, F_OK) != -1 || filenameInStash(stash, filename)) {
		std::cerr << argv[0]
			  << ": cannot create '"
			  << filename
			  << "'; it already exists"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	std::pair<Quoter, std::string> newPair(Quoter(), filename);
	stash.push_back(newPair);
}

void ArgParser::option_overwrite(int argc, char** argv,
				 std::vector<std::pair<Quoter, std::string>>& stash,
				 bool strictMode, bool& strictMode_exit) {
	std::string filename(optarg);
	if (access(optarg, F_OK) == -1 || filenameInStash(stash, filename)) {
		std::cerr << argv[0]
			  << ": cannot overwrite '"
			  << filename
			  << "'; it doesn't exist"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	std::pair<Quoter, std::string> newPair(Quoter(), filename);
	stash.push_back(newPair);
}

void ArgParser::option_load(int argc, char** argv,
			    std::vector<std::pair<Quoter, std::string>>& stash,
			    bool strictMode, bool& strictMode_exit) {
	std::string filename(optarg);
	if (filenameInStash(stash, filename)) {
		std::cerr << argv[0]
			  << ": cannot load '"
			  << filename
			  << "'; it has already been stashed"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	if (access(optarg, F_OK) == -1) {
		std::cerr << argv[0]
			  << ": cannot load '"
			  << filename
			  << "'; it doesn't exist"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	try {
		Quoter newQuoter;
		newQuoter.readData(filename);
		std::pair<Quoter, std::string> newPair(newQuoter, filename);
		stash.push_back(newPair);
	} catch (QuoterError& e) {
		std::cerr << argv[0]
			  << ": cannot load quoter data: "
			  << e.what()
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
	}
}

void ArgParser::option_merge(int argc, char** argv,
			     std::vector<std::pair<Quoter, std::string>>& stash,
			     bool strictMode, bool& strictMode_exit) {
	std::string filename(optarg);
	if (access(optarg, F_OK) != -1 || filenameInStash(stash, filename)) {
		std::cerr << argv[0]
			  << ": cannot create '"
			  << filename
			  << "'; it already exists"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	//TODO
	return;
}

void ArgParser::option_feed(int argc, char** argv,
			    std::vector<std::pair<Quoter, std::string>>& stash,
			    bool strictMode, bool& strictMode_exit) {
	std::string filename(optarg);
	if (access(optarg, F_OK) == -1) {
		std::cerr << argv[0]
			  << ": cannot feed '"
			  << filename
			  << "' to bigram quoters; it doesn't exist"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	std::vector<std::pair<Quoter, std::string>>::iterator s_it;
	for (s_it=stash.begin(); s_it!=stash.end(); ++s_it) {
		try {
			s_it->first.feed_file(filename);
		} catch (QuoterError& e) {
			std::cerr << argv[0]
				  << ": cannot feed to '"
				  << s_it->second
				  << "': "
				  << e.what();
		}
	}
}

void ArgParser::option_build(int argc, char** argv,
			     std::vector<std::pair<Quoter, std::string>>& stash,
			     bool strictMode, bool& strictMode_exit) {
	if (stash.empty()) {
		std::cerr << argv[0]
			  << ": cannot build sentences; stash is empty"
			  << std::endl;
		if (strictMode)
			strictMode_exit = true;
		return;
	}
	std::vector<std::pair<Quoter, std::string>>::iterator s_it;
	for (s_it=stash.begin(); s_it!=stash.end(); ++s_it)
		std::cout << s_it->first.buildSentence() << std::endl;
}

bool ArgParser::filenameInStash(std::vector<std::pair<Quoter, std::string>>& stash,
				const std::string& filename) {
	std::vector<std::pair<Quoter, std::string>>::iterator s_it;
	for (s_it=stash.begin(); s_it!=stash.end(); ++s_it)
		if (s_it->second == filename)
			return true;
	return false;
}
