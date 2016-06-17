
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "usb_conf.h"
#include "..\inc\vosapi.h" 
#include "pcicmd.h" 
#include "voice.h"
#include "var.h"

#define s_printf trace_debug_printf //12/11/08

#define TEST_INIT 1  /*yuhongyue for test*/
#define MAJ_VER		0
#define MIN_VER		1

unsigned short Rs_Inquiry(unsigned char *psRet,uchar *pnAmountBcd);


extern int Test_ShowTime(int mode);
extern int ShowVosVersion(void);
extern unsigned char ClssTransInit(void);
extern unsigned char getkey(void);
extern int  Contactless_SetTLVData(unsigned short usTag, uchar *pucDtIn, int nDtLen);
extern unsigned char disp_clss_err(int err);
extern void dat_bcdtoasc( uchar  *asc, uchar  *bcd, unsigned short asc_len);
extern int Contactless_GetTLVData(unsigned short Tag, uchar *DtOut, int *nDtLen);
extern void delay_ms(ulong t);

#if 1

enum TEST_ITEM {TEST_LCD, TEST_KB, TEST_LED, TEST_BEEP, TEST_VOICE, TEST_CLK, TEST_PICC, TEST_PSAM};

//void GetItem(unsigned char item, char *buf);
//BYTE WaitforKey(void);

void GetItem(unsigned char item, char *buf)
{
	switch(item)
	{
		case TEST_LCD:
			strcpy(buf,"Lcd ");
			break;
		case TEST_KB:
			strcpy(buf,"Kb ");
			break;
		case TEST_LED:
			strcpy(buf,"Led ");
			break;
		case TEST_BEEP:
			strcpy(buf,"Beep ");
			break;
		case TEST_VOICE:
			strcpy(buf,"Voice ");
			break;
		case TEST_CLK:
			strcpy(buf,"Clk ");
			break;
		case TEST_PICC:
			strcpy(buf,"Picc ");
			break;
		case TEST_PSAM:
			strcpy(buf,"Psam");
			break;
		default:
			break;
	}
}

unsigned char WaitEsc(unsigned short seconds)
{
  
	Lib_SetTimer(0,seconds*10);
	while(1){

          
		if(!Lib_CheckTimer(0)) break;
                
		if (!Lib_KbCheck()){
			if(Lib_KbGetCh()== KEYCANCEL) return 1;
			return 0;
		}
	}
	return 0;
}

BYTE WaitforKey(void)
{
	BYTE key;
	
	Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(0, 2*8, 0x00, "[Cancel]-Repeat");
	Lib_LcdPrintxy(0, 3*8, 0x00, "[Enter]-Continue");
	
	while(1)
	{
		if (Lib_KbCheck() == 0)
		{
			key = Lib_KbGetCh();
			if (key == KEYCANCEL || key == KEYENTER)
			{
				Lib_KbFlush();
				return key;
			}			
		}

	}
}

BYTE char_to_bin(BYTE bchar)
{
	if ((bchar >='0')&&(bchar <='9'))
		return(bchar-'0');
	if ((bchar >='A')&&(bchar <='F'))
		return(bchar-'A'+10);
	else
		return(0xf);
}

void asc_to_bcd(BYTE *asc, DWORD asc_len, BYTE *bcd)
{
	DWORD i,j;
	BYTE bOddFlag, bchar;

	bOddFlag =0;
	if (asc_len%2)		bOddFlag =1;

	for (i=0,j=0; j<asc_len; i++)
	{
		if ((i==0) && bOddFlag)
			bcd[i] = char_to_bin(asc[j++]);
		else
		{
			bchar = char_to_bin(asc[j++]);
			bcd[i] = bchar*16 + char_to_bin(asc[j++]);
		}
	}
}

void getstring(unsigned char ucKey, char *buf)
{
	switch(ucKey)
	{
		case KEY1:
			strcpy(buf,"KEY: 1      ");
			break;
		case KEY2:
			strcpy(buf,"KEY: 2      ");
			break;
		case KEY3:
			strcpy(buf,"KEY: 3      ");
			break;
		case KEY4:
			strcpy(buf,"KEY: 4      ");
			break;
		case KEY5:
			strcpy(buf,"KEY: 5      ");
			break;
		case KEY6:
			strcpy(buf,"KEY: 6      ");
			break;
		case KEY7:
			strcpy(buf,"KEY: 7      ");
			break;
		case KEY8:
			strcpy(buf,"KEY: 8      ");
			break;
		case KEY9:
			strcpy(buf,"KEY: 9      ");
			break;
		case KEY0:
			strcpy(buf,"KEY: 0      ");
			break;
#if 0                        
		case KEYDOWN:
			strcpy(buf,"KEY: DOWN   ");
			break;
		case KEYUP:
			strcpy(buf,"KEY: UP     ");
			break;
#endif                        
		case KEYCLEAR:
			strcpy(buf,"KEY: CLEAR  ");
			break;
		case KEYMENU:
			strcpy(buf,"KEY: MENU   ");
			break;
		case KEYFN:
			strcpy(buf,"KEY: FUNC  ");
			break;
		case KEYBACKSPACE:
			strcpy(buf,"KEY: <--    ");
			break;
		case KEYENTER:
			strcpy(buf,"KEY: ENTER  ");
			break;			  
		case KEYPRINTERUP:
			strcpy(buf,"KEY: REPRN  ");
			break;
		case KEYF1:
			strcpy(buf,"KEY: F1     ");
			break;	
		case KEYF2:
			strcpy(buf,"KEY: F2     ");		
			break;				  
		case KEYALPHA:
			strcpy(buf,"KEY: ALPHA  ");
			break;
		default:
			break;
	}
}


/***********************************

// mode:0 自动测试
// mode:1 手动测试

// burnIn:默认为0，为burn in专门设置；为其他值时，为burn in模式

***********************************/
int Test_LcdOnce(int mode, int burnIn)
{
	uchar i, j;//, ret;
	BYTE key;
	int sec;

	/*若循环，则等待时间短；若不循环，等待时间长一点*/
	if (mode == 0)
	{
		sec = 1;
	}
	else
	{
		sec = 2;
	}

	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "      LCD TEST      ");
	
	/*小字体*/
	Lib_LcdSetFont(8, 16, 0);
	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 1*8);
	Lib_Lcdprintf("abcdefghijklmnopqrstuvwxyz ");
	Lib_Lcdprintf("!@#$%&*()_-+=[]{},;.<>|\\/?");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
          
		if (WaitEsc(sec) == 1)
		{
                  
			Lib_LcdCls();
			return 1;			
		}
                
	}

	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 1*8);	
	Lib_Lcdprintf("ABCDEFGHIJKLMNOPQRSTUVWXYZ ");
	Lib_Lcdprintf("1234567890");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}

        //not support 16x16 v10p
       
	/*大字体*/
	//Lib_LcdSetFont(16, 16, 0);
	Lib_LcdSetFont(12, 12, 0);
	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
        
	Lib_LcdGotoxy(0, 0*8);
	Lib_Lcdprintf("abcdefghijklmnopqrstuvwxyz");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
	
	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 0*8);
	Lib_Lcdprintf("!@#$%&*()_-+=[]{},;.<>|\\/?");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
	
	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 0*8);
	Lib_Lcdprintf("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
	
	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 0*8);	
	Lib_Lcdprintf("1234567890");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}

	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 0*8);	
	//Lib_Lcdprintf("艾体威尔电子技术(北京)有限");
	Lib_LcdPrintxy(0,0,2,"艾体威尔电子技术有限公司");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}

	
      #if 0
	Lib_LcdClrLine(0*8, LCD_HIGH_MINI-1);
	Lib_LcdGotoxy(0, 0*8);	
	Lib_Lcdprintf("司-深圳研发中心");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
      #endif
	  
	/*开关背光*/
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "      LCD TEST      ");
	Lib_LcdSetBackLight(0);
	Lib_LcdPrintxy(2*6, 2*8, 0x00, "Close backlight");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
	
	Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
	Lib_LcdSetBackLight(1);
	Lib_LcdPrintxy(2*6, 2*8, 0x00, "Open backlight");
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}

	/*全黑*/
	Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(5*6, 2*8, 0x00, "Black");
	Lib_DelayMs(300);
	for(i=0; i<122; i++)
		for(j=0; j<32; j++)
			Lib_LcdDrawPlot(i, j, 1);
	if (burnIn != 0)
	{
		Lib_DelayMs(1000);
	}
	else
	{
		if (WaitEsc(sec) == 1)
		{
			Lib_LcdCls();
			return 1;			
		}
	}
	
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "      LCD TEST      ");
	Lib_LcdPrintxy(0*6, 1*8, 0x00, "Lcd test finished~!");
#if 0
	//WaitEsc(sec);
#else
	while (1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(500);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
	}
#endif
	Lib_LcdCls();
	return 0;
}

int Test_LcdLoop()
{
	while(1)
	{
		if (1 == Test_LcdOnce(0, 0))
		{
			break;
		}
	}
	
	return 0;
}

void Test_LcdFunc(void)
{
	BYTE key;

	while(1)
	{
		Lib_LcdCls();
		Lib_KbFlush();
		Lib_LcdPrintxy(0, 0, 0x80, "      LCD TEST      ");
		Lib_LcdPrintxy(5, 2*8, 0x00, "1-Once 2-Loop");

		while(1)
		{
			key = Lib_KbGetCh();

			if (KEYCANCEL == key)
			{
				return ;
			}

			if ((key < KEY1) || (key > KEY2))
			{
				continue;
			}

			if (KEY1 == key)
			{
				Test_LcdOnce(1, 0);
				break;
			}
			else if (KEY2 == key)
			{
				Test_LcdLoop();
				break;
			}
		}
	}
}

int Test_KbFunc(int mode)
{
	int i, j;
	uchar TestAll[256], buf[30];
	BYTE key;

//KB:
	while(1)
	{
		for (i=0; i< 256; i++)
		{
			TestAll[i] = 0;
		}
		
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0, 0x80, "       KB TEST      ");
		Lib_LcdPrintxy(1*6, 1*8, 0x00, "Pls press key...");

		while(1)
		{
			key = Lib_KbGetCh();
			if (KEYCANCEL == key)
			{
				j = 0;
				
				for (i =0; i<256; i++)
				{
					if (TestAll[i] == 1)
					{
						j++;
					}
				}

				if (14 == j)
				{
					Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
					Lib_LcdPrintxy(1*6, 1*8, 0x00, "Keyboard test ok~!");
					#if 0
					while (1)
					{
						if (mode == 0)
						{
							Lib_DelayMs(500);
							key = KEYENTER;
						}
						else
						{
							key = WaitforKey();
						}

						if (key == KEYENTER)
						{
							return 0;//break;
						}
						else if (key == KEYCANCEL)
						{
							goto KB;
						}
					}
					#else
					Lib_DelayMs(1000);
					return 0;
					#endif
					//return 0;
				}

				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdGotoxy(0, 1*8);
				Lib_LcdSetFont(8, 16, 0);
				Lib_Lcdprintf("Not finished!\n");
				#if 0
				Lib_Lcdprintf("[Cancel]-Exit\n");
				Lib_Lcdprintf("[Enter]-Again\n");

				key = Lib_KbGetCh();
				if (KEYCANCEL == key)
				{
					return 1;
				}
				break;
				#else
				if (KEYENTER == WaitforKey())
				{
					return 1;
				}
				break;
				#endif
			}
			
			/*其他按键的处理*/
			memset(buf, 0, sizeof(buf));
			getstring(key, (char *)buf);
			Lib_LcdPrintxy(0, 3*8, 0x00, "%s", (char *)buf);
			TestAll[key] = 1;
		}
	}
}

