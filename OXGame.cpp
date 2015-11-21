#include "OXGame.h"

OXGame::OXGame()
{
  _moveCount = 0;
  _boardSize = 0;
  _winner = EMPTY;
  _finished = true;
}

void OXGame::start(int bsize)
{
  _moveCount = 0;
  _boardSize = bsize;
  _winner = EMPTY;
  _finished = 0;
  // clear the board
  int i, j;
  for (i = 0; i < bsize; i++)
    for (j = 0; j < bsize; j++)
      _cell[i][j] = EMPTY;
  // first move
  _player = OP;
}

bool OXGame::move(int x, int y)
{
  if (_finished) return 1;
  _cell[x][y] = _player;
  _move[_moveCount].x = x;
  _move[_moveCount].y = y;
  _moveCount++;
  if (is5inRow(x, y))
    {
      _winner = _player;
      _finished = true;
    }
  if (_moveCount == _boardSize * _boardSize) _finished = true;
  _player = OPPONENT(_player);
  return _finished;
}

void OXGame::undo()
{
  _player = OPPONENT(_player);
  _moveCount--;
  int x, y;
  x = _move[_moveCount].x;
  y = _move[_moveCount].y;
  _cell[ x ][ y ] = EMPTY;
}

bool OXGame::is5inRow(int x, int y)
{
  #define bs _boardSize
  OXPiece a = _cell[x][y];
  int k, c1, c2, xx, yy;
  for (k = 0; k < 4; k++)
    {
      c1 = c2 = 0;
      for (xx = x + DX[k], yy = y + DY[k]; cell(xx, yy) == a; xx += DX[k], yy += DY[k]) c1++;
      for (xx = x - DX[k], yy = y - DY[k]; cell(xx, yy) == a; xx -= DX[k], yy -= DY[k]) c2++;
      if (c1 + c2 >= 4)
        {
          // wyznaczanie winPoint-ow
          xx = x - c2 * DX[k];
          yy = y - c2 * DY[k];
          for (int i = 0; i < 5; i++)
            {
              _winPoint[i] = OXPoint(xx, yy);
              xx += DX[k];
              yy += DY[k];
            }
          return true;
        }
    }
  return false;
  #undef bs
}

bool OXGame::isWinPoint(int x, int y) const 
{
  if (_winner != EMPTY)
    {
      OXPoint p(x, y);
      for (int i = 0; i < 5; i++) if (p == _winPoint[i]) return true;
    }
  return false;
}


