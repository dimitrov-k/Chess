#pragma once
#include "chess.h"
#include "Move.h"
class Game : private Chess
{
public:
	Game();
	~Game();

	static const char MENU_OPTION_NEW = 'N';
	static const char MENU_OPTION_MOVE = 'M';
	static const char MENU_OPTION_QUIT = 'Q';
	static const char MENU_OPTION_UNDO = 'U';
	static const char MENU_OPTION_SAVE = 'S';
	static const char MENU_OPTION_LOAD = 'L';

	void movePiece(Move* currentMove);

	void undoLastMove();

	bool isUndoPossible();

	bool isCastlingAllowed(Chess::Side side, int color);

	char getPieceAtPosition(int row, int column);

	char getPieceAtPosition(Position position);

	char considerMove(int row, int column, IntendedMove* intendedMove = nullptr);

	Chess::UnderAttack underAttack(Chess::Position currentPosition, int color, IntendedMove* intendedMove = nullptr);

	bool isReachable(Chess::Position currentPosition, int color);

	bool isSquareOccupied(int row, int column);

	bool isPathFree(Position starting, Position finishing, int direction);

	bool canBeBlocked(Position starting, Position finishing, int direction);

	bool isCheckMate();

	bool isKingInCheck(int color, IntendedMove* intendedMove = nullptr);

	bool isPlayerKingInCheck(IntendedMove* intendedMove = nullptr);

	bool wouldKingBeInCheck(char piece, Move* currentMove);

	Position findKing(int color);

	void changeTurns(void);

	bool isFinished(void);

	int getCurrentTurn(void);

	int getOpponentColor(void);

	void parseMove(string move, Position* from, Position* to, char* promoted = nullptr);

	void logMove(std::string& record);

	string getLastMove(void);

	void deleteLastMove(void);

	void initCastlingTrue(void);

	// Save all the moves
	struct Round
	{
		string whiteMove;
		string blackMove;
	};

	//std::deque<std::string> moves;
	std::deque<Round> rounds;

	// Save the captured pieces
	std::vector<char> whiteCaptured;
	std::vector<char> blackCaptured;

private:

	// Represent the pieces in the board
	char board[8][8];

	// Undo is possible?
	struct Undo
	{
		bool undo;
		bool lastMoveCaptured;

		bool allowedCastlingKingSide;
		bool allowedCastlingQueenSide;

		EnPassant enPassant;
		Castling  castling;
		Promotion promotion;

		void initFalse(void);

	} undoMove;

	// Castling requirements
	bool castlingKingSideAllowed[2];
	bool castlingQueenSideAllowed[2];

	// Holds the current turn
	int  currentTurn;

	// Has the game finished already?
	bool gameFinished;

	void checkReachableHorizontal(Chess::Position currentPosition, int color, bool& bReachable);

	void checkReachableVertical(Chess::Position currentPosition, int color, bool& bReachable);

	void checkReachableDiagonal(Chess::Position currentPosition, int color, bool& bReachable);

	void checkReachableLShaped(Chess::Position currentPosition, int color, bool& bReachable);

	void checkUnderAttackHorizontal(Chess::Position currentPosition, int color, IntendedMove* intendedMove, UnderAttack& attack);

	void checkUnderAttackVertical(Chess::Position currentPosition, int color, IntendedMove* intendedMove, UnderAttack& attack);

	void checkUnderAttackDiagonal(Chess::Position currentPosition, int color, IntendedMove* intendedMove, UnderAttack& attack);

	void checkUnderAttackLShape(Chess::Position currentPosition, int color, IntendedMove* intendedMove, UnderAttack& attack);

	void canBeBlockedHorizontal(Position starting, Position finishing, bool& bBlocked);

	void canBeBlockedVertical(Position starting, Position finishing, bool& bBlocked);

	void canBeBlockedDiagonal(Position starting, Position finishing, bool& bBlocked);

	void isPathFreeHorizontal(Position starting, Position finishing, bool& bFree);

	void isPathFreeVertical(Position starting, Position finishing, bool& bFree);

	void isPathFreeDiagonal(Position starting, Position finishing, bool& bFree);
};

