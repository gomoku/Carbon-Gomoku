#include "AISimple.h"
#include "Random.h"

#define NOTEMPTY(x, y) (game.cell(x, y) != EMPTY && game.cell(x, y) != WRONG)
#define FREE(x, y)     (game.cell(x, y) == who || game.cell(x, y) == EMPTY)

void AISimple::start(int size)
{
  _randomize();
  game.start(size);
}

void AISimple::move(int x, int y)
{
  game.move(x, y);
}

void AISimple::undo()
{
  game.undo();
}

void AISimple::yourTurn(int &x, int &y, int, int)
{
  int i, j;
  int val, valA, valB;
  int best = -1;
  //for (i = 0; i < 1000000; i++) for (j = 0; j < 100; j++);
  for (i = 0; i < game.boardSize(); i++)
    for (j = 0; j < game.boardSize(); j++)
      if (game.cell(i, j) == EMPTY && isCand(i, j) == true)
        {
          valA = evalCell(i, j, game.player());
          valB = evalCell(i, j, OPPONENT(game.player()));    
          val = valA + valB + _random(4);
          if (valA == 100000) {x = i; y = j; game.move(x, y); return;} // wygrywam!
          if (valB == 2000) val = 80000; // przeciwnik ma niekryta 3-ke
          if (valA == 2000) val = 90000; // mam niekryta 3-ke
          if (val > best)
            {
              x = i; y = j;
              best = val;
            }
        }
  game.move(x, y);
}

// Sprawdza, czy warto brac to pole pod uwage,
// tzn. czy ma obok siebie jakis znak
bool AISimple::isCand(int x, int y)
{
  return NOTEMPTY(x - 1, y - 1) ||
         NOTEMPTY(  x  , y - 1) ||
         NOTEMPTY(x + 1, y - 1) ||
         NOTEMPTY(x - 1,   y  ) ||
         NOTEMPTY(  x  ,   y  ) ||
         NOTEMPTY(x + 1,   y  ) ||
         NOTEMPTY(x - 1, y + 1) ||
         NOTEMPTY(  x  , y + 1) ||
         NOTEMPTY(x + 1, y + 1);
}

// Zwraca:
// 1. Normalnie:      0..1600
// 2. Niekryta trojka:   2000
// 3. Czworka:         100000
int AISimple::evalCell(int xp, int yp, OXPiece who)
{
  int x, y;
  int space1, space2;
  int k, i;
  int valTot = 0, valDir, valOne5;
  int n3;
  OXPiece opp = OPPONENT(who);
  for (k = 0; k < 4; k++)
    {
      valDir = 0;
      // obliczanie wolnej przestrzeni
      x = xp + DX[k]; y = yp + DY[k]; space1 = 0;
      while (FREE(x, y) && space1 < 4) space1++, x += DX[k], y += DY[k];
      x = xp - DX[k]; y = yp - DY[k]; space2 = 0;
      while (FREE(x, y) && space2 < 4) space2++, x -= DX[k], y -= DY[k];
      // przegladanie mozliwych 5-tek
      x = xp - DX[k] * space2;
      y = yp - DY[k] * space2;
      n3 = 0;
      for (i = 0; i < space1 + space2 - 3; i++)
        {
          valOne5 = count(x, y, DX[k], DY[k], who);
          x += DX[k]; y += DY[k];
          // szczegolne przypadki
          if (valOne5 == 4) {return 100000;}  // jest 4-ka!
          if (valOne5 == 3) n3++;             // zliczamy 3-ki
          valDir += valOne5;
        }
      if (n3 >= 2) valDir = 50; // mamy niekryta 3-ke!
      // sumowanie kwadratow liczby mozliwych 5-tek z kazdego kierunku
      valTot += valDir * valDir;
    }
  if (valTot >= 2000) return 2000; // wszystkie niekryte trojki rownouprawnione
  return valTot;
}

// zlicza znaki <p> od (x, y) w 5 polach w kierunku [dx, dy]
int AISimple::count(int x, int y, int dx, int dy, OXPiece p)
{
  int i, c = 0;
  for (i = 0; i < 5; i++)
    {
      if (game.cell(x, y) == p) c++;
      x += dx; y += dy;
    }
  return c;
}











