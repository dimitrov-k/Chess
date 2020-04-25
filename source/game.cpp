#include "game.h"
#include "chess.h"
#include "includes.h"
#include "user_interface.h"

Game::Game()
{
	// White player always starts
	currentTurn = WHITE_PLAYER;

	// Game on!
	gameFinished = false;

	// Nothing has happend yet
	undoMove.initFalse();

	// Initial board settings
	memcpy(board, initial_board, sizeof(char) * 8 * 8);

	// Castling is allowed (to each side) until the player moves the king or the rook
	initCastlingTrue();
}

Game::~Game()
{
	whiteCaptured.clear();
	blackCaptured.clear();
	rounds.clear();
}

void Game::movePiece(Position present, Position future, Chess::EnPassant* S_enPassant, Chess::Castling* S_castling, Chess::Promotion* S_promotion)
{
	// Get the piece to be moved
	char piece = getPieceAtPosition(present);

	// Is the destination square occupied?
	char chCapturedPiece = getPieceAtPosition(future);

	// So, was a piece captured in this move?
	if (0x20 != chCapturedPiece)
	{
		if (WHITE_PIECE == getPieceColor(chCapturedPiece))
		{
			// A white piece was captured
			whiteCaptured.push_back(chCapturedPiece);
		}
		else
		{
			// A black piece was captured
			blackCaptured.push_back(chCapturedPiece);
		}

		// Set Undo structure. If a piece was captured, then no "en passant" move performed
		undoMove.lastMoveCaptured = true;

		// Reset undoMove.castling
		memset(&undoMove.enPassant, 0, sizeof(Chess::EnPassant));
	}
	else if (true == S_enPassant->applied)
	{
		char chCapturedEP = getPieceAtPosition(S_enPassant->PawnCaptured.row, S_enPassant->PawnCaptured.column);

		if (WHITE_PIECE == getPieceColor(chCapturedEP))
		{
			// A white piece was captured
			whiteCaptured.push_back(chCapturedEP);
		}
		else
		{
			// A black piece was captured
			blackCaptured.push_back(chCapturedEP);
		}

		// Now, remove the captured pawn
		board[S_enPassant->PawnCaptured.row][S_enPassant->PawnCaptured.column] = EMPTY_SQUARE;

		// Set Undo structure as piece was captured and "en passant" move was performed
		undoMove.lastMoveCaptured = true;
		memcpy(&undoMove.enPassant, S_enPassant, sizeof(Chess::EnPassant));
	}
	else
	{
		undoMove.lastMoveCaptured = false;

		// Reset undoMove.castling
		memset(&undoMove.enPassant, 0, sizeof(Chess::EnPassant));
	}

	// Remove piece from present position
	board[present.row][present.column] = EMPTY_SQUARE;

	// Move piece to new position
	if (true == S_promotion->applied)
	{
		board[future.row][future.column] = S_promotion->after;

		// Set Undo structure as a promotion occured
		memcpy(&undoMove.promotion, S_promotion, sizeof(Chess::Promotion));
	}
	else
	{
		board[future.row][future.column] = piece;

		// Reset undoMove.promotion
		memset(&undoMove.promotion, 0, sizeof(Chess::Promotion));
	}

	// Was it a castling move?
	if (S_castling->applied == true)
	{
		// The king was already move, but we still have to move the rook to 'jump' the king
		char piece = getPieceAtPosition(S_castling->rookBefore.row, S_castling->rookBefore.column);

		// Remove the rook from present position
		board[S_castling->rookBefore.row][S_castling->rookBefore.column] = EMPTY_SQUARE;

		// 'Jump' into to new position
		board[S_castling->rookAfter.row][S_castling->rookAfter.column] = piece;

		// Write this information to the undoMove struct
		memcpy(&undoMove.castling, S_castling, sizeof(Chess::Castling));

		// Save the 'isCastlingAllowed' information in case the move is undone
		undoMove.allowedCastlingKingSide = castlingKingSideAllowed[getCurrentTurn()];
		undoMove.allowedCastlingQueenSide = castlingQueenSideAllowed[getCurrentTurn()];
	}
	else
	{
		// Reset undoMove.castling
		memset(&undoMove.castling, 0, sizeof(Chess::Castling));
	}

	// Castling requirements
	if (Chess::PIECE_TYPE_KING == toupper(piece))
	{
		// After the king has moved once, no more castling allowed
		castlingKingSideAllowed[getCurrentTurn()] = false;
		castlingQueenSideAllowed[getCurrentTurn()] = false;
	}
	else if (Chess::PIECE_TYPE_ROOK == toupper(piece))
	{
		// If the rook moved from column 'A', no more castling allowed on the queen side
		if (0 == present.column)
		{
			castlingQueenSideAllowed[getCurrentTurn()] = false;
		}

		// If the rook moved from column 'A', no more castling allowed on the queen side
		else if (7 == present.column)
		{
			castlingKingSideAllowed[getCurrentTurn()] = false;
		}
	}

	// Change turns
	changeTurns();

	// This move can be undone
	undoMove.undo = true;
}

