/*
File:	ThreadManager.cpp
Author:	Matthew Wrobel
Date:	2019-12-05
Brief:	Implementation of ThreadManager class Header
*/

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <algorithm>

#include "ThreadManager.hpp"

using namespace std;

ThreadManager::ThreadManager(unsigned numThreads, const string& uQuery, const string& eList, bool verb, SocketServer &sockServ) : userQuery(uQuery), socketRef(sockServ) {
	verbose = verb;
	nThreads = numThreads;
	barrierThreshold = nThreads + 1;
	barrierCount = barrierThreshold;
	barrierCollapsed = false;
	processTaskList = true;

	//Process extensions
	string token;
	stringstream ss(eList);
	while (getline(ss, token, '.')) {
		extList.push_back('.' + token);
	}

	//Create threads
	for (unsigned i = 0; i < nThreads; ++i) {
		threadPool.push_back(thread(&ThreadManager::ProcessTask, this));
	}
}

ThreadManager::~ThreadManager() {
	StopThreads();
}

void ThreadManager::StopThreads() {
	processTaskList = false;
	wakeCondition.notify_all();
	for (thread& t : threadPool) {
		if (t.joinable())
			t.join();
	}
}

void ThreadManager::TriggerBarrier() {
	unique_lock<mutex> barLock(barrierMutex);
	if (--barrierCount == 0) {
		barrierCollapsed = true;
		barrierCondition.notify_all();
		{	lock_guard<mutex> outLock(outputMutex);
			stringstream ss;
			ss << "[" << this_thread::get_id() << "]\tBarrier collapsed." << endl;
			socketRef.SendToClient(ss.str());
		}
	}
	else {
		{	lock_guard<mutex> outLock(outputMutex);
			stringstream ss;
			ss << "[" << this_thread::get_id() << "]\tBarrier triggered." << endl;
			socketRef.SendToClient(ss.str());
		}
		while (barrierCollapsed == false)
			barrierCondition.wait(barLock);
	}
}

void ThreadManager::AddTask(string task) {
	{
		lock_guard<mutex> tLock(taskMutex);
		taskList.push(task);
	}
}

void ThreadManager::AddTaskResult(GrepFileInfo item) {
	{
		lock_guard<mutex> tLock(taskMutex);
		taskResultList.push_back(item);
	}
}

void ThreadManager::ProcessTask() {
	{
		lock_guard<mutex> outLock(outputMutex);
		stringstream ss;
		ss << "[" << this_thread::get_id() << "]\tThread launching." << endl;
		socketRef.SendToClient(ss.str());
	}
	TriggerBarrier();

	while (processTaskList) {
		{
			unique_lock<mutex> wakeLock(wakeMutex);
			wakeCondition.wait(wakeLock);
		}
		while (!taskList.empty()) {
			filesystem::path taskFilePath;
			bool acquiredTask = false;
			{
				lock_guard<mutex> tLock(taskMutex);
				if (!taskList.empty()) {
					taskFilePath = taskList.front();
					taskList.pop();
					acquiredTask = true;
				}
			}

			if (acquiredTask) {
				//Determine GREP or Scan
				if (taskFilePath.has_extension() && (find(extList.begin(), extList.end(), taskFilePath.extension()) != extList.end())) {
					//Grep the file
					if (verbose) {
						{
							lock_guard<mutex> outLock(outputMutex);
							stringstream ss;
#ifdef _DEBUG
							ss << "[" << this_thread::get_id() << "]\t";
#endif
							ss << "Grepping: " << taskFilePath << endl;
							socketRef.SendToClient(ss.str());
						}
					}

					ifstream ifile(taskFilePath);
					unsigned lineNumber = 1;
					string currLine;
					if (ifile.is_open()) {
						OccurrenceSet occSet;
						while (getline(ifile, currLine)) {
							smatch matchList;
							unsigned matchOccurs = 0;
							string regIn = currLine;
							while (regex_search(regIn, matchList, regex(userQuery.c_str()))) {
								++matchOccurs;
								regIn = matchList.suffix();
							}
							if (matchOccurs > 0)
								occSet.push_back({ currLine, {lineNumber, matchOccurs } });
							++lineNumber;
						}
						//Display results
						if (occSet.size() > 0 && verbose) {
							{
								lock_guard<mutex> outLock(outputMutex);
								stringstream ss;
#ifdef _DEBUG
								ss << "[" << this_thread::get_id() << "]\t";
#endif
								ss << "Match found in: " << taskFilePath << endl;
								socketRef.SendToClient(ss.str());
							}

							taskResultList.push_back(GrepFileInfo(taskFilePath.string(), occSet));
						}
					}
					else if (filesystem::is_directory(taskFilePath)) {
						//Scan the directory
						for (filesystem::directory_entry currPath : filesystem::directory_iterator(taskFilePath)) {
							if (verbose) {
								{
									lock_guard<mutex> outLock(outputMutex);
									stringstream ss;
#ifdef _DEBUG
									ss << "[" << this_thread::get_id() << "]\t";
#endif
									ss << "Scanning: " << taskFilePath << endl;
									socketRef.SendToClient(ss.str());
								}
							}
							AddTask(currPath.path().string());
							NotifyThreadPool();
						}
					}
				}
			}
		}
	}
}