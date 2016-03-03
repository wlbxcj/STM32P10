
#include "vosapi.h"
#include "dll.h"
#include "FlashOperate.h"
#include "buzzer.h"
#include "pci.h"
#include "..\..\inc\FunctionList.h"
#include "comm.h"
#include <time.h>
#include "stm32f10x_lib.h"
#include "Toolkit.h"
#include "smartcard.h"
#include "pci.h"
#include "misc.h"
#include "kf701dh.h"
#include "..\lcd\lcd.h"
#include "base.h"

//size
#define FIRST_RUN_M   0     //16
#define MAN_APP_KEY_M 16    //32*2+4
#define SYSPARA_M     84    //76+4+10 -> 84+4+10
#define LCD_GRAY_M    182   //2 174->174+8 size:2
//#define DOWNLOAD_OK_M    166   //10

#define  PICC_SETPARA 184

#define STORAGE_WORD_COUNT 16

#define TMSTAG_VOS 0x01


#define DES Lib_DES
int  g_iVosFirstRunFlag;  // 1:first run, 0:not first run
int g_iActionLimits = 0;

extern unsigned char _glbPiccSetPara;
extern unsigned long LCDValue;
extern void settime (struct tm *tDate);
int CheckTime(uchar *str,uchar *week);

int TmsTag_Read(uchar type, char *tag_str)	
{
  char sTmp[20]={0},sLocDate[20]={0},i;
  char sLocMon[12][4]=
  {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec",
  };
  
  sprintf(sTmp,"%s",__DATE__);
  //test
  //trace_debug_printf("date[%s]\n",sTmp);
  memcpy(sLocDate,&sTmp[9],2);
  for(i=0;i<12;i++)
  {
    if(memcmp(sLocMon[i],sTmp,3)==0)
      break;
  }
  sprintf(&sLocDate[2],"%02d",i+1);
  memcpy(&sLocDate[4],&sTmp[4],2);
  if(sLocDate[4]==' ')
    sLocDate[4] = '0';
  
  if(tag_str == NULL)
    return -1;
  
  switch (type) 
  {
		
    case TMSTAG_VOS:
#ifdef V10X  
      if(LCDValue < 3000)
	sprintf(tag_str, "V10X2C0%s%s",   //13/10/23
			//FIRMWARE_MAJOR_VERSION, 
			//FIRMWARE_MINOR_VERSION, 
			sLocDate, //YYMMDD
			//"130628", //YYMMDD
			"00"); //Fw_Version
       else if(LCDValue >= 3000) 
        sprintf(tag_str, "V10X1C0%s%s",   //13/10/23
			//FIRMWARE_MAJOR_VERSION, 
			//FIRMWARE_MINOR_VERSION, 
			sLocDate, //YYMMDD
			//"130628", //YYMMDD
			"00"); //Fw_Version
#else
    if(LCDValue < 3000)
	    sprintf(tag_str, "V10P2C0%s%s",   //13/10/23
			//FIRMWARE_MAJOR_VERSION, 
			//FIRMWARE_MINOR_VERSION, 
			sLocDate, //YYMMDD
			//"130628", //YYMMDD
			"00"); //Fw_Version  
    else if(LCDValue >= 3000) 
        sprintf(tag_str, "V10P1C0%s%s",   //13/10/23
			//FIRMWARE_MAJOR_VERSION, 
			//FIRMWARE_MINOR_VERSION, 
			sLocDate, //YYMMDD
			//"130628", //YYMMDD
			"00"); //Fw_Version   
#endif
			break;
    default:	
	return -2;	
  }
	
  return 0;  
}



int  sys_flash_syspara_read(u32 Address,u8 * buff,u32 len)
{
    return sys_flash_read_operate(SysEndAddr-SysStartAddr-200+1+Address,buff,len);
}

int sys_flash_syspara_write(u32 Address,u8 * buff,u32 len)
{
    return sys_flash_write_operate(SysEndAddr-SysStartAddr-200+1+Address, buff,len); 
}

/*****************************************************
功能:  核查VOS是否第一次运行
           : 返回1;第一次运行VOS
                 0;不是第一次运行VOS
******************************************************/
int  CheckVosIsFirstRun(void)
{
    BYTE FirstFlag[64];
    int i, j;
#if 0
    CheckFlash();

    s_UartPrint(COM_DEBUG,"FS=%x BT=%x\r\n",FlashType_Size,FlashType_BT);
#endif
    // change by Ryan 20080919
    memset(FirstFlag, 0, sizeof(FirstFlag));
    j = 0;
    sys_flash_syspara_read(FIRST_RUN_M,FirstFlag,16);
    
    for (i=0; i<16; i++)
    {
      
#if 0      
		FirstFlag[i] = *(BYTE*)(0xd03c0000+i); 
        //FirstFlag[i] = *(BYTE*)(0xD0000000+i);
#endif                
        if (FirstFlag[i] == 0x55)
            j++;
    }
    if (j < 10)
    {
        memset(FirstFlag, 0x55, 16);
        //(void)SectorErase(0);
        //j = s_FlashWrite(0x000000, FirstFlag, 16);
#if 0        
        s_EraseSector(0xd03c0000);
        j = s_WriteFlash((WORD*)0xd03c0000, FirstFlag,16);  
#endif
        j = sys_flash_syspara_write(FIRST_RUN_M,FirstFlag,16);
        
        if (0 != j)
            {
#ifdef UARTPRINF_DEBUG  
            Lib_Lcdprintf("Write flash error");
            Lib_KbGetCh();
#endif            
            }
        
        
        return 1;
    }
    
    
    return 0;
}

void AM_InitAll(void)
{
#if 0
    s_EraseSector(MA_SECTOR_TWO_ADDR);
    s_EraseSector(MA_SECTOR_ONE_ADDR);
#endif    
}

/************************************************************
 针对第一次运行VOS时，须初始化
 ************************************************************/
int s_InitVos(void)
{
    int iRet;
	BYTE buf[8];
	DWORD temp[2];
	//DWORD dwCount;
    /*test
    unsigned char MMK[24];
    unsigned char ELRCK[16];
    unsigned char i;*/
        
	memset(buf,0xff,sizeof(buf));
	ByteToDword(&buf[0],4,&temp[0]);
	ByteToDword(&buf[4],4,&temp[1]);
        g_iVosFirstRunFlag = CheckVosIsFirstRun();
        
        //test 13/07/12
        trace_debug_printf("g_iVosFirstRunFlag[%d]\n",g_iVosFirstRunFlag);
        //1-first time run
        //0-not first time run
        if (0 != g_iVosFirstRunFlag)
        {
          
	    s_PciWritePinCount(0,temp[0]);
	    s_PciWritePinCount(1,temp[1]);
        //(void)s_RecreateSysKeyFile();
          /*
          //13/06/25
          if(g_iVosFirstRunFlag!=0)
          {
#ifdef UARTPRINF_DEBUG
            Lib_LcdCls();
            Lib_LcdPrintxy(0,0,1," DelAllSecrecyData");
            Lib_KbGetCh();
#endif            
          }
          */
            
	    s_DelAllSecrecyData();
            
            AM_InitAll();
        }

    iRet = s_PciMMKCheck();
    
        //test 13/07/12
        trace_debug_printf("s_PciMMKCheck[%d]\n",iRet);
    
    if (0!=iRet)
    {
        //(void)SectorErase(0);
        //(void)s_RecreateSysKeyFile();
        //test 13/07/12
        trace_debug_printf("g_iVosFirstRunFlag[%d]\n",g_iVosFirstRunFlag);
      
        //13/06/25
        //1-first time run
        //0-not first time run
        if(g_iVosFirstRunFlag==0)
        {
#ifdef UARTPRINF_DEBUG
          Lib_LcdCls();
          Lib_LcdPrintxy(0,0,1," MMK FAIL ");
        //test 13/07/12
        trace_debug_printf("MMK FAIL\n");
          
          Lib_KbGetCh();
#endif          
        }
        
	s_DelAllSecrecyData();
        AM_InitAll();
        s_PciInitMMK();
        g_iVosFirstRunFlag = 1;
    }
#if 0    
    //test 13/06/25
    s_PciReadMMK(0, (DWORD*)MMK);
    s_PciReadMMK(1, (DWORD*)ELRCK);
    trace_debug_printf("MMK:");
        for(i=0;i<24;i++)
          trace_debug_printf("%02x ",MMK[i]);
        trace_debug_printf("\n");
    trace_debug_printf("ELRCK:");
        for(i=0;i<16;i++)
          trace_debug_printf("%02x ",ELRCK[i]);
        trace_debug_printf("\n");
#endif    
    
    return 0;
}

int TAMPERClear(void)
{
	DWORD temp[2];
	BYTE buf[8],i;
        
  	memset(buf,0xff,sizeof(buf));
	ByteToDword(&buf[0],4,&temp[0]);
	ByteToDword(&buf[4],4,&temp[1]);

	s_PciWritePinCount(0,temp[0]);
	s_PciWritePinCount(1,temp[1]);
        //(void)s_RecreateSysKeyFile();
        
	s_DelAllSecrecyData();
        AM_InitAll();
        
        //13/05/24
        temp[0] = 0;
        for(i=0;i<STORAGE_WORD_COUNT;i++)
          ss_BpkWriteReg(i,temp[0]);
    return 0;
}


int s_CheckSDRAM(void)
{
    //int i;
	//unsigned char ch;
	//for(i=0;i<(8*1024*1024);i++)
	//{
	//	*(unsigned char *)(0x20800000+i)=0x55;
	//	ch=*(unsigned char *)(0x20800000+i);
	//	if(ch!=0x55) return -1;
	//} 
    return 0;
}

int s_CheckAllKeys(void)
{

    return 0; 
}



int s_SystemTestSelf(void)
{
    //return 0;
    int iret;
    Lib_LcdCls();
    Lib_LcdSetFont(8, 16, 0);
    Lib_LcdPrintxy(0,0,0x80,"   SYSTEM SELF TEST  ");
    //Lib_LcdPrintxy(0,16/2,1,"Test SDRAM.... ");
	Lib_LcdPrintxy(0,8*1,0,"Test SDRAM.... ");
    //test sdram
    iret=s_CheckSDRAM();
    if(iret)
    {
	s_PciInitMMK();
        //s_PciInitBPK();
        //s_PciClearAllKeys();
        Lib_LcdCls();
        Lib_LcdPrintxy(0,0,1," SDRAM FAIL ");
        //Lib_LcdPrintxy(0,16/2,1," PINPAD LOCKED ");
	Lib_LcdPrintxy(0,8*2,1," PINPAD LOCKED ");
	WriteLog("Test SDRAM FAILED\n", 1, 0xff);
        for(;;);
    }
    Lib_DelayMs(300);
    //Lib_LcdPrintxy(0,16/2,0x01,"Test SDRAM....OK");
    Lib_LcdPrintxy(0,8*1,0x00,"Test SDRAM....OK");
    WriteLog("Test SDRAM OK\n", 1, 0xff);
	

  /*  Lib_LcdPrintxy(0,32/2,1,"Test MMK....     ");*/
    Lib_LcdPrintxy(0,8*2,0,"Test MMK....     ");
    //test bpk
    iret = s_PciMMKCheck();
    if (0 != iret)
    {
	s_PciInitMMK();
        Lib_LcdCls();
        Lib_LcdPrintxy(0,0,1," MMK FAIL ");
        Lib_LcdPrintxy(0,32/2,1," PINPAD LOCKED ");
	WriteLog("Test MMK FAILED\n", 1, 0xff);
        for(;;);
    }
    

    Lib_DelayMs(300);
    //Lib_LcdPrintxy(0,32/2,1,"Test MMK....OK");
	Lib_LcdPrintxy(0,8*2,0,"Test MMK....OK");
	WriteLog("Test MMK OK\n", 1, 0xff);


    //Lib_LcdPrintxy(0,48/2,1,"Test KEYS...");
	Lib_LcdPrintxy(0,8*3,0,"Test KEYS...");
	//s_UartPrint(COM_DEBUG,"test keys\r\n");
    //test lrc of keys
    iret=s_CheckAllKeys();
    if(iret)
    {
	s_PciInitMMK();
        //s_PciInitBPK();
        //s_PciClearAllKeys();
        Lib_LcdCls();
        Lib_LcdPrintxy(0,0,1," KEY FAIL  ");
        //Lib_LcdPrintxy(0,48/2,1," PINPAD LOCKED  ");
	Lib_LcdPrintxy(0,32/2,1," PINPAD LOCKED  ");
	WriteLog("Test KEYS FAILED\n", 1, 0xff);
        for(;;);
    }
    Lib_DelayMs(300);
    //Lib_LcdPrintxy(0,48/2,1,"Test KEYS....OK");
    Lib_LcdPrintxy(0,8*3,0,"Test KEYS....OK");
    WriteLog("Test KEYS OK\n", 1, 0xff);



    //s_UartPrint(COM_DEBUG,"self test ok\r\n");
    return 0;
}

