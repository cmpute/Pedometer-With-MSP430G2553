#include "timing.h"
#include "button.h"
#include "digitaltube.h"
#include "eeprom.h"

t_byte sec = 00;  //主表秒数
t_byte min = 00;  //主表分钟数
t_byte hour = 00; //主表小时数

t_byte STsec = 00;  //秒表秒数
t_byte STdsec = 00; //秒表分秒数

t_byte TA0FLG = 0;  //主表处理标志
t_byte TA1SFLG = 0; //秒表处理标志
t_byte BeepFLG = 0; //蜂鸣器延时标志
t_byte SleepFLG = Sleeptime;  //待机计时标志
t_byte MTimeSuspFLG = 0;      //计时暂停标志
t_byte STimeSuspFLG = 1;      //秒表暂停标志
#ifdef IsAutoSave
t_byte delaySave = AutoSavetime; //自动保存延时标志
#endif
void TimerInit()
{  
  //时钟设置  
  BCSCTL3 |= LFXT1S1;
  //TA0计时器设置，1s
  TA0CTL |= TACLR;
  TA0CTL |= TASSEL_1+ MC_1;
  TA0CCR0 = TARv; 
  TA0CCTL0 |= CCIE; 
  //TA1计时器设置，0.01s
  TA1CTL |= TACLR;
  TA1CTL |= TASSEL_1+ MC_1;
  TA1CCR0 = TARv/100;
  TA1CCTL0 |= CCIE; 
}

void TAFlagProcess()
{
  if(TA0FLG)
  {
    sec++;
    if(sec==60)
    {
      sec=0;
      min++;
      if(min==60)
      {
        min=0;
        hour++;
      }
    } 
    if(mode==0 || mode == 3 )
      ContentFresh();
    TA0FLG=0;
  }
  if(TA1SFLG)
  {
    STdsec++;
    if(STdsec >= 100)
    {
      STdsec=0;
      STsec++;
    }
    if(mode == 2)
      ContentFresh();
    TA1SFLG=0;
  }
}

//进入待机模式
void Sleep()
{
#ifdef IsSleep
  SendData(~0x80);//digidis[31]);
  P1OUT |= 0x0F; //显示“....”表示待机模式
  LPM4;
#endif
}

void SleepFlagProcess()
{
  if(!SleepFLG)
    Sleep();
}

//退出待机模式
void Wake()
{ /*由于不允许在中断中调用函数退出低功耗模式,因此这个函数无效*/ }

//TA0 : 1s中断,计时用
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0Intr()
{
  //如果主表没有暂停则置处理标志
  if(!MTimeSuspFLG)
    TA0FLG = 1;
  //待机计时标志处理
  if(SleepFLG && STimeSuspFLG)
    --SleepFLG;
#ifdef IsAutoSave
  if(!(--delaySave))
  {
    delaySave = AutoSavetime;
    SaveData();
  }
#endif
}

//TA1 : 0.01s中断,延时用
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1Intr()
{
  //按键防抖延时处理
  if(delayFLG1)
    delayFLG1--;
  if(delayFLG2)
    delayFLG2--;
  if(delayFLG3)
    delayFLG3--;
  //蜂鸣器延时处理
  if(BeepFLG)
    if(!(--BeepFLG))
      P1OUT |= BIT4;//时间到后关闭蜂鸣器
  //按键长按计时标志处理
  if(delayHold1)
    if(!(--delayHold1))
       K1Hold();
  if(delayHold2)
    if(!(--delayHold2))
       K2Hold(); 
  //如果秒表没有暂停则置处理标志
  if(!STimeSuspFLG)
    TA1SFLG = 1; 
  //ContentFresh();
}

//响蜂鸣器
void Beep()
{
  P1OUT &= ~BIT4;
  BeepFLG = Beeptime;
}