/*
 *  AICarbon.h
 *  08.01.2002, Michal Czardybon
 *
 *  Tablica transpozycji
 *
 */ 

#ifndef _AICARBONHASH
#define _AICARBONHASH

#include "OXTypes.h"

#include "hashValA.cpp"
#include "hashValB.cpp"
#include "hashValC.cpp"

//const ULONG hashASize = 524288;
const ULONG hashASize = 65536;

class HashTable
{
  public:
    
    // kasowanie wszystkiego
    void clear()
    {
      for (int i = 0; i < hashASize; i++)
        {
          elem[i].depth =
          elem[i].moves =
          elem[i].value = 0;
          elem[i].hashB = 
          elem[i].hashC = 0;
        }
    }
     
    // czy aktualna pozycja jest zapamietana
    bool present()
    {
      return elem[hashA].hashB == hashB &&
             elem[hashA].hashC == hashC;
    }

    // uwzglednij wykonanie ruchu
    void move(int x, int y, OXPiece who)
    {
      int offset = x + (y << 5) + (who << 10);
      hashA = (hashA + hashValA[offset]) % hashASize;
      hashB = (hashB + hashValB[offset]);
      hashC = (hashC + hashValC[offset]);
    }

    // cofanie <move>
    void undo(int x, int y, OXPiece who)
    {
      int offset = x + (y << 5) + (who << 10);
      hashA = (hashA - hashValA[offset]) % hashASize;
      hashB = (hashB - hashValB[offset]);
      hashC = (hashC - hashValC[offset]);
    }

    short   value() {return elem[hashA].value;} // wartosc pozycji
    short   depth() {return elem[hashA].depth;} // glebokosc na jaka zostala przeszukana 
    short   moves() {return elem[hashA].moves;} // ktory to ruch od poczatku gry
    OXPoint best()  {return elem[hashA].best;}  // najlepszy ruch w tej sytuacji

    // zapamietuje biezaca sytuacje
    void   update(short _value, short _depth, short _moves, OXPoint _best)
    {
      elem[hashA].value = _value;
      elem[hashA].depth = _depth;  
      elem[hashA].moves = _moves;
      elem[hashA].hashB = hashB;
      elem[hashA].hashC = hashC;
      elem[hashA].best  = _best;
    }

  private:
    struct HashRec
    {
      ULONG   hashB, hashC;
      short   value;
      short   depth;
      short   moves;
      OXPoint best;
    };

    ULONG   hashA, hashB, hashC; // sygnatura biezacej sytuacji
    HashRec elem[hashASize];     // tablica
};

#endif