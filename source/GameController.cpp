#include "GameController.h"

GameController::GameController()
{
	this->currentGame = NULL;
}

void GameController::loadMenu(void)
{
	bool bRun = true;

	// Clear screen an print the logo
	clearScreen();
	printLogo();

	string input = "";

	while (bRun)
	{
		printMessage();
		printMenu();

		// Get input from user
		cout << "Type here: ";
		getline(cin, input);

		try {
			if (input.length() != 1)
			{
				throw invalid_argument("Invalid option. Type one letter only\n");
			}
		}
		catch (invalid_argument& error) {
			cout << error.what();
			continue;
		}

		try
		{
			switch (toupper(input[0]))
			{
			case Game::MENU_OPTION_NEW:
			{
				newGame();
				clearScreen();
				printLogo();
				printSituation(*currentGame);
				printBoard(*currentGame);
			}
			break;

			case Game::MENU_OPTION_MOVE:
			{
				if (currentGame)
				{
					if (currentGame->isFinished())
					{
						throw invalid_argument("This game has already finished!\n");
					}
					else
					{
						movePiece();
						//clearScreen();
						printLogo();
						printSituation(*currentGame);
						printBoard(*currentGame);
					}
				}
				else
				{
					throw invalid_argument("No game running!\n");
				}

			}
			break;

			case Game::MENU_OPTION_QUIT:
			{
				bRun = false;
			}
			break;

			case Game::MENU_OPTION_UNDO:
			{
				if (NULL != currentGame)
				{
					undoMove();
					//clearScreen();
					printLogo();
					printSituation(*currentGame);
					printBoard(*currentGame);
				}
				else
				{
					throw invalid_argument("No game running!\n");
				}
			}
			break;

			case Game::MENU_OPTION_SAVE:
			{
				if (NULL != currentGame)
				{
					saveGame();
					clearScreen();
					printLogo();
					printSituation(*currentGame);
					printBoard(*currentGame);
				}
				else
				{
					throw invalid_argument("No game running!\n");
				}
			}
			break;

			case Game::MENU_OPTION_LOAD:
			{
				loadGame();
				clearScreen();
				printLogo();
				printSituation(*currentGame);
				printBoard(*currentGame);
			}
			break;

			default:
			{
				throw invalid_argument("Option does not exist\n");
			}
			break;

			}

		}
		catch (const char* s)
		{
			s;
		}
		catch (invalid_argument& error) {
			cout << error.what();
		}
	}
}

