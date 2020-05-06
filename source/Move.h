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
	void setPresent(Chess::Position present) {
		this->present = present;
	}
	void setFuture(Chess::Position future) {
		this->future = future;
	}
	void setEnPassant(Chess::EnPassant* enPassant) {
		this->enPassant = enPassant;
	}
	void setCastling(Chess::Castling* castling) {
		this->castling = castling;
	}
	void setPromotion(Chess::Promotion* promotion) {
		this->promotion = promotion;
	}

private:
	Chess::Position present;
	Chess::Position future;
	Chess::EnPassant* enPassant;
	Chess::Castling* castling;
	Chess::Promotion* promotion;
};