int Test_Clk(int mode)
{
	BYTE key, old_time[7], ucBuf[7];
	int iRet, i, get_time_return;

	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(0, 1*8, 0x00, "RTC test...");

	get_time_return = Lib_GetDateTime(old_time);
	/*Test Set Time*/
	//iRet = Lib_SetDateTime((BYTE *)"\x99\x12\x31\x23\x59\x58");
	iRet = Lib_SetDateTime((BYTE *)"\x99\x12\x31\x23\x59\x01");
	if(iRet)
	{
		Lib_LcdPrintxy(0, 1*8, 0x00, "RTC Test Error!");
		Lib_KbGetCh();
		return 1;
	}
	
	memset(ucBuf, 0, sizeof(ucBuf));
	Lib_GetDateTime(ucBuf);
	if(memcmp(ucBuf, "\x99\x12\x31\x23\x59\x58", 5))
	{
		Lib_LcdPrintxy(0, 1*8, 0x00, "RTC Test Error1!");
		Lib_KbGetCh();
		return 1;
	}
	
	Lib_SetDateTime((BYTE *)"\x99\x12\x31\x23\x59\x59");
	for(i=0; i<100; i++)
	{
		memset(ucBuf, 0, sizeof(ucBuf));
		Lib_GetDateTime(ucBuf);
		if(!memcmp(ucBuf, "\x00\x01\x01\x00\x00\x00\x06", 7))//06 means Saturday
			break;
		Lib_DelayMs(100);
	}
	
	if(i == 100)
	{
		Lib_LcdPrintxy(0, 1*8, 0x00, "RTC Test Error2!");
		Lib_KbGetCh();
		return 1;
	}

	if(get_time_return != 0)
	{
		Lib_SetDateTime((BYTE *)"\x12\x12\x12\x12\x12\x12"/*"\x02\x09\x01\x00\x00\x00"*/);
		Lib_LcdPrintxy(0, 1*8, 0x00, "RTC adjust itself~!");
		//Lib_KbGetCh();
		//return 1;
	}
	else
	{
		Lib_SetDateTime(old_time);
		Lib_LcdPrintxy(0, 1*8, 0x00, "RTC test ok~!");		
	}

	
	while (1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(500);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
	}

	//Lib_KbGetCh();
	return 0;
}

int Test_SetTime(int mode)
{
	int i, j, iRet;
	unsigned char temp[20], datetime[7];
	BYTE ch, key;
	
	while(1)
	{
		i = j = 0;
		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "Input DateTime:");
		Lib_LcdPrintxy(0, 2*8, 0x00, "YY/MM/DD HH:MM:SS");
		Lib_LcdPrintxy(0, 3*8, 0x00, "__/__/__ __:__:__");
		Lib_LcdGotoxy(0, 3*8);//go to ahead of "__/__/__/ __:__:__"
		Lib_LcdSetFont(8, 16, 0);
		memset(temp, 0, sizeof(temp));
		
		while(1)
		{
			j = i + i/2;/*越过"__/__/__/ __:__:__"中的'/'和':'*/
			Lib_LcdGotoxy((unsigned char)(j*6), 3*8);
			Lib_KbFlush();
			
			ch = Lib_KbGetCh();
			/*取消时间设置*/
			if(ch == KEYCANCEL)
				return 1;
			
			/*时间输入完成*/
			if((ch == KEYENTER) && (i > 11))
				break;
			
			/*修改输入的数字*/
			if((ch == KEYCLEAR)&&(i != 0))
			{
				i--;
				j = i+i/2;
				Lib_LcdGotoxy((unsigned char)(j*6),3*8);
				Lib_Lcdprintf("_");
				continue;
			}
			
			/*按键非数字键，这个要最后处理，不然上面的所有按键都跑到这里面了*/
			if((ch < KEY0)||(ch > KEY9))
			{
				Lib_Beep();
				continue;
			}
			
			/*时间输入完成还在按数字键*/
			if(i >= 12)
			{
				Lib_Beep();
				continue;
			}
			
			/*正常输入的处理*/
			Lib_Lcdprintf("%d", ch-'0');
			temp[i] = ch;
			i = i+1;
		}

		asc_to_bcd(temp, 12, datetime);
		iRet = Lib_SetDateTime(datetime);
		if(iRet)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Set DateTime Error!");
			Lib_LcdPrintxy(0, 3*8, 0x00, "Error=%d", iRet);
			Lib_KbGetCh();
			continue;
		}
		
		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "Set DateTime:");
		Lib_LcdPrintxy(0, 2*8, 0x00, "YY/MM/DD HH:MM:SS");
		Lib_GetDateTime(datetime);
		Lib_LcdPrintxy(0, 3*8, 0x00, "%02x/%02x/%02x %02x:%02x:%02x\n", 
			datetime[0], datetime[1], datetime[2], 
			datetime[3], datetime[4], datetime[5]);
			
		while (1)
		{
			if (mode == 0)
			{
				Lib_DelayMs(500);
				Test_ShowTime(0);
				key = KEYENTER;
			}
			else
			{
				key = Lib_KbGetCh();
			}

			if ((key == KEYCANCEL) || (key == KEYENTER))
			{
				break;
			}
		}
		
		//Lib_KbGetCh();
		return 0;
	}
	//return 0;
}

int Test_ShowTime(int mode)
{
	unsigned char datetime[7], oldtime[7];
	BYTE key;
	
        
	while(1)
	{
		Lib_GetDateTime(datetime);
		if(memcmp(oldtime, datetime, 6) != 0)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Cur DateTime:");
			Lib_LcdPrintxy(0, 2*8, 0x00, "YY/MM/DD HH:MM:SS");
			Lib_LcdPrintxy(0, 3*8, 0x00, "%02x/%02x/%02x %02x:%02x:%02x\n",
				datetime[0], datetime[1], datetime[2],
				datetime[3], datetime[4], datetime[5]);
			
			memcpy(oldtime, datetime, 6);
		}
		
		if (mode == 0)
		{
			Lib_DelayMs(1000);
			return 0;
		}
		else
		{
			if(Lib_KbCheck() == 0)
			{
				key = Lib_KbGetCh();
				if(key == KEYCANCEL || key == KEYENTER)
					return 0;
			}		
		}
		
	}//end of while(1)
}

void Test_ClkFunc()
{
	BYTE key;

	while(1)
	{
		Lib_LcdCls();
		Lib_KbFlush();
		Lib_LcdPrintxy(0, 0, 0x80, "      CLK TEST      ");
		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "1-Function test");
		Lib_LcdPrintxy(0, 2*8, 0x00, "2-Set time");
		Lib_LcdPrintxy(0, 3*8, 0x00, "3-Show time");

		while(1)
		{
			key = Lib_KbGetCh();

			if (KEYCANCEL == key)
			{
				return ;
			}

			if ((key < KEY1) || (key > KEY3))
			{
				continue;
			}

			switch (key)
			{
				case KEY1:
					Test_Clk(1);
					break;
				case KEY2:
					Test_SetTime(1);
					break;
				case KEY3:
					Test_ShowTime(1);
					break;
				default:
					break;
			}
			break;
		}
	}
}

#define MaxSlotNum	2

#if 0
int Test_PsamFunc(int mode)
{
	uchar i, j, ucBuf[80], key, err[4];
	APDU_SEND ApduSend;
	APDU_RESP ApduResp;
	int iret;

AGAIN:
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "     PSAM TEST      ");

	iret = 0;
	memset(err, 0x00, sizeof(err));

	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);

	for(i=0; i<2; i++)
	{
		if (mode != 0)
		{
			if (Lib_KbCheck() == 0)
			{
				if (KEYCANCEL == Lib_KbGetCh())
				{
					return 1;
				}
			}
		}

		Lib_LcdPrintxy(0, (1+i)*8, 0, "SLOT%d: ...", i+1);
		if (Lib_IccOpen((uchar)(1+i), 1, ucBuf))
		{
			Lib_LcdPrintxy(0, (1+i)*8, 0, "SLOT%d: RESET ERR", i+1);
			err[i] = 0x01;
			continue;
		}
#if 0
		memcpy(ApduSend.Command,"\x00\xa4\x04\x00",4);
		memcpy(ApduSend.DataIn,"1PAY.SYS.DDF01",14);
		ApduSend.Le=256;
		ApduSend.Lc=14;
		ucRet = Lib_IccCommand(i,&ApduSend,&ApduResp);
		if(ucRet)
		{
			Lib_LcdPrintxy(0,(2+i*2)*8/2,0," SLOT %d:COMMAND ERR1",i+1);
			DrawThreeLine();
			continue;
		}
		if(ApduResp.SWA!=0x90)
		{
			Lib_LcdPrintxy(0,(2+i*2)*8/2,0," SLOT %d:COMMAND ERR2",i+1);
			DrawThreeLine();
			continue;
		}
#endif
		Lib_LcdPrintxy(0, (1+i)*8, 0, "SLOT%d: PASSED", i+1);
	}

	Lib_DelayMs(200);
	if (mode != 0)
	{
		Lib_KbFlush();
	}

	j = 0;
	for (i=0; i<MaxSlotNum; i++)
	{
		if (err[i] == 0x01)
		{
			j++;
		}
	}
	if (j != 0)
	{
		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "Psam test failed~!");
		Lib_Beef(3, 500);
		iret = 1;	
	}
	else
	{
		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "Psam test ok~!");
		iret = 0;	
	}
	
#if 0
	while(1)
	{
		key = Lib_KbGetCh();
		if(KEYCANCEL == key)
		{
			return 0;
		}
		else if(KEYENTER == key)
		{
			goto AGAIN;
		}
	}
#else
	while (1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
	}
#endif
	return iret;
}
#endif

#if 1
int Test_PsamFunc(int mode)
{
	BYTE ucBuf[80], atrErr[6], comErr[6], swaErr[6], key;
	APDU_SEND ApduSend;
	APDU_RESP ApduResp;
	int i, j, iRet;

	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0*8, 0x80, "      PSAM TEST    ");

AGAIN:
	memset(atrErr, 0x00, sizeof(atrErr));
	memset(comErr, 0x00, sizeof(comErr));
	memset(swaErr, 0x00, sizeof(swaErr));
	memset(ucBuf, 0x00, sizeof(ucBuf));
	
	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);

	for(i=0; i<MaxSlotNum; i++)
	//for(i=1; i<MaxSlotNum; i++)
	{
		if (mode != 0)
		{
			Lib_KbFlush();
			if (Lib_KbCheck() == 0)
			{
				if (KEYCANCEL == Lib_KbGetCh())
				{
					return 1;
				}
			}
		}
		
		Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:...", i+1);
		Lib_IccClose(i+1);
		
		iRet = Lib_IccOpen((BYTE)(i+1), 1, ucBuf); 
		if (iRet) 
		{
			Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:RESET ERROR", i+1);
			Lib_Beef(3, 500);
			atrErr[i] = 0x01;
			continue;
		}
#if 0                
                //test
                trace_debug_printf("atr:");
                for(i=0;i<ucBuf[0];i++)
                  trace_debug_printf("%02x ",ucBuf[i+1]);
                Lib_KbGetCh();
#endif                
		memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
		memcpy(ApduSend.DataIn, "1PAY.SYS.DDF01", 14);
		ApduSend.Le=256;
		ApduSend.Lc=14;

		iRet = Lib_IccCommand(i+1, &ApduSend, &ApduResp);
		if (iRet)
		{
			Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:CMD ERROR", i+1);
			Lib_Beef(3, 500);
			comErr[i] = 0x01;
			continue;
		}
		if(ApduResp.SWA != 0x90)
		{
			Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:WARNING SW(%02X%02X)", i+1, ApduResp.SWA, ApduResp.SWB);
			Lib_Beef(3, 500);
			swaErr[i] = 0x01;
			continue;
		}
		
		Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:PASSED", i+1);
		Lib_IccClose(i+1);//yuhongyue
	}

	Lib_DelayMs(200);
	for(j=0; j<MaxSlotNum; j++)
	//for(j=1; j<MaxSlotNum; j++)
	{
		if ((atrErr[j] == 0x01) ||(comErr[j] == 0x01) || (swaErr[j] == 0x01))
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);	
			Lib_LcdPrintxy(0, 1*8, 0x00, "Psam test failed~!\n");
			Lib_Beef(3, 500);
			
			while(1)
			{
				if (mode == 0)
				{
					Lib_DelayMs(1000);
					return 1;
				}
				else
				{
					Lib_LcdPrintxy(0, 2*8, 0x00, "[Cancel]-Exit");
					Lib_LcdPrintxy(0, 3*8, 0x00, "[Enter]-Again");
				
					while(1)
					{
						key = Lib_KbGetCh();
						if (key == KEYENTER)
						{
							goto AGAIN;
						}
						else if (key == KEYCANCEL)
						{
							return 1;
						}
					}
				}
			}
		}
	}
	
	Lib_DelayMs(300);
	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);	
	Lib_LcdPrintxy(0, 1*8, 0x00, "Psam test ok~!");
	
	while (1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
	}
	
	return 0;
}
#endif

