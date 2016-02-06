#include "pisqpipe.h"
#include <windows.h>
#include <stdio.h>
#include "AICarbon.h"

const char *infotext="name=\"Carbon\", author=\"Michal Czardybon\", version=\"2.2\", country=\"Poland\", www=\"http://mczard.republika.pl/gomoku.en.html\"";

AICarbon *ai;


void brain_init()
{
  if(width<5 || height < 5){
    pipeOut("ERROR Invalid size of the board");
    return;
  }
  if(width > MAX_BOARD_WIDTH){
    pipeOut("ERROR Maximal board width is %d", MAX_BOARD_WIDTH);
    return;
  }
  if(height > MAX_BOARD_HEIGHT){
    pipeOut("ERROR Maximal board height is %d", MAX_BOARD_HEIGHT);
    return;
  }
  brain_restart();
}

void brain_restart()
{
  if(!ai) ai = new AICarbon();
  ai->start(width, height);
  pipeOut("OK");
}

void brain_my(int x, int y)
{
  ai->setWho(XP);
  ai->move(x, y);
}

void brain_opponents(int x, int y)
{
  ai->setWho(OP);
  ai->move(x, y);
}

void brain_block(int x, int y)
{
  ai->block(x, y);
}

int brain_takeback(int x, int y)
{
  return ai->undo(x, y);
}

void brain_turn()
{
  int x, y;
  ai->setWho(XP);
  ai->yourTurn(x, y);
  ai->move(x, y);
  pipeOut("%d,%d", x, y);
}

void brain_end()
{
  delete ai;
}

void WriteLog(int points, int nSearched, int speed, int depth, bool debug)
{
  pipeOut("%s eval %d, depth %d, nodes %d, speed %d", debug ? "DEBUG" : "MESSAGE", 
    points, depth, nSearched, speed);
}

long getTime()
{ 
  return (long)GetTickCount();
}

#ifdef DEBUG_EVAL

void brain_eval(int x, int y)
{
  ai->eval(x, y);
}

static HWND wnd = FindWindowA("Piskvork", 0);

static void vprint(int x, int y, int w, char *format, va_list va)
{
  HDC dc = GetDC(wnd);
  RECT rc;
  rc.top = y;
  rc.bottom = y+16;
  rc.left = x - w/2;
  rc.right = rc.left + w;
  char buf[128];
  int n = vsprintf(buf, format, va);
  SetTextAlign(dc, TA_CENTER);
  ExtTextOutA(dc, x, y, ETO_OPAQUE, &rc, buf, n, 0);
  ReleaseDC(wnd, dc);
}

static void printXY(int x, int y, int w, char *format, ...)
{
  va_list va;
  va_start(va, format);
  vprint(x, y, w, format, va);
  va_end(va);
}

void AICarbon::eval(int x, int y)
{
  OXCell *c = &cell[x+4][y+4];
  printXY(300, 0, 60, " [%d,%d,%d,%d],[%d,%d,%d,%d] ", 
    c->status1[0][0], c->status1[1][0], c->status1[2][0], c->status1[3][0], 
    c->status1[0][1], c->status1[1][1], c->status1[2][1], c->status1[3][1]);
  printXY(300, 16, 60, "%d,%d", c->status4[0], c->status4[1]);
  printXY(300, 32, 60, "%d", c->prior());
}
#endif
