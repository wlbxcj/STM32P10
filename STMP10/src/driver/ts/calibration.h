/****************************************Copyright (c)****************************************************
**                               vanstone Co.,LTD
**
**                                 http://www.vanstone.com.cn
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           calibration.h
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
#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#define PADRECALI -2
typedef struct {
	unsigned int x[5], xfb[5];
	unsigned int y[5], yfb[5];
	unsigned int get_len,got_len;
	int a[7] ;
} calibration;

#define PAD_STAT_IDLE                         (0)
#define PAD_STAT_NO_OPEN                      (1)
#define PAD_STAT_SIGNNING                     (2)
#define PAD_STAT_SIGN_ERR                     (3)
#define PAD_STAT_SIGN_FINISH                  (4)
#define PAD_STAT_TS_CALIBRATING               (5)

//unsigned char Get_Timer;
int s_Padcalibration(void);
int s_PadSign(unsigned char * pcode,unsigned char TimeOut);
bool getxy(unsigned int *x, unsigned int *y);
#endif
/********************************************************************************************************
                                           END OF FILE
*********************************************************************************************************/

