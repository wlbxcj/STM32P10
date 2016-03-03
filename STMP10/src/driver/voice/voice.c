#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h> 

#include "KF701DH.h"
#include "stm32f10x_lib.h"

#include "..\..\inc\FunctionList.h"

#include  "base.h"
#include  "comm.h"
#include  "vosapi.h"

////////////////////////////////////////////////////////////////////////
/*
PA11-------------S1
PA12------------S2
PA13------------S3
PA14------------S4
PA15------------SBT
PB3------------SPY0030_CS~ 
*/
#if 0
#define VOICE_S1   GPIO_Pin_11
#define VOICE_S2   GPIO_Pin_12
//#define VOICE_S3   GPIO_Pin_13
#define VOICE_S3   GPIO_Pin_9 //sxl
//#define VOICE_S4   GPIO_Pin_14
#define VOICE_S4   GPIO_Pin_4 //sxl
#endif
//顺序倒过来
//#define VOICE_S1   GPIO_Pin_14
#define VOICE_S1   GPIO_Pin_4 //sxl
//#define VOICE_S2   GPIO_Pin_13
#define VOICE_S2   GPIO_Pin_9 //sxl
#define VOICE_S3   GPIO_Pin_12
#define VOICE_S4   GPIO_Pin_11

//#define VOICE_STB  GPIO_Pin_15
#define VOICE_STB  GPIO_Pin_0 //sxl

//#define SPY_CS     GPIO_Pin_3
#define SPY_CS     GPIO_Pin_2

#if 0
#define SET_S1_HIGH			(GPIO_SetBits(GPIOA, VOICE_S1))
#define SET_S1_LOW			(GPIO_ResetBits(GPIOA, VOICE_S1))
#define SET_S2_HIGH			(GPIO_SetBits(GPIOA, VOICE_S2))
#define SET_S2_LOW			(GPIO_ResetBits(GPIOA, VOICE_S2))
#define SET_S3_HIGH			(GPIO_SetBits(GPIOA, VOICE_S3))
#define SET_S3_LOW			(GPIO_ResetBits(GPIOA, VOICE_S3))
#define SET_S4_HIGH			(GPIO_SetBits(GPIOA, VOICE_S4))
#define SET_S4_LOW			(GPIO_ResetBits(GPIOA, VOICE_S4)) 
#define SET_STB_HIGH		        (GPIO_SetBits(GPIOA, VOICE_STB))
#define SET_STB_LOW			(GPIO_ResetBits(GPIOA, VOICE_STB))

#define SET_SPY_CS_HIGH		        (GPIO_SetBits(GPIOB, SPY_CS)) 
#define SET_SPY_CS_LOW		        (GPIO_ResetBits(GPIOB, SPY_CS))

#endif

//#define SET_S1_HIGH	    (GPIO_WriteBit(GPIOA, VOICE_S1,1))
//#define SET_S1_LOW	    (GPIO_WriteBit(GPIOA, VOICE_S1,0))
#define SET_S1_HIGH			(GPIO_WriteBit(GPIOC, VOICE_S1, (BitAction)1)) //sxl
#define SET_S1_LOW			(GPIO_WriteBit(GPIOC, VOICE_S1, (BitAction)0)) //sxl
//#define SET_S2_HIGH	    (GPIO_WriteBit(GPIOA, VOICE_S2,1))
//#define SET_S2_LOW	    (GPIO_WriteBit(GPIOA, VOICE_S2,0))
#define SET_S2_HIGH			(GPIO_WriteBit(GPIOB, VOICE_S2, (BitAction)1)) //sxl
#define SET_S2_LOW			(GPIO_WriteBit(GPIOB, VOICE_S2, (BitAction)0)) //sxl
#define SET_S3_HIGH			(GPIO_WriteBit(GPIOA, VOICE_S3, (BitAction)1))
#define SET_S3_LOW			(GPIO_WriteBit(GPIOA, VOICE_S3, (BitAction)0))
#define SET_S4_HIGH			(GPIO_WriteBit(GPIOA, VOICE_S4, (BitAction)1))
#define SET_S4_LOW			(GPIO_WriteBit(GPIOA, VOICE_S4, (BitAction)0)) 