DWORD s_AM_ReadMaKey(MAN_APP_KEY *psOutMaKey)
{
    sys_flash_syspara_read(MAN_APP_KEY_M,(uchar*)psOutMaKey,sizeof(MAN_APP_KEY));
    return 0;
}

void s_AM_WriteMaKey(MAN_APP_KEY *psInMaKey)
{
  int nRet;  
  nRet = sys_flash_syspara_write(MAN_APP_KEY_M,(uchar*)psInMaKey,sizeof(MAN_APP_KEY));
  if(nRet)
  {
        Lib_LcdCls();
       /* Lib_LcdGotoxy(0, 0);
        Lib_Lcdprintf("Write flash error!=%d,", iRet);*/
	Lib_LcdPrintxy(0, 8*0,0x00,"Write flash error!=%d,", nRet);
        while (1);
    
  }
  
}
// 0x55:Lock
BYTE AM_GetLockStatus(void)
{
    DWORD dwRet;
    MAN_APP_KEY loc_sMaKey;

    dwRet = s_AM_ReadMaKey(&loc_sMaKey);
    if (AM_SUCCESS != dwRet)
    {
        return 0xff;
    }
    return loc_sMaKey.m_abyLock[0];
}

void AM_CheckPedLock(void)
{
    if (0x55 == AM_GetLockStatus())
    {
        Lib_LcdClrLine(0, LCD_HIGH_MINI-1);
       /* Lib_LcdGotoxy(0, 32);
        Lib_LcdPrintfCE("    PED 被锁    ", "   PED locked!  ");*/
	Lib_LcdPrintxy(0, 8*2,0x00,"   PED locked!  ");
        while (1);
    }
}

void DrawThreeLine(void)
{
  
}

int CheckVOSSignKey(void)
{
	return 0;
#if 0        
    int iRet;
    //DWORD adwMMK[5];
    BYTE abyKey[16]; 
    //iRet = s_PciReadMMK(2, adwMMK);
//    iRet = s_ReadBSK(abyKey);
//    if (0 != iRet)
//    {
//        Lib_LcdCls();
//        Lib_LcdPrintxyCE(0,  0, 0x81, "    系统菜单    ", "   SYSTEM MENU  ");
//        if (-1 == iRet)
//        {
//            Lib_LcdPrintxyCE(0, 20, 0x01, " BSK未下载  ", " Not BSK!  ");
//            Lib_LcdPrintxyCE(0, 40, 0x01, " 请先下载该密钥 ", "Pls load BSK. ");
//        }
//        else
//        {
//            Lib_LcdPrintxyCE(0, 32, 0x01, "读BSK错误  ", "Read BSK error!   ");
//        }
//        DrawThreeLine();
//        Lib_KbFlush();
//        (void)Lib_KbGetCh();
//        return 1;
//    }
    
    
//    iRet=s_CheckBootSign();
//    if(iRet)
//    {
//        Lib_LcdCls();
//        Lib_LcdPrintxy(0,0,1, " BSK&BOOT FAIL ");
//        Lib_LcdPrintxy(0,16,1," PINPAD LOCKED ");
//        for(;;);
//    } 
    
    

    //iRet = s_PciReadMMK(2, adwMMK);
    iRet = s_ReadVSK(abyKey);
    if (0 != iRet)
    {
        Lib_LcdCls();
        //Lib_LcdPrintxyCE(0,  0, 0x80, "    系统菜单    ", "   SYSTEM MENU  ");
		Lib_LcdPrintxy(0,  0, 0x80, "    SYSTEM MENU     ");

        if (-1 == iRet)
        {
            //Lib_LcdPrintxyCE(0, 8*1, 0x00, " VSK未下载  ", " Not VSK!  ");
			 Lib_LcdPrintxy(0, 8*1, 0x00, " Not VSK!  ");
           /* Lib_LcdPrintxyCE(0, 8*2, 0x00, " 请先下载该密钥 ", "Pls load VSK. ");*/
			  Lib_LcdPrintxy(0, 8*2, 0x00, "Pls load VSK. ");
        }
        else
        {
            /*Lib_LcdPrintxyCE(0, 8*2, 0x00, "读VSK错误  ", "Read VSK error!   ");*/
			Lib_LcdPrintxy(0, 8*2, 0x00, "Read VSK error!   ");
        }
        DrawThreeLine();
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        return 1;
    }
    
    
    iRet=s_CheckVosSign();
    if(iRet)
    {
        Lib_LcdCls();
        Lib_LcdPrintxy(0,0,1, " VSK&VOS FAIL ");
        Lib_LcdPrintxy(0,8*2,1," PINPAD LOCKED ");
        for(;;);
    }
    
    
    return 0;
#endif
}



/****************************************************************************
  函数名     :  int SPF_GetAppRunStatus(BYTE AppNo)
  描述       :  查询应用程序运行状态，如果应用程序未载入或者未激活则程序是不会运行的
  输入参数   :  1、BYTE AppNo：应用程序编号，取值0~24
  输出参数   :  无
  返回值     :  0：程序未运行 / 1：程序已运行 / -1：创建文件错误 / -2: 输入参数错误
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_GetAppRunStatus(BYTE AppNo)
{
    //int iRet = SPF_SUCCESS;
    T_SYSTEM_PATA tSystempara;

    if (AppNo >= APP_MAX_NUM)
    {
        return SPF_PARA_ERROR;
    }
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA)); 
    
    
    if(4 == (tSystempara.bAttrib&0x04))
      return 1;
    else
      return 0;
#if 0
    READ_UPDATE(iRet);

    if (4 == (s_abyBuff[APP_ATTR_BASE_OFFSET+AppNo]&0x04))
    {
        return 1;
    }
    else
    {
        return 0;
    }
#endif
    
}

/****************************************************************************
  函数名     :  int SPF_SetAppRunStatus(BYTE AppNo, BYTE flag)
  描述       :  设置程序运行状态
  输入参数   :  1、BYTE AppNo：应用程序编号，取值0~24
                2、BYTE flag：运行标志
  输出参数   :  无
  返回值     :  0：成功 / -2：输入参数错误 / -3:创建系统参数文件错误
              / -5：程序未载入或者未激活
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_SetAppRunStatus(BYTE AppNo, BYTE flag)
{
    //int iRet = SPF_SUCCESS;
    T_SYSTEM_PATA tSystempara;
    
    if (AppNo >= APP_MAX_NUM)
    {
        return SPF_PARA_ERROR;
    }
    if ((0!=flag) && (1!=flag))
    {
        return SPF_PARA_ERROR;
    }
    
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA)); 
    if (1 == flag)
      tSystempara.bAttrib |=  0x04;
    else
      tSystempara.bAttrib |=  0x03;
    return sys_flash_syspara_write(  SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
#if 0
    READ_UPDATE(iRet);

    if (1 == flag)
    {
        s_abyBuff[APP_ATTR_BASE_OFFSET+AppNo] |= 0x04;
    }
    else
    {
        s_abyBuff[APP_ATTR_BASE_OFFSET+AppNo] &= 0x03;
    }

    return _SPF_WriteFile();
#endif    
}

int SPF_GetAppDownUnLockStatus(BYTE AppNo)
{
    //int iRet = SPF_SUCCESS;
    T_SYSTEM_PATA tSystempara;
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    //tSystempara.bAttrib &0x08;
    sys_flash_syspara_write(  SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    return 0;
}

int SPF_SetAppDownUnLockStatus(BYTE AppNo)
{
    //int iRet = SPF_SUCCESS;
    T_SYSTEM_PATA tSystempara;
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    
    tSystempara.bAttrib &=0xf7;
    
    return 0;
}

/****************************************************************************
  函数名     :  int SPF_GetAppStatus(BYTE AppNo)
  描述       :  查询应用程序状态
  输入参数   :  1、BYTE AppNo：应用程序编号，取值0~24
  输出参数   :  无
  返回值     :  0~7表示应用程序状态
               / -2:SPF_PARA_ERROR / -1:SPF_CREATE_FILE_ERROR
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-10  V1.0         创建
****************************************************************************/
int SPF_GetAppStatus(BYTE AppNo)
{
    //int iRet = SPF_SUCCESS;
    T_SYSTEM_PATA tSystempara;
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    return tSystempara.bAttrib &0x07;
#if 0    
    if (AppNo >= APP_MAX_NUM)
    {
        return SPF_PARA_ERROR;
    }

    READ_UPDATE(iRet);

    return (s_abyBuff[APP_ATTR_BASE_OFFSET+AppNo]&0x07);
#endif    
}
/****************************************************************************
  函数名     :  int SPF_GetAppName(BYTE byAppNO, char *pName)
  描述       :  根据应用程序序号查询应用程序名
  输入参数   :  1、BYTE byAppNO：应用程序序号 ，取值0~24
  输出参数   :  1、char *pName：应用程序名，该参数长度不小于32字节
  返回值     :  SPF_SUCCESS / SPF_CREATE_FILE_ERROR / SPF_PARA_ERROR
                / SPF_APP_NOEXIST
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_GetAppName(BYTE byAppNO, char *pName)
{
    //int iRet = SPF_SUCCESS;

    T_SYSTEM_PATA tSystempara;
    
    if (NULL == pName)
    {
        return SPF_PARA_ERROR;
    }
    if (byAppNO >= APP_MAX_NUM)
    {
        return SPF_PARA_ERROR;
    }
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    if( (tSystempara.bAttrib&0x01)==0)
      return SPF_APP_NOEXIST;
    memcpy(pName,  tSystempara.AppName,32);  
    
#if 0
    READ_UPDATE(iRet);

    if (0 == (s_abyBuff[APP_ATTR_BASE_OFFSET+byAppNO]&0x01))
    {
        return SPF_APP_NOEXIST;
    }
    memcpy(pName, (char *)s_abyBuff+APP_NAME_BASE_OFFSET+byAppNO*APP_INFO_LEN, 32);
    
#endif
    
    //printf("appno=%d pname=%s\r\n",byAppNO,pName);
    return SPF_SUCCESS;
}

/****************************************************************************
  函数名     :  int SPF_GetAppNO(const char *pName)
  描述       :  根据应用程序名查询应用程序的序号
  输入参数   :  1、const char *pName：应用程序名
  输出参数   :  无
  返回值     :  0~24：应用程序序号 / -2:SPF_PARA_ERROR / -1:SPF_CREATE_FILE_ERROR
               / -3:SPF_APP_NOEXIST
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_GetAppNO(const char *pName)
{
    T_SYSTEM_PATA tSystempara;
    //int i = 0,iRet = SPF_SUCCESS;

    if (NULL == pName)
    {
        return SPF_PARA_ERROR;
    }
    //only one app
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    if( (tSystempara.bAttrib&0x01)==0)
      return SPF_APP_NOEXIST;
    
#if 0    
    if(strcmp(tSystempara.AppName,pName)==0)
      return 0;
#endif
    return 0;
    
    //return SPF_APP_NOEXIST;

}

int SPF_GetDownloadOK(BYTE byAppNO, char *pDownloadOK)
{
    T_SYSTEM_PATA tSystempara;
    
    if (byAppNO >= APP_MAX_NUM)
    {
        return SPF_PARA_ERROR;
    }
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    if( (tSystempara.bAttrib&0x01)==0)
      return SPF_APP_NOEXIST;
    memcpy(pDownloadOK,tSystempara.sDownloadOK,10);
    return 0;
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void LoadApp(void)
{
    int i;
    //DWORD StartAddr, EntryAddr, dwAppInfo; 
    char strAppName[33];
    int iRet, iAppNum;
    //APP_MSG *pAppMsg;
    //SAppInfo sAppInfo;
    BYTE abyTemp[41];

    Lib_LcdCls();
    /*(void)Lib_LcdSetFont(8, 16, 0);
    (void)Lib_LcdSetAttr(1);
    Lib_LcdGotoxy(0, 0);
    Lib_LcdPrintfCE("    系统菜单    ", "     SYSTEM MENU    ");*/
	Lib_LcdPrintxy(0, 8*0,0x80,"     SYSTEM MENU    ");
    DrawThreeLine();
   /* (void)Lib_LcdSetAttr(0);
    Lib_LcdGotoxy(0, 32/2);*/
    for (i = 0; i < 25; i++)
    {
        iRet = SPF_GetAppStatus(i);
        if ((iRet>0) && ((iRet&0x03)==0x03))
        {
            break;
        }
    }
    if (i >= 25)
    {
        //Lib_LcdPrintfCE("    无 应 用    ", "   No program   ");
		Lib_LcdPrintxy(0, 8*2,0x00, "   No program   ");
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8,LCD_HIGH_MINI-1);
		WriteLog("No Program\n", 1, 0xff);
        return;
    }
    iAppNum = i;
    iRet = SPF_GetAppName(iAppNum, strAppName);
    if (0 != iRet)
    {
        //Lib_LcdPrintfCE("  读应用名错误  ", "Read appname err");
		Lib_LcdPrintxy(0, 8*2,0x00, "Read appname err");
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8/2,LCD_HIGH_MINI-1);
		WriteLog("Read AppName Error\n", 1, 0xff);
        return;
    }
    
