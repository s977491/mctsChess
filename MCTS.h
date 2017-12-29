/*
* MCTS.h
*
*  Created on: Dec 10, 2017
*      Author: laia
*/

#ifndef MCTS_H_
#define MCTS_H_

#include <memory>
#include "Chess.h"
#include <array>
#include <stack>
#include <unordered_map>
#include <tuple>
#include <cmath>
#include <climits>
#include <chrono>
#include "latencytimer.h"
#include "config.h"
#include <boost/pool/object_pool.hpp>
#include <cstdlib>
#include <ctime>

const double DefaultProb = 0.01;
using namespace std;

enum ChessResult {
	Playing,
	Won,
	NoMove,
	Draw,
	Error
};
struct key_hash : public unary_function<tupleKey_t, size_t>
{
	size_t operator() (const tupleKey_t& k) const
	{
		return (get<0>(k) << 12) +
			(get<1>(k) << 8) +
			(get<2>(k) << 4) +
			(get<3>(k));
	}
};
struct key_equal : public binary_function<tupleKey_t, tupleKey_t, bool>
{
	bool operator ()(const tupleKey_t& lhs, const tupleKey_t& rhs) const
	{
		return (
			get<0>(lhs) == get<0>(rhs) &&
			get<1>(lhs) == get<1>(rhs) &&
			get<2>(lhs) == get<2>(rhs) &&
			get<3>(lhs) == get<3>(rhs)
			);
	}
};

array<double, 8> scorex = { 0, 1250, 2500, 5000, 10000, 20000, 30000, 100000 };
array<double, 8> scorey = { 0, 0.1, 0.2, 0.35, 0.55, 0.825, 0.9, 1};


//array<double, 8> scorex = { 0, 1250, 2500, 5000, 10000, 20000, 30000, 100000 };
//array<double, 8> scorey = { 0, 0.05, 0.15, 0.35, 0.7, 0.8, 0.9, 1};
//array<double, 8> scorex = { 0,  20000, 30000, 100000 };
//array<double, 8> scorey = { 0,  0.8, 0.9, 1};
double calYv(int retScore) {
	int sign = 1;
	if (retScore < 0) {
		sign = -1;
		retScore = -retScore;
	}
	for (size_t i = 1; i < scorex.size() ; ++i) {
		if (retScore < scorex[i]) {
			return ( scorey[i - 1] + (retScore - scorex[i - 1])
				/ (scorex[i] - scorex[i - 1])
				* (scorey[i] - scorey[i - 1]) )* sign;
		}
	}
	return 1 * sign;
}
double focusProbCal(double ateScore) {
	double retScore = calYv(ateScore) * 100 * DefaultProb;
	return retScore;
}

template <class T>
class SharedPool
{
 private:
  struct External_Deleter {
    explicit External_Deleter(std::weak_ptr<SharedPool<T>* > pool)
        : pool_(pool) {}

    void operator()(T* ptr) {
      if (auto pool_ptr = pool_.lock()) {
        try {
        	ptr->uninit();
          (*pool_ptr.get())->add(std::unique_ptr<T>{ptr});
          return;
        } catch(...) {}
      }
      std::default_delete<T>{}(ptr);
    }
   private:
    std::weak_ptr<SharedPool<T>* > pool_;
  };

 public:
  using ptr_type = std::unique_ptr<T, External_Deleter >;

  SharedPool() : this_ptr_(new SharedPool<T>*(this)) {}
  virtual ~SharedPool(){}

  void add(std::unique_ptr<T> t) {
    pool_.push(std::move(t));
  }

  ptr_type getNullptr() {
	  ptr_type tmp(nullptr,
					   External_Deleter{std::weak_ptr<SharedPool<T>*>{this_ptr_}});
	  return std::move(tmp);
	}
  ptr_type acquire() {
    assert(!pool_.empty());
    ptr_type tmp(pool_.top().release(),
                 External_Deleter{std::weak_ptr<SharedPool<T>*>{this_ptr_}});
    pool_.pop();
    return std::move(tmp);
  }

  bool empty() const {
    return pool_.empty();
  }

  size_t size() const {
    return pool_.size();
  }

 private:
  std::shared_ptr<SharedPool<T>* > this_ptr_;
  std::stack<std::unique_ptr<T> > pool_;
};

class MCTSNode;

