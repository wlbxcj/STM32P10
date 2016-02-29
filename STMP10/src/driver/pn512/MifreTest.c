#include "mifre_tmdef.h"


#if (TERM_TYPE == TERM_KF322)

#define RF_KEY_F1  '1'
#define RF_KEY_F2  '2'
#define RF_KEY_F3  '3'
#define RF_KEY_F4  '4'

#define ASCII 0
#endif

//13/07/06
extern PICC_PARA c_para;
unsigned char _glbPiccSetPara = 0;

#if 0 //12/09/25

//extern uchar B_gl_RxThreshold;

extern RF_WORKSTRUCT RF_WorkInfo;

typedef void (*PRFFUN)();

char* strTestMenu[]={ 
	"1-PICCTest",
	"2-LoopAB",     
	"3-PICCTest_Fac", 
	"4-BeepTest",
	"5-Det_NoOpen", 
    "6-Det_Open", 
	"7-M1Test", 
	"8-M1_OperTest" 
	
};
char* strOtherTstMenu[] = {
	"1-Carrier Set",
	"2-Polling",
	"3-WUPA",
	"4-WUPB",
	"5-RATS",
	"6-Attrib",
	"7-Speed",
	"8-LoopAB",
	"9-StepTest"
};


uchar Userfun_test(void);//用于用户实际使用环境循环测试

uchar test_GetNewVal(uchar ucMin, uchar ucMax, uchar ucRst, uchar ucCur, uchar *pucVal);

void DispData2(uint len, uchar *pucData);
void MifreTest(void);
void test_spi(void);
void test_vWriteReg(void);
//void test_vReadReg(void);
uchar char_to_bin(uchar bchar);
extern void vOneTwo(unsigned char *One,unsigned short len,unsigned char *Two);
ulong gulOpenSucCnt=0, gulOpenFailCnt=0;
ulong gulDetectSucCnt = 0, gulDetectFailCnt = 0;
ulong gulIsoCmdSucCnt = 0, gulIsoCmdFailCnt = 0;
ulong gulM1AuthSucCnt = 0, gulM1AuthFailCnt = 0;
ulong gulM1ReadSucCnt = 0, gulM1ReadFailCnt = 0;
ulong gulM1WriteSucCnt = 0, gulM1WriteFailCnt = 0;
ulong gulM1OpeSucCnt = 0, gulM1OpeFailCnt = 0;
ulong gulLoopCnt = 0;
uchar gaucCmdErr[256];
uchar gaucDetErr[256];

uchar ucDetErrNm = 0;
uchar ucCmdErrNm = 0; 

ulong ulCnt1 = 0;
uchar ulCnt2[40]={0};
ulong ulCnt3 = 0;

PICC_PARA my_c_para;
uchar my_GSNON_val = 0;
uchar my_RFCFG_val = 0;

uchar gl_ucTstFlg;
uchar gl_aucRspTstBuf[300];
uchar gl_aucCmdTstBuf[300];

//for analogue & Digital test
uchar gl_aucRecData[1024*1];
ushort gl_usRecLen = 0;

uchar gl_aucLoopBackData[1024];
ushort gl_usLoopBackLen = 0;
uchar gl_ucLoopBackFlg = 0;

uchar gl_ucShowDbgFlg = 0;
ulong gl_ulFwtExtVal = 0;

void test_SendDataByCom(void);

void test_SetParam(void);
 
void DoMenu(uchar ucMaxItem, char* pStrMenu[], PRFFUN* pFun);
void DispMenu(uchar curPage, uchar curLine, uchar MaxItem, char *pMenu[]);


/***********************other PCD with uart and Lcd print************************************/


#define PRINT_UART_LCD  (1) //0-LCD输出操作界面 1-串口输出操作界面

#if(PRINT_UART_LCD ==1)
#define RF_KEY_F1  '1'
#define RF_KEY_F2  '2'
#define RF_KEY_F3  '3'
#define RF_KEY_F4  '4'

#define COM1 0

extern void s_ScrPrint(unsigned char col,unsigned char row,unsigned char mode, char *str);
extern int force_uart_printf(char * fmt,...);
extern void debug_printf(char * str ,...);

#define ScrPrint	debug_printf


extern unsigned char RF_kbhit(void);
static unsigned char kbhit(void)
{
	#if (TERM_TYPE == TERM_SP30 || TERM_TYPE == TERM_R50)
	return RF_kbhit();
	#endif
}

static unsigned char getkey(void)
{
 	unsigned char ch=0;
	//if(!uart0OpenFlag) PortOpen(COM1, "115200,8,n,1");//打开串口
	while(!ch)
		PortRecv(COM1,&ch,0);
	return ch;
}

#endif

/*******************R30_function****************************/
#if (TERM_TYPE == TERM_KF322)
#define CFONT 0
#define COM1 0



void   kbflush(void)
{
	return;
}

void ScrCls(void)
{
	return;
}
void ScrClrLine(unsigned char startline, unsigned char endline)
{
	return;
}
void ScrBackLight(unsigned char mode)
{
	return;
}
void ScrGray(unsigned char level)
{
	return;
}
void ScrGotoxy(unsigned char x, unsigned char y)
{
	return;
}
unsigned char ScrFontSet(unsigned char font_type)
{
	return 0;
}
unsigned char ScrAttrSet(unsigned char attr)
{
	return 0;
}

void ScrSetIcon(unsigned char icon_no, unsigned char mode)
{
	return;
}

int Lcdprintf(unsigned char *fmt,...)
{
	return 0;
}

void kbmute(unsigned char flag)
{
	return ;
}

unsigned char GetString(unsigned char *str, unsigned char mode,
						unsigned char minlen, unsigned char maxlen)
{
	return 0;
}
#endif
/*******************R30_function end****************************/
#if 0
void DispData(uint len, uchar *pucData)
{
    uint i = 0, j = 0, k = 1;
	uchar Temp = 0;
	
	for(j = 0; j < len; j++)
	{
        ScrPrint((char)i, (char)k, 0, "%02x", pucData[j]);
		i = i + 16;
		if(i >= 128)
	//	if (i >= 96)
		{
		   i = 0;
		   k++;
		}

		if(k >= 7)
		{
		    getkey();
			//ScrCls();
			ScrClrLine(1,7);
			k = 1;
		}
	}
}
#endif

void DispData2(uint len, uchar *pucData)
{
    uint i = 0, j = 0, k = 3,p=0;
	uchar Temp = 0;

	for(j = 0; j < len; j++)
	{
		if(p)			
			ScrPrint(0,2,0,"Pg: %d ", p);
        //ScrPrint((char)i, (char)k, 0, "%02x", pucData[j]);
        ScrPrint( "%02x", pucData[j]);
		i = i + 16;
		if(i >= 128)
	//	if (i >= 96)
		{
		   i = 0;
		   k++;
		}

		if(k >= 7)
		{
		    getkey();
			//ScrCls();
			ScrClrLine(3,7);
			k = 3;
			p++;
		}
	}
}

