#pragma once
#include "includes.h"
#include "user_interface.h"
#include "chess.h"
#include "debug.h"
#include "game.h"
#include "Move.h"

class GameController
{
public:
	GameController();
	void loadMenu(void);

private:
	Game* currentGame;

	bool isPawnMovementValid(Move* currentMove) const;
	bool isRookMovementValid(Move* currentMove) const;
	bool isKnightMovementValid(Move* currentMove) const;
	bool isBishopMovementValid(Move* currentMove) const;
	bool isQueenMovementValid(Move* currentMove) const;
	bool isKingMovementValid(Move* currentMove) const;
	bool isPieceColourValid(Chess::Position present, Chess::Position future) const;
	bool isMoveValid(Move* currentMove) const;
	void makeTheMove(Move* currentMove);
	void newGame(void);
	void undoMove(void);
	bool isPickedPieceValid(Chess::Position& present, std::string& record);
	bool isPickedHouseValid(Chess::Position& future, Chess::Position present, std::string& record);
	bool isPromotionSuccessful(Move& currentMove, std::string& record);
	void checkIfKingInCheck() const;
	void movePiece(void);
	void saveGame(void);
	bool isLoadedMoveValid(Move& currentMove, std::string loadedMove);
	void loadGame(void);
};

