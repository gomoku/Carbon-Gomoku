/*
 *  OXTypes.h
 *  Michal Czardybon
 *
 *  Wpolne typy danych
 *
 */

#ifndef _OXTYPES
#define _OXTYPES

// maksymalny rozmiar planszy
const int MAX_BOARD_SIZE = 32;

// maksymalna liczba pol / ruchow
const int MAX_CELLS = MAX_BOARD_SIZE * MAX_BOARD_SIZE;

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

// wektory dla 4 kierunkow
const int DX[4] = {1, 0, 1, 1};
const int DY[4] = {0, 1, 1, -1};

// wspolrzedne na planszy
struct OXPoint 
{
  OXPoint(int _x = 0, int _y = 0) : x(_x), y(_y) {}
  bool operator == (const OXPoint &p) {return x == p.x && y == p.y;}
  unsigned char x, y;
};   

// zawartosc pola na planszy
enum OXPiece {EMPTY = 2, OP = 0, XP = 1, WRONG = 3};         

// makro zwracajace symbol przeciwnika
#define OPPONENT(x) ((x) == OP ? XP : OP)

#endif