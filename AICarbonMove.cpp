#include "AICarbon.h"
#include <assert.h>
#include <stdio.h>
// ----------------------------------------------------------------------------
// Podstawowe funkcje
// ----------------------------------------------------------------------------
void AICarbon::start(int bsize)
{
  int x, y, xx, yy, k, p;

  init();

  boardSize = bsize;
  // wypelnianie planszy wartosciami poczatkowymi
  for (y = 0; y < boardSize + 8; y++)
    for (x = 0; x < boardSize + 8; x++)
      {
        (x < 4 || y < 4 || x >= boardSize + 4 || y >= boardSize + 4) ?
          cell[x][y].piece = WRONG :
          cell[x][y].piece = EMPTY;

        for (k = 0; k < 4; k++)
          {
            cell[x][y].pattern[k][0] = cell[x][y].pattern[k][1] = 0;
          }
      } 
  // inicjowanie <pat>
  for (y = 4; y < boardSize + 4; y++)
    for (x = 4; x < boardSize + 4; x++)
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
  for (y = 4; y < boardSize + 4; y++)
    for (x = 4; x < boardSize + 4; x++)
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

  x = y = boardSize / 2 + 4;
  totalSearched = 0;
  who = OP;
  opp = XP;
  moveCount = 0;
  
  upperLeftCand = OXPoint(x - 1, y - 1);
  lowerRightCand = OXPoint(x + 1, y + 1);
  
  table.clear();
}
// ----------------------------------------------------------------------------
// xp, yp in <0, boardSize)
void AICarbon::move(int xp, int yp)
{
  _move(xp + 4, yp + 4);
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
void AICarbon::_move(int xp, int yp)
{
  nSearched++;

  int x, y, k;
  UCHAR p;

  assert(check());
  nSt[0][cell[xp][yp].status4[0]]--;
  nSt[1][cell[xp][yp].status4[1]]--;
  
  cell[xp][yp].piece = who;
  remMove[moveCount] = OXPoint(xp, yp);
  remULCand[moveCount] = upperLeftCand;
  remLRCand[moveCount] = lowerRightCand;
  moveCount++;

  if (xp < upperLeftCand.x + 2) upperLeftCand.x = xp - 2;
  if (yp < upperLeftCand.y + 2) upperLeftCand.y = yp - 2;
  if (xp > lowerRightCand.x - 2) lowerRightCand.x = xp + 2;
  if (yp > lowerRightCand.y - 2) lowerRightCand.y = yp + 2;
  
  if (upperLeftCand.x < 4) upperLeftCand.x = 4;
  if (upperLeftCand.y < 4) upperLeftCand.y = 4;
  if (lowerRightCand.x > boardSize + 3) lowerRightCand.x = boardSize + 3;
  if (lowerRightCand.y > boardSize + 3) lowerRightCand.y = boardSize + 3;

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
  table.move(xp, yp, who);

  // zamiana graczy    
  who = OPPONENT(who);
  opp = OPPONENT(opp);

  assert(check());
}
// ----------------------------------------------------------------------------
void AICarbon::undo()
{
  int x, y, k;
  unsigned char p;
  int xp, yp;

  assert(check());

  moveCount--;
  xp = remMove[moveCount].x;
  yp = remMove[moveCount].y;
  upperLeftCand = remULCand[moveCount];
  lowerRightCand = remLRCand[moveCount];

  cell[xp][yp].update1(0);
  cell[xp][yp].update1(1);
  cell[xp][yp].update1(2);
  cell[xp][yp].update1(3);
  cell[xp][yp].update4();

  nSt[0][cell[xp][yp].status4[0]]++;
  nSt[1][cell[xp][yp].status4[1]]++;
  
  cell[xp][yp].piece = EMPTY;
  
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