#define PICC_WAIT_TIME	5//second

/*************
// mode:0	auto 
// mode:1	manual
// burnIn:默认为0，为burn in专门设置；为其他值时，为burn in模式

************/
#define PCI_TEST		1

/***************************
TypeMode:'A', 'B', 'M'
****************************/
int CheckPicc(BYTE TypeMode, int mode, int burnIn)
{
	BYTE cardType[50], snbuf[100], tmps[100], key;
	int i, iRet;
	
	APDU_SEND ApduSend;
	APDU_RESP ApduResp;
	
	memset(snbuf, 0, sizeof(snbuf));
	memset(tmps, 0, sizeof(tmps));	
	
	Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
	Lib_SetTimer(0, PICC_WAIT_TIME*10);
	
	/*卡搜寻*/
	while(1)
	{
		if ((mode == 1) && (burnIn == 0))
		{
			if (Lib_KbCheck() == 0)
			{
				if (KEYCANCEL == Lib_KbGetCh())
				{
					return 2;
				}
			}			
		}
		
		iRet = Lib_PiccCheck(TypeMode, cardType, snbuf);
		if (iRet)
		{
			Lib_LcdPrintxy(0, 2*8, 0x00, "Waiting card %c...", TypeMode);
		}
		else
		{
			break;
		}
		
		/*定时器时间到*/
		if (Lib_CheckTimer(0) == 0)
		{
			Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 2*8, 0x00, "No card %c~!", TypeMode);
			Lib_Beef(3, 500);
			if (mode ==0)
			{
				if (burnIn == 0)
				{
					/*********
					* burnIn(1)模式自动循环，若测试员按下cancel键，
					* 不应该在此处响应，需在burnIn函数中响应以退出循环。
					**********/
					Lib_KbFlush();//清除之前的cancel键
					key = WaitforKey();
					if (key == KEYENTER)
					{
						return 1;
					}
					else if (key == KEYCANCEL)
					{
						Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);/*屏幕好像反应慢，先把屏幕清了*/
						return 2;
					}					
				}
				else
				{
					Lib_DelayMs(300);
					return 1;
				}
			}
			else
			{
				key = Lib_KbGetCh();
				if (key == KEYENTER)
				{
					Lib_SetTimer(0, PICC_WAIT_TIME*10);
					continue;
				}
				else if (key == KEYCANCEL)
				{
					return 2;
				}
			}
		}
	}
	
	if (TypeMode == 'A')
	{
		iRet = Lib_PiccReset();
		if (iRet)
		{
			Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 2*8, 0x00, "Reset error~!");
			Lib_Beef(3, 500);
			if (mode == 0)
			{
				return 1;
			}
			else
			{
				key = Lib_KbGetCh();
				if (KEYENTER == key)
				{
					return 2;
				}
				else if (KEYCANCEL == key)
				{
					return 1;
				}
			}
		}
		
		memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
		ApduSend.Lc = 0x0e;
		ApduSend.Le = 256;
		//memcpy(ApduSend.DataIn, "1PAY.SYS.DDF02", ApduSend.Lc/*14*/);
		memcpy(ApduSend.DataIn, "2PAY.SYS.DDF01", ApduSend.Lc/*14*/);
	}
	
	if (TypeMode == 'B')
	{
	#if 0//shengwei
		memcpy(ApduSend.Command, "\x00\xa4\x00\x00", 4);
		ApduSend.Lc = 2;
		ApduSend.Le = 256;
		memcpy(ApduSend.DataIn, "\x3f\x00", ApduSend.Lc);	
	#else
		memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
		ApduSend.Lc = 0x0e;
		ApduSend.Le = 256;
		//memcpy(ApduSend.DataIn, "1PAY.SYS.DDF02", ApduSend.Lc/*14*/);
		memcpy(ApduSend.DataIn, "2PAY.SYS.DDF01", ApduSend.Lc/*14*/);
	#endif
	}
	
	if (TypeMode == 'M')
	{
		;
	}
	
	/*向卡片发送APDU格式的数据，并接收响应*/
	iRet = Lib_PiccCommand(&ApduSend, &ApduResp);
	if (iRet == 0)
	{
		Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 2*8, 0x00, "-----%c Success-----", TypeMode);
		Lib_LcdGotoxy(0, 3*8);
		Lib_LcdSetFont(8, 16, 0);
		for (i=0; i<ApduResp.LenOut; i++)
		{
			Lib_Lcdprintf("%02x", ApduResp.DataOut[i]);
		}
		Lib_Lcdprintf("%02x", ApduResp.SWA);
		Lib_Lcdprintf("%02x", ApduResp.SWB);
	}
	else
	{
		Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 2*8, 0x00, "-----%c Failing-----", TypeMode);
		Lib_LcdPrintxy(0, 3*8, 0x00, "ERR:[%d]", iRet);
#if 1/*读错了，要能选择重测*/
		if (mode ==0)
		{
			if (burnIn == 0)
			{
				/*********
				* burnIn(1)模式自动循环，若测试员按下cancel键，
				* 不应该在此处响应，需在burnIn函数中响应以退出循环。
				**********/
				Lib_KbFlush();//清除之前的cancel键
				key = WaitforKey();
				if (key == KEYENTER)
				{
					return 1;
				}
				else if (key == KEYCANCEL)
				{
					Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);/*屏幕好像反应慢，先把屏幕清了*/
					return 2;
				}				
			}
			else
			{
				Lib_DelayMs(300);
				return 1;
			}
		}
/*		else
		{
			key = Lib_KbGetCh();
			if (key == KEYENTER)
			{
				Lib_SetTimer(0, PICC_WAIT_TIME*10);
				continue;
			}
			else if (key == KEYCANCEL)
			{
				return 2;
			}
		}*/
#endif
	}
	
	Lib_DelayMs(500);
	return iRet;
}

int Test_PiccAuto(int burnIn)
{
	//BYTE key;
	int iRet, iresult[2];

	//APDU_SEND ApduSend;
	//APDU_RESP ApduResp;

	while(1)
	{
		iresult[0] = iresult[1] = 0;
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "     PICC TEST      ");
	
		/*A卡测试*/
#if 0
		iRet = CheckPicc('A', 0, burnIn);
		if (iRet == 2)
		{
			continue;
		}
		else if (iRet == 0)
		{
			iresult[0] = 1;
			//continue;
		}
#else
		while(1)
		{
       		/*打开模块*/
       		iRet = Lib_PiccOpen();
       		if (iRet)
       		{
       			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
       			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
       			Lib_Beef(3, 500);
       			continue;
       		}
		
			iRet = CheckPicc('A', 0, burnIn);
			if (iRet == 2)/*非burnin模式auto模式用到*/
			{
				continue;
			}
			else if (iRet == 1)/*burnin模式*/
			{
				break;
			}
			else if (iRet == 0)
			{
				iresult[0] = 1;
				break;
				//continue;
			}			
		}
		Lib_PiccClose();
#endif

		/*B卡测试*/
#if 0
		iRet = CheckPicc('B', 0, burnIn);
		if (iRet == 2)
		{
			continue;
		}
		else if (iRet == 0)
		{
			iresult[1] = 1;
		}
#else
		while(1)
		{
       		/*打开模块*/
       		iRet = Lib_PiccOpen();
       		if (iRet)
       		{
       			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
       			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
       			Lib_Beef(3, 500);
       			continue;
       		}
			
			iRet = CheckPicc('B', 0, burnIn);
			if (iRet == 2)
			{
				continue;
			}
			else if (iRet == 1)/*burnin模式*/
			{
				break;
			}
			else if (iRet == 0)
			{
				iresult[1] = 1;
				break;
			}		
		}
#endif

		/*关闭PICC模块,统计结果*/
		Lib_PiccClose();
		if ((iresult[0] == 1) && (iresult[1] == 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}		
	}	
}

int Test_PiccFunc(int mode, int burnIn)
{
	BYTE snbuf[100], tmps[100], cardType[50], key;
	int iRet, i,iresult[2],iOKCnt,iErrCnt;

	APDU_SEND ApduSend;
	APDU_RESP ApduResp;
#ifdef PCI_TEST
	uchar rc; 
	uchar buf[1024]; 
	unsigned short nRet;
#endif

LOOP:	
	while(1)
	{
		memset(cardType, 0, sizeof(cardType));
		memset(snbuf, 0, sizeof(snbuf));
		memset(tmps, 0, sizeof(tmps));
		iresult[0] = iresult[1] = 1;
		
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "     PICC TEST     ");
		Lib_LcdPrintxy(0, 1*8, 0x00, "1-Type A");
		Lib_LcdPrintxy(12*6, 1*8, 0x00, "2-Type B");
#ifdef PCI_TEST
		if (mode == 1)
		{
			Lib_LcdPrintxy(0, 2*8, 0x00, "3-PCI test");
		}
#endif
		Lib_LcdPrintxy(12*6, 2*8, 0x00, "4-Debug");

#if 0
		//Lib_KbFlush();
		key = Lib_KbGetCh();
		if (key == KEYCANCEL)
		{
			Lib_PiccClose();
			return 1;
		}
#else
		if (mode ==0)
		{
			Lib_DelayMs(500);
			key = KEY1;
		}
		else//mode ==1
		{
			//Lib_KbFlush();
			key = Lib_KbGetCh();
			if (key == KEYCANCEL)
			{
				Lib_PiccClose();
				return 1;
			}			
		}
#endif

		iRet = Lib_PiccOpen();
		if (iRet)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
			#if 0
			Lib_KbFlush();
			Lib_KbGetCh();
			//goto LOOP;
			#else
			Lib_Beef(3, 500);
			if (mode == 0)
			{
				Lib_DelayMs(1000);
			}
			else
			{
				Lib_KbGetCh();
			}
			#endif
			continue;
		}
		
		/*Type A test*/
		if (key == KEY1)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_SetTimer(0, PICC_WAIT_TIME*10);

			while(1)
			{
				if ((mode == 1) && (burnIn == 0))
				{
					if (Lib_KbCheck() == 0)
					{
						if (KEYCANCEL == Lib_KbGetCh())
						{
							goto LOOP;
						}
					}					
				}

				//iRet = Lib_PiccCheck('A', 0x00, snbuf); //为什么我用0x00就出错?
				iRet = Lib_PiccCheck('A', cardType, snbuf);
				if (iRet)
				{
					//Lib_LcdPrintxy(0, 1*8, 0x00, "Waiting A card...");
					Lib_LcdPrintxy(0, 1*8, 0x00, "Waiting A card%d",iRet);
				}
				else
				{
					break;
				}

				if (Lib_CheckTimer(0) == 0)
				{
					Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
					Lib_LcdPrintxy(0, 1*8, 0x00, "No A card~!");
					Lib_Beef(3, 500);
					#if 0
					key = Lib_KbGetCh();
					if(key == KEYENTER)
					{
						Lib_SetTimer(0, PICC_WAIT_TIME*10);
						continue;
					}
					else if (KEYCANCEL == key)
					{
						goto LOOP;
					}
					#else
					if (mode == 0)
					{
						if (burnIn == 0)
						{
							Lib_KbFlush(); /*清除之前的cancel 键*/
							key = WaitforKey();
							if (key == KEYENTER)
							{
								goto TESTB; //return 1;
							}
							else if (key == KEYCANCEL)
							{
								goto LOOP;
							}
						}
						else
						{
							Lib_DelayMs(500);
							goto TESTB; //return 1;
						}

					}
					else
					{
						key = Lib_KbGetCh();
						if(key == KEYENTER)
						{
							Lib_SetTimer(0, PICC_WAIT_TIME*10);
							continue;
						}
						else if (KEYCANCEL == key)
						{
							goto LOOP;
						}
					}
					#endif
				}
			}

			iRet = Lib_PiccReset();
			if (iRet)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "Reset error~!");
				Lib_Beef(3, 500);
				#if 0
				key = Lib_KbGetCh();
				if(key == KEYENTER)
				{
					continue; //the first while(1)
				}
				else if (KEYCANCEL == key)
				{
					return 1;
				}
				#else
				if (mode == 0)
				{
					goto TESTB; //return 1;
				}
				else
				{
					key = Lib_KbGetCh();
					if (KEYENTER == key)
					{
						continue;
					}
					else if (KEYCANCEL == key)
					{
						goto TESTB; //return 1;
					}
				}
				#endif
			}

			memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
			memcpy(ApduSend.DataIn, "1PAY.SYS.DDF02", 14);
			ApduSend.Lc = 0x0e;
			ApduSend.Le = 256;
			
			iresult[0] = Lib_PiccCommand(&ApduSend, &ApduResp);
			if (iresult[0] == 0)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "-----A Success-----");
				Lib_LcdGotoxy(0, 2*8);
				Lib_LcdSetFont(8, 16, 0);
				for (i=0; i<ApduResp.LenOut; i++)
				{
					Lib_Lcdprintf("%02x", ApduResp.DataOut[i]);
				}
				Lib_Lcdprintf("%02x", ApduResp.SWA);
				Lib_Lcdprintf("%02x", ApduResp.SWB);
			}
			else
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "-----A Failing-----");
				Lib_LcdPrintxy(0, 2*8, 0x00, "ERR:[%d]", iresult[0]);
			}
