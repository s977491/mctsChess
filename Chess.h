
#ifndef CHESS_H_
#define CHESS_H_

#include <iostream>
#include <math.h>
#include <deque>
#include <sstream>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <climits>
#include <utility>
#include <future>
#include <thread>
#include "config.h"


//#include "thread_pool.hpp"
//tp::ThreadPool Config::getInstance().getPool();

using namespace std;



typedef tuple<int, int, int, int> tupleKey_t;
typedef array<array<char, 9>, 10> board_t;
#define LEVEL -1
#define pk 1
#define pr 2
#define ph 3
#define pc 4
#define pa 5
#define pe 6
#define pp 7

#define invalid -100000000
//int PieceScore[] = { 0, 100000, 10000, 4800, 5200, 1500, 1500, 500 };
int PieceScore[] = { 0, 100000, 10000, 4800, 5200, 2500, 2500, 500 };
const int ppbonus = 750;
const int bonusUnit = 50;
class Piece;
class Move {
public:
	int x1 = -1;
	int y1 = -1;
	int x2 = -1;
	int y2 = -1;
	Piece* pieceStart;
	Piece* pieceEnd;
	int ate;
	Move(Piece* p, int yy1, int xx1, int yy2, int xx2) :
		x1(xx1), y1(yy1), x2(xx2), y2(yy2), pieceStart(p), pieceEnd(nullptr) {
		ate = 0;
	}
	Move() {}
	string toString() const {
		ostringstream oss;
		oss << y1 << "," << x1 << " to " << y2 << "," << x2;
		return oss.str();
	}
	tupleKey_t getTupleKey() const {
		return make_tuple(y1, x1, y2, x2);
	}
	/*
	void addMove(Move& nextMove) {
	score += nextMove.score;
	moveList.insert(moveList.end(), nextMove.moveList.begin(), nextMove.moveList.end());
	}
	void finalizeMove() {
	moveList.push_back(*this);
	}*/

};
//auto comp = [](const Move& lhs, const Move& rhs) {
//	return lhs.score > rhs.score;
//};
//auto compOpp = [](const Move& lhs, const Move& rhs) {
//	return lhs.score < rhs.score;
//};

class Piece {
public:
	int x;
	int y;
	int piece;
	bool dead;
	Piece(int _y, int _x, int _piece) :x(_x), y(_y), piece(_piece) {
		dead = false;
	}
	string toString() {
		ostringstream oss;
		oss << "piece:" << piece << " loc: " << y << "," << x << " dead:" << dead;
		return oss.str();
	}
};



