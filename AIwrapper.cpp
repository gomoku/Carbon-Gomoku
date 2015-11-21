#include "pisqpipe.h"
#include <windows.h>
#include "AICarbon.h"

#define MATCH_SPARE 7      //how much is time spared for the rest of game
#define TIMEOUT_PREVENT 5  //how much is alfabeta slower when the depth is increased

const char *infotext="name=\"Carbon\", author=\"Michal Czardybon\", version=\"2.1\", country=\"Poland\", www=\"http://mczard.republika.pl/gomoku.en.html\"";

AICarbon *ai;


void brain_init()
{
  if(width<5 || width>MAX_BOARD_SIZE || height!=width){
    pipeOut("ERROR size of the board");
    return;
  }
  brain_restart();
}

void brain_restart()
{
  delete ai;
  ai = new AICarbon();
  ai->start(width);
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

DWORD stopTime()
{
  return start_time + min(info_timeout_turn, info_time_left/MATCH_SPARE)-30;
}

void brain_turn()
{
  int x, y, x1, y1, i;
  DWORD t0, t1;

  for(i=2; i<=50; i++){
    t0=GetTickCount();
    ai->yourTurn(x1, y1, i, 0);
    if(terminateAI && i>4) break;
    x=x1; y=y1;
    t1=GetTickCount();
    if(terminateAI || t1+TIMEOUT_PREVENT*(t1-t0)>=stopTime()) break;
  }
  ai->move(x, y);
  pipeOut("%d,%d", x, y);
}

void brain_checkTimeout()
{
  if(GetTickCount()>stopTime()){
    terminateAI=2;
  }
}

void brain_end()
{
  delete ai;
}

void WriteLog(int points, int nSearched, int speed)
{
}