#if 0
			#if 0 
			Lib_KbGetCh();
			Lib_PiccClose();
			#else
			Lib_PiccClose();
			if (mode == 0)
			{
				Lib_DelayMs(1000);
				key = KEYENTER;
			}
			else
			{
				key = Lib_KbGetCh();
			}

			if ((key == KEYCANCEL) || (key == KEYENTER))
			{
				break;
			}
			#endif
#else
			//Lib_PiccClose();
			if (mode == 0)
			{
				Lib_DelayMs(1000);
				goto TESTB; 
				//key = KEYENTER;
			}
			else
			{
				Lib_PiccClose();
				key == Lib_KbGetCh();
			}
			
                   #if 0
			if ((key == KEYCANCEL) || (key == KEYENTER))
			{
				continue; //goto TESTB; //break;				
			}
			#else
			
			continue;
			
			#endif
#endif
		}
		
		/*Type B test*/
		if (key == KEY2)
		{
TESTB:
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_SetTimer(1, PICC_WAIT_TIME*10);

			while(1)
			{
				if ((mode == 1) && (burnIn == 0))
				{
					if (Lib_KbCheck() == 0)
					{
						if (KEYCANCEL == Lib_KbGetCh())
						{
							goto LOOP;
						}
					}					
				}

				//iRet = Lib_PiccCheck('A', 0x00, snbuf); //为什么我用0x00就出错?
				iRet = Lib_PiccCheck('B', cardType, snbuf);
				if (iRet)
				{
					Lib_LcdPrintxy(0, 1*8, 0x00, "Waiting B card...");
				}
				else
				{
					break;
				}

				if (Lib_CheckTimer(1) == 0)
				{
					Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
					Lib_LcdPrintxy(0, 1*8, 0x00, "No B card~!");
					Lib_Beef(3, 500);
					#if 0
					key = Lib_KbGetCh();
					if(key == KEYENTER)
					{
						Lib_SetTimer(0, PICC_WAIT_TIME*10);
						continue;
					}
					else if (KEYCANCEL == key)
					{
						goto LOOP;
					}
					#else
					if (mode == 0)
					{
						if (burnIn == 0)
						{
							Lib_KbFlush(); /*清除之前的cancel 键*/
							key = WaitforKey();
							if (key == KEYENTER)
							{
								return 1;
							}
							else if (key == KEYCANCEL)
							{
								goto LOOP;
							}
						}
						else
						{
							Lib_DelayMs(500);
							return 1;
						}

					}
					else
					{
						key = Lib_KbGetCh();
						if(key == KEYENTER)
						{
							Lib_SetTimer(1, PICC_WAIT_TIME*10);
							continue;
						}
						else if (KEYCANCEL == key)
						{
							goto LOOP;
						}
					}
					#endif
				}
			}
			
              #if 0  /*B卡不需要复位操作*/
			iRet = Lib_PiccReset();
			if (iRet)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "Reset error~!");
				Lib_Beef(3, 500);
				#if 0
				key = Lib_KbGetCh();
				if(key == KEYENTER)
				{
					continue; //the first while(1)
				}
				else if (KEYCANCEL == key)
				{
					return 1;
				}
				#else
				if (mode == 0)
				{
					return 1;
				}
				else
				{
					key = Lib_KbGetCh();
					if (KEYENTER == key)
					{
						continue;
					}
					else if (KEYCANCEL == key)
					{
						return 1;
					}
				}
				#endif
			}
		#endif
		
#if 0
			memcpy(ApduSend.Command, "\x00\xa4\x00\x00", 4);
			ApduSend.Lc = 2;
			ApduSend.Le = 256;
			memcpy(ApduSend.DataIn, "\x3f\x00", ApduSend.Lc);	
#else
			memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
			ApduSend.Lc = 0x0e;
			ApduSend.Le = 256;
			memcpy(ApduSend.DataIn, "1PAY.SYS.DDF02", ApduSend.Lc/*14*/);
#endif
			
			iresult[1] = Lib_PiccCommand(&ApduSend, &ApduResp);
			if (iresult[1] == 0)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "-----B Success-----");
				Lib_LcdGotoxy(0, 2*8);
				Lib_LcdSetFont(8, 16, 0);
				for (i=0; i<ApduResp.LenOut; i++)
				{
					Lib_Lcdprintf("%02x", ApduResp.DataOut[i]);
				}
				Lib_Lcdprintf("%02x", ApduResp.SWA);
				Lib_Lcdprintf("%02x", ApduResp.SWB);
			}
			else
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "-----B Failing-----");
				Lib_LcdPrintxy(0, 2*8, 0x00, "ERR:[%d]", iresult[1]);
			}

			#if 0 
			Lib_KbGetCh();
			Lib_PiccClose();
			#else
			Lib_PiccClose();
			if (mode == 0)
			{
				Lib_DelayMs(1000);
				key = KEYENTER;
			}
			else
			{
				key = Lib_KbGetCh();
			}

			if ((key == KEYCANCEL) || (key == KEYENTER))
			{
				break;
			}
			#endif
		}
		
#ifdef PCI_TEST
		if (key == KEY3)
		{
		      iOKCnt = 0;
			iErrCnt = 0;
		      while(1)
		      	{
       			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
//#if 0                        
       			nRet = Rs_Inquiry(&rc,buf);
//#endif                        
       			if (0 != nRet)
       			{
       			      iErrCnt ++;
       				Lib_LcdCls();
       				Lib_LcdPrintxy(0, 0, 0x00, "nRet=%d [0x%x][0x%x].", nRet, rc, *(int *)buf);		
       			}
				else
				{
                                iOKCnt ++;
				}
				
                          Lib_LcdPrintxy(0, 16, 0x00, "OK=%d, Err=%d.", iOKCnt, iErrCnt);
				if(0 != WaitEsc(1))
				{
                               break;
				}
			 }
		}
#endif

		if (key == KEY5)//for debug
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);

			while(1)
			{

				iRet = Lib_PiccOpen();
				if (iRet)
				{
					Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
					Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
					Lib_KbGetCh();
					continue;
				}
		
				if (Lib_KbCheck() == 0)
				{
					if (KEYCANCEL == Lib_KbGetCh())
					{
						goto LOOP;
					}
				}	

				memset(cardType, 0, sizeof(cardType));
				memset(snbuf, 0, sizeof(snbuf));
				
				iRet = Lib_PiccCheck(0, cardType, snbuf);
				if (iRet)
				{
					Lib_Beep();
					Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
					Lib_LcdPrintxy(0, 1*8, 0x00, "Waiting card...");
				}
				else
				{
					iRet = Lib_PiccReset();
					if (iRet)
					{
						Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
						Lib_LcdPrintxy(0, 1*8, 0x00, "Reset error~!");
						Lib_KbGetCh();
						continue;
					}

					memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
					memcpy(ApduSend.DataIn, "1PAY.SYS.DDF02", 14);
					ApduSend.Lc = 0x0e;
					ApduSend.Le = 256;
					
					iRet = Lib_PiccCommand(&ApduSend, &ApduResp);
					if (iRet == 0)
					{
						Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
						Lib_LcdPrintxy(0, 1*8, 0x00, "-----Success-----");
						Lib_LcdGotoxy(0, 2*8);
						Lib_LcdSetFont(8, 16, 0);
						for (i=0; i<ApduResp.LenOut; i++)
						{
							Lib_Lcdprintf("%02x", ApduResp.DataOut[i]);
						}
						Lib_Lcdprintf("%02x", ApduResp.SWA);
						Lib_Lcdprintf("%02x", ApduResp.SWB);

						Lib_PiccClose();
						Lib_DelayMs(500);
						continue;
					}
					else
					{
						Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
						Lib_LcdPrintxy(0, 1*8, 0x00, "-----Failing-----");
						Lib_LcdPrintxy(0, 2*8, 0x00, "ERR:[%d]", iRet);
						
						Lib_KbGetCh();
						continue;
					}
				}
			}
		}//end of KEY4		
	}
	
	//return iRet;
	if ((iresult[0] == 0) && (iresult[1] == 0))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int Test_PiccCheckBalance(int mode)
{
	uchar rc; 
	uchar buf[1024]; 
	unsigned short nRet;
	BYTE key;

	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
       
	nRet = Rs_Inquiry(&rc,buf);/*此函数需要按键才能退出*/
        
	if (0 != nRet)
	{
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0, 0x00, "nRet=%d\n[0x%x][0x%x].", nRet, rc, *(int *)buf);		
	}

	while(1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}		
	}

	return nRet;
}

int Test_PiccM()
{
	
	return 0;
}

int Test_PiccB(int mode, int burnIn)
{
	BYTE key;
	int i, j, iRet, TestB[3]; 
	int result;
	
	char TestItem[][5] = {"Max", "Mid", "Min"};

LOOP:	
	while(1)
	{
		j = 0;
		for (i=0; i<3; i++)
		{
			TestB[i] = 0;
		}
		
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "    TYPE B TEST     ");

		if (mode != 0)
		{
			key = Lib_KbGetCh();
			if (KEYCANCEL == key)
			{
				Lib_PiccClose();
				return 1;
			}
		}
		/*打开PICC模块*/
		iRet = Lib_PiccOpen();
		if (iRet)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
			Lib_Beef(3, 500);
			if (mode == 0)
			{
				Lib_DelayMs(1000);
			}
			else
			{
				Lib_KbGetCh();
			}
			continue;
		}
		
		/*B 卡的循环测试*/
		for (i=0; i<3; i++)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "%s:", TestItem[i]);
			
			iRet = CheckPicc('B', mode, burnIn);
			Lib_DelayMs(1000);
			if (iRet == 2)
			{
				goto LOOP;
			}
			else if (iRet == 0)
			{
				TestB[i] = 1;
				continue;
			}
			else
			{
				break;
			}
		}
		
		/*关闭PICC模块,统计结果*/
		Lib_PiccClose();
		
		for (i=0; i<3; i++)
		{
			if (TestB[i] == 1)
			{
				j++;
			}
		}
		
		if (j != 3)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type B error~!");
			Lib_Beef(3, 500);
			result = 1;
			//Lib_KbGetCh();
			//return 1;
		}
		else
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type B ok~!");
			result = 0;
			//Lib_KbGetCh();
			//break;
		}

		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
		
	}
	
	return result;
}