#if 0    
    memset(strAppFileName,0,sizeof(strAppFileName));
    sprintf(strAppFileName, "AppFile%d", i);
    iFileLen = Lib_FileSize(strAppFileName);

    *(volatile ulong*)APP_APPMAN_FLAG_SPACE = 0x00;
    if (i == 0)
    {
        fd = s_open(strAppFileName, O_RDWR, (uchar *)"\xff\x00");
        StartAddr = APPMAN_RUN_SPACE;
        *(volatile ulong*)APP_APPMAN_FLAG_SPACE = 0xff00;
    }
    else
    {
        fd = s_open(strAppFileName, O_RDWR, (uchar *)"\xff\x01");
        StartAddr = APP_RUN_SPACE;
    }

    if (fd < 0)
    {
        //Lib_LcdPrintfCE("    无 应 用    ", "   No program   ");
		Lib_LcdPrintxy(0, 8*2,0x00, "   No program   ");
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8,LCD_HIGH_MINI-1);
		WriteLog("No Program\n", 1, 0xff);
        return;
    }
    EntryAddr = StartAddr;

    i=0;
    while (1)
    {
        cnt = Lib_FileRead(fd, gFileBuffer, sizeof(gFileBuffer));
        memcpy_byte((BYTE*)StartAddr, gFileBuffer, cnt);
        StartAddr += cnt;
        if(i==0)
        {
            i=1;
        }

        if (cnt < sizeof(gFileBuffer))
            break;
    }
    (void)Lib_FileClose(fd);
#endif
    
    iRet = SPF_GetDownloadOK(iAppNum, (char *)abyTemp); //add 
    //if(memcmp((BYTE *)(StartAddr-10),APP_DOWNLOAD_OK_FLAG,10))
    if( iRet ||   memcmp(abyTemp,APP_DOWNLOAD_OK_FLAG,10) )
    {
        //Lib_LcdPrintfCE("应用程序不完整", "App check err");
        Lib_LcdPrintxy(0, 8*2,0x00, "App check err");
        Lib_KbFlush();
        Lib_KbGetCh();
        Lib_LcdClrLine(2*8/2,LCD_HIGH_MINI-1);
	 WriteLog("App Check Error\n", 1, 0xff);
        for(;;);
        //return;
    }

#ifndef BPK_DEBUG
    Lib_Hash((BYTE *)EntryAddr, iFileLen-40, abyTemp);
    pbyAddr = (BYTE *)(StartAddr-40);
    if (0 != memcmp(pbyAddr,abyTemp,20))
    {
        //Lib_LcdPrintfCE("应用程序Hash错", "App hash error");
		Lib_LcdPrintxy(0, 8*2,0x00, "App hash error");
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8/2,LCD_HIGH_MINI-1);
		WriteLog("App Hash Error\n", 1, 0xff);
        return;
    }
    iRet = s_ReadAuthKey(strAppName, AUTHAPP_TYPE, 0, &byKeyLen, abyTemp);
    if ((0!=iRet) || (24!=byKeyLen))
    {
        //Lib_LcdPrintfCE("应用签名密钥错误", "Read sign key err");
		Lib_LcdPrintxy(0, 8*2,0x00, "Read sign key err");
  //      Lib_Lcdprintf(",%d,%d,", byKeyLen,iRet);
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8/2,LCD_HIGH_MINI-1);
		WriteLog("Read Sign Key Error\n", 1, 0xff);
        return;
    }
    s_DesMac24(pbyAddr, abyTemp, abyResult);
/*    Lib_Des24(pbyAddr, abyResult, abyTemp, 1);
    for (i=0; i<8; i++)
    {
        abyResult[i] ^= pbyAddr[i+8];
    }
    Lib_Des24(abyResult, abyData, abyTemp, 1);
    for (i=0; i<8; i++)
    {
        abyData[i] ^= pbyAddr[i+16];
    }
    Lib_Des24(abyData, abyResult, abyTemp, 1);*/
    pbyAddr = (BYTE *)(StartAddr-16);
    if (0 != memcmp(abyResult,pbyAddr,6))
    {
        //Lib_LcdPrintfCE("应用签名校验错误", "Signkey check err");
		Lib_LcdPrintxy(0, 8*2,0x00, "Signkey check err");
        Lib_KbFlush();
        (void)Lib_KbGetCh();
        Lib_LcdClrLine(2*8/2,LCD_HIGH_MINI-1);
		WriteLog("Signkey Check Error\n", 1, 0xff);
        return;
    }

#endif
 
    g_byCurAppNum = iAppNum;  // 运行应用程序 move
   
#if 0
    Down_Close();
    StartAddr = EntryAddr;
    GetAppEntry((uchar *)EntryAddr, &EntryAddr, NULL, &dwAppInfo);
    pAppMsg = (APP_MSG *)(APP_RUN_SPACE+dwAppInfo);
    pAppMsg->Num = iAppNum;
    iRet = SPF_GetAppInfo(&sAppInfo, iAppNum);
    if (0 == iRet)
    {
        ConvertBcdTimeToStr(sAppInfo.DownloadTime, pAppMsg->LoadTime);
    }
    EntryAddr += StartAddr;
    s_InitDll();
    Lib_KbFlush();
    (void)Lib_LcdSetFont(8, 16, 0);

    g_byCurAppNum = iAppNum;  // 运行应用程序
	//ClearIDCache();

	//__syscall(3);
	//switch_to_user_mode_new(EntryAddr);
#ifndef DLL_NOT_SWI
	switch_to_user_mode(EntryAddr);
#endif

	ClearIDCache();
    CallMain = (void (*)(void))EntryAddr;
    CallMain();
    ClearIDCache();
    g_byCurAppNum = 0xff;  // 运行VOS
    
	WriteLog("Load App OK\n", 1, 0xff);
	Down_Open();
#endif    
        
    return;
}

/****************************************************************************
  函数名     :  int SPF_GetSN(BYTE *pbySN)
  描述       :  查询序列号
  输入参数   :  无
  输出参数   :  1、BYTE *pbySN：基本序列号，最长32个字节
            注意：输入的数组长度必须不小于33个字节
  返回值     :  SPF_SUCCESS / SPF_PARA_ERROR / SPF_CREATE_FILE_ERROR
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_GetSN(BYTE *pbySN)
{
    T_SYSTEM_PATA tSystempara;
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    memcpy(pbySN,tSystempara.bSN,SERIAL_NUMBER_LEN);
  
#if 0 //12/10/28
    int iRet = SPF_SUCCESS;

    if (NULL == pbySN)
    {
        return SPF_PARA_ERROR;
    }

    READ_UPDATE(iRet);

    memcpy(pbySN, s_abyBuff, SERIAL_NUMBER_LEN);
    pbySN[SERIAL_NUMBER_LEN] = 0;
#endif    
    return SPF_SUCCESS;
}

/****************************************************************************
  函数名     :  int SPF_SetSN(const BYTE *pbyBaseSN)
  描述       :  修改序列号
  输入参数   :  1、const BYTE *pbySN：基本序列号，
                注意：数组pbySN的长度必须不小于33个字节
  输出参数   :  无
  返回值     :  SPF_SUCCESS / SPF_PARA_ERROR / SPF_CREATE_FILE_ERROR
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-28  V1.0         创建
****************************************************************************/
int SPF_SetSN(const BYTE *pbySN)
{
    T_SYSTEM_PATA tSystempara;
    sys_flash_syspara_read(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    if (NULL == pbySN)
    {
        return SPF_PARA_ERROR;
    }
    memcpy(tSystempara.bSN, pbySN, SERIAL_NUMBER_LEN);
    return sys_flash_syspara_write(SYSPARA_M,(BYTE *)&tSystempara,sizeof(T_SYSTEM_PATA));
    
    //return 0;
#if 0    //12/10/28
    int iRet = SPF_SUCCESS;

    if (NULL == pbySN)
    {
        return SPF_PARA_ERROR;
    }

    READ_UPDATE(iRet);

    memcpy(s_abyBuff, pbySN, SERIAL_NUMBER_LEN);

    return _SPF_WriteFile();
#endif    
}

int Lib_ReadSN(uchar *SerialNo)
{
#if 0
    int i,fd;
    unsigned char TmpSN[40];//32->40 13/01/28

    fd = SPF_GetSN(TmpSN);
    if(fd < 0)
    {
        SerialNo[0]=0;
        return -1;
    }
    for(i=0;i<40;i++) //32->40
    {
        if(!TmpSN[i])
            break;
        else
            SerialNo[i]=TmpSN[i];
    }
    SerialNo[i]=0;  
    return 0;
#else
    return SPF_GetSN(SerialNo);
#endif
}

int Lib_WriteSN(BYTE *SN)
{
#if 0
    int fd;
    unsigned char TmpSN[41];//32->40 13/01/28
    memset(TmpSN,0,41);
    strcpy((char *)TmpSN,(char *)SN);
    fd = SPF_SetSN(TmpSN);
    if(fd < 0)
    {
        return -1;
    }
    return 0;
#else
    return SPF_SetSN(SN);
#endif
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
//void ShowVosVersion(void)
extern unsigned long LCDValue;
int ShowVosVersion(void)

{
    int i;
    uchar ret;
    uchar sn[41], VerInfo[9];//33->41

    memset(sn, 0, 41);
    memset(VerInfo, 0, 9);
    SPF_GetSN(sn);
#if 0    
    if (fd < 0)
    {
        sn[0] = 0;
    }
    else
    {
        sn[12] = 0;
    }
#endif

    Lib_LcdCls();
    /*Lib_LcdSetFont(8, 16, 0);
    (void)Lib_LcdSetAttr(1);
    Lib_LcdGotoxy(0, 0);
    Lib_LcdPrintfCE("    版本信息    " , "      Version Info    ");
    (void)Lib_LcdSetAttr(0);
    Lib_LcdGotoxy(0, 2*8/2);
    Lib_LcdPrintfCE("  1-NAME    ", " NAME: VPOS35S ");
    Lib_LcdGotoxy(0, 4*8/2);
    Lib_LcdPrintfCE("  2-VERINFO    ", " BOOT: (1.0) ");
    Lib_LcdGotoxy(0, 6*8/2);
    Lib_LcdPrintfCE("  3-VERNO    ", " VOS : (1.0) ");*/
	Lib_LcdPrintxy(0,8*0,0x80,"     Version Info   ");
        
	//Lib_LcdPrintxy(0,8*1,0x00,"NAME:V10P BOOT:2.13" ); //2.13 2.23 13/06/27 
#ifdef V10X
        Lib_LcdPrintxy(0,8*1,0x00,"NAME:V10" ); //sxl 13/10/09 //lb,2.33->2.38,20150929
#else
        Lib_LcdPrintxy(0,8*1,0x00,"NAME:V10" ); //sxl 13/10/09 //lb,2.34->2.38,20150929
#endif

#if 1         
        Lib_LcdPrintxy(0,8*2,0x00, "SN:");
        Lib_LcdSetAttr(0);
        Lib_LcdSetFont(8, 16, 0);
        Lib_LcdGotoxy(3*6, 8*2);
        i = 2;
        while (1) {
          if( (sn[i]>='0') && (sn[i]<='9') || 
              ((sn[i]>= 'a') && (sn[i]<='f')) || 
              ((sn[i]>= 'A') && (sn[i]<='F')))
            Lib_Lcdprintf("%c",sn[i++]);
          else
            break;
          
          if(LCDValue >= 3000)  //LCD_AD_FLAG = 3000
          {
                if (i >= 39)
                    break;
          }
          else
          {
              if (i >= 40)
                break;

          }
          //if (i >= 40)
          //  break;
        }
#endif 
#if 0
    Lib_LcdCls();
    Lib_LcdPrintxy(0, 0*8, 0x00,"LCDValue:%d",LCDValue);
    Lib_KbGetCh();
    
    sn[39] = 0;
    if( (sn[0]>='0') && (sn[0]<='9'))
    {
        Lib_LcdPrintxy(0,2*8,0x00, "SN:");
        for(i=2;i<19;i++)
            Lib_LcdPrintxy((i+1)*6,2*8,0x00, "%c",sn[i]);     
        Lib_LcdPrintxy(0,3*8,0x00, "%s",&sn[19]);
    }
#endif       
        
        
#if 0        
	/*Version specified by makefile*/
	#ifdef FIRMWARE_VERSION
	    Lib_LcdPrintxy(0,3*8,0," VOS : (%d.%d.%d)", FIRMWARE_MAJOR_VERSION, FIRMWARE_MINOR_VERSION, FIRMWARE_MODIFIED_VERSION);
	#else
	    Lib_LcdPrintxy(0,3*8,0," VOS : (%d.%d.%d)", 0, 0, 0);
	#endif	
#endif
            
    /*if(sn[0])
    {
        Lib_LcdPrintxy(0,5*8,0," S/N : %s", sn);
    }
    else
    {
        Lib_LcdPrintxy(0,5*8,0," S/N : NULL");
    }
    if(!VerInfo[5])
    { 
		Lib_LcdPrintxy(0,6*8,0," Main: **  "); 
    }
    else
    { 
		Lib_LcdPrintxy(0,6*8,0," Main: %02d  ",VerInfo[5]); 
    }*/
    DrawThreeLine();

    (void)Lib_LcdSetFont(8, 16, 0);
    ret = Lib_KbGetCh();
    if (ret == KEYCANCEL)
    {
        if (0 == CheckVOSSignKey())
        {
            LoadApp();
            return 0xff;
        }
        return 0;
    }
    return 0;
}

/****************************************************************************
  函数名     :  DWORD ManageLogin(void)
  描述       :  1、用户登录处理函数
  输入参数   :  无
  输出参数   :  无
  返回值     :  AM_SUCCESS / AM_USER_PRESS_CANCEL
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-28  V1.0         创建
****************************************************************************/
DWORD ManageLogin(void)
{
#ifdef VOSSIGN_ENABLE    
    DWORD dwRet;
    while (1)
    {
        dwRet = AM_Login(AM_MANAGE_NO1);
        if (AM_SUCCESS == dwRet)
        {
			WriteLog("Login OK\n", 1, AM_MANAGE_NO1);
            break;
        }
		else if (AM_INPUT_TIME_OUT == dwRet)
		{
			Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 48/2);
            //Lib_LcdPrintfCE("用户输入超时！", "Input Timeout!");
			Lib_LcdPrintxy(0, 8*3,0x00,"Input Timeout!");
            Lib_DelayMs(1000);
			WriteLog("Login Failed\n", 1, AM_MANAGE_NO1);
			return AM_INPUT_TIME_OUT;
		}
        else if (AM_USER_PRESS_CANCEL == dwRet)
        {
			WriteLog("Login Failed\n", 1, AM_MANAGE_NO1);
            return AM_USER_PRESS_CANCEL;
        }
        Lib_DelayMs(3000);
    }
    while (1)
    {
        dwRet = AM_Login(AM_MANAGE_NO2);
        if (AM_SUCCESS == dwRet)
        {
			WriteLog("Login OK\n", 1, AM_MANAGE_NO2);
            return AM_SUCCESS;
        }
		else if (AM_INPUT_TIME_OUT == dwRet)
		{
			Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 48/2);
            //Lib_LcdPrintfCE("用户输入超时！", "Input Timeout!");
			Lib_LcdPrintxy(0, 8*3,0x00,"Input Timeout!");
            Lib_DelayMs(1000);
			WriteLog("Login Failed\n", 1, AM_MANAGE_NO2);
			return AM_INPUT_TIME_OUT;
		}
        else if (AM_USER_PRESS_CANCEL == dwRet)
        {
			WriteLog("Login Failed\n", 1, AM_MANAGE_NO2);
            return AM_USER_PRESS_CANCEL;
        }
        Lib_DelayMs(3000);
    }
#endif    
    return AM_SUCCESS;
}

