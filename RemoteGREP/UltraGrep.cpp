/*
File:	UltraGrep.cpp
Author:	Matthew Wrobel
Date:	2019-12-05
Brief:	Implementation of UltraGrep driver method
*/

#include "ThreadManager.hpp"
#include "UltraGrep.hpp"
#include "SocketServer.hpp"
#include <sstream>
#include <string>

using namespace std;

bool PerformUltraGrep(int argc, vector<string> argv, SocketServer sockServ) {
	bool verboseOutput = false;
	string initPath = filesystem::current_path().string();
	string userQuery;
	string extList = ".txt";
	deque<GrepFileInfo> tResultSet;

	try {
		LARGE_INTEGER timeStart, timeStop, frequency;
		QueryPerformanceCounter(&frequency);

		sockServ.SendToClient("Project 2: UltraGrep (Matthew Wrobel 2019)");

		//Process parameters
		if (argc < 3 || argc > 5) {
			throw exception("Invalid number of parameters. Usage: grep [-v(erbose)] folderPath find [extension]*");
		}
		if (strcmp(argv[1].c_str(), "-v") == 0 || strcmp(argv[1].c_str(), "-verbose") == 0) {
			verboseOutput = true;
			if (strcmp(argv[2].c_str(), ".") != 0) {
				initPath = argv[2];
			}
			userQuery = argv[3];
			if (argc > 4) {
				extList = argv[4];
			}
		}
		else {
			if (strcmp(argv[1].c_str(), ".") != 0) {
				initPath = argv[1];
			}
			userQuery = argv[2];
			if (argc > 3) {
				extList = argv[3];
			}
		}
#ifdef _DEBUG
		stringstream ss;
		ss << endl << L"Verbose:" << (verboseOutput ? L"Enabled" : L"Disabled") << L"\ninitPath:" << initPath << L"\nuserQuery:" << userQuery << L"\nextList:" << extList << endl << endl;
		sockServ.SendToClient(ss.str());
#endif

		//Initialize threads
		ThreadManager tManager(thread::hardware_concurrency(), userQuery, extList, verboseOutput, sockServ);
		tManager.TriggerBarrier();
		QueryPerformanceCounter(&timeStart);

		//Establish initial directory iterator
		for (filesystem::directory_entry currPath : filesystem::directory_iterator(initPath)) {
			tManager.AddTask(currPath.path().string());
			tManager.NotifyThreadPool();
		}

		//Result
		tManager.StopThreads();
		tResultSet = tManager.GetTaskResults();
		QueryPerformanceCounter(&timeStop);

		{
			lock_guard<mutex> outLock(tManager.outputMutex);
			unsigned totalMatchCount = 0, totalMatchedFiles = 0;
			sort(tResultSet.begin(), tResultSet.end(), GrepFileInfo::GrepSort());

			stringstream ss;
			ss << endl << endl << "== UltraGREP Results Summary == " << endl << endl;
			for (GrepFileInfo res : tResultSet) {
				if (res.occSet.size() > 0) {
					++totalMatchedFiles;
					ss << (int)res.occSet.size() << " Matches found in " << res.filePath << endl;					
					for (auto pair : res.occSet) {
						string printSpice = (pair.second.second > 1 ? "Occurences" : "Occurence");						
						ss << "[Line " << pair.second.first << ":" << pair.second.second << " " << printSpice << "]\t" << pair.first << endl;
						totalMatchCount += pair.second.second;
					}
					ss << endl;
				}
			}
			ss << "Total matches files: " << totalMatchedFiles << endl;
			ss << "Total number of matches: " << totalMatchCount << endl;
			ss << "Scan duration: " << ((timeStop.QuadPart - timeStart.QuadPart) / double(frequency.QuadPart)) << endl;
			sockServ.SendToClient(ss.str());
		}
	}
	catch (exception ex) {
		stringstream ss;
		ss << "PerformUltraGrep() failed: " << ex.what() << endl;
		sockServ.SendToClient(ss.str());
		return false;
	}
	return true;
}