int Test_PiccA(int mode, int burnIn)
{
	BYTE key;
	int i, j, iRet, TestA[3]; 
	int result;
	
	char TestItem[][5] = {"Max", "Mid", "Min"};

LOOP:	
	while(1)
	{
		j = 0;
		for (i=0; i<3; i++)
		{
			TestA[i] = 0;
		}
		
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "    TYPE A TEST     ");

		if (mode != 0)
		{
			key = Lib_KbGetCh();
			if (KEYCANCEL == key)
			{
				Lib_PiccClose();
				return 1;
			}
		}
	#if 1
		/*打开PICC模块*/
		iRet = Lib_PiccOpen();
		if (iRet)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
			Lib_Beef(3, 500);
			if (mode == 0)
			{
				Lib_DelayMs(1000);
			}
			else
			{
				Lib_KbGetCh();
			}
			continue;
		}
		
		/*A 卡的循环测试*/
		for (i=0; i<3; i++)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "%s:", TestItem[i]);
			
			iRet = CheckPicc('A', mode, burnIn);
			Lib_DelayMs(1000);
			if (iRet == 2)
			{
				goto LOOP;
			}
			else if (iRet == 0)
			{
				TestA[i] = 1;
				continue;
			}
			else
			{
				break;
			}
		}
		
		/*关闭PICC模块,统计结果*/
		Lib_PiccClose();
	#else
		for (i=0; i<3; i++)
		{
			/*打开PICC模块*/
			iRet = Lib_PiccOpen();
			if (iRet)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
				Lib_Beef(3, 500);
				if (mode == 0)
				{
					Lib_DelayMs(1000);
				}
				else
				{
					Lib_KbGetCh();
				}
				continue;
			}
			
			/*A 卡的循环测试*/
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "%s:", TestItem[i]);
			
			iRet = CheckPicc('A', mode, burnIn);
			Lib_DelayMs(1000);
			if (iRet == 2)
			{
				goto LOOP;
			}
			else if (iRet == 0)
			{
				TestA[i] = 1;
				continue;
			}
			else
			{
				break;
			}

			/*关闭PICC模块,统计结果*/
			Lib_PiccClose();
		}
	#endif
		
		for (i=0; i<3; i++)
		{
			if (TestA[i] == 1)
			{
				j++;
			}
		}
		
		if (j != 3)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type A error~!");
			Lib_Beef(3, 500);
			result = 1;
			//Lib_KbGetCh();
			//return 1;
		}
		else
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type A ok~!");
			result = 0;
			//Lib_KbGetCh();
			//break;
		}

		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}
		
	}
	
	return result;
}

int Test_Picc(BYTE TypeMode, int mode, int burnIn)
{
	BYTE key;
	int i, j, iRet, Test[3]; 
	int result;
	
	char TestItem[][5] = {"Max", "Mid", "Min"};

LOOP:	
	while(1)
	{
		j = 0;
		for (i=0; i<3; i++)
		{
			Test[i] = 0;
		}
		
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "    TYPE %c TEST    ", TypeMode);

		if (mode != 0)
		{
			key = Lib_KbGetCh();
			if (KEYCANCEL == key)
			{
				Lib_PiccClose();
				return 1;
			}
		}
	#if 0
		/*打开PICC模块*/
		iRet = Lib_PiccOpen();
		if (iRet)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
			Lib_Beef(3, 500);
			if (mode == 0)
			{
				Lib_DelayMs(1000);
			}
			else
			{
				Lib_KbGetCh();
			}
			continue;
		}
		
		/*A 卡的循环测试*/
		for (i=0; i<3; i++)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "%s:", TestItem[i]);
			
			iRet = CheckPicc(TypeMode, mode, burnIn);
			Lib_DelayMs(1000);
			if (iRet == 2)
			{
				goto LOOP;/*这个地方是跑到最上面测试，还是在for内呢?*/
			}
			else if (iRet == 0)
			{
				Test[i] = 1;
				continue;
			}
			else
			{
				break;
			}
		}
		
		/*关闭PICC模块,统计结果*/
		Lib_PiccClose();
	#else
		/*A 卡的循环测试*/
		for (i=0; i<3; i++)
		{
			/*打开PICC模块*/
			iRet = Lib_PiccOpen();
			if (iRet)
			{
				Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0, 1*8, 0x00, "Open picc failed~!");
				Lib_Beef(3, 500);
				if (mode == 0)
				{
					Lib_DelayMs(1000);
				}
				else
				{
					Lib_KbGetCh();
				}
				continue;
			}

			/*A卡发送命令*/
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "%s:", TestItem[i]);
                        
			iRet = CheckPicc(TypeMode, mode, burnIn);
			Lib_DelayMs(1000);
			if (iRet == 2)
			{
				goto LOOP;/*这个地方是跑到最上面测试，还是在for内呢?*/
			}
			else if (iRet == 0)
			{
				Test[i] = 1;
				continue;
			}
			else
			{
				break;
			}
			
            /*关闭PICC模块,统计结果*/
            //Lib_PiccClose();			
		}
	#endif
	
		for (i=0; i<3; i++)
		{
			if (Test[i] == 1)
			{
				j++;
			}
		}
		
		if (j != 3)
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type %c error~!", TypeMode);
			Lib_Beef(3, 500);
			result = 1;
		}
		else
		{
			Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 1*8, 0x00, "Test type %c ok~!", TypeMode);
			result = 0;
		}

		if (mode == 0)
		{
			Lib_DelayMs(1000);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if ((key == KEYCANCEL) || (key == KEYENTER))
		{
			break;
		}		
	}
	
	return result;	
}

void Test_PiccABMFunc(void)
{
	BYTE key;
	while(1)
	{
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0*8, 0x80, "     PICC TEST     ");
		Lib_LcdPrintxy(5*6, 1*8, 0x00, "1-Type A");
		Lib_LcdPrintxy(5*6, 2*8, 0x00, "2-Type B");
		Lib_LcdPrintxy(5*6, 3*8, 0x00, "3-Type M");
		
		while(1)
		{
			key = Lib_KbGetCh();
			
			if (KEYCANCEL == key)
			{
				return ;
			}
			
			if ((key < KEY1) || (key > KEY3))
			{
				continue;
			}
			
			if (KEY1 == key)
			{
				//Test_PiccA(1, 0);
				Test_Picc('A', 1, 0);
				break;
			}
			if (KEY2 == key)
			{
				//Test_PiccB(1, 0);
				Test_Picc('B', 1, 0);
				break;
			}
			if(KEY3 == key)
			{
				//Test_PiccM();
				break;
			}
		}
	}	
}

int Test_LedFunc(int mode)
{
	int i, j;
	BYTE key;
	
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "      LED TEST      ");
	Lib_LcdPrintxy(1*6, 1*8, 0x00, "Led test ...");

	Lib_LedInit();
	Lib_LedOff(LCD_LED1|LCD_LED2|LCD_LED3|LCD_LED4);
	Lib_DelayMs(200);

	for (j=0; j<2; j++)
		for(i=0; i<4; i++)
		{
			Lib_LedOn((unsigned short)1<<i);
			Lib_DelayMs(200);
			Lib_LedOff((unsigned short)1<<i);
			Lib_DelayMs(200);
		}

	for (i=0; i<2; i++)
	{
		Lib_LedOn(LCD_LED1|LCD_LED2|LCD_LED3|LCD_LED4);
		Lib_DelayMs(200);
		Lib_LedOff(LCD_LED1|LCD_LED2|LCD_LED3|LCD_LED4);
		Lib_DelayMs(200);
	}

	Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(0, 1*8, 0x00, "Led test finish~!");

	while (1)
	{
		if (mode == 0)
		{
			Lib_DelayMs(500);
			key = KEYENTER;
		}
		else
		{
			key = Lib_KbGetCh();
		}

		if (key == KEYCANCEL || key == KEYENTER)
		{
			break;
		}
	}
	/*退出前清屏*/
	Lib_LcdCls();
	return 0;
}

extern int s_VoiceInit(void);

int  Test_VoiceFunc(void)
{
    uchar pageNo=0, key;

	while(1)
	{
        trace_debug_printf("------KbGetCh[%02x]",key);
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0, 0x80, "     VOICE TEST    ");
		//Lib_KbFlush();
		Lib_LcdClrLine(2*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0, 1*8, 0x00, "1-S1  2-S2  3-S3");

		Lib_KbFlush();
		key = Lib_KbGetCh();
		switch(key)
		{
			case KEYCANCEL:
				return 0;

			case KEYF1:
			case KEYF2:
			case KEYENTER:
				if (pageNo == 0) 
					pageNo = 1;
				else
					pageNo = 0;
				break;
			case KEY1:
				if(pageNo == 0)
					SetVoice(VOICE_PLS_INPUT_PWD);
				else 
					SetVoice(VOICE_TRADE_FAILER);
				break;
			case KEY2:
				if(pageNo == 0)
					SetVoice(VOICE_PLS_INPUT_PWD_AGAIN);
				else 
					SetVoice(VOICE_WELCOME);
				break;
			case KEY3:
				if(pageNo == 0)
					SetVoice(VOICE_PWD_ERR);
				else 
					SetVoice(VOICE_THANKS_PATRONAGE);
				break;
			case KEY4:
				if(pageNo == 0)
					SetVoice(VOICE_PLS_SWIPE_MAGCARD);
				else 
					SetVoice(VOICE_THANKS_USE);
				break;

			case KEY5:
				if(pageNo == 0)
					SetVoice(VOICE_PLS_INSERT_ICCARD);
				else 
					SetVoice(VOICE_COMM_FAILER);
				break;
			case KEY6:
				if(pageNo == 0)
					SetVoice(VOICE_TRADE_SUCCESS);
				else
					SetVoice(VOICE_BALANCE_LACK);
				break;
                                
            case KEY7:
              SetVoice(VOICE_TRADE_FAILER);
              break;

            case KEY8:
              SetVoice(VOICE_WELCOME);
              break;

            case KEY9:  
              SetVoice(VOICE_THANKS_PATRONAGE);
              break;

            default:
				break;
		}          
	} 
	//return 0;      
}

int  Test_VoiceFunc_Auto(int burnIn)
{	 
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 0, 0x80, "    VOICE TEST     ");

	Lib_LcdPrintxy(0, 1*8, 0, "S1 voice test ...");    
	SetVoice(VOICE_PLS_INPUT_PWD);    
	Lib_DelayMs(2000);

	Lib_LcdPrintxy(0, 2*8, 0, "S2 voice test ...");    	
	SetVoice(VOICE_PLS_INPUT_PWD_AGAIN);
	Lib_DelayMs(3000);

	Lib_LcdPrintxy(0, 3*8, 0, "S3 voice test ...");    		
	SetVoice(VOICE_PWD_ERR);
	
	Lib_DelayMs(1000);
	if (burnIn == 0)
	{
		Lib_KbFlush();/*防止在显示结果前退出*/
	}
	//Lib_Beep();
	
	return 0;
}

int Test_BeepOnce()
{
	int i;

	//Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
	Lib_LcdClrLine(1*8, 3*8);
	Lib_LcdPrintxy(0, 1*8, 0x00, "Beep test...");

	i = 0;
	
	Lib_DelayMs(250);
	while(i < 7)
	{
		Lib_Beef(i%7, 200);
		i++;
		Lib_DelayMs(300);
	}
	Lib_DelayMs(250);
	
	Lib_LcdCls();
	return 0;
}

int Test_BeepLoop()
{
	BYTE key;
	int num;

	num = 0;
	
	while(1)
	{
		if (0 == Lib_KbCheck())
		{
			key = Lib_KbGetCh();
			if (KEYCANCEL == key)
			{
				break;
			}
		}
		Lib_LcdClrLine(3*8, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0*6, 3*8, 0x00, "Loop: %d ", num+1);
		Test_BeepOnce();
		num++;
	}
	
	return 0;

}