#define SET_STB_HIGH		        (GPIO_WriteBit(GPIOA, VOICE_STB, (BitAction)1))
#define SET_STB_LOW			        (GPIO_WriteBit(GPIOA, VOICE_STB, (BitAction)0))

//#define SET_SPY_CS_HIGH		    (GPIO_WriteBit(GPIOB, SPY_CS,1)) 
#define SET_SPY_CS_HIGH		        (GPIO_WriteBit(GPIOB, SPY_CS, (BitAction)1)) 
//#define SET_SPY_CS_LOW		    (GPIO_WriteBit(GPIOB, SPY_CS,0))
#define SET_SPY_CS_LOW		        (GPIO_WriteBit(GPIOB, SPY_CS, (BitAction)1))


#define s_DelayUs WaitNuS
#define s_DelayMs1 delay_ms

extern void WaitNuS(u32 x);

int s_VoiceInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
    GPIO_InitStructure.GPIO_Pin     = VOICE_S1|VOICE_S2|VOICE_S3|VOICE_S4|VOICE_STB; 
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    SET_S1_LOW;
    SET_S2_LOW;
    SET_S3_LOW;
    SET_S4_LOW;
    SET_STB_LOW;
    
    GPIO_InitStructure.GPIO_Pin     = SPY_CS; 
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    SET_SPY_CS_HIGH;
    
#if 0   
    //test
    for(;;)
    {
    SET_S1_LOW;
    SET_S2_LOW;
    SET_S3_LOW;
    SET_S4_LOW;
    SET_STB_LOW;
    SET_SPY_CS_LOW;      
    delay_ms(100);
    SET_S1_HIGH;
    SET_S2_HIGH;
    SET_S3_HIGH;
    SET_S4_HIGH;
    SET_STB_HIGH;
    SET_SPY_CS_HIGH;      
    delay_ms(10);
    
    }
#endif
    
    return 0;
}



int SetVoice(uchar voiceID)
{
	if (voiceID==0 || voiceID>12)
	{
		return VOICE_ID_ERR;
	}

	switch(voiceID)
	{
	case VOICE_PLS_INPUT_PWD: // H L L L
	//case VOICE_PLS_SWIPE_MAGCARD: // L L L H  //顺序要反？

		SET_S1_HIGH;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		//s_DelayMs1(30); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_PLS_INPUT_PWD_AGAIN: // L H L L
	//case VOICE_PWD_ERR: // L L H L

		SET_S1_LOW;
		SET_S2_HIGH;
		SET_S3_LOW;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		//s_DelayMs1(25); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_PWD_ERR: // L L H L
	//case VOICE_PLS_INPUT_PWD_AGAIN: // L H L L
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_HIGH;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_PLS_SWIPE_MAGCARD: // L L L H
        //case  VOICE_PLS_INPUT_PWD:         
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;
//below enable
	case VOICE_PLS_INSERT_ICCARD: // H H L L
		SET_S1_HIGH;
		SET_S2_HIGH;
		SET_S3_LOW;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;

		break;

	case VOICE_TRADE_SUCCESS:	// L H H L
		SET_S1_LOW;
		SET_S2_HIGH;
		SET_S3_HIGH;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_TRADE_FAILER: //L L H H
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_HIGH;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_WELCOME: //H L L H
		SET_S1_HIGH;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_THANKS_PATRONAGE: //H H H L
		SET_S1_HIGH;
		SET_S2_HIGH;
		SET_S3_HIGH;
		SET_S4_LOW;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_THANKS_USE: //L H H H
		SET_S1_LOW;
		SET_S2_HIGH;
		SET_S3_HIGH;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_COMM_FAILER: //H L H H
		SET_S1_HIGH;
		SET_S2_LOW;
		SET_S3_HIGH;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	case VOICE_BALANCE_LACK: //H H L H
		SET_S1_HIGH;
		SET_S2_HIGH;
		SET_S3_LOW;
		SET_S4_HIGH;
		s_DelayUs(10);
		SET_STB_HIGH;
		SET_SPY_CS_LOW;
		s_DelayMs1(20); // >= 20ms
		SET_SPY_CS_HIGH;
		SET_STB_LOW;

		s_DelayUs(10);
		SET_S1_LOW;
		SET_S2_LOW;
		SET_S3_LOW;
		SET_S4_LOW;
		break;

	default:
		break;
	}

	return 0;
}