class Chess {
public:
	void init() {
		init(board);
	}
	void init(board_t& _board)
	{
		board = _board;
		//posList.reserve(16);
		//negList.reserve(16);
		posList.clear();
		negList.clear();
		score = 0;
		//		Py_ssize_t n;
		int i;
		int ky = 0;
		bool hasAttacker = false;

		for (i = 0; i< 10; i++) {
			for (int j = 0; j < 9; ++j) {
				int sign = 1;
				int piece = board[i][j];
				int v = abs(piece);
				if (v == 0)
					continue;
				if (piece < 0) {
					sign = -1;
				}
				switch (v) {
				case pk:
					if (sign  > 0) {
						ky = i;
					}
					break;
				case pr:
					hasAttacker = true;
					break;
				case ph:
					hasAttacker = true;
					break;
				case pc:
					hasAttacker = true;
					break;
				case pa:
					break;
				case pe:
					break;
				case pp:
					hasAttacker = true;
					break;
				default:
					break;
				}
				score += PieceScore[v] * sign;
				if (sign > 0) {
					if (v == pk)
						posList.push_front(Piece(i, j, piece));
					else
						posList.push_back(Piece(i, j, piece));

				}
				else {
					if (v == pk)
						negList.push_front(Piece(i, j, piece));
					else
						negList.push_back(Piece(i, j, piece));
				}
			}
		}
		if (!hasAttacker)
			draw = 1;
		for (auto& piece : posList) {
			if (piece.dead)
				continue;
			int bonus = getBonus(piece, ky<5); //caled the score inside the move
			score += bonus;
			//			cout << piece.piece  << " bonus:" << bonus << " " << score << endl;
		}
		for (auto& piece : negList) {
			if (piece.dead)
				continue;
			int bonus = getBonus(piece, ky < 5); //caled the score inside the move
			score -= bonus;
			//			cout << piece.piece  << " bonus:-" << bonus << " " << score << endl;
		}
		valid = true;
	}
	string getPieceString(int pieceId, bool positiveList) {
		if (pieceId <0)
			return "";
		if (positiveList) {
			return posList[pieceId].toString();
		}
		else {
			return negList[pieceId].toString();
		}
	}
	void recalBaseScore() {
		score = 0;
		for (auto& piece : posList) {
			if (piece.dead)
				continue;
			score += PieceScore[abs(piece.piece)];
			int bonus = getBonus(piece, boardSign > 0); //caled the score inside the move
			score += bonus;
			//			cout << piece.piece  << " bonus:" << bonus << " " << score << endl;
		}
		for (auto& piece : negList) {
			if (piece.dead)
				continue;
			score -= PieceScore[abs(piece.piece)];
			int bonus = getBonus(piece, boardSign > 0); //caled the score inside the move
			score -= bonus;
			//			cout << piece.piece  << " bonus:-" << bonus << " " << score << endl;
		}
	}
	int getBonus(Piece& piece, bool kingUpper) {
		int v = abs(piece.piece);
		int yDeta;
		if (piece.piece > 0) {
			yDeta = piece.y;
		}
		else {
			yDeta = 9 - piece.y;
		}
		int xDeta = abs(piece.x - 4);
		int bonus = 0;
		int movCount = 0;

//		int totalPieces = negList.size() + posList.size();

		Move mov(&piece, piece.y, piece.x, 0, 0);
		//cout << yDeta << " " << xDeta << " " << v << " " << sign << endl;
		switch (v) {
		case pk:
			return  -(int)((yDeta + xDeta * 2 + 5) * (yDeta)* bonusUnit);
		case pa:
			if (yDeta <= 1) {
				bonus = 5 * bonusUnit;
			}
			return bonus;
		case pe:
			if (xDeta + yDeta <= 2) {
				bonus += 4 * bonusUnit;
			}
			return bonus;
		case pp:
			if (yDeta > 5) {
				if (yDeta < 8)
					return ppbonus;
				//				if (yDeta == 8)
				//					return ppbonus * 2/4;
			}
			return 0;
		case ph:

			for (int i = -1; i <= 2; i += 2) {
				for (int j = -1; j <= 2; j += 2) {
					mov.y2 = mov.y1 + 2 * i;
					mov.x2 = mov.x1 + 1 * j;
					if (validMov(mov, true) && board[mov.y2 - i][mov.x1] == 0) {
						++movCount;
					}

					mov.y2 = mov.y1 + 1 * i;
					mov.x2 = mov.x1 + 2 * j;
					if (validMov(mov) && board[mov.y1][mov.x2 - j] == 0) {
						++movCount;
					}
				}
			}
			if (movCount == 0) {
				bonus = - 4 *bonusUnit ;
//				bonus = (movCount - 3);
//				bonus = bonus * bonusUnit ;
			}
			//bonus -= (totalPieces-16) * bonusUnit ;
			if (yDeta != 0 || xDeta != 3)
				bonus += 4 * bonusUnit;
			return bonus;
		case pr:
		{
			int bonus = 0;
			if (xDeta != 4)
				bonus +=  4 * bonusUnit;
			if (yDeta > 0)
				bonus +=  4 * bonusUnit;
//
//			list<Move> ret;
//			addMoves(piece, ret);
//			//cout << "pr: " << ret.size() <<  "," << piece.piece << endl;
//			if (ret.size() <= 3) {
//				for (auto& mov : ret) {
//					//					cout << mov.toString() <<  endl;
//					if (board[mov.y2][mov.x2] * piece.piece < 0)
//						return 0;
//				}
//				return bonus +-5 * bonusUnit;
//			}
			return bonus;
			break;
		}
		case pc:
//			list<Move> ret;
//			addMoves(piece, ret);
//			bool diffX = false;
//			bool diffY = false;
//			for (auto& mov : ret) {
//				if (mov.x2 != piece.x)
//					diffX = true;
//				if (mov.y2 != piece.y)
//					diffY = true;
//				if (diffX && diffY)
//					break;
//			}
//			if (!diffX || !diffY)
//				return -bonusUnit * 10;
			return 0;
			//return (9-yDeta)*bonusUnit + (totalPieces-16) * bonusUnit/3;
		}

		return 0;
	}
	bool isValid() {
		return valid;
	}
	~Chess() {
	}