void Test_BeepFunc(void)
{
	BYTE key;

	while(1)
	{
		Lib_LcdCls();
		Lib_KbFlush();
		Lib_LcdPrintxy(0, 0, 0x80, "     BEEP TEST      ");
		Lib_LcdPrintxy(1*6, 2*8, 0x00, "1-Once 2-Loop");
		
		while(1)
		{
			key = Lib_KbGetCh();

			if (KEYCANCEL == key)
			{
				return;
			}

			if ((key < KEY1) || (key > KEY2))
			{
				continue;
			}

			if (KEY1 == key)
			{
				Test_BeepOnce();
				break;
			}
			else if (KEY2 == key)
			{
				Test_BeepLoop();
				break;
			}
		}
	}
}


void Test_Usb(void)
{
    unsigned char i = 0, j = 0, key = 0;
    unsigned char sBuf1[MAX_PACK_SIZE] = {0};
    unsigned char sBuf0[MAX_PACK_SIZE] = {0};
    //unsigned char ucOld     = 0;

    while (1)
    {
        Lib_LcdCls();
        Lib_LcdPrintxy(0, 0, 0x80,   "     USB HID TEST      ");
        Lib_LcdPrintxy(0, 1*8, 0x00,   "1 sendself loop");
        Lib_LcdPrintxy(0, 2*8, 0x00,   "2 press key to send");

        key = Lib_KbGetCh();
        if (KEYCANCEL == key)
        {
            return;
        }
        else if (KEY1 == key)
        {
            Lib_LcdCls();
            Lib_LcdPrintxy(0, 0, 0x80,   "     Sending...    ");
            while (1)
            {
                Hid_Send(sBuf0, MAX_PACK_SIZE);
                ///trace_debug_printf("bDeviceState = %d\r\n", Hid_GetStatus());
                if (!Lib_KbCheck())
                {
                    if (KEYCANCEL == Lib_KbGetCh())
                        break;
                }
                sBuf0[0]++;
                if (Hid_Rec(sBuf1) > 0)
                {
                    trace_debug_printf("REC:\r\n");
                    for (i = 0; i < MAX_PACK_SIZE / 16; i++)
                    {
                        for (j = 0; j < 16; j++)
                            trace_debug_printf("%02x ", sBuf1[i * 16 + j]);

                        trace_debug_printf("\r\n");
                    }
                    if (MAX_PACK_SIZE % 16)
                    {
                        for (j = 0; j < MAX_PACK_SIZE % 16; j++)
                            trace_debug_printf("%02x ", sBuf1[i * 16 + j]);
                        trace_debug_printf("\r\n");
                    }
                }
            }
        }
        else if (KEY2 == key)
        {
            Lib_LcdCls();
            Lib_LcdPrintxy(0, 1*8, 0x00, "press any key to send...");
            while (1)
            {
                if (0 == Lib_KbCheck())
                {
                    for (i = 0; i < MAX_PACK_SIZE; i++)
                        sBuf0[i] = i + j;

                    key = Lib_KbGetCh();
                    if (key == KEYCANCEL)
                        break;

                    Hid_Send(sBuf0, MAX_PACK_SIZE);
                    trace_debug_printf("KB = 0x%02X\r\n", key);
                    j++;
                }

                if (Hid_Rec(sBuf1) > 0)
                {
                    trace_debug_printf("REC:\r\n");
                    for (i = 0; i < MAX_PACK_SIZE / 16; i++)
                    {
                        for (j = 0; j < 16; j++)
                            trace_debug_printf("%02x ", sBuf1[i * 16 + j]);

                        trace_debug_printf("\r\n");
                    }
                    if (MAX_PACK_SIZE % 16)
                    {
                        for (j = 0; j < MAX_PACK_SIZE % 16; j++)
                            trace_debug_printf("%02x ", sBuf1[i * 16 + j]);
                        trace_debug_printf("\r\n");
                    }
                }
            }
        }
    }
}

/******************************************************************************* 
 * 函数名称: Test_Tp(*)
 * 功能描述: 触屏测试
 * 作    者: WLB
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 其它说明: 无
 * 修改历史: 
 *           1. 2016-4-7  WLB  Created
 *******************************************************************************/
void Test_Tp(void)
{
    BYTE key;

    //while (1)
    {
        while(1)
    	{
            Lib_LcdCls();
            Lib_KbFlush();
            Lib_LcdPrintxy(0, 0, 0x80, "         TP TEST       ");
            Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
            Lib_LcdPrintxy(0, 1*8, 0x00, "1-calibreation");
            Lib_LcdPrintxy(0, 2*8, 0x00, "2-Signature   ");

            key = Lib_KbGetCh();
    		if (KEYCANCEL == key)
    		{
    			return ;
    		}

            switch (key)
    	    {
                case KEY1:
                    Lib_PadOpen();
                    Lib_Padcalibration();
                    break;

                case KEY2:
                    Lib_PadOpen();
                    Lib_PadSign("12345678", 30);
                    break;

                default:
                    break;
            }
        }
    }
}

void Test_AllSingleFunc(void)
{
	BYTE key;
    char page = 0;
    char MenuBuf[][25] =        // 显示的菜单必须为3的整数倍,长度要保持一致，3行为一页
    {
        {"1-Lcd  2-Kb   3-Clk "},
        {"4-Led  5-Beep 6-Psam"},
        {"7-Picc 8-usb 9-Voice"},
        {"1-TP   2-gm         "},
        {"                    "},
        {"                    "},
    };
        

	//while(1)
	{
		while(1)
		{
            Lib_LcdCls();
    		Lib_KbFlush();

    		Lib_LcdPrintxy(0, 0, 0x80, "     SINGLE TEST       ");
    		Lib_LcdClrLine(1*8, LCD_HIGH_MINI-1);
            Lib_LcdPrintxy(0, 1*8, 0x00, (char *)&MenuBuf[page * 3]);
    		Lib_LcdPrintxy(0, 2*8, 0x00, (char *)&MenuBuf[page * 3 + 1]);
    		Lib_LcdPrintxy(0, 3*8, 0x00, (char *)&MenuBuf[page * 3 + 2]);

			key = Lib_KbGetCh();

			if (KEYCANCEL == key)
			{
				return ;
			}

			switch (key)
			{
				case KEY1:
                    if (0 == page)
					    Test_LcdFunc();
                    else
                        Test_Tp();
					break;

                case KEY2:
                    if (0 == page)
					    Test_KbFunc(1);
                    else
                        gm_test();
					break;

                case KEY3:
					Test_ClkFunc();
					break;

                case KEY4:
					Test_LedFunc(1);
					break;

                case KEY5:
					Test_BeepFunc();
					break;

                case KEY6:
					Test_PsamFunc(1);
					break;

                case KEY7:
					Test_PiccFunc(1, 0);
					break;

                case KEY8:
					Test_Usb();
					break;

                case KEY9:
                    Test_VoiceFunc();
                    break;

                case KEYENTER:
                    page++;
                    page %= sizeof(MenuBuf) / sizeof(MenuBuf[0]) / 3;
                    break;

				default:
					break;
			}
		}
	}
}

#define FACTORY_NUM	8
int Test_Machine(void)
{
	int i, j;
	char Test[FACTORY_NUM], buf[6], errBuf[25];

	Lib_LcdCls();
	Lib_KbFlush();
	Lib_LcdPrintxy(0, 0, 0x80, "     FACTORY TEST    ");
	
	while(1)
	{
		j = 0;
		memset(errBuf, 0, sizeof(errBuf));
		
		for (i=0; i<FACTORY_NUM; i++)
		{
			Test[i] = 0;
		}
		
		if (Test_LcdOnce(0, 0) == 0)
		{
			Test[TEST_LCD] = 1;
		}
		if (KEYCANCEL == WaitforKey())
		{
			continue;
		}

		if (Test_KbFunc(1) == 0)
		{
			Test[TEST_KB] = 1;
		}
		
		if (Test_LedFunc(0) == 0)
		{
			Test[TEST_LED] = 1;
		}
		
		if (Test_BeepOnce() == 0)
		{
			Test[TEST_BEEP] = 1;
		}

		if (Test_Clk(0) == 0)
		{
			Test[TEST_CLK] = 1;
		}
		
		if (Test_PsamFunc(0) == 0)
		{
			Test[TEST_PSAM] = 1;
		}
       #if 0
	 Test[TEST_PICC] = 1;
	 #else
		//if (Test_PiccFunc(0, 0) == 0)/*不需要手动选择*/
		//if (Test_PiccA(0, 0) == 0)
		if (Test_Picc('A', 0, 0) == 0)
		{
			Test[TEST_PICC] = 1;
		}
               
		if (Test_Picc('B', 0, 0) == 1)
		{
			Test[TEST_PICC] = 0;
		}
                
		#if 0
		if (Test_PiccCheckBalance(0) != 0)
		{
			Test[TEST_PICC] = 0;
		}
		#endif
	#endif	
		if (Test_VoiceFunc_Auto(0) == 0)
		{
			Test[TEST_VOICE] = 1;
		}

		for (i =0; i<FACTORY_NUM; i++)
		{
			if (Test[i] == 1)
			{
				j++;
			}
			else
			{
				memset(buf, 0, sizeof(buf));
				GetItem(i, buf);
				strcat(errBuf, buf);
			}
		}
		
		if (j < FACTORY_NUM)
		{
			Lib_LcdCls();
			Lib_LcdPrintxy(0, 0, 0x80, "     FACTORY TEST    ");
			Lib_LcdPrintxy(0*6, 1*8, 0x00, "Total:8 Error:%d", (FACTORY_NUM - j));
			Lib_LcdPrintxy(0, 2*8, 0x00, "Error:%s", errBuf);
			Lib_KbGetCh();
			return 1;
		}
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0, 0x80, "     FACTORY TEST    ");
		Lib_LcdPrintxy(1*6, 1*8, 0x00, "Board test finished~!");
		Lib_KbGetCh();
		break;
	}
	
	return 0;
}

int Test_BurnIn()
{
	Lib_LcdCls();
	Lib_KbFlush();
	Lib_LcdPrintxy(0, 0, 0x80, "    BURN-IN TEST    ");
	
	while(1)
	{
		if (Lib_KbCheck() == 0)
		{
			if (KEYCANCEL == Lib_KbGetCh())
			{
				return 0;
			}
		}
		
		Test_LcdOnce(0, 1);
		Test_Clk(0);
		Test_LedFunc(0);
		Test_BeepOnce();
		Test_PsamFunc(0);
		//Test_PiccFunc(0, 1);
          #if 1
		Test_PiccAuto(1);
	    #endif	
		Test_VoiceFunc_Auto(1);
	}
}

#define BOARD_NUM		7

