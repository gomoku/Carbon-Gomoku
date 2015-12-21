#include <windows.h>

// We can't use standard function rand() from stdlib because it does not work.
// It returns same value for every move because OXMain.cpp creates new thread for every move.

static DWORD seed;

void _randomize()
{
  seed = GetTickCount();
}

unsigned _random(unsigned x)
{
  seed = seed * 367413989 + 174680251;
  return (unsigned)(UInt32x32To64(x, seed) >> 32);
}
