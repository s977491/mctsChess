/*
 * config.h
 *
 *  Created on: Dec 13, 2017
 *      Author: laia
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include <memory>
#include "ThreadPool.h"

class Config {
public:
	static Config& getInstance() {
		static Config instance;
		return instance;
	}
	int getSearchTimes() {
		return searchTimes;
	}
	void setSearchTimes(int value) {
		searchTimes = value;
	}
	int getCPUCT() {
		return cPUCT;
	}
	void setCPUCT(int value) {
		cPUCT = value;
	}

	int getPoolSize() {
		return poolSize;
	}
	void setPoolSize(int value) {
		poolSize = value;
	}

	int getMaxCheckmateTimes() {
		return maxCheckmateTimes;
	}
	void setMaxCheckmateTimes(int value) {
		maxCheckmateTimes = value;
	}

	int getHumanSide() {
		return humanSide;
	}
	void setHumanSide(int value) {
		humanSide = value;
	}

	ThreadPool& getPool() {
		static ThreadPool instance(0);
		return instance;
	}

private:
	Config() {}
	int searchTimes = 600000;
	int cPUCT = 1000;
	int poolSize = 23000000;
	int humanSide = 1;
	int maxCheckmateTimes = 3;
//	int nodeSize = 23000000

	//unique_ptr<ThreadPool> pPool;
};


#endif /* CONFIG_H_ */
