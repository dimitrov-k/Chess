#include "includes.h"
#include "chess.h"
#include "user_interface.h"

// Chess class
int Chess::getPieceColor(char piece)
{
	if (isupper(piece))
	{
		return WHITE_PIECE;
	}
	else
	{
		return BLACK_PIECE;
	}
}

bool Chess::isWhitePiece(char piece)
{
	return getPieceColor(piece) == Chess::WHITE_PIECE ? true : false;
}

bool Chess::isBlackPiece(char piece)
{
	return getPieceColor(piece) == Chess::BLACK_PIECE ? true : false;
}

std::string Chess::describePiece(char piece)
{
	std::string description;

	if (isWhitePiece(piece))
	{
		description += "White ";
	}
	else
	{
		description += "Black ";
	}

	switch (toupper(piece))
	{
	case Chess::PIECE_TYPE_PAWN:
	{
		description += "pawn";
	}
	break;

	case Chess::PIECE_TYPE_KNIGHT:
	{
		description += "knight";
	}
	break;

	case Chess::PIECE_TYPE_BISHOP:
	{
		description += "bishop";
	}
	break;

	case Chess::PIECE_TYPE_ROOK:
	{
		description += "rook";
	}
	break;

	case Chess::PIECE_TYPE_QUEEN:
	{
		description += "queen";
	}
	break;

	default:
	{
		description += "unknow piece";
	}
	break;
	}

	return description;
}


// Game class