/****************************************************************************
  函数名     :  void ShowMainMenu(BYTE *page_no)
  描述       :  显示操作选择提示菜单
  输入参数   :  BYTE *page_no : 页号
  输出参数   :  无
  返回值     :  1：文件对拷
                2：系统测试
                3：下载密钥
                4：修改密钥
                5：PED 解锁
                6: 设置对比度
                0xff：本地下载
                0x10：显示版本
                0x11：运行应用
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
1、   黄俊斌     2007-10-15  V1.0         创建
****************************************************************************/
int ShowSystemMenu(void)
{
    static int iCurPage = 0;
    int iPageChange = 1;
    BYTE ch;

    s_SetTimer(0,6000);  //modify 5mins to 1mins 
    while (1)
    {
      
        if(!s_CheckTimer(0))
        {
            Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
           /* Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE("    等待超时    ", "TimeOut For Load");*/
			Lib_LcdPrintxy(0,8*2,0, "TimeOut For Load");
            Lib_DelayMs(2000);
            return KM_WAIT_TIMEOUT_ERROR;
        } 
        
        if(!s_CheckTimer(3))
        {
            Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
           /* Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE("    等待超时    ", "TimeOut Quit");*/
			Lib_LcdPrintxy(0,8*2,0, "TimeOut Quit");
            Lib_DelayMs(2000);
            return KM_WAIT_TIMEOUT_ERROR;
        } 
        
        
        if (1 == iPageChange)
        {
            Lib_LcdCls();
            /*(void)Lib_LcdSetFont(8, 16, 0);
            (void)Lib_LcdSetAttr(1);
            Lib_LcdGotoxy(0, 0);
            Lib_LcdPrintfCE("    系统菜单    ", "     SYSTEM MENU     ");*/
			Lib_LcdPrintxy(0,8*0,0x80, "      SYSTEM MENU      ");

            (void)Lib_LcdSetAttr(0);
            switch (iCurPage)
            {
            case 0:
                /*Lib_LcdGotoxy(0, 2*8/2);
                Lib_LcdPrintfCE("  1-下载密钥    ", " 1-Load Key    ");

                Lib_LcdGotoxy(0, 4*8/2);
                Lib_LcdPrintfCE("  2-系统测试    ", " 2-SystemTest  ");

                Lib_LcdGotoxy(0, 6*8/2);
                Lib_LcdPrintfCE("  3-修改密码    ", " 3-Change Pwd  ");*/
				Lib_LcdPrintxy(0,8*1,0x00, " 1-Load Key    ");
				//Lib_LcdPrintxy(0,8*2,0x00, " 2-SystemTest  ");
				Lib_LcdPrintxy(0,8*2,0x00, " 2-Set LcdGray  ");
				Lib_LcdPrintxy(0,8*3,0x00, " 3-Change Pwd  ");

				break;
			case 1:
				/*Lib_LcdGotoxy(0, 2*8/2);
                Lib_LcdPrintfCE("  4-自检测试    ", " 4-Self Test   ");
				Lib_LcdGotoxy(0, 4*8/2);
                Lib_LcdPrintfCE("  5-设置对比度  ", " 5-Set LcdGray ");

                Lib_LcdGotoxy(0, 6*8/2);
                Lib_LcdPrintfCE("  6-上传日志    ", " 6-Send Log  ");*/
#if 0                          
				Lib_LcdPrintxy(0,8*1,0x00, " 4-Self Test   ");
				Lib_LcdPrintxy(0,8*2,0x00, " 5-Set LcdGray ");
#endif                                
				Lib_LcdPrintxy(0,8*3,0x00, " 6-Send Log    ");

                break;
            default:
                break;
            }
            DrawThreeLine();
            iPageChange = 0;
        }
        
        if (0 == CheckHandShack())
        {
            s_SetTimer(0,6000);  //modify 5mins to 1mins
            return 0xff;
        }
        
        
        if (0 == Lib_KbCheck())
        {
          
            s_SetTimer(0,6000);  //modify 5mins to 1mins
            ch = Lib_KbGetCh();
            
            
            switch (ch)
            {
            case KEYUP:
                if (iCurPage > 0)
                {
                    iCurPage--;
                }
                else
                {
                    iCurPage = 1;
                }
                iPageChange = 1;
                break;
            case KEYDOWN:
                if (iCurPage < 1)
                {
                    iCurPage++;
                }
                else
                {
                    iCurPage = 0;
                }
                iPageChange = 1;
                break; 
            case '1':
                if (0 == iCurPage)
                {
                    return 1;
                }
                break;
            case '2':
                if (0 == iCurPage)
                {
                    return 2;
                }
                break;
            case '3':
                if (0 == iCurPage)
                {
                    return 3;
                }
                break;
             case '4':
                if (1 == iCurPage)
                {
                    return 4;
                }
                break;
              
            case '5':
                if (1 == iCurPage)
                {
                    return 5;
                }
                break;
  
            case '6':
                if (1 == iCurPage)
                {
                    return 6;
                }
                break;
/*            case '7':
                if (2 == iCurPage)
                {
                    return 7;
                }
                break;*/
            case KEYENTER:
                return 0x10;
            case KEYCANCEL:
                return 0x11;
            default:
                break;
            }
        }
    }
}




// 0:fault, 1:ok
BYTE ISAllCharInString(BYTE *pAllChar, BYTE AllCharLen, BYTE *str, BYTE strLen)
{
    BYTE i, j;
    BYTE chFlag = 0;
    for (i=0; i<AllCharLen; i++)
    {
        chFlag = 0;
        for (j=0; j<strLen; j++)
        {
            if (pAllChar[i] == str[j])
            {
                chFlag = 1;
                break;
            }
        }
        if (0 == chFlag)
        {
            return 0;
        }
    }
    return 1;
}
void StringToHex(BYTE *pbyInData, int iInLen, BYTE *pbyOut)
{
    int i;

    if (0 != (iInLen%2))
    {
        pbyInData[iInLen] = 0;
        iInLen++;
    }
    for (i=0; i<iInLen; i+=2)
    {
        if ((pbyInData[i]>='A') && (pbyInData[i]<='F'))
        {
            pbyOut[i/2] = (BYTE)((pbyInData[i]-'A'+10)*16);
        }
        else if ((pbyInData[i]>='a') && (pbyInData[i]<='f'))
        {
            pbyOut[i/2] = (BYTE)((pbyInData[i]-'a'+10)*16);
        }
        else
        {
            pbyOut[i/2] = (BYTE)((pbyInData[i]-'0')*16);
        }

        if ((pbyInData[i+1]>='A') && (pbyInData[i+1]<='F'))
        {
            pbyOut[i/2] |= (BYTE)((pbyInData[i+1]-'A'+10)&0x0f);
        }
        else if ((pbyInData[i+1]>='a') && (pbyInData[i+1]<='f'))
        {
            pbyOut[i/2] |= (BYTE)((pbyInData[i+1]-'a'+10)&0x0f);
        }
        else
        {
            pbyOut[i/2] |= (BYTE)((pbyInData[i+1]-'0')&0x0f);
        }
    }
}

/****************************************************************************
  函数名     :  int InputPassword(BYTE *pbyOutPassword, BYTE byDisplayInfo, BYTE byKeyMode)
  描述       :  让用户输入密码
  输入参数   :  1、BYTE byDisplayInfo : 显示提示信息，可取值0~3，其他值不显示信息
                2、BYTE byKeyMode : 密码的显示方式，0表示明码方式显示，1表示密码方式显示
  输出参数   :  1、BYTE *pbyOutPassword : 8字节的密码
  返回值     :  SUCCESS / USER_INPUT_ERROR / USER_PRESS_KEYCANCEL
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-04-08  V1.0         创建
  1、 黄俊斌     2008-09-19  V1.1         移植到VPOS306
****************************************************************************/
DWORD InputPassword(BYTE *pbyOutPassword, BYTE byDisplayInfo, BYTE byKeyMode)
{
    BYTE byMode;
    BYTE abyTemp[20];
    int iRet;

    Lib_LcdClrLine(32/2, LCD_HIGH_MINI-1);
    //Lib_LcdGotoxy(0, 32/2);
    switch (byDisplayInfo)
    {
    case 0:
        //Lib_LcdPrintfCE(" 输入密码:", " Password:");
		Lib_LcdPrintxy(0, 8*2,0x00," Password:");
        break;
    case 1:
        //Lib_LcdPrintfCE(" 确认密码:", " Confirm Pwd:");
		Lib_LcdPrintxy(0, 8*2,0x00," Confirm Pwd:");
        break;
    case 2:
        //Lib_LcdPrintfCE(" 输入新密码:", " New Pwd:");
		Lib_LcdPrintxy(0, 8*2,0x00," New Pwd:");
        break;
    case 3:
        //Lib_LcdPrintfCE(" 确认新密码:", " Confirm Pwd:");
		Lib_LcdPrintxy(0, 8*2,0x00," Confirm New Pwd:");
        break;
    case 4:
        //Lib_LcdPrintfCE(" 输入原密码:", " Old Pwd:");
		Lib_LcdPrintxy(0, 8*2,0x00," Old Pwd:");
        break;
    }
    if (0 == byKeyMode)
    {
        byMode = 0x55;  // 明码方式
    }
    else
    {
        byMode = 0x5d;  // 密码方式
    }
    while (1)
    {
        Lib_LcdClrLine(48/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 48/2);
        //Lib_Lcdprintf("[                 ]");
        //Lib_LcdGotoxy(8, 48);
		Lib_LcdGotoxy(0, 48/2);
        abyTemp[0] = 0;
        iRet = Lib_KbGetStr(abyTemp, AM_MANAGE_PWD_LEN*2,
            AM_MANAGE_PWD_LEN*2, byMode, 60);//change 120 to 60
        if (KB_UserCancel_Err == iRet)
        {
            return AM_USER_PRESS_CANCEL;
        }
		else if(KB_InputTimeOut_Err == iRet)
		{
			return AM_INPUT_TIME_OUT;
		}

        else if (0 == iRet)
        {
            if (1 == ISAllCharInString(abyTemp, AM_MANAGE_PWD_LEN*2,
                "0123456789abcdefABCDEF", 22))
            {
                break;
            }
           /* Lib_LcdClrLine(48/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 48/2);
            Lib_LcdPrintfCE("用户输入非法字符", "Input error char");*/
			Lib_LcdClrLine(8*3, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 8*3,0x00,"Input error char");
        }
        else
        {
            /*Lib_LcdClrLine(48/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 48/2);
            Lib_LcdPrintfCE("用户输入错误", "Input error!");*/
			Lib_LcdClrLine(8*3, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0, 8*3,0x00,"Input error!");
        }
        Lib_KbGetCh();
    }
    StringToHex(abyTemp, AM_MANAGE_PWD_LEN*2, pbyOutPassword);
    return AM_SUCCESS;
}