void vOneTwo(unsigned char *One,unsigned short len,unsigned char *Two)
{
    unsigned char  i;
    static unsigned char TAB[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (i = 0; i < len; i++) {
        Two[i * 2] = TAB[One[i] >> 4];
        Two[i * 2 + 1] = TAB[One[i] & 0x0f];
    }
}




/********************************************************************
以下为无差异函数实现
********************************************************************/

void test_ParaSetup(void)
{
	PiccSetup('w',&my_c_para); 
}

void DispMenu(uchar curPage, uchar curLine, uchar MaxItem, char *pMenu[])
{ 
    uchar i, j;
	ScrCls();
    ScrFontSet(CFONT);
    for (i = 0; i < 4; i++) {
        if (i == curLine)  ScrAttrSet(0x80); 
        else ScrAttrSet(0); 
        j = curPage * 4 + i;
		if (j >= MaxItem)  
			return; 
        ScrGotoxy(0, (unsigned char)(i * 2)); 
        Lcdprintf("%s", pMenu[j]);
    }
}

int GetMenuNo(uchar *pcurPage, uchar *pcurLine, uchar ucMaxItem, char* pStrMenu[])
{   
	uchar rc, totalPage, totalNum,ucMaxLine;
	uchar curPage = *pcurPage;
	uchar curLine = *pcurLine;

	ucMaxLine = 4;
	totalPage = ucMaxItem / ucMaxLine;
	if (ucMaxItem % ucMaxLine) 
	{
		totalPage++;
	}
			
	ScrCls();	 
	while (1)
	{
        DispMenu(curPage, curLine, ucMaxItem, pStrMenu);
        rc = getkey();
        switch (rc)
		{
			case RF_KEY_F4:
				ScrCls();			
				*pcurPage = curPage;
				*pcurLine = curLine;
				return (curPage*ucMaxLine+curLine);
			case RF_KEY_F3:
				ScrCls();
				return -1;

			case RF_KEY_F1:
				if (curLine > 0)
					curLine--;
				else
				{
					if (curPage)
					{
						curPage--;
						curLine = ucMaxLine-1;
					}
					else
					{
						curPage = totalPage-1;
						curLine = (ucMaxItem-1)%ucMaxLine;
					}
				}
				break;

			case RF_KEY_F2:
				if (curLine < (ucMaxLine-1))
				{
					if ((curPage < totalPage-1) || curLine < (ucMaxItem-1) % ucMaxLine)
						curLine++;
					else
					{
						curLine = 0;
						curPage = 0;
					}
				}
				else
				{
					if (curPage < totalPage-1) 
					{
						curPage++;
						curLine = 0;
					}  
					else
					{
						curPage = 0;
						curLine = 0;
					}
				}
				break;

			default:
				break;
		}
    }
} 

void DoMenu(uchar ucMaxItem, char* pStrMenu[], PRFFUN* pFun)
{   
	uchar rc, curPage, curLine, totalPage, totalNum,ucMaxLine;
	int No = 0;
 
 	curLine = 0;
	curPage = 0;
	ucMaxLine = 4;
	totalPage = ucMaxItem / ucMaxLine;
	if (ucMaxItem % ucMaxLine) 
	{
		totalPage++;
	}
			
	ScrCls();	 
	while (1)
	{
		No = GetMenuNo(&curPage, &curLine, ucMaxItem, pStrMenu);
		if (No == -1) 
		{
			ScrCls();
			return;
		}	

		pFun[No]();
		ScrCls();	
    }
} 
uchar test_GetNewVal(uchar ucMin, uchar ucMax, uchar ucRst, uchar ucCur, uchar *pucVal)
{
	
	uchar ucStep;
	uchar ucKey =  0;	 
	uchar ucTemp = ucCur; 
	uchar ucVal = ucCur;

	while(1)
	{
		Lib_LcdPrintxy(0,1*8,0,"Range:%02X~%02XH,Rst:%02XH", ucMin, ucMax, ucRst);  
		Lib_LcdPrintxy(0,2*8,0,"Cur: %02X  ", ucVal);
		Lib_LcdPrintxy(0,3*8,0,"New: %02X  ", ucTemp);

		Lib_LcdPrintxy(0,4*8,0,"STEP: F1-1 F2-5 \nF3-10 F4-Quit ");
                
                ucStep = 1;
#if 0
		ucKey = getkey();
		if (ucKey == RF_KEY_F1) 
		{
			ucStep = 1;
		}
		else if(ucKey == RF_KEY_F2)
		{
			ucStep = 5;
		}
		else if(ucKey == RF_KEY_F3)
		{
			ucStep = 10;
		}
		else
		{
			*pucVal=ucVal;//确保第一次进入立即出去，值设置上
			return 0 ;
		}
		
		ScrClrLine(4,7);
		ScrPrint(0,5,0,"F1-INC %d  F2-DEC %d ", ucStep, ucStep);			
		ScrPrint(0,6,0,"F3-Cancel  F4-Confirm");
#endif			
                Lib_LcdPrintxy(0,4*8,0,"1-INC 2-DEC 3-ESC 4-Ent");
                
		while (2) 
		{
			//if (!(kbhit())) 
			{
				ucKey = getkey();
				if (ucKey ==RF_KEY_F3) 
				{
					ScrClrLine(4,7);
					ScrPrint(0,6,0," Set Canceled ");
					getkey();
					ScrCls();
					return 1;
				}
				if (ucKey == RF_KEY_F1) 
				{
					if ((ucTemp + ucStep) > ucMax ) 
					{
						ucTemp = ucMin;
					}
					else
						ucTemp += ucStep;
					 #if(TERM_TYPE==TERM_R30 ||PRINT_UART_LCD ==1)
					 ScrPrint(0,6,0," include %d",ucStep);
					 ScrPrint(0,3,0,"Current NewVal: %02X  ", ucTemp);
					 #endif
				}
				if (ucKey == RF_KEY_F2) 
				{
					if (ucTemp < ucStep) 
					{
						ucTemp = ucMax;
					}
					else if ((ucTemp - ucStep) < ucMin) 
					{
						ucTemp = ucMax;
					}
					else
						ucTemp -= ucStep;

					#if(TERM_TYPE==TERM_R30 || PRINT_UART_LCD ==1)
					ScrPrint(0,6,0," decrease %d",ucStep);
					ScrPrint(0,3,0,"Current NewVal: %02X  ", ucTemp);
					#endif
				}	
				if (ucKey == RF_KEY_F4) 
				{
					ScrClrLine(4,7);
					ScrPrint(0,6,0,"Set OK ");
					getkey();
					*pucVal = ucTemp;
					ucVal = ucTemp;
					ScrCls();
  				    break;
				}
			}
			#if(TERM_TYPE!=TERM_R30 && PRINT_UART_LCD !=1)
			ScrPrint(0,3,0,"New: %02X  ", ucTemp);
			#endif
		}
	}
}


void test_SetParam(void)
{
	uchar ucMax;
	uchar ucMin;
	uchar ucRst;
	uchar ucStep;
	uchar ucKey = 0x00;	
    uchar ucRet = 0;
	uchar ucTemp = 0;
	uchar aucStr[10];
	uchar ucVal = 0;
        uchar ucTemp;
        //output value
          
	PN512_s_vRFWriteReg(1,PN512_MODGSP_REG, &ucTemp);//ModGsP
        trace_debug_printf(" B ModGsP[%02x]\n",ucTemp);
        PN512_s_vRFWriteReg(1,PN512_GSNON_REG, &ucTemp)
        trace_debug_printf(" B ModGsP[%02x]\n",ucTemp);
          
          
	Lib_LcdCls();
	while(1)
	{
		Lib_LcdPrintxy(0,0,0x80,"  PCD Param Set");
		Lib_LcdPrintxy(0,1*8,0,"1-A_CwConduct"); // CWGsPREG ADD:28H,DEF VAL:0x3F
		Lib_LcdPrintxy(0,2*8,0,"2-B_ModConduct "); // GsNOnREG bit7~4 ADD:27H,DEF VAL:0xF0 低4位数据将被清除
		Lib_LcdPrintxy(0,3*8,0,"3-B_RxThrsHld"); // ModGsPREG bit5~0 add 29h, DEF val:0x11 
		Lib_LcdPrintxy(0,4*8,0,"4-Quit ");
	    
		ucKey = getkey();
		switch(ucKey)
		{
		case RF_KEY_F1:
			Lib_LcdCls();
			ucMax = 0x3F;
			ucMin = 0x00;
			ucRst = 0x20; 
			ucTemp = my_c_para.a_conduct_val;
			Lib_LcdPrintxy(0,0,0," A_CwConduct:%02X  ", ucTemp);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal;
			my_c_para.a_conduct_val = ucTemp;
			Lib_LcdCls();
			break;
		case RF_KEY_F2:
			Lib_LcdCls();
			ucMax = 0x3F;
			ucMin = 0x00;
			ucRst = 0x20; 
			ucTemp = my_c_para.b_modulate_val;
			Lib_LcdPrintxy(0,0,0," B_ModConduct:%02X  ", ucTemp);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal;	 
			my_c_para.b_modulate_val = ucTemp;
			Lib_LcdCls();
			break;
			
		case RF_KEY_F3:
			Lib_LcdCls();
			ucMax = 0xFF;
			ucMin = 0x00;
			ucRst = 0x88; 
			//ucTemp = B_gl_RxThreshold;
			ucTemp = my_c_para.a_conduct_val;
                        
			Lib_LcdPrintxy(0,0,0," B_RxThreshold:%02X  ", ucTemp); 		
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal; 
			//B_gl_RxThreshold = ucTemp;
			my_c_para.a_conduct_val = ucTemp;
                        
			Lib_LcdCls();
			break;
			
		case RF_KEY_F4: 
			Lib_LcdCls();
			return;
		 
		default:
			break;
		}
				
		//memset(&my_c_para, 0, sizeof(PICC_PARA));
		//memcpy(&my_c_para, &c_para, sizeof(PICC_PARA));
		my_c_para.a_conduct_w = 1;
		my_c_para.b_modulate_w = 1;
//		my_c_para.card_buffer_w = 1;
		my_c_para.m_conduct_w = 1;
		my_c_para.wait_retry_limit_w = 0;
	}
}

void test_SetRFParam(void)
{
	uchar ucRet = 0;
	uchar ucTemp = 0;

	kbflush();

	ScrCls();
	ScrPrint(0,0,0," PARAM SET/RESET ");
	ScrPrint(0,2,0,"F1-Set New Val   ");
	ScrPrint(0,3,0,"F2-Reset to Default");
	ScrPrint(0,4,0,"F3-Set Debug Show");
	ScrPrint(0,5,0,"F4-Cancel  ");

	ucRet = getkey();
	if (ucRet == RF_KEY_F1) 
	{		
		PiccOpen();
		test_SetParam();
		PiccClose();	
		return;
	}
	else if (ucRet == RF_KEY_F2) 
	{
		memset(&my_c_para, 0, sizeof(my_c_para));
		
		ucRet = PiccOpen();
		if (ucRet) 
		{
			ScrCls();
			ScrPrint(0,3,0,"Open Err: %02X ", ucRet);
			getkey();
			return;
		}
		PiccSetup('r',&my_c_para);
		PiccClose();

		ScrClrLine(2,7);
		ScrPrint(0,4,0,"Reset Param OK!");
		DelayMs(500);
		return;
	}
	else if(ucRet == RF_KEY_F3) // 20081210 FOR 2ND TB201 TEST, SHOW DEBUG INFO
	{
		kbflush();
		ScrCls();
		ScrPrint(0,0,0,"Debug Set");
		ScrPrint(0,2,0,"F1-Set Show Flg");
		ScrPrint(0,3,0,"F2-Set FWT Ext ");
		ScrPrint(0,4,0,"F3/F4-Cancel");
		ucRet = getkey();
		if (ucRet == RF_KEY_F1) 
		{		
			ScrCls();
			ScrPrint(0,0,0," Set Show Flg ");
			ScrPrint(0,2,0,"F1-Show   ");
			ScrPrint(0,3,0,"F2-No Show");
			ScrPrint(0,4,0,"F3/F4-Cancel  ");
			if (gl_ucShowDbgFlg == 0) 
			{
				ScrPrint(0,6,0,"Cur: No Show");
			}
			else
			{
				ScrPrint(0,6,0,"Cur: Show   ");
			}

			ucRet = getkey();
			if (ucRet == RF_KEY_F1) 
			{
				gl_ucShowDbgFlg = 1;
			}
			else if(ucRet == RF_KEY_F2)
			{
				gl_ucShowDbgFlg = 0;
			}
			else
			{
				ScrCls();
				return;
			}

			if (gl_ucShowDbgFlg == 0) 
			{
				ScrPrint(0,7,0,"New: No Show");
			}
			else
			{
				ScrPrint(0,7,0,"New: Show   ");
			}
		}
		else if (ucRet == RF_KEY_F2) 
		{
			ScrCls();
			ScrPrint(0,0,0," Set FWT Ext  ");
			ScrPrint(0,2,0,"F1-Add 100 ETU");
			ScrPrint(0,3,0,"F2-Normal");
			ScrPrint(0,4,0,"F3-Other Val");
			ScrPrint(0,5,0,"F4-Cancel  ");
			ScrPrint(0,6,0,"Cur Add: %d ", gl_ulFwtExtVal);

			ucRet = getkey();
			if (ucRet == RF_KEY_F1) 
			{
				gl_ulFwtExtVal = 100;
			}
			else if(ucRet == RF_KEY_F2)
			{
				gl_ulFwtExtVal = 0;
			}
			else if (ucRet == RF_KEY_F3) 
			{
				ScrClrLine(1,7);
				ucRet = test_GetNewVal(0, 20, 0, (uchar)gl_ulFwtExtVal, &ucTemp);
				if (ucRet == 0) 
				{
					gl_ulFwtExtVal = (ulong)ucTemp;
				}
			}
			else
			{
				ScrCls();
				return;
			}

			ScrPrint(0,7,0,"New Add: %d ", gl_ulFwtExtVal);
		}
		getkey();
		return;
	}
	else //if (ucRet == RF_KEY_F3 || ucRet == RF_KEY_F4) 
	{
		ScrClrLine(2,7);
		ScrPrint(0,4,0,"Param Set Canceled");
		DelayMs(500);
		return;
	}
}

void test_LoopABTest(void)
{
	uchar ucKey = 0x00;	
    uchar ucRet = 0;
	uchar aucOut[100];
	uchar Other[100];
	uchar ucLoopDetFlg = 0;
	uchar i = 0;
	uchar ucMode = 0;
	uchar ucRemove = 1;
	uchar ucClose = 1;
	uchar ucLc = 0;
	uchar aucDc[255];
	uchar ucTemp = 0;

	
	APDU_SEND apdu_s;
	APDU_RESP apdu_r;
			
	ScrBackLight(2);
	ScrCls();
	ScrPrint(0,2,0,"F1-'A' F2-'B' F3-'M' \n F4-Auto ");
	kbflush();
	ucKey = getkey();
	if(ucKey == RF_KEY_F1)
		ucMode = 'A';
	else if(ucKey == RF_KEY_F2)
		ucMode = 'B';
	else if(ucKey == RF_KEY_F3)
		ucMode = 'M';
	else
	{
		ucMode = 0;
		//*
		ScrPrint(0,2,0,"F1-0 F2-1 F3-Quit  ");
		kbflush();
		ucKey = getkey();
		if (ucKey == RF_KEY_F1) 
			ucMode = 0;
		else if (ucKey == RF_KEY_F2) 
		{
			ucMode = 1;
		}
		else
			return;//*/
	}
	gulLoopCnt = 0;
	gulOpenSucCnt = 0;
	gulOpenFailCnt = 0;
	gulDetectSucCnt = 0;
	gulDetectFailCnt = 0;
	gulIsoCmdSucCnt = 0;
	gulIsoCmdFailCnt = 0;
	memset(gaucCmdErr, 0, sizeof(gaucCmdErr));
	memset(gaucDetErr, 0, sizeof(gaucDetErr));
	
	ucDetErrNm = 0;
	ucCmdErrNm = 0;
//		ScrPrint(0,0,0," Loop Cnt [ %lu ]", gulLoopCnt);
		// open
//		DelayMs(100);
	//	test_ParaSetup();
//*
	ScrCls();
	ScrPrint(0,2,0,"Set Param?");
	ScrPrint(0,4,0,"F1-Y");
	if(getkey() == RF_KEY_F1)
	{
		test_SetParam();
//		test_ParaSetup();
	}
	ScrCls();
	ScrPrint(0,0,0,"F1- LOOP A/B TEST ");
	ScrPrint(0,1,0,"F2- Loop Detect   ");
//	ScrPrint(0,2,0,"3- Loop Poll Test");
	ucKey = getkey();
	if(ucKey == RF_KEY_F2)
	{
		ucLoopDetFlg = 1;
		ScrCls();
		ScrPrint(0,0,0x40," LOOP DETECT TEST ");
	}
	
	else
	{		
		memset(aucDc, 0, sizeof(aucDc));
		ucLc = 0x0E;

		ScrCls();
		ScrPrint(0,1,0,"APDU CMD");
		ScrPrint(0,2,0,"1-SLT PSE  2-PPSE\n3-Quit");
		ucKey = getkey();
		if(ucKey == RF_KEY_F1)
		{
			strcpy(aucDc, "1PAY.SYS.DDF01");
		}
		else if(ucKey == RF_KEY_F2)
		{
			strcpy(aucDc, "2PAY.SYS.DDF01");
		}
		else if(ucKey == RF_KEY_F3)
		{
			return;
		}
		ucLoopDetFlg = 0;

		ScrCls();	
		ScrPrint(0,0,0x40," LOOP A/B TEST ");
	}
	ScrPrint(0,1,0," [F3] to exit ");
	ScrCls();
//	*/
	DelayMs(1000);
	while(1)
	{
		if ((!kbhit()) && (getkey() == RF_KEY_F3))
			break;
		
		ScrPrint(0,0,0, " Loop Cnt [ %d ] ", gulLoopCnt);
		ScrPrint(0,2,0, "Op Suc[%d] Err[%d] ", gulOpenSucCnt, gulOpenFailCnt);
		ScrPrint(0,3,0, "De Suc[%d] Err[%d] ", gulDetectSucCnt, gulDetectFailCnt);
		ScrPrint(0,4,0, "cm Suc[%d] Err[%d] ", gulIsoCmdSucCnt, gulIsoCmdFailCnt);

		gulLoopCnt++;
		ucRet = PiccOpen();
		if(ucRet == RET_RF_OK)
		{
			gulOpenSucCnt++;
		}	
		else
		{
			gulOpenFailCnt++;	
			ScrPrint(0,7,0,"Op ER[%d %d] ", gulOpenFailCnt,gulLoopCnt);
			// Close
			PiccClose();	
			getkey();
			return;
		}
		ScrClrLine(5,7);
		 	
		test_ParaSetup();	
//		s_RFResetCarrier();
		// detect 
		memset(aucOut, 0, sizeof(aucOut));
		ucRet = PiccDetect(ucMode,&ucKey,aucOut,NULL,Other);
		//s_vRFReadReg(1,RF_REG_MODCONDUCTANCE,&ucTemp);
		s_vRFReadReg(1,PN512_MODGSP_REG,&ucTemp);
                
//		ScrPrint(0,5,0,"B_ModIndex:%02X ", ucTemp);
//		DelayMs(50);

		if(ucRet == RET_RF_OK)
		{
			gulDetectSucCnt++;
			ScrPrint(0,6,0,"Dt %c OK: %02X \n sn: %02X %02X %02X %02X",ucKey, aucOut[0], aucOut[1],aucOut[2],aucOut[3],aucOut[4]); 
//			getkey();
			if(ucLoopDetFlg)
			{	
				continue;
			}
		}	
		else
		{	
			for(i = 0; i<ucDetErrNm;i++)
			{
				if(Other[1] == gaucDetErr[i])
					break;
			}
			if(i == ucDetErrNm)
			{
				gaucDetErr[ucDetErrNm] = Other[1];	
				ucDetErrNm++;
			}
			
			gulDetectFailCnt++;	
			ScrPrint(0,7,0,"Dt ER[%d %d]", gulDetectFailCnt,gulLoopCnt);	 
			ScrPrint(0,6,0,"Detect Err:%02X %02X ", ucRet,Other[1]);
		//	test_ErrBeep();	
			
//			DelayMs(400);
			// Close
			PiccClose();
//			ScrPrint(0,6,0,"Close ");
			continue;
		}

		if(ucKey != 'M')
		{	
			// isocmd
			memcpy(apdu_s.Command, "\x00\xA4\x04\x00", 4);
			apdu_s.Lc = ucLc;//0x0E;
			apdu_s.Le = 256;
			//memcpy(apdu_s.DataIn, "2PAY.SYS.DDF01", 0x0E);
			memcpy(apdu_s.DataIn, aucDc, ucLc);
			ucRet = PiccIsoCommand(0, &apdu_s, &apdu_r);
			if(ucRet == RET_RF_OK)
			{
				gulIsoCmdSucCnt++;
//				ScrPrint(0,7,0,"Cm OK[%lu %lu]", gulIsoCmdSucCnt,gulLoopCnt);
//				DelayMs(400);	 
			}	
			else
			{
				for(i = 0; i<ucCmdErrNm;i++)
				{
					if(apdu_r.SWA == gaucCmdErr[i])
						break;
				}
				if(i == ucCmdErrNm)
				{
					gaucCmdErr[ucCmdErrNm] = apdu_r.SWA;	
					ucCmdErrNm++;
				}
				gulIsoCmdFailCnt++;	
				ScrPrint(0,7,0,"Cm ER[%d %d]", gulIsoCmdFailCnt,gulLoopCnt);	 
				ScrPrint(0,6,0,"iso cmd Err:%02X %02X ", ucRet, apdu_r.SWA);
			//	test_ErrBeep();
				
				DelayMs(400);
				PiccClose();
				continue;
			}
		}

	}	
	PiccClose();
	ScrCls();
	ScrPrint(0,2,0," Loop Cnt [ %d %d %d ]", gulLoopCnt, ucDetErrNm, ucCmdErrNm);
	ScrPrint(0,3,0,"Opn Suc[%d] Err[%d]", gulOpenSucCnt, gulOpenFailCnt);
	ScrPrint(0,4,0,"Det Suc[%d] Err[%d]", gulDetectSucCnt, gulDetectFailCnt);
	ScrPrint(0,5,0,"cmd Suc[%d] Err[%d]", gulIsoCmdSucCnt, gulIsoCmdFailCnt);
	if(ucDetErrNm)
		ScrPrint(0,6,0,"DE:%02X %02X %02X %02X %02X %02X ",gaucDetErr[0],gaucDetErr[1],gaucDetErr[2],gaucDetErr[3]
	,gaucDetErr[4],gaucDetErr[5]);
	
	if(ucCmdErrNm)
		ScrPrint(0,7,0,"CE:%02X %02X %02X %02X %02X %02X ",gaucCmdErr[0],gaucCmdErr[1],gaucCmdErr[2],gaucCmdErr[3]
	,gaucCmdErr[4],gaucCmdErr[5]);
	getkey();
	ScrCls();

}

uchar char_to_bin(uchar bchar)
{
    if ((bchar >= '0') && (bchar <= '9'))
        return (bchar - '0');
    else {
        if ((bchar >= 'A') && (bchar <= 'F'))
            return (bchar - 'A' + 10);
        if ((bchar >= 'a') && (bchar <= 'f'))
            return (bchar - 'a' + 10);
        else
            return(0);        // old value is 0x0f
    }
}
uchar test_ucGetInput(uchar *pucStr)
{
	uchar ucRet = 0;
	uchar aucStr[10];

	ScrPrint(0,3,0,"Input New Val:");
	memset(aucStr,0,sizeof(aucStr));
	ScrGotoxy(0,5);
	ucRet = GetString(aucStr,0xB5,0,2);
	if(ucRet == 0)
	{
		if(aucStr[0] > 0)
			*pucStr = char_to_bin(aucStr[1]) * 16 + char_to_bin(aucStr[2]);
		ScrPrint(0,7,0,"new val:%02X",*pucStr);
		return aucStr[0];
	}
	else
	{
		ScrPrint(0,7,0,"CanCeled");
		return 0;
	}
}

void test_ProcErr(uchar ucRet)
{
	switch (ucRet)
	{
	case RET_RF_OK	          :  
		ScrPrint(0,7,0,"OK	         ");
		break;   
	case RET_RF_ERR_PARAM	  :   
		ScrPrint(0,7,0,"ERR_PARAM	 "); 
		break;   
	case RET_RF_ERR_NO_OPEN	  :   
		ScrPrint(0,7,0,"ERR_NO_OPEN	 ");
		break;   
	case RET_RF_ERR_NOT_ACT	  :   
		ScrPrint(0,7,0,"ERR_NOT_ACT	 ");
		break;   
	case RET_RF_ERR_MULTI_CARD:   
		ScrPrint(0,7,0,"ERR_MULTI_CARD");
		break;   
	case RET_RF_ERR_TIMEOUT	  :   
		ScrPrint(0,7,0,"ERR_TIMEOUT	 "); 
		break;	  
	case RET_RF_ERR_PROTOCOL  :    
		ScrPrint(0,7,0,"ERR_PROTOCOL  "); 
		break;   
	case RET_RF_ERR_TRANSMIT  :    
		ScrPrint(0,7,0,"ERR_TRANSMIT  "); 
		break;   
	case RET_RF_ERR_AUTH	  :    
		ScrPrint(0,7,0,"ERR_AUTH	     ");
		break;   
	case RET_RF_ERR_NO_AUTH	  :    
		ScrPrint(0,7,0,"ERR_NO_AUTH	 ");
		break;   
	case RET_RF_ERR_VAL   	  :   
		ScrPrint(0,7,0,"ERR_VAL   	 ");
		break;   
	case RET_RF_ERR_CARD_EXIST:   
		ScrPrint(0,7,0,"ERR_CARD_EXIST"); 
		break; 
	case RET_RF_ERR_STATUS	  :   
		ScrPrint(0,7,0,"ERR_STATUS	 "); 
		break;   
	default:
		ScrPrint(0,7,0,"Unknown Err:%02X",ucRet);
		break;
	}
} 
void test_api(void)
{
	ulong ulLong = 0;
	uchar aucStr[20] = {0};
    uchar ucRet = RET_RF_OK;
	uchar ucResp[100];
	uchar aucUID[20];
	uchar ucSAK = 0;
	uchar ucATSLen = 0;
	uchar ucATSData[100];
	uchar aucSn[10];

 	uchar ucKey = 0x00;
	uchar ucTemp = 0;
	uchar ucBlkNo = 0;
	APDU_SEND  ApduSend;
	APDU_RESP  ApduRecv;
	uchar aucValRd[20];
	uchar aucValWr[20];
	uchar ucUpBlkNo = 0;
	ulong i = 0;
	
	
	uchar rc, curPage, curLine, totalPage, totalNum,ucMaxLine, ucMaxItem = 11;
	uchar ucMax ,ucMin ,ucRst ,  ucVal;
	uchar operateValue[4]={0};
	int No = 0;
	char *acStrApiMenu[]=
	{
		"1-RF Open",
		"2-RF Close",
		"3-Detect",
		"4-APDU",
		"5-M1Auth",
		"6-M1Read",
		"7-M1Write",
		"8-M1Ope",
		"9-Remove",
		"10-RATS",
		"11-Time"
	};
 
 	curLine = 0;
	curPage = 0;
	ucMaxLine = 4;
	totalPage = ucMaxItem / ucMaxLine;
	if (ucMaxItem % ucMaxLine) 
	{
		totalPage++;
	}
			
	memset(aucValRd,0,sizeof(aucValRd));
	memset(aucValWr,0,sizeof(aucValWr));
	

	ScrCls();
	
	while (1)
	{
		#if (TERM_TYPE == TERM_R30 ||PRINT_UART_LCD ==1)
		ScrPrint(0,0,0,"0-RF Open,1-RF Close,2-Detect,3-APDU");
		ScrPrint(0,1,0,"4-M1Auth,5-M1Read,6-M1Write,7-M1Ope");
		ScrPrint(0,2,0,"8-Remove,9-RATS,A/a-Time,E/e-exit");
		No=getkey();
		#else
		No = GetMenuNo(&curPage, &curLine, ucMaxItem, acStrApiMenu);
	
		if (No == -1) 
		{
			ScrCls();
			return;
		}
		#endif
		switch(No) 
		{

		case -1:
		case 'E':
		case 'e':
		{
			ScrCls();
			return;
		}
		break;
		case 0:
		case '0':
			ScrCls();
			ScrPrint(0,1,0,"RF Open");
			test_ProcErr(PiccOpen());
			getkey();
			ScrCls();
			break;
		case 1:
		case '1':
			ScrCls();
			ScrPrint(0,1,0,"RF Close");
			PiccClose();
			ScrPrint(0,7,0,"RF Close OK");
			getkey();
			ScrCls();
			break;
		case 2:
		case '2':
			ScrCls();
			ScrPrint(0,1,0,"RF Detect");

			ScrPrint(0,2,0,"1- A");
			ScrPrint(0,3,0,"2- M");
			ScrPrint(0,4,0,"3- B");
			ScrPrint(0,5,0,"4- 0");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)
				ucTemp = 'A';
			else if(ucKey == RF_KEY_F2)
				ucTemp = 'M';
			else if(ucKey == RF_KEY_F3)
				ucTemp = 'B';
			else 
				ucTemp = 0;
			
			memset(aucUID,0,sizeof(aucUID));
			memset(ucATSData,0,sizeof(ucATSData));
			memset(ucResp,0,sizeof(ucResp));
			memset(aucSn,0,sizeof(aucSn)); 
			ScrCls();
			ScrPrint(0,0,0,"Pls Wait...");
			ucRet = PiccDetect(ucTemp, &ucTemp, aucUID, ucResp, ucATSData); 
			if(ucTemp != RF_TYPE_B)
			{				
				ScrPrint(0,2,0,"Card Type:%c SN[%d]: ",ucTemp, aucUID[0]);
				#if (TERM_TYPE != TERM_S80 && TERM_TYPE != TERM_S90)
				DispData2(aucUID[0], aucUID+1); 
				#endif
				ScrPrint(0,5,0,"err code:%02X %02X", ucATSData[1],ucATSData[2]);
				ScrPrint(0,6,0,"ATQA:%02X %02X SAK:%02X",ucATSData[3],ucATSData[4],ucATSData[5]);
			}
			else //if(ucTemp == 'B')
			{
				ScrPrint(0,3,0,"Card Type:%c",ucTemp);
				// ATQB[12] 
				ScrPrint(0,4,0,"SN:%02X %02X %02X %02X",aucUID[0],aucUID[1],aucUID[2],aucUID[3]);
				ScrPrint(0,5,0,"ATQB:%02X %02X %02X %02X %02X",ucATSData[3],ucATSData[4],ucATSData[5]
					,ucATSData[6],ucATSData[7]);
			}
			test_ProcErr(ucRet);
			getkey();
			if(ucRet == 0)
			{
				memcpy(aucSn,aucUID+1,aucUID[0]);
				ScrPrint(0,3,0,"Len:%d,aucSn:%02x,%02x,%02x,%02x",aucUID[0],aucSn[0],aucSn[1],aucSn[2],aucSn[3]);
				getkey();
			}
				
			ScrCls();
		
			break;
		case 3:
		case '3':
			ScrCls();
			ScrPrint(0,1,0,"APDU CMD");
			ScrPrint(0,2,0,"1-SLT PSE  2-PPSE\n3-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)
			{
				memcpy(ApduSend.Command,"\x00\xA4\x04\x00",4);
				ApduSend.Lc = 0x0E;
				memset(ApduSend.DataIn,0,sizeof(ApduSend.DataIn));
				strcpy(ApduSend.DataIn,"1PAY.SYS.DDF01");
				ApduSend.Le = 256;					
			}
			else if(ucKey == RF_KEY_F2)
			{
				memcpy(ApduSend.Command,"\x00\xA4\x04\x00",4);
				ApduSend.Lc = 0x0E;
				memset(ApduSend.DataIn,0,sizeof(ApduSend.DataIn));
				strcpy(ApduSend.DataIn,"2PAY.SYS.DDF01");
				ApduSend.Le = 256;					
			}
			else if(ucKey == RF_KEY_F3)
			{
				return;
			}
			else
			{
				ScrCls();
				break;
			}				
			memset(&ApduRecv,0,sizeof(ApduRecv));
			ucRet = PiccIsoCommand(0, &ApduSend, &ApduRecv);
			ScrPrint(0,5,0,"ret:%02X SW:%02X %02X",ucRet,ApduRecv.SWA,ApduRecv.SWB);
			
			ScrPrint(0,6,0,"RSP:%02X %02X %02X %02X %02X",ApduRecv.DataOut[0],ApduRecv.DataOut[1],
				ApduRecv.DataOut[2],ApduRecv.DataOut[3],ApduRecv.DataOut[4]);
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;
		case 4:
		case '4':
			ScrCls();
			ScrPrint(0,1,0," M1Auth ");
			ScrPrint(0,3,0,"1- Key A   2 - Key B");
			ucTemp = getkey();
			if(ucTemp == RF_KEY_F1)
				ucTemp = 'A';
			else
				ucTemp = 'B';


			getkey();
			ScrCls();
			ucMax = 64;//共64个块
			ucMin = 0x00;
			ucRst = 22; 
			ucBlkNo = 22;
			ScrPrint(0,0,0," CurBlkNo:%d", ucBlkNo);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucBlkNo, &ucVal))
				ucBlkNo = ucVal;
			ScrCls();
			ScrPrint(0,4,0," SetBlkNo:%d", ucBlkNo);
			getkey();

			//memset(aucUID,0,sizeof(aucUID));	
			ucRet = M1Authority(ucTemp, ucBlkNo, "\xFF\xFF\xFF\xFF\xFF\xFF", aucSn);
			ScrPrint(0,3,0,"Auth Ret:%02X          ",ucRet);
			ScrPrint(0,4,0,"SN:%02X %02X %02X %02X",aucSn[0],aucSn[1],aucSn[2],aucSn[3]);
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;

		case 5:
		case '5':
			ScrCls();
			ScrPrint(0,0,0," M1 Read ");
			getkey();
			ScrCls();
			ucMax = 64;//共64个块
			ucMin = 0x00;
			ucRst = 22; 
			ucBlkNo = 22;
			ScrPrint(0,0,0," CurBlkNo:%d", ucBlkNo);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucBlkNo, &ucVal))
				ucBlkNo = ucVal;
			ScrCls();
			ScrPrint(0,4,0," SetBlkNo:%d", ucBlkNo);
			getkey();

			memset(aucUID,0,sizeof(aucUID));
			ucRet = M1ReadBlock(ucBlkNo, aucUID);
			ScrClrLine(2,6);
			ScrPrint(0,2,0,"Read Ret:%02X          ",ucRet);
			if (ucRet == 0) 
			{
				ScrPrint(0,3,0,"Val:%02X %02X %02X %02X",aucUID[0],aucUID[1],aucUID[2],aucUID[3]);
				ScrPrint(0,4,0,"    %02X %02X %02X %02X",aucUID[4],aucUID[5],aucUID[6],aucUID[7]);
				ScrPrint(0,5,0,"    %02X %02X %02X %02X",aucUID[8],aucUID[9],aucUID[10],aucUID[11]);
				ScrPrint(0,6,0,"    %02X %02X %02X %02X",aucUID[12],aucUID[13],aucUID[14],aucUID[15]);
				memcpy(aucValRd, aucUID, 16);
			}
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;
		
		case 6:
		case '6':
			ScrCls();
			ScrPrint(0,1,0," M1 Write ");
			
			getkey();
			ScrCls();
			ucMax = 64;//共64个块
			ucMin = 0x00;
			ucRst = 22; 
			ucBlkNo = 22;
			ScrPrint(0,0,0," CurBlkNo:%d", ucBlkNo);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucBlkNo, &ucVal))
				ucBlkNo = ucVal;
			ScrCls();
			ScrPrint(0,4,0," SetBlkNo:%d", ucBlkNo);
			getkey();

			memcpy(aucValWr,aucValRd,16);
			 
			aucValWr[0] += 1;
			aucValWr[1] += 1;
			aucValWr[4]=~aucValWr[0];
			aucValWr[5]=~aucValWr[1];
			aucValWr[6]=~aucValWr[2];
			aucValWr[7]=~aucValWr[3];
			memcpy(aucValWr+8, aucValWr, 4);
			aucValWr[12]=aucValWr[14]= ucBlkNo;
			aucValWr[13]=aucValWr[15]=~ucBlkNo;

			ucRet = M1WriteBlock(ucBlkNo, aucValWr);

			ScrClrLine(2,6);
			ScrPrint(0,2,0,"Write Ret:%02X         ",ucRet);
			if (ucRet == 0) 
			{
				ScrPrint(0,3,0,"Val:%02X %02X %02X %02X",aucValWr[0],aucValWr[1],aucValWr[2],aucValWr[3]);
				ScrPrint(0,4,0,"    %02X %02X %02X %02X",aucValWr[4],aucValWr[5],aucValWr[6],aucValWr[7]);
				ScrPrint(0,5,0,"    %02X %02X %02X %02X",aucValWr[8],aucValWr[9],aucValWr[10],aucValWr[11]);
				ScrPrint(0,6,0,"    %02X %02X %02X %02X",aucValWr[12],aucValWr[13],aucValWr[14],aucValWr[15]);
			}
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;
		
		case 7:
		case '7':
			ScrCls();
			ScrPrint(0,1,0," M1 Operate ");
			ScrPrint(0,2,0,"     Src Blk NO     ");
			
			getkey();
			ScrCls();
			ucMax = 64;//共64个块
			ucMin = 0x00;
			ucRst = 22; 
			ucBlkNo = 22;
			ScrPrint(0,0,0," CurBlkNo:%d", ucBlkNo);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucBlkNo, &ucVal))
				ucBlkNo = ucVal;
			ScrCls();
			ScrPrint(0,4,0," CurBlkNo:%d", ucBlkNo);
			getkey();

			ScrClrLine(2,3);
			ScrPrint(0,2,0,"    Update Blk NO     ");
			getkey();
			ScrCls();
			ucMax = 64;//共64个块
			ucMin = 0x00;
			ucRst = 22; 
			ucUpBlkNo = 22;
			ScrPrint(0,0,0," Update BlkNo:%d", ucUpBlkNo);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucUpBlkNo, &ucVal))
				ucUpBlkNo = ucVal;
			ScrCls();
			ScrPrint(0,4,0," Update BlkNo:%d", ucUpBlkNo);
			getkey();

			ScrClrLine(2,4);
			ScrPrint(0,3,0,"1 - Add  2 - Del");
			ScrPrint(0,4,0,"3 - Backup");
			ucTemp = getkey();
			memset(operateValue,0,sizeof(operateValue));
			if(ucTemp == RF_KEY_F1)
			{
				ucTemp = '+';			 
				operateValue[0] = 1;
				operateValue[1] = 1;
			}
			else if(ucTemp == RF_KEY_F2)
			{
				ucTemp = '-';
							 
				operateValue[0] = 1;
				operateValue[1] = 1;			
			}
			else if(ucTemp == RF_KEY_F3)
			{
				ucTemp = '>';
				operateValue[0] = 1;
				operateValue[1] = 1;
			}
			else
			{
				ScrCls();
				break;
			}

			ucRet = M1Operate(ucTemp, ucBlkNo, operateValue, ucUpBlkNo);
			ScrPrint(0,3,0,"Operate Ret:%02X        ",ucRet);
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;
						
		case 8:
		case '8':
			ScrCls();
			ScrPrint(0,1,0,"Remove Card");
			ScrPrint(0,3,0,"1-Halt  2-Remove");
			ucTemp = getkey();
			if(ucTemp == RF_KEY_F1)
				ucTemp = 'H';
			else
				ucTemp = 'R';
			ucRet = PiccRemove(ucTemp, 0);
			ScrPrint(0,4,0,"ret:%02X",ucRet);
			test_ProcErr(ucRet);
			getkey();
			ScrCls();					
			break;
		case 9:
		case '9':
			ScrCls();
			ScrPrint(0,1,0,"RATS CMD");
			ucRet = M1ToPro_RATS(&ucATSLen, ucATSData);
			//ScrPrint(0,2,0,"ret:%02X",ucRet);
			ScrPrint(0,3,0,"ATSLen:%02X",ucATSLen);
			test_ProcErr(ucRet);
			getkey();
			ScrCls();
			break;
		case 10:
		case 'A':
		case 'a':
			ScrCls();
			ScrPrint(0,0,0,"  Time Test");
			ScrPrint(0,1,0,"1-9.44s(1000*1000etu)");
			ScrPrint(0,2,0,"2-4.72s(1000*500 etu)");
			ScrPrint(0,3,0,"3-2.36s(1000*250 etu)");
			ScrPrint(0,4,0,"4-9.44ms(1000    etu)");
			ucRet = getkey();
			PiccOpen();
			if (ucRet == RF_KEY_F1) 
			{
				ScrCls();
				ScrPrint(0,0,0,"9.44s(1000*1000etu)");
				ScrPrint(0,7,0,"any key to start ");
				getkey();
				s_RFDelay10ETUs(1000*100); // 关闭载波1000ETU   
			}
			if (ucRet == RF_KEY_F2) 
			{
				ScrCls();
				ScrPrint(0,0,0,"4.72s(1000*500 etu)");
				ScrPrint(0,7,0,"any key to start ");
				getkey();
				s_RFDelay10ETUs(1000*50); // 关闭载波1000ETU    
			}
			if (ucRet == RF_KEY_F3) 
			{
				ScrCls();
				ScrPrint(0,0,0,"2.36s(1000*250 etu)");
				ScrPrint(0,7,0,"any key to start ");
				getkey();
				s_RFDelay10ETUs(1000*25); // 关闭载波1000ETU    
			}
			if (ucRet == RF_KEY_F4) 
			{
				ScrCls();
				ScrPrint(0,0,0,"9.44ms(1000    etu)");
				ScrPrint(0,1,0,"1-1   2-100");
				ScrPrint(0,2,0,"3-500  4-1000");
				ucRet = getkey();
				ulLong = 10;
				if (ucRet == RF_KEY_F1) 
				{
					ulLong = 1;
				}
				if (ucRet == RF_KEY_F2) 
				{
					ulLong = 100;
				}
				if (ucRet == RF_KEY_F3) 
				{
					ulLong = 500;
				}
				if (ucRet == RF_KEY_F4) 
				{
					ulLong = 1000;
				}

				ScrClrLine(1,7);
				ScrPrint(0,2,0," %d ms \n %d etu", 944*ulLong/100, 1000*ulLong);
				ScrPrint(0,5,0,"loop:%d ", ulLong);
				ScrPrint(0,7,0,"any key to start ");
				getkey();
			//	for (i = 0; i<ulLong; i++) 
				{
					s_RFDelay10ETUs(1000*ulLong/10); // 关闭载波1000ETU    
				}
			}
			Beep();			
			ScrPrint(0,7,0,"    Test End      ");
			PiccClose();
			getkey();
			break;
		default:
			ScrCls();
			break;
		}
    }					
}

