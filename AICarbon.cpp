#include "AICarbon.h"
#include "OXLog.h"
#include "Timer.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const int AICarbon::WIN_MIN = 25000;
const int AICarbon::WIN_MAX = 30000;
const int AICarbon::INF     = 32000;
const int AICarbon::MAX_CAND = 256;

int   AICarbon::RANK[107];
int   AICarbon::PRIOR[256][256];
UCHAR AICarbon::STATUS4[10][10][10][10];

template <class T>
inline T max(T a, T b)
{
  return a > b ? a : b;
}
// ----------------------------------------------------------------------------
// Funkcje AI
// ----------------------------------------------------------------------------
// s1, s2, s3, s4 - status1 dla 4 kierunkow
UCHAR AICarbon::getStatus4(UCHAR s0, UCHAR s1, UCHAR s2, UCHAR s3)
{
  int n[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  n[s0]++; n[s1]++; n[s2]++; n[s3]++;

  if (n[9] >= 1) return A;              // OOOO_
  if (n[8] >= 1) return B;              // OOO_
  if (n[7] >= 2) return B;              // XOOO_ * _OOOX
  if (n[7] >= 1 && n[6] >= 1) return C; // XOOO_ * _OO
  if (n[7] >= 1 && n[5] >= 1) return D; // XOOO_ * _OOX
  if (n[7] >= 1 && n[4] >= 1) return D; // XOOO_ * _O
  if (n[7] >= 1) return E;              // XOOO_
  if (n[6] >= 2) return F;              // OO_ * _OO
  if (n[6] >= 1 && n[5] >= 1) return G; // OO_ * _OOX
  if (n[6] >= 1 && n[4] >= 1) return G; // OO_ * _O
  if (n[6] >= 1) return H;              // OO_
  
  return 0;
}
// ----------------------------------------------------------------------------
// cfg - 0..106 - numer konfiguracji
int AICarbon::getRank(char cfg)
{
  int mul[5] = {3, 7, 11, 15, 19}; // mcoix
  //int mul[5] = {0, 1, 4, 9, 16};
  return 
    mul[4] * COUNT5[cfg][4] +
    mul[3] * COUNT5[cfg][3] +
    mul[2] * COUNT5[cfg][2] +
    mul[1] * COUNT5[cfg][1] +
    mul[0] * COUNT5[cfg][0];
}
// ----------------------------------------------------------------------------
// a, b - wzor dla dwoch graczy
int AICarbon::getPrior(UCHAR a, UCHAR b)
{
  return _PRIOR[a][b];  
}
// ----------------------------------------------------------------------------
void AICarbon::init()
{
  int a, b, c, d;
  for (a = 0; a < 10; a++)
    for (b = 0; b < 10; b++)
      for (c = 0; c < 10; c++)
        for (d = 0; d < 10; d++)
          STATUS4[a][b][c][d] = getStatus4(a, b, c, d);
  for (a = 0; a < 107; a++)
    {
      RANK[a] = getRank(a);
    }
  for (a = 0; a < 256; a++)
    for (b = 0; b < 256; b++)
      PRIOR[a][b] = getPrior(a, b);
}
// ----------------------------------------------------------------------------
void AICarbon::yourTurn(int &x, int &y, int depth, int time)
{
  if(moveCount == 0){
    x = boardSize/2; y = boardSize/2;
    return;
  }

  Timer t;
  OXMove m(0, 0, 0);

  if (depth == 0) depth = 1;
  nSearched = 0;

  t.start();  
  m = minimax(depth, true, -INF, INF);
  t.stop();

  totalSearched += nSearched;
  
  OXPoint best = m;
  //if (moveCount == 1) best.x = 16;

  WriteLog(m.value, nSearched, (int)(nSearched / t.time()));

  x = best.x - 4;
  y = best.y - 4;
  assert(!(x < 0 || x >= boardSize || y < 0 || y >= boardSize));
}
// ----------------------------------------------------------------------------
int AICarbon::evaluate()
{
  int a, i, k, p[2] = {0, 0}, x, y;
  for (i = 0; i < moveCount; i++)
    {
      x = remMove[i].x;
      y = remMove[i].y;
      a = cell[ x ][ y ].piece;
      for (k = 0; k < 4; k++)
        {
          p[a] += RANK[ CONFIG[ cell[x][y].pattern[k][a] ][ cell[x][y].pattern[k][1-a] ] ];
        }
    }
  return p[who] - p[opp];
}
// ----------------------------------------------------------------------------
void AICarbon::generateCand(OXCand *cnd, int &nCnd)
{
  int i, x, y;

  cnd[0].x = -1;
  nCnd = 0;
  
  // ustawiamy pierszego kandydata, jesli istnieje w tablicy transpozycji
  if (table.present() && table.depth() >= 0 && table.best().x != 0)
    {
      cnd[0].x = table.best().x;
      cnd[0].y = table.best().y;
      cnd[0].value = 10000;
      assert(cnd[0].x != 0 && cnd[0].y != 0);
      assert(cell[cnd[0].x][cnd[0].y].piece == EMPTY);
      nCnd = 1;
    }

  // reszta kandydatow
  FOR_EVERY_CAND(x, y)
    if (x != cnd[0].x || y != cnd[0].y)
      {
        cnd[nCnd].x = x;
        cnd[nCnd].y = y;
        cnd[nCnd].value = cell[x][y].prior();
        if (cnd[nCnd].value > 1) nCnd++;      // odrzucamy kiepskie mozliwosci
        assert(nCnd < MAX_CAND);
      }
  
  // makro zwracajace jednoelementowa liste - pole o statusie <st> gracza <plr>
  #define ONE_CAND(plr, st)\
    {\
      i = 0; while (cell[cnd[i].x][cnd[i].y].status4[plr] != st) i++;\
      cnd[0] = cnd[i];\
      nCnd = 1;\
      return;\
    }
 
  if (nSt[who][A] > 0) ONE_CAND(who, A); // ustawienie piatki  
  if (nSt[opp][A] > 0) ONE_CAND(opp, A); // blokowanie czworki
  if (nSt[who][B] > 0) ONE_CAND(who, B); // tworzy niekryta czworke

  // blokowanie trojki
  if (nSt[opp][B] > 0)
    {
      // ogranicza ruchy do 
      nCnd = 0;
      FOR_EVERY_CAND(x, y)
        if (cell[x][y].status4[who] >= E || cell[x][y].status4[opp] >= E)
          {
            cnd[nCnd].x = x;
            cnd[nCnd].y = y;
            cnd[nCnd].value = cell[x][y].prior();
            if (cnd[nCnd].value > 0) nCnd++;
          }
      return;          
    }
}
// ----------------------------------------------------------------------------
// zwraca:
// 0 - normalka
// dodatnia - liczba ruchow do wygranej
// ujemna   - minus liczba ruchow do przegranej
int AICarbon::quickWinSearch()
{
  int x, y, q;
  if (nSt[who][A] >= 1) return 1;   // istnieje ruch wygrywajacy
  if (nSt[opp][A] >= 2) return -2;  // nie mozna zablokowac
  if (nSt[opp][A] == 1)             // trzeba blokowac czworke
    {     
      FOR_EVERY_CAND(x, y)
        if (cell[x][y].status4[opp] == A)
          {
            _move(x, y);
            q = -quickWinSearch();
            undo();
            if (q < 0) q--; else if (q > 0) q++;
            return q;
          }
    }
  if (nSt[who][B] >= 1) return 3;   // tworzymy niekryta czworke
  if (nSt[who][C] >= 1)             // XOOO_ * _OO
    {
      // przeciwnika ratuje tylko ulozenie czworki psujacej ten uklad
      if (nSt[opp][B] == 0 && nSt[opp][C] == 0 && nSt[opp][D] == 0 && nSt[opp][E] == 0) return 5;
      // analizuj jeszcze jeden ruch
      FOR_EVERY_CAND(x, y)
        if (cell[x][y].status4[who] == C) 
          {
            _move(x, y);
            q = -quickWinSearch();
            undo();
            if (q > 0) return q + 1;
          }
    }
  if (nSt[who][F] >= 1)
    {
      // przeciwnika ratuje ulozenie czworki
      if (nSt[opp][B] == 0 && nSt[opp][C] == 0 && nSt[opp][D] == 0 && nSt[opp][E] == 0) return 5;
      // analizuj jeszcze jeden ruch - chyba jednak zbyt duzy koszt
/*      FOR_EVERY_CAND(x, y)
        if (cell[x][y].status4[who] == F) 
          {
            _move(x, y);
            q = -quickWinSearch();
            undo();
            if (q > 0) return q + 1;
          }*/
    }
  return 0;
}
// ----------------------------------------------------------------------------
// porownywanie dla qsort
inline int candComp(const void *a, const void *b)
{
  return ((OXCand*)b)->value - ((OXCand*)a)->value;
}
// ----------------------------------------------------------------------------
inline short AICarbon::OXCell::prior()
{
  return  PRIOR[ pattern[0][0] ][ pattern[0][1] ] +
          PRIOR[ pattern[1][0] ][ pattern[1][1] ] +
          PRIOR[ pattern[2][0] ][ pattern[2][1] ] +
          PRIOR[ pattern[3][0] ][ pattern[3][1] ] +
          PRIOR[ pattern[0][1] ][ pattern[0][0] ] +
          PRIOR[ pattern[1][1] ][ pattern[1][0] ] +
          PRIOR[ pattern[2][1] ][ pattern[2][0] ] +
          PRIOR[ pattern[3][1] ][ pattern[3][0] ] +
          (adj1 != 0);
}
// ----------------------------------------------------------------------------
// h - glebokosc, h = 0 -> zwraca ocene sytuacji
// root - czy to pierwsze wywolanie (true), czy rekurencyjne (false)
// alpha - dolne ograniczenie na punkty przeszukiwanych ruchow
// beta  - ograniczenie gorne
// Przedzial <alpha, beta> jest domkniety.
OXMove AICarbon::minimax(int h, bool root, int alpha, int beta)
{
  if (alpha > beta + 1) return OXMove(0, 0, beta + 1);
  OXMove best(0, 0, alpha - 1);
  
  int  i, x, y, value;

  static int cnt;
  if(--cnt<0){ cnt=1000; brain_checkTimeout(); }

  // szybkie rozpoznawanie zakonczenia
  int q = quickWinSearch();
  if (root == false && q != 0)
    {
      if (q > 0) return OXMove(0, 0, +WIN_MAX - q);
      if (q < 0) return OXMove(0, 0, -WIN_MAX - q);
    }
  else if (q == 1)
    {
      // wykonywanie ruchu wygrywajacego
      FOR_EVERY_CAND(x, y)
        if (cell[x][y].status4[who] == A)
          return OXMove(x, y, WIN_MAX - 1);
    }
  
  // ocenianie sytuacji
  if (h == 0)
    {
      return OXMove(0, 0, evaluate());
    }

  // lista kandydujacych ruchow
  OXCand  cnd[MAX_CAND]; // elementy
  int     nCnd;          // ich liczba

  // generowanie kandydatow
  generateCand(cnd, nCnd);
  if (root && nCnd == 1) return OXMove(cnd[0].x, cnd[0].y, 0);
  qsort(cnd, nCnd, sizeof(OXCand), candComp);
 
  // dla prawie pelnej planszy, nie odrzucamy zadnego kandydata
  if (nCnd == 0)
    {
      FOR_EVERY_CAND(x, y) cnd[nCnd++] = OXCand(x, y, 0);
    }

  // symulowanie ruchow
  for (i = 0; i < nCnd; i++)
    {
      _move(cnd[i].x, cnd[i].y);
      
      assert(best.value <= beta);
      
      if (table.present() && table.depth() == h - 1)
        {
          value = table.value();  // ten przypadek jest juz obliczony
        }
      else
        {
          short  vA, vB;
          OXMove m;

          // okno poszukiwania
          vA = -beta;              // Nie wazne czy ruch ma wartosc <beta>, czy +INF.
                                   // I tak zostanie odrzucony.
          vB = -(best.value + 1);  // Chcemy ruchow lepszych od dotychczas najlepszego.

          // poszerzenie okna uwzgledniajace zmiany punktacji ruchow wygrywajacych
          if (vB >= +WIN_MIN) vB++;
          if (vA <= -WIN_MIN) vA--;
      
          m = minimax(h - 1, false, vA, vB);
          value = -m.value;
     
          // zwyciestwo w k ruchach jest lepsze niz w k + 1 ruchach
          if (value >= +WIN_MIN) value--;
          if (value <= -WIN_MIN) value++;
      
          // wynik jest dokladny, tylko wtedy gdy miesci sie w oknie
          if (-vB <= value && value <= -vA && !terminateAI)
            table.update(value, h - 1, moveCount, m);
        }
      
      undo();
      
      // aktualizowanie najlepszego ruchu
      if (value > best.value)
        {
          best = OXMove(cnd[i].x, cnd[i].y, value);
          if (value > beta) return OXMove(best.x, best.y, beta + 1);
        }

      if(terminateAI) break;
    }
  return best;
}
// ----------------------------------------------------------------------------
