// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <memory>
#include <array>
#include "Chess.h"
#include "MCTS.h"
#include "latencytimer.h"
#include "config.h"

using namespace std;
using namespace std::placeholders;
board_t board{ {
	{ 2, 3, 6, 5, 1, 5, 6, 3, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 4, 0, 0, 0, 0, 0, 4, 0 },
	{ 7, 0, 7, 0, 7, 0, 7, 0, 7 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{-7, 0,-7, 0,-7, 0,-7, 0,-7 },
	{ 0,-4, 0, 0, 0, 0, 0,-4, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{-2,-3,-6,-5,-1,-5,-6,-3, -2 }
} };

void defaultConfiger(int argc, const char* argv[], int index, const char* title,  function<int()> getter, function<void(int)> setter) {
	int defaultValue = getter();
	if (argc > index) {
		defaultValue = atoi(argv[index]);
	}
	cout << "use " << title << ": " <<  defaultValue << endl;
	setter(defaultValue);
}

int main(int argc, const char* argv[]) {

	// enqueue and store future
	cout << "Thread concurrency:" << std::thread::hardware_concurrency() << endl;
//	thread t1([] {svc.run();});
	//std::bind(&boost::asio::io_service::run, &svc)));
//	thread t2(std::bind(&boost::asio::io_service::run, &svc)));
//	thread t3(std::bind(&boost::asio::io_service::run, &svc)));
//	thread t4(std::bind(&boost::asio::io_service::run, &svc)));
	defaultConfiger(argc, argv, 1, "searchTimes",
				[]() { return Config::getInstance().getSearchTimes(); } ,
				bind(&Config::setSearchTimes, &(Config::getInstance()), _1)
				);

	defaultConfiger(argc, argv, 2, "cPUCT",
					[]() { return Config::getInstance().getCPUCT(); } ,
					[](int value) { Config::getInstance().setCPUCT(value); });

	defaultConfiger(argc, argv, 3, "poolsize",
					[]() { return Config::getInstance().getPoolSize(); } ,
					[](int value) { Config::getInstance().setPoolSize(value); });

	cout << "thread actual size: " << Config::getInstance().getPool().getSize() << endl;

	Chess chess;
	Chess qChess;
	chess.init(board);
	auto rootNode = MCTSNode::rootNode(chess, 1);
	MCTSNode* curRoot = rootNode;
	MCTS trainer;
	string cmd = "";
	bool suggested = false;
	cout << rootChess.toString() << endl;
	int side = 1;
	int humanFlag = 1;
	MCTSNode* qNode;
	bool print = true;
	while (cmd != "q") {
		Move mov;
		qChess = rootChess;
		if (!suggested) {
			cout << "size running at  " << PoolMgr::getInstance().size() <<endl;
			mov = trainer.suggestMove(curRoot, humanFlag != side);
			cout << "size running at  " << PoolMgr::getInstance().size() <<endl;

			if (humanFlag != side)
				cout << "computer suggest mov: " << mov.toString() << endl;
			else {
				cout << "your turn!" << endl;
			}
			suggested = true;
			qNode = curRoot;
			if (print)
				qNode->printChildrenSummary();
		}
		cin >> cmd;
		if (cmd.length() == 4) {
			tupleKey_t cmdMove = make_tuple(cmd[0] - '0', cmd[1] - '0', cmd[2] - '0', cmd[3] - '0');
			auto iter = curRoot->findChild(cmd[0] - '0', cmd[1] - '0', cmd[2] - '0', cmd[3] - '0');
			if (iter !=nullptr) {
				suggested = false;
//				MCTSNode* parentRoot = curRoot;
				curRoot = iter;

				if (!curRoot->isExpanded())
					trainer.treeSearch(curRoot);
				curRoot->moved = true;
				chess.apply(curRoot->move);
				cout << chess.toString() << endl;
				side = -side;
				PoolMgr::getInstance().free();

				curRoot = MCTSNode::rootNode(chess, side);
				cout << "after erase size running at  " << PoolMgr::getInstance().size() <<endl;

			}
			else {
				cout << "bad move" << endl;
			}
		}
		else if (cmd.length() > 4) {
			tupleKey_t cmdNode = make_tuple(cmd[0] - '0', cmd[1] - '0', cmd[2] - '0', cmd[3] - '0');
			if (cmd[4] == 'g') {
				auto iter = curRoot->findChild(cmd[0] - '0', cmd[1] - '0', cmd[2] - '0', cmd[3] - '0');
				if (iter != nullptr) {
					qNode = iter;
					qNode->printChildrenSummary();
					qChess.apply(qNode->move);
					qChess.flip();

				}
			}
		}
		else if (cmd == "b") {
			if (qNode != curRoot) {
				qChess.flip();
				qChess.rollback(qNode->move);
				qNode = qNode->parent;
				qNode->printChildrenSummary();
			}
		}
		else if (cmd == "p") {
			cout << qChess.toString() << endl;
		}
	}
	return 0;
}
