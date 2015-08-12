//
//  file:     timer.h
//  author:   Michal Czardybon
//  version:  2.0
//  modified: 03-07-2001, 03.12.2001
//  descr.:   Time measurement.
//

#ifndef _TIMER
#define _TIMER

#include <time.h>

class Timer
{
  public:
    Timer() {reset();}
    void   start() {start_time = clock();}
    void   stop()  {_time += (double)(clock() - start_time) / CLOCKS_PER_SEC;}
    void   reset() {_time = 0;}
    double time()  {return _time;}
  private:
    clock_t start_time;
    double  _time;
};

#endif