bool GameController::isPawnMovementValid(Move* currentMove) const
{
	bool valid = false;
	char piece = currentGame->getPieceAtPosition(currentMove->getPresent().row, currentMove->getPresent().column);
	// Wants to move forward
	if (currentMove->getFuture().column == currentMove->getPresent().column)
	{
		// Simple move forward
		if ((Chess::isWhitePiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row + 1) ||
			(Chess::isBlackPiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row - 1))
		{
			if (EMPTY_SQUARE == currentGame->getPieceAtPosition(currentMove->getFuture().row, currentMove->getFuture().column))
			{
				valid = true;
			}
		}

		// Double move forward
		else if ((Chess::isWhitePiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row + 2) ||
			(Chess::isBlackPiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row - 2))
		{
			// This is only allowed if the pawn is in its original place
			if (Chess::isWhitePiece(piece))
			{
				if (EMPTY_SQUARE == currentGame->getPieceAtPosition(currentMove->getFuture().row - 1, currentMove->getFuture().column) &&
					EMPTY_SQUARE == currentGame->getPieceAtPosition(currentMove->getFuture().row, currentMove->getFuture().column) &&
					1 == currentMove->getPresent().row)
				{
					valid = true;
				}
			}
			else // if ( isBlackPiece(piece) )
			{
				if (EMPTY_SQUARE == currentGame->getPieceAtPosition(currentMove->getFuture().row + 1, currentMove->getFuture().column) &&
					EMPTY_SQUARE == currentGame->getPieceAtPosition(currentMove->getFuture().row, currentMove->getFuture().column) &&
					6 == currentMove->getPresent().row)
				{
					valid = true;
				}
			}
		}
		else
		{
			// This is invalid
			return false;
		}
	}

	// The "en passant" move
	else if ((Chess::isWhitePiece(piece) && 4 == currentMove->getPresent().row && 5 == currentMove->getFuture().row && 1 == abs(currentMove->getFuture().column - currentMove->getPresent().column)) ||
		(Chess::isBlackPiece(piece) && 3 == currentMove->getPresent().row && 2 == currentMove->getFuture().row && 1 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		// It is only valid if last move of the opponent was a double move forward by a pawn on a adjacent column
		string last_move = currentGame->getLastMove();

		// Parse the line
		Chess::Position LastMoveFrom;
		Chess::Position LastMoveTo;
		currentGame->parseMove(last_move, &LastMoveFrom, &LastMoveTo);

		// First of all, was it a pawn?
		char chLstMvPiece = currentGame->getPieceAtPosition(LastMoveTo.row, LastMoveTo.column);

		if (toupper(chLstMvPiece) != 'P')
		{
			return false;
		}

		// Did the pawn have a double move forward and was it an adjacent column?
		if (2 == abs(LastMoveTo.row - LastMoveFrom.row) && 1 == abs(LastMoveFrom.column - currentMove->getPresent().column))
		{
			cout << "En passant move!\n";
			valid = true;

			currentMove->getEnPassant()->applied = true;
			currentMove->getEnPassant()->PawnCaptured.row = LastMoveTo.row;
			currentMove->getEnPassant()->PawnCaptured.column = LastMoveTo.column;
		}
	}

	// Wants to capture a piece
	else if (1 == abs(currentMove->getFuture().column - currentMove->getPresent().column))
	{
		if ((Chess::isWhitePiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row + 1) || (Chess::isBlackPiece(piece) && currentMove->getFuture().row == currentMove->getPresent().row - 1))
		{
			// Only allowed if there is something to be captured in the square
			if (EMPTY_SQUARE != currentGame->getPieceAtPosition(currentMove->getFuture().row, currentMove->getFuture().column))
			{
				valid = true;
				cout << "Pawn captured a piece!\n";
			}
		}
	}
	else
	{
		// This is invalid
		return false;
	}

	// If a pawn reaches its eight rank, it must be promoted to another piece
	if ((Chess::isWhitePiece(piece) && 7 == currentMove->getFuture().row) ||
		(Chess::isBlackPiece(piece) && 0 == currentMove->getFuture().row))
	{
		cout << "Pawn must be promoted!\n";
		currentMove->getPromotion()->applied = true;
	}
	return valid;
}

bool GameController::isRookMovementValid(Move* currentMove) const
{
	bool valid = false;
	// Horizontal move
	if ((currentMove->getFuture().row == currentMove->getPresent().row) && (currentMove->getFuture().column != currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::HORIZONTAL))
		{
			valid = true;
		}
	}
	// Vertical move
	else if ((currentMove->getFuture().row != currentMove->getPresent().row) && (currentMove->getFuture().column == currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::VERTICAL))
		{
			valid = true;
		}
	}
	return valid;
}

bool GameController::isKnightMovementValid(Move* currentMove) const
{
	bool valid = false;
	if ((2 == abs(currentMove->getFuture().row - currentMove->getPresent().row)) && (1 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		valid = true;
	}

	else if ((1 == abs(currentMove->getFuture().row - currentMove->getPresent().row)) && (2 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		valid = true;
	}
	return valid;
}

bool GameController::isBishopMovementValid(Move* currentMove) const
{
	bool valid = false;
	// Diagonal move
	if (abs(currentMove->getFuture().row - currentMove->getPresent().row) == abs(currentMove->getFuture().column - currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::DIAGONAL))
		{
			valid = true;
		}
	}
	return valid;
}

bool GameController::isQueenMovementValid(Move* currentMove) const
{
	bool valid = false;
	// Horizontal move
	if ((currentMove->getFuture().row == currentMove->getPresent().row) && (currentMove->getFuture().column != currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::HORIZONTAL))
		{
			valid = true;
		}
	}
	// Vertical move
	else if ((currentMove->getFuture().row != currentMove->getPresent().row) && (currentMove->getFuture().column == currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::VERTICAL))
		{
			valid = true;
		}
	}

	// Diagonal move
	else if (abs(currentMove->getFuture().row - currentMove->getPresent().row) == abs(currentMove->getFuture().column - currentMove->getPresent().column))
	{
		// Check if there are no pieces on the way
		if (currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::DIAGONAL))
		{
			valid = true;
		}
	}
	return valid;
}

