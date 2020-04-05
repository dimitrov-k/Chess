#pragma once
#include "includes.h"

class Chess
{
public:
   static int getPieceColor( char piece );

   static bool isWhitePiece( char piece );

   static bool isBlackPiece( char piece );

   static std::string describePiece( char piece );

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

   enum Side
   {
      QUEEN_SIDE = 2,
      KING_SIDE  = 3
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
      { 'R',  'N',  'B',  'Q',  'K',  'B',  'N',  'R' },
      { 'P',  'P',  'P',  'P',  'P',  'P',  'P',  'P' },
      { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
      { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
      { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
      { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
      { 'p',  'p',  'p',  'p',  'p',  'p',  'p',  'p' },
      { 'r',  'n',  'b',  'q',  'k',  'b',  'n',  'r' },
   };
};

class Game : Chess
{
public:
   Game();
   ~Game();

   void movePiece( Position present, Position future, Chess::EnPassant* S_enPassant, Chess::Castling* S_castling, Chess::Promotion* S_promotion );

   void undoLastMove();

   bool isUndoPossible();

   bool isCastlingAllowed( Side side, int color );

   char getPieceAtPosition( int row, int column );

   char getPieceAtPosition( Position position );

   char considerMove( int row, int column, IntendedMove* intendedMove = nullptr );

   UnderAttack underAttack( int row, int column, int color, IntendedMove* intendedMove = nullptr );

   bool isReachable( int row, int column, int color );

   bool isSquareOccupied( int row, int column );

   bool isPathFree( Position starting, Position finishing, int direction ); 

   bool canBeBlocked( Position starting, Position finishing, int direction );

   bool isCheckMate();

   bool isKingInCheck( int color, IntendedMove* intendedMove = nullptr );

   bool isPlayerKingInCheck( IntendedMove* intendedMove = nullptr );

   bool wouldKingBeInCheck( char piece, Position present, Position future, EnPassant* S_enPassant );

   Position findKing( int color );

   void changeTurns( void );

   bool isFinished( void );

   int getCurrentTurn( void );

   int getOpponentColor( void );

   void parseMove( string move, Position* from, Position* to, char* promoted = nullptr );

   void logMove( std::string &record );

   string getLastMove( void );

   void deleteLastMove( void );

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
   } m_undo;

   // Castling requirements
   bool castlingKingSideAllowed[2];
   bool castlingQueenSideAllowed[2];

   // Holds the current turn
   int  currentTurn;

   // Has the game finished already?
   bool gameFinished;
};
