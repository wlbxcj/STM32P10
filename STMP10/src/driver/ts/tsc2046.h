/****************************************Copyright (c)****************************************************
**                               vanstone Co.,LTD
**
**                                 http://www.vanstone.com.cn
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           TSC2046.h
** Last modified Date:  2014-9-17
** Last Version:        V1.0
** Descriptions:        touch
**
**--------------------------------------------------------------------------------------------------------
** Created by:          WXF
** Created date:        2014-9-17
** Version:             V1.0
** Descriptions:        touch
**
**--------------------------------------------------------------------------------------------------------
** Modified by:					WXF
** Modified date:				2014-9-17
** Version:
** Descriptions:
**
** Rechecked by:
*********************************************************************************************************/

#ifndef _TSC2046_H_
#define _TSC2046_H_

extern void TSC2046_init(void);
//extern void Timer_T2Init(void);
extern unsigned char TSC2046_Read_Position(unsigned short *X_Position, unsigned short *Y_Position);
extern unsigned char TSC2046_Read_Single(unsigned short *X_Position, unsigned short *Y_Position);
unsigned short TSC2046_Read_X(void);
unsigned short TSC2046_Read_Y(void);
void Timer_Delay(unsigned int count);
unsigned char TSC2046_GetIRQStatus(void);
#endif
/********************************************************************************************************
                                           END OF FILE
*********************************************************************************************************/

