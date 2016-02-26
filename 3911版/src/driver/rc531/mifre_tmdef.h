#ifndef _MIFARE_TMDEF_H_
#define	_MIFARE_TMDEF_H_

/**************************************************************
*common section
**************************************************************/

#define RF_L1_RECDBG//emv test switch

//#define Debug_Isr//test 

//#define USE_NEW_PROTOCOL//WHETHER USE NEW PROTOCOL


#define TERM_KF311  1
#define TERM_KF322  2
#define KF322_M

#define TERM_TYPE TERM_KF322//type switch

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
    volatile uchar ucRFOpen;
    volatile ulong ulSendBytes;      
    volatile ulong ulBytesSent;     
    volatile ulong ulBytesReceived;  
    volatile long lBitsReceived;    

    volatile uchar ucAnticol;       

    volatile uchar ucCurResult;     
    volatile uchar ucINTSource;     
    volatile uchar ucErrFlags;      
    volatile uchar ucSaveErrState;  

    volatile uchar ucMifCardType;
	volatile uchar ucCurType;        
    volatile uchar ucCurPCB;        

    volatile uchar ucFWI;            

	volatile uchar ucSAK1;           
	volatile uchar ucSAK2;           
	volatile uchar ucSAK3;           

	volatile uchar ucMifActivateFlag; 
	volatile uchar ucUIDLen;          
	volatile uchar ucATQA[2];         

	volatile uchar ucATQB[12];        
	volatile uchar ucUIDCL1[5];       
	volatile uchar ucUIDCL2[5];       
	volatile uchar ucUIDCL3[5];       
    volatile uchar ucUIDB[4];         
	volatile uchar ucSFGI;           

	
	volatile uchar ucCIDFlag;         
    volatile uchar ucNADFlag;         
	volatile uchar ucWTX;             

	volatile ulong ulSFGT;            
	volatile ulong ulFWT;             
	volatile ulong ulFWTTemp;         

	volatile uint  FSC;      		 
    volatile uint  FSD;      		 

	volatile uchar aucBuffer[272];   
	volatile uchar ucPN512Cmd;      

	
	volatile ushort usErrNo; 
	volatile uchar  ucInProtocol;    
	volatile uchar  ucCollPos;       

	volatile uchar  ucCmd;
	volatile uchar  ucHalt;
	volatile uchar  ucRemove_Step;
	volatile uchar  ucStatus;
	volatile uchar  ucRemoved;
	volatile uchar  ucM1Stage;
	

	volatile uchar ucPollMode; 

}RF_WORKSTRUCT;




typedef struct
{
	uchar  drv_ver[5];  //e.g. "1.01A", read only
	uchar drv_date[12]; //e.g. "2006.08.25",read only

	uchar a_conduct_w;  //Type A conduct write enable: 1--enable,else disable
	uchar a_conduct_val;//Type A output conduct value,0~63


	uchar b_modulate_w;  //Type B modulate index write enable,1--enable,else disable
	uchar b_modulate_val;//Type B modulate index value

	int card_RxThreshold_w;   // 接收灵敏度写入允许：1－允许，其它值－不允许*/
	int card_RxThreshold_val;  // 接收灵敏度，有效范围0～255，默认值与机型相关 */
        
	uchar card_buffer_w;//added in V1.00C,20061204
	ushort card_buffer_val;//max_frame_size of PICC

	uchar wait_retry_limit_w;//added in V1.00F,20071212
	ushort wait_retry_limit_val;//max retry count for WTX block requests,default 3

	// 20080617 
	uchar card_type_check_w; // 
	uchar card_type_check_val; //
	
	uchar card_RxThreshold_w;   // card RxThreshold
	uchar card_RxThreshold_val; 
	
	
	uchar f_modulate_w;   
	uchar f_modulate_val;  //FeliCa card modulate
	
	uchar reserved[20];  //

}PICC_PARA;

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
   uchar 	DataOut[512];
   uchar        SWA;
   uchar        SWB;
} APDU_RESP;


#ifndef PICC_LED_RED
#define PICC_LED_RED    0x01  
#endif
#ifndef PICC_LED_GREEN
#define PICC_LED_GREEN  0x02
#endif
#ifndef PICC_LED_YELLOW
#define PICC_LED_YELLOW 0x04
#endif
#ifndef PICC_LED_BLUE
#define PICC_LED_BLUE   0x08
#endif


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

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include  <ctype.h>
//include internal title
#include "mifrehal.h"
#include "mifrecmd.h"
#include "Mifre.h"
#if (TERM_TYPE == TERM_KF311)
#define	RFINT	BIT0
#include <MAXQ1850.h> 
#include <inmaxq.h>
#include "delay.h"
#define  DelayMs     delay_ms
#endif

#if (TERM_TYPE == TERM_KF322)
//#define	RFINT	BIT0
#define	RFINT	BIT1

#define  DelayMs     delay_ms
#endif

#endif




