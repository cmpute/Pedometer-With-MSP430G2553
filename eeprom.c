#include "eeprom.h"
#include "timing.h"

const unsigned char s_byte = sizeof(t_byte);

//初始化EEPROM设置引脚
void EE_Init()
{
  PeSEL &= ~(eeSCK + eeSI + eeCS + eeSO);
  PeSEL2 &= ~(eeSCK + eeSI + eeCS + eeSO);
  PeDIR |= eeSCK + eeSI + eeCS;
  PeDIR &= ~eeSO;
}
//刷时钟信号一次
void send_SCK()
{
  PeOUT &= ~eeSCK;      // SCK=0
  PeOUT |= eeSCK;       // SCK=1
  PeOUT &= ~eeSCK;      // SCK=0

}
//写字节
void send_SI_byte(unsigned int a)
{
  t_byte i,b;
  for(i=0;i<8;++i)
  {
    b=a<<i;
    switch(b&0x80){
    case 0x80: PeOUT=PeOUT|eeSI; break; // a中第(8-i)位为1，置SI=1.
    case 0: PeOUT=PeOUT&(~eeSI); break;   // a中第(8-i)位为0，置SI=0.
    default:;
    }
    send_SCK();                           // 通过SCK输出一个clock
  }
}
//读字节
t_byte read_SO_byte()
{
  t_byte i,a=0;
  for(i=0;i<8;++i)
  {
    a=a<<1;                              // 将a左移一位
    if((PeIN&eeSO)==0)       a=a&254;   // 置a末位为0
    else a=a|1;                         // 置a末位为1
    send_SCK();                          // 通过SCK输出一个clock
  }
  return a;
}
//读EEPROM状态
t_byte ReadStatus()
{
  t_byte a;
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_RDSR);                  // 发送读状态命令
  a=read_SO_byte();                      // 读取状态值
  PeOUT=PeOUT|eeCS;                      // CS=1 
  return a;
}
//等待直到无写操作进行
void CheckWIP()
{
  t_byte k;
  while(1){
  k=ReadStatus();;
  if((k&WIP)==0) break;                    // 当WIP位为0（不忙时）跳出无限循环
  }
}
//允许写操作
void WriteEnable()
{
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_WREN);                  // 写入写允许命令
  PeOUT=PeOUT|eeCS;                      // CS=1
}
//禁止写操作
void WriteDisable()
{
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_WRDI);                  // 写入写禁止命令
  PeOUT=PeOUT|eeCS;                      // CS=1  
}
//读取指定地址内容
t_byte EE_ReadByte(t_byte address)
{
  t_byte a;
  CheckWIP();                             // 查询状态
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_READ);                  // 发送读命令
  send_SI_byte(address);                 // 发送读单元地址
  a=read_SO_byte();                      // 读取指定单元的内容
  PeOUT=PeOUT|eeCS;                      // CS=1 
  return a;
}
//向指定地址写入数据
void EE_WriteByte(t_byte address,t_byte data)
{
  CheckWIP();                             // 查询状态
  WriteEnable();                              // 设置写允许
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_WRITE);                 // 发送写命令
  send_SI_byte(address);                 // 发送写单元地址
  send_SI_byte(data);                    // 发送写入单元的内容
  PeOUT=PeOUT|eeCS;                      // CS=1
}
//从指定地址开始写入数据数组
void EE_WriteArray(t_byte address,unsigned char* data,t_byte length)
{
  t_byte i=0;
  for(;i<length;i++)
    EE_WriteByte(address+i, *(data+i*s_byte));
}
//从指定地址开始读取数组并存储到data中
void EE_ReadArray(t_byte address,unsigned char* data,t_byte length)
{
  t_byte i=0;
  for(;i<length;i++)
    *(data+i*s_byte) = EE_ReadByte(address+i);
}
//写入状态
void WriteStatus(t_byte data)
{
  CheckWIP();                             // 查询状态
  WriteEnable();                              // 设置写允许
  PeOUT=PeOUT&(~eeCS);                   // CS=0
  send_SI_byte(c_WRSR);                  // 写状态寄存器命令
  send_SI_byte(data);                    // 发送要写入的内容
  PeOUT=PeOUT|eeCS;                      // CS=1
}

//保存运行时间、计步器值、秒表值、校验字符
void SaveData()
{
  EE_WriteByte(0x80, STdsec);
  EE_WriteByte(0x90, STsec);
  EE_WriteByte(0x20, sec);
  EE_WriteByte(0x21, min);
  EE_WriteByte(0x22, hour);
  EE_WriteByte(0x30, (steps&0xFF000000)>> 24);
  EE_WriteByte(0x31, (steps&0x00FF0000)>> 16);
  EE_WriteByte(0x32, (steps&0x0000FF00)>> 8);
  EE_WriteByte(0x33, (steps&0x000000FF));
  EE_WriteByte(0x10, MTimeSuspFLG);
  EE_WriteByte(0x11, STimeSuspFLG);
  EE_WriteArray(0xA0, CheckContent,CheckLength);
}

//读取上述数据,校验成功返回0,校验出错返回1
t_byte ReadData()
{
  t_byte checkd[CheckLength];
  t_byte checkt[CheckLength] = CheckContent;
  EE_ReadArray(0xA0, checkd, CheckLength);
  t_byte j=0;
  for(;j<CheckLength;j++)
    if(checkd[j]-checkt[j])
      return 1;
  STdsec = EE_ReadByte(0x80);
  STsec = EE_ReadByte(0x90);
  sec = EE_ReadByte(0x20);
  min = EE_ReadByte(0x21);
  hour = EE_ReadByte(0x22);
  steps = (unsigned long)EE_ReadByte(0x30) << 24;
  steps += (unsigned long)EE_ReadByte(0x31) << 16;
  steps += (unsigned long)EE_ReadByte(0x32) << 8;
  steps += (unsigned long)EE_ReadByte(0x33);
  MTimeSuspFLG = EE_ReadByte(0x10); 
  STimeSuspFLG = EE_ReadByte(0x11);
  return 0;
}