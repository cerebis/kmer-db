#pragma once
/*
This file is a part of Kmer-db software distributed under GNU GPL 3 licence.
The homepage of the Kmer-db project is http://sun.aei.polsl.pl/REFRESH/kmer-db

Authors: Sebastian Deorowicz, Adam Gudys, Maciej Dlugosz, Marek Kokot, Agnieszka Danek

*/

#include "kmc_file_wrapper.h"
#include "queue.h"
#include "filter.h"

#include <vector>
#include <memory>
#include <map>
#include <fstream>

// *****************************************************************************************
//
struct Task {
	size_t fileId;
	size_t threadId;
	std::string filePath;
	std::string sampleName;
	std::shared_ptr<InputFile> file;
	std::vector<kmer_t>* kmers;
	std::vector<uint32_t>* positions;
	uint32_t kmerLength;
	double fraction;

	// *****************************************************************************************
	//
	Task(size_t fileId, size_t threadId, const std::string& filePath) :
		fileId(fileId), threadId(threadId), filePath(filePath), file(nullptr), kmers(nullptr), positions(nullptr) {
	
		size_t pos = filePath.find_last_of("/\\");
		if (pos != string::npos) {
			sampleName = filePath.substr(pos + 1);
		}
		else {
			sampleName = filePath;
		}
	}
};

// *****************************************************************************************
//
class Loader {
public:
	
	size_t getCurrentFileId() const { return currentFileId; }

	Loader(std::shared_ptr<AbstractFilter> filter, InputFile::Format inputFormat, int _num_threads, bool storePositions = false);
	// *****************************************************************************************
	//
	~Loader() {
		readerQueue.MarkCompleted();
		prefetcherQueue.MarkCompleted();
		intermediateQueue.MarkCompleted();

		for (auto& t : readers) {
			t.join();
		}

		prefetcher.join();
	}

	void configure(const std::string& multipleKmcSamples);
	void initPrefetch();
	void waitForPrefetch() { prefetcherSemaphore.waitForZero();  }
	void initLoad();
	void waitForLoad() { readerSemaphore.waitForZero();  }

	std::map<size_t, std::shared_ptr<Task>>& getLoadedTasks() { return loadedTasks;  }

private:
	
	InputFile::Format inputFormat;

	uint32_t kmerLength;

	bool storePositions;

	size_t currentFileId;

	int numThreads;

	std::vector<std::string> kmcFileList;

	std::vector<std::vector<kmer_t>> kmersCollections;

	std::vector<std::vector<uint32_t>> positionsCollections;

	std::thread prefetcher;

	std::vector<std::thread> readers;

	Semaphore prefetcherSemaphore;

	Semaphore readerSemaphore;

	std::mutex outputMutex;

	CRegisteringQueue<std::shared_ptr<Task>> readerQueue;
	
	CRegisteringQueue<std::shared_ptr<Task>> prefetcherQueue;

	CRegisteringQueue<std::shared_ptr<Task>> intermediateQueue;

	std::map<size_t, std::shared_ptr<Task>> loadedTasks;
};