bool GameController::isKingMovementValid(Move* currentMove) const
{
	bool valid = false;
	char piece = currentGame->getPieceAtPosition(currentMove->getPresent().row, currentMove->getPresent().column);
	// Horizontal move by 1
	if ((currentMove->getFuture().row == currentMove->getPresent().row) && (1 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		valid = true;
	}

	// Vertical move by 1
	else if ((currentMove->getFuture().column == currentMove->getPresent().column) && (1 == abs(currentMove->getFuture().row - currentMove->getPresent().row)))
	{
		valid = true;
	}

	// Diagonal move by 1
	else if ((1 == abs(currentMove->getFuture().row - currentMove->getPresent().row)) && (1 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		valid = true;
	}

	// Castling
	else if ((currentMove->getFuture().row == currentMove->getPresent().row) && (2 == abs(currentMove->getFuture().column - currentMove->getPresent().column)))
	{
		// Castling is only allowed in these circunstances:

		// 1. King is not in check
		if (currentGame->isPlayerKingInCheck())
		{
			return false;
		}

		// 2. No pieces in between the king and the rook
		if (!currentGame->isPathFree(currentMove->getPresent(), currentMove->getFuture(), Chess::HORIZONTAL))
		{
			return false;
		}

		// 3. King and rook must not have moved yet;
		// 4. King must not pass through a square that is attacked by an enemy piece
		if (currentMove->getFuture().column > currentMove->getPresent().column)
		{
			// if currentMove->getFuture().column is greather, it means king side
			if (!currentGame->isCastlingAllowed(Chess::Side::KING_SIDE, Chess::getPieceColor(piece)))
			{
				createNextMessage("Castling to the king side is not allowed.\n");
				return false;
			}
			else
			{
				// Check if the square that the king skips is not under attack
				Chess::Position currentPosition = { currentMove->getPresent().row, currentMove->getPresent().column + 1 };
				Chess::UnderAttack square_skipped = currentGame->underAttack(currentPosition, currentGame->getCurrentTurn());
				if (!square_skipped.underAttack)
				{
					// Fill the currentMove->getCastling() structure
					currentMove->getCastling()->applied = true;

					// currentMove->getPresent() position of the rook
					currentMove->getCastling()->rookBefore.row = currentMove->getPresent().row;
					currentMove->getCastling()->rookBefore.column = currentMove->getPresent().column + 3;

					// currentMove->getFuture() position of the rook
					currentMove->getCastling()->rookAfter.row = currentMove->getFuture().row;
					currentMove->getCastling()->rookAfter.column = currentMove->getPresent().column + 1; // currentMove->getFuture().column -1

					valid = true;
				}
			}
		}
		else //if (currentMove->getFuture().column < currentMove->getPresent().column)
		{
			// if currentMove->getPresent().column is greather, it means queen side
			if (!currentGame->isCastlingAllowed(Chess::Side::QUEEN_SIDE, Chess::getPieceColor(piece)))
			{
				createNextMessage("Castling to the queen side is not allowed.\n");
				return false;
			}
			else
			{
				// Check if the square that the king skips is not attacked
				Chess::Position currentPosition = { currentMove->getPresent().row, currentMove->getPresent().column - 1 };
				Chess::UnderAttack square_skipped = currentGame->underAttack(currentPosition, currentGame->getCurrentTurn());
				if (!square_skipped.underAttack)
				{
					// Fill the currentMove->getCastling() structure
					currentMove->getCastling()->applied = true;

					// currentMove->getPresent() position of the rook
					currentMove->getCastling()->rookBefore.row = currentMove->getPresent().row;
					currentMove->getCastling()->rookBefore.column = currentMove->getPresent().column - 4;

					// currentMove->getFuture() position of the rook
					currentMove->getCastling()->rookAfter.row = currentMove->getFuture().row;
					currentMove->getCastling()->rookAfter.column = currentMove->getPresent().column - 1; // currentMove->getFuture().column +1

					valid = true;
				}
			}
		}
	}
	return valid;
}

bool GameController::isPieceColourValid(Chess::Position present, Chess::Position future) const
{
	char piece = currentGame->getPieceAtPosition(present.row, present.column);
	if (currentGame->isSquareOccupied(future.row, future.column)) {
		char chAuxPiece = currentGame->getPieceAtPosition(future.row, future.column);
		if (Chess::getPieceColor(piece) == Chess::getPieceColor(chAuxPiece)) {
			cout << "Position is already taken by a piece of the same color\n";
			return false;
		}
	}
	return true;
}

bool GameController::isMoveValid(Move* currentMove) const
{
	bool valid = false;
	char piece = currentGame->getPieceAtPosition(currentMove->getPresent().row, currentMove->getPresent().column);

	// 1. Is the piece  allowed to move in that direction?
	switch (toupper(piece))
	{
	case Chess::PIECE_TYPE_PAWN: {
		valid = isPawnMovementValid(currentMove);
		break;
	}

	case Chess::PIECE_TYPE_ROOK: {
		valid = isRookMovementValid(currentMove);
		break;
	}

	case Chess::PIECE_TYPE_KNIGHT: {
		valid = isKnightMovementValid(currentMove);
		break;
	}

	case Chess::PIECE_TYPE_BISHOP: {
		valid = isBishopMovementValid(currentMove);
		break;
	}

	case Chess::PIECE_TYPE_QUEEN: {
		valid = isQueenMovementValid(currentMove);
		break;
	}

	case Chess::PIECE_TYPE_KING: {
		valid = isKingMovementValid(currentMove);
		break;
	}

	default: {
		cout << "!!!!Should not reach here. Invalid piece: " << char(piece) << "\n\n\n";
		break;
	}
	}

	// If it is a move in an invalid direction, do not even bother to check the rest
	if (!valid)
	{
		cout << "Piece is not allowed to move to that square\n";
		return false;
	}

	// 2. Is there another piece of the same color on the destination square?
	valid = isPieceColourValid(currentMove->getPresent(), currentMove->getFuture());

	// 3. Would the king be in check after the move?
	if (valid && currentGame->wouldKingBeInCheck(piece, currentMove)) {
		cout << "Move would put player's king in check\n";
		valid = false;
	}
	return valid;
}

void GameController::makeTheMove(Move* currentMove)
{
	char piece = currentGame->getPieceAtPosition(currentMove->getPresent().row, currentMove->getPresent().column);

	// Captured a piece?
	if (currentGame->isSquareOccupied(currentMove->getFuture().row, currentMove->getFuture().column))
	{
		char chAuxPiece = currentGame->getPieceAtPosition(currentMove->getFuture().row, currentMove->getFuture().column);

		if (Chess::getPieceColor(piece) != Chess::getPieceColor(chAuxPiece))
		{
			createNextMessage(Chess::describePiece(chAuxPiece) + " captured!\n");
		}
		else
		{
			cout << "Error. We should not be making this move\n";
			throw("Error. We should not be making this move");
		}
	}
	else if (currentMove->getEnPassant()->applied)
	{
		createNextMessage("Pawn captured by \"en passant\" move!\n");
	}

	if ((currentMove->getCastling()->applied))
	{
		createNextMessage("Castling applied!\n");
	}

	currentGame->movePiece(currentMove);
}

void GameController::newGame(void)
{
	if (currentGame)
	{
		delete currentGame;
	}

	currentGame = new Game();
}

void GameController::undoMove(void)
{
	if (!currentGame->isUndoPossible())
	{
		createNextMessage("Undo is not possible now!\n");
		return;
	}

	currentGame->undoLastMove();
	createNextMessage("Last move was undone\n");
}

void GameController::movePiece(void)
{
	Move currentMove;
	std::string record;

	// Get user input for the piece they want to move
	cout << "Choose piece to be moved. (example: A1 or b2): ";

	std::string move_from;
	getline(cin, move_from);

	if (move_from.length() > 2)
	{
		createNextMessage("You should type only two characters (column and row)\n");
		return;
	}
	Chess::Position present;
	present.column = move_from[0];
	present.row = move_from[1];

	// Did the user pick a valid piece?
	// Must check if:
	// - It's inside the board (A1-H8)
	// - There is a piece in the square
	// - The piece is consistent with the player's turn
	present.column = toupper(present.column);

	if (present.column < 'A' || present.column > 'H')
	{
		createNextMessage("Invalid column.\n");
		return;
	}
	if (present.row < '0' || present.row > '8')
	{
		createNextMessage("Invalid row.\n");
		return;
	}

	// Put in the string to be logged
	record += present.column;
	record += present.row;
	record += "-";

	// Convert column from ['A'-'H'] to [0x00-0x07]
	present.column = present.column - 'A';

	// Convert row from ['1'-'8'] to [0x00-0x07]
	present.row = present.row - '1';

	char piece = currentGame->getPieceAtPosition(present.row, present.column);
	cout << "Piece is " << char(piece) << "\n";

	if (piece == EMPTY_SQUARE)
	{
		createNextMessage("You picked an EMPTY square.\n");
		return;
	}

	if (Chess::WHITE_PIECE == currentGame->getCurrentTurn())
	{
		if (!Chess::isWhitePiece(piece))
		{
			createNextMessage("It is WHITE's turn and you picked a BLACK piece\n");
			return;
		}
	}
	else
	{
		if (!Chess::isBlackPiece(piece))
		{
			createNextMessage("It is BLACK's turn and you picked a WHITE piece\n");
			return;
		}
	}
	currentMove.setPresent(present);

	// Get user input for the square to move to
	cout << "Move to: ";
	std::string move_to;
	getline(cin, move_to);

	if (move_to.length() > 2)
	{
		createNextMessage("You should type only two characters (column and row)\n");
		return;
	}

	// Did the user pick a valid house to move?
	// Must check if:
	// - It's inside the board (A1-H8)
	// - The move is valid
	Chess::Position future;
	future.column = move_to[0];
	future.row = move_to[1];

	future.column = toupper(future.column);

	if (future.column < 'A' || future.column > 'H')
	{
		createNextMessage("Invalid column.\n");
		return;
	}

	if (future.row < '0' || future.row > '8')
	{
		createNextMessage("Invalid row.\n");
		return;
	}

	// Put in the string to be logged
	record += future.column;
	record += future.row;

	// Convert columns from ['A'-'H'] to [0x00-0x07]
	future.column = future.column - 'A';

	// Convert row from ['1'-'8'] to [0x00-0x07]
	future.row = future.row - '1';

	// Check if it is not the exact same square
	if (future.row == present.row && future.column == present.column)
	{
		createNextMessage("[Invalid] You picked the same square!\n");
		return;
	}
	currentMove.setFuture(future);

	// Is that move allowed?

	if (!isMoveValid(&currentMove))
	{
		createNextMessage("[Invalid] Piece can not move to that square!\n");
		return;
	}

	// Promotion: user most choose a piece to
	// replace the pawn
	if (currentMove.getPromotion()->applied)
	{
		cout << "Promote to (Q, R, N, B): ";
		std::string piece;
		getline(cin, piece);

		if (piece.length() > 1)
		{
			createNextMessage("You should type only one character (Q, R, N or B)\n");
			return;
		}

		char promoted = toupper(piece[0]);

		if (promoted != 'Q' && promoted != 'R' && promoted != 'N' && promoted != 'B')
		{
			createNextMessage("Invalid character.\n");
			return;
		}
		Chess::Promotion promotion;
		promotion.applied = true;
		promotion.before = currentGame->getPieceAtPosition(present.row, present.column);

		if (Chess::WHITE_PLAYER == currentGame->getCurrentTurn())
		{
			promotion.after = toupper(promoted);
		}
		else
		{
			promotion.after = tolower(promoted);
		}
		currentMove.setPromotion(&promotion);

		record += '=';
		record += toupper(promoted); // always log with a capital letter
	}

	// Log the move: do it prior to making the move
	// because we need the getCurrentTurn()
	currentGame->logMove(record);

	// Make the move
	makeTheMove(&currentMove);

	// Check if this move we just did put the oponent's king in check
	// Keep in mind that player turn has already changed
	if (currentGame->isPlayerKingInCheck())
	{
		if (currentGame->isCheckMate())
		{
			if (Chess::WHITE_PLAYER == currentGame->getCurrentTurn())
			{
				appendToNextMessage("Checkmate! Black wins the game!\n");
			}
			else
			{
				appendToNextMessage("Checkmate! White wins the game!\n");
			}
		}
		else
		{
			// Add to the string with '+=' because it's possible that
			// there is already one message (e.g., piece captured)
			if (Chess::WHITE_PLAYER == currentGame->getCurrentTurn())
			{
				appendToNextMessage("White king is in check!\n");
			}
			else
			{
				appendToNextMessage("Black king is in check!\n");
			}
		}
	}
	return;
}

void GameController::saveGame(void)
{
	string file_name;
	cout << "Type file name to be saved (no extension): ";

	getline(cin, file_name);
	file_name += ".dat";

	std::ofstream ofs(file_name);
	if (ofs.is_open())
	{
		// Write the date and time of save operation
		auto time_now = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(time_now);
		ofs << "[Chess console] Saved at: " << std::ctime(&end_time);

		// Write the moves
		for (unsigned i = 0; i < currentGame->rounds.size(); i++)
		{
			ofs << currentGame->rounds[i].whiteMove.c_str() << " | " << currentGame->rounds[i].blackMove.c_str() << "\n";
		}

		ofs.close();
		createNextMessage("Game saved as " + file_name + "\n");
	}
	else
	{
		cout << "Error creating file! Save failed\n";
	}
	return;
}

void GameController::loadGame(void)
{
	string file_name;
	cout << "Type file name to be loaded (no extension): ";

	getline(cin, file_name);
	file_name += ".dat";

	std::ifstream ifs(file_name);

	if (ifs)
	{
		// First, reset the pieces
		if (currentGame)
		{
			delete currentGame;
		}
		currentGame = new Game();

		// Now, read the lines from the file and then make the moves
		std::string line;

		while (std::getline(ifs, line))
		{
			// Skip lines that starts with "[]"
			if (0 == line.compare(0, 1, "["))
			{
				continue;
			}

			// There might be one or two moves in the line
			string loaded_move[2];

			// Find the separator and subtract one
			std::size_t separator = line.find(" |");

			// For the first move, read from the beginning of the string until the separator
			loaded_move[0] = line.substr(0, separator);

			// For the second move, read from the separator until the end of the string (omit second parameter)
			loaded_move[1] = line.substr(line.find("|") + 2);

			for (int i = 0; i < 2 && loaded_move[i] != ""; i++)
			{
				Move currentMove;
				// Parse the line
				Chess::Position from;
				Chess::Position to;

				char promoted = 0;

				currentGame->parseMove(loaded_move[i], &from, &to, &promoted);

				// Check if line is valid
				if (from.column < 0 || from.column > 7 ||
					from.row < 0 || from.row    > 7 ||
					to.column < 0 || to.column   > 7 ||
					to.row < 0 || to.row      > 7)
				{
					createNextMessage("[Invalid] Can't load this game because there are invalid lines!\n");

					// Clear everything and return
					currentGame = new Game();
					return;
				}
				currentMove.setPresent(from);
				currentMove.setFuture(to);
				// Is that move allowed? (should be because we already validated before saving)

				if (!isMoveValid(&currentMove))
				{
					createNextMessage("[Invalid] Can't load this game because there are invalid moves!\n");

					// Clear everything and return
					currentGame = new Game();
					return;
				}

				// A promotion occurred
				if (currentMove.getPromotion()->applied)
				{
					if (promoted != 'Q' && promoted != 'R' && promoted != 'N' && promoted != 'B')
					{
						createNextMessage("[Invalid] Can't load this game because there is an invalid promotion!\n");

						// Clear everything and return
						currentGame = new Game();
						return;
					}
					Chess::Promotion promotion;
					promotion.applied = true;
					promotion.before = currentGame->getPieceAtPosition(from.row, from.column);

					if (Chess::WHITE_PLAYER == currentGame->getCurrentTurn())
					{
						promotion.after = toupper(promoted);
					}
					else
					{
						promotion.after = tolower(promoted);
					}
					currentMove.setPromotion(&promotion);
				}

				// Log the move
				currentGame->logMove(loaded_move[i]);

				// Make the move
				makeTheMove(&currentMove);
			}
		}

		// Extra line after the user input
		createNextMessage("Game loaded from " + file_name + "\n");
		return;
	}
	else
	{
		createNextMessage("Error loading " + file_name + ". Creating a new game instead\n");
		currentGame = new Game();
		return;
	}
}
