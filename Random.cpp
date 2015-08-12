#include "Random.h"

#include <stdlib.h>
#include <time.h>

void _randomize()
{
  time_t t;
  time(&t);
  srand(t);
}

int  _random(int x)
{
  return x * rand() / RAND_MAX;
}
