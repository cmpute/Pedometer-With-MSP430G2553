#ifndef TIMING
#define TIMING

#include "universal.h"

#define TARv 11430 //1s对应的TA0R数，根据实际电压需要调节  //MSP430VCC供电时，VCLOCK 实测为11430Hz

#define Beeptime 32  //蜂鸣器响的时间 (x0.01s)
#define Sleeptime 10  //无响应待机的时间 (x1s)
#define AutoSavetime 30 //自动保存的时间 (x1s)

extern t_byte sec;
extern t_byte min;
extern t_byte hour;
extern t_byte SleepFLG;   
extern t_byte MTimeSuspFLG;  
extern t_byte STimeSuspFLG;  

extern t_byte STsec;
extern t_byte STdsec;
#ifdef IsAutoSave
extern t_byte delaySave;
#endif

void TAFlagProcess();
void SleepFlagProcess();
void TimerInit();
void Beep();
void Wake();

#endif