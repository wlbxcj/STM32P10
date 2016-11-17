/*******************************************************************************
 * Copyright (C) 2006 ViewAt Technology Co., LTD.
 * All rights reserved.
 *******************************************************************************
 *
 * 文件名称: voice.h
 *
 * 文件描述: 语音模块头文件
 * 作    者: WLB
 * 创建时间: 2016-4-28
 * 文件版本: 
 * 修改历史: 
 *           1. 2016-4-28  WLB  Created
 *******************************************************************************/

#ifndef __VOICE_H__
#define __VOICE_H__

#include "stm32f10x_gpio.h"

/*******************************************************************************
 *                                宏定义                                       *
 *******************************************************************************/
    
    
#define BAONA_PROJECT   1               // 瀹濈撼椤圭洰
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
    
    
#if 0//BAONA_PROJECT
#define VOICE_S1    GPIO_Pin_4 //sxl
#define VOICE_S2    GPIO_Pin_9 //sxl
#define VOICE_S3    GPIO_Pin_3
#define VOICE_S4    GPIO_Pin_2
    
#define S1_PORT     GPIOC
#define S2_PORT     GPIOB
#define S3_PORT     GPIOA
#define S4_PORT     GPIOA
    
//#else
    //#define VOICE_S1   GPIO_Pin_14
#define VOICE_S1    GPIO_Pin_4 //sxl
    //#define VOICE_S2   GPIO_Pin_13
#define VOICE_S2    GPIO_Pin_9 //sxl
#define VOICE_S3    GPIO_Pin_12
#define VOICE_S4    GPIO_Pin_11
    
#define S1_PORT     GPIOC
#define S2_PORT     GPIOB
#define S3_PORT     GPIOA
#define S4_PORT     GPIOA
    
#endif
    
    //#define VOICE_STB  GPIO_Pin_15
#define VOICE_STB   GPIO_Pin_0 //sxl
    
    //#define SPY_CS     GPIO_Pin_3
#define SPY_CS      GPIO_Pin_2
    
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
    
    //#define SET_S1_HIGH       (GPIO_WriteBit(GPIOA, VOICE_S1,1))
    //#define SET_S1_LOW        (GPIO_WriteBit(GPIOA, VOICE_S1,0))
#define SET_S1_HIGH			(GPIO_WriteBit(S1_PORT, VOICE_S1, (BitAction)1)) //sxl
#define SET_S1_LOW			(GPIO_WriteBit(S1_PORT, VOICE_S1, (BitAction)0)) //sxl
    //#define SET_S2_HIGH       (GPIO_WriteBit(GPIOA, VOICE_S2,1))
    //#define SET_S2_LOW        (GPIO_WriteBit(GPIOA, VOICE_S2,0))
#define SET_S2_HIGH			(GPIO_WriteBit(S2_PORT, VOICE_S2, (BitAction)1)) //sxl
#define SET_S2_LOW			(GPIO_WriteBit(S2_PORT, VOICE_S2, (BitAction)0)) //sxl
#define SET_S3_HIGH			(GPIO_WriteBit(S3_PORT, VOICE_S3, (BitAction)1))
#define SET_S3_LOW			(GPIO_WriteBit(S3_PORT, VOICE_S3, (BitAction)0))
#define SET_S4_HIGH			(GPIO_WriteBit(S4_PORT, VOICE_S4, (BitAction)1))
#define SET_S4_LOW			(GPIO_WriteBit(S4_PORT, VOICE_S4, (BitAction)0)) 
    
#define SET_STB_HIGH		        (GPIO_WriteBit(GPIOA, VOICE_STB, (BitAction)1))
#define SET_STB_LOW			        (GPIO_WriteBit(GPIOA, VOICE_STB, (BitAction)0))
    
    //#define SET_SPY_CS_HIGH           (GPIO_WriteBit(GPIOB, SPY_CS,1)) 
#define SET_SPY_CS_HIGH		        (GPIO_WriteBit(GPIOB, SPY_CS, (BitAction)1)) 
    //#define SET_SPY_CS_LOW            (GPIO_WriteBit(GPIOB, SPY_CS,0))
#define SET_SPY_CS_LOW		        (GPIO_WriteBit(GPIOB, SPY_CS, (BitAction)0))


#define s_DelayUs WaitNuS
#define s_DelayMs1 delay_ms

/*******************************************************************************
 *                               数据类型                                      *
 *******************************************************************************/


/*******************************************************************************
 *                              全局变量声明                                   *
 *******************************************************************************/


/*******************************************************************************
 *                              全局函数原型                                   *
 *******************************************************************************/
extern int s_VoiceInit(void);

extern int SetVoice(uchar voiceID);


#endif

/*******************************************************************************
 *                                End of File                                  *
 *******************************************************************************/

