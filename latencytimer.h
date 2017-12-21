/*
 * latencytimer.h
 *
 *  Created on: Dec 12, 2017
 *      Author: laia
 */

#ifndef LATENCYTIMER_H_
#define LATENCYTIMER_H_

#include <string>
#include <unordered_map>
#include <memory>

using namespace std;

class Timer {
public:
	Timer(long long* _pTime):
		pTime(_pTime)
	{
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		auto finish = std::chrono::high_resolution_clock::now();
		long long nanoPassed = std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count();
		*pTime += nanoPassed;
	}
private:
	long long* pTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
};
class TimerFactory {
public:
	static TimerFactory& getInstance() {
		if (pInstance == nullptr) {
			pInstance.reset(new TimerFactory);
		}
		return *pInstance;
	}
	Timer createTimerInstance(const string key) {
		auto& ptr = timerMap[key];
		if (ptr == nullptr) {
			ptr.reset(new long long(0));
		}
		return Timer(ptr.get());
	}
	void printSummary() {
		cout << "Timer summary" << endl;
		for (auto& entry : timerMap) {
			cout << entry.first << "\t";
			cout << *entry.second / 1000  << endl;
		}
	}
	void clear() {
		timerMap.clear();
	}
private:
	TimerFactory() {}
	static unique_ptr<TimerFactory> pInstance;
	unordered_map<string, unique_ptr<long long> > timerMap;
};

unique_ptr<TimerFactory> TimerFactory::pInstance(new TimerFactory);
#endif /* LATENCYTIMER_H_ */