// 输出参数以明文输出
DWORD AM_ReadManageKey(int iManageNo, MANAGE_KEY *psManageKey)
{
    BYTE abyCrc32[4];
    int iRet;
    DWORD dwRet;
    BYTE abyMMK1[24];
    MAN_APP_KEY loc_sMaKey;

    if (iManageNo >= MANAGE_NUMBER)
    {
        return AM_MANAGE_NOT_EXIST;
    }
    dwRet = s_AM_ReadMaKey(&loc_sMaKey);
    if (AM_SUCCESS != dwRet)
    {
        return dwRet;
    }
    memcpy(psManageKey, &(loc_sMaKey.asManKey[iManageNo]), sizeof(MANAGE_KEY));
    memset(&loc_sMaKey, 0, sizeof(MAN_APP_KEY));
    //if (0 != memcmp(&g_wValidateFlag,psManageKey->abyFlag,2))
    if (0 != memcmp("\x55\x55",psManageKey->abyFlag,2))
    {
        return AM_INVALIDATE;  // 密钥未初始化
    }
    QuickCrc32((BYTE*)psManageKey, sizeof(MANAGE_KEY)-4, abyCrc32);
    if (0 != memcmp(abyCrc32,psManageKey->abyCrc32,4))
    {
        return AM_CRC_CHECK_ERROR;
    }
    iRet = s_PciReadMMK(MA_MMK_INDEX, (DWORD*)abyMMK1);
    if (AM_SUCCESS != iRet)
    {
        return AM_READ_MMK_ERROR;
    }
    Lib_DES3_16(psManageKey->abyKeyData, abyMMK1, DECRYPT);
    Lib_DES3_16(&(psManageKey->abyKeyData[8]), abyMMK1, DECRYPT);
    Lib_DES3_16(&(psManageKey->abyKeyData[16]), abyMMK1, DECRYPT);
    memset(abyMMK1, 0, sizeof(abyMMK1));
    return AM_SUCCESS;  
}

/****************************************************************************
  函数名     :  DWORD AM_InputNewPassword(BYTE *pbyOutPassword, BYTE byKeyMode)
  描述       :  获取操作员密码
  输入参数   :  1、BYTE byKeyMode : 密码的显示方式，0表示明码方式显示，1表示密码方式显示
  输出参数   :  1、BYTE *pbyOutPassword : 操作员密码，占8个字符?
                     输出值为"00000000"~"00999999"
  返回值     :  AM_SUCCESS / AM_USER_PRESS_CANCEL / AM_USER_INPUT_ERROR
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-04-08  V1.0         创建
****************************************************************************/
DWORD AM_InputNewPassword(BYTE *pbyOutPassword, BYTE byKeyMode)
{
    BYTE abyTemp[20],
        byRetryTimes = 0;
    DWORD dwRet;
    while (1)
    {
        byRetryTimes++;
        if (byRetryTimes > 3)
        {
            return AM_USER_INPUT_ERROR;
        }
        dwRet = InputPassword(pbyOutPassword, 2, byKeyMode);
        if (AM_USER_PRESS_CANCEL == dwRet)
        {
            return AM_USER_PRESS_CANCEL;
        }
		else if (AM_INPUT_TIME_OUT == dwRet)
		{
			return AM_INPUT_TIME_OUT;
		}

        else if (AM_SUCCESS != dwRet)
        {
            Lib_LcdClrLine(32/2, LCD_HIGH_MINI-1);
           /* Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE(" 输入错误", "  Input error!");
            Lib_LcdGotoxy(0, 48/2);
            Lib_LcdPrintfCE(" 请重新输入！", "  Reinput PWD");*/
			Lib_LcdPrintxy(0, 8*2,0x00,"  Input error!");
			Lib_LcdPrintxy(0, 8*3,0x00,"  Reinput PWD");
            (void)Lib_KbGetCh();
            continue;
        }
        dwRet = InputPassword(abyTemp, 3, byKeyMode);
        if (AM_USER_PRESS_CANCEL == dwRet)
        {
            return AM_USER_PRESS_CANCEL;
        }
		else if (AM_INPUT_TIME_OUT == dwRet)
		{
			return AM_INPUT_TIME_OUT;
		}
        else if (AM_SUCCESS != dwRet)
        {
            Lib_LcdClrLine(32/2, LCD_HIGH_MINI-1);
           /* Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE(" 输入错误", " Input error!");
            Lib_LcdGotoxy(0, 48/2);
            Lib_LcdPrintfCE(" 请重新输入！", " Reinput PWD");*/
			Lib_LcdPrintxy(0, 8*2,0x00,"  Input error!");
			Lib_LcdPrintxy(0, 8*3,0x00,"  Reinput PWD");
            (void)Lib_KbGetCh();
            continue;
        }
        if (0 != memcmp(pbyOutPassword,abyTemp,AM_MANAGE_PWD_LEN))
        {
            Lib_LcdClrLine(32/2, LCD_HIGH_MINI-1);
            /*Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE(" 两次输入不同", " Input error!");
            Lib_LcdGotoxy(0, 48/2);
            Lib_LcdPrintfCE(" 请重新输入！", " Reinput PWD");*/
			Lib_LcdPrintxy(0, 8*2,0x00,"Input error!");
			Lib_LcdPrintxy(0, 8*3,0x00,"  Reinput PWD");
            (void)Lib_KbGetCh();
            continue;
        }
        return AM_SUCCESS;
    }
}

DWORD AM_SaveManageKey(int iManageNo, MANAGE_KEY *psManageKey)
{
    //BYTE abyCrc32[4];
    int iRet;
    BYTE abyMMK1[24];
    MAN_APP_KEY g_sMaKey;
    
    if (iManageNo >= MANAGE_NUMBER)
    {
        return AM_MANAGE_NOT_EXIST;
    }
    (void)s_AM_ReadMaKey(&g_sMaKey);
    //memcpy(psManageKey->abyFlag, &g_wValidateFlag, 2);
    memcpy(psManageKey->abyFlag, "\x55\x55", 2);
    iRet = s_PciReadMMK(MA_MMK_INDEX, (DWORD*)abyMMK1);
    if (AM_SUCCESS != iRet)
    {
        return AM_READ_MMK_ERROR;
    }
    Lib_DES3_16(psManageKey->abyKeyData, abyMMK1, ENCRYPT);
    Lib_DES3_16(&(psManageKey->abyKeyData[8]), abyMMK1, ENCRYPT);
    Lib_DES3_16(&(psManageKey->abyKeyData[16]), abyMMK1, ENCRYPT);
    memset(abyMMK1, 0, sizeof(abyMMK1));
    QuickCrc32((BYTE*)psManageKey, sizeof(MANAGE_KEY)-4, psManageKey->abyCrc32);
    memcpy(&(g_sMaKey.asManKey[iManageNo]), psManageKey, sizeof(MANAGE_KEY));
    s_AM_WriteMaKey(&g_sMaKey);
    memset(&g_sMaKey, 0, sizeof(MAN_APP_KEY));
    return AM_SUCCESS;
}


void AM_ChangePassword(void)
{
    BYTE byRet, abyPwd[10], abyTemp[10];
    DWORD dwRet;
    int iManageNo, iErrorTimes;
    MANAGE_KEY sManageKey;

    Lib_LcdCls();
    /*Lib_LcdGotoxy(0, 0);
    Lib_LcdSetAttr(1);
    Lib_LcdPrintfCE("   管理员选择   ", " Manage select  ");
    Lib_LcdSetAttr(0);
    Lib_LcdGotoxy(0, 16/2);
    Lib_LcdPrintfCE(" 1.管理员A     ", " 1. Manage A  ");
    Lib_LcdGotoxy(0, 48/2);
    Lib_LcdPrintfCE(" 2.管理员B     ", " 2. Manage B  ");*/
	Lib_LcdPrintxy(0, 8*0,0x80,"    Manage select    ");
	Lib_LcdPrintxy(0, 8*1,0x00," 1. Manage A  ");
	Lib_LcdPrintxy(0, 8*2,0x00," 2. Manage B  ");
    while (1)
    {
        byRet = Lib_KbGetCh();
        if (KEY1 == byRet)
        {
            iManageNo = AM_MANAGE_NO1;
            break;
        }
        else if (KEY2 == byRet)
        {
            iManageNo = AM_MANAGE_NO2;
            break;
        }
        else if (KEYCANCEL == byRet)
        {
            return ;
        }
    }
    iErrorTimes = 0;
    while (1)
    {
        if (iErrorTimes >= 3)
        {
			WriteLog("Change Password FAILED!\n", 1, iManageNo);
            return ;
        }
        Lib_LcdCls();
       /* Lib_LcdGotoxy(0, 0);
        Lib_LcdSetAttr(1);
        Lib_LcdPrintfCE(" 管理员修改密码 ", "Manage changekey");
        Lib_LcdSetAttr(0);*/
		Lib_LcdPrintxy(0, 8*0,0x80,"  Manage changekey  ");
        dwRet = InputPassword(abyPwd, 4, 1);

      /*  Lib_LcdClrLine(20/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 32/2);*/
		 Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
        if (AM_USER_PRESS_CANCEL == dwRet)
        {
            //Lib_LcdPrintfCE("  用户取消输入  ", "  User cancel   ");
			Lib_LcdPrintxy(0, 8*2,0x00, "  User cancel   ");
            Lib_KbGetCh();
 			WriteLog("Change Password FAILED!\n", 1, iManageNo);
            return ;
        }
		else if (AM_INPUT_TIME_OUT == dwRet)
		{
			//Lib_LcdPrintfCE("  用户输入超时  ", "  Input TimeOut ");
			Lib_LcdPrintxy(0, 8*2,0x00,  "  Input TimeOut ");
            Lib_KbGetCh();
 			WriteLog("Change Password FAILED!\n", 1, iManageNo);
            return ; 
		}
        else if (AM_USER_INPUT_ERROR == dwRet)
        {
            //Lib_LcdPrintfCE("    输入错误    ", "  Input error   ");
			Lib_LcdPrintxy(0, 8*2,0x00, "  Input error   ");
            Lib_KbGetCh();
			WriteLog("Change Password FAILED!\n", 1, iManageNo);
            return ;
        }
        dwRet = AM_ReadManageKey(iManageNo, &sManageKey);
        memcpy(abyTemp, sManageKey.abyKeyData, 8);
        if (AM_MA_KEY_NOT_EXIST == dwRet)
        {
            memset(abyTemp, 0, 8);
        }
        if (0 == memcmp(abyPwd, abyTemp, 8))
        {
            break;
        }
        //Lib_LcdPrintfCE("    密码错误    ", "  Pwd error    ");
		Lib_LcdPrintxy(0, 8*2,0x00, "  Pwd error    ");
        Lib_KbGetCh();
        iErrorTimes++;
    }
  /*  Lib_LcdClrLine(20/2, LCD_HIGH_MINI-1);
    Lib_LcdGotoxy(0, 32/2);*/
	Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
    dwRet = AM_InputNewPassword(abyPwd, 1);
    //Lib_LcdClrLine(20/2, LCD_HIGH_MINI-1);
    //Lib_LcdGotoxy(0, 32/2);
	Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
    if (AM_USER_PRESS_CANCEL == dwRet)
    {
        //Lib_LcdPrintfCE("    用户取消    ", "   User cancel  ");
		Lib_LcdPrintxy(0, 8*2,0x00, "   User cancel  ");
        Lib_KbGetCh();
		WriteLog("Change Password FAILED!\n", 1, iManageNo);
        return ;
    }
    else if (AM_SUCCESS != dwRet)
    {
		WriteLog("Change Password FAILED!\n", 1, iManageNo);
        return ;
    }
    memcpy(sManageKey.abyKeyData, abyPwd, 8);
    sManageKey.byKeyLen = 8;
    dwRet = AM_SaveManageKey(iManageNo, &sManageKey);
    if (AM_SUCCESS != dwRet)
    {
        //Lib_LcdPrintfCE("    保存失败    ", "   Save failed   ");
		Lib_LcdPrintxy(0, 8*2,0x00, "   Save failed   ");
		WriteLog("Change Password FAILED!\n", 1, iManageNo);
    }
    else
    {
        //Lib_LcdPrintfCE("    保存成功    ", "   Save success  ");
		Lib_LcdPrintxy(0, 8*2,0x00,  "   Save success  ");
		WriteLog("Change Password OK!\n", 1, iManageNo);
    }
    Lib_KbGetCh();
}