	void flip() {
		score = -score;
		boardSign = -boardSign;
		vector< vector<int>  > board;
		swap(posList, negList);
		/*for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 9; ++j) {
		board[i][j] = -board[i][j];
		}
		}*/
	}


	bool validMov(Move mov, bool allowSamePiece = false) {
		if (mov.x1 <0 || mov.x2 < 0 || mov.y1<0 || mov.y2<0 || mov.x1 >= 9 || mov.x2 >= 9 || mov.y1 >= 10 || mov.y2 >= 10)
			return false;

		if (allowSamePiece || board[mov.y1][mov.x1] * board[mov.y2][mov.x2] <= 0)
			return true; //same piece cannot overlap

		return false;
	}

	void apply(Move& move) {
		int p = board[move.y1][move.x1];
		move.ate = board[move.y2][move.x2];
		board[move.y2][move.x2] = p;
		board[move.y1][move.x1] = 0;

		//int oriScore = score;
		if (move.ate != 0) {
			// = getPieceScore(move.y2, move.x2, move.ate, pMoveList);
			//score += scoreChange;

			for (size_t i = 0; i < negList.size(); ++i) {
				Piece& piece = negList[i];
				if (piece.y == move.y2 && piece.x == move.x2 && !piece.dead) {
					piece.dead = true;
					move.pieceEnd = &piece;
					break;
				}
			}

		}
		for (size_t i = 0; i < posList.size(); ++i) {
			Piece& piece = posList[i];
			if (piece.y == move.y1 && piece.x == move.x1 && !piece.dead) {
				move.pieceStart = &piece;
				move.pieceStart->y = move.y2;
				move.pieceStart->x = move.x2;
				break;
			}
		}
	}
	void rollback(Move& move) {
		int p = board[move.y2][move.x2];
		board[move.y2][move.x2] = move.ate;
		board[move.y1][move.x1] = p;

		if (move.pieceEnd != nullptr) {
			Piece& undeadPiece = *(move.pieceEnd);
			undeadPiece.dead = false;
			undeadPiece.x = move.x2;
			undeadPiece.y = move.y2;
		}
		move.pieceStart->y = move.y1;
		move.pieceStart->x = move.x1;
		//cout << "after rollback"<< endl << toString() <<endl;
	}
	/*list<Move> getMoves(Piece piece, int sign, int mode) {

		return list<Move>{};
	}*/
	bool pcChecker(Move& mov, int& obstacle, list<Move>& ret)
	{
		if (!validMov(mov, true))
			return true;
		if (obstacle == 0) {
			if (board[mov.y2][mov.x2] == 0)
				ret.push_back(mov);
			else {
				obstacle++;
			}
			return false;
		}
		else if (obstacle == 1) {
			if (board[mov.y2][mov.x2] == 0)
				return false;
			++obstacle;
			if (board[mov.y2][mov.x2] * board[mov.y1][mov.x1] < 0)
			{
				ret.push_back(mov);
				return true;
			}
			return false;
		}
		return true;
	}
	int getPieceScore(int y, int x, int piece, deque<Piece>* pieceSideList) {
		Piece* pMyKingPiece = &(pieceSideList->front());
		int scoreChange = PieceScore[abs(piece)];
		if (abs(piece) == pp) { //crossed river has more power unless at the bottom
			if (pMyKingPiece->y > 5 && y > 4 && y < 9) {
				scoreChange += ppbonus;
			}
			else if (pMyKingPiece->y < 4 && y <= 4 && y >0) {
				scoreChange += ppbonus;
			}
		}
		if (piece > 0)
			return -scoreChange;
		else
			return scoreChange;
	}
	list<Move> getPossibleMove() {
		list<Move> ret;
		for (auto& piece : posList) {
			if (piece.dead) continue;
			addMoves(piece, ret); //caled the score inside the move
		}
		return ret;
	}
	inline bool isCheckmated(int& attackerId) {
		list<Move> ret;
		Piece& king = posList.front();
		if (king.dead)
			return false;

		int i = 0;
		for (auto& piece : negList) {
			if (piece.dead) continue;
			if (isReasonableKill(piece.piece, piece.y, piece.x, king.y, king.x)) {
				attackerId = i;
				return true;
			}
			++i;
		}
		return false;
	}
	inline bool isReasonableKill(int piece, int fromY, int fromX, int toY, int toX) {
		int src = abs(piece);
		if (src == pa || src == pe)
			return false;
		int ydiff = abs(fromY - toY);
		int xdiff = abs(fromX - toX);
		int obs = 0;

		switch (src){
		case pp:
			if (xdiff + ydiff == 1) {
				return true;
			}
			break;
		case ph:
			if (xdiff + ydiff == 3 && xdiff > 0 && ydiff > 0) {
				if (xdiff == 2 && board[fromY][(fromX+toX)/2] == 0) {
					return true;
				}
				if (ydiff == 2 && board[(fromY+toY)/2][fromX] == 0) {
					return true;
				}
			}
			break;
		case pc:
		case pr:
		case pk:
			 if (src == pc)
				 obs = 1;

			 if (xdiff == 0) {
				 int sign = -1;
				 if (toY > fromY)
					 sign = 1;
				 for ( int i = fromY + sign; i != toY; i += sign) {
					 if (board[i][fromX] != 0)
						 --obs;
				 }
				 if (obs == 0)
					 return true;
				 return false;
			 }
			 if (ydiff == 0) {
				 int sign = -1;
				 if (toX > fromX)
					 sign = 1;
				 for ( int i = fromX + sign; i != toX; i += sign) {
					 if (board[fromY][i] != 0)
						 --obs;
				 }
				 if (obs == 0)
					 return true;
				 return false;
			 }
			break;
		}
		return false;
	}
	bool getWinMove(Move& winMove) {
		return getWinMove(getPossibleMove(), winMove);
	}