void test_spi(void)
{
    uchar ucRet = 0;
    uchar ucTemp = 0x00;
	uchar ucKey = 0x00;
	uchar buf[300];
	uchar buftwo[600];
	uchar ucAddr = 0;
	ulong ulLen = 0;
	ulong ulLoop = 0;
	ulong ulOK = 0;
	ulong ulFail = 0;
	int iRet = 0;

	ScrCls();
	s_RF_Init(); // 初始化SPI控制
	while(1)
	{
	    ScrPrint(0,0,0x40, "Register R_W Test"); 
		ScrPrint(0,5,0,"F1 - W + R");
		ScrPrint(0,6,0,"F2 - R + W + R"); 

        ucKey = getkey();
		switch(ucKey)
		{  
		case RF_KEY_F1:
			ScrCls();
			//s_vRFReadReg(1, RF_REG_MODWIDTH, &ucTemp);
			s_vRFReadReg(1, PN512_MODWIDTH_REG, &ucTemp);
                        
			ulLoop = 0;
			ulOK = 0;
			ulFail = 0;
			if(ucTemp == 0x13)
			{ 
					ScrPrint(0,6,0," DATA OK[%02X] ",ucTemp);
			}
			else
			{
				ScrPrint(0,7,0," DATA FAIL[%02X] ",ucTemp);
				//Beep();
			}	
			getkey();
			ScrCls();
			while(1)
			{
				if(!kbhit())
				{
					if(getkey() == RF_KEY_F3)
						break;					
				}
				ulLoop++; 	 
				buf[0] = (uchar)(ulLoop & 0x0FF);//0x62;//
				s_vRFWriteReg(1, PN512_MODWIDTH_REG, buf);
				iRet = s_vRFReadReg(1, PN512_MODWIDTH_REG, buftwo);
				
				ScrPrint(0,0,0,"Loop [%X] ", ulLoop);						
				ScrPrint(0,3,0,"Dt: wr[%02X] rd[%02X] ", buf[0], buftwo[0]);

				if(iRet)
				{
					ScrPrint(0,6,0,"Read Err[%X]", ulLoop);
				}
				
				if(buf[0] == buftwo[0])
				{ 
					ulOK++;
					ScrPrint(0,4,0," DATA OK[%d ] ",ulOK);
//					DelayMs(1000);
				}
				else
				{
					ulFail++;
					ScrPrint(0,5,0," DATA FAIL[%d ] ",ulFail);
					//getkey();
					DelayMs(10);
					//DelayMs(1000);
					//Beep();
				}
			}

			ScrPrint(0,6,0," DATA OK[%d ] ",ulOK);
			ScrPrint(0,7,0," DATA FAIL[%d ] ",ulFail);
			getkey();
			ScrCls();
			break; 
		case RF_KEY_F2:

			ScrCls();
			s_vRFReadReg(1, PN512_MODWIDTH_REG, &ucTemp);
			ulLoop = 0;
			ulOK = 0;
			ulFail = 0;
			if(ucTemp == 0x13)
			{ 
					ScrPrint(0,6,0," DATA OK[%02X] ",ucTemp);
			}
			else
			{
				ScrPrint(0,7,0," DATA FAIL[%02X] ",ucTemp);
				//Beep();
			}	
			getkey();
			while(1)
			{
				if(!kbhit())
				{
					if(getkey() == RF_KEY_F3)
						break;					
				}
				ulLoop++; 
				s_vRFReadReg(1, 0x1A, buf);
				//buf[0] = buf[0]+1;//(uchar)ulLoop;
				buf[0] = (uchar)(ulLoop & 0x0FF);
				s_vRFWriteReg(1, PN512_MODWIDTH_REG, buf);
				s_vRFReadReg(1, PN512_MODWIDTH_REG, buftwo);
									
				ScrPrint(0,0,0,"Loop [%X] ", ulLoop);
				ScrPrint(0,3,0,"Dt: wr[%02X] rd[%02X] ", buf[0], buftwo[0]);
				if(buf[0] == buftwo[0])
				{ 
					ulOK++;
					ScrPrint(0,4,0," DATA OK[%d ] ",ulOK);
//					DelayMs(1000);
				}
				else
				{
					ulFail++;
					ScrPrint(0,5,0," DATA FAIL[%d ] ",ulFail);
					DelayMs(1000);
					//Beep();
				}
			}

			ScrPrint(0,6,0," DATA OK[%d ] ",ulOK);
			ScrPrint(0,7,0," DATA FAIL[%d ] ",ulFail);
			getkey();
			ScrCls();
			break; 
			default:
				buf[0] = 0x13;
				s_vRFWriteReg(1, PN512_MODWIDTH_REG, buf);
			  return;
		}
	}

}
void test_WUPATest(void)
{
	uchar ucKey = 0x00;	
    uchar ucRet = 0;
	uchar ucResp[100];
	uchar ucTemp = 0;
	ulong ulOK = 0;
	ulong ulFail =0;
 
	{
		ScrCls();
		ScrPrint(0,0,0," WUPA LOOP TEST ");
		ScrPrint(0,1,0," [F3] to exit ");
		PiccOpen(); 
		while (1)
		{
			if ((!kbhit()) && (getkey() == RF_KEY_F3))
				break;
			
			memset(ucResp, 0, sizeof(ucResp));
			*ulCnt2 = 1;
			s_RFResetCarrier();
			ucRet = s_RF_ucWUPA(ucResp);
			ulCnt3++;
			ScrPrint(0,3,0," loop cnt: %d ", ulCnt3);	 
			if(ucRet == RET_RF_OK)
			{
				ulOK++;
//				Beep();
				ScrPrint(0,4,0," Success[%d]     ",ulOK);
//				DelayMs(100);				
			}
			else
			{
				ulFail++;
				ScrPrint(0,5,0," Fail[%d]: %02X ",ulFail, ucRet);	
//				DelayMs(100);
			}	
		}
		PiccClose();
	}
	ScrCls();
}
void test_inter_func(void)
{
    uchar ucRet = 0;
	uchar ucResp[100];
	uchar aucUID[20];
	uchar ucSAK = 0;
	uchar ucATSLen = 0;
	uchar ucATSData[100];

 	uchar ucKey = 0x00;
	uchar ucTemp = 0;
	uchar ucMode = 0;
	uchar ucComOk;

	ScrCls();
	if( PiccOpen() )
	{
		ScrPrint(0,5,0,"PiccOpen Err");
		getkey();
		return;
	}
	s_RFOpenCarrier();

	ucComOk = 1;
	if(PortOpen(COM1, "115200,8,N,1"))
	{
		ScrPrint(0,4,0,"Com Open Err!");
		getkey();
		ucComOk = 0;
	}

	ScrCls();
	while(1)
	{
	     ScrPrint(0,0,0x40," Inter Fun Test ");		 
		 ScrPrint(0,2,0,"F1-'A' F2-'B'");
		 ScrPrint(0,3,0,"F3-'M' F4-Quit ");
		 ucKey = getkey();
		 if (ucKey == RF_KEY_F1) 
		 {
			 ucMode = 'A';
		 }
		 else if (ucKey == RF_KEY_F2)
			 ucMode = 'B';
		 else if (ucKey == RF_KEY_F3)
		 {
			 ScrCls();
			 break;
		 }
		 else
			 ucMode = 'A';


		while(1)
		{
		 	 ScrCls();
			 ScrPrint(0,0,0x40," Inter Fun Test ");	
			 if (ucMode == 'A') 
			 {
				 ScrPrint(0,2,0,"F1-WUPA   F2-Anticoll");
				 ScrPrint(0,3,0,"F3-Select F4-Quit");
			// ScrPrint(0,4,0,"7-HLTA     ");
			 }
			 else
			 {
				 ScrPrint(0,5,0,"F1-WUPB F2-AttribB");
				 ScrPrint(0,6,0,"F3-HLTB F4-Quit  ");	
			 }

			 gl_usRecLen = 0;
			 memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					 
			 ucKey = getkey();
			 if (ucKey == RF_KEY_F4) 
			 {
				 break;
			 }
			 switch(ucKey)
			 { 				
				case RF_KEY_F1:
					if (ucMode == 'A') 
					{
						ScrCls();				
						memset(ucResp, 0, sizeof(ucResp));
						ucRet = s_RF_ucWUPA(ucResp);
						//ucRet = s_RF_ucWUPB(&ucTemp,ucResp);
						ScrPrint(0,1,0,"Ret:%02x,ErrNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						ScrPrint(0,2,0,"Dat: %02x %02x",ucResp[0], ucResp[1]);
						if(ucComOk)
						test_SendDataByCom();
						getkey();
					}
					else
					{
						ucTemp = 0;
						memset(ucResp, 0, sizeof(ucResp));
						ucRet = s_RF_ucWUPB(&ucTemp, ucResp);
						ScrCls();
						ScrPrint(0,2,0,"WUPB Ret:%02x,ErrNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						if(ucTemp)
						{
							ScrPrint(0,4,0,"len:%d",ucTemp);
							
							ScrPrint(0,5,0,"ATQB: %02X %02X %02X %02X %02X",ucResp[0],ucResp[1],
								ucResp[2],ucResp[3],ucResp[4]);
							ScrPrint(0,6,0," %02X %02X %02X %02X %02X %02X",ucResp[5],ucResp[6],
								ucResp[7],ucResp[8],ucResp[9],ucResp[10]);					
							ScrPrint(0,7,0,"%02X %02X %02X",ucResp[11],ucResp[12],ucResp[13]);
						}				
						getkey();
					}
					ScrCls();
					break;
					

				case RF_KEY_F2:
					if (ucMode == 'A') 
					{
						ScrCls();
						memset(aucUID, 0, sizeof(aucUID));
						ucRet = s_RF_ucAnticoll(PICC_ANTICOLL1, aucUID);
						ScrCls();
						ScrPrint(0,2,0,"Anticoll Ret:%02X,errNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						ScrPrint(0,4,0,"UID:%02X %02X %02X %02X %02X",aucUID[0],aucUID[1],aucUID[2],aucUID[3], aucUID[4]);
						if(ucComOk)
						test_SendDataByCom();
						getkey();
					}
					else
					{
						memset(ucResp, 0, sizeof(ucResp));
						ucRet = s_RF_ucAttrib(ucResp);
						ScrCls();
						ScrPrint(0,2,0,"Attrib Ret:%02x,ErrNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						//if(ucTemp)
						{
							//ScrPrint(0,4,0,"len:%d",ucTemp);
							
							ScrPrint(0,5,0,"Attrib: %02X %02X %02X %02X %02X",ucResp[0],ucResp[1],
								ucResp[2],ucResp[3],ucResp[4]);
						}			
						getkey();	
					}
					ScrCls();
					break;

				case RF_KEY_F3:
					ScrCls();
					if (ucMode == 'A') 
					{
						ucSAK = 0;
						ucRet = s_RF_ucSelect(PICC_ANTICOLL1, aucUID, &ucSAK);
						ScrPrint(0,2,0,"Select1Ret:%02X,errNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						ScrPrint(0,4,0,"SAK: %02X",ucSAK);
						if(ucComOk)
						test_SendDataByCom();
						getkey();
					}
					else
					{			
						memset(ucResp, 0, sizeof(ucResp));
						ucRet = s_RF_ucHALTB(ucResp);
						ScrPrint(0,1,0,"Ret:%02x,ErrNo:%02x",ucRet,RF_WorkInfo.usErrNo);
						ScrPrint(0,2,0,"HALTB: %02x",ucResp[0]);
						getkey();
					}
					ScrCls();
					break;
					 
				default: 
					ScrCls();		
					break;  
			}
		}
	}
	PiccClose();
}
void test_M1_OperTest(void)
{
 	uchar ucRet, uci, ucMode;
	uchar SerialNo[15], BlkValue[17], Pwd[7], BlkWrVal[17], BlkRdVal[17], MoneyValueAdd[4];
    uint  BlkNo=6;
    uint UpdateBlkNo = 6;
	uchar ucTemp = 0;

	while(1)
	{
		ScrCls();
		if (!kbhit() && getkey() == RF_KEY_F3) 
		{
			return;
		}
		ScrPrint(0, 0, 1, " Read Block 6 ");

	//	getkey();

		BlkNo=6;
		memset(MoneyValueAdd, 0x00, sizeof(MoneyValueAdd));
		memset(BlkValue, 0x00, sizeof(BlkValue));
		memset(BlkWrVal, 0x00, sizeof(BlkWrVal));
		memset(BlkRdVal, 0x00, sizeof(BlkRdVal));
		memset(SerialNo, 0x00, sizeof(SerialNo));
		memset(Pwd, 0x00, sizeof(Pwd));
		memcpy(Pwd, "\xff\xff\xff\xff\xff\xff", 6);

		
		PiccOpen();
  		ucRet = PiccDetect('M', &ucTemp, SerialNo, NULL, NULL);
		DelayMs(500);
		if(ucRet)
		{
		   ScrPrint(0,7,0,"Det Err: %02X ", ucRet);
		  // getkey();
		   PiccClose();
		   continue;	   
		}
		ScrPrint(0,7,0,"Det OK ");
		ucRet = M1Authority('A', (uchar)BlkNo, Pwd, SerialNo+1);
		DelayMs(500);
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
		 //  getkey();
		   PiccClose();
		   continue;	  
		}
		ScrPrint(0, 6, 0, "Authority(A,%2d)=%02d",BlkNo, ucRet); 
 //		getkey();
 		{
			ucRet = M1ReadBlock((uchar)BlkNo, BlkValue);
		DelayMs(500);
		//	ScrCls();
			ScrPrint(0, 7, 0, "M1ReadBlock(%2d)=%02d",BlkNo, ucRet);
		/*	ScrFontSet(0);
			ScrGotoxy(0, 1);
			for(uci=0; uci<16; uci++)
			{
      			Lcdprintf("%02x ", BlkValue[uci]);
			}
			getkey();*/
		}
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1ReadBlock Err: %02X ", ucRet);
		   //getkey();
		   PiccClose();
		   continue;	  
		}

		ScrCls();
		ScrPrint(0, 0, 1, " Write Block 6 "); 
	//	getkey();
		memcpy(BlkWrVal, BlkValue, 16);
		BlkWrVal[0] += 1;
		BlkWrVal[1] += 1;
		BlkWrVal[4]=~BlkWrVal[0];
		BlkWrVal[5]=~BlkWrVal[1];
		memcpy(BlkWrVal+8, BlkWrVal, 4);
		BlkWrVal[12]=BlkWrVal[14]= BlkNo;
		BlkWrVal[13]=BlkWrVal[15]=~BlkNo;
    
		MoneyValueAdd[0] = 1;
		MoneyValueAdd[1] = 1;
		
	/*	ScrPrint(0,2,0,"F1-KEYA ");
		ucRet = getkey();
		if (ucRet == RF_KEY_F1) */
		{
			ucMode = 'A';
		}
	/*	else
			ucMode = 'B';
		ucRet = M1Authority(ucMode, (uchar)BlkNo, Pwd, SerialNo+1);
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
//		   getkey();
		   PiccClose();
		   continue;
		}
		ScrPrint(0, 6, 0, "Authority(%c,%2d)=%02d",ucMode,BlkNo, ucRet);*/
 //		getkey();
 		{
			//ucRet = M1WriteBlock((uchar)BlkNo, BlkWrVal);
			ucRet = M1Operate('+', (uchar)BlkNo, MoneyValueAdd, (uchar)(UpdateBlkNo));
		DelayMs(500);
		//	ScrCls();
			ScrPrint(0, 7, 0, "M1Operate(%2d)=%02d",BlkNo, ucRet);
		/*	ScrFontSet(0);
			ScrGotoxy(0, 1);
			for(uci=0; uci<16; uci++)
			{
      			Lcdprintf("%02x ", BlkWrVal[uci]);
			}
			getkey();*/
		}
		
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1Operate Err: %02X ", ucRet);
		 //  getkey();
		   PiccClose();
		   continue;	  
	//	   return;
		}
			
//		Beep();
	/*	ScrCls();
		ScrPrint(0,1,0,"Pls Tap Card ");
		getkey();
  		ucRet = PiccDetect('M', &ucTemp, SerialNo, NULL, NULL);
		if(ucRet)
		{
		   ScrPrint(0,7,0,"Det Err: %02X ", ucRet);
		   getkey();
		   return;
		}
		ucRet = M1Authority('A', (uchar)BlkNo, Pwd, SerialNo+1);
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
		   getkey();
		   return;
		}
		ScrPrint(0, 6, 0, "Authority(A,%2d)=%02d",BlkNo, ucRet);
		*/
		ScrCls();
		ScrPrint(0,1,0,"Read and Cmp ");
 //		getkey();
 		{
			ucRet = M1ReadBlock((uchar)BlkNo, BlkRdVal);
		DelayMs(500);
		//	ScrCls();
			ScrPrint(0, 7, 0, "M1ReadBlock 2 (%2d)=%02d",BlkNo, ucRet);
		/*	ScrFontSet(0);
			ScrGotoxy(0, 1);
			for(uci=0; uci<16; uci++)
			{
      			Lcdprintf("%02x ", BlkRdVal[uci]);
			}
			getkey();*/
		}
		if(ucRet)
		{
		   ScrPrint(0,7,0,"M1ReadBlock 2 Err: %02X ", ucRet);
		 //  getkey();
		   PiccClose();
		   continue;	  
		   
		}
		if(memcmp(BlkRdVal,BlkWrVal,16)==0)
		{
		   ScrPrint(0,7,0,"Val OK ");
		  // getkey();
		}
		else
		{
		   ScrPrint(0,7,0,"Val Err ");
		 //  getkey();
		}
	/*	if(ucRet == 0)
		{	
			ScrCls();
			ScrPrint(0, 0, 0, "M1Read & Write Val");
			ScrFontSet(0);
			ScrGotoxy(1, 1);
			for(uci=0; uci<16; uci++)
			{
      			Lcdprintf("%02x ", BlkRdVal[uci]);
			}
			getkey();
			for(uci=0; uci<16; uci++)
			{
      			Lcdprintf("%02x ", BlkValue[uci]);
			}
			getkey();
		}*/
		
//		Beep();
		DelayMs(500);
		PiccClose();
		continue;	  
   }
}
void test_M1Test(void)
{
 	uchar ucRet, uci, ucMode;
	uchar SerialNo[15], BlkValue[17], Pwd[7], BlkWrVal[17], BlkRdVal[17];
    uint  BlkNo=6;
	uchar ucTemp = 0;

	ScrCls();
	ScrPrint(0, 0, 1, " Read Block 6 ");
	getkey();

	BlkNo=6;
	memset(BlkValue, 0x00, sizeof(BlkValue));
	memset(BlkWrVal, 0x00, sizeof(BlkWrVal));
	memset(BlkRdVal, 0x00, sizeof(BlkRdVal));
	memset(SerialNo, 0x00, sizeof(SerialNo));
	memset(Pwd, 0x00, sizeof(Pwd));
	memcpy(Pwd, "\xff\xff\xff\xff\xff\xff", 6);

	
	PiccOpen();
  	ucRet = PiccDetect('M', &ucTemp, SerialNo, NULL, NULL);
	if(ucRet)
	{
	   ScrPrint(0,7,0,"Det Err: %02X ", ucRet);
	   getkey();
	   return;
	}
	ScrPrint(0,2,0,"F1-KEYA ");
	ucRet = getkey();
	if (ucRet == RF_KEY_F1) 
	{
		ucMode = 'A';
	}
	else
		ucMode = 'B';
	
	ucRet = M1Authority(ucMode, (uchar)BlkNo, Pwd, SerialNo+1);
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
	   getkey();
	   return;
	}
	ScrPrint(0, 6, 0, "Authority(%c,%2d)=%02d",ucMode,BlkNo, ucRet);
 	getkey();
 	{
		ucRet = M1ReadBlock((uchar)BlkNo, BlkValue);
		ScrCls();
		ScrPrint(0, 7, 0, "M1ReadBlock(%2d)=%02d",BlkNo, ucRet);
		ScrFontSet(0);
		ScrGotoxy(0, 1);
		for(uci=0; uci<16; uci++)
		{
      		Lcdprintf("%02x ", BlkValue[uci]);
		}
		getkey();
	}
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1ReadBlock Err: %02X ", ucRet);
	   getkey();
	   return;
	}

	ScrCls();
	ScrPrint(0, 0, 1, " Write Block 6 ");	
	getkey();
	memcpy(BlkWrVal, BlkValue, 16);
	BlkWrVal[0] += 1;
	BlkWrVal[1] += 1;
    BlkWrVal[4]=~BlkWrVal[0];
    BlkWrVal[5]=~BlkWrVal[1];
    memcpy(BlkWrVal+8, BlkWrVal, 4);
    BlkWrVal[12]=BlkWrVal[14]= BlkNo;
    BlkWrVal[13]=BlkWrVal[15]=~BlkNo;
	 
	ScrPrint(0,2,0,"F1-KEYA ");
	ucRet = getkey();
	if (ucRet == RF_KEY_F1) 
	{
		ucMode = 'A';
	}
	else
		ucMode = 'B';
	
	ucRet = M1Authority(ucMode, (uchar)BlkNo, Pwd, SerialNo+1);
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
	   getkey();
	   return;
	}
	ScrPrint(0, 6, 0, "Authority2(%c,%2d)=%02d",ucMode,BlkNo, ucRet);
 	getkey();
	
 	{
		ucRet = M1WriteBlock((uchar)BlkNo, BlkWrVal);
		ScrCls();
		ScrPrint(0, 7, 0, "M1WriteBlock(%2d)=%02d",BlkNo, ucRet);
		ScrFontSet(0);
		ScrGotoxy(0, 1);
		for(uci=0; uci<16; uci++)
		{
      		Lcdprintf("%02x ", BlkWrVal[uci]);
		}
		getkey();
	}
	
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1WriteBlock Err: %02X ", ucRet);
	   getkey();
