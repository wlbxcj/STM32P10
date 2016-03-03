#ifndef __KF701DH_H__
#define __KF701DH_H__

//#include "w78e58.h"
//#include <absacc.h>
//#include <intrins.h>
#include <string.h>
#include <stdio.h>
#include "Display.h"
#include "calendar.h"

#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long

//定时器初值
#define	TIMER0H	  0xee		/* 定时器0时间,5ms */
#define	TIMER0L	  0x1
#define	TIMER2H	  0xee		/* 定时器0时间,5ms */
#define	TIMER2L	  0x1

#define	 SEND_WT_TIME	  5000 //25S

//应答命令
#define OK                       1       //请求能执行的应答
#define NP                       2       //请求下一包数据的应答
#define WT                       3       //请求再等待30s的应答
#define ER                       4       //请求不能执行的应答
//extern uchar power_down_flag(void);

/***************************************
函数功能：上电后完成初始化系统,具体有如下操作
           1:   关闭灯和蜂鸣器
           2:   关闭id卡座电源,打开sam卡座电源,关闭卡座时钟信号                
           3:   初始化串口,波特率9600,打开串口中断
           4;   关闭所有中断, 
           5。  关闭后备电源    

返回  0: 初始化成功  !=0 初始化失败
***************************************/
extern uchar initial_system(void);

/**************************************************************************
函数功能：打开后备电源                         
 入口参数：无  
 出口参数：无                                                 
**************************************************************************/
extern void bat_on(void);

/**************************************************************************
函数功能：关闭后备电源                         
 入口参数：无  
 出口参数：无                                                 
**************************************************************************/
extern void bat_off(void);

/*  ---------------------  sram  ------------------------*/
/***************************************************************************/
/* 函数功能：将SRAM中leng长度的数据读到缓存区中                            */
/* 入口参数：addr为读数据的起始地址,leng为数据长度，*buf为存放数据的指针   */
/* 出口参数：返回值0 正确                                                  */
/*                 1 出错                                                  */
/***************************************************************************/
extern uchar read_sramb(uint addr,uint leng,void *buf);
/***************************************************************************/
/* 函数功能：将leng长度的数据写入SRAM中                                    */
/* 入口参数：addr为写入数据的起始地址,leng为数据长度，*buf为存放数据的指针 */
/* 出口参数：返回值0 正确                                                  */
/*                 1 出错                                                  */
/***************************************************************************/
extern uchar write_sramb(uint addr,uint leng,void *buf);
/*  ---------------------  flash (S29GL064)------------------------*/
/***************************************
函数功能：分扇区擦除flash
函数入口：4M字节的flash共分64个扇区,每个扇区64K，sector为0～63
          8M字节的flash共分128个扇区,每个扇区64K，sector为0～127
          32M字节的flash共分512个扇区,每个扇区64K，sector为0～511
函数出口：0：正确      1：错误
***************************************/
extern uchar flash_sector_erase(uint sector);

/***************************************
函数功能：整片擦除flash
返回值 ： 0：擦除正确 1：擦除失败
***************************************/
extern uchar flash_chip_erase(void);

/***************************************
函数功能：读wr_buf缓冲区中wr_length长度的数据写入flash指定单元
函数入口：wr_addr:  写操作起始地址(0---0X3FFFFF)  4M空间 
                    写操作起始地址(0---0X7FFFFF)  8M空间
                    写操作起始地址(0---0X1FFFFFF)  32M空间
		 wr_length:写入字节数
		 wr_buf:   写操作数据缓冲区 
返回值 ： 0：写入正确 1：写入失败
***************************************/
extern uchar write_flash(ulong wr_addr,uint wr_length,  void *wr_buf);

/***************************************
函数功能：将FLASH中rd_length长度的数据读到rd_buf缓冲区中
函数入口：rd_addr:  读操作起始地址 (0---0X3FFFFF)  4M空间 
                    读操作起始地址 (0---0X7FFFFF)  8M空间   
                    读操作起始地址 (0---0X1FFFFFF)  32M空间
		 rd_length:读出字节数
		 rd_buf:   读操作数据缓冲区 
返回值 ： 0：读操作正确 1：读操作失败,地址溢出
***************************************/
extern uchar read_flash(ulong rd_addr,uint rd_leng, void *rd_buf);

