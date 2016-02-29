/*******************************************************************************
* File Name			 : struct.h
* Author             : guohonglv
* Date First Issued  : 2010/11/12
* Description        : 移植时用到的结构体（稍微有点乱，没有分类）
********************************************************************************/

#ifndef STRUCT_H
#define STRUCT_H


#define  TRACK2_LEN        100 
#define  TRACK3_LEN        107

//typedef unsigned char  u8;
#define uchar unsigned char
#define u8  unsigned char
#include <time.h>

/* //time.h 对下面两个已经有定义，可以注释掉
struct tm 
{ 
	int tm_sec; 
	int tm_min; 
	int tm_hour; 
	int tm_mday; 
	int tm_mon; 
	int tm_year; 
	int tm_wday; 
	int tm_yday; 
	int tm_isdst; 
}; 


typedef struct
{
  u8 sec_l;
  u8 sec_h;
  u8 min_l;
  u8 min_h;
  u8 hour_l;
  u8 hour_h;
}time_t;
*/

/*
struct ClssProcInter 
{
	uchar aucAID[17];       
	uchar ucAidLen; 
	
	// payWave
	uchar   ucZeroAmtFlg;       		// 0-交易金额!=0; 1-交易金额=0
	uchar   ucStatusCheckFlg;   		// 读卡器是否支持状态检查
    uchar   aucReaderTTQ[5];      // 终端交易性能，用于VISA/PBOC中，tag =9F66
	uchar   ucCLAppNotAllowed; 			// 1-该AID不能进行非接触交易
	
	// common
	uchar ucTermFLmtExceed; 
	uchar ucRdCLTxnLmtExceed; 
	uchar ucRdCVMLmtExceed;  
	uchar ucRdCLFLmtExceed;  
	
	uchar ucTermFLmtFlg;
	uchar aucTermFLmt[4];
	
	uchar aucRFU[2];
}; 
extern struct ClssProcInter ClssProcInter;  //guohonglv //这样可以防止变量的重复定义


struct ClssEc
{
	unsigned long  ulAmntAuth;     // 授权金额(ulong)
    unsigned long  ulAmntOther;    // 其他金额(ulong) 
    unsigned long  ulTransNo;      // 交易序列计数器(4 BYTE)
	uchar  ucTransType;      // 交易类型 not  '9C' !!! (GOODS CASH)
	uchar  aucTransDate[4];  // 交易日期 YYMMDD
	uchar  aucTransTime[4];  // 交易时间 HHMMSS
};  
extern struct ClssEc ClssEc; //guohonglv //这样可以防止变量的重复定义

struct gtPosParamet 
{
	uchar ucCommWaitTime; 	//通讯等待时间
	uchar szUnitNum[16];
	uchar szPosId[9];
	uchar szUnitChnName[41];
	int  iMaxTransTotal;  	//流水保存的最大交易笔数，可设
	uchar ucEmvSupport;
	uchar ucSelectCardType;	// PARAM_OPEN: oper select card type, PARAM_CLOSE: auto-select
	long lNowTraceNo; 		//当前终端交易流水号
	long lNowBatchNum;		//当前批次号 
	uchar ucOprtLimitTime;  // 操作限时时间 30S-60S
};
extern struct gtPosParamet gtPosParamet;  


struct PiccPara 
{
	unsigned char drv_ver[5];  		//驱动程序的版本信息，如：”1.01A”；只能读取，写入无效
	unsigned char drv_date[12];  	// 驱动程序的日期信息，如：”2006.08.25”； 只能读取
	unsigned char a_conduct_w;  	//A型卡输出电导写入允许：1--允许，其它值—不允许，该值不可读
	unsigned char a_conduct_val;	// A型卡输出电导控制变量，有效范围0~63,超出时视为63
	//缺省值为63；a_conduct_w=1时才有效，否则该值在驱动内部不作改变用于调节驱动A型卡的输出功率，由此能调节其最大感应距离
	unsigned char b_modulate_w;
	unsigned char b_modulate_val;  // B型卡调制指数控制变量，有效范围0~63,超出时视为63
	//;b_modulate_w=1时才有效，否则该值在驱动内部不作改变用于调节驱动B型卡的调制指数，由此能调节其最大感应距离
	unsigned char card_buffer_w;	//卡片接收缓冲区大小写入允许：1--允许，其它值—不允许
	unsigned short card_buffer_val;	//卡片接收缓冲区大小参数（单位：字节），有效值1~256。
	//大于256时，将以256写入；设为0时，将不会写入。
	//卡片接收缓冲区大小直接决定了终端向卡片发送一个命令串
	//时是否需启动分包发送、以及各个分包的最大包大小。若待 
	//发送的命令串大于卡片接收缓冲区大小，则需将它切割成小
	//包后，连续逐次发送。在Picc_detect( )函数执行过程中，卡片
	//接收缓冲区大小之参数由卡片报告给终端，一般无需更改此
	//值。但对于非标准卡，可能需要重设此参数值，以保证传输 有
	//效进行。
	unsigned char wait_retry_limit_w;	// S(WTX)响应发送次数写入允许：1－允许，其它值—不允 许
	unsigned short wait_retry_limit_val;	// S(WTX)响应发送次数最大值，默认为3；
	unsigned char card_type_check_w;   	// 卡片类型检查写入允许，该值不可读
	unsigned char card_type_check_val;  	// 0-检查卡片类型，其他－不检查卡片类型(默认为检查卡  
	//片类型)
	unsigned char card_RxThreshold_w;   // card RxThreshold
	unsigned char card_RxThreshold_val;
	unsigned char reserved[20];         // 保留字节，用于将来扩展；写入时应全清零
};
//extern struct PiccPara PiccPara;  //guohonglv 
*/
//下面是对于lcd.c的宏定义以及结构体
#define FBIOGET_VSCREENINFO 1 //guohonglv 
#define FBIOGET_FSCREENINFO 1 //guohonglv  

#define PROT_READ 1 //guohonglv 
#define PROT_WRITE 1 //guohonglv  
#define MAP_SHARED 1 //guohonglv

struct fb_var_screeninfo
{
  int xres;
  int yres;
};

struct fb_fix_screeninfo
{
  int smem_len;
};
 
#endif