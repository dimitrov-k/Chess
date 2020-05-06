#include "Move.h"

Move::Move()
{
	present = { 0 };
	future = { 0 };
	enPassant = new Chess::EnPassant;
	castling = new Chess::Castling;
	promotion = new Chess::Promotion;
	enPassant->applied = false;
	castling->applied = false;
	promotion->applied = false;
}

Chess::Position Move::getPresent() 
{
	return this->present;
}

Chess::Position Move::getFuture() const
{
	return this->future;
}

Chess::EnPassant* Move::getEnPassant() const
{
	return this->enPassant;
}

Chess::Castling* Move::getCastling() const
{
	return this->castling;
}

Chess::Promotion* Move::getPromotion() const
{
	return this->promotion;
}
