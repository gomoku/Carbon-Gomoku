/*
 *  AICarbon.h
 *  08.01.2002, Michal Czardybon
 *
 *  Tablica transpozycji
 *
 */ 

#ifndef _AICARBONHASH
#define _AICARBONHASH

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "OXTypes.h"

#include "hashValA.cpp"
#include "hashValB.cpp"
#include "hashValC.cpp"

extern int info_max_memory;

class HashTable
{
  public:
    
    // kasowanie wszystkiego
    void clear()
    {
      hashASize = 0;
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

    void resize(ULONG size)
    {
      if(size > hashASize)
      {
        ULONG maxBytes = info_max_memory;
        if(maxBytes == 0) maxBytes = 1000000000; //1GB
        maxBytes = __max(maxBytes, 7500000) - 7000000;
        ULONG num = maxBytes / sizeof(HashRec);

        if(maxSize * 2 < num || maxSize > num){
          elem = (HashRec*)realloc(elem, maxBytes);
          maxSize = num;
        }
        if(hashASize < maxSize){
          hashASize = __min(size * 2, maxSize);
          memset(elem, 0, hashASize * sizeof(HashRec));
        }
      }
    }

    HashTable()
    {
      maxSize = 0;
      hashASize = 0;
      elem = 0;
    }

    ~HashTable()
    {
      if(elem) free(elem);
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

    ULONG hashASize, maxSize;
    ULONG   hashA, hashB, hashC; // sygnatura biezacej sytuacji
    HashRec* elem;     // tablica
};

#endif