void Game::undoLastMove()
{
	string last_move = getLastMove();

	// Parse the line
	Chess::Position from;
	Chess::Position to;
	parseMove(last_move, &from, &to);

	// Since we want to undo a move, we will be moving the piece from (iToRow, iToColumn) to (iFromRow, iFromColumn)
	char piece = getPieceAtPosition(to.row, to.column);

	// Moving it back
	// If there was a castling
	if (true == undoMove.promotion.applied)
	{
		board[from.row][from.column] = undoMove.promotion.before;
	}
	else
	{
		board[from.row][from.column] = piece;
	}

	// Change turns
	changeTurns();

	// If a piece was captured, move it back to the board
	if (undoMove.lastMoveCaptured)
	{
		// Let's retrieve the last captured piece
		char chCaptured;

		// Since we already changed turns back, it means we should we pop a piece from the oponents vector
		if (WHITE_PLAYER == currentTurn)
		{
			chCaptured = blackCaptured.back();
			blackCaptured.pop_back();
		}
		else
		{
			chCaptured = whiteCaptured.back();
			whiteCaptured.pop_back();
		}

		// Move the captured piece back. Was this an "en passant" move?
		if (undoMove.enPassant.applied)
		{
			// Move the captured piece back
			board[undoMove.enPassant.PawnCaptured.row][undoMove.enPassant.PawnCaptured.column] = chCaptured;

			// Remove the attacker
			board[to.row][to.column] = EMPTY_SQUARE;
		}
		else
		{
			board[to.row][to.column] = chCaptured;
		}
	}
	else
	{
		board[to.row][to.column] = EMPTY_SQUARE;
	}

	// If there was a castling
	if (undoMove.castling.applied)
	{
		char chRook = getPieceAtPosition(undoMove.castling.rookAfter.row, undoMove.castling.rookAfter.column);

		// Remove the rook from present position
		board[undoMove.castling.rookAfter.row][undoMove.castling.rookAfter.column] = EMPTY_SQUARE;

		// 'Jump' into to new position
		board[undoMove.castling.rookBefore.row][undoMove.castling.rookBefore.column] = chRook;

		// Restore the values of castling allowed or not
		castlingKingSideAllowed[getCurrentTurn()] = undoMove.allowedCastlingKingSide;
		castlingQueenSideAllowed[getCurrentTurn()] = undoMove.allowedCastlingQueenSide;
	}

	// Clean undoMove struct
	undoMove.undo = false;
	undoMove.lastMoveCaptured = false;
	undoMove.enPassant.applied = false;
	undoMove.castling.applied = false;
	undoMove.promotion.applied = false;

	// If it was a checkmate, toggle back to game not finished
	gameFinished = false;

	// Finally, remove the last move from the list
	deleteLastMove();
}

bool Game::isUndoPossible()
{
	return undoMove.undo;
}

bool Game::isCastlingAllowed(Side side, int color)
{
	if (QUEEN_SIDE == side)
	{
		return castlingQueenSideAllowed[color];
	}
	else //if ( KING_SIDE == side )
	{
		return castlingKingSideAllowed[color];
	}
}

char Game::getPieceAtPosition(int row, int column)
{
	return board[row][column];
}