/*  ---------------------      eeprom      ------------------------*/
//1个24c64,共8k字节存储容量
/***************************************
函数功能：读eeprom
函数入口：rd_addr:  读操作起始地址(0--0x1FFF);
		 rd_length:读出字节数
		 rd_buf:   读操作数据缓冲区 
返回值 ： 0：读操作正确 1：读操作失败
***************************************/
extern uchar read_eeprom(uint rd_addr,uint rd_length,void *rd_buf);

/***************************************
函数功能：向eeprom某地址开始写wr_length长的数据
函数入口：wr_addr:  写操作起始地址(0--0x1FFF);
		  wr_length:写入字节数
		  wr_buf:   写操作数据缓冲区 
返回值 ： 0：写入正确   1：写入失败
***************************************/
extern uchar write_eeprom(uint wr_addr,uint wr_length,void *wr_buf);

/*  ---------------------     时钟函数     ------------------------*/
/***********************
函数功能：设置时钟
函数入口：rtc_buf:设置时钟数据缓冲区
			rtc_buf中的数据定义如下： 

			rtc_buf[0]=年低位 
			rtc_buf[1]=月 
			rtc_buf[2]=日 
			rtc_buf[3]=时 
			rtc_buf[4]=分 
			rtc_buf[5]=秒 
			数据采用压缩BCD码格式
返回值：0:设置成功  1：设置失败,时钟数据格式错误 
***********************/
extern uchar set_rtc(uchar *rtc_buf);	


/***********************
函数功能：读日期时间
函数入口：rtc_buf:读操作数据缓冲区
			rtc_buf中的数据存放如下： 

			rtc_buf[0]=年低位
			rtc_buf[1]=月 
			rtc_buf[2]=日 
			rtc_buf[3]=时 
			rtc_buf[4]=分 
			rtc_buf[5]=秒 
			数据采用压缩BCD码格式
***********************/
extern void read_rtc(uchar *rtc_buf);

/********************************************
函数功能:验证rtc_buf所指向的日历/时钟数据的合法性   
入口:   rtc_buf:存放日期时间的首地址
            rtc_buf中的数据定义如下： 

        	rtc_buf[0]=年
        	rtc_buf[1]=月 
        	rtc_buf[2]=日 
        	rtc_buf[3]=时 
        	rtc_buf[4]=分 
        	rtc_buf[5]=秒 
        
            数据采用压缩BCD码格式， 
出口：无                                   
返回值: 数据合法返回0，否则返回1 
*********************************************/
extern uchar check_rtc(uchar *rtc_buf);

/*--------------------        串口通讯         ---------------------------*/
/*----------------------------------------------------
函数功能:修改波特率
入口参数：rate     对应波特率
            1       300
            2       600
            3       1200
            4       2400
            5       4800
            6       9600
            7       14400
            8       19200
            9       33600
            10      57600

出口参数：0=成功   !0=入口参数错误

修改者:  
修改时间：
-----------------------------------------------------*/
extern uchar modify_rate(uchar rate);

/***************************
函数功能:以查询方式,接收一个字节(8位数据,1个起始一个停止,无校验)
入口:无
出口:buf 收到的数据首地址
返回值: 
        == 0 正确 收到数据,
        != 0 超时(TIMEO)
****************************/		
extern uchar rece(uint length,uchar *buffer);
/********************************
函数功能:以查询方式,发送一个字节(8位数据,1个起始一个停止,无校验),
入口:    c 发送的字符
出口:   
返回值: 无
*********************************/		
extern void send_str(uint length,uchar *buffer);

/**
函数功能:清除串口缓冲区 
*/
extern void clear_rece_buffer(void);

/*------------------------ 卡操作函数 ---------------------------------*/

/***************************************
函数功能：检测是否有卡插入
返回值  ：0：有卡   1：无卡
***************************************/
extern uchar inserted_card(void);

