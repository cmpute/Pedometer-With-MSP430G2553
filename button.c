#include "button.h"
#include "digitaltube.h"
#include "timing.h"
#include "eeprom.h"

t_byte delayFLG1 = 0; //K1按键延时标志
t_byte delayFLG2 = 0; //K2按键延时标志
t_byte delayFLG3 = 0; //K3按键延时标志
t_byte delayHold1 = 0; //K1长按标志
t_byte delayHold2 = 0; //K2长按标志
t_byte SensorFLG = 0; //传感器处理标志
t_byte ClrFLG = 0; //第二次清空的标志
t_byte ClrDisp = 0; //在mode4下，0显示CLr,1显示CLrd,2显示SA'd,3显示CLrA

//初始化按键中断
void ButtonInit()
{
  P1SEL &= ~BIT67;
  P1SEL2 &= ~BIT67;
  P1DIR &= ~BIT67;
  P1REN |= BIT67;
  P1OUT |= BIT67;
  P1IE |= BIT67;
  P1IFG &= ~BIT67;
  P1IES |= BIT67;
  P2SEL &= BIT5;
  P2SEL2 &= ~BIT5;
  P2DIR &= ~BIT5;
  P2REN |= BIT5;
  P2OUT |= BIT5;
  P2IE |= BIT5;
  P2IFG &= ~BIT5;
  P2IES |= BIT5;
}
//处理传感器中断标志
void SensorFlagProcess()
{
  if(SensorFLG)
  {
    steps++;
    if(mode == 1 || mode == 3)
      ContentFresh();
    SensorFLG = 0;
  }
}
//判断按键是否继续长按
void ButtonHoldProcess()
{
  if(delayHold1)
    if(ReadOp(P2IN,5))
      delayHold1=0;
  if(delayHold2)
    if(ReadOp(P1IN,6))
      delayHold2=0;
}

//按键K1的响应函数
void K1()
{
  delayHold1 = ButtonHold; //开始长按计时
  switch(mode)
  {
  case 0:
    TimeOffset=!TimeOffset;
    ContentFresh();
    break;
  case 1:
    if(StepOffset>0)
    {
      StepOffset--;
      ContentFresh();
    }
    break;
  case 2:
    STsec=0;
    STdsec=0;
    ContentFresh();
    break;
  case 3:
    Stepmode = 1;
    ContentFresh();
    break;
  case 4:
    //SaveData();
    break;
  }
}
//按键K2的响应函数
void K2()
{
  delayHold2 = ButtonHold; //开始长按计时
  switch(mode)
  {
  case 0:
    MTimeSuspFLG = !MTimeSuspFLG; //暂停主表
    break;
  case 1:
    if(StepOffset<5)
    {
      StepOffset++;
      ContentFresh();
    }
    break;
  case 2:
    STimeSuspFLG = !STimeSuspFLG; //暂停秒表
    break;
  case 3:
    Stepmode = 0;
    ContentFresh();
    break;
  }
}
//按键K3的响应函数
void K3()
{
  mode=(mode+1)%5;
  switch(mode)
  {
  case 0:
    TimeOffset = 0;//时钟显示默认为mm.ss
    break;
  case 1:
    StepOffset = 0;//步数显示默认为最低位
    break;
  case 4:
    ClrDisp = 0;
    break;
  }
  ContentFresh();
}

//长按K1的响应函数
void K1Hold()
{
  Beep();
  switch(mode)
  {
  case 0: //清零计时
    sec=0;
    min=0;
    hour=0;
    TimeOffset=0;
    ContentFresh();
    break;
  case 1:
    steps=0;
    StepOffset=0;
    ContentFresh();
    break;
  case 4: //清零所有内容
    sec=0;
    min=0;
    hour=0;
    steps=0;
    STsec=0;
    STdsec=0;
#ifdef IsAutoSave
    delaySave=0;
#endif
    MTimeSuspFLG = 0; 
    STimeSuspFLG = 1;
    ClrDisp=1;
    if(ClrFLG)
    {
      SaveData();
      ClrDisp=3;
    }
    ClrFLG = 1;
    ContentFresh();
    break;
  }
}
//长按K2的响应函数
void K2Hold()
{
  if(mode == 4)
  {
    SaveData();
    ClrDisp=2;
    ContentFresh();
    Beep();
  }
}
//Port1中断子程
#pragma vector=PORT1_VECTOR
__interrupt void P1Intr()
{
  t_byte wakeflg = 1; //如果是按键唤醒时不执行按键功能
#ifdef IsSleep
  if(!SleepFLG)
  {
    LPM4_EXIT;//Wake();
    wakeflg = 0;
  }
#endif
  SleepFLG = Sleeptime;
  if(ReadOp(P1IFG,6))
  {
    SetOpFalse(P1IFG,6);
    if(wakeflg && (delayFLG2==0))
    {
      K2();
      delayFLG2 = ButtonDelay;
    }
  }
  if(ReadOp(P1IFG,7))
  {
    SetOpFalse(P1IFG,7);
    if(wakeflg && (delayFLG3==0))
    {
      K3();
      delayFLG3 = ButtonDelay;
    }
  }
}

//Port2中断子程
#pragma vector=PORT2_VECTOR
__interrupt void P2Intr()
{
  t_byte wakeflg = 1; //如果是按键唤醒时不执行按键功能
#ifdef IsSleep
  if(!SleepFLG)
  {
    LPM4_EXIT;//Wake();
    wakeflg = 0;
  }
#endif
  SleepFLG = Sleeptime;
  if(ReadOp(P2IFG,0))
  {
    //传感器中断
    SetOpFalse(P2IFG,0);
    SensorFLG = 1;
  }
  if(ReadOp(P2IFG,5))
  {
    SetOpFalse(P2IFG,5);
    if(wakeflg && (delayFLG1==0))
    {
      K1();
      delayFLG1 = ButtonDelay;
    }
  }
}