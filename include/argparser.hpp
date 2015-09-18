#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>
#include <vector>
#include <getopt.h>
#include "quoter.hpp"

namespace ArgParser {
	const struct option opts_long[]={
		{"strict",    no_argument,       NULL, 's'},
		{"tolerant",  no_argument,       NULL, 't'},
		{"new",       required_argument, NULL, 'n'},
		{"overwrite", required_argument, NULL, 'o'},
		{"load",      required_argument, NULL, 'l'},
		{"merge",     required_argument, NULL, 'm'},
		{"feed",      required_argument, NULL, 'f'},
		{"build",     no_argument,       NULL, 'b'},
		{0, 0, 0, 0}
	};

	void parseArgs(int argc, char **argv);
	void option_new(int argc, char **argv,
			std::vector<std::pair<Quoter, std::string>>& stash,
		        bool strictMode, bool& strictMode_exit);
	void option_load(int argc, char **argv,
			 std::vector<std::pair<Quoter, std::string>>& stash,
			 bool strictMode, bool& strictMode_exit);
	void option_overwrite(int argc, char **argv,
			 std::vector<std::pair<Quoter, std::string>>& stash,
			 bool strictMode, bool& strictMode_exit);
	void option_merge(int argc, char **argv,
			  std::vector<std::pair<Quoter, std::string>>& stash,
			  bool strictMode, bool& strictMode_exit);
	void option_feed(int argc, char **argv,
			 std::vector<std::pair<Quoter, std::string>>& stash,
			 bool strictMode, bool& strictMode_exit);
	void option_build(int argc, char **argv,
			  std::vector<std::pair<Quoter, std::string>>& stash,
			  bool strictMode, bool& strictMode_exit);
        bool filenameInStash(std::vector<std::pair<Quoter, std::string>>& stash,
			     const std::string& filename);
}

#endif //ARGPARSER_H
