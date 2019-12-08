#pragma once
/*
File:	UltraGrep.hpp
Author:	Matthew Wrobel
Date:	2019-12-05
Brief:	Header declaration for UltraGrep class and associated structures
*/

#include <string>
#include <deque>
#include <queue>
#include <vector>
#include "SocketServer.hpp"

typedef std::deque<std::pair<std::string, std::pair<unsigned, unsigned>>> OccurrenceSet;

struct GrepFileInfo {
	std::string filePath;
	OccurrenceSet occSet;
	GrepFileInfo() {}
	GrepFileInfo(std::string fp, OccurrenceSet ol) : filePath(fp), occSet(ol) {}
	struct GrepSort {
		inline bool operator() (const GrepFileInfo& i, const GrepFileInfo& j) { return i.filePath < j.filePath; }
	};
};

bool PerformUltraGrep(int argc, std::vector<std::string> argv, SocketServer &sockServ);