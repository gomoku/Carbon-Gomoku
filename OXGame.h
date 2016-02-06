/*
 *  OXGame.h
 *  Michal Czardybon
 *  16.07.2001, 29.10.2001, 24.11.2001, 26.11.2001,
 *  27.11.2001, 30.11.2001, 02.12.2001
 *
 *  Klasa przeprowadzajaca gre i wykrywajaca koniec.
 *
 */

#ifndef _OXGAME
#define _OXGAME

#include "OXTypes.h"

class OXGame
{
  public:
    OXGame();
    void    start(int bsize); // new game
    bool    move(int x, int y);    
    void    undo();
    OXPiece cell(int x, int y) const;
    bool    isWinPoint(int x, int y) const;
    int     moveCount()        const {return _moveCount;}
    int     boardSize()        const {return _boardSize;}
    OXPoint lastMove()         const {return _move[_moveCount - 1];}
    OXPiece player()           const {return _player;}
    bool    finished()         const {return _finished;}
    OXPiece winner()           const {return _winner;}
    
  private:
    OXPiece   _cell[MAX_BOARD_WIDTH][MAX_BOARD_HEIGHT];
    OXPiece   _player;
    OXPoint   _move[MAX_CELLS];
    int       _moveCount;
    int       _boardSize;
    bool      _finished;
    OXPiece   _winner;
    OXPoint   _winPoint[5];
    
    bool is5inRow(int x, int y);
};

inline OXPiece OXGame::cell(int x, int y) const
{
  if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize) return WRONG;
  return _cell[x][y];
}
#endif
