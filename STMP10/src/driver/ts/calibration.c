/****************************************Copyright (c)****************************************************
**                               vanstone Co.,LTD
**
**                                 http://www.vanstone.com.cn
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           calibration.c
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
#include "stdlib.h"
#include "stm32f10x_lib.h"
#include "tsc2046.h"
#include "calibration.h"
#include "..\lcd\lcd.h"
#include "..\kb\kb.h"
#include "vosapi.h"
#include "string.h"
#include "comm.h"
#include "kf701dh.h"

#define MIN_AD_X 0
#define MAX_AD_X 4094
#define MIN_AD_Y 0
#define MAX_AD_Y 4094
#define GET_COUNT (10)

#ifndef LCD_AD_FLAG
#define LCD_AD_FLAG (3000)
#endif
extern unsigned long LCDValue;//<3000 新屏 >=3000 旧屏 新屏值2043左右 老屏值3520左右

calibration cal;
unsigned char sign_buffer[136][48];
unsigned char sign_data[850];
BYTE gs_PadStatus = PAD_STAT_NO_OPEN; 
BYTE sign_finish = 0;
static int (*pad_callback_func)(void) = NULL;
#define MAXTIMER 1501
#define SIGN_TIMEOUT -2;
static unsigned int Pad_TimerSet(void)//系统开机时就先调用一下,把定时器启运起来
{
	unsigned int TimeId = Lib_CheckTimer(0);  //得到是100ms级别的
	
	if(TimeId == 0)
		Lib_SetTimer(0, MAXTIMER);

	return TimeId;
}

unsigned int Pad_TimerCheck(unsigned int Timeid, unsigned int ms )
{
 	unsigned short TimeOut = (ms + 99) / 100;
	unsigned short CurTime = Lib_CheckTimer(0);
	
	if (CurTime == 0) 				//重新设定定时器
		Lib_SetTimer(0, MAXTIMER);

	if(Timeid >= CurTime)                   //当没有重新开这个定时器时就走这个支路
	{
		if(Timeid - CurTime >= TimeOut)
		{
			return 1;
		}
		else
			return 0;
	}
	else
	{
		if((unsigned short)(MAXTIMER - CurTime + Timeid) >= TimeOut)
		{
			return 1;
		}
		else
			return 0;
	}
}

static bool check_touch_pressed(int xPhys,int yPhys)
{
    if ((xPhys < MIN_AD_X) ||
        (xPhys > MAX_AD_X) ||
        (yPhys < MIN_AD_Y) ||
        (yPhys > MAX_AD_Y))
    {
        return (FALSE);
    }
    return (TRUE);
}


int perform_calibration(calibration *cal) 
{	
    int j;	
    float n, x, y, x2, y2, xy, z, zx, zy;	
    float det, a, b, c, e, f, i;	
    float scaling = 65536.0;// Get sums for matrix	
    n = x = y = x2 = y2 = xy = 0;	
    for(j=0;j<5;j++) 
    {		
        n += 1.0;		
        x += (float)cal->x[j];		
        y += (float)cal->y[j];		
        x2 += (float)(cal->x[j]*cal->x[j]);		
        y2 += (float)(cal->y[j]*cal->y[j]);		
        xy += (float)(cal->x[j]*cal->y[j]);	
    }// Get determinant of matrix -- check if determinant is too small	
    det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);	
    if(det < 0.1 && det > -0.1) 
    {		
        trace_debug_printf("ts_calibrate: determinant is too small -- %f\n\r",det);		
        return 0;	
    }
    // Get elements of inverse matrix	
    a = (x2*y2 - xy*xy)/det;	
    b = (xy*y - x*y2)/det;	
    c = (x*xy - y*x2)/det;	
    e = (n*y2 - y*y)/det;	
    f = (x*y - n*xy)/det;	
    i = (n*x2 - x*x)/det;

    // Get sums for x calibration	
    z = zx = zy = 0;	
    for(j=0;j<5;j++) 
    {		
        z += (float)cal->xfb[j];		
        zx += (float)(cal->xfb[j]*cal->x[j]);		
        zy += (float)(cal->xfb[j]*cal->y[j]);	
    }

    // Now multiply out to get the calibration for framebuffer x coord	
    cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));	
    cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));	
    cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));	
    trace_debug_printf("Now: %f %f %f\n\r",(a*z + b*zx + c*zy),				
                                        (b*z + e*zx + f*zy),
                                        (c*z + f*zx + i*zy));
    // Get sums for y calibration	
    z = zx = zy = 0;	
    for(j=0;j<5;j++) 
    {		
        z += (float)cal->yfb[j];		
        zx += (float)(cal->yfb[j]*cal->x[j]);		
        zy += (float)(cal->yfb[j]*cal->y[j]);	
    }

    // Now multiply out to get the calibration for framebuffer y coord	
    cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));	
    cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));	
    cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));	
    trace_debug_printf("multiply: %f %f %f\n\r",(a*z + b*zx + c*zy),
                                        (b*z + e*zx + f*zy),
                                          (c*z + f*zx + i*zy));

    // If we got here, we're OK, so assign scaling to a[6] and return	
    cal->a[6] = (int)scaling;	
    return 1;
}

bool getxy(unsigned int *x, unsigned int *y)
{
	int x1,y1;
	int x2,y2;
	int i;

    *x = 0;
	*y = 0; 

//	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) == Bit_SET);
//获取10次，求平均值

    while(TSC2046_GetIRQStatus());
    x1 = TSC2046_Read_X();
    y1 = TSC2046_Read_Y();

    for(i = 0;i < 10;i++);
    x2 = TSC2046_Read_X();
    y2 = TSC2046_Read_Y();
    
    if(abs(x1 - x2) < 200 && abs(y1 - y2) < 200)
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
//        if(*x != 4095)
//        trace_debug_printf("\n*x = %d,*y = %d\n",*x,*y);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

static int get_sample (calibration *cal,
			int index, int x, int y, char *name)
{
    //static int last_x = -1, last_y;
    unsigned int xTmp,yTmp;

//    trace_debug_printf("%04x\n",*name);
    //draw cross   
    Lib_LcdCls();
    if(*name == '0')
        Lib_LcdPrintxy(x,y,0,"+");
    else if(*name == '1')
        Lib_LcdPrintxy(x-5,y,0,"+");
    else if(*name == '2')
        Lib_LcdPrintxy(x,y-7,0,"+");
    else if(*name == '3')
        Lib_LcdPrintxy(x-5,y-7,0,"+");
    else
        Lib_LcdPrintxy(x,y,0,"+");
    while(1)
    {
      if(!TSC2046_GetIRQStatus()) {
        if(getxy(&xTmp, &yTmp))
        {
            if(check_touch_pressed(xTmp, yTmp))
            {
                cal->x[index] = xTmp;
                cal->y[index] = yTmp;
                break;
            }
        }
      }
      if (pad_callback_func) {
        if ((*pad_callback_func)()) {
          return -1;
        }
      }
    }
    //draw cross
    while(!TSC2046_GetIRQStatus()) {
     if (pad_callback_func) {
        if ((*pad_callback_func)()) {
          return -1;
        }
      }
    }
    //last_x = cal->xfb [index] = x;	
    //last_y = cal->yfb [index] = y;

    trace_debug_printf("%s : LCD X = %4d Y = %4d\n\r", name, cal->xfb [index], cal->yfb [index]);	
    trace_debug_printf("%s : X = %4d Y = %4d\n\r", name, cal->x [index], cal->y [index]);
    return 0;
}

/*
*校准信息存储在66*1024-101 这个位置
*/
int s_Padcalibration(void)
{
    int i,j, ret;
    unsigned char cal_buffer[256];
    unsigned char cal_flag = 0;
 //   unsigned char cal_data[60];
//    int xtmp,ytmp,x1,y1,x2,y2;
//    BYTE key;
    
    memset(cal_buffer,0x00,sizeof(cal_buffer));

    cal_flag = 0;
    Lib_TscWrite(0,&cal_flag,1);

    ret = get_sample(&cal,0,0,0,"0");
    if (ret) {
      trace_debug_printf("Calibration failed 1.\n\r");
      return 0x1b;
    }
    ret = get_sample(&cal,1,131,0,"1");
    if (ret) {
      trace_debug_printf("Calibration failed 2.\n\r");
      return 0x1b;
    }
    ret = get_sample(&cal,2,0,47,"2");
    if (ret) {
      trace_debug_printf("Calibration failed 3.\n\r");
      return 0x1b;
    }
    ret = get_sample(&cal,3,131,47,"3");
    if (ret) {
      trace_debug_printf("Calibration failed 4.\n\r");
      return 0x1b;
    }
    ret = get_sample(&cal,4,65,24,"4");
    if (ret) {
      trace_debug_printf("Calibration failed 5.\n\r");
      return 0x1b;
    }
/*    
    for(i = 0;i < 5;i++)
    {
        trace_debug_printf("\n**cal.x = %d,cal.xfb = %d,cal.y = %d,cal.yfb = %d\n",cal.x[i],cal.xfb[i],cal.y[i],cal.yfb[i]);
    }
*/    
    if(perform_calibration (&cal))
    {
        trace_debug_printf("Calibration constants: ");				
        //sprintf (cal_buffer,"%d %d %d %d %d %d %d ",			 
        //            cal.a[0], cal.a[1], cal.a[2],
        //            cal.a[3], cal.a[4], cal.a[5], cal.a[6]);
        //sprintf (cal_buffer,"%x",			 
        //            cal.a[0]);
        trace_debug_printf("cal.a = %d %d %d %d %d %d %d",
                    cal.a[0], cal.a[1], cal.a[2],
                    cal.a[3], cal.a[4], cal.a[5], cal.a[6]);
        trace_debug_printf("\n\r");
        for(i = 0;i < 7;i++)
        {
            for(j = 0;j < 4;j++)
            {
                cal_buffer[4 * i + j] = (cal.a[i] >> (24 - (j*8))) & 0xff;
            }            
        }
        //len = strlen(cal_buffer);
        //for(i = 0;i < 28;i++)
        //   trace_debug_printf("%4d",cal_buffer[i]);
        //tmp = (cal_buffer[0] << 24 )|(cal_buffer[1] << 16 )|(cal_buffer[2] << 8 )|(cal_buffer[3] );
        //trace_debug_printf("\n\r%d",tmp);
        Lib_TscWrite(1,cal_buffer,28);
        Lib_LcdCls();
        cal_flag = 1;
        Lib_TscWrite(0,&cal_flag,1);
        return 0;
        //i = 0;
    }
    else 
    {
        trace_debug_printf("Calibration failed.\n\r");		
        //i = -1;	
        Lib_LcdCls();
        cal_flag = 0;
        Lib_TscWrite(0,&cal_flag,1);
        return -1;
    }
}