	bool getWinMove(list<Move> moveList, Move& winMove) {
		for (auto& mov : moveList) {
			if (abs(board[mov.y2][mov.x2]) == pk) {
				winMove = mov;
				return true;
			}
		}
		return false;
	}
//	static pair<int, Move> getCalValue(Chess& chess) {
//		list<Move> possibleMoves = chess.getPossibleMove();
//		vector<Chess> arr;
//
//		for (Move& move : possibleMoves) {
//			arr.push_back(chess);
//		}
//
//		int i = 0;
//		vector<future<pair<int, Move> > > arrRet;
//		for (Move& move : possibleMoves) {
//			packaged_task<pair<int, Move>() > task(
//					[i,move, &arr]() {
//
//									int ate = arr[i].board[move.y2][move.x2];
//									int ateScore = 0;
//									if (ate != 0) {
//										ateScore = PieceScore[abs(ate)];
//									} else {
//										return pair<int, Move>(0, move);
//									}
//									Move tmpMove = move;
//									arr[i].apply(tmpMove);
//									arr[i].flip();
//									int revScore = arr[i].getCalValue(0, 0).first;
//
//									return pair<int, Move>(ateScore - revScore, move);
//								}
//			);
//			auto result = task.get_future();
//
////			auto result = Config::getInstance().getPool().enqueue(
////				);
//			++i;
//			arrRet.push_back(std::move(result));
//			Config::getInstance().getPool().post(std::move(task));
//		}
//		int maxValue = INT_MIN;
//		pair<int, Move> maxIndex;
//		for (auto& f : arrRet) {
//			auto result = f.get();
//			//cout << " getting " << result.second.toString() << " " << result.first << endl;
//			if (result.first > maxValue) {
//				maxValue = result.first;
//				maxIndex = result;
//			}
//		}
//
//		return maxIndex;
//
//	}
	static pair<int, Move> getCalValue(Chess& chess) {
		list<Move> possibleMoves = chess.getPossibleMove();
		vector<Chess> arr;

		for (Move& move : possibleMoves) {
			arr.push_back(chess);
		}

		int i = 0;
		vector<future<pair<int, Move> > > arrRet;
		for (Move& move : possibleMoves) {
			auto result = Config::getInstance().getPool().enqueue([i,move, &arr]() {

				int ate = arr[i].board[move.y2][move.x2];
				int ateScore = 0;
				if (ate != 0) {
					ateScore = PieceScore[abs(ate)];
				} else {
					return pair<int, Move>(0, move);
				}
				Move tmpMove = move;
				arr[i].apply(tmpMove);
				arr[i].flip();
				int revScore = arr[i].getCalValue(0, 0).first;

				return pair<int, Move>(ateScore - revScore, move);
			});
			++i;
			arrRet.push_back(std::move(result));
		}
		int maxValue = INT_MIN;
		pair<int, Move> maxIndex;
		for (auto& f : arrRet) {
			auto result = f.get();
			//cout << " getting " << result.second.toString() << " " << result.first << endl;
			if (result.first > maxValue) {
				maxValue = result.first;
				maxIndex = result;
			}
		}

		return maxIndex;

	}

