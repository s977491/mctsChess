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

	ThreadPool& getPool() {
		static ThreadPool instance(poolSize);
		return instance;
	}

private:
	Config() {}
	int searchTimes = 1000;
	int cPUCT = 250;
	int poolSize = 2;

	//unique_ptr<ThreadPool> pPool;
};


#endif /* CONFIG_H_ */
