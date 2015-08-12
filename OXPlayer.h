/*
 *  AIPlayer.h
 *  30.11.2001, Michal Czardybon
 *
 *  Klasa abstrakcyjna dla algorytmu sztucznej inteligencji
 *
 */

#ifndef _OXPlayer
#define _OXPlayer

class OXPlayer
{
  public:
    // nowa gra
    virtual void start(int size) = 0;
    
    // informuje AI o ruchu przeciwnika
    virtual void move(int x, int y) = 0;
    
    // polecenie wybrania ruchu przez AI
    virtual void yourTurn(int &x, int &y, int depth = 0, int time = 0) = 0;
    
    // nie wykorzystywane
    virtual void think() {};
    
    // nakazuje AI cofnac ruch
    virtual void undo() = 0;
    
    // zwraca nazwe AI
    virtual const char* name() const = 0;
};

#endif