void s_SaveGrayValue(BYTE level)
{
  BYTE buf[2]={0};
  buf[0] = 0xa5;
  buf[1] = level;
  sys_flash_syspara_write(LCD_GRAY_M,buf,2);
}

BYTE s_ReadGrayValue(void)
{
	//BYTE *startAddr;
	BYTE buf[2]={0};
	BYTE level;
#if 0  //tmp disable
	startAddr = (BYTE *)(0xA1028000); 
	hal_flash_read(buf, startAddr, 2);
	if (buf[0] != 0xa5) {
		return 0xff;
	}
#endif
        sys_flash_syspara_read(LCD_GRAY_M,buf,2);
        
        
        //test
        //buf[1] = 10;
#ifdef ST7567_M
    buf[1] = 19;
#else        
    buf[1] = 19;
#endif        
	level = buf[1];
    if (level < GRAY_MIN_LEVEL)
    {
        level = GRAY_MIN_LEVEL;
    }
    else if (level > GRAY_MAX_LEVEL)
    {
        level = GRAY_MAX_LEVEL;
    }

	return level;
}

//void AdjustLcdGrayValue()
int AdjustLcdGrayValue()
{
    BYTE byVal, byRet;
    //int y;

    (void)Lib_LcdSetFont(8, 16, 0);
    Lib_LcdCls();
    Lib_KbFlush();

     byVal = s_ReadGrayValue();
    if (byVal == 0xff)
		byVal = DEFAULT_GRAY_LEVEL;
	
    Lib_LcdCls();
    /*Lib_LcdPrintxyCE(0, 0, 0x81, "  设置显示灰度  " , "  SET LCD GRAY  ");
    Lib_LcdPrintxyExtCE(0, 16/2, 0x01, "   灰度（%02d）   ", "    Gray(%02d)    ", byVal);
    Lib_LcdPrintxyCE(0, 48/2, 0x01, "按'0'恢复默认值" , "Press 0 default");*/
	Lib_LcdPrintxy(0,8*0,0x80,"    SET LCD GRAY    " );
	Lib_LcdPrintxy(0,8*1,0x00,"     Gray(%02d)    ", byVal );
	Lib_LcdPrintxy(0,8*3,0x00,"Press 0 default" );
    /*for (y = 32/2; y < 48/2; y++)  //25   21
    {
        
		Lib_LcdDrawBox(16, y/2, byVal*2, y); 
    }
    Lib_LcdDrawBox(0,15,127,63/2);*/
    while (1)
    {
        byRet = Lib_KbGetCh();
        if ((byRet==KEYENTER) || (byRet==KEYMENU))
        {
        	s_SaveGrayValue(byVal);
            return 0;
        }
        if (byRet == KEYCANCEL)
        {
            LoadApp();
            return 0xff;
        }

        //　调整LCD的对比度
        if (byRet == KEYUP)
        {
            if (byVal < GRAY_MAX_LEVEL)
            {
                byVal++;
            }
        }
        else if (byRet == KEYDOWN)
        {
            if (byVal > GRAY_MIN_LEVEL)
            {
                byVal--;
            }
        }
        else if (byRet == KEY0)
        {
            byVal = DEFAULT_GRAY_LEVEL;
        }
        else
        {
            continue;
        } 
        Lib_LcdSetGray(byVal);
       /* (void)Lib_LcdPrintxyExtCE(0, 16/2, 0x01, "   灰度（%02d）", "    Gray(%02d)", byVal);*/
		Lib_LcdPrintxy(0,8*1,0x00,"     Gray(%02d)    ", byVal );
        /*Lib_LcdClrLine(32/2, 47/2);
        for (y = 32/2; y < 48/2; y++)  
        { 
			Lib_LcdDrawBox(16, y/2, byVal*2, y); 
        }
        Lib_LcdDrawBox(0,15,127,63/2);*/
    }
}

void SPF_CheckApp(void)
{
  
}

void SendLogStart()
{
  
#if 0  
    int fp;
    int iFileSize;
    BYTE byRet;
    BYTE abyRecvBuff[2100];
    BYTE abySendBuff[2100];
    WORD wRecvLen;
    DWORD dwOffset;
    int iRet;

    Lib_LcdCls();
    //Lib_LcdPrintxyCE(0, 0, 0x01, "上传日志", "Send Log file");
	Lib_LcdPrintxy(0,8*0,0x80,"   Send  Log  file   ");

    iFileSize = Lib_FileSize(OPERATE_LOG_FILE_NAME);
    if (iFileSize <= 0)
    {
       /* Lib_LcdPrintxyCE(0, 16/2, 0x01, "日志文件不存在", "No log file");*/
		Lib_LcdPrintxy(0,8*1,0x00, "No log file");
        (void)Lib_KbGetCh();
        return ;
    }

    fp = Lib_FileOpen(OPERATE_LOG_FILE_NAME, O_RDWR);
    if (fp < 0)
    {
        //Lib_LcdPrintxyCE(0, 16/2, 0x01, "打开日志文件失败", "Open log failed");
		Lib_LcdPrintxy(0,8*1,0x00, "Open log failed");
        (void)Lib_KbGetCh();
        return ;
    }
    g_iComBauds = 115200;
    iRet = Down_Open();
//    iRet = Lib_ComOpen(LOAD_COM, "115200,8,n,1");
    if (0 != iRet)
    {
        Lib_FileClose(fp);
        //Lib_LcdPrintxyCE(0, 16/2, 0x01, "打开串口失败", "Open COM failed");
		Lib_LcdPrintxy(0,8*1,0x00, "Open COM failed");
        (void)Lib_KbGetCh();
        return ;
    }
    (void)Lib_FileSeek(fp, 0, FILE_SEEK_SET);
    //Lib_LcdPrintxyCE(0, 0, 0x01, "日志上传文件", "Send Log file");
	Lib_LcdPrintxy(0,8*0,0x80, "  Send  Log  file    ");
    iRet = SendLogShackhand();
    if (0 != iRet)
    {
        /*Lib_LcdClrLine(2*8/2, 8*8/2);
        Lib_LcdPrintxyCE(0, 4*8/2, 0x01, "用户取消上传日志", "User cancelled!");*/
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00, "User cancelled!");
        Lib_FileClose(fp);
        Down_Close();
//        (void)Lib_ComClose(LOAD_COM);
        (void)Lib_KbGetCh();
        return ;
    }
  /*  Lib_LcdClrLine(2*8/2, 8*8/2);
    Lib_LcdPrintxyCE(0, 4*8/2, 0x01, "开始下载..", "Download file...");*/
	Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(0,8*2,0x00, "Download file...");

    while (1)
    {
        iRet = LrcRecvPacket(abyRecvBuff, &wRecvLen, 2000);
        if (0 == iRet)
        {
            switch (abyRecvBuff[1])
            {
            case CMD_GET_LOG_FILE_LENGTH:
                abySendBuff[0] = (BYTE)((iFileSize/65536)/256);
                abySendBuff[1] = (BYTE)((iFileSize/65536)%256);
                abySendBuff[2] = (BYTE)((iFileSize%65536)/256);
                abySendBuff[3] = (BYTE)(iFileSize%256);
                LrcSendPacket(abySendBuff, 4, CMD_GET_LOG_FILE_LENGTH);
                break;
            case CMD_GET_LOG_DATA:
                dwOffset = (abyRecvBuff[4]*256+abyRecvBuff[5])*65536
                    + abyRecvBuff[6]*256 + abyRecvBuff[7];
                if (dwOffset >= (DWORD)iFileSize)
                {
                    LrcSendPacket(abySendBuff, 0, CMD_GET_LOG_DATA);
                }
                else
                {
                    (void)Lib_FileSeek(fp, (long)dwOffset, FILE_SEEK_SET);
                    iRet = Lib_FileRead(fp, &abySendBuff[1], 1000);
                    if (iRet <= 0)
                    {
                        abySendBuff[0] = 1;
                        LrcSendPacket(abySendBuff, 1, CMD_GET_LOG_DATA);
                    }
                    else
                    {
                        abySendBuff[0] = 0;
                        LrcSendPacket(abySendBuff, (WORD)(iRet+1), CMD_GET_LOG_DATA);
                    }
                }
                break;
            case CMD_DELETE_LOG_FILE:
                Lib_FileClose(fp);
                iRet = Lib_FileRemove(OPERATE_LOG_FILE_NAME);
                if (0 == iRet)
                {
                    abySendBuff[0] = 0;
                }
                else
                {
                    abySendBuff[0] = 1;
                }
                LrcSendPacket(abySendBuff, 1, CMD_DELETE_LOG_FILE);
                break;
            case SEND_FINISH:
//                (void)Lib_ComClose(LOAD_COM);
               /* Lib_LcdClrLine(2*8/2, 8*8/2);
                Lib_LcdPrintxyCE(0, 4*8/2, 0x01, "上传日志成功!", "Send Logfile OK!");*/
				Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0,8*2,0x00, "Send Logfile OK!");
                (void)Lib_KbGetCh();
                return ;
            default:
                break;
            }
        }
    }
#endif
    
}

int s_AppPciInit(void)
{
    int i;
    uchar ch;
    int ret;
    //DWORD dret;
    

    if((SPF_GetAppStatus(0)&0x04)==0)//未运行
    {
      s_PciClearAppKey(0);
    }
    
    (void)s_PciInit(); 
    s_InitVos();

    (void)s_SystemTestSelf();
    
    g_byCurAppNum = 0xff; 
    
 
	//s_PciViewAuthKey(); 
    AM_CheckPedLock();

    
#if 0
    Lib_LcdCls();
    /*Lib_LcdSetFont(8, 16, 0);
    Lib_LcdGotoxy(0, 8*1);*/
    //Lib_LcdPrintfCE("按\"确认\"键查询", "Press\"ENTER\"key show ver.");
    Lib_LcdPrintxy(0, 8*1, 0, "Press\"ENTER\"key show ver.");
    //Lib_LcdGotoxy(0, 8*2);
    //Lib_LcdPrintfCE("按\"功能\"键设置", "Press \"F1\" key into set");
	 Lib_LcdPrintxy(0,8*2,0, "Press \"F1\" key into set");
#endif 
         
    i = 100;
    ch = 0;
    while (i > 0)
    {
        if (!Lib_KbCheck())
        {
            ch = Lib_KbGetCh();
            if (KEYENTER == ch)
            {
                Lib_KbFlush();
                if(ShowVosVersion()==0xff)
                  return 0xff;
            }
            else if (KEYF1 == ch)
            {
                break;
            }
        }
        Lib_DelayMs(20);
        i--;
    }
    
    
    Lib_LcdCls();
    Lib_LcdSetFont(8, 16, 0);
    SPF_CheckApp();
    
    
    
    if (i <= 0)
    {
        if (0 == CheckVOSSignKey())
        {
            LoadApp();
            return 0xff;
        }
    }

Loop_ReLogin:    
	s_SetTimer(3,90000);//15 minutes
        
    while (1)
    {
        if (0 == ManageLogin())
        {
	    g_iActionLimits=0;
            break;
        }
        if (0 == CheckVOSSignKey())
        {
            LoadApp();
            return 0xff;
        }
    }

    
    while (1)
    {
        Lib_KbFlush();
	Lib_ComReset(LOAD_COM);//12/12/12 disble
        
	//对进入系统菜单后的所有操作都视为敏感服务，对所有敏感服务的操作进行10次的限制，超过10次则退出系统菜单
	if (g_iActionLimits >= 10)
	{
            if (0 == CheckVOSSignKey())
            {
                LoadApp();
                return 0xff;
            }
            goto Loop_ReLogin;               
	}
        
        ret = ShowSystemMenu();


        if(ret == KM_WAIT_TIMEOUT_ERROR)
        {
            if (0 == CheckVOSSignKey())
            {
                LoadApp();
                return 0xff;
            }
            goto Loop_ReLogin;               
        } 
        switch (ret)
        {
          
        case 1:
            KM_LoadKey();
//            //if(dret == KM_WAIT_TIMEOUT_ERROR || dret == KM_SUCCESS)
//            //{
//			//下载密钥成功或失败都退回应用，每次只限一次下载密钥操作
//				if (0 == CheckVOSSignKey())
//				{
//					LoadApp();
//				}
//                goto Loop_ReLogin;               
//            //} 
	  g_iActionLimits++;
            break;
            
#if 0            
        case 2:
            (void)SystemTest();
	    g_iActionLimits++;
            break;
#endif            
        case 3:
            AM_ChangePassword();
	    g_iActionLimits++;
            break;
#if 0            
        case 4:
            s_SystemTestSelf();
	    g_iActionLimits++;
            break;
#endif            
	//case 5:
        case 2:  
	    AdjustLcdGrayValue();
	    g_iActionLimits++;
	    break;
	case 6:
	    SendLogStart();
	    g_iActionLimits++;
	    break;
        case 0x10:
            if(ShowVosVersion()==0xff)
              return 0xff;
            break;
        case 0x11:
            while (1)
            {
                if (0 == CheckVOSSignKey())
                {
                    LoadApp();
                    return 0xff;
                }
                if (0 == ManageLogin())
                {
					g_iActionLimits=0;
                    break;
                }
            }
            break;
//        default:  // 1和0xff
#if 0            
	case 0xff:
            MyLocalDownload();
            if (115200 != g_iComBauds)
            {
                g_iComBauds = 115200;
                (void)Down_Open();
            }
            SPF_CheckApp();
	    g_iActionLimits++;
            break;
#endif            
        }
    }    
  
}

