#ifndef EEPROM
#define EEPROM

#include "universal.h"

// 用符号定义25LC020A 各命令
#define  c_READ         3       //读命令           0000 0011B
#define  c_WRITE        2       //写命令           0000 0010B
#define  c_WRDI         4       //写禁止           0000 0100B
#define  c_WREN         6       //写允许           0000 0110B
#define  c_RDSR         5       //读状态寄存器     0000 0101B
#define  c_WRSR         1       //写状态寄存器     0000 0001B

// 用符号定义状态寄存器中的状态位
#define	 BP1    BIT3
#define	 BP0    BIT2
#define	 WEL    BIT1
#define	 WIP    BIT0		   
 
// 用符号定义使用的端口和管脚，可据具体的连线进行修改
#define  PeSEL   P2SEL
#define  PeSEL2  P2SEL2
#define  PeDIR   P2DIR
#define  PeOUT   P2OUT
#define  PeIN  	 P2IN
#define  eeSCK   BIT3	//SCK -> P2.3	
#define  eeSI  	 BIT4    //SI -> P2.4
#define  eeCS  	 BIT2    //CS -> P2.2
#define  eeSO 	 BIT1    //SO -> P2.1

//校验内容的设置
#define CheckContent    "DtCheck"
#define CheckLength     8

void EE_Init();
t_byte ReadData();
void SaveData();

#endif