int Lib_Padcalibration(void)
{
    int result;

    if(LCDValue >= LCD_AD_FLAG)     // 因为二合一，所以要判断一下是不是允许
        return -1;

    gs_PadStatus = PAD_STAT_TS_CALIBRATING;
    result = s_Padcalibration();
    gs_PadStatus = PAD_STAT_IDLE;
    return result;
}
/*
unsigned int s_PadRecali()
{
    unsigned char cal_flag;
    int flag;

    cal_flag = 0;
    flag = Lib_TscWrite(0,&cal_flag,1);
    trace_debug_printf("flag = %x\n\r",flag);
    
    return flag;
}

int Lib_PadRecali(void)
{
    s_PadRecali();
    return 0;
}
*/
int s_PadSign(unsigned char * pcode,unsigned char TimeOut)
{
    int xtmp,ytmp,x1,y1,x2,y2;
    int i,j;
    char sign_tmp,tmp,tmp_r;
    unsigned int sign_len;
    unsigned int TimeId = 0;
    BYTE key;
    int lSinCnt = 0, lSinRealCnt = 0;            // 记录下签名时下写的点的个数
    unsigned char pad_code[10],code_len;
   
    memset(sign_buffer, 0x00, sizeof(sign_buffer));
    memset(sign_data, 0x00, sizeof(sign_data));
    memset(pad_code,0x00,sizeof(pad_code));
    sign_len = 0;
    sign_finish = 0;//每次进来签名，都需要将签名完成标志置位

    code_len = 8;//strlen(pcode);
    memcpy(pad_code,pcode,code_len);
    //pad_code[code_len] = '\0';
    //trace_debug_printf("code_len = %d\n\r",code_len);
    Lib_LcdCls();
    Lib_LcdSetFont(8, 16, 0);
	//Lib_LcdClrLine(1*8, 32-1);
	Lib_LcdGotoxy(40, 2*8);
//	trace_debug_printf("\n\rcode\n\r");
    for(i = 0;i < code_len;i++)
    {
        Lib_Lcdprintf("%c",pad_code[i]);
        //Lib_Lcdprintf(" ");
    }
//    for(i = 0;i < 31;i++)
//        for(j = 0;j < 122;j++)
//            if(sign_buffer[j][i] == 1)
//                trace_debug_printf("i = %d,j = %d\n\r",i,j);
    for(i = 0;i < 48;i++)
   {
       for(j = 0;j < 132;j++)
       {
           if (sign_buffer[j][i])
           {
               lSinCnt++;
           } 
       }
    }
    x2 = -1;
    y2 = -1;
    sign_tmp = 0;
    tmp = 7;
    tmp_r = 3;

//    for(i = 0;i < 7;i++)
//        trace_debug_printf("cal.a = %4d\n",cal.a[i]);
    TimeId = Pad_TimerSet();
    
    while(1)
    {
        if(!TSC2046_GetIRQStatus())
        {
            // TimeId = Pad_TimerSet();         // 不再重计时
            getxy(&cal.x[0],&cal.y[0]);
            xtmp = cal.x[0];
            ytmp = cal.y[0]; 
//            trace_debug_printf("before xtmp = %4d,ytmp = %4d\n",xtmp,ytmp);
            x1 = (int)(( cal.a[0] +
        			      cal.a[1]*xtmp + 
        				   cal.a[2]*ytmp ) / cal.a[6]);
            y1 =  (int)(( cal.a[3] +
        				   cal.a[4]*xtmp +
        					cal.a[5]*ytmp ) / cal.a[6]);
//        	trace_debug_printf("after Calibration x = %d y=%d\n\r",x1,y1);
            //if((x1 < 132)&&(x1 >= 0)&&(y1 < 48)&&(y1 >= 0))
               // sign_buffer[x1][y1] = 1;               
            if((x1 < 132)&&(x1 >= 0)&&(y1 < 48)&&(y1 >= 0))
        	    Lib_LcdDrawLine(x1,y1,x2,y2,1);
        	if(!TSC2046_GetIRQStatus())
        	{
            	x2 = x1;
            	y2 = y1;
        	}
        	else
        	{
        	    x2 = -1;
        	    y2 = -1;
        	    for(i = 0;i < 10000;i++);
        	}
    	}
        if(Pad_TimerCheck(TimeId, TimeOut*1000) == 1 )
        {
            Lib_LcdCls();
	        gs_PadStatus = PAD_STAT_IDLE ;
    	    Lib_Lcdprintf("sign timeout!!!");
    	    delay_ms(800);
            return SIGN_TIMEOUT;
        }
        if (pad_callback_func) {
          if ((*pad_callback_func)()) {
              Lib_LcdCls();
             // Lib_Lcdprintf("CANCEL!!!");
             // delay_ms(800);
              memset(sign_buffer, 0x00, sizeof(sign_buffer));
              memset(sign_data, 0x00, sizeof(sign_data));
              sign_len = 0;
              sign_tmp = 0;
              sign_finish = 2;
              gs_PadStatus = PAD_STAT_IDLE ;
              return 0x1B;
          }
      }
        
    	if(0 == Lib_KbCheck())
    	{
    	    key = Lib_KbGetCh();
    	    //trace_debug_printf("%d",key);
    	    switch(key)
    	    {
    	        case KEYCANCEL:
    	            Lib_LcdCls();
    	            //Lib_Lcdprintf("CANCEL!!!");
    	            //delay_ms(800);
    	            memset(sign_buffer, 0x00, sizeof(sign_buffer));
    	            memset(sign_data, 0x00, sizeof(sign_data));
    	            sign_len = 0;
    	            sign_tmp = 0;
    	            sign_finish = 2;
    	            gs_PadStatus = PAD_STAT_IDLE ;
    	            return 0x1B;

                case KEYENTER://上报
    	            for(i = 0;i < 48;i++)
    	            {
    	                for(j = 0;j < 132;j++)
    	                {
                            if (sign_buffer[j][i])
                            {
                                lSinRealCnt++;
                            } 
    	                    if(j < 128)
    	                    {
                                
    	                        if((j%8 == 0)&&(j != 0))
        	                    {        	                        
        	                        //trace_debug_printf("%3x",sign_tmp);
        	                        sign_data[sign_len] = sign_tmp;
        	                        sign_len ++;
        	                        sign_tmp = 0;
        	                        tmp = 7;
        	                    }
    	                        //trace_debug_printf("%d\n\r",sign_buffer[j][i]);
        	                    sign_tmp |= sign_buffer[j][i] << tmp;
        	                    //trace_debug_printf("sign_tmp = %3x\n\r",sign_tmp);
        	                    tmp--;
        	                    //if(tmp < 0)
        	                        
        	                    
    	                    }
    	                    else
    	                    {
    	                        //trace_debug_printf("\n\n\r");
    	                        if((j%8 == 0)&&(j != 0))
        	                    {
//        	                        trace_debug_printf("%3x\n\r",sign_tmp);
        	                        sign_data[sign_len] = sign_tmp;
        	                        sign_len ++;
        	                        sign_tmp = 0;
        	                        tmp = 7;
        	                    }
    	                        //trace_debug_printf("%d\n",sign_buffer[j][i]);
    	                        sign_tmp |= sign_buffer[j][i] << tmp_r;   	                        
//        	                    trace_debug_printf("sign_tmp = %d\n\r",sign_tmp);
    	                        tmp_r--;
    	                        if(j == 131)
    	                        {    
    	                            tmp_r = 3;
    	                            //trace_debug_printf("%3x\n\r",sign_tmp);
    	                            sign_data[sign_len] = sign_tmp;
//    	                            trace_debug_printf("sign_len = %d\n\r",sign_len);
        	                        sign_len ++;
    	                            sign_tmp = 0;
    	                        }
    	                    }    	                    
    	                }
    	            }//end for
    	            //sign_data[sign_len] = '\0';
    	            //trace_debug_printf("\n\n\r");
    	            #if 0
    	            //trace_debug_printf("sign+buff\n");
    	            for(i = 0;i < 48;i++)
    	            {
    	                for(j = 0;j < 136;j++)
    	                    trace_debug_printf("%2d",sign_buffer[j][i]);
    	                trace_debug_printf("\n\r");
    	            }
    	            trace_debug_printf("\n\n\r");
    	            
    	            for(i = 0;i <850;i++)
    	                trace_debug_printf("%3x",sign_data[i]);
    	            #endif
                    if (20 > lSinRealCnt- lSinCnt)       // 签名时小于20点则认为错误
                    {
                        lSinRealCnt = 0;
                        Lib_LcdCls();
                        Lib_LcdGotoxy(40, 2*8);
        	            Lib_Lcdprintf("Please resign!");
        	            //TimeId = Pad_TimerSet();          // 清除时，不重计时
        	            memset(sign_buffer, 0x00, sizeof(sign_buffer));
        	            memset(sign_data, 0x00, sizeof(sign_data));
        	            sign_len = 0;
        	            sign_tmp = 0;
        	            delay_ms(800);
        	            Lib_LcdCls();
        	            Lib_LcdSetFont(12, 12, 0);
                    	Lib_LcdClrLine(3*8, 32-1);
                    	Lib_LcdGotoxy(40, 2*8);
        	            for(i = 0;i < code_len;i++)
                        {
                            Lib_Lcdprintf("%c",pad_code[i]);
                            //Lib_Lcdprintf(" ");
                        }
        	            break;

                    }
    	            Lib_LcdCls();
    	            //Lib_Lcdprintf("sign over!!!");
    	            //delay_ms(800);
    	            //sign_len = 0;//此处需复位，防止连续按enter时候，buff指针溢出
    	            gs_PadStatus = PAD_STAT_SIGN_FINISH;
    	            sign_finish = 1;
    	            return 0;

                case KEYCLEAR:
    	            Lib_LcdCls();
                    Lib_LcdGotoxy(40, 2*8);
    	            Lib_Lcdprintf("Please resign!");
    	            //TimeId = Pad_TimerSet();          // 清除时，不重计时
    	            memset(sign_buffer, 0x00, sizeof(sign_buffer));
    	            memset(sign_data, 0x00, sizeof(sign_data));
    	            sign_len = 0;
    	            sign_tmp = 0;
    	            delay_ms(800);
    	            Lib_LcdCls();
    	            Lib_LcdSetFont(12, 12, 0);
                	Lib_LcdClrLine(3*8, 32-1);
                	Lib_LcdGotoxy(40, 2*8);
    	            for(i = 0;i < code_len;i++)
                    {
                        Lib_Lcdprintf("%c",pad_code[i]);
                        //Lib_Lcdprintf(" ");
                    }
    	            break;
    	        default:
    	            break;
    	    }
    	}
    }
}

