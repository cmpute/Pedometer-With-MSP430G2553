#ifndef UNIVERSAL
#define UNIVERSAL

#include "io430.h"
#include "in430.h"

typedef unsigned char t_byte; //将无符号char表示成t_byte，便于程序编写
extern unsigned long steps;
extern t_byte conts[];
extern t_byte mode;
extern t_byte StepOffset;
extern t_byte TimeOffset;

//方便设置引脚而写的预处理指令
#define SetPort1Out(data,pin) P1OUT = (data==1) ? P1OUT | (1<<pin) : P1OUT &~(1<<pin)
#define SetPort2Out(data,pin) P2OUT = (data==1) ? P2OUT | (1<<pin) : P2OUT &~(1<<pin)
#define SetOpTrue(reg,pin) reg |= (1<<pin)
#define SetOpFalse(reg,pin) reg &= ~(1<<pin)
#define SetOp(reg,pin,val) reg = (val==1) ? reg | (1<<pin) : reg & ~(1<<pin)  //未成功实现
#define ReadOp(reg,pin) (reg>>pin)&1

#define BIT67 (BIT6 + BIT7)

#define IsSleep //是否自动进入低功耗状态,注释掉则不会进入低功耗模式
#define IsAutoSave //是否自动保存，注释掉则不会保存

#endif