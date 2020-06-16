#pragma once
#include "includes.h"
#include "chess.h"

class Move
{
public:
	Move();
	Chess::Position getPresent() ;
	Chess::Position getFuture() const;
	Chess::EnPassant* getEnPassant() const;
	Chess::Castling* getCastling() const;
	Chess::Promotion* getPromotion() const;
	void setPresent(Chess::Position present);
	void setFuture(Chess::Position future);
	void setEnPassant(Chess::EnPassant* enPassant);
	void setCastling(Chess::Castling* castling); 
	void setPromotion(Chess::Promotion* promotion); 

private:
	Chess::Position present;
	Chess::Position future;
	Chess::EnPassant* enPassant;
	Chess::Castling* castling;
	Chess::Promotion* promotion;
};

