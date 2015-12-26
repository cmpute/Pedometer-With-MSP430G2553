#include "universal.h"
#include "eeprom.h"
#include "digitaltube.h"
#include "timing.h"
#include "button.h"

unsigned long steps = 0; //123456789UL;
t_byte conts[4]={20,23,24,22} ;//数码管四位内容，默认显示"Proc"，表示在读取存储器
t_byte mode = 0; //0:显示运行时间  1:显示步数  2:秒表 3:显示速度 4:清零模式
t_byte StepOffset = 0;//显示步数时的数字首位偏移,使用long的情况下最多给9位的步数
t_byte TimeOffset = 0;//0:显示mm.ss  1:显示hh.mm

//长时间delay函数
void delay()
{
  int i = 0;
  for(i=0;i<0xfff;i++);
}

//传感器中断设置
void SensorInit()
{
  SetOpFalse(P2DIR,0);
  SetOpTrue(P2IE,0);
  SetOpFalse(P2IFG,0);
  SetOpTrue(P2IES,0);
  SetOpTrue(P2OUT,0);
  SetOpTrue(P2REN,0);
}

//初始化
void MainInit()
{
  DigiTubeInit();
  ButtonInit();
  SensorInit();
  TimerInit();
  EE_Init();
  _EINT();
}

int main( void )
{
  //关闭看门狗
  WDTCTL = WDTPW + WDTHOLD;
  MainInit();
  ReadData(); //启动时读取存储器中的内容
  ContentFresh();
  while(1)
  {
      TAFlagProcess();
      SensorFlagProcess();
      DigiTubeProcess();
      ButtonHoldProcess();
      SleepFlagProcess(); //由于不可在中断过程中进入低功耗模式,否则GIE被关闭无法退出,因此进入低功耗模式的过程放到主循环来
  }
  //return 0;
}



