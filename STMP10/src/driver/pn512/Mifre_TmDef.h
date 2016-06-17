#ifndef _MIFARE_TMDEF_H_
#define	_MIFARE_TMDEF_H_

/**************************************************************
*common section
**************************************************************/

//#define RF_L1_RECDBG//emv test switch

//#define Debug_Isr//测试512中断及错误产生情况
//#define PN512_Debug_Isr//测试512中断及错误产生情况

//#define FOR_LINUX_SYS  

#define TERM_KF900  1
#define TERM_KF322  2


#define TERM_TYPE TERM_KF322


extern unsigned char _glbPiccSetPara;

//extern void DelayMs(unsigned int ms);

#ifdef FOR_LINUX_SYS
	#include <linux/types.h>
	#include <linux/module.h>
	#include <linux/moduleparam.h>
	
	#include <linux/timer.h>
	#include <linux/cdev.h>
	#include <linux/delay.h>
	#include <linux/watchdog.h>
	#include <linux/fs.h>
	#include <linux/init.h>
	#include <linux/platform_device.h>
	#include <linux/interrupt.h>
	#include <linux/clk.h>
	#include <linux/kthread.h>
	#include <linux/wait.h>
#endif

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ulong
#define ulong unsigned long
#endif
#ifndef uint
#define uint unsigned int
#endif
#ifndef ushort
#define ushort unsigned short
#endif

typedef struct
{
    volatile    uchar ucRFOpen;
    volatile	ulong ulSendBytes;      // 预计发送数据长度
    volatile    ulong ulBytesSent;      // 当前已发送字节数
    volatile    ulong ulBytesReceived;  // 当前已经接收字节数
    volatile    long lBitsReceived;    // 当前已经接收位数
    volatile    uchar ucAnticol;       // 防冲突操作标志
    volatile    uchar ucCurResult;     // 在命令处理过程中的结果
    volatile    uchar ucINTSource;     // 在命令处理过程中所有发生的中断
    volatile    uchar ucErrFlags;      // 在命令执行中产生错误标志
    volatile    uchar ucSaveErrState;  // 保存在命令执行中产生错误标志
    volatile    uchar ucMifCardType;
    volatile	uchar ucCurType;        // 当前使用的卡类型号: EM_mifs_TYPEA-TYPE A, EM_mifs_TYPEB-TYPE B
    volatile    uchar ucCurPCB;         // 帧号
    
    volatile  	uchar ucFWI;            // 超时等待时间设置
    volatile	uchar ucSAK1;           // 第一级SELECT返回SAK值
    volatile	uchar ucSAK2;           // 第二级SELECT返回SAK值
    volatile	uchar ucSAK3;           // 第三级SELECT返回SAK值
    
    volatile	uchar ucMifActivateFlag; // 射频卡激活标志 0 - 未激活   1 - 已激活
    volatile	uchar ucUIDLen;          // 射频卡的UID长度，TypeA型卡可能有4、7、10字节，TypeB卡只有4字节
    volatile	uchar ucATQA[2];         // 射频卡在WUPA命令中返回的ATQA参数
    
    volatile	uchar ucATQB[12];        // 射频卡在WUPB命令中返回的参数
    volatile	uchar ucUIDCL1[5];       // TypeA第一级序列号，包含BCC字节
    volatile	uchar ucUIDCL2[5];       // TypeA第二级序列号，包含BCC字节
    volatile	uchar ucUIDCL3[5];       // TypeA第三级序列号，包含BCC字节
    volatile  	uchar ucUIDB[4];         // TypeB序列号
    volatile	uchar ucSFGI;            // TypeA型射频卡，RATS后的正向帧等待时间整数
    
            // 不管PICC是否支持CID和NAD机制，
            // 按照PAYPASS要求，在PCD与PICC之间均不使用CID和NAD机制
    volatile	uchar ucCIDFlag;         // PICC是否支持CID机制，0 - 不支持， 1 - 支持
    volatile    uchar ucNADFlag;         // PICC是否支持NAD机制，0 - 不支持， 1 - 支持
    volatile	uchar ucWTX;             // 帧等待时间扩展整数，适用于TypeA和TypeB卡
    
    volatile	ulong ulSFGT;            // TypeA型射频卡，RATS后的正向帧等待时间， 单位：ETU（128/13.56MHz）
    volatile	ulong ulFWT;             // 帧等待时间，适用于TypeA和TypeB卡，单位：ETU（128/13.56MHz）
    volatile	ulong ulFWTTemp;         // 帧等待时间扩展，适用于TypeA和TypeB卡，单位：ETU（128/13.56MHz）
    
    volatile	uint  FSC;      		 // FSC的值,卡能接收的帧最大长度
    volatile  	uint  FSD;      		 // FSD的值，终端能接收的帧最大长度
    
    volatile	uchar aucBuffer[272];   // 收发缓冲区
    volatile	uchar ucPN512Cmd;      // transmit, idle, transceive命令类型
    
            // added by liuxl 20070928 用于记录询卡时出现的错误位置，以便定位错误原因
    volatile	ushort usErrNo; 
    volatile	uchar  ucInProtocol;    // 卡片进入ISO14443协议态标志(主要用于判断PRO_S50/PRO_S70是进行M1操作还是进行TYPE A操作)
    volatile	uchar  ucCollPos;       // 冲突位置
    
    volatile	uchar  ucCmd;
    volatile	uchar  ucHalt;
    volatile	uchar  ucRemove_Step;
    volatile	uchar  ucStatus;
    volatile	uchar  ucRemoved;
    volatile	uchar  ucM1Stage;
            
    volatile	uchar ucPollMode; // added by liuxl，用于区别emv和paypass 

    //以下为felica驱动相关成员
    volatile	uchar f_ispoll;//表示是否为polling命令
}RF_WORKSTRUCT;