/** 
* 启动签名 
* 
* @param pcode 特征码 
* 
* @return 0:正确 
* <0：特征码格式错误 
*/ 
int Lib_PadSign(unsigned char * pcode,unsigned char TimeOut)
{  
    int result;
    if(LCDValue >= LCD_AD_FLAG)     // 因为二合一，所以要判断一下是不是允许
        return -1;

    gs_PadStatus = PAD_STAT_SIGNNING;  
    result = s_PadSign(pcode,TimeOut);
//    gs_PadStatus = PAD_STAT_SIGN_FINISH;

    return result;
}

int Lib_PadSetCallback(int (*func)(void))
{
  pad_callback_func = func;
  return 0;
}

/** 
* 打开签名板 
* 
* @return 0：成功 
* <0：错误 
*/ 
extern void SPI1_Init(void);
int Lib_PadOpen(void)
{   
    unsigned int j,k;    
    uchar sBuf[100],cal_flag;

    if(LCDValue >= LCD_AD_FLAG)     // 因为二合一，所以要判断一下是不是允许
        return PADRECALI;

    SPI1_Init();
    Lib_TscRead(0,&cal_flag,1);
    if(cal_flag == 1)
    {
        memset(sBuf,0,sizeof(sBuf));    
        Lib_TscRead(1,sBuf,28);
        for(j = 0;j < 7;j++)
        {        
            for(k= 0;k < 4;k++)
            {            
                cal.a[j] |= (int)sBuf[4*j + k] << (24 - k*8);
            }
            //trace_debug_printf("%4d\n",cal.a[j]);
        }
        gs_PadStatus = PAD_STAT_IDLE;
        return 0;
    }
    else
    {
        return PADRECALI;
    }   
}

