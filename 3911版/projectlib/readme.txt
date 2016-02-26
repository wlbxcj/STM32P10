PC0:硬件版本
  adc:
  
2013.06.26(035)
   1)库加了flash 保护.
   2)没用管脚初始化。

2013.04.03
1)下载应用程序后不会删除密钥

2013.03.25
1)update boot
  用v10pupdateboot.bin更新v10pboot2.0.bin
2)修改stm32f10x_flash.icf
  0x08009000改为0x8006800
3)用新的库v10plib02.a

2013.02.21
  1)保存日期(Lib_SetDateTime)
2013.02.18
1)int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
    mode不管什么值，可刷A卡/B卡
2013.1.25
  1)程序最大为146K
  2)用户可用为66K
  3)格式是 两个自己长度（asc码） +  序列号
stm32f10x_flash.icf:0x08009000改为0x8006800  

2013.1.4
                  //Address:0-(66*1024-1)  size:66k                  
int Lib_AppShareRead(ulong Address,uchar * buff,ulong len);                  
int Lib_AppShareWrite(ulong Address,uchar * buff,ulong len);