/***************************************
函数功能：小卡座上电
***************************************/
extern void sam_pwr_on(void);

/***************************************
函数功能：小卡座下电
***************************************/
extern void sam_pwr_off(void);

/***************************************
函数功能：大卡座上电
***************************************/
extern void id_pwr_on(void);

/***************************************
函数功能：大卡座下电
***************************************/
extern void id_pwr_off(void);

/***************************************
函数功能：大卡座上的cpu卡复位应答(含有上电)
函数入口：无
函数出口：返回0：复位正常  否则：复位异常
		 buf：存放复位信息的首地址，数据格式为：ATR长度、ATR1信息、ATR2信息......		
***************************************/
extern uchar atr_id(uchar *buf);

/***************************************
功能描述：与大卡座上电CPU卡进行通讯
入口参数: 
	ptr:输入数据缓冲区。格式为：60H,LEN,ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n,le
	60H --命令头
	LEN	--ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n的总字节数
	ISO_MODE--0DAH,ISO_IN操作，写卡/校密码
			--0DBH,ISO_OUT操作，读卡
			--其它内容为非法
	ISO_IN :60H,LEN,ISO_MODE,CLS,INS,P1,P2,Lc,data_1,...,data_n
	ISO_OUT:60H,LEN,ISO_MODE,CLS,INS,P1,P2,Le
	        le最大为240
出口参数:  返回0：操作成功  否则操作失败	
	ptr：输出数据缓冲区，其格式为：60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	LEN	--data_1,...,data_n的字节数
	ISO_IN:	60H,LEN,ISO_MODE,SW1,SW2
	ISO_OUT:60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	无论ISO_IN还是ISO_OUT,ptr缓冲区都会被改写,
***************************************/
extern uchar id_iso(uchar *ptr);

/***************************************
函数功能：SAM卡座上的CPU卡复位
函数入口：无
函数出口：返回0：复位正常   否则：复位异常
		 buf：存放复位信息的首地址，数据格式为：ATR长度、ATR1信息、ATR2信息......
***************************************/
extern uchar atr_sam(uchar *buf);

/***************************************
功能描述：与小卡座上电CPU卡进行通讯
入口参数: 
	ptr:输入数据缓冲区。格式为：60H,LEN,ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n
	60H --命令头
	LEN	--ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n的总字节数
	ISO_MODE--0DAH,ISO_IN操作，写卡/校密码
			--0DBH,ISO_OUT操作，读卡
			--其它内容为非法
	ISO_IN :60H,LEN,ISO_MODE,CLS,INS,P1,P2,Lc,data_1,...,data_n
	ISO_OUT:60H,LEN,ISO_MODE,CLS,INS,P1,P2,Le
	        le最大为240
出口参数:  返回0：操作成功  否则操作失败	
	ptr：输出数据缓冲区，其格式为：60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	LEN	--data_1,...,data_n的字节数
	ISO_IN:	60H,LEN,ISO_MODE,SW1,SW2
	ISO_OUT:60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
***************************************/
extern uchar sam_iso(uchar *ptr);


//   flash card
/**********************************************
函数功能:AT45D041 RESET
***********************************************/
extern void at45d041_reset(void);

/*********************************
函数功能：写flash卡
入口参数：  ADDR:开始的地址 (0---0x83fff)
            LENGTH:长度     (1---264)
            BUF:数据的起始地址
返回值：  写入成功0，否则1: 入口参数错误(length>264或者length==0)
*********************************/
extern uchar f45d041_writecard(ulong addr,uint length,uchar *buf);

/************************************
函数功能:从卡中读length bytes到缓冲区
输入参数:addr(F45D041卡:0..0x82fff):卡地址;
		lenght(0..263):长度;
		buf:缓冲区;

适用机型：
修改时间:2002.4.12
****************************/
extern uchar f45d041_readcard(ulong addr,uint length,uchar *buf);

//   4442 card
/* 从4442卡的address单元开始读length个数据，写入buffer */
//extern void read_sle42(uchar address,uchar length,uchar *buffer);

 /* 4442卡的复位应答 */
