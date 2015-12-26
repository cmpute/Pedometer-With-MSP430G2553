#include "digitaltube.h"
#include "timing.h"
#include "button.h"

//数码管字符映射表
const t_byte digidis[32] = {~0x3F,~0x06,~0x5B,~0x4F,~0x66,~0x6D,~0x7D,~0x07,~0x7F,~0x6F,   //0123456789 | 0-9
  ~0x77,~0x7C,~0x39,~0x5E,~0x79,~0x71, //AbCdEF | 10-15
  ~0x3D,~0x76,~0x0E,~0x38,~0x73,~0x3E,~0x58,~0x50,~0x5c,~0x74,~0x78,~0x1C,~0x6E,  //GHJLPUcrohtuy | 16-28
  ~0x00,~0x20,~0x80}; //空'. | 29-31
t_byte Stepmode = 0; // 0: shortstep, 1: longstep

//数码管刷新delay函数
void qdelay()
{
  unsigned char i;
  for(i=0;i<0xff;i++);
}

//初始化数码管部分
void DigiTubeInit()
{
  P1SEL &= ~0x2F;  //BIT0+BIT1+BIT2+BIT3+BIT4
  P1SEL2 &= ~0x2F;
  P1DIR |= 0x2F;
  P1OUT |= 0x2F;
  P2SEL &= ~BIT67;
  P2SEL2 &= ~BIT67;
  P2DIR |= BIT67;
}

//向串并转换芯片写入一个数码管的数据
void SendData(t_byte d)
{
  signed char i;
  for(i=7;i>=0;i--)
  {
    SetPort1Out((d>>i)&1,5);
    SetPort2Out(1,6);
    SetPort2Out(0,6);
  }
  SetPort2Out(1,7);
  SetPort2Out(0,7);
}
//while中进行的数码管处理
void DigiTubeProcess()
{
  //P1OUT = (P1OUT & 0xf0);//SendData函数运行时间长，需要先关闭数码管显示,但为了增加亮度就不关了
  SendData(( mode == 3 ) ? (digidis[conts[0]] & ~BIT7) : digidis[conts[0]]);
  P1OUT = (P1OUT & 0xf0) | BIT3;
  qdelay();
  //P1OUT = (P1OUT & 0xf0);
  SendData(( mode == 0 || mode == 2 ) ? (digidis[conts[1]] & ~BIT7) : digidis[conts[1]]);//BIT7：分钟秒钟分割小数点
  P1OUT = (P1OUT & 0xf0) | BIT2;
  qdelay();
  //P1OUT = (P1OUT & 0xf0);
  SendData(digidis[conts[2]]);
  P1OUT = (P1OUT & 0xf0) | BIT1;
  qdelay();
  //P1OUT = (P1OUT & 0xf0);
  SendData(digidis[conts[3]]);
  P1OUT = (P1OUT & 0xf0) | BIT0;
  qdelay();
}

//数码管内容刷新
void ContentFresh()
{
  switch(mode)
  {
  case 0:
    if(TimeOffset)
    {//显示hh.mm
      conts[0]=(hour-hour%10)/10;
      conts[1]=hour%10;
      conts[2]=(min-min%10)/10;
      conts[3]=min%10;
    }
    else
    {//显示mm.ss
      conts[0]=(min-min%10)/10;
      conts[1]=min%10;
      conts[2]=(sec-sec%10)/10;
      conts[3]=sec%10;
    }
    break;
  case 1:
    t_byte j = 0;
    unsigned long stepoffs = steps;
    for(;j<StepOffset;j++)
      stepoffs /= 10;
    conts[0]=(stepoffs/1000)%10;
    conts[1]=(stepoffs/100)%10;
    conts[2]=(stepoffs/10)%10;
    conts[3]=stepoffs%10;
    break;
  case 2:
    conts[0]=(STsec-STsec%10)/10;
    conts[1]=STsec%10;
    conts[2]=(STdsec-STdsec%10)/10;
    conts[3]=STdsec%10;//STdsec看起来比STdsec%10的效果好
    break;
  case 3:
    unsigned int speed = steps * (Stepmode ? LongStep : ShortStep) / (long)(hour * 3600 + min * 60 +sec); //speed单位mm/s
    conts[0]=(speed/1000)%10;
    conts[1]=(speed/100)%10;
    conts[2]=(speed/10)%10;
    conts[3]=speed%10;
    break;
  case 4:
    if(ClrDisp == 0)
    {
      conts[0]=12;  //C
      conts[1]=19;  //L
      conts[2]=23;  //r
      conts[3]=29;  //空
    }
    else if(ClrDisp == 1)
    {
      conts[0]=12;  //C
      conts[1]=19;  //L
      conts[2]=23;  //r
      conts[3]=13;  //d
    }
    else if(ClrDisp == 2)
    {
      conts[0]=5;  //S
      conts[1]=10;  //A
      conts[2]=30;  //'
      conts[3]=13;  //d
    }
    else if(ClrDisp == 3)
    {
      conts[0]=12;  //C
      conts[1]=19;  //L
      conts[2]=23;  //r
      conts[3]=10;  //A
    }
    break;
  }
}