//
//  file:     timer.h
//  author:   Michal Czardybon
//  version:  2.0
//  modified: 03-07-2001, 03.12.2001
//  descr.:   Time measurement.
//

#ifndef _TIMER
#define _TIMER

class Timer
{
  public:
    Timer() {reset();}
    void   start() { start_time = getTime(); }
    void   stop()  { _time += (double)(getTime() - start_time) / 1000; }
    void   reset() {_time = 0;}
    double time()  {return _time;}
  private:
    DWORD start_time;
    double  _time;
};

#endif