//extern void initial_sle44xx(uchar *buffer);

/*-----------------------------------其它函数---------------------------------*/
/***************************************
函数功能：打开蜂鸣器
***************************************/
extern void buz_on(void);		
extern void Buzzer_Ring(u16 Fre);
extern void Buzzer_Off(void);
/***************************************
函数功能：关闭蜂鸣器
***************************************/
extern void buz_off(void);	

/***************************************
函数功能：绿指示灯亮
***************************************/
extern void ledgreen_on(void);		

/***************************************
函数功能：绿指示灯灭
***************************************/
extern void ledgreen_off(void);		

/***************************************
函数功能：黄指示灯亮
***************************************/
extern void ledorange_on(void);		

/***************************************
函数功能：黄指示灯灭
***************************************/
extern void ledorange_off(void);	

/***************************************
函数功能：红指示灯亮
***************************************/
extern void ledred_on(void);		

/***************************************
函数功能：红指示灯灭
***************************************/
extern void ledred_off(void);	

extern void ledall_on(void);
extern void ledall_off(void);
extern void LcdBL_Control(u8 Control);
extern void LcdReset(void);
extern void LcdWriteCmd(u8 Command);
extern u8 LcdReadStatus(void);
extern void LcdWriteData(u8 DisplayData);
extern u8 LcdReadData(void);
extern void LcdDotTest(void);
//extern u8 Char2Dot8X8(u8 *pCharSrc,u8 *pDotSrc);
extern void Lcdarray_On(u8 LineX,u8 LineY,u8 ByteLength, u8 *NewLcdData);
extern void Lcdstring_On(u8 LineX,u8 LineY,u8 *NewLcdStr);
extern void Lcdarray_clear(u8 LineBit);

extern void ChangeDate(u8 *);
extern void ChangeTime(u8 *);
extern u8 TimeGet(u8 *);
extern void DateGet(u8 *);

extern void RF1356_Init(void);
extern u8 PN512RegsWrite(u8 *pData);
extern u8 PN512RegsRead(u8 *pData);
extern u8 RF1356Reset(u8 *Infor);
extern u8 RF1356Start(u8 *Infor);
extern u8 RF1356StartTypeA(u8 *Infor);
extern u8 RF1356PollTypeA(u8 *Infor);

//extern void WeekDayNum(u32 nyear, u8 nmonth, u8 nday);
/*********************************
函数功能:延时时间 t ms
*********************************/
extern	void delay_ms(ulong t);

/*********************************************
函数功能： CRC校验算法 (国标正序算法)                   
入口参数： length 参加运算数据长度        
            clear==1 CRC初值清零开始运算,仅仅计算buffer内数据的crc校验值
            clear==0 在保留CRC初值基础上计算buffer内数据的crc校验值
            buffer 数据地址指针			 
返回值： CRC校验码                        
*********************************************/
extern uint crc_16(uint length,uchar clear,void *buffer);
//反序算法
extern uint crcxdata_old(uint length,uchar clear,void *buffer);

/*-------------------------------------------------------------------------------
 Funtion: 将压缩Bcd转换为Bin 
 Input  : TUlBinValue:BIN码值  
	  TUcCompBcdLen:压缩bcd码字节数 
	  TUcCompBcdBuf: 压缩的bcd码 
 excemple:十进制数：12h,34h,56h 转换为0x1e240h
 Editor : NingNeng
 Date   : 2002-11-19 
----------------------------------------------------------------------------*/
extern void bcd_to_bin(ulong *TUlBinValue,uchar TUcCompBcdLen,uchar *TUcCompBcdBuf);
/*-------------------------------------------------------------------------------
 Funtion: 将BIN转换为压缩BCD 
 Input  : TUlBinValue:BIN码值 
	  TUcCompBcdLen:压缩后的bcd码字节数, 
	  TUcCompBcdBuf: 压缩后的bcd码 
 excemple:十进制数：123456 转换为12H，34H，56H的序列
----------------------------------------------------------------------------*/
extern void bin_to_bcd(ulong TUlBinValue,uchar TUcCompBcdLen,uchar *TUcCompBcdBuf);