char Game::getPieceAtPosition(Position position)
{
	return board[position.row][position.column];
}

char Game::considerMove(int row, int column, IntendedMove* intendedMove)
{
	char piece;

	// If there is no intended move, just return the current position of the board
	if (nullptr == intendedMove)
	{
		piece = getPieceAtPosition(row, column);
	}
	else
	{
		// In this case, we are trying to understand what WOULD happed if the move was made,
		// so we consider a move that has not been made yet
		if (intendedMove->from.row == row && intendedMove->from.column == column)
		{
			// The piece wants to move from that square, so it would be empty
			piece = EMPTY_SQUARE;
		}
		else if (intendedMove->to.row == row && intendedMove->to.column == column)
		{
			// The piece wants to move to that square, so return the piece
			piece = intendedMove->piece;
		}
		else
		{
			piece = getPieceAtPosition(row, column);
		}
	}

	return piece;
}

Chess::UnderAttack Game::underAttack(int row, int column, int color, IntendedMove* intendedMove)
{
	UnderAttack attack = { 0 };

	// a) Direction: HORIZONTAL
	{
		// Check all the way to the right
		for (int i = column + 1; i < 8; i++)
		{
			char chPieceFound = considerMove(row, i, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the right, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = row;
				attack.attacker[attack.numAttackers - 1].position.column = i;
				attack.attacker[attack.numAttackers - 1].direction = HORIZONTAL;
				break;
			}
			else
			{
				// There is a piece that does not attack horizontally, so no problem
				break;
			}
		}

		// Check all the way to the left
		for (int i = column - 1; i >= 0; i--)
		{
			char chPieceFound = considerMove(row, i, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the right, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = row;
				attack.attacker[attack.numAttackers - 1].position.column = i;
				attack.attacker[attack.numAttackers - 1].direction = HORIZONTAL;
				break;
			}
			else
			{
				// There is a piece that does not attack horizontally, so no problem
				break;
			}
		}
	}

	// b) Direction: VERTICAL
	{
		// Check all the way up
		for (int i = row + 1; i < 8; i++)
		{
			char chPieceFound = considerMove(i, column, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the right, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = column;
				attack.attacker[attack.numAttackers - 1].direction = VERTICAL;
				break;
			}
			else
			{
				// There is a piece that does not attack vertically, so no problem
				break;
			}
		}

		// Check all the way down
		for (int i = row - 1; i >= 0; i--)
		{
			char chPieceFound = considerMove(i, column, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the right, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = column;
				attack.attacker[attack.numAttackers - 1].direction = VERTICAL;
				break;
			}
			else
			{
				// There is a piece that does not attack vertically, so no problem
				break;
			}
		}
	}

	// c) Direction: DIAGONAL
	{
		// Check the diagonal up-right
		for (int i = row + 1, j = column + 1; i < 8 && j < 8; i++, j++)
		{
			char chPieceFound = considerMove(i, j, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(i == row + 1) &&
				(j == column + 1) &&
				(color == WHITE_PIECE))
			{
				// A pawn only puts another piece in jeopardy if it's (diagonally) right next to it
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else
			{
				// There is a piece that does not attack diagonally, so no problem
				break;
			}
		}

		// Check the diagonal up-left
		for (int i = row + 1, j = column - 1; i < 8 && j > 0; i++, j--)
		{
			char chPieceFound = considerMove(i, j, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(i == row + 1) &&
				(j == column - 1) &&
				(color == WHITE_PIECE))
			{
				// A pawn only puts another piece in jeopardy if it's (diagonally) right next to it
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else
			{
				// There is a piece that does not attack diagonally, so no problem
				break;
			}
		}

		// Check the diagonal down-right
		for (int i = row - 1, j = column + 1; i > 0 && j < 8; i--, j++)
		{
			char chPieceFound = considerMove(i, j, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(i == row - 1) &&
				(j == column + 1) &&
				(color == BLACK_PIECE))
			{
				// A pawn only puts another piece in jeopardy if it's (diagonally) right next to it
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else
			{
				// There is a piece that does not attack diagonally, so no problem
				break;
			}
		}

		// Check the diagonal down-left
		for (int i = row - 1, j = column - 1; i > 0 && j > 0; i--, j--)
		{
			char chPieceFound = considerMove(i, j, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(i == row - 1) &&
				(j == column - 1) &&
				(color == BLACK_PIECE))
			{
				// A pawn only puts another piece in jeopardy if it's (diagonally) right next to it
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the piece is in jeopardy
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = i;
				attack.attacker[attack.numAttackers - 1].position.column = j;
				attack.attacker[attack.numAttackers - 1].direction = DIAGONAL;
				break;
			}
			else
			{
				// There is a piece that does not attack diagonally, so no problem
				break;
			}
		}
	}

	// d) Direction: L_SHAPED
	{
		// Check if the piece is put in jeopardy by a knigh

		Position knight_moves[8] = { {  1, -2 }, {  2, -1 }, {  2, 1 }, {  1, 2 },
									 { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 } };
		for (int i = 0; i < 8; i++)
		{
			int iRowToTest = row + knight_moves[i].row;
			int iColumnToTest = column + knight_moves[i].column;

			if (iRowToTest < 0 || iRowToTest > 7 || iColumnToTest < 0 || iColumnToTest > 7)
			{
				// This square does not even exist, so no need to test
				continue;
			}

			char chPieceFound = considerMove(iRowToTest, iColumnToTest, intendedMove);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color, so no problem
				continue;
			}
			else if ((toupper(chPieceFound) == 'N'))
			{
				attack.underAttack = true;
				attack.numAttackers += 1;

				attack.attacker[attack.numAttackers - 1].position.row = iRowToTest;
				attack.attacker[attack.numAttackers - 1].position.column = iColumnToTest;
				break;
			}
		}
	}

	return attack;
}

bool Game::isReachable(int row, int column, int color)
{
	bool bReachable = false;

	// a) Direction: HORIZONTAL
	{
		// Check all the way to the right
		for (int i = column + 1; i < 8; i++)
		{
			char chPieceFound = getPieceAtPosition(row, i);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the right, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move horizontally
				break;
			}
		}

		// Check all the way to the left
		for (int i = column - 1; i >= 0; i--)
		{
			char chPieceFound = getPieceAtPosition(row, i);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook to the left, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move horizontally
				break;
			}
		}
	}

	// b) Direction: VERTICAL
	{
		// Check all the way up
		for (int i = row + 1; i < 8; i++)
		{
			char chPieceFound = getPieceAtPosition(i, column);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(getPieceColor(chPieceFound) == BLACK_PIECE) &&
				(i == row + 1))
			{
				// There is a pawn one square up, so the square is reachable
				bReachable = true;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook on the way up, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move vertically
				break;
			}
		}

		// Check all the way down
		for (int i = row - 1; i >= 0; i--)
		{
			char chPieceFound = getPieceAtPosition(i, column);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == 'P') &&
				(getPieceColor(chPieceFound) == WHITE_PIECE) &&
				(i == row - 1))
			{
				// There is a pawn one square down, so the square is reachable
				bReachable = true;
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_ROOK))
			{
				// There is a queen or a rook on the way down, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move vertically
				break;
			}
		}
	}

	// c) Direction: DIAGONAL
	{
		// Check the diagonal up-right
		for (int i = row + 1, j = column + 1; i < 8 && j < 8; i++, j++)
		{
			char chPieceFound = getPieceAtPosition(i, j);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move diagonally
				break;
			}
		}

		// Check the diagonal up-left
		for (int i = row + 1, j = column - 1; i < 8 && j > 0; i++, j--)
		{
			char chPieceFound = getPieceAtPosition(i, j);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move diagonally
				break;
			}
		}

		// Check the diagonal down-right
		for (int i = row - 1, j = column + 1; i > 0 && j < 8; i--, j++)
		{
			char chPieceFound = getPieceAtPosition(i, j);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == 'B'))
			{
				// There is a queen or a bishop in that direction, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move diagonally
				break;
			}
		}

		// Check the diagonal down-left
		for (int i = row - 1, j = column - 1; i > 0 && j > 0; i--, j--)
		{
			char chPieceFound = getPieceAtPosition(i, j);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				break;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_QUEEN) ||
				(toupper(chPieceFound) == Chess::PIECE_TYPE_BISHOP))
			{
				// There is a queen or a bishop in that direction, so the square is reachable
				bReachable = true;
				break;
			}
			else
			{
				// There is a piece that does not move diagonally
				break;
			}
		}
	}

	// d) Direction: L_SHAPED
	{
		// Check if the piece is put in jeopardy by a knigh

		Position knight_moves[8] = { {  1, -2 }, {  2, -1 }, {  2, 1 }, {  1, 2 },
									 { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 } };
		for (int i = 0; i < 8; i++)
		{
			int iRowToTest = row + knight_moves[i].row;
			int iColumnToTest = column + knight_moves[i].column;

			if (iRowToTest < 0 || iRowToTest > 7 || iColumnToTest < 0 || iColumnToTest > 7)
			{
				// This square does not even exist, so no need to test
				continue;
			}

			char chPieceFound = getPieceAtPosition(iRowToTest, iColumnToTest);
			if (EMPTY_SQUARE == chPieceFound)
			{
				// This square is empty, move on
				continue;
			}

			if (color == getPieceColor(chPieceFound))
			{
				// This is a piece of the same color
				continue;
			}
			else if ((toupper(chPieceFound) == Chess::PIECE_TYPE_KNIGHT))
			{
				bReachable = true;
				break;
			}
		}
	}

	return bReachable;
}