typedef RF_WORKSTRUCT PN512_RF_WORKSTRUCT;//保持一致性


typedef struct
{
	int a_conduct_w;  	//A型卡输出电导写入允许：1--允许，其它值—不允许，
	int a_conduct_val;  // A型卡输出电导控制变量，有效范围0~63,超出时视为63
	
	int m_conduct_w;  	//M型卡输出电导写入允许：1--允许，其它值—不允许，
	int m_conduct_val;  // M型卡输出电导控制变量，有效范围0~63,超出时视为63
	
	int b_modulate_w;  
	int b_modulate_val;  // B型卡调制指数控制变量，有效范围0~63,超出时视为63
	
	int card_RxThreshold_w;   // 接收灵敏度写入允许：1－允许，其它值－不允许*/
	int card_RxThreshold_val;  // 接收灵敏度，有效范围0～255，默认值与机型相关 */
	
	int f_modulate_w;   
	int f_modulate_val;  //FeliCa卡调制指数控制变量，有效范围0~63,超出时视为63
	
	int card_buffer_w;    //卡片接收缓冲区大小写入允许：1--允许，其它值—不允许
	int card_buffer_val;   //卡片接收缓冲区大小参数（单位：字节），有效值1~256。                       
	                   
	int card_type_check_w;   // 卡片类型检查写入允许，该值不可读
	int card_type_check_val;  // 0-检查卡片类型，其他－不检查卡片类型(默认为检查卡  

	uchar wait_retry_limit_w;//added in V1.00F,20071212
	ushort wait_retry_limit_val;//max retry count for WTX block requests,default 3
        
	int Reserved[20];       //保留字节，用于将来扩展；写入时应全清零
}ST_PICC_PARA;

#define PICC_PARA  ST_PICC_PARA//保持与以前命名一致


	
typedef struct
{
   uchar   	    Command[ 4 ]; // CLA INS  P1 P2
   ushort  	    Lc;           // P3
   uchar   	    DataIn[512];
   ushort  	    Le;
} APDU_SEND;

typedef struct
{
   ushort       LenOut;    // length of dataout 
   uchar 	    DataOut[512];
   uchar        SWA;
   uchar        SWB;
} APDU_RESP;



#if (TERM_TYPE==TERM_KF900)
 
#include <asm/uaccess.h>
#include<asm/arch/irqs.h>//中断号
#include<asm/mach/irq.h>//中断结构体定义
#include <asm/arch/io.h>//虚拟地址映射方式
//#include<asm/arch/regs-gpio.h>//gpio寄存器虚实地址转换
//#include<asm/arch/regs-irq.h>//interrupt寄存器虚实地址转换
//#include<asm/arch/regs-clock.h>

#define SPI0_BASE       0xFFFEE000
#define SPI1_BASE       0xFFFEF000

#define GPIO0_BASE      0xFFFF5000
#define GPIO1_BASE      0xFFFF6000
#define GPIO2_BASE      0xFFFF7000

#define PMU_BASE        0xFFFFE000

//FOR SPI

#define rSPI1_DAT_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X000))
#define rSPI1_CTL_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X004))
#define rSPI1_STA_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X008))
#define rSPI1_MOD_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X00C))
#define rSPI1_DIAG_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X010))
#define rSPI1_BRG_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X014))
#define rSPI1_DMA_REG	(*(volatile unsigned *) IO_ADDRESS(SPI1_BASE+0X018))

//FOR INTERRUPT
#define rGPIO2_EN		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X000))//1-GPIO FUN 0-OTHER FUN
#define rGPIO2_OE		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X00C))//1-OUTPUT EN 0-OUTPUT DISABLE
#define rGPIO2_OUT		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X018))//1-SET 1 0-SET 0
#define rGPIO2_IN		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X024))//0-READ 0 1-READ 1
#define rGPIO2_IMOD		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X028))//0-LEVEL INT 1-EDGE INT
#define rGPIO2_IPOL		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X02C))//0-FALLING 1-RISING
#define rGPIO2_ISEL		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X030))//0-USE CHANNEL A INT 1-USE CHANNEL B INT
#define rGPIO2_IEN		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X034))//0-DISABLE INT 1-ENABLE INT
#define rGPIO2_IAST		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X040))//0- NO A INT 1-HAVE A INT
#define rGPIO2_IBST		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X044))//0- NO B INT 1-HAVE B INT
#define rGPIO2_ICLR		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X048))//1-CLEAN INT
#define rGPIO2_WKEN		(*(volatile unsigned *) IO_ADDRESS(GPIO2_BASE+0X04C))//0-PMU WAKE DISABLE 1-PMU WAKE ENABLE




