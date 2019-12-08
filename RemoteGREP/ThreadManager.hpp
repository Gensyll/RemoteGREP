#pragma once
/*
File:	ThreadManager.hpp
Author:	Matthew Wrobel
Date:	2019-12-05
Brief:	Header declaration for ThreadManager class
*/

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <filesystem>

#include "UltraGrep.hpp"
#include "SocketServer.hpp"

typedef std::vector<std::thread> ThreadPool;
typedef std::queue<std::filesystem::path> FSPathQueue;

class ThreadManager {
private:
	SocketServer& socketRef;

	unsigned nThreads;
	ThreadPool threadPool;

	//Task processing
	FSPathQueue taskList;
	std::mutex taskMutex;
	bool processTaskList;
	std::deque<GrepFileInfo> taskResultList;

	const std::string userQuery;
	std::list<std::string> extList;

	//Thread notification
	std::mutex wakeMutex;
	std::condition_variable wakeCondition;

	//Barrier
	unsigned barrierThreshold;
	unsigned barrierCount;
	bool barrierCollapsed;
	std::mutex barrierMutex;
	std::condition_variable barrierCondition;

	//Output
	bool verbose;
public:
	std::mutex outputMutex;
public:
	ThreadManager(unsigned, const std::string&, const std::string&, bool, SocketServer&);
	~ThreadManager();

	void StopThreads();
	void TriggerBarrier();
	void AddTask(std::string);
	void AddTaskResult(GrepFileInfo);
	void ProcessTask();

	inline void NotifyThreadPool() { wakeCondition.notify_one(); }
	inline FSPathQueue GetTaskList() { return taskList; }
	inline std::deque<GrepFileInfo>& GetTaskResults() { return taskResultList; }
};