bool Game::isSquareOccupied(int row, int column)
{
	bool bOccupied = false;

	if (0x20 != getPieceAtPosition(row, column))
	{
		bOccupied = true;
	}

	return bOccupied;
}

bool Game::isPathFree(Position starting, Position finishing, int direction)
{
	bool bFree = false;

	switch (direction)
	{
	case Chess::HORIZONTAL:
	{
		// If it is a horizontal move, we can assume the starting.row == finishing.row
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (starting.column == finishing.column)
		{
			cout << "Error. Movement is horizontal but column is the same\n";
		}

		// Moving to the right
		else if (starting.column < finishing.column)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = starting.column + 1; i < finishing.column; i++)
			{
				if (isSquareOccupied(starting.row, i))
				{
					bFree = false;
					cout << "Horizontal path to the right is not clear!\n";
				}
			}
		}

		// Moving to the left
		else //if (starting.column > finishing.column)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = starting.column - 1; i > finishing.column; i--)
			{
				if (isSquareOccupied(starting.row, i))
				{
					bFree = false;
					cout << "Horizontal path to the left is not clear!\n";
				}
			}
		}
	}
	break;

	case Chess::VERTICAL:
	{
		// If it is a vertical move, we can assume the starting.column == finishing.column
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (starting.row == finishing.row)
		{
			cout << "Error. Movement is vertical but row is the same\n";
			throw("Error. Movement is vertical but row is the same");
		}

		// Moving up
		else if (starting.row < finishing.row)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = starting.row + 1; i < finishing.row; i++)
			{
				if (isSquareOccupied(i, starting.column))
				{
					bFree = false;
					cout << "Vertical path up is not clear!\n";
				}
			}
		}

		// Moving down
		else //if (starting.column > finishing.row)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = starting.row - 1; i > finishing.row; i--)
			{
				if (isSquareOccupied(i, starting.column))
				{
					bFree = false;
					cout << "Vertical path down is not clear!\n";
				}
			}
		}
	}
	break;

	case Chess::DIAGONAL:
	{
		// Moving up and right
		if ((finishing.row > starting.row) && (finishing.column > starting.column))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (isSquareOccupied(starting.row + i, starting.column + i))
				{
					bFree = false;
					cout << "Diagonal path up-right is not clear!\n";
				}
			}
		}

		// Moving up and left
		else if ((finishing.row > starting.row) && (finishing.column < starting.column))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (isSquareOccupied(starting.row + i, starting.column - i))
				{
					bFree = false;
					cout << "Diagonal path up-left is not clear!\n";
				}
			}
		}

		// Moving down and right
		else if ((finishing.row < starting.row) && (finishing.column > starting.column))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (isSquareOccupied(starting.row - i, starting.column + i))
				{
					bFree = false;
					cout << "Diagonal path down-right is not clear!\n";
				}
			}
		}

		// Moving down and left
		else if ((finishing.row < starting.row) && (finishing.column < starting.column))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			bFree = true;

			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (isSquareOccupied(starting.row - i, starting.column - i))
				{
					bFree = false;
					cout << "Diagonal path down-left is not clear!\n";
				}
			}
		}

		else
		{
			throw("Error. Diagonal move not allowed");
		}
	}
	break;
	}

	return bFree;
}