class PoolMgr {
public:
	static PoolMgr& getInstance() {
		static PoolMgr inst;
		return inst;
	}

//	SharedPool<MCTSNode>::ptr_type acquire() {
//		if (pool.empty())
//			return pool.getNullptr();
//		return pool.acquire();
//	}
	MCTSNode* acquire() {
		if (nextIndex >= vPool.size())
			return nullptr;
		auto ptr = &vPool[nextIndex++];
		return ptr;
	}
	size_t size() const {
	    return vPool.size() -nextIndex ;
	  }
	void free() {
		nextIndex = 0;
	}
private:
	PoolMgr();
	vector<MCTSNode> vPool;
	size_t nextIndex = 0;
	//SharedPool<MCTSNode> pool;
};

Chess rootChess;
Chess curChess;

class MCTSNode {
public:
	//typedef unordered_map<tupleKey_t, MCTSNode*, key_hash, key_equal> nodeMap_t;
	typedef list< MCTSNode*> nodeMap_t;

	static MCTSNode* rootNode(Chess& chess, int side, int beingCheckMatedCnt, int firedCheckMateCnt, int beingAttackerId, int firedAttackerId) {
		auto pRetNode = PoolMgr::getInstance().acquire();
		pRetNode->init(nullptr, Move(), 0);

		rootChess = chess;
		//rootChess.init();
		rootChess.recalBaseScore();
//		if (side == -1)
//			rootChess.flip();
		pRetNode->beingCheckMatedCnt = beingCheckMatedCnt;
		pRetNode->firedCheckMateCnt = firedCheckMateCnt;
		pRetNode->beingAttackerId = beingAttackerId;
		pRetNode->firedAttackerId = firedAttackerId;
		curChess = rootChess;
		cout << "score:" <<rootChess.score << endl;
		//auto ret = Chess::getCalValue(rootChess);
		auto ret = pair<int, Move>(0, Move());
		pRetNode->moved = true;
		if (ret.first > 2000)
			pRetNode->expand(&ret.second, focusProbCal(ret.first));
		else
			pRetNode->expand(nullptr, 0);

		pRetNode->N = 1;
		return pRetNode;
	}
	int N = 0;
	double Q = 0, U = 0, W = 0;
	int N2 = 0;
	bool expanded;
	bool moved = false;
	ChessResult done = Playing;
	int side = 1;
	//Chess chess;
	nodeMap_t children;
	MCTSNode* parent;
	Move move;
	double prior;

	int beingCheckMatedCnt;
	int firedCheckMateCnt;
	int beingAttackerId;
	int firedAttackerId;
private:
	double c_PUCT;

public:
	MCTSNode()
		: c_PUCT(Config::getInstance().getCPUCT()/100.0)
	{
	}
	MCTSNode* findChild(int y1, int x1, int y2, int x2) {
		for (auto ptr : children) {
			auto& mov = ptr->move;
			if (mov.y1 == y1 && mov.x1 == x1 && mov.y2 == y2 && mov.x2 == x2)
				return ptr;
		}
		return nullptr;
	}
	void init(MCTSNode* _parent, const Move& _move, double _prior)
	{
		parent = _parent;
		move = _move;
		prior = _prior;

		N = 0;
		Q = 0, U = 0, W = 0;
		N2 = 0;

		if (parent != nullptr) {
			Q = -parent->Q;
			W = -parent->Q;
			side = -parent->side;
		}
		expanded = false;
		moved = false;
		done = Playing;
		children.clear();
		beingCheckMatedCnt = 0;
		firedCheckMateCnt = 0;
		firedAttackerId = -1;
		beingAttackerId = -1;
	}


