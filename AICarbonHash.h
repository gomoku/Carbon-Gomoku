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
#include <assert.h>
#include "OXTypes.h"

extern const ULONG hashValA[4096], hashValB[4096], hashValC[4096];
extern int info_max_memory;

class HashTable
{
private:

  struct HashRec
  {
    ULONG   hashB, hashC;
    short   value;
    short   depth;
#ifndef NDEBUG
    short   moves;
#endif
    OXPoint best;
  };

public:
    
    // kasowanie wszystkiego
    void clear()
    {
      hashASize = 0;
    }
     
    // czy aktualna pozycja jest zapamietana
    bool present()
    {
      return currentItem->hashB == hashB && currentItem->hashC == hashC;
    }

    // uwzglednij wykonanie ruchu
    void move(int x, int y, OXPiece who)
    {
      int offset = x + (y << 6) + (who << 11);
      hashA = (hashA + hashValA[offset]) % hashASize;
      currentItem = &elem[hashA];
      hashB = (hashB + hashValB[offset]);
      hashC = (hashC + hashValC[offset]);
    }

    // cofanie <move>
    void undo(int x, int y, OXPiece who)
    {
      int offset = x + (y << 6) + (who << 11);
      hashA = ((int)(hashA - hashValA[offset]) % (int)hashASize);
      if((int)hashA < 0) hashA += hashASize;
      currentItem = &elem[hashA];
      hashB = (hashB - hashValB[offset]);
      hashC = (hashC - hashValC[offset]);
    }

    short   value() {return currentItem->value; } // wartosc pozycji
    short   depth() {return currentItem->depth;} // glebokosc na jaka zostala przeszukana 
#ifndef NDEBUG
    short   moves() {return currentItem->moves;} // ktory to ruch od poczatku gry
#endif
    OXPoint best()  {return currentItem->best;}  // najlepszy ruch w tej sytuacji

    // zapamietuje biezaca sytuacje
    void   update(short _value, short _depth, short _moves, OXPoint _best)
    {
      HashRec* c = currentItem;
      c->value = _value;
      c->depth = _depth;
#ifndef NDEBUG
      c->moves = _moves;
#endif
      c->hashB = hashB;
      c->hashC = hashC;
      c->best  = _best;
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

#ifndef NDEBUG
      for(int n = 0; n < 4096; n++) assert(hashValA[n] < 2000000000);
#endif
    }

    ~HashTable()
    {
      if(elem) free(elem);
    }

  private:
    HashRec* currentItem;
    ULONG hashASize, maxSize;
    ULONG   hashA, hashB, hashC; // sygnatura biezacej sytuacji
    HashRec* elem;     // tablica
};

#endif