#pragma once
#include "includes.h"

class Chess
{
public:
	static int getPieceColor(char piece);

	static bool isWhitePiece(char piece);

	static bool isBlackPiece(char piece);

	static std::string describePiece(char piece);

	static const char PIECE_TYPE_PAWN = 'P';
	static const char PIECE_TYPE_KNIGHT = 'N';
	static const char PIECE_TYPE_BISHOP = 'B';
	static const char PIECE_TYPE_ROOK = 'R';
	static const char PIECE_TYPE_QUEEN = 'Q';
	static const char PIECE_TYPE_KING = 'K';
	static const char PIECE_TYPE_PAWN_LOW = 'p';
	static const char PIECE_TYPE_KNIGHT_LOW = 'n';
	static const char PIECE_TYPE_BISHOP_LOW = 'b';
	static const char PIECE_TYPE_ROOK_LOW = 'r';
	static const char PIECE_TYPE_QUEEN_LOW = 'q';
	static const char PIECE_TYPE_KING_LOW = 'k';
	static const int EMPTY_FIELD = 0x20;

	enum PieceColor
	{
		WHITE_PIECE = 0,
		BLACK_PIECE = 1
	};

	enum Player
	{
		WHITE_PLAYER = 0,
		BLACK_PLAYER = 1
	};

	enum class Side
	{
		QUEEN_SIDE = 2,
		KING_SIDE = 3
	};

	enum Direction
	{
		HORIZONTAL = 0,
		VERTICAL,
		DIAGONAL,
		L_SHAPE
	};

	struct Position
	{
		int row;
		int column;
	};

	struct EnPassant
	{
		bool applied;
		Position PawnCaptured;
	};

	struct Castling
	{
		bool applied;
		Position rookBefore;
		Position rookAfter;
	};

	struct Promotion
	{
		bool applied;
		//Position  position;
		char before;
		char after;
	};

	struct IntendedMove
	{
		char piece;
		Position from;
		Position to;
	};

	struct Attacker
	{
		Position  position;
		Direction direction;
	};

	struct UnderAttack
	{
		bool underAttack;
		int numAttackers;
		Attacker attacker[9]; //maximum theorical number of attackers
	};

	const char initial_board[8][8] =
	{
		// This represents the pieces on the board.
		// Keep in mind that pieces[0][0] represents A1
		// pieces[1][1] represents B2 and so on.
		// Letters in CAPITAL are white
		{ PIECE_TYPE_ROOK,  PIECE_TYPE_KNIGHT,  PIECE_TYPE_BISHOP,  PIECE_TYPE_QUEEN,  PIECE_TYPE_KING,  PIECE_TYPE_BISHOP,  PIECE_TYPE_KNIGHT,  PIECE_TYPE_ROOK },
		{ PIECE_TYPE_PAWN,  PIECE_TYPE_PAWN  ,  PIECE_TYPE_PAWN  ,  PIECE_TYPE_PAWN ,  PIECE_TYPE_PAWN,  PIECE_TYPE_PAWN  ,  PIECE_TYPE_PAWN  ,  PIECE_TYPE_PAWN },
		{ EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD },
		{ EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD },
		{ EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD },
		{ EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD, EMPTY_FIELD },
		{ PIECE_TYPE_PAWN_LOW,  PIECE_TYPE_PAWN_LOW  , PIECE_TYPE_PAWN_LOW  ,  PIECE_TYPE_PAWN_LOW ,  PIECE_TYPE_PAWN_LOW,  PIECE_TYPE_PAWN_LOW  ,  PIECE_TYPE_PAWN_LOW  ,  PIECE_TYPE_PAWN_LOW },
		{ PIECE_TYPE_ROOK_LOW,  PIECE_TYPE_KNIGHT_LOW, PIECE_TYPE_BISHOP_LOW,  PIECE_TYPE_QUEEN_LOW,  PIECE_TYPE_KING_LOW,  PIECE_TYPE_BISHOP_LOW,  PIECE_TYPE_KNIGHT_LOW,  PIECE_TYPE_ROOK_LOW },
	};
};