bool Game::canBeBlocked(Position starting, Position finishing, int direction)
{
	bool bBlocked = false;

	Chess::UnderAttack blocker = { 0 };

	switch (direction)
	{
	case Chess::HORIZONTAL:
	{
		// If it is a horizontal move, we can assume the starting.row == finishing.row
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (starting.column == finishing.column)
		{
			cout << "Error. Movement is horizontal but column is the same\n";
		}

		// Moving to the right
		else if (starting.column < finishing.column)
		{
			for (int i = starting.column + 1; i < finishing.column; i++)
			{
				if (true == isReachable(starting.row, i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		// Moving to the left
		else //if (starting.column > finishing.column)
		{
			for (int i = starting.column - 1; i > finishing.column; i--)
			{
				if (true == isReachable(starting.row, i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}
	}
	break;

	case Chess::VERTICAL:
	{
		// If it is a vertical move, we can assume the starting.column == finishing.column
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (starting.row == finishing.row)
		{
			cout << "Error. Movement is vertical but row is the same\n";
			throw("Error. Movement is vertical but row is the same");
		}

		// Moving up
		else if (starting.row < finishing.row)
		{
			for (int i = starting.row + 1; i < finishing.row; i++)
			{
				if (true == isReachable(i, starting.column, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		// Moving down
		else //if (starting.column > finishing.row)
		{
			for (int i = starting.row - 1; i > finishing.row; i--)
			{
				if (true == isReachable(i, starting.column, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}
	}
	break;

	case Chess::DIAGONAL:
	{
		// Moving up and right
		if ((finishing.row > starting.row) && (finishing.column > starting.column))
		{
			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (true == isReachable(starting.row + i, starting.column + i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		// Moving up and left
		else if ((finishing.row > starting.row) && (finishing.column < starting.column))
		{
			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (true == isReachable(starting.row + i, starting.column - i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		// Moving down and right
		else if ((finishing.row < starting.row) && (finishing.column > starting.column))
		{
			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (true == isReachable(starting.row - i, starting.column + i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		// Moving down and left
		else if ((finishing.row < starting.row) && (finishing.column < starting.column))
		{
			for (int i = 1; i < abs(finishing.row - starting.row); i++)
			{
				if (true == isReachable(starting.row - i, starting.column - i, getOpponentColor()))
				{
					// Some piece can block the way
					bBlocked = true;
				}
			}
		}

		else
		{
			cout << "Error. Diagonal move not allowed\n";
			throw("Error. Diagonal move not allowed");
		}
	}
	break;
	}

	return bBlocked;
}

bool Game::isCheckMate()
{
	bool bCheckmate = false;

	// 1. First of all, it the king in check?
	if (false == isPlayerKingInCheck())
	{
		return false;
	}

	// 2. Can the king move the other square?
	Chess::Position king_moves[8] = { {  1, -1 },{  1, 0 },{  1,  1 }, { 0,  1 },
									   { -1,  1 },{ -1, 0 },{ -1, -1 }, { 0, -1 } };

	Chess::Position king = findKing(getCurrentTurn());

	for (int i = 0; i < 8; i++)
	{
		int iRowToTest = king.row + king_moves[i].row;
		int iColumnToTest = king.column + king_moves[i].column;

		if (iRowToTest < 0 || iRowToTest > 7 || iColumnToTest < 0 || iColumnToTest > 7)
		{
			// This square does not even exist, so no need to test
			continue;
		}

		if (EMPTY_SQUARE != getPieceAtPosition(iRowToTest, iColumnToTest))
		{
			// That square is not empty, so no need to test
			continue;
		}

		Chess::IntendedMove intendedMove;
		intendedMove.piece = getPieceAtPosition(king.row, king.column);
		intendedMove.from.row = king.row;
		intendedMove.from.column = king.column;
		intendedMove.to.row = iRowToTest;
		intendedMove.to.column = iColumnToTest;

		// Now, for every possible move of the king, check if it would be in jeopardy
		// Since the move has already been made, current_game->getCurrentTurn() now will return 
		// the next player's color. And it is in fact this king that we want to check for jeopardy
		Chess::UnderAttack king_moved = underAttack(iRowToTest, iColumnToTest, getCurrentTurn(), &intendedMove);

		if (false == king_moved.underAttack)
		{
			// This means there is at least one position when the king would not be in jeopardy, so that's not a checkmate
			return false;
		}
	}

	// 3. Can the attacker be taken or another piece get into the way? 
	Chess::UnderAttack king_attacked = underAttack(king.row, king.column, getCurrentTurn());
	if (1 == king_attacked.numAttackers)
	{
		// Can the attacker be taken?
		Chess::UnderAttack king_attacker = { 0 };
		king_attacker = underAttack(king_attacked.attacker[0].position.row, king_attacked.attacker[0].position.column, getOpponentColor());

		if (true == king_attacker.underAttack)
		{
			// This means that the attacker can be taken, so it's not a checkmate
			return false;
		}
		else
		{
			// Last resort: can any piece get in between the attacker and the king?
			char chAttacker = getPieceAtPosition(king_attacked.attacker[0].position.row, king_attacked.attacker[0].position.column);

			switch (toupper(chAttacker))
			{
			case Chess::PIECE_TYPE_PAWN:
			case Chess::PIECE_TYPE_KNIGHT:
			{
				// If it's a pawn, there's no space in between the attacker and the king
				// If it's a knight, it doesn't matter because the knight can 'jump'
				// So, this is checkmate
				bCheckmate = true;
			}
			break;

			case Chess::PIECE_TYPE_BISHOP:
			{
				if (false == canBeBlocked(king_attacked.attacker[0].position, king, Chess::DIAGONAL))
				{
					// If no piece can get in the way, it's a checkmate
					bCheckmate = true;
				}
			}
			break;

			case Chess::PIECE_TYPE_ROOK:
			{
				if (false == canBeBlocked(king_attacked.attacker[0].position, king, king_attacked.attacker[0].direction))
				{
					// If no piece can get in the way, it's a checkmate
					bCheckmate = true;
				}
			}
			break;

			case Chess::PIECE_TYPE_QUEEN:
			{
				if (false == canBeBlocked(king_attacked.attacker[0].position, king, king_attacked.attacker[0].direction))
				{
					// If no piece can get in the way, it's a checkmate
					bCheckmate = true;
				}
			}
			break;


			default:
			{
				throw("!!!!Should not reach here. Invalid piece");
			}
			break;
			}
		}
	}
	else
	{
		// If there is more than one attacker and we reached this point, it's a checkmate
		bCheckmate = true;
	}

	// If the game has ended, store in the class variable
	gameFinished = bCheckmate;

	return bCheckmate;
}

bool Game::isKingInCheck(int color, IntendedMove* intendedMove)
{
	bool bCheck = false;

	Position king = { 0 };

	// Must check if the intended move is to move the king itself
	if (nullptr != intendedMove && Chess::PIECE_TYPE_KING == toupper(intendedMove->piece))
	{
		king.row = intendedMove->to.row;
		king.column = intendedMove->to.column;
	}
	else
	{
		king = findKing(color);
	}

	UnderAttack king_attacked = underAttack(king.row, king.column, color, intendedMove);

	if (true == king_attacked.underAttack)
	{
		bCheck = true;
	}

	return bCheck;
}

bool Game::isPlayerKingInCheck(IntendedMove* intendedMove)
{
	return isKingInCheck(getCurrentTurn(), intendedMove);
}

bool Game::wouldKingBeInCheck(char piece, Position present, Position future, EnPassant* S_enPassant)
{
	IntendedMove intendedMove;

	intendedMove.piece = piece;
	intendedMove.from.row = present.row;
	intendedMove.from.column = present.column;
	intendedMove.to.row = future.row;
	intendedMove.to.column = future.column;

	return isPlayerKingInCheck(&intendedMove);
}

Chess::Position Game::findKing(int color)
{
	char chToLook = (WHITE_PIECE == color) ? Chess::PIECE_TYPE_KING : tolower(Chess::PIECE_TYPE_KING);
	Position king = { 0 };

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (chToLook == getPieceAtPosition(i, j))
			{
				king.row = i;
				king.column = j;
			}
		}
	}

	return king;
}

void Game::changeTurns(void)
{
	if (WHITE_PLAYER == currentTurn)
	{
		currentTurn = BLACK_PLAYER;
	}
	else
	{
		currentTurn = WHITE_PLAYER;
	}
}

bool Game::isFinished(void)
{
	return gameFinished;
}

int Game::getCurrentTurn(void)
{
	return currentTurn;
}

int Game::getOpponentColor(void)
{
	int color;

	if (Chess::WHITE_PLAYER == getCurrentTurn())
	{
		color = Chess::BLACK_PLAYER;
	}
	else
	{
		color = Chess::WHITE_PLAYER;
	}

	return color;
}

void Game::parseMove(string move, Position* from, Position* to, char* promoted)
{
	from->column = move[0];
	from->row = move[1];
	to->column = move[3];
	to->row = move[4];

	// Convert columns from ['A'-'H'] to [0x00-0x07]
	from->column = from->column - 'A';
	to->column = to->column - 'A';

	// Convert row from ['1'-'8'] to [0x00-0x07]
	from->row = from->row - '1';
	to->row = to->row - '1';

	if (promoted != nullptr)
	{
		if (move[5] == '=')
		{
			*promoted = move[6];
		}
		else
		{
			*promoted = EMPTY_SQUARE;
		}
	}
}

void Game::logMove(std::string& record)
{
	// If record contains only 5 chracters, add spaces
	// Because when 
	if (record.length() == 5)
	{
		record += "  ";
	}

	if (WHITE_PLAYER == getCurrentTurn())
	{
		// If this was a white player move, create a new round and leave the blackMove empty
		Round round;
		round.whiteMove = record;
		round.blackMove = "";

		rounds.push_back(round);
	}
	else
	{
		// If this was a blackMove, just update the last Round
		Round round = rounds[rounds.size() - 1];
		round.blackMove = record;

		// Remove the last round and put it back, now with the black move
		rounds.pop_back();
		rounds.push_back(round);
	}
}

string Game::getLastMove(void)
{
	string last_move;

	// Who did the last move?
	if (BLACK_PLAYER == getCurrentTurn())
	{
		// If it's black's turn now, white had the last move
		last_move = rounds[rounds.size() - 1].whiteMove;
	}
	else
	{
		// Last move was black's
		last_move = rounds[rounds.size() - 1].blackMove;
	}

	return last_move;
}

void Game::deleteLastMove(void)
{
	// Notice we already changed turns back
	if (WHITE_PLAYER == getCurrentTurn())
	{
		// Last move was white's turn, so simply pop from the back
		rounds.pop_back();
	}
	else
	{
		// Last move was black's, so let's 
		Round round = rounds[rounds.size() - 1];
		round.blackMove = "";

		// Pop last round and put it back, now without the black move
		rounds.pop_back();
		rounds.push_back(round);
	}
}

void Game::initCastlingTrue(void) {
	castlingKingSideAllowed[WHITE_PLAYER] = true;
	castlingKingSideAllowed[BLACK_PLAYER] = true;

	castlingQueenSideAllowed[WHITE_PLAYER] = true;
	castlingQueenSideAllowed[BLACK_PLAYER] = true;
}

void Game::Undo::initFalse(void) {
	this->undo = false;
	this->lastMoveCaptured = false;
	this->allowedCastlingKingSide = false;
	this->allowedCastlingQueenSide = false;
	this->enPassant.applied = false;
	this->castling.applied = false;
}