//	   return;
	}
		
	ScrCls();
	ScrPrint(0,1,0,"Pls Tap Card ");
	getkey();
  	ucRet = PiccDetect('M', &ucTemp, SerialNo, NULL, NULL);
	if(ucRet)
	{
	   ScrPrint(0,7,0,"Det Err: %02X ", ucRet);
	   getkey();
	   return;
	}

	ScrPrint(0,2,0,"F1-KEYA ");
	ucRet = getkey();
	if (ucRet == RF_KEY_F1) 
	{
		ucMode = 'A';
	}
	else
		ucMode = 'B';
	
	ucRet = M1Authority(ucMode, (uchar)BlkNo, Pwd, SerialNo+1);
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1Auth Err: %02X ", ucRet);
	   getkey();
	   return;
	}
	ScrPrint(0, 6, 0, "Authority(%c,%2d)=%02d",ucMode,BlkNo, ucRet);
	
	ScrCls();
	ScrPrint(0,1,0,"Read and Cmp ");
 	getkey();
 	{
		ucRet = M1ReadBlock((uchar)BlkNo, BlkRdVal);
		ScrCls();
		ScrPrint(0, 7, 0, "M1ReadBlock 2 (%2d)=%02d",BlkNo, ucRet);
		ScrFontSet(0);
		ScrGotoxy(0, 1);
		for(uci=0; uci<16; uci++)
		{
      		Lcdprintf("%02x ", BlkRdVal[uci]);
		}
		getkey();
	}
	if(ucRet)
	{
	   ScrPrint(0,7,0,"M1ReadBlock 2 Err: %02X ", ucRet);
	   getkey();
	   
	}
	if(memcmp(BlkRdVal,BlkWrVal,16)==0)
	{
	   ScrPrint(0,7,0,"Val OK ");
	   getkey();
	}
	else
	{
	   ScrPrint(0,7,0,"Val Err ");
	   getkey();
	}
	if(ucRet == 0)
	{	
		ScrCls();
		ScrPrint(0, 0, 0, "M1Read & Write Val");
		ScrFontSet(0);
		ScrGotoxy(1, 1);
		for(uci=0; uci<16; uci++)
		{
      		Lcdprintf("%02x ", BlkRdVal[uci]);
		}
		getkey();
		for(uci=0; uci<16; uci++)
		{
      		Lcdprintf("%02x ", BlkValue[uci]);
		}
		getkey();
	}
	
	PiccClose();
}