#define rGPIO1_EN		(*(volatile unsigned *) IO_ADDRESS(GPIO1_BASE+0X000))//1-GPIO FUN 0-OTHER FUN
#define rGPIO1_OE		(*(volatile unsigned *) IO_ADDRESS(GPIO1_BASE+0X00C))//1-OUTPUT EN 0-OUTPUT DISABLE
#define rGPIO1_OUT		(*(volatile unsigned *) IO_ADDRESS(GPIO1_BASE+0X018))//1-SET 1 0-SET 0


//FOR PMU CLK
#define rPMU_CKEN_REG	(*(volatile unsigned *) IO_ADDRESS(PMU_BASE+0X008))//CLK ENABLE
#define	PMU_SPI1_CKEN	(0X1<<15)

#define BIT_EINT1      (0x1<<1)
#define BIT_EINT3      (0x1<<3)

#endif

#if (TERM_TYPE==TERM_KF322)

#include "stm32f10x_it.h"
#endif

#ifndef PICC_LED_RED
#define PICC_LED_RED    0x01  //红灯
#endif
#ifndef PICC_LED_GREEN
#define PICC_LED_GREEN  0x02//绿灯
#endif
#ifndef PICC_LED_YELLOW
#define PICC_LED_YELLOW 0x04 //黄灯
#endif
#ifndef PICC_LED_BLUE
#define PICC_LED_BLUE   0x08//蓝灯
#endif


// 寄存器位定义
#ifndef BIT0
 #define BIT0    ((uint)1<<0)
#endif
#ifndef BIT1
 #define BIT1    ((uint)1<<1)
#endif
#ifndef BIT2
 #define BIT2    ((uint)1<<2)
#endif
#ifndef BIT3
 #define BIT3    ((uint)1<<3)
#endif
#ifndef BIT4
 #define BIT4    ((uint)1<<4)
#endif
#ifndef BIT5
 #define BIT5    ((uint)1<<5)
#endif
#ifndef BIT6
 #define BIT6    ((uint)1<<6)
#endif
#ifndef BIT7
 #define BIT7    ((uint)1<<7)
#endif
#ifndef BIT8
 #define BIT8   ((uint)1<<8)
#endif
#ifndef BIT9
 #define BIT9   ((uint)1<<9)
#endif
#ifndef BIT10
 #define BIT10    ((uint)1<<10)
#endif
#ifndef BIT11
 #define BIT11    ((uint)1<<11)
#endif
#ifndef BIT12
 #define BIT12    ((uint)1<<12)
#endif
#ifndef BIT13
 #define BIT13    ((uint)1<<13)
#endif
#ifndef BIT14
 #define BIT14    ((uint)1<<14)
#endif
#ifndef BIT15
 #define BIT15    ((uint)1<<15)
#endif
#ifndef BIT16
 #define BIT16    ((uint)1<<16)
#endif
#ifndef BIT17
 #define BIT17    ((uint)1<<17)
#endif
#ifndef BIT18
 #define BIT18   ((uint)1<<18)
#endif
#ifndef BIT19
 #define BIT19   ((uint)1<<19)
#endif
#ifndef BIT20
 #define BIT20    ((uint)1<<20)
#endif
#ifndef BIT21
 #define BIT21    ((uint)1<<21)
#endif
#ifndef BIT22
 #define BIT22    ((uint)1<<22)
#endif
#ifndef BIT23
 #define BIT23    ((uint)1<<23)
#endif
#ifndef BIT24
 #define BIT24    ((uint)1<<24)
#endif
#ifndef BIT25
 #define BIT25    ((uint)1<<25)
#endif
#ifndef BIT26
 #define BIT26    ((uint)1<<26)
#endif
#ifndef BIT27
 #define BIT27    ((uint)1<<27)
#endif
#ifndef BIT28
 #define BIT28   ((uint)1<<28)
#endif
#ifndef BIT29
 #define BIT29   ((uint)1<<29)
#endif
#ifndef BIT30
 #define BIT30   ((uint)1<<30)
#endif
#ifndef BIT31
 #define BIT31   ((uint)1<<31)
#endif
#ifndef BIT32
 #define BIT32   ((uint)1<<32)
#endif

#ifndef NULL
  #define NULL   ((void*)0)
#endif


#include "MifreHal.h"
#include "MifreCmd.h"
#include "Mifre.h"


#if (TERM_TYPE == TERM_KF322)
//#define	RFINT	BIT0
#define	RFINT	BIT1
#define RFRESET BIT0

#define  DelayMs     delay_ms
#define  picc_open PiccOpen
#define  picc_close PiccClose
#define  picc_detect PiccDetect
#define  picc_isocommand PiccIsoCommand
#define  piccremove PiccRemove
#define  picc_setup PiccSetup
#define  picc_cmdexchange PiccCmdExchange

#endif

#endif