	pair<int, Move> getCalValue(int scoreRequirement, int level) {
		list<Move> possibleMove;
		for (auto& piece : posList) {
			if (piece.dead) continue;
			addMoves(piece, possibleMove); //caled the score inside the move
		}
		int maxScore = 0;

		//cout << "boardScore" << score << ", " << INT_MIN << endl;
		Move* maxMove = nullptr;
		for (Move& move : possibleMove) {
			if (board[move.y1][move.x1] == 0) {
				cerr << "unexpected no move from but in move list!!!" << endl;
			}
			int ate = board[move.y2][move.x2];
			if (ate == 0)
				continue;
			int ateScore = PieceScore[abs(ate)];
			if (abs(ate) == pk) {
				return pair<int, Move>(ateScore, move);
			}
			if (ateScore  < scoreRequirement)
				continue;
			if (level < LEVEL)
				cout << "Lvl:" << level << ate << " studying move : " << move.toString() << " scoreReq" << scoreRequirement << " score:" << score << endl;

			//do move
			apply(move);
			score += ateScore;
			flip();
			if (level < LEVEL)
				cout << "Lvl:" << level << "move Score ate" << ateScore << endl;
			int revScore = getCalValue(ateScore - scoreRequirement, level + 1).first;
			if (level < LEVEL)
				cout << "Lvl:" << level << "revScore Score ate" << revScore << endl;
			flip();
			if (ateScore - revScore > maxScore) {
				maxScore = ateScore - revScore;
				maxMove = &move;
			}
			//maxScore = max(maxScore, ateScore - revScore);

			if (level < LEVEL)
				cout << "Lvl:" << level << "maxScore " << maxScore << endl;
			score -= ateScore;
			rollback(move);
		}
		//		if (maxScore == INT_MIN) {
		//			//no eat move
		//			if (level < LEVEL)
		//			cout << "Lvl:" << level <<" no eat move"<<endl;
		//			return 0;
		//		}
		Move retMove;
		if (maxMove != nullptr) {
			retMove = *maxMove;
		}
		if (level < LEVEL)
		{
			if (maxMove != nullptr)
				cout << "Lvl:" << level << "return maxScore " << maxScore << " Move:" << maxMove->toString() << endl;
			else
				cout << "Lvl:" << level << "return maxScore due to eat leads even worse or no move" << maxScore << endl;
		}
		return pair<int, Move>(maxScore, retMove);
	}
	void addMoves(Piece& piece, list<Move>& ret) {
		int v = abs(piece.piece);
		Move mov(&piece, piece.y, piece.x, 0, 0);
		switch (v) {
		case pk:
			for (int i = 0; i < 2; ++i) {
				int j = 1 - i;
				mov.y2 = piece.y + i;
				mov.x2 = piece.x + j;
				if (validMov(mov) && (mov.y2 <= 2 || mov.y2 >= 7) && (mov.x2 >= 3 && mov.x2 <= 5)) {
					ret.push_back(mov);
				}
				mov.y2 = piece.y - i;
				mov.x2 = piece.x - j;
				if (validMov(mov) && (mov.y2 <= 2 || mov.y2 >= 7) && (mov.x2 >= 3 && mov.x2 <= 5)) {
					ret.push_back(mov);
				}
			}
			for (int i = piece.y + boardSign; i >= 0 && i <10; i = i + boardSign) {
				if (board[i][piece.x] == 0)
					continue;
				if (abs(board[i][piece.x]) == pk) {
					mov.y2 = i;
					mov.x2 = piece.x;
					ret.push_back(mov);
					break;
				}
				break;
			}
			break;
		case pa:
			for (int i = -1; i < 2; i += 2) {
				mov.y2 = piece.y + i;
				mov.x2 = piece.x + i;
				if (validMov(mov) && (mov.y2 <= 2 || mov.y2 >= 7) && (mov.x2 >= 3 && mov.x2 <= 5)) {
					ret.push_back(mov);
				}
				mov.y2 = piece.y + i;
				mov.x2 = piece.x - i;
				if (validMov(mov) && (mov.y2 <= 2 || mov.y2 >= 7) && (mov.x2 >= 3 && mov.x2 <= 5)) {
					ret.push_back(mov);
				}
			}
			break;
		case pe:
			for (int i = -2; i < 3; i += 4) {
				mov.y2 = piece.y + i;
				mov.x2 = piece.x + i;
				int blockY = (mov.y2 + mov.y1) / 2;
				int blockX = (mov.x2 + mov.x1) / 2;
				if (validMov(mov) && board[blockY][blockX] == 0 && ((mov.y2 <= 4 && mov.y1 <= 4) || (mov.y2 >4 && mov.y1 >4))) {
					ret.push_back(mov);
				}
				mov.y2 = piece.y + i;
				mov.x2 = piece.x - i;
				blockY = (mov.y2 + mov.y1) / 2;
				blockX = (mov.x2 + mov.x1) / 2;
				if (validMov(mov) && board[blockY][blockX] == 0 && ((mov.y2 <= 4 && mov.y1 <= 4) || (mov.y2 >4 && mov.y1 >4))) {
					ret.push_back(mov);
				}
			}
			break;
		case pp:

			mov.y2 = piece.y + boardSign;
			mov.x2 = piece.x;
			if (validMov(mov))
				ret.push_back(mov);

			if ((mov.y1 >4 && boardSign == 1) || (mov.y1 <= 4 && boardSign == -1)) {
				mov.y2 = piece.y;

				mov.x2 = piece.x + 1;
				if (validMov(mov))
					ret.push_back(mov);

				mov.x2 = piece.x - 1;
				if (validMov(mov))
					ret.push_back(mov);
			}
			break;
		case ph:
			for (int i = -1; i <= 2; i += 2) {
				for (int j = -1; j <= 2; j += 2) {
					mov.y2 = mov.y1 + 2 * i;
					mov.x2 = mov.x1 + 1 * j;
					if (validMov(mov) && board[mov.y2 - i][mov.x1] == 0) {
						ret.push_back(mov);
					}

					mov.y2 = mov.y1 + 1 * i;
					mov.x2 = mov.x1 + 2 * j;
					if (validMov(mov) && board[mov.y1][mov.x2 - j] == 0) {
						ret.push_back(mov);
					}

				}
			}
			break;
		case pr:
			mov.y2 = mov.y1;
			for (int i = mov.x1 - 1; i >= 0; --i) {
				mov.x2 = i;
				if (!validMov(mov))
					break;
				ret.push_back(mov);
				if (board[mov.y2][mov.x2] != 0)
					break;
			}
			for (int i = mov.x1 + 1; i < 9; ++i) {
				mov.x2 = i;
				if (!validMov(mov))
					break;
				ret.push_back(mov);
				if (board[mov.y2][mov.x2] != 0)
					break;
			}
			mov.x2 = mov.x1;
			for (int i = mov.y1 - 1; i >= 0; --i) {
				mov.y2 = i;
				if (!validMov(mov))
					break;
				ret.push_back(mov);
				if (board[mov.y2][mov.x2] != 0)
					break;
			}
			for (int i = mov.y1 + 1; i < 10; ++i) {
				mov.y2 = i;
				if (!validMov(mov))
					break;
				ret.push_back(mov);
				if (board[mov.y2][mov.x2] != 0)
					break;
			}
			break;
		case pc:
			mov.y2 = mov.y1;
			int obstacle = 0;
			for (int i = mov.x1 - 1; i >= 0; --i) {
				mov.x2 = i;
				if (pcChecker(mov, obstacle, ret))
					break;
			}
			obstacle = 0;
			for (int i = mov.x1 + 1; i < 9; ++i) {
				mov.x2 = i;
				if (pcChecker(mov, obstacle, ret))
					break;
			}
			mov.x2 = mov.x1;
			obstacle = 0;
			for (int i = mov.y1 - 1; i >= 0; --i) {
				mov.y2 = i;
				if (pcChecker(mov, obstacle, ret))
					break;
			}
			obstacle = 0;
			for (int i = mov.y1 + 1; i < 10; ++i) {
				mov.y2 = i;
				if (pcChecker(mov, obstacle, ret))
					break;
			}
			break;

		}

	}
	//Move getMaxScoreMove(int sign, int level, int lastScore) {
	//	vector<Move> scoredMove;
	//	scoredMove.reserve(160);
	//	vector<Piece> * ptrList = &posList;
	//	if (sign < 0)
	//		ptrList = &negList;
	//	for (auto piece : *ptrList) {
	//		if (piece.dead) continue;
	//		list<Move> possibleMove = getMoves(piece, sign, level); //caled the score inside the move

