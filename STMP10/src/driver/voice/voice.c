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

#include  "voice.h"

/* 原来的宏直接改为全局变量 默认为USB 的语音IO配置 */
unsigned short VOICE_S1 = GPIO_Pin_4;
unsigned short VOICE_S2 = GPIO_Pin_9;
unsigned short VOICE_S3 = GPIO_Pin_3;
unsigned short VOICE_S4 = GPIO_Pin_2;
    
GPIO_TypeDef *S1_PORT = GPIOC;
GPIO_TypeDef *S2_PORT = GPIOB;
GPIO_TypeDef *S3_PORT = GPIOA;
GPIO_TypeDef *S4_PORT = GPIOA;

extern void WaitNuS(u32 x);
extern unsigned long LCDValue;

int s_VoiceInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if (LCDValue >= 4000 || LCDValue <= 300)    // 带USB的设备，语音IO不一样
    {
        VOICE_S1 = GPIO_Pin_4;
        VOICE_S2 = GPIO_Pin_9;
        VOICE_S3 = GPIO_Pin_3;
        VOICE_S4 = GPIO_Pin_2;
            
        S1_PORT = GPIOC;
        S2_PORT = GPIOB;
        S3_PORT = GPIOA;
        S4_PORT = GPIOA;
    }
    else
    {
        VOICE_S1 = GPIO_Pin_4;
        VOICE_S2 = GPIO_Pin_9;
        VOICE_S3 = GPIO_Pin_12;
        VOICE_S4 = GPIO_Pin_11;
            
        S1_PORT = GPIOC;
        S2_PORT = GPIOB;
        S3_PORT = GPIOA;
        S4_PORT = GPIOA;
    }

    GPIO_InitStructure.GPIO_Pin     = VOICE_S1; 
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(S1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = VOICE_S2; 
    GPIO_Init(S2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = VOICE_S3; 
    GPIO_Init(S3_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = VOICE_S4; 
    GPIO_Init(S4_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = VOICE_STB; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin     = SPY_CS; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    SET_S1_LOW;
    SET_S2_LOW;
    SET_S3_LOW;
    SET_S4_LOW;
    SET_STB_LOW;
   
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
    s_VoiceInit();

    trace_debug_printf("SetVoice (%d)\r\n", voiceID);

	switch(voiceID)
	{
	case VOICE_PLS_INPUT_PWD: // H L L L
	//case VOICE_PLS_SWIPE_MAGCARD: // L L L H  //????????

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

	case VOICE_PLS_SWIPE_MAGCARD: // L L L H  4
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

	case VOICE_WELCOME: //H L L H   8
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