	ChessResult expand(Move* pFocusMove, double focusProb) {
		expanded = true;
		ChessResult won = Won;


		list<Move> moveList;
		{
//			auto timerInstance = TimerFactory::getInstance().createTimerInstance("getPossibleMove");
			moveList = curChess.getPossibleMove();
		}
		Move winMove;
		{
//			auto timerInstance = TimerFactory::getInstance().createTimerInstance("getWinMove");
			if (!curChess.getWinMove(moveList, winMove)) {
				won = Playing;
				if (firedCheckMateCnt >=  Config::getInstance().getMaxCheckmateTimes()) { //cannot checkmate more than 3 times)
					//remove all move that result in checkMate
					auto iter = moveList.begin();
					while (iter != moveList.end()) {
						bool needErase = false;
						curChess.apply(*iter);
						curChess.flip();
						int attacker = -1;
						if (curChess.isCheckmated(attacker)) {
							if (attacker == firedAttackerId )
								needErase = true;
						}
						curChess.flip();
						curChess.rollback(*iter);

						if (needErase)
							moveList.erase(iter++);
						else
							++iter;
					}
				}
			}
			else {
				moveList.clear();
				moveList.push_back(winMove);
				pFocusMove = nullptr;
			}
		}
//		auto timerInstance = TimerFactory::getInstance().createTimerInstance("expand remains");
		if (moveList.empty())
		{
			won = NoMove;
			done = NoMove;
			return NoMove;
		}
//		double sumProb = 0;

		double avgProb = DefaultProb;
		tupleKey_t focusMovKey;
		if (pFocusMove != nullptr && focusProb > avgProb) {
//			if (focusProb > 1)
//				focusProb = 1;
//			avgProb = (1 - focusProb) / moveList.size();
			focusMovKey = pFocusMove->getTupleKey();
		}
		for (const auto& mov : moveList) {
			const auto& movKey = mov.getTupleKey();
			double prob = avgProb;
			if (focusMovKey == movKey) {
				prob = focusProb;
			}
			auto pChildNode = PoolMgr::getInstance().acquire();
			pChildNode->init(this, mov, prob);
			pChildNode->firedCheckMateCnt = this->beingCheckMatedCnt;
			pChildNode->firedAttackerId = this->beingAttackerId;

			if (won == Won) {
				pChildNode->done = Won;
			}
			this->children.push_back(
				(pChildNode));
		}
		return won;
	}
	bool isExpanded() {
		return expanded;
	}
//	vector<MCTSNode*> selectLeaves(int num) {
//		vector<MCTSNode*> ret;
//		MCTSNode* cur = this;
//		if (done != Playing) {
//			cerr << "strange that root already done!, no solution!" << endl;
//			return ret;
//		}
//		while (cur->isExpanded()) {
//			double maxScore = INT_MIN;
//			MCTSNode* maxPtr = nullptr;
//			for (auto& entry : cur->children) {
//				auto ptr = entry.second.get();
//				ptr->calU();
//				double score = ptr->actionScore();
//				if (score > maxScore) {
//					maxScore = score;
//					maxPtr = ptr;
//				}
//			}
//			if (maxPtr == nullptr ) {
//				return cur;
//			}
//			cur = maxPtr;
//			if (maxPtr->done != Playing) {
//				return cur;
//			}
//		}
//		return cur;
//	}
	MCTSNode* selectLeaf() {
		curChess = rootChess;
		MCTSNode* cur = this;
		if (done != Playing) {
			cerr << "strange that root already done!, no solution!" << endl;
			return this;
		}
		while (cur->isExpanded()) {
			double maxScore = INT_MIN;
			vector<MCTSNode*> maxPtrList;

			for (auto& ptr : cur->children) {
				ptr->calU();
				double score = ptr->actionScore();
				if (score > maxScore) {
					maxScore = score;
					maxPtrList.clear();
					maxPtrList.push_back(ptr);
				}
				else if (score == maxScore) {
					maxPtrList.push_back(ptr);
				}
			}
			if (maxPtrList.empty()) {
				return cur;
			}
			MCTSNode* maxPtr = maxPtrList[0];
			if (maxPtrList.size() > 1) {
				int random_variable = std::rand();
				maxPtr = maxPtrList[random_variable % maxPtrList.size()];
			}
			curChess.apply(maxPtr->move);
			curChess.flip();
			cur = maxPtr;
			if (maxPtr->done != Playing) {
				return cur;
			}
		}
		return cur;
	}
	void backup(double score) {
		auto curPtr = this;

		while (curPtr != nullptr) {
			curPtr->backupImpl(score);
			if (curPtr->moved)
				return;
			score = -score;
			curPtr = curPtr->parent;
		}
	}
	void backupImpl(double score) {
//		if (this->move.y1 == 7 && this->move.x1 == 1 && this->move.y2 == 0 && this->move.x2 == 1) {
//			cerr << "score:" << score << " total:" << this->Q << endl;
//		}
		if (moved)
			N2++;
		else
			N++;
		W += score;
		if (N == 0) {
			cerr << "unexpected N = 0 but backup!" << endl;
			Q = 0;
		}
		else
			Q = W / N;
	}
	void computePosition() {
//		chess = parent->chess;
//		chess.apply(this->move);
//
//		chess.flip();
//		chess.recalBaseScore();

	}
	void calU() {
		U = c_PUCT * (prior)* sqrt(parent->N + parent->N2) / (N + 1);
	}
	double actionScore() {
		return Q + U;
	}
	void printChildrenSummary() {
		vector<MCTSNode*> arr(children.size());
		int i = 0;
		for (auto& ptr : children) {
			arr[i++] = ptr;
//			cout << ptr->move.toString() << ", " << ptr->N <<"\t" << ptr->prior << "\t" <<ptr->Q <<"\t" << endl;
		}
		auto comp = [](MCTSNode*& lhs, MCTSNode*& rhs) {
			return lhs->move.toString() < rhs->move.toString();
		};
		sort(arr.begin(), arr.end(),comp);
		for (auto& ptr : arr) {
			cerr << ptr->move.toString() << ", " << ptr->N <<"\t" << ptr->prior << "\t" <<ptr->Q <<"\t" <<ptr->beingCheckMatedCnt <<"\t" <<ptr->firedCheckMateCnt <<"\t" << endl;
		}
		cerr << endl;
	}
};