int Test_Board()
{
	//BYTE key, ret;
	int i, j;
	char Test[BOARD_NUM], buf[6], errBuf[25];

	Lib_LcdCls();
	Lib_KbFlush();
	Lib_LcdPrintxy(0, 0, 0x80, "     BOARD TEST     ");
	
	while(1)
	{
		j = 0;
		memset(errBuf, 0, sizeof(errBuf));
		
		for (i=0; i<BOARD_NUM; i++)
		{
			Test[i] = 0;
		}
		
		if (Test_LcdOnce(0, 0) == 0)
		{
			Test[TEST_LCD] = 1;
		}
		if (KEYCANCEL == WaitforKey())
		{
			continue;
		}

		if (Test_KbFunc(1) == 0)
		{
			Test[TEST_KB] = 1;
		}
		
		if (Test_LedFunc(0) == 0)
		{
			Test[TEST_LED] = 1;
		}
		
		if (Test_BeepOnce() == 0)
		{
			Test[TEST_BEEP] = 1;
		}
		
		if (Test_Clk(0) == 0)
		{
			Test[TEST_CLK] = 1;
		}		
       #if 1
		//if (Test_PiccFunc(0, 0) == 0)
		if (Test_PiccAuto(0) == 0)
		{
			Test[TEST_PICC] = 1;
		}
       #else
	   Test[TEST_PICC] = 1;
	 #endif
	 
		if (Test_VoiceFunc_Auto(0) == 0)
		{
			Test[TEST_VOICE] = 1;
		}

		//Lib_KbFlush();/*防止在显示结果前退出*/
		
		for (i=0; i<BOARD_NUM; i++)
		{
			if (Test[i] == 1)
			{
				j++;
			}
			else
			{
				memset(buf, 0, sizeof(buf));
				GetItem(i, buf);
				strcat(errBuf, buf);
			}
		}

		if (j < BOARD_NUM)
		{
			Lib_LcdCls();
			Lib_LcdPrintxy(0, 0, 0x80, "     BOARD TEST     ");
			Lib_LcdPrintxy(0*6, 1*8, 0x00, "Total:7 Error:%d", (BOARD_NUM - j));
			Lib_LcdPrintxy(0, 2*8, 0x00, "Error:%s", errBuf);
			//Lib_DelayMs(1000);
			Lib_KbGetCh();
			return 1;
		}
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 0, 0x80, "     BOARD TEST     ");
		Lib_LcdPrintxy(1*6, 1*8, 0x00, "Board test finished~!");
		//Lib_DelayMs(1000);
		Lib_KbGetCh();
		break;
	}
	
	//Lib_DelayMs(2000);
	return 0;
}

//test
static void Case_0608(void)
{
	WORD wRet;
	int i;
	
	for (i=0; i<5; i++)
	{
		wRet = Lib_CheckTimer(i);
		if (wRet !=0)
		{
			Lib_LcdClrLine(2*8, 3*8);
			Lib_LcdPrintxy(0, 2*8, 0, "[%d]Remaining Time:%d", i, wRet);			
		}
		Lib_KbGetCh();
	}

	Lib_StopTimer(0);
}

static void Case_0601(void)
{
	WORD wRet;//, Cnt;
	uchar i;

	for (i=0; i<5; i++)
	{
	
		Lib_SetTimer(i, 0);
		wRet = Lib_CheckTimer(i);
		Lib_LcdClrLine(2*8, 3*8);
		Lib_LcdPrintxy(0, 2*8, 0, "[%d]Remaining Time:%d", i, wRet);
		
		Lib_StopTimer(i);	
		Lib_KbGetCh();
	}
}


int maintest(void)
{
	int iRet;
	BYTE key;
    //test
    char sDisp[200];//,i;
    unsigned char sOutBuf[3*1024];//sBuf[1024*3],
#if !(TEST_INIT)
	int i, result;
#endif

#if !TEST_INIT
	iRet = Lib_AppInit();
#endif

    //test
    //Lib_KbSound(0,0);
    Lib_LcdCls();
    /*
    Lib_WriteSN("3801234567890123456789012345678912345678");
    Lib_ReadSN(sDisp);
    Lib_LcdPrintxy(0, 0*8, 0x00,"SN[%s]",sDisp);
    */
    ShowVosVersion();
    if(Lib_KbGetCh()=='1')
    {
        iRet = Lib_PciWriteWORK_MKey(0,16,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",NULL,0,0);
        Lib_LcdPrintxy(0, 1*8, 0x00,"WriteWORK_MKey[%d]",iRet);
        iRet = Lib_PciWriteWorkKey(2,16,"\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22",NULL,0x81,0);
        Lib_LcdPrintxy(0, 2*8, 0x00,"PciWriteWorkKey[%d]",iRet);
    }      
    iRet = Lib_PciGetMac(2,16, (unsigned char *)sDisp,sOutBuf,1);
    
    Lib_LcdPrintxy(0, 3*8, 0x00,"Lib_PciGetMac[%d]",iRet);
    Lib_KbGetCh();
#if 0
        //test
PEDWriteMkey_Wkey();
//for(;;);
#endif

#if !(TEST_INIT)
	result = 0;
	for (i=0; i<100; i++)
	{
		iRet = Lib_AppInit();
		if (iRet == 0)
		{
			result++;
		}
		Lib_LcdCls();
		Lib_LcdPrintxy(0, 2*8, 0x00, "iRet%d=%d", i, iRet);
		Lib_DelayMs(100);
	}
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 2*8, 0x00, "result=%d", result);
	Lib_KbGetCh();
#endif
	
#if 0	/*for PCI*/   //12/10/25
	Entry_CoreInit();
	QPboc_CoreInit();
#endif

	while(1)
	{
		Lib_LcdCls();
		Lib_KbFlush();
		
		/*显示测试选项*/
		Lib_LcdSetFont(16, 16, 1);
		Lib_LcdPrintxy(0, 0*8, 0x80, "     TEST[V%d.%d]     ", MAJ_VER, MIN_VER);
		Lib_LcdPrintxy(0, 2*8, 0x00, "1-Machine  2-Burn-in");
		Lib_LcdPrintxy(0, 3*8, 0x00, "3-Single   4-Board");

		while(1)
		{
                  
			key = Lib_KbGetCh();
            //test
            trace_debug_printf("KbGetCh[%02x]",key);

#if 0
			if (KEYCANCEL == key)
			{
				return 1;
			}
#endif
			if((key < KEY0) || (key > KEY9))
			{
				continue;
			}

			switch(key)
			{
                case KEY0:
                    Lib_LcdCls();
                    Lib_LcdPrintxy(0,0,2,"文件编译时间\n%s %s",__DATE__,__TIME__);
                    Lib_KbGetCh();            
                    break;

				case KEY1:
					Test_Machine();
					break;

				case KEY2:
					Test_BurnIn();
					break;

				case KEY3:
					Test_AllSingleFunc();
					break;

				case KEY4:
					Test_Board();
					break;
	      #if 0
				case KEY5:/*隐藏的软件系统测试入口*/
					main_SysTest();
					break;
				case KEY6:/*隐藏的软件API测试入口*/
					main_API();
					break;
		#endif
				default:
					break;
			}
			break;
		}
	}
}

//bal


void formatInquire(uchar *amount, int Amountlen) //格式化余额的显示方式 比如： 00010023 -》100.23
{
  int len, j, i = 0;
  uchar tmp[100]; 
  memset(tmp, 0x00, sizeof(tmp));
  
    for(len = 0; len<(Amountlen*2); len++)
      if(amount[len] == '0')
        i++;
      else
        break;
      
      for(j = 0; j<i; j++)
      {
        amount[j] = amount[j + i];
      }
      
       memset(&amount[Amountlen*2 - i], 0x00, sizeof(amount[0]));
      
      amount[Amountlen*2 - i] = amount[Amountlen*2 - i - 1];
      amount[Amountlen*2 - i -1] = amount[Amountlen*2 - i -2];
      amount[Amountlen*2 - i - 2] = '.';
      
      
}

