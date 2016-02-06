#include "AICarbon.h"
#include <assert.h>
#include <stdio.h>
#include "Random.h"
// ----------------------------------------------------------------------------
// Podstawowe funkcje
// ----------------------------------------------------------------------------
void AICarbon::start(int size)
{
  start(size, size);
}

void AICarbon::start(int width, int height)
{
  int x, y, xx, yy, k;
  UCHAR p;

  _randomize();

  boardWidth = width;
  boardHeight = height;
  // wypelnianie planszy wartosciami poczatkowymi
  for(y = 0; y < height + 8; y++)
    for(x = 0; x < width + 8; x++)
      {
        (x < 4 || y < 4 || x >= width + 4 || y >= height + 4) ?
          cell[x][y].piece = WRONG :
          cell[x][y].piece = EMPTY;

        for (k = 0; k < 4; k++)
          {
            cell[x][y].pattern[k][0] = cell[x][y].pattern[k][1] = 0;
          }
      } 
  // inicjowanie <pat>
  for(y = 4; y < height + 4; y++)
    for(x = 4; x < width + 4; x++)
      for (k = 0; k < 4; k++)
        {
          xx = x - DX[k]; yy = y - DY[k];
          for (p = 8; p != 0; p >>= 1)
            {
              if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][0] |= p;
              if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][1] |= p;             
              xx -= DX[k]; yy -= DY[k];
            }
          xx = x + DX[k]; yy = y + DY[k];
          for (p = 16; p != 0; p <<= 1)
            {
              if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][0] |= p;
              if (cell[xx][yy].piece == WRONG) cell[x][y].pattern[k][1] |= p;             
              xx += DX[k]; yy += DY[k];
            }           
        }
  // inicjowanie statusow
  for(y = 4; y < height + 4; y++)
    for(x = 4; x < width + 4; x++)
      {
        cell[x][y].update1(0);
        cell[x][y].update1(1);
        cell[x][y].update1(2);
        cell[x][y].update1(3);
        cell[x][y].update4();
        cell[x][y].adj1 = cell[x][y].adj2 = 0;
      }
  // rozne inicjacje
  int i, j;
  for (i = 0; i < 2; i++) for (j = 0; j < 9; j++) nSt[i][j] = 0;

  totalSearched = 0;
  who = OP;
  opp = XP;
  moveCount = remCount = 0;
  
  upperLeftCand = OXPoint(99, 99);
  lowerRightCand = OXPoint(0, 0);
  
  table.clear();
}
// ----------------------------------------------------------------------------
// xp, yp in <0, boardSize)
void AICarbon::move(int xp, int yp)
{
  table.resize(1);
  _move(xp + 4, yp + 4);
}
// ----------------------------------------------------------------------------
void AICarbon::setWho(OXPiece _who)
{
  who = _who;
  opp = OPPONENT(who);
}
// ----------------------------------------------------------------------------
inline void AICarbon::OXCell::update1(int k)
{
  status1[k][0] = STATUS1[ pattern[k][0] ][ pattern[k][1] ]; 
  status1[k][1] = STATUS1[ pattern[k][1] ][ pattern[k][0] ]; 
}
// ----------------------------------------------------------------------------
void AICarbon::OXCell::update4()
{
  status4[0] = STATUS4[ status1[0][0] ][ status1[1][0] ][ status1[2][0] ][ status1[3][0] ];
  status4[1] = STATUS4[ status1[0][1] ][ status1[1][1] ][ status1[2][1] ][ status1[3][1] ];
}
// ----------------------------------------------------------------------------
// xp, yp in <4, boardSize + 4)
void AICarbon::_move(int xp, int yp, bool updateHash)
{
  nSearched++;

  int x, y, k;
  UCHAR p;

  assert(check());
  nSt[0][cell[xp][yp].status4[0]]--;
  nSt[1][cell[xp][yp].status4[1]]--;
  
  cell[xp][yp].piece = who;
  remCell[remCount] = &cell[xp][yp];
  remMove[moveCount] = OXPoint(xp, yp);
  remULCand[remCount] = upperLeftCand;
  remLRCand[remCount] = lowerRightCand;
  moveCount++;
  remCount++;

  if(xp - 2 < upperLeftCand.x) upperLeftCand.x = __max(xp - 2, 4);
  if(yp - 2 < upperLeftCand.y) upperLeftCand.y = __max(yp - 2, 4);
  if(xp + 2 > lowerRightCand.x) lowerRightCand.x = __min(xp + 2, boardWidth + 3);
  if(yp + 2 > lowerRightCand.y) lowerRightCand.y = __min(yp + 2, boardHeight + 3);
  
  // modyfikowanie <pat> i <points>         
  for (k = 0; k < 4; k++)
    {
      x = xp; y = yp;
      for (p = 16; p != 0; p <<= 1)
        {
          x -= DX[k]; y -= DY[k];
          cell[x][y].pattern[k][who] |= p;
          if (cell[x][y].piece == EMPTY)// && (cell[x][y].adj1 || cell[x][y].adj2))
            {
              cell[x][y].update1(k);
              nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
              cell[x][y].update4();
              nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
      x = xp; y = yp;
      for (p = 8; p != 0; p >>= 1)
        {
          x += DX[k]; y += DY[k];
          cell[x][y].pattern[k][who] |= p;
          if (cell[x][y].piece == EMPTY)// && (cell[x][y].adj1 || cell[x][y].adj2))
            {
              cell[x][y].update1(k);
              nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
              cell[x][y].update4();
              nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
    }
    
  // dodawanie kandydatow
  cell[xp - 1][yp - 1].adj1++; cell[xp    ][yp - 1].adj1++; cell[xp + 1][yp - 1].adj1++;
  cell[xp - 1][yp    ].adj1++;                              cell[xp + 1][yp    ].adj1++;
  cell[xp - 1][yp + 1].adj1++; cell[xp    ][yp + 1].adj1++; cell[xp + 1][yp + 1].adj1++;
  cell[xp - 2][yp - 2].adj2++; cell[xp    ][yp - 2].adj2++; cell[xp + 2][yp - 2].adj2++;
  cell[xp - 2][yp    ].adj2++;                              cell[xp + 2][yp    ].adj2++;
  cell[xp - 2][yp + 2].adj2++; cell[xp    ][yp + 2].adj2++; cell[xp + 2][yp + 2].adj2++;

  // aktualizowanie mieszania
  if(updateHash) table.move(xp, yp, who);

  // zamiana graczy    
  who = OPPONENT(who);
  opp = OPPONENT(opp);

  assert(check());
}
// ----------------------------------------------------------------------------
void AICarbon::undo()
{
  int x, y, k;
  UCHAR p;
  int xp, yp;

  assert(check());

  moveCount--;
  remCount--;
  xp = remMove[moveCount].x;
  yp = remMove[moveCount].y;
  upperLeftCand = remULCand[remCount];
  lowerRightCand = remLRCand[remCount];

  OXCell* c = remCell[remCount];
  c->update1(0);
  c->update1(1);
  c->update1(2);
  c->update1(3);
  c->update4();

  nSt[0][c->status4[0]]++;
  nSt[1][c->status4[1]]++;
  
  assert(c->piece == OP || c->piece == XP);
  c->piece = EMPTY;
  
  // zamiana graczy    
  who = OPPONENT(who);
  opp = OPPONENT(opp);

  // aktualizowanie mieszania
  table.undo(xp, yp, who);

  // modyfikowanie <pat>
  for (k = 0; k < 4; k++)
    {
      x = xp; y = yp;
      for (p = 16; p != 0; p <<= 1)
        {
          x -= DX[k]; y -= DY[k];
          cell[x][y].pattern[k][who] ^= p;
          if (cell[x][y].piece == EMPTY)// && (cell[x][y].adj1 || cell[x][y].adj2))
            {
              cell[x][y].update1(k);
              nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
              cell[x][y].update4();
              nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
      x = xp; y = yp;
      for (p = 8; p != 0; p >>= 1)
        {
          x += DX[k]; y += DY[k];
          cell[x][y].pattern[k][who] ^= p;
          if (cell[x][y].piece == EMPTY)// && (cell[x][y].adj1 || cell[x][y].adj2))
            {
              cell[x][y].update1(k);
              nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
              cell[x][y].update4();
              nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
            }
        }
    }
  
  // usuwanie kandydatow
  cell[xp - 1][yp - 1].adj1--; cell[xp    ][yp - 1].adj1--; cell[xp + 1][yp - 1].adj1--;
  cell[xp - 1][yp    ].adj1--;                              cell[xp + 1][yp    ].adj1--;
  cell[xp - 1][yp + 1].adj1--; cell[xp    ][yp + 1].adj1--; cell[xp + 1][yp + 1].adj1--;
  cell[xp - 2][yp - 2].adj2--; cell[xp    ][yp - 2].adj2--; cell[xp + 2][yp - 2].adj2--;
  cell[xp - 2][yp    ].adj2--;                              cell[xp + 2][yp    ].adj2--;
  cell[xp - 2][yp + 2].adj2--; cell[xp    ][yp + 2].adj2--; cell[xp + 2][yp + 2].adj2--;

  assert(check());
}

int AICarbon::undo(int x, int y)
{
  if(moveCount > 0 && remMove[moveCount - 1].x == x + 4 && remMove[moveCount - 1].y == y + 4){
    undo();
    return 0;
  }
  return 1;
}
// ----------------------------------------------------------------------------
void AICarbon::block(int x, int y)
{
  int xp, yp, k;
  UCHAR p;

  xp = x+4; yp = y+4;

  assert(check());
  nSt[0][cell[xp][yp].status4[0]]--;
  nSt[1][cell[xp][yp].status4[1]]--;

  cell[xp][yp].piece = WRONG;
  remMove[moveCount] = OXPoint(xp, yp);
  moveCount++;

  // modyfikowanie <pat> i <points>         
  for(k = 0; k < 4; k++)
  {
    x = xp; y = yp;
    for(p = 16; p != 0; p <<= 1)
    {
      x -= DX[k]; y -= DY[k];
      cell[x][y].pattern[k][0] |= p;
      cell[x][y].pattern[k][1] |= p;
      if(cell[x][y].piece == EMPTY)
      {
        cell[x][y].update1(k);
        nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
        cell[x][y].update4();
        nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
      }
    }
    x = xp; y = yp;
    for(p = 8; p != 0; p >>= 1)
    {
      x += DX[k]; y += DY[k];
      cell[x][y].pattern[k][0] |= p;
      cell[x][y].pattern[k][1] |= p;
      if(cell[x][y].piece == EMPTY)
      {
        cell[x][y].update1(k);
        nSt[0][cell[x][y].status4[0]]--; nSt[1][cell[x][y].status4[1]]--;
        cell[x][y].update4();
        nSt[0][cell[x][y].status4[0]]++; nSt[1][cell[x][y].status4[1]]++;
      }
    }
  }

  // zamiana graczy    
  who = OPPONENT(who);
  opp = OPPONENT(opp);

  assert(check());
}
// ----------------------------------------------------------------------------
bool AICarbon::check()
{
  int n[2][9];
  int i, j, x, y;
  for (i = 0; i <= 1; i++) for (j = 0; j < 9; j++) n[i][j] = 0;
  FOR_EVERY_CAND(x, y)
    {
      n[0][cell[x][y].status4[0]]++;
      n[1][cell[x][y].status4[1]]++;
    }
  for (i = 0; i < 2; i++)
    for (j = 1; j < 9; j++)
      if (n[i][j] != nSt[i][j]) return false;
  return true;
}
// ----------------------------------------------------------------------------
static signed char data[] = {
  15, 1, 4, 0, 0, 0, 1, 1, 2, 3, 2, 2, 4, 2, 5, 3, 3, 4, 3, 3, 5, 5, 4, 3, 3, 6, 4, 5, 6, 5, 5, 4, 3, 1,
  11, 1, 1, 1, 0, 0, 2, 2, 3, 2, 3, 3, 1, 3, 2, 4, 1, 5, 3, 3, 3, 2, 5, 1, 4, 3,
  11, 1, 1, 0, 0, 0, 1, 1, 3, 2, 2, 2, 2, 3, 3, 3, 2, 4, 2, 5, 1, 5, 1, 4, 0, 3,
  9, 1, 0, 0, 0, 3, 0, 1, 1, 2, 0, 2, 2, 3, 3, 3, 3, 4, 2, 4, 1, 1,
  9, 1, 1, 1, 0, 0, 2, 2, 3, 2, 3, 3, 2, 3, 1, 4, 1, 3, 1, 5, 2, 4,
  9, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 2, 3, 1, 2, 0, 3, 1, 4,
  9, 1, 0, 0, 2, 0, 1, 0, 1, 1, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 0, 2,
  9, 1, 1, 1, 0, 0, 2, 2, 0, 2, 2, 1, 3, 1, 1, 2, 1, 3, 0, 3, 3, 0,
  9, 1, 0, 0, 2, 0, 1, 1, 1, 2, 2, 1, 1, 4, 3, 2, 4, 3, 2, 3, 0, 1,
  8, 1, 0, 2, 1, 1, 1, 2, 2, 0, 2, 2, 3, 0, 3, 1, 3, 2, 4, 0,
  8, 1, 1, 1, 3, 0, 0, 2, 3, 1, 1, 3, 2, 2, 2, 3, 1, 4, 3, 3,
  7, 1, 0, 0, 1, 1, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 0, 2,
  7, 1, 3, 2, 2, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1, 2,
  7, 1, 0, 0, 0, 1, 1, 0, 0, 3, 2, 1, 3, 2, 1, 2, 1, -1,
  7, 1, 1, 0, 0, 1, 0, 2, 2, 2, 2, 1, 3, 2, 5, 2, 3, 0,
  7, 1, 0, 0, 0, 1, 2, 0, 0, 3, 2, 1, 1, 3, 1, 2, 2, 3,
  7, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 2, 1, 2, 1, 3, 2, 3,
  7, 1, 0, 1, 1, 1, 2, 0, 0, 2, 1, 2, 3, 4, 2, 3, 2, 1,
  6, 1, 1, 0, 0, 0, 0, 2, 1, 1, 3, 3, 2, 2, 1, -1,
  6, 1, 1, 0, 0, 0, 1, 1, 2, 1, 2, 2, 1, 2, 0, -1,
  6, 1, 0, 0, 1, 0, 2, 1, 3, 1, 3, 2, 2, 2, 1, -1,
  6, 1, 3, 0, 0, 0, 1, 1, 2, 1, 2, 2, 1, 2, -1, 0,
  6, 1, 0, 0, 1, 1, 3, 1, 2, 2, 4, 3, 3, 3, 2, 4,
  6, 1, 0, 2, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 3, 0,
  6, 1, 2, 1, 0, 0, 2, 2, 0, 1, 2, 3, 1, 2, 2, 0,
  6, 1, 2, 1, 0, 0, 1, 3, 0, 1, 2, 3, 1, 2, 2, 0,
  5, 2, 2, 2, 0, 0, 1, 0, 1, 1, 0, 1, 2, 1, 1, 2,
  5, 1, 0, 0, 2, 1, 0, 1, 2, 3, 1, 2, 1, 3,
  5, 1, 1, 0, 0, 0, 1, 1, 1, 2, 2, 1, 1, -1,
  5, 1, 1, 1, 1, 0, 0, 2, 1, 3, 1, 2, 2, 2,
  5, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 3, 2, 2,
  5, 2, 0, 0, 1, 1, 1, 2, 0, 1, 2, 1, 2, 0, 3, 0,
  5, 1, 0, 2, 1, 0, 2, 0, 2, 1, 1, 1, -1, 3,
  5, 1, 1, 0, 1, 1, 0, 1, 2, 1, 3, 2, 2, -1,
  5, 1, 1, 1, 0, 0, 2, 1, 0, 2, 3, 1, 0, 1,
  5, 2, 1, 1, 0, 0, 2, 1, 1, 3, 1, 2, 0, 1, 3, 1,
  5, 2, 1, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 2, -1, 2,
  5, 1, 1, 0, 1, 2, 0, 1, 3, 2, 2, 1, 2, -1,
  5, 2, 0, 1, 1, 2, 2, 1, 3, 0, 0, 3, 1, 1, 0, 2,
  5, 1, 2, 0, 0, 2, 1, 1, 1, 2, 2, 2, 0, 0,
  5, 1, 0, 0, 1, 1, 1, 0, 1, 2, 0, 2, 0, 1,
  4, 1, 1, 0, 0, 1, 1, 3, 1, 2, -1, 0,
  4, 1, 1, 0, 0, 1, 1, 1, 1, 2, -1, 0,
  4, 1, 1, 0, 1, 1, 0, 2, 2, 2, 0, 0,
  4, 1, 2, 0, 0, 0, 0, 2, 1, 1, 2, 2,
  4, 1, 0, 0, 1, 1, 3, 1, 2, 2, 3, 3,
  4, 2, 2, 0, 0, 0, 2, 2, 1, 1, 0, -1, 1, -1,
  4, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2, 2,
  4, 1, 1, 0, 0, 1, 2, 0, 1, 2, -1, 0,
  4, 1, 1, 0, 0, 0, 0, 2, 1, 1, -1, -1,
  4, 1, 1, 0, 0, 1, 2, 2, 1, 2, -1, 0,
  4, 1, 0, 0, 1, 1, 2, 1, 2, 2, 3, 3,
  3, 1, 0, 1, 0, 0, 1, 0, 1, 1,
  3, 1, 0, 0, 2, 0, 1, 1, 2, 2,
  3, 4, 1, 0, 0, 1, 1, 2, 0, 2, 0, 0, 2, 0, 2, 2,
  3, 1, 1, 2, 0, 0, 1, 1, 1, 3,
  3, 1, 1, 0, 0, 0, 1, 1, 1, 2,
  3, 1, 0, 0, 0, 2, 0, 1, 0, -1,
  3, 1, 0, 0, 2, 1, 1, 1, 2, 2,
  3, 1, 0, 0, 0, 1, 1, 2, 1, 1,
  3, 2, 0, 0, 2, 2, 1, 1, 2, 1, 1, 2,
  3, 1, 0, 1, 3, 0, 2, 0, 1, 1,
  3, 1, 0, 0, 2, 1, 2, 2, 1, 1,
  3, 4, 0, 0, 1, 1, 2, 2, 1, -1, -1, 1, 3, 1, 1, 3,
  3, 1, 0, 0, 0, 2, 1, 2, 1, 0,
  3, 2, 1, 0, 0, 3, 1, 2, 2, 3, 1, 1,
  3, 2, 0, 0, 0, 3, 0, 2, 1, 3, -1, 3,
  3, 1, 0, 0, 0, 3, 1, 2, 1, 1,
  3, 1, 1, 0, 0, 2, 1, 2, 1, 1,
  3, 1, 0, 0, 2, 1, 1, 2, 0, 2,
  3, 1, 0, 0, 2, 2, 1, 2, 0, 1,
  3, 1, 0, 0, 3, 3, 2, 2, 1, 1,
  3, 1, 0, 0, 3, 2, 2, 2, 1, 1,
  3, 1, 0, 0, 3, 1, 2, 2, 3, 3,
  3, 6, 0, 0, 1, 0, 2, 0, 1, 1, 1, -1, 0, 2, 0, -2, 2, 2, 2, -2,
  3, 3, 0, 0, 3, 2, 2, 1, 1, 0, 1, 1, 0, 1,
  2, 3, 0, 0, 1, 1, 0, 2, 2, 0, 1, 2,
  2, 8, 0, 0, 1, 0, -1, -1, 0, -1, 1, -1, 2, -1, -1, 1, 0, 1, 1, 1, 2, 1,
  2, 2, 0, 0, 2, 2, 1, 3, 3, 1,
  1, 8, 0, 0, -1, 0, 1, 0, 0, 1, 0, -1, 1, 1, -1, 1, 1, -1, -1, -1,
  0, 0
};

bool AICarbon::databaseMove(int &x0, int &y0)
{
  signed char *s, *sn;
  int i, x, y, x1, y1, flip, len1, len2, left, top, right, bottom;

  //board rectangle
  left = upperLeftCand.x + 2;
  top = upperLeftCand.y + 2;
  right = lowerRightCand.x - 2;
  bottom = lowerRightCand.y - 2;
  //find current board in the database
  for(s = data;; s = sn){
    len1 = *s++;
    len2 = *s++;
    sn = s + 2 * (len1 + len2);
    if(len1 != moveCount){
      if(len1 < moveCount) return false; //data must be sorted by moveCount descending
      continue;
    }
    //try all symmetries
    for(flip = 0; flip < 8; flip++){
      for(i = 0;; i++){
        x1 = s[2 * i];
        y1 = s[2 * i + 1];
        if(i == len1){
          s += 2 * (len1 + _random(len2));
          x1 = *s++;
          y1 = *s;
        }
        switch(flip){
          case 0: x = left + x1; y = top + y1; break;
          case 1: x = right - x1; y = top + y1; break;
          case 2: x = left + x1; y = bottom - y1; break;
          case 3: x = right - x1; y = bottom - y1; break;
          case 4: x = left + y1; y = top + x1; break;
          case 5: x = right - y1; y = top + x1; break;
          case 6: x = left + y1; y = bottom - x1; break;
          default: x = right - y1; y = bottom - x1; break;
        }
        if(x - 4 < 0 || x - 4 >= boardWidth || y - 4 < 0 || y - 4 >= boardHeight) break;
        if(i == len1){
          x0 = x - 4; 
          y0 = y - 4;
          return true;
        }
        //compare current board and database
        if(cell[x][y].piece != ((i & 1) ? XP : OP)) break;
      }
    }
  }
}