/******************
功能    进入软件下载程序（78E516) 
*****************/
//extern void  prg_load();
//================================================
// 函 数 名: calc_next_day
// 功能描述: 判断输入日期的合法性并返回下一日的日期
// 全局变量: 无
// 输入参数: date 要计算的日期(CCYYMMDD)
// 输出参数: 无
// 返 回 值: 所指字符串的头4字节均为0xFF 输入日期错误;
//           ==其他，下一日的日期(CCYYMMDD)
// 作    者: 周剑冬
// 创建日期: 2003-06-XX
// 修 改 者: 周剑冬
// 修改日期: 2003-06-XX
//================================================
extern uchar * calc_next_day(uchar *date);
extern uchar * calc_last_day(uchar *date);
/**********************************************
功能:   指令测试程序 
返回值：测试成功0，否则!0
*************************************************/
extern uchar test_ins(void);

extern void dtr_on(void);

extern void dtr_off(void);

//this interrupt will defaultly be called during power fall interrupt
extern void IT_PowerOff(void); 

//this API is used when app determine to power off the tax box itself
//any code behind this API will never be involked because of power down
void TaxBox_PowerOff();
extern void usb_com_mode(void);

extern void uart_com_mode(void);

//this interrupt will defaultly be called every 5 mS
extern void IT_SendWait(void);
extern void System_SoftReset(void);
u8 UARTCOM_RecvCmd(u8 *Data);
void UARTCOM_SendString(u8 *Data, u16 DataLength);

//for big end and little end feature
///获得一个32bits的u32 输入为大数端，返回当前平台的u32
u32 GetU32(void* value);
///设置一个32bits的u32 输入为大数端的 被设数 和一个本地平台的u32数
void SetU32(void* value, u32 fact);
///自增一个32bits的u32 输入为大数端
void IncU32(void* value);
///加法一个32bits的u32 输入为大数端的 被加数 何一个本地平台的加数
void AddU32(void* value, u32 fact);
///获得一个16bits的u16 输入为大数端，返回当前平台的u16
u16 GetU16(void* value);
///设置一个16bits的u16 输入为大数端的 被设数 和一个本地平台的u16数
void SetU16(void* value, u16 fact);
///自增一个16bits的u16 输入为大数端
void IncU16(void* value);
///加法一个16bits的u16 输入为大数端的 被加数 何一个本地平台的u16加数
void AddU16(void* value, u16 fact);
extern unsigned char USB_Wait_InsertDisk();
//USB_Wait_InsertDisk();

///Read a file from given offset
extern unsigned char USB_ReadFile(unsigned int uiOffset,unsigned char *pucData,unsigned int uiLen);
//USB_ReadFile("\\NEWFILE.TXT",5,temp,10);


///write a file from given offset
extern unsigned char USB_WriteFile(unsigned int uiOffset,unsigned char *pucData,unsigned int uiLen, unsigned char ucWriteMode);
//USB_WriteFile("\\NEWFILE.TXT",1,"ab",2);


///append a file from the end of current file
extern unsigned char USB_AppendFile(unsigned char *pucFileName,unsigned char *pucData,unsigned int uiLen);
//USB_AppendFile("\\NEWFILE.TXT","zzzzzzzzzz",10);


///inquire the file size of given file name (temp file handler)
extern unsigned char USB_FileSize(unsigned char *pFilename,unsigned int *piSize);



///erase the file with given file name (temp file handler)
extern unsigned char USB_EraseFile(unsigned char *pucFileName);
extern unsigned char USB_CreateFile(unsigned char *pucFileName);
extern unsigned char USB_OpenFile(unsigned char *pucFileName);
extern unsigned char USB_CloseFile(void);

/***************************************
函数功能：开电源保护
***************************************/
extern void power_on(void);


/***************************************
函数功能：关电源保护
***************************************/
extern void power_off(void);

extern void WriteToBackupReg(u16 index ,u16 FirstBackupData);
u32 CheckBackupReg(u16 index,u16 FirstBackupData);

#endif
