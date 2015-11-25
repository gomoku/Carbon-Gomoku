#include "pisqpipe.h"
#include <windows.h>
#include "AICarbon.h"

const char *infotext="name=\"Carbon\", author=\"Michal Czardybon\", version=\"2.1\", country=\"Poland\", www=\"http://mczard.republika.pl/gomoku.en.html\"";

AICarbon *ai;


void brain_init()
{
  if(width<5 || width>MAX_BOARD_SIZE || height<5 || height>MAX_BOARD_SIZE){
    pipeOut("ERROR size of the board");
    return;
  }
  brain_restart();
}

void brain_restart()
{
  delete ai;
  ai = new AICarbon();
  ai->start(width, height);
  pipeOut("OK");
}

void brain_my(int x, int y)
{
  ai->move(x, y);
}

void brain_opponents(int x, int y)
{
  ai->move(x, y);
}

void brain_block(int x, int y)
{
  pipeOut("ERROR brain_block not implemented");
}

int brain_takeback(int x, int y)
{
  return 1;
}

void brain_turn()
{
  int x, y;
  ai->yourTurn(x, y);
  ai->move(x, y);
  pipeOut("%d,%d", x, y);
}

void brain_end()
{
  delete ai;
}

void WriteLog(int points, int nSearched, int speed)
{
}