void _SetLock_EraseApp(void)
{
#if 0
        unsigned char bVal,sBuf[2048]={0};
        RM_flash_erase_page(0);
        RM_flash_erase_page(1);
        RM_flash_erase_page(2);
        RM_flash_erase_page(3);
        RM_flash_erase_page(4);
        RM_flash_erase_page(5);
        RM_flash_erase_page(6);
        memset(sBuf,0,sizeof(sBuf));
        flash_write_operate(0x08000000+APPLICATIONADDRESS,(unsigned short*)sBuf,2048);
#endif
       s_SetLockState(1); 
  
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int  Lib_AppInit(void)
{
    //initial_system();
  int fd;
  static unsigned char bRun=0;
  unsigned char sBuf[10]={0};
  
  //第二次调用
  if(bRun++>0)
  {
    bRun = 1;
    return 0xff;
  }    
  
//#if 0  
    initial_system();
//#endif    
//trace_debug_printf(" initial_system end");
  

  
    s_AppPciInit();
//trace_debug_printf(" s_AppPciInit end");
   // Lib_ComOpen(0,"115200,8,N,1");



    s_PciClearAuthFlag();
    
  fd = SPF_GetAppRunStatus(g_byCurAppNum);
  if (fd < 0)
    return 0;

  if(fd==0)//未运行过
  {
      
    SPF_SetAppRunStatus(g_byCurAppNum, 1);
    
      WriteToBackupReg(5,0xA5A5); //13/04/20
    
  }
  
  s_GetLockStateBuf(sBuf);
  //test
  //Lib_LcdPrintxy(0, 0*8, 0x00, "Get[%s]",sBuf);
  //Lib_DelayMs(3000);
    
  if(memcmp(sBuf,"UNLOCKER",8)==0) //system unlock
  {
      WriteToBackupReg(5,0xA5A5); //13/04/20
      s_SetLockState(0);
  }  
      //13/04/20
//#if 0 //move to boot
#if 1   //chang by wf
      if(CheckBackupReg(5,0xA5A5)) //开盖触发
      {
	//Lib_LcdCls();
	Lib_LcdPrintxy(0, 2*8, 0x00, "DEVICE LOCKED!!!");
        //SPF_SetAppRunStatus(g_byCurAppNum, 1);//下次为第一次
        _SetLock_EraseApp();
        
        TAMPERClear();  //13/05/25
        
        WriteToBackupReg(5,0xA5A5); //13/04/20
        SPF_SetAppDownUnLockStatus(0);
        for(;;);
      }
#endif
//#endif
  
  return 0;
}

void   Lib_DelayMs(int ms)
{
    if(ms<=0)
      return;
    delay_ms(ms);
}

//#if 0 //gplian
#ifndef AS3911_M
int  Lib_PiccOpen(void)
{
  return PiccOpen();
}

void Lib_PiccHalt(void)
{
}

int  Lib_PiccClose(void)
{
  PiccClose();
  return 0;  
}

int  Lib_PiccRemove(void)
{
  unsigned char bRet;
  bRet= PiccRemove('R',0);
  if(bRet==2) //RET_RF_ERR_NO_OPEN
    return PICC_NotOpen;
  else if(bRet==6)
    return PICC_HaveCard;
  return bRet;
  
}
#if 0
int   Lib_IccCheck(uchar slot)
{
  return 0;
}
#endif

int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
{
  unsigned char Other[50];
  unsigned char bRet;
  //13/01/16 for app
  if(mode==3)
    mode = 0;
    //13/08/09
  PN512_s_RFResetCarrier();
  
  //bRet = PiccDetect(mode,CardType,SerialNo,NULL,Other);
  bRet = PiccDetect(0,CardType,SerialNo,NULL,Other);//13/02/19 the same as before
  if(bRet==2)
    return PICC_NotOpen;
  else if(bRet==1)//RET_RF_ERR_PARAM
    return PICC_ParameterErr;
  else if(bRet==4)//multi card 13/08/02
    return PICC_Collision;
  return bRet;
  //return   PiccDetect(0,CardType,SerialNo,NULL,Other);
  
}

int  Lib_PiccReset(void)
{
  //PN512_s_RFResetCarrier();
  unsigned char bRet;
  bRet = PiccReset();
  if(bRet==1)
    return PICC_NotOpen;
  else if(bRet==2)
    return PICC_CardTyepErr;
  return bRet;
}

int  Lib_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp)
{
  unsigned char bRet;
  bRet = PiccIsoCommand(0,ApduSend,ApduResp);
  if(bRet==2)
    return PICC_NotOpen;
  return bRet;
  
}
#endif

//#endif


/*********************************************************************
 NAME: uchar dat_bytetobcd(uchar bchar)
 FUNC: convert one uchar to bcd(bchar<100)
 IN  : bchar:to be converted
 OUT : 
 RTRN: the result converted
 NOTE:for example: convert 12 ->  0x18 ( because 0x12=18)
*********************************************************************/
static uchar dat_bytetobcd(uchar bchar)
{
	unsigned char bcd_code, btmp;

	btmp = bchar/10;
	bcd_code = btmp*16;
	bcd_code += bchar-btmp*10;
	return(bcd_code);
}

int    Lib_GetDateTime(uchar *datetime)
{
  
  struct tm tDate;
  int nSec,nHour,nHourOth;
  //uchar tm_hour,tm_min,tm_sec;
  uchar sBuf[10];
  //gettime(&tDate);
  
  nSec = RTC_GetCounter();
  nHour = (nSec)/3600;
  nHourOth = (nSec)%3600;
    
  tDate.tm_hour = (nHour);
  tDate.tm_min =  (nHourOth/60);
  tDate.tm_sec =  (nHourOth%60);
  
  DateGetYYMMDD(sBuf);
  tDate.tm_year = sBuf[0]  ;
  tDate.tm_mon = sBuf[1];
  tDate.tm_mday = sBuf[2];
  
  datetime[0] = dat_bytetobcd(tDate.tm_year);
  datetime[1] = dat_bytetobcd(tDate.tm_mon);
  datetime[2] = dat_bytetobcd(tDate.tm_mday);
  datetime[3] = dat_bytetobcd(tDate.tm_hour);
  datetime[4] = dat_bytetobcd(tDate.tm_min);
  datetime[5] = dat_bytetobcd(tDate.tm_sec);
   
  CheckTime(datetime,datetime+6);
  
  return 0;
}

int CheckBCD(unsigned char str)
{
    unsigned char ch;
    ch=str & 0x0f;
    if(ch>=0x0a) return TIME_FORMAT_ERR;
    ch=str & 0xf0;
    if(ch>=0xa0) return TIME_FORMAT_ERR;
    return 0;
}

int CheckTime(uchar *str,uchar *week)
{
    long days;
    uchar day,ucBuf[12];
    int i,j,k;
    uchar ret=0;

    if(CheckBCD(str[0])) return TIME_YEAR_ERR;         //ret |= 0x01;
    if(CheckBCD(str[1])) return TIME_MONTH_ERR;        //ret |= 0x02;
    if(CheckBCD(str[2])) return TIME_DAY_ERR;          //ret |= 0x04;
    if(CheckBCD(str[3])) return TIME_HOUR_ERR;         //ret |= 0x08;
    if(CheckBCD(str[4])) return TIME_MINUTE_ERR;       //ret |= 0x10;
    if(CheckBCD(str[5])) return TIME_SECOND_ERR;       //ret |= 0x20;

    if(str[1]==0 || str[1]>0x12) return TIME_MONTH_ERR; //ret |= 0x02;
    if(str[2]==0 || str[2]>0x31) return TIME_DAY_ERR;  //ret |= 0x04;
    if(str[3]>0x23) return TIME_HOUR_ERR;              //ret |= 0x08;
    if(str[4]>0x59) return TIME_MINUTE_ERR;            //ret |= 0x10;
    if(str[5]>0x59) return TIME_SECOND_ERR;            //ret |= 0x20;

    i=(str[0]>>4)*10+(str[0] & 0x0f);
    if(i<50) i+=2000;
    else i+=1900;
    memcpy(ucBuf,"\x1f\x1c\x1f\x1e\x1f\x1e\x1f\x1f\x1e\x1f\x1e\x1f",12);
    if(i%4==0) ucBuf[1]=0x1d;
    k=(str[1]>>4)*10+(str[1] & 0x0f)-1;
    day=(str[2]>>4)*10+(str[2] & 0x0f);
    if(day>ucBuf[k]) return TIME_DAY_ERR; //ret |= 0x04;

    days=0;
    for(j=1950;j<i;j++){
        if(j%4==0) days+=366;
        else days+=365;
    }

    for(i=0;i<k;i++) days+=ucBuf[i];
        days+=(day+5);
    i=(int)(days%7+1);
    *week=(uchar)i;

    return ret;
}

int    Lib_SetDateTime(uchar *datetime)
{
  struct tm tDate;
  int iRet;
  
     uchar BcdTime[8];
     
#if 0     
//test
Lib_LcdPrintxy(0,0,0," RCC_FLAG_PORRST[%d]",RCC_GetFlagStatus(RCC_FLAG_PORRST));
Lib_KbGetCh();
Lib_LcdPrintxy(0,0,0," RCC_FLAG_LPWRRST[%d]",RCC_GetFlagStatus(RCC_FLAG_LPWRRST));
Lib_KbGetCh();
#endif

     memset(BcdTime,0,8);
     memcpy(BcdTime,datetime,6);
     BcdTime[6]=0x00;
     
//#if 0
     iRet=CheckTime(datetime,BcdTime+6);
     if(iRet) return iRet;
//#endif  
  tDate.tm_year = (datetime[0]>>4)*10+(datetime[0]&0x0f);
  tDate.tm_mon = (datetime[1]>>4)*10+(datetime[1]&0x0f);
  tDate.tm_mday = (datetime[2]>>4)*10+(datetime[2]&0x0f);
  tDate.tm_hour = (datetime[3]>>4)*10+(datetime[3]&0x0f);
  tDate.tm_min = (datetime[4]>>4)*10+(datetime[4]&0x0f);
  tDate.tm_sec = (datetime[5]>>4)*10+(datetime[5]&0x0f);
  
  settime(&tDate);  
  
  return 0;
}

void Lib_LedInit(void)
{
  
}

void Lib_LedOff(WORD mask)
{
  //if(mask==1)
  if(mask&1)
  {
    LED_Show((LEDType)1,0);
  }
  //else if(mask==2)
  if(mask&2)
    LED_Show((LEDType)2,0);
  //else if(mask==4)
#if 0
  if(mask&4)
    LED_Show(3,0);
  //else if(mask==8)
  if(mask&8)
    LED_Show(4,0);
#endif
  if(mask&4)
    LED_Show((LEDType)4,0);
  //else if(mask==8)
  if(mask&8)
    LED_Show((LEDType)3,0);
  
}