	//		for (auto move : possibleMove) {
	//			if (board[move.y1][move.x1] * board[move.y2][move.x2] >= 0)
	//				continue;

	//			//cout << "Lvl:" << level << "studying move : " << move.toString() << " lastScore" << lastScore  <<endl;

	//			if (abs(board[move.y2][move.x2]) == pk) {
	//				move.score = sign * PieceScore[pk];
	//				move.finalizeMove();
	//				return move;
	//			}

	//			//dont consider some can't get back score move
	//			int ate = board[move.y2][move.x2];
	//			int scoreChange = getPieceScore(move.y2, move.x2, ate, ptrList);

	//			bool canSkip = false;
	//			{
	//				if (abs(scoreChange) < abs(lastScore)) {
	//					//cout << "Lvl:" << level << " move rev store not enough MScore" <<  scoreChange  << "  LScore" << lastScore <<endl;

	//					//cannot revenage back, do not consider this, just return
	//					move.ate = board[move.y2][move.x2];
	//					move.score = scoreChange / 2; // try to short cut and est 1/2 of the value
	//					scoredMove.push_back(move);

	//					continue;
	//				}
	//			}

	//			vector<Piece> posTmpList = posList;
	//			vector<Piece> negTmpList = negList;
	//			int nextScore = lastScore + scoreChange;
	//			move.score = scoreChange;
	//			apply(move);
	//			Move tmpMove = move;
	//			{
	//				Move nextMove = getMaxScoreMove(-sign, level + 1, nextScore - 1); //actual score