void appRemovePicc(void)
{
	uchar ucKey = 0;
	uchar ucRet = 0;
	ulong i = 0;
	
	Beep();
	PiccRemove_EMV(0, 0);
}
void test_LoopBackTest(void)
{
	uchar ucKey = 0x00;	
    uchar ucRet = 0;
	uchar aucOut[300];
	uchar Other[300];
	uchar ucLoopDetFlg = 0;
	uchar i = 0;
	uchar ucMode = 1;
	uchar ucRemove = 0;
	uchar ucClose = 0;
	uchar ucLc = 0;
	uchar aucDc[300];
	ushort usLe = 0;	
	APDU_SEND apdu_s;
	APDU_RESP apdu_r;
	uint uiLenIn = 0;
	uint uiLenOut = 0;
	
	gulLoopCnt = 0;
			
	ucMode = 1;	// EMV MODE
	//ucMode = 0; // PAYPASS MODE
	ScrCls();
	//ucRet = PortOpen(COM1, "115200,8,N,1");
	
	while(1)
	{
		PiccOpen();
		// 20081027 for analogue & Digital test
		memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
		gl_usRecLen = 0;

		
		test_ParaSetup();
		PiccLight(PICC_LED_RED|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
		PiccLight(PICC_LED_GREEN, 1);
		
		ScrPrint(0,0,0, " Loop Back Test[%d] ",gulLoopCnt);
		ScrPrint(0,1,0, "[F3] to exit ");
		while (2) 
		{
			ScrClrLine(2,7);
			ScrPrint(0,4,0x01, "Present Card >>>");
			memset(aucOut, 0, sizeof(aucOut));

			if ((!kbhit()) && (getkey() == RF_KEY_F3))
			{	
				PiccClose();
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
				gl_usRecLen = 0;
				return;
			}
			
			//ucRet = PiccDetect(1,&ucKey,aucOut,NULL,Other);
			ucRet = PiccDetect(ucMode,&ucKey,aucOut,NULL,Other);
			if(ucRet == RET_RF_OK)
			{
				//ScrPrint(0,5,0,"Card Detected "); 
				//getkey();
				break;
			}	
			else if (ucRet == 4) 
			{
				ScrPrint(0,4,1," Slt One Card Pls ");
				
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
				gl_usRecLen = 0;
//				DelayMs(500);
			}
			else if (ucRet != 3)  // != NO_PICCARD
			{
				ScrPrint(0,4,1," Det Err[%02X] ", ucRet);			
//				DelayMs(500);	

				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
				gl_usRecLen = 0;
			}
			else
			{			
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
				gl_usRecLen = 0;
			}
		}

		uiLenIn = 0;
		memset(aucDc, 0, sizeof(aucDc));
		memcpy(aucDc, "\x00\xA4\x04\x00", 4);
		uiLenIn += 4;
		aucDc[uiLenIn++] = 0x0E;
		memcpy(aucDc+uiLenIn, "2PAY.SYS.DDF01", 0x0E);
		uiLenIn += 0x0E;
		aucDc[uiLenIn++] = 0;
		while(3)
		{
			ScrClrLine(2,7);
		 	
			if ((!kbhit()) && (getkey() == RF_KEY_F3))
			{
				ScrPrint(0,7,0," Test Canceled ");
				
				PiccClose();
				
				return;
			}
								
			// isocmd
			gulLoopCnt++;
			memset(&apdu_r, 0, sizeof(apdu_r));
			

			ScrPrint(0,4,0x01, "APDU Exchange...");
			ucRet = PiccCmdExchange(uiLenIn, aucDc, &uiLenOut, apdu_r.DataOut);//*/
			
			
			if(ucRet != RET_RF_OK)
			{
				PiccLight(PICC_LED_GREEN|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
				PiccLight(PICC_LED_RED, 1);
				ScrPrint(0, 4, 0," APDU Err ");
				ScrPrint(0,6,0,"Remove Card ");
				goto LOOPBACK_END_2;
			}	
			
			if (apdu_r.DataOut[1] == 0x70) 
			{
				PiccLight(PICC_LED_RED|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
				PiccLight(PICC_LED_GREEN, 1);
				//Beep();
				ScrPrint(0, 4, 0," R-APDU = 0x70 ");
				ScrPrint(0,6,0,"Remove Card ");
				goto LOOPBACK_END;
			}
			
			if (uiLenOut < 6) 
			{
				PiccLight(PICC_LED_GREEN|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
				PiccLight(PICC_LED_RED, 1);
				ScrPrint(0, 4, 0," Rsp Len < 6 ");
				ScrPrint(0,6,0,"Remove Card ");
				goto LOOPBACK_END_2;
			}	
					
			memset(aucDc, 0, sizeof(aucDc));
			uiLenIn = uiLenOut-2;
			memcpy(aucDc, apdu_r.DataOut, uiLenIn);	
//			DelayMs(6);
		}
LOOPBACK_END:
		//AT_EMV
		//Beep(); 

		PiccRemove_EMV(0, 0);
		//PiccRemove_PayPass(0,0);

LOOPBACK_END_2: 
		
		PiccClose();
#ifdef RF_L1_RECDBG 
		if(gl_usRecLen)
		{
			ScrClrLine(2,7);
			ScrPrint(0,4,0,"Send Data");
			ucRet = PortOpen(COM1, "115200,8,N,1");
			if(ucRet)
			{
				ScrCls();
				ScrPrint(0,0,1,"Com Open Err:%02X ", ucRet);
				getkey();			
			}
			else
			{
				test_SendDataByCom();	

			}
			#if(TERM_TYPE!=TERM_R30 || PRINT_UART_LCD !=1)
			PortClose(COM1);
			#endif
			
		}
#endif
		DelayMs(6);
		ScrClrLine(2,7);
		
	/*del 20081208 for loop 
		ScrPrint(0,4,0,"Loop Back Test End");
		getkey();
		ScrCls();
		return;//*/
	}
}
void test_LoopDetTest(void)
{
	uchar ucKey = 0x00;	
    uchar ucRet = 0; 
	uchar ucType = 0;
	uchar aucOut[100]; 
	uchar ucMode = 0;
	uchar ucTemp = 0;
		 

	ScrCls();
	ScrBackLight(2);

	 ScrPrint(0, 0, 0, " Det A/B Test ");
	 ScrPrint(0, 2, 0, "F1 - Det A   ");	
	 ScrPrint(0, 3, 0, "F2 - Det B   ");
	 ScrPrint(0, 4, 0, "Other - Auto Det ");
	 ucKey = getkey();
	 if (ucKey == RF_KEY_F1) 
	 {
		 ucMode = 'A';
	 }
	 else if(ucKey == RF_KEY_F2)
		 ucMode = 'B';
	 else
		 ucMode = 0;
	 
	 ScrCls();
	 
	gulLoopCnt = 0;
	gulOpenSucCnt = 0;
	gulOpenFailCnt = 0;
	gulOpenFailCnt = 0;
	gulDetectFailCnt = 0;
	gulIsoCmdSucCnt = 0;
	gulIsoCmdFailCnt = 0;
	
	ucRet = PiccOpen();	
	if(ucRet != RET_RF_OK)
	{ 		
		ScrPrint(0,7,0, "Op Err:%02X \n", ucRet);
		DelayMs(1000); 
		PiccClose(); 
		return;
	}
		
	ScrCls();
	if (ucMode == 0) 
	{
		ScrPrint(0,0,0, " DET A/B TEST   ");
	}
	else
		ScrPrint(0,0,0, " DET %c TEST   ", ucMode);

	ucRet = PiccOpen();
	test_ParaSetup();
	while(1)
	{	 
		ScrPrint(0,1,0, " Press F3 to Quit ");
		ScrPrint(0,4,0, " Pls Tap Card >>> ");
		
		if (!kbhit() && getkey() == RF_KEY_F3) 
		{
			break;
		}

		ucRet = PiccDetect(ucMode,&ucType,NULL,NULL,NULL);
		
		ScrClrLine(2,7);
		if(ucRet == RET_RF_OK)
		{
    		ScrPrint(0,6,0, " Type %c OK:%02X  ", ucType,ucRet);
		}	
		DelayMs(5);
	}	
	PiccClose(); 

}


void MifreTest(void)
{
	uchar ucRet = 0;
	uint OpenNum=0;

	ScrCls();
	kbmute(0);
	
	while(1)
	{
		ucRet = PiccOpen();
		if (ucRet) 
		{
			ScrCls();
			ScrPrint(0,3,0,"Open Err: %02X,Num:%d ", ucRet,OpenNum++);
			//getkey();
		}
		else
			break;
	}
	
	PiccSetup('r',&my_c_para);
	PiccClose();

	while (1) 
	{
		kbflush();
		ScrPrint(0,0,0x81, " Mifre Test ");
		ScrPrint(0,3,0,"1-API ");
		ScrPrint(0,4,0,"2-Loop Test"); 
		
		ucRet = getkey();
		switch(ucRet) 
		{
		case RF_KEY_F1:
			ScrCls(); 
			{
				test_api();
			} 
			ScrCls();
			break;
		case RF_KEY_F2:
			ScrCls();
			test_LoopABTest();	
			ScrCls();
			break;
		case RF_KEY_F3: 
			ScrCls(); 
			break;	
		case RF_KEY_F4: 
			ScrCls();
			break;
		default:
			break;
		}
	}

}

void test_SendDataByCom(void)
{
	uchar ucRet = 0;
	
	PortReset(COM1);
	ucRet = PortSends(COM1, gl_aucRecData, gl_usRecLen);
	if(ucRet)
	{
		ScrPrint(0,6,0,"Com Send Err:%02X ", ucRet);
		getkey();
	}
	memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
	gl_usRecLen = 0;
	
}
void test_Analogue(void)
{
	uchar ucLoopCnt = 0;
    uchar ucRet = RET_RF_OK;
	uchar aucResp[300];
	uchar ucLen = 0;

 	uchar ucKey = 0x00;
	uchar ucTemp = 0;
	APDU_SEND  ApduSend;
	APDU_RESP  ApduRecv;
	ulong i = 0;
	uchar ucCarrierOn = 0;
	uchar ucComOk = 0;
	uchar ucPollOk = 0;
	uchar ucType = 0;
	ulong ul100Ms = 0;
	
	
	uchar rc, curPage, curLine, totalPage, totalNum,ucMaxLine, ucMaxItem;
	int No = 0;
	char *acStrAnalogueMenu[]=
	{
		"1-Carrier On/Off",
		"2-Reset",
		"3-WUPA",
		"4-WUPB",
		"5-Polling",
		"6-WUPA->RATS",
		"7-WUPB->Attrib",
		"8-Removal",
		"9-Foruser"
	};

    ucMaxItem = 9;//8;
 	curLine = 0;
	curPage = 0;
	ucMaxLine = 4;
	totalPage = ucMaxItem / ucMaxLine;
	if (ucMaxItem % ucMaxLine) 
	{
		totalPage++;
	}
			
	ScrCls();	 

	PiccOpen();
	test_ParaSetup();
	ucCarrierOn = 0;
	ucComOk = 1;
	if(PortOpen(COM1, "115200,8,N,1"))
	{
		ScrPrint(0,4,0,"Com Open Err!");
		getkey();
		ucComOk = 0;
	}
	

	gl_usRecLen = 0;
	memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
	
	while (1)
	{
		#if (TERM_TYPE == TERM_R30 ||PRINT_UART_LCD ==1)
		ScrPrint(0,0,0,"Select Function:");
		ScrPrint(0,0,0,"0-Carrier On/Off ,1-Reset,2-WUPA,3-WUPB");
		ScrPrint(0,0,0,"4-Polling ,5-WUPA->RATS,6-WUPB->Attrib,7-Removal,8-Foruser");
		ScrPrint(0,0,0,"E/e-Exit");
		No=getkey();
		#else
		No = GetMenuNo(&curPage, &curLine, ucMaxItem, acStrAnalogueMenu);
	
		if (No == -1) 
		{
			ScrCls();
			return;
		}
		#endif
		
		gl_usRecLen = 0;
		memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
		
		switch(No) 
		{
		case -1:
		case 'E':
		case 'e':
			ScrCls();
			PiccClose();
			#if(TERM_TYPE!=TERM_R30 || PRINT_UART_LCD !=1)
			//PortClose(COM1);//此处不可关闭串口,否则无法返回主菜单
			#endif
			return;
		case 0: //Carrier On/Off
		case '0': //Carrier On/Off
			ScrCls();
			ScrPrint(0,0,0,"Carrier Control");
			ScrPrint(0,2,0,"F1-On  F2-Off");
			ScrPrint(0,4,0,"Other-Quit");
			ucRet = getkey();
			if(ucRet == RF_KEY_F1)
			{
				s_RFOpenCarrier();
				ucCarrierOn = 1;
				ScrPrint(0,6,0,"Carrier Opened!");
			}			
			else if(ucRet == RF_KEY_F2)
			{
				s_RFCloseCarrier();
				ucCarrierOn = 0;
				ScrPrint(0,6,0,"Carrier Closed!");
			}	
			else
			{
				ScrPrint(0,6,0,"No Change!");
			}
			
			getkey();
			ScrCls();
			break;
		case 1: // Reset
		case '1': 
			ScrCls();
			ScrPrint(0,0,0,"Reset Carrier");
			//
			ScrPrint(0,2,0,"F1-Reset");
			ScrPrint(0,4,0,"Other-Quit");
			ucRet = getkey();
			if(ucRet == RF_KEY_F1)
			{
				s_RFResetCarrier();
				ucCarrierOn = 1;
				ScrPrint(0,6,0,"Carrier Reset!");
			}	
			else
			{
				ScrPrint(0,6,0,"No Change!");
			}  
			getkey();
			ScrCls();
			break;
		case 2: // WUPA
		case '2':
			ScrCls();
			
			ScrPrint(0,0,0,"WUPA");
			if(ucCarrierOn == 0)
			{
				ScrPrint(0,6,0, "Pls Open Carrier!");
				getkey();
				ScrCls();
				break;
			}
			
			ScrPrint(0,2,0,"F1-Single Test");
			ScrPrint(0,3,0,"F2-Loop Test");
			ScrPrint(0,4,0,"Other-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)
			{
				ucRet = s_RF_ucWUPA(aucResp);
				test_ProcErr(ucRet);
				if(ucComOk)
					test_SendDataByCom();
			}
			else if(ucKey == RF_KEY_F2)
			{
				ScrCls();
				ScrPrint(0,0,0,"  WUPA LOOP ");
				ScrPrint(0,1,0," [F3] to Quit ");
				i = 0;
				while(1)
				{
					ScrPrint(0,2,0,"Loop Cnt:%d ", i);
					
					if(!kbhit() && getkey() == RF_KEY_F3)
						break;
					gl_usRecLen = 0;
					memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					ucRet = s_RF_ucWUPA(aucResp);
					test_ProcErr(ucRet);
					i++;
					if(ucComOk)
						test_SendDataByCom();
					DelayMs(5);					
				}
				gl_usRecLen = 0;
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
			}
			else
			{
				ScrPrint(0,6,0," No Test ");
			}
			getkey();
			ScrCls();
		
			break;
		case 3:// WUPB
		case '3':
			ScrCls();
			
			ScrPrint(0,0,0,"WUPB");
			if(ucCarrierOn == 0)
			{
				ScrPrint(0,6,0, "Pls Open Carrier!");
				getkey();
				ScrCls();
				break;
			}
			
			ScrPrint(0,2,0,"F1-Single Test");
			ScrPrint(0,3,0,"F2-Loop Test");
			ScrPrint(0,4,0,"Other-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)
			{
				ucRet = s_RF_ucWUPB(&ucLen, aucResp);
				test_ProcErr(ucRet);
				if(ucComOk)
					test_SendDataByCom();
			}
			else if(ucKey == RF_KEY_F2)
			{
				ScrCls();
				ScrPrint(0,0,0,"  WUPB LOOP ");
				ScrPrint(0,1,0," [F3] to Quit ");
				i = 0;
				while(1)
				{
					ScrPrint(0,2,0,"Loop Cnt:%d ", i);
					
					if(!kbhit() && getkey() == RF_KEY_F3)
						break;
					gl_usRecLen = 0;
					memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					ucRet = s_RF_ucWUPB(&ucLen, aucResp);
					i++;
					test_ProcErr(ucRet);
					if(ucComOk)
						test_SendDataByCom();
					DelayMs(5);					
				}
				gl_usRecLen = 0;
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));				
			}
			else
			{
				ScrPrint(0,6,0," No Test ");
			}
			getkey();
			ScrCls();
		
			break;
		case 4: // Polling
		case '4':
			ScrCls();
			ScrPrint(0,0,0," Polling ");
			if(ucCarrierOn == 0)
			{
				ScrPrint(0,6,0, "Pls Open Carrier!");
				getkey();
				ScrCls();
				break;
			}
			ScrPrint(0,2,0,"F1-Single Test");
			ScrPrint(0,3,0,"F2-Loop Test");
			ScrPrint(0,4,0,"Other-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)
			{
				ucPollOk = 0;
				ucRet = s_RF_ucPOLL_EMV_Step(&ucType);
				test_ProcErr(ucRet);
				if(ucComOk)
					test_SendDataByCom();
				if(ucRet == 0)
					ucPollOk = 1;
			}
			else if(ucKey == RF_KEY_F2)
			{				
				ScrCls();
				ScrPrint(0,0,0,"  Polling LOOP ");
				ScrPrint(0,1,0,"  [F3] to Quit ");
				i = 0;
				while(1)
				{
					ScrPrint(0,2,0,"Loop Cnt:%d ", i);
					
					if(!kbhit() && getkey() == RF_KEY_F3)
						break;

					ucPollOk = 0;
					gl_usRecLen = 0;
					memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					ucRet = s_RF_ucPOLL_EMV_Step(&ucType);
					i++;
					 
					test_ProcErr(ucRet);
					if(ucComOk)
						test_SendDataByCom();
					DelayMs(5);					
				}
				gl_usRecLen = 0;
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
				if(ucRet == 0)
					ucPollOk = 1;
			}
			else
			{
				ScrPrint(0,6,0," No Test ");
			}
			getkey();
			ScrCls();
			break;	

		case 5: // WUPA->RATS
		case '5':
			ScrCls();
			ScrPrint(0,0,0," WUPA->RATS ");
			if(ucPollOk == 0)
			{
				ScrPrint(0,6,0, "Pls Polling First!");
				getkey();
				ScrCls();
				break;
			}				
			//*
			ScrPrint(0,2,0,"F1-Single Test");
			ScrPrint(0,3,0,"F2-Loop Test");
			ScrPrint(0,4,0,"Other-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)//*/
			{
				ucRet = s_RF_Activate('A',aucResp);
				test_ProcErr(ucRet);
				if(ucComOk)
					test_SendDataByCom();
			}
			//*
			else if(ucKey == RF_KEY_F2)
			{
				ScrCls();
				ScrPrint(0,0,0,"  WUPA->RATS LOOP ");
				ScrPrint(0,1,0,"  [F3] to Quit ");
				i = 0;
				while(1)
				{
					ScrPrint(0,2,0,"Loop Cnt:%d ", i);
					
					if(!kbhit() && getkey() == RF_KEY_F3)
						break;
					gl_usRecLen = 0;
					memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					ucRet = s_RF_Activate('A',aucResp);
					i++;
					test_ProcErr(ucRet);
					if(ucComOk)
						test_SendDataByCom();
					DelayMs(5);					
				}
				gl_usRecLen = 0;
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
			}
			else
			{
				ScrPrint(0,6,0," No Test ");
			}//*/
			getkey();
			ScrCls();			
			break;
		
		case 6:// WUPB->ATTRIB
		case '6':
			ScrCls();
			ScrPrint(0,0,0," WUPB->ATTRIB ");
			if(ucPollOk == 0)
			{
				ScrPrint(0,6,0, "Pls Polling First!");
				getkey();
				ScrCls();
				break;
			}				
			//*
			ScrPrint(0,2,0,"F1-Single Test");
			ScrPrint(0,3,0,"F2-Loop Test");
			ScrPrint(0,4,0,"Other-Quit");
			ucKey = getkey();
			if(ucKey == RF_KEY_F1)//*/
			{
				ucRet = s_RF_Activate('B',aucResp);
				test_ProcErr(ucRet);
				if(ucComOk)
					test_SendDataByCom();
			}
			//*
			else if(ucKey == RF_KEY_F2)
			{
				ScrCls();
				ScrPrint(0,0,0,"  WUPB->ATTRIB LOOP ");
				ScrPrint(0,1,0,"  [F3] to Quit ");
				i = 0;
				while(1)
				{
					ScrPrint(0,2,0,"Loop Cnt:%d ", i);
					
					if(!kbhit() && getkey() == RF_KEY_F3)
						break;
					gl_usRecLen = 0;
					memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
					ucRet = s_RF_Activate('B',aucResp);
					i++;
					test_ProcErr(ucRet);
					if(ucComOk)
						test_SendDataByCom();
					DelayMs(5);					
				}
				gl_usRecLen = 0;
				memset(gl_aucRecData, 0, sizeof(gl_aucRecData));
			}
			else
			{
				ScrPrint(0,6,0," No Test ");
			}
			//*/
			getkey();
			ScrCls();			
			break;

		case 8:// 模拟用户测试(获取主文件->外部认证->内部认证->写文件->读文件)
		case '8':
			ScrCls();
			kbflush();
			Userfun_test();		
			break;
		default:
			ScrCls();
			break;
		}
    }					
}
void test_PreValidate(void)
{	 
	uchar ucTemp = 0; 
	uchar ucRet = 0;
	APDU_SEND apdu_s;
	APDU_RESP apdu_r;
	

	PiccOpen();
	test_ParaSetup();
	ScrCls();
	ScrPrint(0,0,0," Pre-Validate Test");
	ScrPrint(0,1,0," [F3] to Quit ");
	ScrPrint(0,4,0," Present Card >>>");
	while(1)
	{
		PiccLight(PICC_LED_RED|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
		PiccLight(PICC_LED_GREEN, 1);
		if(!kbhit() && getkey() == RF_KEY_F3)
		{
			PiccClose();
			return;
		}
		if(PiccDetect(1, NULL, NULL, NULL,NULL) == 0)
		{
			ScrCls();
			ScrPrint(0,4,0,"Pls Wait...");
			break;
		}
	}

	Beep();
	PiccLight(PICC_LED_BLUE, 1);

	memset(&apdu_s, 0, sizeof(apdu_s));
	memset(&apdu_r, 0, sizeof(apdu_r));
	
	memcpy(apdu_s.Command, "\x00\xA4\x04\x00", 4);
	apdu_s.Lc = 0x0E;
	memcpy(apdu_s.DataIn, "2PAY.SYS.DDF01", 0x0E);
	apdu_s.Le = 0;
	ucRet = PiccIsoCommand(0, &apdu_s, & apdu_r);
	if(ucRet || (apdu_r.SWA != 0x90 && (apdu_r.SWA & 0x60) != 0x60))
	{
		PiccLight(PICC_LED_GREEN|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
		PiccLight(PICC_LED_RED, 1);
		ScrCls();
		ScrPrint(0,0,0," Pre-Validate Test");
		ScrPrint(0,2,0,"   Failed ");
	}
	else
	{
		PiccLight(PICC_LED_RED|PICC_LED_BLUE|PICC_LED_YELLOW, 0);
		PiccLight(PICC_LED_GREEN, 1);
		Beep();
		ScrCls();
		ScrPrint(0,0,0," Pre-Validate Test");
		ScrPrint(0,2,0,"   Succeed ");
		ScrPrint(0,4,0," Pls Remove Card");
		
		while(1)
		{
			if(PiccRemove_EMV(0, 0) == 0)
				break;
		}
	}
	
	PiccClose();			

	ScrClrLine(3,7);
	ScrPrint(0,4,0,"Pre-Valid Test End!");
	getkey();	
}
void MifreQpbocTest(void)
{
	uchar ucKey = 0x00;	
	uchar ucRet = 0;
	
	memset(&my_c_para, 0, sizeof(PICC_PARA));
	gl_ucShowDbgFlg = 0;
	gl_ulFwtExtVal = 0;

	ScrCls();
	
	PiccLight(0x0F, 0);
	PiccLight(PICC_LED_GREEN, 1);
	
	ucRet = PiccOpen();
	if (ucRet) 
	{
		ScrCls();
		ScrPrint(0,3,0,"Open Err: %02X ", ucRet);
		getkey();
		while(1)
		{
		};
	}
	PiccSetup('r',&my_c_para);
	PiccClose();


	kbflush();
	ScrCls();
	while(1)
	{ 
	     ScrPrint(0, 0, 0x81, "PP L1 Test v0.21");	
		 ScrPrint(0, 3, 0, "F1 - Analogue Test ");	
		 ScrPrint(0, 4, 0, "F2 - Digital Test");
		 ScrPrint(0, 5, 0, "F3 - Pre-Validate ");
		 ScrPrint(0, 6, 0, "F4 - Param Set/Reset");
		 kbflush();
		 ucKey = getkey();

		 if (ucKey == RF_KEY_F1) 
		 {
			 test_Analogue();
			 ScrCls();
		 }
		 if (ucKey == RF_KEY_F2) 
		 {
			 test_LoopBackTest();
			 ScrCls();
		 }
		 if(ucKey == RF_KEY_F3)
		 {
		 	test_PreValidate();
			ScrCls();
		 }				
		 if (ucKey == RF_KEY_F4) 
		 {	 
			 test_SetRFParam();
			 ScrCls();
		 }
	}
	
}  



 
/******************************************************/

//add userfun test
//返回100个字节测试
uchar Userfun_test(void)
{
	uchar result;
	uchar Slot = 0;
	APDU_SEND ICOM;
	APDU_RESP OUTCOM;
	int i;
	int cnt;
	uchar input[8],output[8],deskeyL[8],deskeyR[8];
	uchar data[512];
	
	uchar keyvar=0;

	uint e_count1=0,e_count2=0,e_count3=0,e_count4=0,e_count5=0,e_count6=0,e_count7=0,e_count8=0,e_count9=0;
	uint r_count1=0,r_count2=0,r_count3=0,r_count4=0,r_count5=0,r_count6=0,r_count7=0,r_count8=0,r_count9=0;
	uint total_count=0,page=0;
	
	ScrCls();	
	ScrPrint(0,0,0,"100 bytes compress test,4-exit");//返回100个字节压力测试

//	if(PiccOpen())
//	{
//	   ScrPrint(0,0,CFONT,"本机无射频卡模块");
//		getkey();
//		return 0;
//	}
//	ScrClrLine(2,7);
	i=0;
	cnt=0;
	while(1)
	{

		if(!kbhit()  )
		{
			keyvar=getkey();
			if(keyvar==RF_KEY_F3 || keyvar=='3')
			break;
		}
			
		total_count++;

		while((result=PiccDetect(0, NULL, NULL, NULL, NULL)) != 0 )
		{
			if(!kbhit()  )
			{
				keyvar=getkey();
				if(keyvar==RF_KEY_F3 || keyvar=='3')
				break;
			}
		
		   ScrPrint(0,2,0,"input a card...");//请在感应区内放入A卡
		   ScrPrint(0,4,0,"search...");//搜卡中.....
		}

		ScrClrLine(2,7);
		if(result)
		{
			ScrPrint(0,4,0," detect err,ret=0x%02x",result);//检测卡错误
			e_count1++;
			//getkey();
			continue;
		}
		else r_count1++;
		
		ScrClrLine(2,7);
		ScrPrint(0,4,0,"wait...");//请稍等
				
		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0xa4;
		ICOM.Command[2] = 0x04;
		ICOM.Command[3] = 0x00;
		ICOM.Lc = 0x0e;
		ICOM.Le = 256;
		memcpy(ICOM.DataIn,"1PAY.SYS.DDF01",14);
		ICOM.DataIn[14]=0;
		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0	||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"select 3F00 file err,ret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count2++;
			continue;
		} 
		else r_count2++;

/*
取8位随机数:
->0084000008
*/
		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0x84;
		ICOM.Command[2] = 0x00;
		ICOM.Command[3] = 0x00;
		ICOM.Lc = 0x00;
		ICOM.Le = 0x08;
		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0	||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"get 8 byte random,\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count3++;
			continue;
		} 
		else r_count3++;

/*
外部认证:
->00820000081506CDB503629780(认证密钥：5741544348444154 4154696D65434F53)
*/

		memcpy(deskeyL,(uchar *)"\x57\x41\x54\x43\x48\x44\x41\x54",8);
		memcpy(deskeyR,(uchar *)"\x41\x54\x69\x6D\x65\x43\x4F\x53",8);
		memcpy(input,OUTCOM.DataOut,8);

		des(input,output,deskeyL,1);
		memcpy(input,output,8);
		des(input,output,deskeyR,0);
		memcpy(input,output,8);
		des(input,output,deskeyL,1);


		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0x82;
		ICOM.Command[2] = 0x00;
		ICOM.Command[3] = 0x00;
		ICOM.Lc = 0x08;
		ICOM.Le = 0x00;
		memcpy(ICOM.DataIn, output, 8);	
		ICOM.DataIn[8]=0;

		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0	||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"out auth\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count4++;
			continue;
		} 
		else r_count4++;


		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0xa4;
		ICOM.Command[2] = 0x04;
		ICOM.Command[3] = 0x00;
		ICOM.Lc = 0x09;
		ICOM.Le = 256;
		memcpy(ICOM.DataIn,(uchar *)"\xA0\x00\x00\x00\x03\x86\x98\x07\x01",9);
		ICOM.DataIn[9]=0;
		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0	||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"select 20f1 file err\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count5++;
			continue;
		}
		else r_count5++;
		
//内部认证
		memset(data,0x01,sizeof(data));
		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0x88;
		ICOM.Command[2] = 0x00;
		ICOM.Command[3] = 0x01;
		ICOM.Lc = 255;
		memcpy(ICOM.DataIn, data,255);	
		ICOM.Le = 256;

		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0	||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"in auth err \nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count6++;
			continue;
		} 	
		else r_count6++;

		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0xa4;
		ICOM.Command[2] = 0x00;
		ICOM.Command[3] = 0x00;
		ICOM.Lc = 0x02;
		ICOM.Le = 256;
		memcpy(ICOM.DataIn,(uchar *)"\x00\x19",2);
		ICOM.DataIn[2]=0;
		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);

		if(result != 0 ||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"select 0019 file err\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count7++;
			continue;
		}
		else r_count7++;
		
		ScrClrLine(2,7);
		ScrPrint(0,0,0,"writing......");
		
		ICOM.Command[0] = 0x00;
		ICOM.Command[1] = 0xE2;
		ICOM.Command[2] = 0x00;
		ICOM.Command[3] = 0xC8;
		ICOM.Lc = 0x64;
		ICOM.Le = 256;
		memset(ICOM.DataIn,0x11,100);
		
		result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);
		if(result != 0 ||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
		{
			ScrClrLine(2,7);
			ScrPrint(0,2,0,"write err\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
			//getkey();
			e_count8++;
			continue;
		}
		else r_count8++;

		ScrClrLine(2,7);
		ScrPrint(0,2,0,"reading......");
			memset(OUTCOM.DataOut,0,sizeof(OUTCOM.DataOut));
			OUTCOM.LenOut=0;
			ICOM.Command[0] = 0x00;
			ICOM.Command[1] = 0xb2;
			ICOM.Command[2] = 1;
			ICOM.Command[3] = 0xCC;
			ICOM.Lc = 0;
			ICOM.Le =100;
			result=PiccIsoCommand(Slot,&ICOM,&OUTCOM);
			if(result != 0 ||OUTCOM.SWA != 0x90 || OUTCOM.SWB != 0x00)
			{
				ScrClrLine(2,7);
				ScrPrint(0,2,0,"read err\nret=%02x\nSWA=0x%02x,SWB=0x%02x",result,OUTCOM.SWA,OUTCOM.SWB);
				e_count9++;
				continue;
//			getkey();
//			break;
			}
			else if(OUTCOM.LenOut!=100 || memcmp(OUTCOM.DataOut,ICOM.DataIn,100))
			{
				ScrClrLine(2,7);
				ScrPrint(0,2,0,"read 100 byte err,len:%d",OUTCOM.LenOut);
				e_count9++;
				continue;
//			getkey();
//			break;
			}
			else
			{
				r_count9++;
			}
			ScrCls();
			PiccRemove('h',0);//将卡片由协议态回到dele态
		}//end while(1)
		//PiccClose();	
	
	//实现翻页显示
	#if (TERM_TYPE == TERM_R30 ||PRINT_UART_LCD ==1)
	ScrCls();
	ScrPrint(0,0,ASCII,"Sata:All(%d),det[y:%d/n:%d]",total_count,r_count1,e_count1);
	ScrPrint(0,1,ASCII,"sel 3f00[y:%d/n:%d],sel 20f1[y:%d/n:%d]",r_count2,e_count2,r_count5,e_count5);
	ScrPrint(0,3,ASCII,"out auth[y:%d/n:%d],in auth[y:%d/n:%d]",r_count4,e_count4,r_count6,e_count6);
	ScrPrint(0,5,ASCII,"get rand[y:%d/n:%d],sel 0019[y:%d/n:%d]",r_count3,e_count3,r_count7,e_count7);
	ScrPrint(0,7,ASCII,"write 0019[y:%d/n:%d],read 0019[y:%d/n:%d]",r_count8,e_count8,r_count9,e_count9);
	getkey();
	#else
	while(1)
	{
		//第一页
		if(page%2==0)
		{
			ScrCls();
			ScrPrint(0,0,ASCII,"Sata:All[y:%d/n:0]",total_count);
			ScrPrint(0,1,ASCII,"det[%d/%d]",r_count1,e_count1);
			ScrPrint(0,2,ASCII,"sel 3f00[%d/%d]",r_count2,e_count2);
			ScrPrint(0,3,ASCII,"sel 20f1[%d/%d]",r_count5,e_count5);
			ScrPrint(0,4,ASCII,"out auth[%d/%d]",r_count4,e_count4);
			ScrPrint(0,5,ASCII,"in auth[%d/%d]",r_count6,e_count6);
			ScrPrint(0,6,ASCII,"get rand[%d/%d]",r_count3,e_count3);
			ScrPrint(0,7,ASCII,"sel 0019[%d/%d]",r_count7,e_count7);
		}
		else
		{
			ScrCls();
			ScrPrint(0,0,ASCII,"write 0019[%d/%d]",r_count8,e_count8);
			ScrPrint(0,1,ASCII,"read 0019[%d/%d]",r_count9,e_count9);
		}

		keyvar=getkey();
		switch(keyvar)
		{
			case RF_KEY_F2://next page
			page++;
			break;
			case RF_KEY_F4:
			case RF_KEY_F3:
			ScrCls();
			return 0;
			break;
			default:
			break;
		}
	}
	#endif
	return 0;

}

#endif

uchar test_GetNewVal(uchar ucMin, uchar ucMax, uchar ucRst, uchar ucCur, uchar *pucVal)
{
	
	uchar ucStep;
	uchar ucKey =  0;	 
	uchar ucTemp = ucCur; 
	uchar ucVal = ucCur;
#define ScrClrLine Lib_LcdClrLine
#define ScrPrint  Lib_Lcdprintf
#define ScrCls Lib_LcdCls
        
	while(1)
	{
		Lib_LcdPrintxy(0,1*8,0,"Range:%02X~%02XH,Rst:%02XH", ucMin, ucMax, ucRst);  
		Lib_LcdPrintxy(0,2*8,0,"Cur: %02X  ", ucVal);
		Lib_LcdPrintxy(0,3*8,0,"New: %02X  ", ucTemp);

		Lib_LcdPrintxy(0,4*8,0,"STEP: F1-1 F2-5 \nF3-10 F4-Quit ");
                
                ucStep = 1;
#if 0
		ucKey = getkey();
		if (ucKey == RF_KEY_F1) 
		{
			ucStep = 1;
		}
		else if(ucKey == RF_KEY_F2)
		{
			ucStep = 5;
		}
		else if(ucKey == RF_KEY_F3)
		{
			ucStep = 10;
		}
		else
		{
			*pucVal=ucVal;//确保第一次进入立即出去，值设置上
			return 0 ;
		}
		
		ScrClrLine(4,7);
		ScrPrint(0,5,0,"F1-INC %d  F2-DEC %d ", ucStep, ucStep);			
		ScrPrint(0,6,0,"F3-Cancel  F4-Confirm");
#endif			
                Lib_LcdPrintxy(0,4*8,0,"1-INC 2-DEC 3-ESC 4-Ent");
                
		while (2) 
		{
			//if (!(kbhit())) 
			{
				ucKey = getkey();
				if (ucKey ==RF_KEY_F3) 
				{
					ScrClrLine(4,7);
					Lib_LcdPrintxy(0,6*8,0," Set Canceled ");
					getkey();
					ScrCls();
					return 1;
				}
				if (ucKey == RF_KEY_F1) 
				{
					if ((ucTemp + ucStep) > ucMax ) 
					{
						ucTemp = ucMin;
					}
					else
						ucTemp += ucStep;
					 #if(TERM_TYPE==TERM_R30 ||PRINT_UART_LCD ==1)
					 ScrPrint(0,6,0," include %d",ucStep);
					 ScrPrint(0,3,0,"Current NewVal: %02X  ", ucTemp);
					 #endif
				}
				if (ucKey == RF_KEY_F2) 
				{
					if (ucTemp < ucStep) 
					{
						ucTemp = ucMax;
					}
					else if ((ucTemp - ucStep) < ucMin) 
					{
						ucTemp = ucMax;
					}
					else
						ucTemp -= ucStep;

					#if(TERM_TYPE==TERM_R30 || PRINT_UART_LCD ==1)
					ScrPrint(0,6,0," decrease %d",ucStep);
					ScrPrint(0,3,0,"Current NewVal: %02X  ", ucTemp);
					#endif
				}	
				if (ucKey == RF_KEY_F4) 
				{
					ScrClrLine(4,7);
					ScrPrint(0,6,0,"Set OK ");
					getkey();
					*pucVal = ucTemp;
					ucVal = ucTemp;
					ScrCls();
  				    break;
				}
			}
			#if(TERM_TYPE!=TERM_R30 && PRINT_UART_LCD !=1)
			ScrPrint(0,3,0,"New: %02X  ", ucTemp);
			#endif
		}
	}
}

void test_SetParam(void)
{
	uchar ucMax;
	uchar ucMin;
	uchar ucRst;
	uchar ucStep;
	uchar ucKey = 0x00;	
    uchar ucRet = 0;
	uchar ucTemp = 0;
	uchar aucStr[10];
	uchar ucVal = 0;
		
	Lib_LcdCls();
	while(1)
	{
		Lib_LcdPrintxy(0,0,0x80,"  PCD Param Set");
		Lib_LcdPrintxy(0,1*8,0,"1-A_CwConduct"); // CWGsPREG ADD:28H,DEF VAL:0x3F
		Lib_LcdPrintxy(0,2*8,0,"2-B_ModConduct "); // GsNOnREG bit7~4 ADD:27H,DEF VAL:0xF0 低4位数据将被清除
		Lib_LcdPrintxy(0,3*8,0,"3-B_RxThrsHld"); // ModGsPREG bit5~0 add 29h, DEF val:0x11 
		Lib_LcdPrintxy(0,4*8,0,"4-Quit ");
	    
		ucKey = getkey();
		switch(ucKey)
		{
		case RF_KEY_F1:
			Lib_LcdCls();
			ucMax = 0x3F;
			ucMin = 0x00;
			ucRst = 0x20; 
			//ucTemp = my_c_para.a_conduct_val;
                        ucTemp = c_para.a_conduct_val;
			Lib_LcdPrintxy(0,0,0," A_CwConduct:%02X  ", ucTemp);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal;
			//my_c_para.a_conduct_val = ucTemp;
                        c_para.a_conduct_val = ucTemp;
			Lib_LcdCls();
			break;
		case RF_KEY_F2:
			Lib_LcdCls();
			ucMax = 0x3F;
			ucMin = 0x00;
			ucRst = 0x20; 
			//ucTemp = my_c_para.b_modulate_val;
                        ucTemp = c_para.b_modulate_val;
			Lib_LcdPrintxy(0,0,0," B_ModConduct:%02X  ", ucTemp);			
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal;	 
			//my_c_para.b_modulate_val = ucTemp;
                        c_para.b_modulate_val = ucTemp;
			Lib_LcdCls();
			break;
			
		case RF_KEY_F3:
			Lib_LcdCls();
			ucMax = 0xFF;
			ucMin = 0x00;
			ucRst = 0x88; 
			//ucTemp = B_gl_RxThreshold;
			//ucTemp = my_c_para.a_conduct_val;
                        ucTemp = c_para.card_RxThreshold_val;
			Lib_LcdPrintxy(0,0,0," B_RxThreshold:%02X  ", ucTemp); 		
			if(!test_GetNewVal(ucMin, ucMax, ucRst, ucTemp, &ucVal))
				ucTemp = ucVal; 
			//B_gl_RxThreshold = ucTemp;
			//my_c_para.a_conduct_val = ucTemp;
                        c_para.card_RxThreshold_val = ucTemp;
			Lib_LcdCls();
			break;
			
		case RF_KEY_F4: 
			Lib_LcdCls();
			return;
		 
		default:
			break;
		}
				
		//memset(&my_c_para, 0, sizeof(PICC_PARA));
		//memcpy(&my_c_para, &c_para, sizeof(PICC_PARA));
                _glbPiccSetPara = 1;
#if 0
		my_c_para.a_conduct_w = 1;
		my_c_para.b_modulate_w = 1;
//		my_c_para.card_buffer_w = 1;
		my_c_para.m_conduct_w = 1;
		my_c_para.wait_retry_limit_w = 0;
#endif
                
	}
}