void Lib_LedOn(WORD mask)
{
  //if(mask==1)
  if(mask&1)
  {
    LED_Show((LEDType)1,1);
  }
  //else if(mask==2)
  if(mask&2)
    LED_Show((LEDType)2,1);
  //else if(mask==4)
#if 0
  if(mask&4)
    LED_Show(3,1);
  //else if(mask==8)
  if(mask&8)
    LED_Show(4,1);
#endif
  if(mask&4)
    LED_Show((LEDType)4,1);
  //else if(mask==8)
  if(mask&8)
    LED_Show((LEDType)3,1);
  
}

#if 0

int   Lib_IccOpen(uchar slot,uchar VCC_Mode,uchar *ATR)
{
  SC_ADPU_Commands SC_ADPU;
  SC_ADPU_Responce SC_Responce;
  extern unsigned char SC_ATR_Table[40];
  unsigned long nTime;
  //test
  uchar i;
  //add 12/11/15
  SC_Init();
  
  SelectSlot(slot-1);
#if 0  
  //test
  SC_Init();
  Lib_LcdPrintxy(0, 0, 0x00, "111");		
  for(;;);
#endif  
  SC_State SCState = SC_POWER_OFF;
  while(1)
  {
    
    //add 12/09/28
    /* Smartcard detected */
    //CardInserted = 1;
    /* Power ON the card */
    SC_PowerCmd(ENABLE);
    /* Reset the card */
    SC_Reset(Bit_RESET);

    /* Wait A2R --------------------------------------------------------------*/
    SCState = SC_POWER_ON;

    SC_ADPU.Header.CLA = 0x00;
    SC_ADPU.Header.INS = SC_GET_A2R;
    SC_ADPU.Header.P1 = 0x00;
    SC_ADPU.Header.P2 = 0x00;
    SC_ADPU.Body.LC = 0x00;
    nTime = 0;
    while(SCState != SC_ACTIVE_ON_T0) 
    {
      
      SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
      if(++nTime==0xffff)
      {
        return 1;
        break;
      }
    }
    
#if 0        
    /* Disable the Smartcard interface */
    SCState = SC_POWER_OFF;
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    //CardInserted = 0;    
#endif   
    memcpy(ATR,SC_ATR_Table,40);
#if 0    
    //test
    trace_debug_printf("ATR:");
    for(i=0;i<40;i++)
      trace_debug_printf("%02x ",ATR[i]);
#endif                       
    return 0;
  }  
}

int   Lib_IccClose(uchar slot)
{
  SC_ADPU_Commands SC_ADPU;
  SC_ADPU_Responce SC_Responce;
  SC_State SCState;
  
    /* Disable the Smartcard interface */
    SCState = SC_POWER_OFF;
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    return 0;  
}

//#endif

int   Lib_IccCommand(uchar slot,APDU_SEND * ApduSend,APDU_RESP * ApduResp)
{
//#if 0  
  SC_ADPU_Commands SC_ADPU;
  SC_ADPU_Responce SC_Responce;
  unsigned char i;
  
  SC_State SCState=SC_ACTIVE_ON_T0;
  
    /* Select MF -------------------------------------------------------------*/
    SC_ADPU.Header.CLA = ApduSend->Command[0];
    SC_ADPU.Header.INS = ApduSend->Command[1];
    SC_ADPU.Header.P1 = ApduSend->Command[2];
    SC_ADPU.Header.P2 = ApduSend->Command[3];
    SC_ADPU.Body.LC = ApduSend->Lc;
    
    memcpy(SC_ADPU.Body.Data,ApduSend->DataIn,SC_ADPU.Body.LC );
    
    i = SC_ADPU.Body.LC;
#if 0
    for(i = 0; i < SC_ADPU.Body.LC; i++)
    {
      SC_ADPU.Body.Data[i] = MasterRoot[i];
    }
#endif
    
    
    while(i < LC_MAX) 
    {    
      SC_ADPU.Body.Data[i++] = 0;
    }
    
    SC_ADPU.Body.LE = 0;

    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    
    //test
    trace_debug_printf("SW1[%02x]SW[%02x]\n",SC_Responce.SW1,SC_Responce.SW2);
    
    /* Get Response on MF ----------------------------------------------------*/
    if( (SC_Responce.SW1 == SC_DF_SELECTED) || (SC_Responce.SW1 ==0x61) )
    {
      SC_ADPU.Header.CLA = 0;
      SC_ADPU.Header.INS = SC_GET_RESPONCE;
      SC_ADPU.Header.P1 = 0x00;
      SC_ADPU.Header.P2 = 0x00;
      SC_ADPU.Body.LC = 0x00;
      SC_ADPU.Body.LE = SC_Responce.SW2;

      SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    }
    
    ApduResp->SWA = SC_Responce.SW1;
    ApduResp->SWB = SC_Responce.SW2;
    
    //test
    trace_debug_printf("22SW1[%02x]SW[%02x]\n",SC_Responce.SW1,SC_Responce.SW2);
#if 0    
    for(i=0;i<SC_ADPU.Body.LE;i++)
      trace_debug_printf("%02x ",SC_Responce.Data[i]);
#endif    
    memcpy(ApduResp->DataOut,SC_Responce.Data,SC_ADPU.Body.LE);
    ApduResp->LenOut = SC_ADPU.Body.LE;
//#endif
    
    return 0;
}

#endif

void Lib_GetRand(uchar *out_rand ,uchar  out_len)
{
  unsigned char i,bCount;
  union 
  {
    unsigned  int nVal;
    unsigned char sDat[4];
  }uRand;
  bCount =   out_len/4;
  if(out_len%4)
    bCount++;
  for(i=0;i<bCount;i++)
  {
	srand(RTC_GetCounter());
        uRand.nVal= rand();	
        memcpy(&out_rand[i*4],(unsigned char*)uRand.sDat,4);    
  }
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
//int Lib_ReadVerInfo(uchar *VerInfo, uchar *DllInfo)
int Lib_ReadVerInfo(uchar *VerInfo)
{

    VerInfo[0]=1;				//BOOT软件主版本
    VerInfo[1]=0;				//BOOT软件次版本
//    VerInfo[2]=k_UseAreaCode;					//BOOT区域版本
#ifdef FIRMWARE_VERSION
	VerInfo[2]=FIRMWARE_MAJOR_VERSION;		//VOS主版本
	VerInfo[3]=FIRMWARE_MINOR_VERSION; 		//VOS次版本
	VerInfo[4]=FIRMWARE_MODIFIED_VERSION; 	//VOS修正版版本
#else
    VerInfo[2]=0;		
    VerInfo[3]=0;		
	VerInfo[4]=0;
#endif
    VerInfo[5]=0;							//DLL库主版本
    VerInfo[6]=0;							//DLL库次版本
    VerInfo[7]=0;
  
  
  
#if 0  
    VerInfo[0]=k_BiosVerMajor;				//BOOT软件主版本
    VerInfo[1]=k_BiosVerMinor;				//BOOT软件次版本
//    VerInfo[2]=k_UseAreaCode;					//BOOT区域版本
#ifdef FIRMWARE_VERSION
	VerInfo[2]=FIRMWARE_MAJOR_VERSION;		//VOS主版本
	VerInfo[3]=FIRMWARE_MINOR_VERSION; 		//VOS次版本
	VerInfo[4]=FIRMWARE_MODIFIED_VERSION; 	//VOS修正版版本
#else
    VerInfo[2]=0;		
    VerInfo[3]=0;		
	VerInfo[4]=0;
#endif
    VerInfo[5]=DllInfo[0];							//DLL库主版本
    VerInfo[6]=DllInfo[1];							//DLL库次版本
    VerInfo[7]=0;
    //VerInfo[8]=0;
#endif
    
    return 0;
}

int Lib_PiccGetPara(uchar *sFlag)
{
  int nRet;
  uchar sBuf[10]={0};
  nRet = sys_flash_syspara_read(PICC_SETPARA,sBuf,1);
  
    trace_debug_printf("read[%02x]\n",sBuf[0]);

  memcpy(sFlag,sBuf,1);
  
  return nRet;
  
}

  /*
          A            B
  bFlag:xxx1-  0cm      xxx1- 0cm
        x1xx-  3cm      x1xx- 3cm

        xx1x-  other    xx1x-  other
       0xf0 -正常     0x0f-正常
  */

int Lib_PiccSetPara( uchar bFlag)
{
  uchar sBuf[10];
  
  //bAFlag = sFlag[0];
  //bBFlag = sFlag[1];
  
#if 0
  if(!( (bFlag==0) &&  (bFlag==1) && (bFlag==2) && (bFlag==3) && (bFlag==0xff)))
  {
    return 1;
  }
  
#endif
  sBuf[0] = bFlag;
  sys_flash_syspara_write(PICC_SETPARA,sBuf,1);
  
  _glbPiccSetPara = bFlag;
  
  trace_debug_printf("write[%02x]\n",_glbPiccSetPara);
  
  return 0;
}


/*==============================================================
  函数功能: DES(3DES)计算
  mackey_n  计算DES的密钥(DES密钥区)
  datain    输入数据
  macout    输出数据
  mode      非零：DES加密   0x00： DES解密   
  keytype   低4位：0x00：DES, 0x01：双倍长DES密钥，0x02:三倍长DES密钥
                高4位：0x00: 用DES密钥，0x01：用主密钥， 0x02:PINKEY
  ================================================================*/
int  Pci_Des(BYTE deskey_n, BYTE *indata, BYTE *outdata, BYTE mode, BYTE keytype)
{
    uchar   deskeybuf[24],deskeylen,datain[8],keytypeL4; 
    int     iret;
    uchar bCurAppNum;//因为只有一个应用
      
    //gCurAppNum=App_Msg.Num;
    bCurAppNum = 0;
    if(deskey_n>=100)
    {
        return PCI_KeyNo_Err;
    }

    if ((indata == NULL) || (outdata == NULL))
    return PCI_NoInput_Err;
    if ((keytype & 0xf0) == 0x10)
    //iret=s_ReadDesKey(gCurAppNum,MKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
    iret=s_ReadDesKey(bCurAppNum,PIN_MKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
    
    else if ((keytype & 0xf0) == 0x20)
    iret=s_ReadDesKey(bCurAppNum,PINKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
    else 
    iret=PCI_KeyType_Err;	
    if(iret) return iret;
		
    keytypeL4=keytype & 0x0f;
		
    memset(datain,0,sizeof(datain));
    memcpy(datain,indata,8);
    
    switch(deskeylen/8)
    {
    case 0x01:
		if(keytypeL4 != 0) return PCI_KeyType_Err;
        if(mode)
        {
            DES(datain,deskeybuf,1);
        }
        else
        {
            DES(datain,deskeybuf,0);
        }   
        
        break;
    case 0x02:
			if(keytypeL4 == 0)
			{
			if(mode) DES(datain,deskeybuf,1);
			else DES(datain,deskeybuf,0);
			}
			else if(keytypeL4 ==1)
			{
			if(mode)
			{
			DES(datain,deskeybuf,1);
			DES(datain,&deskeybuf[8],0);
			DES(datain,deskeybuf,1);
			}
			else
			{
			DES(datain,deskeybuf,0);
			DES(datain,&deskeybuf[8],1);
			DES(datain,deskeybuf,0);
			}
			}
			else return PCI_KeyType_Err;

        break;
    case 0x03:
			if(keytypeL4 == 0)
			{
			if(mode) DES(datain,deskeybuf,1);
			else DES(datain,deskeybuf,0);
			}
			else if(keytypeL4 ==1)
			{
			if(mode)
			{
			DES(datain,deskeybuf,1);
			DES(datain,&deskeybuf[8],0);
			DES(datain,deskeybuf,1);
			}
			else
			{
			DES(datain,deskeybuf,0);
			DES(datain,&deskeybuf[8],1);
			DES(datain,deskeybuf,0);
			}
			}
			else if(keytypeL4 ==2)
			{
			if(mode)
			{
			DES(datain,deskeybuf,1);
			DES(datain,&deskeybuf[8],0);
			DES(datain,&deskeybuf[16],1);
			}
			else
			{
			DES(datain,&deskeybuf[16],0);
			DES(datain,&deskeybuf[8],1);
			DES(datain,deskeybuf,0);
			}
			}
			else return PCI_KeyType_Err;
			        break;
    default: return PCI_KeyLen_Err;
    }
    
    memcpy(outdata,datain,8);
    memset(deskeybuf,0,24);
    return 0;
}

void   Lib_Reboot(void)
{
    Buzzer_Off();
    NVIC_GenerateSystemReset();
    NVIC_GenerateCoreReset();
    while(1);
}