unsigned short Rs_Inquiry(unsigned char *psRet,uchar *pnAmountBcd)
{
	int result, len, Aidlen, i;
        uchar ret, buf[100], mode=0, tmp[100], Aid[32]; 
        uchar inActype[256],outActype[256];
        unsigned short tag;
		unsigned char pDateTime[20]; //记录当前时间
        
        T_Clss_TransParam tClssEcParam;
        T_Clss_PreProcInterInfo tClssProcInterInfo;
        //PICC_PARA PiccPara;
        POS_COM gtPosCom;
	  uchar sType[10],sNo[20];
    
	  memset(sType,0,sizeof(sType));
	  memset(sNo,0,sizeof(sNo));

		Lib_LcdCls();  //清屏
        //Lib_LcdPrintxy(1,1*16,0,"请稍候...");
        Lib_LcdPrintxy(1,1*16,2,"请稍候...");
		
	if( (ret=ClssTransInit()) != EMV_OK) 
	{
		TraceDisp("ClssTransInit[%i]\n",ret);
		*psRet = RC_FILE_ERR;
		return NO_DISP;
	}

	gtPosCom.ucSwipedFlag = CONTACTLESS_ENTER; //刷卡的标志

	memcpy(gtPosCom.tEmvTrans.sAmount,"0",6); //金额，赋值为0
	memset(buf,0,sizeof(buf));
	memcpy(buf,gtPosCom.tEmvTrans.sAmount,6);
	
	tClssEcParam.ulAmntAuth = atol((char const *)buf);  
	tClssEcParam.ulAmntOther = 0;
	tClssEcParam.ucTransType = 0;

      Lib_GetDateTime(pDateTime);

	tClssEcParam.aucTransDate[0] = 0x20; //年份，eg 20  两个asc合并成一个BCD
	tClssEcParam.aucTransDate[1] = pDateTime[0]; //年份，eg 11  
	tClssEcParam.aucTransDate[2] = pDateTime[1]; //月份，eg 05 
	tClssEcParam.aucTransDate[3] = pDateTime[2]; //日期，eg 04 
	
	tClssEcParam.aucTransTime[0] = pDateTime[3];  //小时，eg 16  两个asc合并成一个BCD
	tClssEcParam.aucTransTime[1] = pDateTime[4];  //分钟，eg 28  
	tClssEcParam.aucTransTime[2] = pDateTime[5];  //秒  ，eg 36 

	tClssEcParam.ulTransNo = gtPosParameter.lNowTraceNo; //当前终端流水交易号

	result = Entry_PreTransProc((T_Clss_TransParam *)&tClssEcParam);//交易预处理---在轮询非接触卡片之前调用该函数

	TraceDisp("Entry_PreTransProc[%i]\n", result);
	
	if(result == CLSS_USE_CONTACT) //必须用其他界面进行交易
	{
		*psRet = RC_USE_CONTACT;
		return (disp_clss_err(result));		
	}
	
	if(result != EMV_OK) //参数错误
	{
		*psRet = RC_FAILURE;
		return E_TRANS_FAIL;
	}
		
	while(1) //开始第一个 while(1)
	{	
		if(Lib_PiccOpen() != 0) //检测射频模块
		{
			//Lib_LcdPrintxy(1,1*16,0,"射频模块故障");
			Lib_LcdPrintxy(1,1*16,2,"射频模块故障");
			Lib_PiccClose();
			WaitKey(3);
			TraceDisp("picc_open err");
			*psRet = RC_READER_ERR;
			return NO_DISP;
		}
		/*
		PiccPara.wait_retry_limit_w = 1;
		PiccPara.wait_retry_limit_val = 0;
		PiccPara.card_buffer_w = 1;
		PiccPara.card_buffer_val = 256; 
		picc_setup('W',&PiccPara);
		*/

		//Lib_LcdPrintxy(1,1*16,0,"     请挥卡");
		Lib_LcdPrintxy(1,1*16,2,"     请挥卡");

		while(1)
		{
            if(getkey() == KEYCANCEL) 
			    return E_TRANS_CANCEL;
			//ret = picc_detect(mode,NULL,NULL,NULL,NULL); //轮询卡片是否在感应范围内
			ret =  Lib_PiccCheck(mode,sType,sNo);

			if(ret == 0) 
			{
                          Lib_PiccReset();
				Lib_DelayMs(10);/*huangzhirong*/
				
       			#if 0//yuhongyue
       			Lib_LcdCls();
       			Lib_LcdPrintxy(0, 0, 0x00, "2b or not 2b");
       			Lib_DelayMs(5000);
       			#endif
       			break;
			}
			if(ret == 1 ||ret==2)
			{
				*psRet = RC_READER_ERR;
				return E_TRANS_FAIL;
			}
			if(ret == 3||ret==5||ret==6){
				Lib_DelayMs(100);
				continue;
			}
			if(ret == 4){
				//Lib_LcdPrintxy(0,2*16,0,"感应区卡片过多");
				Lib_LcdPrintxy(0,2*16,2,"感应区卡片过多");
				Lib_PiccClose();
				Lib_DelayMs(1000);
				break;
			}
		}
		if(ret == 4) continue;
		break;
	} //结束第一个 while(1)

	//Lib_LcdPrintxy(0,1*16,0,"请稍侯...");
	Lib_LcdPrintxy(0,1*16,2,"请稍侯...");

        
	buf[0] = 0x07;
	Contactless_SetTLVData(0x9f39,buf,1);
	Contactless_SetTLVData(0x9c,"\x00",1);

	//应用选择
	result = Entry_AppSlt(0,0);
	TraceDisp("Entry_AppSlt[%i]\n",result);
    
	if( result == EMV_NO_APP ||result == ICC_BLOCK || result == EMV_APP_BLOCK) //没有应用，卡被锁，应用被锁
	{
		TraceDisp("Entry_AppSlt[%i]\n",result);
		if(result == EMV_NO_APP)
			*psRet = RC_EMV_NO_APP;
		else if(result == ICC_BLOCK)
			*psRet = RC_ICC_BLOCK;
		else
			*psRet = RC_EMV_APP_BLOCK;
		
		return(disp_clss_err(result));
	}
	if(result != EMV_OK) 
	{
		if(result==ICC_CMD_ERR) //IC卡命令失败
		{
			*psRet =RC_ICC_CMD_ERR;
			#if 1//yuhongyue
			//Lib_LcdPrintxy(0,0,0,"交易失败_0通讯错");
			Lib_LcdPrintxy(0,0,2,"交易失败_0通讯错");
			#else
			Lib_LcdCls();
			//Lib_LcdPrintxy(0,0,0,"trade fail -0, comm_err");
			Lib_LcdPrintxy(0,0,2,"trade fail -0, comm_err");
			Lib_DelayMs(5000);
			#endif
			return E_TRANS_FAIL;
			
		}
		*psRet = RC_TRAN_FAILURE;
		//Lib_LcdPrintxy(0,0,0,"交易失败_1交易错"); //参数错误
		Lib_LcdPrintxy(0,0,2,"交易失败_1交易错"); //参数错误
		return E_TRANS_FAIL;
	}


	while(1) //开始第二个 while(1)
	{
		memset(buf,0,sizeof(buf));

		//最终选择，由内核自动选择最高优先级应用
		result = Entry_FinalSelect(buf, Aid, &Aidlen);
		
		TraceDisp("Entry_FinalSelect[%i]\n",result);
		
		if(result == EMV_NO_APP || result == CLSS_USE_CONTACT ) //没有应用支持 必须使用其他界面（没有这个返回值）
		{
			if(result==EMV_NO_APP)
				*psRet = RC_EMV_NO_APP;
			else
				*psRet = RC_USE_CONTACT;
			sprintf((char *)buf,"error: %i",result);
			//Lib_LcdPrintxy(0,3*16,0,(char *)buf);
			Lib_LcdPrintxy(0,3*16,2,(char *)buf);
			//Lib_LcdPrintxy(0,0,0,"交易失败_2界面错");
			Lib_LcdPrintxy(0,0,2,"交易失败_2界面错");
			return(disp_clss_err(result));
		}
		if(result != EMV_OK)  //命令失败或参数错误
		{
#ifdef PAYWAVE_M					//shall select the next application with highest priority when the SW1
						        //SW2 response t the SELECT ADF command is not ‘9000’
			//删除候选列表中的当前应用
			Entry_DelCurCandApp();
			continue;				
#else
			*psRet = RC_TRAN_FAILURE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_3界面错");
			Lib_LcdPrintxy(0,0,2,"交易失败_3界面错");
			return E_TRANS_FAIL;
#endif
		}
		
		TraceDisp("kernel type[%i]\n",buf[0]);
		
		if( buf[0] != KERNTYPE_PBOC )
		{
			*psRet = RC_TRAN_FAILURE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_4交易错");
			Lib_LcdPrintxy(0,0,2,"交易失败_4交易错");
			return E_TRANS_FAIL;
		}
                result = QPboc_GetTLVData(0x9F5D,tmp,&len); //仅仅用于测试9F5D的返回值
		
		//设置指定标签的数据值
		QPboc_SetTLVData(0x9C,"\x00",1);
		 //获取与最终选择应用AID对应的在交易预处理过程中动态设置的内部参数
		result = Entry_GetPreProcInterFlg(&tClssProcInterInfo);
		
		TraceDisp("Entry_GetPreProcInterFlg[%i]\n",result);
		
		if( result == CLSS_USE_CONTACT ) //必须使用其他界面完成交易
		{
			*psRet = RC_USE_CONTACT;
			//Lib_LcdPrintxy(0,0,0,"交易失败_5界面错");
			Lib_LcdPrintxy(0,0,2,"交易失败_5界面错");
			return disp_clss_err(result);
		}
		else if( result == CLSS_TERMINATE) //应用终止交易（没有这个返回值）
		{
			*psRet = RC_TERMINATE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_6交易终止");
			Lib_LcdPrintxy(0,0,2,"交易失败_6交易终止");
			sprintf((char *)buf,"Error: %i",result);
			//Lib_LcdPrintxy(0,1*16,0,buf);
			Lib_LcdPrintxy(0,1*16,2, (char *)buf);
			return disp_clss_err(result);			
		}
		
		if( result != EMV_OK ) //参数错误
		{
			*psRet = RC_TRAN_FAILURE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_7交易错");
			Lib_LcdPrintxy(0,0,2,"交易失败_7交易错");
			return E_TRANS_FAIL;
		}
		
		//获取与最终选择应用AID对应的在交易预处理过程中动态设置的内部参数
		result = Entry_GetFinalSelectData(inActype, &len);
		
		TraceDisp("Entry_GetFinalSelectData[%i]\n",result);
		if( result != EMV_OK ) //参数错误
		{
			*psRet = RC_TRAN_FAILURE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_8交易错");
			Lib_LcdPrintxy(0,0,2,"交易失败_8交易错");
			return E_TRANS_FAIL;
		}
		
		//设置最终选择相关的数据
		result = QPboc_SetFinalSelectData(inActype,len);
		TraceDisp("QPboc_SetFinalSelectData[%i]\n",result);
		
		if( result != EMV_OK )
		{
			*psRet = RC_TRAN_FAILURE;
			//Lib_LcdPrintxy(0,0,0,"交易失败_9交易错");
			Lib_LcdPrintxy(0,0,2,"交易失败_9交易错");
			return E_TRANS_FAIL;
		}
		
		//设置交易相关参数以及预处理结果传递
		result = QPboc_SetTransData(&tClssEcParam, &tClssProcInterInfo);
		TraceDisp("QPboc_SetTransData[%i]\n",result);
		
		if( result != EMV_OK)
		{
			*psRet = RC_TRAN_FAILURE;
			Lib_LcdPrintxy(0,0,2,"交易失败_10交易错");
			return E_TRANS_FAIL;
		}
		
		memset(tmp,0,sizeof(tmp));
		memset(outActype,0,sizeof(outActype));
		result = QPboc_Proctrans(tmp, outActype); //PBOC非接触交易处理（包括初始化应用及读记录等处理）
                //函数：QPboc_Proctrans(tmp, outActype)结束后，可以查询余额
		
		TraceDisp("QPboc_Proctrans[%i]\n",result);
		if(result == CLSS_RESELECT_APP){
			Entry_DelCurCandApp();
			continue;
		}
		
		if(result == CLSS_USE_CONTACT ) //重新执行最终选择, 重新选择应用
		{
			*psRet = RC_USE_CONTACT;
			Lib_LcdPrintxy(0,0,2,"交易失败_11界面错");
			return(disp_clss_err(result));
		}
		
		if( (result == EMV_USER_CANCEL) || (result==EMV_DATA_ERR) ) //终止非接触交易, 使用非接触界面方式执行交易
																	//返回值没有 EMV_DATA_ERR
		{
			
			*psRet = RC_TERMINATE; 
			Lib_LcdPrintxy(0,0,2,"交易失败_12终止");
			return(disp_clss_err(result));			
		}
		
		
		if(result != EMV_OK) 
		{
			//CLSS_CARD_EXPIRED
			//PAYWAVE: ensure that if the card returns a TC, the reader shall continue the
			//transaction online if the Application Expiration Date is earlier than the
			//current terminal date and if the CTQ indicates “G Online If Application
			//Expired” is 1.
			if(result==CLSS_CARD_EXPIRED) //卡片失效
				*psRet = RC_DECLINE;	
			else
				*psRet = RC_TRAN_FAILURE; 
			Lib_LcdPrintxy(0,0,2,"交易失败_13文件错");
			sprintf((char *)buf,"error: %i",result);
			Lib_LcdPrintxy( 0,1*16,2, (char *)buf);
			return E_TRANS_FAIL;
		}
		
		break;
	} //结束第二个 while(1)

	tag = 0x57;
	ret = Contactless_GetTLVData(tag,tmp,&len);
	if(ret != EMV_OK)
	{
		*psRet = RC_TRAN_FAILURE; 
		Lib_LcdPrintxy(0,0,2,"交易失败_14文件错");
		return E_TRANS_FAIL;
	}
	dat_bcdtoasc(gtPosCom.szTrack2,tmp,len*2);
	
	len = len*2;
	gtPosCom.szTrack2[len]=0;
	if((tmp[len/2-1] & 0x0f) == 0x0f){
		gtPosCom.szTrack2[len-1] = 0;
	}
	gtPosCom.szTrack2[37]=0;
	for(i=0;i<len;i++){
		if(gtPosCom.szTrack2[i] == 'D'){
			gtPosCom.szTrack2[i] = '=';
			break;
		}
	}

	ret = GetCardFromTrack(gtPosCom.szCardNo,gtPosCom.szTrack2,NULL);
	if(ret != EMV_OK)
	{
		*psRet = RC_TRAN_FAILURE;
		Lib_LcdPrintxy(0,0,2,"交易失败_15文件错");
		return E_TRANS_FAIL;
	}

	gtPosCom.tEmvTrans.PANSN = 0xff;
	ret = Contactless_GetTLVData(0x5f34,tmp,&len);
	
	if(ret == EMV_OK) 
	{
		gtPosCom.tEmvTrans.PANSN = tmp[0];
	}
/* !!!
	if(outActype[0] == AC_AAC || 
		outActype[0]==0x11 )   //cryptogram type cannot be determined,the reader shall set the Decline
	{
		//return TRANS_REF;
		*psRet = RC_DECLINE;
		Lib_LcdPrintxy(0,0,0,"交易失败_16拒绝");
		return 29;
	}
*/
        //判断并非拒绝交易，可以查询余额
	   memset(tmp,0,sizeof(tmp));
         result = QPboc_GetTLVData(0x9F5D,tmp,&len); //9F5D的返回值就是余额（tmp使用BCD码记录）
        if(result != 0)
        {
		  Lib_LcdPrintxy(0,0,2,"抱歉，查询余额出错");
          return 30;
        }
        else if (0 == len && result == 0)
        {
          Lib_LcdPrintxy(0,0,2,"Remaining Sum:");
          Lib_LcdPrintxy(0,1*16,2,"0.00");
          #if 0//yuhongyue
	    Lib_KbGetCh();	//huangzhirong
	    #else
	    Lib_DelayMs(1500);
	    #endif
          return 0;
        }
        else
        {
		//test
		s_printf("Bal:");
		for(i=0;i<len;i++)
		    s_printf("%02x ",tmp[i]);
		s_printf("\n");
		memcpy(pnAmountBcd,tmp,6);
		  Lib_LcdPrintxy(0,0,2,"Remaining Sum:");
          memset(buf, 0x00, sizeof(buf));
          dat_bcdtoasc( buf, tmp, len*2);
          
          formatInquire(buf, len); //格式化余额的显示方式 比如： 00010023 -》100.23
	      sprintf((char *)tmp,"  %s",buf);
	      Lib_LcdPrintxy(0,1*16,2,(char *)tmp);
	      #if 0//yuhongyue
		  Lib_KbGetCh();	//huangzhirong
		  #else
		  Lib_DelayMs(1500);
		 #endif
		  
		  //sdk_KbClear(); //清除键盘缓存值
        
          return 0;
        }
}

#endif
 