	//				if (abs(nextMove.score) != PieceScore[pk]) { // must not allow other to kill own king
	//					tmpMove.addMove(nextMove);
	//					if (tmpMove.score* sign > 0)
	//						scoredMove.push_back(tmpMove);
	//				}
	//			}
	//			rollback(move);
	//			posList = posTmpList;
	//			negList = negTmpList;
	//			//move.score = lastScore;
	//		}
	//	}

	//	if (scoredMove.empty()) {
	//		return Move();
	//	}
	//	if (sign > 0) {
	//		sort(scoredMove.begin(), scoredMove.end(), comp);
	//	}
	//	else {
	//		sort(scoredMove.begin(), scoredMove.end(), compOpp);
	//	}
	//	int index = 0;
	//	int maxScore = scoredMove[0].score;

	//	for (auto m : scoredMove) {
	//		if (m.score != maxScore)
	//			break;
	//		++index;
	//	}
	//	//[0,index) are equal score, randome one out
	//	Move ret = scoredMove[rand() % index];
	//	ret.finalizeMove();

	//	//		if (level <= 0){
	//	//			cout << "chosen:" <<  ret.y1 << "," << ret.x1 << " to " << ret.y2 << "," << ret.x2 << " :score:" << ret.score << endl;
	//	//			cout << "move path" << endl;
	//	//			for (auto m : ret.moveList) {
	//	//				cout << m.toString() <<endl;
	//	//			}
	//	//		}
	//	return ret;