/** 
* 关闭签名板 
* 
* @return 0：成功 
* <0：错误 
*/
int Lib_PadClose(void)
{
    return 0;
}

/** 
* 检查签名板状态 
* 
* @return 
*/ 
int Lib_PadStatus(void)
{
    BYTE status;

    status = gs_PadStatus;

    return status;
}


/** 
* 读取签名点阵数据 
* 
* @param pbuf 存放数据的buffer 
* 
* @return >0:buffer的长度 
* <0：错误 
* 点阵数据格式如下：2byte + 2byte + 点阵数据(尽量保持和打印数据格式一致) 
*/ 
int Lib_PadRead(unsigned char * pbuf)
{
    unsigned int i,j,len,total_len;
    
    i = 0;    

    if((gs_PadStatus == PAD_STAT_SIGN_FINISH)&&(sign_finish == 1))
    {
       sign_finish = 0;
       *(pbuf + i) = LCD_WIDTH / 256;
       i++;
       *(pbuf + i) = LCD_WIDTH % 256;
       i++;
       *(pbuf + i) = LCD_HIGH/ 256;
       i++;
       *(pbuf + i) = LCD_HIGH % 256;
       i++;
       len = (LCD_WIDTH /8 + 1) * LCD_HIGH;
       //trace_debug_printf("len = %d\n\r",len);
       for(j = 0;j < len;j++)
       {
            *(pbuf + i + j) = sign_data[j];
            //trace_debug_printf("%3x",*(pbuf +i+ j));
       }
       total_len = len + 4;
       //trace_debug_printf("\n\r");
       //for(j = 0;j < total_len;j++)
      // {
      //     trace_debug_printf("%3x",*(pbuf + j));
      // }
      // trace_debug_printf("%3x",total_len);
       gs_PadStatus = PAD_STAT_IDLE;
       return total_len;
    }
    else if(sign_finish == 2)//签名失败 
        return -1;

    return -1;
}

/********************************************************************************************************
                                           END OF FILE
*********************************************************************************************************/

