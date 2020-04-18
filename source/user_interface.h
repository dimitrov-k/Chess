#pragma once
#include "chess.h"
#include "game.h"

#define WHITE_SQUARE 0xDB
#define BLACK_SQUARE 0xFF
#define EMPTY_SQUARE 0x20

void createNextMessage(string msg);
void appendToNextMessage(string msg);
void clearScreen(void);
void printLogo(void);
void printLogo(void);
void printMenu(void);
void printMessage(void);
void printLine(int line, int color1, int color2, Game& game);
void printSituation(Game& game);
void printBoard(Game& game);