	//}


	string toString() {
		ostringstream oss;
		for (int i = 9; i >=0; --i) {
			oss << (i ) << ":";
			for (int j = 0; j < 9; ++j) {
				int v = abs(board[i][j]);
				char piece = ' ';
				switch (v) {
				case pk:
					piece = 'k';
					break;
				case pr:
					piece = 'r';
					break;
				case ph:
					piece = 'h';
					break;
				case pc:
					piece = 'c';
					break;
				case pa:
					piece = 'a';
					break;
				case pe:
					piece = 'e';
					break;
				case pp:
					piece = 'p';
					break;
				default:
					break;
				}
				if (board[i][j] < 0) {
					piece = piece - ('a' - 'A');
				}
				if (piece >='a') {
					oss << "\033[1;31m" << piece << "\033[0m ";
				}
				else {
					oss << "\033[1;32m" << piece << "\033[0m ";
				}

			}
			oss << endl;
		}
		oss << "  ";
		for (int j = 0; j < 9; ++j){
			oss << (j ) << " ";
		}
		oss << endl;

		/*oss << "posList:" << endl;
		for (auto ppp : posList) {
			oss << ppp.toString() << endl;
		}
		oss << "negList:" << endl;
		for (auto ppp : negList) {
			oss << ppp.toString() << endl;
		}*/
		oss << "cur board score:" << score << endl;
		return oss.str();
	}
	int score = 0;
	int draw = 0;
	int boardSign = 1;
private:
	bool valid = false;
	//vector< vector<int>  >& board;
	//unique_ptr<vector< vector<int>  > > pBoard;
	board_t board;
	deque<Piece> posList;
	deque<Piece> negList;
};

#endif /* CHESS_H_ */