class MCTS {
public:
	MCTS()
	{
		std::srand(std::time(nullptr)); // use current time as seed for random generator
	}
	virtual ~MCTS() {}

	vector<Move> suggestMove(MCTSNode* root, bool computerTurn) {
		vector<Move> ret;
		Move winMov;
		if (rootChess.getWinMove(winMov)) {
			ret.push_back(winMov);
			return ret;
		}
		int times = 1;
		if (computerTurn) {
			times = Config::getInstance().getSearchTimes();
		}

		cout << "searching for " << times << endl;
		{
			auto timerInstance = TimerFactory::getInstance().createTimerInstance("core Running time");
			for (int i = 0; i < times; ++i) {
				if (PoolMgr::getInstance().size() < 100)
				{
					cout << "size running low  " << PoolMgr::getInstance().size() << " stopping at times: " << i <<endl;
					break;
				}
				treeSearch(root);
			}
		}

		auto childComp = [](const MCTSNode* l, const MCTSNode* r) {
			return l->N > r->N;
		};

		root->children.sort(childComp);
		int numAdvice = 3;
		if (root->children.size() <= 3) {
			numAdvice = root->children.size();
		}
		vector<Move> retMoves(numAdvice);
		int i =0;
		for (auto ptr : root->children) {
			retMoves[i] = ptr->move;
			++i;
			if (i >= numAdvice)
				break;
		}
		ret = retMoves;
//		int maxN = 0;
//		Move maxMove;
//		for (auto& ptr : root->children) {
//			if (ptr->N > maxN) {
//				maxN = ptr->N;
//				maxMove = ptr->move;
//			}
//		}
//		ret.push_back(maxMove);


		TimerFactory::getInstance().printSummary();
		TimerFactory::getInstance().clear();

		return ret;
	}

	void treeSearch(MCTSNode* root) {

		MCTSNode* chosenLeaf;
		{
//			auto timerInstance = TimerFactory::getInstance().createTimerInstance("selectLeaf");
			chosenLeaf = root->selectLeaf();

			if (chosenLeaf->done == Won) {
				chosenLeaf->backup(1);
				//cerr << "already won" << endl;
				return;
			}
			if (chosenLeaf->done != Playing) {
				chosenLeaf->backup(0);
				//cerr << "already not playing " << chosenLeaf->done  << endl;
				return;
			}
			//checkmate setting
			int attackerId = -1;
			if (curChess.isCheckmated(attackerId)) {
				chosenLeaf->beingCheckMatedCnt = 1;
				if (chosenLeaf->parent!= nullptr && chosenLeaf->parent->firedAttackerId == attackerId)
					chosenLeaf->beingCheckMatedCnt += chosenLeaf->parent->firedCheckMateCnt;
			}
			else {
				chosenLeaf->beingCheckMatedCnt = 0;
			}
			chosenLeaf->beingAttackerId = attackerId;


			curChess.recalBaseScore();
			//chosenLeaf->computePosition();

			if (curChess.boardSign > 0) {
				chosenLeaf->done = Playing;
			}

		}
		pair<int, Move> ret;
		{

			//ret = chosenLeaf->chess.getCalValue(0, 0);
//			ret = Chess::getCalValue(*chosenLeaf->pChess);
			ret = pair<int, Move>(0, Move());
		}
		{
			int retScore = ret.first;
			double tscore;
			{
				//auto timerInstance = TimerFactory::getInstance().createTimerInstance("getCalValue");
				tscore = -calYv(retScore + curChess.score);
			}
			{
				//auto timerInstance = TimerFactory::getInstance().createTimerInstance("expand");
				ChessResult won = Playing;
				if (retScore > 2000) {
					won = chosenLeaf->expand(&ret.second, focusProbCal(retScore));
				}
				else {
					won = chosenLeaf->expand(nullptr, 0);
				}
				if (won == Won) {
					tscore = -1;
				}
				else if (won != Playing) {
					tscore = 0;
				}
			}
			{
				//auto timerInstance = TimerFactory::getInstance().createTimerInstance("backup");
				chosenLeaf->backup(tscore);
			}
		}
	}
};

PoolMgr::PoolMgr() :
		vPool(Config::getInstance().getPoolSize())
{
//	for (int i = 0; i < size; ++i) {
//		pool.add(unique_ptr<MCTSNode>(new MCTSNode()));
//	}
}

#endif /* MCTS_H_ */
