/*
 *  AISimple.h
 *  24,27,30.11.2001, Michal Czardybon
 *
 *  Prosty algorytm sztucznej inteligencji.
 *  - zadnego przeszukiwania
 *  - rozpoznawanie niekrytych czworek i trojek
 *  - punktowanie kazdego pola i wybor najlepszego
 *  
 *  PUNKTY POLA = 
 *    SUMA dla kazdego gracza
 *      SUM dla 4 kierunkow
 *        KWADRATOW
 *          SUM dla kazdej mozliwej piatki
 *            LICZB juz ulozonych znakow tej piatki
 *
 */

#ifndef _AISIMPLE
#define _AISIMPLE

#include "OXPlayer.h"
#include "OXGame.h"    // dla uproszczenia korzystam z gotowej planszy
#include "OXTypes.h"

class AISimple : public OXPlayer
{
  public:
    const int INF;
    
    AISimple() : INF(30000) {}
    
    void start(int size);
    void move(int x, int y);
    void yourTurn(int &x, int &y, int depth = 0, int time = 0);
    void undo(); 
    const char* name() const    {return "AI - Simple (24.11.2001)";}

  private:
    OXGame game;

    bool isCand(int x, int y);
    int  evalCell(int xp, int yp, OXPiece who);
    int  count(int xp, int yp, int dx, int dy, OXPiece piece);
};

#endif
