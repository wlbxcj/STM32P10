/*********************************************************************************
Copyright (C) 2006 ViewAt Technology Co., LTD.

System Name    :  vos
Module Name    :  key manage and application(PED) driver
File   Name    :  pci.c
Revision       :  01.00
Date           :  2006/12/1
Dir            :  drv\pci
error code     :

须补充BPK内容：
1)MMK放入BPK中。

BPK共16＊4字节，即保存4个16字节的MMK或8个8字节的MMK或2个24字节的MMK

使用3个MMK，二个用于VOS安全管理，一个用于APP密钥管理；
其余空间：　　REG0：　0XAA55BB55　（首寄存器用作标识）
　　　　　　　REG1,2，3,4,5,6：　MMK1   (24字节)
　　　　　　　REG7：MMK1验证码　
　　　　　　　REG8，9：　MMK2           (8字节)
　　　　　　　REG10：MMK2的验证码
　　　　　　　REG11,12,13,14：MMK3      (16字节)
　　　　　　　REG15：MMK3的验证码

PCI主要采用MK、SK管理方式，MK，SK由MMK3加密后存于外部FLASH文件中

VOS：锁键盘与解锁键盘功能 （操作员1＆2），密码存于FLASH中，与SN有关联；
　　下载显示注册信息或MK　（操作员3＆4），密码存于FLASH中，与SN有关联，可修改。
　
　
***********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include "KF701DH.h"
#include "stm32f10x_lib.h"
#include "..\..\inc\FunctionList.h"

//#include <hal/usip/hal_rng.h> 
#include "base.h"
#include "vosapi.h"
#include "pci.h"
//#include "usip.h"
#include "..\..\inc\FunctionList.h" 
//#include "..\datastruct\struct.h" 

//#include <hal/usip/hal_timer.h>
 
#define PIN_INPUT_LIMIT
//#undef  PIN_INPUT_LIMIT
#define __BCTC_CRC_VERIFY__
#undef  __BCTC_CRC_VERIFY__
//
#define __CHECK_SAME_KEY__
#undef __CHECK_SAME_KEY__
//
//#define  __NEW_PCI_REQUIRE__   //PIN输入时计数：60分钟内，输入密码的次数不超过120次即可。						

#define printf trace_debug_printf //12/11/08
#define DES  Lib_DES //sxl
volatile static int   g_PinInputIntervalCtrl; //PIN输入时间间隔控制：（GETPIN接口只能每隔30秒调用一次）

//#ifdef __NEW_PCI_REQUIRE__
//int s_PciCheckCallInteral(uint uiTimerNo  /* = PCI_TIMER2*/,
//						  uint uiInputCnt /* = PCI_INPUT_COUNT*/, 
//						  ulong ulTimeout /* = PCI_TIME_OUT*/);
//#define s_PciCheckCallInteralMacro() s_PciCheckCallInteral(0,PCI_INPUT_COUNT,PCI_CALL_TMO)
//CQueue gCQ ; //初始化队列用于
//static int   g_PinInputCounterPer60min; //单位小时 //PIN输入时计数：60分钟内，输入密码的次数不超过120次即可。
//#define SaveCurCall(n)   EnQueueAuto(&gCQ,n)
//#define	ClrAllCall()	 ClearQueue(&gCQ) 
//#define DelInvalidCall() DelElement(&gCQ,NULL)
//
////#define PCI_TIMER2     (3)
////#define PCI_TIMER1     (1) 
//
//#define PCI_TIME_OUT   (360000L) //单位10ms  
//#define PCI_TIME_OUT_PER_SEC (PCI_TIME_OUT/100L)  
//#define PCI_INPUT_COUNT (Q_MAX_SIZE)  //120次
//#define PCI_CALL_TMO   (60*60L)
//#else 
//#define PCI_TIME_OUT   (3000) //单位10ms
//#define PCI_TIMER2      (3)
//#endif //__NEW_PCI_REQUIRE__

volatile static int   gLockStatus;
volatile static int   gNoPinEnterDisenable;//0=无密码可确认退出　1＝无密码不可确认退出
volatile static uchar gPciAuthTimes[25]; 
volatile static int   g_PciRandCount=0; 
volatile static int   gCurAppNum;
AUTH_RESULT   Auth_Result; 


//add from app.c
// 当前运行的程序，0xff表示运行VOS，0~24表示运行应用程序
BYTE g_byCurAppNum = 0xff;

int s_PciInitMMK(void);
int s_PciWriteMMK(int iIndex, DWORD *pdwIn);
int s_InitAuthKeyFile(void);
int s_InitMdkFile(void);
int s_CheckAllKeysIsDiff(uchar keytype,uchar keyno,uchar keylen,uchar *keydata); 
 
int s_ReadSK_MACK(uchar *sk_mack,uchar *sk_mack_len);
int s_WriteSK_MACK(uchar *sk_mack,uchar sk_mack_len);
void s_DesMacWithTCBC(unsigned char *pbuInData, unsigned char datalen,unsigned char *pbyKey, unsigned char keylen, unsigned char *pbyOutData);
/////////////////////////////////////////////////////////////////////////////////////////// 
extern uchar gFileBuffer[32*1024]; 
extern uchar g_byStartX, g_byStartY;
extern uchar g_byFontType;
extern const APP_MSG App_Msg;
extern int   g_iAuthForNumberKeyFlag; 
///////////////////////////////////////////////////////////////////////////////////////////
extern uchar g_bDukpt_KSN[10];         //current KSN  
extern uchar g_bDukpt_CurAppNo;
extern uchar g_bDukpt_CurKeyID; 
extern ulong g_lDukpt_EncryptCnt;      //加密计算器，后21位有效 

extern void s_ClearDukptGlobalParameter(void);
extern int s_PciDukptInit(void);
extern int s_DukptEraseKey(uchar AppNo,uchar KeyId,uchar Mode);
//////////////////////////////////////////////////////////////////////////////////////////

//用于操作BPK单元。
extern int  s_rtc_write_reg(unsigned int which, unsigned int value);
extern int  s_rtc_read_reg(unsigned int which, unsigned int *value);
extern void QuickCrc32(BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[4]); 
extern int  s_rtc_write_reg(unsigned int which, unsigned int value); 
extern int  s_rtc_read_reg(unsigned int which, unsigned int *value);
 

extern void DwordToByte(DWORD dwInData, BYTE *pbyOutData);

//port
void hal_rng_read(unsigned int *prdRand)
{
  
	int fd_rng;
	int result;
        static unsigned char i=0; 
        i++;
	srand(RTC_GetCounter()+i);	
	//11/06/13
	//TraceDisp("rand RTC_GetCounter[%x]\n",RTC_GetCounter());
                           
	*prdRand = rand();
        
}

void CRC_ResetDR(void)
{
/* CR register bit mask */

#define CR_RESET_Set    ((uint32_t)0x00000001)
  
  /* Reset CRC generator */
  CRC->CR = CR_RESET_Set;
  
}

void QuickCrc32(BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[4])
{
  unsigned int  index = 0;
  union{
    unsigned int  nDat;
    unsigned char sDat[4];
  }tUnionData;
  unsigned int iLen;
  iLen = dwDataLen/4;
  
  CRC_ResetDR();
  
  for(index = 0; index < iLen; index++)
  {
    memcpy(tUnionData.sDat,&pbyDataIn[index*4],4);
    //CRC->DR = pBuffer[index];
    CRC->DR = tUnionData.nDat;
  }
  tUnionData.nDat = CRC->DR;
  memcpy(abyCrcOut,tUnionData.sDat,4);
  
}

static int storage_write32b( int offset, DWORD value )
{
    //return s_rtc_write_reg((UINT)(USIP_BPK_USR0+offset), value);
    BKP_WriteBackupRegister((UINT)(BKP_DR11+offset),value&0xffff);
    BKP_WriteBackupRegister((UINT)(BKP_DR11+offset+4),value>>16);
    return 0;    
    
    
}

static int storage_read32b( int byte_offset, DWORD *value )
{
    unsigned long nVal1,nVal2;
    //return s_rtc_read_reg((UINT)(USIP_BPK_USR0+byte_offset), (UINT*)value);
    nVal1 = BKP_ReadBackupRegister(BKP_DR11+byte_offset);
    nVal2 = BKP_ReadBackupRegister(BKP_DR11+byte_offset+4);
    *value= (nVal2<<16)|nVal1;
    return 0;
}

int ss_BpkWriteReg(DWORD dwRegNum, DWORD dwRegVal)
{
	//Check parameters  
    //if ( dwRegNum >= HAL_STORAGE_WORD_COUNT )
    if ( dwRegNum >= STORAGE_WORD_COUNT )
      
        return -22;

//#ifndef BPK_BOOT_OLD_VERSION
//    return storage_write32b(dwRegNum*sizeof(int), dwRegVal);
    return storage_write32b(dwRegNum*(sizeof(int)+4), dwRegVal);
//#else
//    DWORD dwRegValTemp;
//    dwRegValTemp = ((dwRegVal&0xff)<<24);
//    dwRegValTemp += ((dwRegVal&0xff00)<<8);
//    dwRegValTemp += ((dwRegVal&0xff0000)>>8);
//    dwRegValTemp += ((dwRegVal&0xff000000)>>24);
//    return storage_write32b(dwRegNum*sizeof(int), dwRegValTemp);
//#endif
}

int ss_BpkReadReg(DWORD dwRegNum, DWORD *pdwRegVal)
{
	//Check parameters  
    //if ( dwRegNum >= HAL_STORAGE_WORD_COUNT )
    if ( dwRegNum >= STORAGE_WORD_COUNT )
        return -22;

//#ifndef BPK_BOOT_OLD_VERSION
    //return storage_read32b(dwRegNum*sizeof(int), pdwRegVal);
    return storage_read32b(dwRegNum*(sizeof(int)+4), pdwRegVal);
//#else
//    DWORD dwRegValTemp;
//    int iRet;
//    iRet = storage_read32b(dwRegNum*sizeof(int), &dwRegValTemp);
//    *pdwRegVal = ((dwRegValTemp&0xff)<<24);
//    *pdwRegVal += ((dwRegValTemp&0xff00)<<8);
//    *pdwRegVal += ((dwRegValTemp&0xff0000)>>8);
//    *pdwRegVal += ((dwRegValTemp&0xff000000)>>24);
//    return iRet;
//#endif
} 

// 删除所有敏感的保密的数据
int s_DelAllSecrecyData(void)
{
    //s_PciInitMMK();
    //s_remove(SYSKEY_FILE, (unsigned char *)"\xff\x02"); //12/10/23 disable
    s_InitSysKeyFile(1);

    //s_remove(AUTHKEY_FILE, (unsigned char *)"\xff\x02"); //12/10/23 disable
    s_InitAuthKeyFile();

    //s_remove(MDK_FILE, (unsigned char *)"\xff\x02"); //12/10/23 disable
    s_InitMdkFile();

    //s_remove(DUKPTKEY_FILE, (unsigned char *)"\xff\x02"); //12/10/23
    s_PciDukptInit(); 
    return 0;
}  

int s_PciWritePinCount(int i, DWORD value)
{
	int iRet;

	iRet = ss_BpkWriteReg(14+i, value);
	if (0 != iRet)
	{
		return -1;
	}
	return iRet;
}

int s_PciReadPinCount(int i,DWORD *pdwOut)
{
	int iRet;
    //DWORD adwTemp[8]; 
	
	iRet = ss_BpkReadReg(14+i,pdwOut);
	//pdwOut[0] = adwTemp[i];
	return iRet;
}
/****************************************************************************
  函数名     :  int s_PciInitMMK(void)
  描述       :  初始化Bpk数据，BPK寄存器存储分配如下：
                      REG0：　0XAA55BB55　（首寄存器用作标识）
        　　　　　　　REG1,2，3,4,5,6：　MMK1
        　　　　　　　REG7：MMK1验证码　
        　　　　　　　REG8，9：　MMK2
        　　　　　　　REG10：MMK2的验证码
        　　　　　　　REG11,12,13,14：MMK3
        　　　　　　　REG15：MMK3的验证码
  输入参数   :  无
  输出参数   :  无
  返回值     :   0：表示初始化成功
                -1：表示写Bpk寄存器失败
  备    注   :  该函数只能被清除敏感数据函数调用，其他地方不要调用
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-16   V1.0        创建
****************************************************************************/
int s_PciInitMMK(void)
{
    DWORD adwTemp[6];
    int i, j, iRet;

    iRet = ss_BpkWriteReg(0, BPK_VALIDATE_FLAG);
    if (0 != iRet)
    {
        return -1;
    }

    // 初始化后的MMK1和MMK2必须能通过CRC32校验
    for (j=0; j<2; j++)
    {
        //(void)hal_rng_init(); //12/10/23 disable
        
        hal_rng_read((unsigned int*)&adwTemp[0]);
        hal_rng_read((unsigned int*)&adwTemp[1]);
        hal_rng_read((unsigned int*)&adwTemp[2]);
        hal_rng_read((unsigned int*)&adwTemp[3]);
        hal_rng_read((unsigned int*)&adwTemp[4]);
        hal_rng_read((unsigned int*)&adwTemp[5]);
        iRet = s_PciWriteMMK(j, adwTemp);
        if (0 != iRet)
        {
            return -1;
        }
    }

    // 初始化后的MMK3不能通过CRC32校验
    //for (i=0; i<5; i++)
    //{
    //    iRet = ss_BpkWriteReg(i+11, 0x0);
    //    if (0 != iRet)
    //    {
    //        return -1;
    //    }
    //}
    return 0;
}

/****************************************************************************
  函数名     :  int s_PciMMKCheck(void)
  描述       :  检查MMK的合法性
  输入参数   :  无
  输出参数   :  无
  返回值     :   0表示校验通过；
                -1表示读BPK寄存器错误；
                -2表示MMK的有效标志位无效；
                -3表示MMK1无效；
                -4表示MMK2无效；
                -5表示MMK3无效。
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-16   V1.0        创建
****************************************************************************/
int s_PciMMKCheck(void)
{
    int i, iRet;
    DWORD adwTemp[6];

    iRet = ss_BpkReadReg(0, adwTemp);
    if (0 != iRet)
    {
        return -1;
    }
    if (BPK_VALIDATE_FLAG != adwTemp[0])
    {
        return -2;
    }

    for (i=0; i<2; i++)
    {
        iRet = s_PciReadMMK(i, adwTemp);
        if (-2 == iRet)
        {
            return -1;
        }
        else if (0 != iRet)
        {
            return -(3+i);
        }
    }
    return 0;
}

/****************************************************************************
  函数名     :  void ByteToDword(BYTE *pbyIn, int iLen, DWORD *pdwOut)
  描述       :  将8位无符号整数换成32位无符号整型数
  输入参数   :  1、BYTE *pbyIn : 转换前的无符号字符型数
                2、int iLen : 输入数据的长度，该参数长度必须是4
  输出参数   :  DWORD *pdwOut  : 所求结果
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-22   V1.0        创建
****************************************************************************/
void ByteToDword(BYTE *pbyIn, int iLen, DWORD *pdwOut)
{
    int i;
    for (i=0; i<iLen; i+=4)
    {
        pdwOut[i/4] = (pbyIn[i]*256+pbyIn[i+1])*65536
            +pbyIn[i+2]*256+pbyIn[i+3];
    }
}

void DwordToBytes(DWORD *pdwIn, int iLen, BYTE *pbyOut)
{
    int i;

    for (i=0; i<iLen; i++)
    {
        pbyOut[i*4  ] = (BYTE)(pdwIn[i]>>24);
        pbyOut[i*4+1] = (BYTE)(pdwIn[i]>>16);
        pbyOut[i*4+2] = (BYTE)(pdwIn[i]>>8);
        pbyOut[i*4+3] = (BYTE)pdwIn[i];
    }
}

/****************************************************************************
  函数名     :  int s_PciReadMMK(int iIndex, DWORD *pdwOut)
  描述       :  读取MMK数据，每次读取4个32位无符号整数
  输入参数   :  1、int iIndex：索引号，取值0、1和2，其他取值错误
  输出参数   :  1、DWORD *pdwOut：输出4个32位无符号整数
  返回值     :   0：成功；
                -1：索引号错误；
                -2：读MMK错误；
                -3：CRC32校验错误
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-16   V1.0        创建
****************************************************************************/
int s_PciReadMMK(int iIndex, DWORD *pdwOut)
{
    int iRet, i;
    BYTE abyTemp[29];
    DWORD adwTemp[8]; 
    
    switch(iIndex)
    {
    case 0:
        for (i=0; i<7; i++)
        {
            iRet = ss_BpkReadReg(1+i, &adwTemp[i]);
            if (0 != iRet)
            {
                return -2;
            }
        }
        DwordToBytes(adwTemp, 6, abyTemp);
        QuickCrc32(abyTemp, 24, &abyTemp[24]);
        DwordToBytes(&adwTemp[6], 1, abyTemp);
        if (0 != memcmp(abyTemp,&abyTemp[24],4))
        {
            return -3;
        }
        pdwOut[0] = adwTemp[0];
        pdwOut[1] = adwTemp[1];
        pdwOut[2] = adwTemp[2];
        pdwOut[3] = adwTemp[3];
        pdwOut[4] = adwTemp[4];
        pdwOut[5] = adwTemp[5];
        return 0; 
    case 1:
        for (i=0; i<5; i++)
        {
            iRet = ss_BpkReadReg(7+1+i, &adwTemp[i]);
            if (0 != iRet)
            {
                return -2;
            }
        }
        DwordToBytes(adwTemp, 4, abyTemp);
        QuickCrc32(abyTemp, 16, &abyTemp[16]);
        DwordToBytes(&adwTemp[4], 1, abyTemp);
        if (0 != memcmp(abyTemp,&abyTemp[16],4))
        {
            return -3;
        }
        pdwOut[0] = adwTemp[0];
        pdwOut[1] = adwTemp[1];
        pdwOut[2] = adwTemp[2];
        pdwOut[3] = adwTemp[3];
        return 0; 
//    case 2:
//        for (i=0; i<3; i++)
//        {
//            iRet = ss_BpkReadReg(12+1+i, &adwTemp[i]);
//            if (0 != iRet)
//            {
//                return -2;
//            }
//        }
//        DwordToBytes(adwTemp, 2, abyTemp);
//        QuickCrc32(abyTemp, 8, &abyTemp[8]);
//        DwordToBytes(&adwTemp[2], 1, abyTemp);
//        if (0 != memcmp(abyTemp,&abyTemp[8],4))
//        {
//            return -3;
//        }
//        pdwOut[0] = adwTemp[0];
//        pdwOut[1] = adwTemp[1];
//        return 0; 
    default:
        return -1;
    } 
}

/****************************************************************************
  函数名     :  int s_PciWriteMMK(int iIndex, DWORD *pdwIn)
  描述       :  写入MMK数据，每次写入4个32位无符号整数
  输入参数   :  1、int iIndex：索引号，取值0、1和2，其他取值错误
                2、DWORD *pdwIn：输入4个32位无符号整数
  输出参数   :  无
  返回值     :   0：成功；
                -1：索引号错误；
                -2：写MMK错误。
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-09-16   V1.0        创建
****************************************************************************/
int s_PciWriteMMK(int iIndex, DWORD *pdwIn)
{
    int iRet, i;
    DWORD dwTemp;
    BYTE abyTemp[29]; 
    
    switch(iIndex)
    {
    case 0:
        DwordToBytes(pdwIn, 6, abyTemp);
        
        QuickCrc32(abyTemp, 24, &abyTemp[24]);
        for (i=0; i<7; i++)
        {
            dwTemp = (abyTemp[i*4]*256+abyTemp[i*4+1])*65536
                + abyTemp[i*4+2]*256+abyTemp[i*4+3];
            iRet = ss_BpkWriteReg(1+i, dwTemp);
            if (0 != iRet)
            {
                return -2;
            }
        }
        return 0;
    case 1:
        DwordToBytes(pdwIn, 4, abyTemp);
        
        
        QuickCrc32(abyTemp, 16, &abyTemp[16]);
        for (i=0; i<5; i++)
        {
            dwTemp = (abyTemp[i*4]*256+abyTemp[i*4+1])*65536
                + abyTemp[i*4+2]*256+abyTemp[i*4+3];
            iRet = ss_BpkWriteReg(7+1+i, dwTemp);
            if (0 != iRet)
            {
                return -2;
            }
        }
        return 0;
//    case 2:
//        DwordToBytes(pdwIn, 2, abyTemp);
//        QuickCrc32(abyTemp, 8, &abyTemp[8]);
//        for (i=0; i<3; i++)
//        {
//            dwTemp = (abyTemp[i*4]*256+abyTemp[i*4+1])*65536
//                + abyTemp[i*4+2]*256+abyTemp[i*4+3];
//            iRet = ss_BpkWriteReg(12+1+i, dwTemp);
//            if (0 != iRet)
//            {
//                return -2;
//            }
//        }
//        return 0;
    default:
        return -1;
    } 
}

int s_PciInit(void)
{
#ifdef __NEW_PCI_REQUIRE__
	(QE*)InitQueue(&gCQ);
	g_PinInputCounterPer60min = 0 ; 
#endif
    //Initialize RNG peripheral 
    //hal_rng_init();
    g_PinInputIntervalCtrl=0;
    gLockStatus=0;
    gNoPinEnterDisenable=0;
    s_InitSysKeyFile(0);
    s_InitAuthKeyFile();  
    s_InitMdkFile();
    memset((uchar *)&Auth_Result,0,sizeof(AUTH_RESULT));  
    memset( (void*)gPciAuthTimes,0,sizeof(gPciAuthTimes));  
    g_PciRandCount=0;  
    s_PciDukptInit(); 
    return 0;
}

int s_GetLockState(void)
{
    uchar buff[10];
    memset(buff,0,sizeof(buff));
    s_ReadSysKeyFile(PCI_LOCK_ADDR,FILE_SEEK_SET,buff,8);
    if(memcmp(buff,LOCKED_STR,6))
    {
        gLockStatus=0;
        return 0;
    }
    gLockStatus=1;
    return 1;//0=unlock  1=locked
}

int s_GetLockStateBuf(unsigned char *sOutBuf)
{
    uchar buff[10];
    memset(buff,0,sizeof(buff));
    s_ReadSysKeyFile(PCI_LOCK_ADDR,FILE_SEEK_SET,buff,8);
    memcpy(sOutBuf,buff,8);
    
    if(memcmp(buff,LOCKED_STR,6))
    {
        gLockStatus=0;
        return 0;
    }
    gLockStatus=1;
    return 1;//0=unlock  1=locked
}

int s_SetLockState(int mode)
{
    uchar buff[10];
    //uchar buff1[10],i;
    memset(buff,0,sizeof(buff));
    if(mode)
    {
        memcpy(buff,LOCKED_STR,6);
        gLockStatus=1;
    }
    else
    {
        memcpy(buff,UNLOCK_STR,6);
        gLockStatus=0;
    }
    s_WriteSysKeyFile(PCI_LOCK_ADDR,FILE_SEEK_SET,buff,8); 
    
    //memset(buff1,0,sizeof(buff1));
    //s_ReadSysKeyFile(PCI_LOCK_ADDR,FILE_SEEK_SET,buff1,8);
    //s_UartPrint(COM_DEBUG,"buff=%s\r\n",buff);
    //s_UartPrint(COM_DEBUG,"buff1=%s\r\n",buff1); 
    return 0;
} 

//int s_GetManagePwd(unsigned char *pwd)
//{
//    s_ReadSysKeyFile(PCI_MANAGEPWD_ADDR,FILE_SEEK_SET,pwd,9);
//    return 0;
//}
//
//
//int s_SetManagePwd(unsigned char *pwd)
//{
//    s_WriteSysKeyFile(PCI_MANAGEPWD_ADDR,FILE_SEEK_SET,pwd,9);
//    return 0;
//}
//
//int s_GetVerifyTimes(void)
//{
//    return 0;
//}

//int s_SetVerifyTimes(int times)
//{
//    return 0;
//}


int s_PciClearAppKey(uchar app_no)
{
    int offset;
    int iret; 
    int fd,i,nNum,nVal;
    uchar appname[33],app_name[33];
    long loffset,nCurOffset;
    AUTH_KEY auth_key;
    unsigned char gFileBuffer[1024];
    //printf("s_pciclearappkey(%02x)\r\n",app_no);
//13/04/02  下载应用不删除密钥
    return 0;
    
    if(app_no>24)   return PCI_AppNumOver_Err;
    //clear all app master key and session key
    //memset(gFileBuffer, 0, PCI_APPKEY_SPACE);
    memset(gFileBuffer, 0, sizeof(gFileBuffer));
    
    offset=(int)app_no*PCI_APPKEY_SPACE;
    
    //iret=s_WriteSysKeyFile(PCI_PIN_MKEY_ADDR+offset,FILE_SEEK_SET,gFileBuffer,PCI_APPKEY_SPACE);
    nNum = PCI_APPKEY_SPACE/1024;
    for(i=0;i<nNum;i++)
    {
      iret=s_WriteSysKeyFile(PCI_PIN_MKEY_ADDR_PAGE0+offset+i*1024,FILE_SEEK_SET,gFileBuffer,1024);
      
    }
      
    //if(iret)        return PCI_WriteKey_Err;
    //clear all app auth key 
    //printf("step0: "); 
    memset(appname,0,sizeof(appname));
    iret=SPF_GetAppName(app_no, appname);
    //printf("step1=%d ",iret);
    if(appname[0]==0) //return -1;
    appname[32]=0;
    //printf("step2=%s \r\n",appname);
    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)    return -2; 
    //nCurOffset = AUTHKEY_FILE_ADDR;
    while(1)
    {
        memset((uchar *)&auth_key,0,sizeof(AUTH_KEY));
        i=my_FileRead(fd,(uchar *)&auth_key,sizeof(AUTH_KEY));
        //sys_flash_read_operate(AUTHKEY_FILE_ADDR,(uchar *)&auth_key,sizeof(AUTH_KEY));
        //nCurOffset+=sizeof(AUTH_KEY);
        //if(nCurOffset>(AUTHKEY_FILE_ADDR+AUTHKEY_FILE_SIZE))
        //  break;
        nVal = sizeof(AUTH_KEY);
        if(i<nVal)
        { 
            break;
        }
        
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
        if(strcmp(app_name,appname)==0)
        {
            loffset=sizeof(AUTH_KEY);
            loffset=0-loffset;
            my_FileSeek(fd,loffset,FILE_SEEK_CUR);
            nCurOffset-=loffset;
            memset((uchar *)&auth_key,0,sizeof(AUTH_KEY));            
            my_FileWrite(fd,(unsigned char *)&auth_key,sizeof(AUTH_KEY));  
            //sys_flash_write_operate(nCurOffset,(unsigned short *)&auth_key,sizeof(AUTH_KEY));
            //nCurOffset+=loffset;
            
            //printf("step3=%s \r\n",app_name);
        } 
    } 
    my_FileClose(fd);  
    
    s_DukptEraseKey(app_no,0,0); 
    
    return 0;
}



int s_PciViewAuthKey(void)
{ 
    int fd,i,nVal;
    uchar app_name[33]; 
    AUTH_KEY auth_key; 
    
    //fd=s_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)    return -2; 
    while(1)
    {
        memset((uchar *)&auth_key,0,sizeof(AUTH_KEY));
        i=my_FileRead(fd,(uchar *)&auth_key,sizeof(AUTH_KEY));
        nVal =sizeof(AUTH_KEY);
        //if(i<sizeof(AUTH_KEY))
        if(i<nVal)
        { 
            break;
        }
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
#if 0        
        printf("app_name: %s  ",app_name);
        printf("key_type: %02x ",auth_key.KeyType);
        printf("key_no  : %02x ",auth_key.KeyNo);
        printf("key_len : %02x ",auth_key.KeyLen);
#endif
    } 
    my_FileClose(fd);  
    return 0;
}


//int s_RecreateSysKeyFile(void)
//{
//    s_remove(SYSKEY_FILE, (unsigned char *)"\xff\x02");
//    s_InitSysKeyFile();
//    return 0;
//}


int s_InitSysKeyFile(unsigned char bFlag)
{
    int fd,needinit,i,nTimes;
    unsigned char gFileBuffer[1024];
    unsigned char nTmp[100],sDate[10];
    
    fd=my_open(SYSKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    needinit=0;
    
    //13/05/27    
if(bFlag)
    fd = -1;

    if(fd<0)
    {
      
      
        fd = my_open(SYSKEY_FILE, O_CREATE, (uchar *)"\xff\x02");
#if 0 //13/05/27
        memset(gFileBuffer, 0, 256);
        my_FileWrite(fd, gFileBuffer, 256);
#endif
        //13/07/11
        my_FileRead(fd, gFileBuffer, 128);
        //test
        trace_debug_printf("PCI_CRC_ADDR_PAGE0[%02x][%02x]\n",gFileBuffer[PCI_CRC_ADDR_PAGE0],
                           gFileBuffer[PCI_CRC_ADDR_PAGE0+1]);
        trace_debug_printf("PCI_LIST_ADDR[%02x][%02x] [%02x]\n",gFileBuffer[PCI_LIST_ADDR],
                           gFileBuffer[PCI_LIST_ADDR+1],gFileBuffer[PCI_LIST_ADDR+12+6]);
        
        i = gFileBuffer[PCI_LIST_ADDR+12+6];
        if( (i>=0x30) && (i<=0x39))
        {
          nTimes = atoi(&gFileBuffer[PCI_LIST_ADDR+12+6]);
          if(nTimes==9999)
           nTimes = 0;
          nTimes++;
        }
        else
          nTimes = 1;
        Lib_GetDateTime(sDate);
        memset(nTmp,0,sizeof(nTmp));
        
        sprintf(nTmp,"%02x-%02x-%02x %02x:%02x:%02x %04d",sDate[0],sDate[1],sDate[2],sDate[3],sDate[4],
                sDate[5],nTimes);
        
        //test
        trace_debug_printf("list str[%s]\n",nTmp);
        
        memcpy(&gFileBuffer[PCI_LIST_ADDR],nTmp,12+6+4+1);
        my_FileSeek(fd,0,FILE_SEEK_SET);
        my_FileWrite(fd, gFileBuffer, 128);
        
        my_FileSeek(fd,PCI_PIN_MKEY_ADDR_PAGE0,FILE_SEEK_SET);
        memset(gFileBuffer, 0, sizeof(gFileBuffer));
        
#if 0
        //10个应用
        Lib_FileWrite(fd, gFileBuffer, 32*1024);
        Lib_FileWrite(fd, gFileBuffer, 32*1024);
        Lib_FileWrite(fd, gFileBuffer, 32*1024);
        Lib_FileWrite(fd, gFileBuffer, 32*1024);
        Lib_FileWrite(fd, gFileBuffer, 32*1024);
#endif

        //13/07/11
//#if 0    //not init
        for(i=0;i<((PCI_BACK_ADDR)/1024)*2;i++)  //13/10/17->2
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_BACK_ADDR)%1024);
//#endif
        
#if 0 //13/07/11 
        //现为3个应用=>1个应用
        //my_FileWrite(fd, gFileBuffer, 100*32*3);
        //for(i=0;i<9;i++)
        for(i=0;i<(PCI_MAX_APPNUM*32*100)/1024;i++)
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_MAX_APPNUM*32*100)%1024);
        
        //my_FileWrite(fd, gFileBuffer, 100*32*3);
        //for(i=0;i<9;i++)
        for(i=0;i<(PCI_MAX_APPNUM*32*100)/1024;i++)
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_MAX_APPNUM*32*100)%1024);
        
        //my_FileWrite(fd, gFileBuffer, 100*32*3);
        //for(i=0;i<9;i++)
        for(i=0;i<(PCI_MAX_APPNUM*32*100)/1024;i++)
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_MAX_APPNUM*32*100)%1024);
        
        //my_FileWrite(fd, gFileBuffer, 100*32*3);
        //for(i=0;i<9;i++)
        for(i=0;i<(PCI_MAX_APPNUM*32*100)/1024;i++)
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_MAX_APPNUM*32*100)%1024);
        
        //my_FileWrite(fd, gFileBuffer, 100*32*3);
        //for(i=0;i<9;i++)
        for(i=0;i<(PCI_MAX_APPNUM*32*100)/1024;i++)
          my_FileWrite(fd, gFileBuffer, 1024);
        //my_FileWrite(fd, gFileBuffer, 384);
        my_FileWrite(fd, gFileBuffer, (PCI_MAX_APPNUM*32*100)%1024);
#endif
        
        needinit=1;
        //return -1;
    }
    my_FileClose(fd);
#if 0 //13/05/27
    if(needinit)  s_SetLockState(0);
#endif    
    return 0;
}

int s_InitAuthKeyFile(void)
{
    int fd;
    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)
    {
        fd = my_open(AUTHKEY_FILE, O_CREATE, (uchar *)"\xff\x02");

    }
    my_FileClose(fd);
    return 0;
}

int s_InitMdkFile(void)
{
    int fd;
    fd=my_open(MDK_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)
    {
        fd = my_open(MDK_FILE, O_CREATE, (uchar *)"\xff\x02");

    }
    my_FileClose(fd);
    return 0;
} 


//for debug
int s_CheckSysKeyFile(void)
{
    int fd;
    int i,flen;
    uchar buff[1024];
    return 0;
    s_UartPrint(COM_DEBUG,"keyfile:\r\n");
    fd=my_open(SYSKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)
    {
        s_UartPrint(COM_DEBUG,"open error =%d ",fd);
    }
    else
    {

        my_FileSeek(fd,7*1024,FILE_SEEK_SET);

        my_FileRead(fd,buff,1024);
        for(i=0;i<1024;i++)
        {
            //Lib_FileRead(fd,buff,1);
            s_UartPrint(COM_DEBUG,"%02x ",buff[i]);
            if(i%16==0)  s_UartPrint(COM_DEBUG,"\r\n");
        }
    }
    my_FileClose(fd);
    return 0;

}


int s_ReadSysKeyFile(long offset,unsigned char fromwhere,unsigned char *outdata,int max_len)
{

    int fd,i;
    fd=my_open(SYSKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;
    my_FileSeek(fd,offset,fromwhere);
    i=my_FileRead(fd,outdata,max_len);
    my_FileClose(fd);
    return i;
}


int s_WriteSysKeyFile(long offset,unsigned char fromwhere,unsigned char *indata,int max_len)
{
    int fd,i;
    //13/06/02
    int j;
    
    fd=my_open(SYSKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;
    //13/06/02
    j = 0;
MY_FILEREWRITE:
    
    my_FileSeek(fd,offset,fromwhere);
    
  
    i=my_FileWrite(fd,indata,max_len);
    
    if(i<0)
    {
      //liantest
      trace_debug_printf("number[%d] my_FileWrite[%d]\n",j,i);
      
      //delay_ms(50);
      delay_ms(10);
      if(j++<3)
        goto MY_FILEREWRITE; 
    }

    
    my_FileClose(fd);
    return i;
}


int s_ReadAuthKeyFile(uchar *appname,uchar keytype,uchar keyno,unsigned char *outdata,int max_len)
{

    int fd,i;
    uchar app_name[33];
    AUTH_KEY auth_key;

    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;
    while(1)
    {
        i=my_FileRead(fd,outdata,max_len);
        if(i<max_len)
        {
            my_FileClose(fd);
            return -1;
        }
        memcpy((uchar *)&auth_key,outdata,max_len);
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
        if((strcmp(app_name,appname)==0)&&(auth_key.KeyType==keytype)&&(auth_key.KeyNo==keyno))
        {
            break;
        }
    }
    my_FileClose(fd);
    return i;
}


int s_WriteAuthKeyFile(uchar *appname,uchar keytype,uchar keyno,unsigned char *indata,int max_len)
{
    int fd,i,havespace,nVal;
    uchar app_name[33],zerostr[33];
    long offset,offset1;
    AUTH_KEY auth_key;

    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;

    offset=0; 
    offset1=0;
    havespace=0;
    memset(zerostr,0,sizeof(zerostr));
    while(1)
    {
        memset((uchar *)&auth_key,0,sizeof(AUTH_KEY));
        i=my_FileRead(fd,(uchar *)&auth_key,sizeof(AUTH_KEY));
        nVal = sizeof(AUTH_KEY);
        
        //if(i<sizeof(AUTH_KEY))
        if(i<nVal)
        {
            
            if(havespace==0)
            {
                my_FileSeek(fd,0,FILE_SEEK_END);
            } 
            else
            {
                my_FileSeek(fd,offset1,FILE_SEEK_SET);
            }
            break;
        } 
        offset=offset+sizeof(AUTH_KEY); 
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
        if((strcmp(app_name,appname)==0)&&(auth_key.KeyType==keytype)&&(auth_key.KeyNo==keyno))
        {
            offset=sizeof(AUTH_KEY);
            offset=0-offset;
            my_FileSeek(fd,offset,FILE_SEEK_CUR);
            break;
        } 
        else if((memcmp(app_name,zerostr,32)==0) && (0 == havespace))
        {
            havespace=1;
            offset1=offset-sizeof(AUTH_KEY); 
        } 
    }
    i=my_FileWrite(fd,indata,max_len);
    my_FileClose(fd);
    return i;
}

/* 
int s_ReadMMKey(uchar *key_data)
{
    unsigned char edc,keylen,i;  
    memcpy(key_data,"\x11\x22\x33\x44\x55\x66\x77\x88\x99\x00\x11\x22\x33\x44\x55\x66",16);
    return 0; 
}
*/



int s_ReadMdkFile(uchar *appname,uchar keytype,uchar keyno,unsigned char *outdata,int max_len)
{

    int fd,i;
    uchar app_name[33];
    AUTH_KEY auth_key;

    fd=my_open(MDK_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;
    while(1)
    {
        i=my_FileRead(fd,outdata,max_len);
        if(i<max_len)
        {
            my_FileClose(fd);
            return -1;
        }
        memcpy((uchar *)&auth_key,outdata,max_len);
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
        if((strcmp(app_name,appname)==0)&&(auth_key.KeyType==keytype)&&(auth_key.KeyNo==keyno))
        {
            break;
        }
    }
    my_FileClose(fd);
    return i;
}


int s_WriteMdkFile(uchar *appname,uchar keytype,uchar keyno,unsigned char *indata,int max_len)
{
    int fd,i,havespace,nVal;
    uchar app_name[33],zerostr[33];
    long offset,offset1;
    AUTH_KEY auth_key;

    fd=my_open(MDK_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return -1;

    offset=0; 
    offset1=0;
    havespace=0;
    memset(zerostr,0,sizeof(zerostr));
    while(1)
    {
        memset((uchar *)&auth_key,0,sizeof(AUTH_KEY));
        i=my_FileRead(fd,(uchar *)&auth_key,sizeof(AUTH_KEY));
        nVal = sizeof(AUTH_KEY);
        //if(i<sizeof(AUTH_KEY))
        if(i<nVal)
        {
            
            if(havespace==0)
            {
                my_FileSeek(fd,0,FILE_SEEK_END);
            } 
            else
            {
                my_FileSeek(fd,offset1,FILE_SEEK_SET);
            }
            break;
        } 
        offset=offset+sizeof(AUTH_KEY); 
        memset(app_name,0,sizeof(app_name));
        memcpy(app_name,auth_key.AppName,32);
        if((strcmp(app_name,appname)==0)&&(auth_key.KeyType==keytype)&&(auth_key.KeyNo==keyno))
        {
            offset=sizeof(AUTH_KEY);
            offset=0-offset;
            my_FileSeek(fd,offset,FILE_SEEK_CUR);
            break;
        } 
        else if((memcmp(app_name,zerostr,32)==0) && (0 == havespace))
        {
            havespace=1;
            offset1=offset-sizeof(AUTH_KEY); 
        } 
    }
    i=my_FileWrite(fd,indata,max_len);
    my_FileClose(fd);
    return i;
}

/*==============================================================================
 *  写主密钥　
 *原型：int s_WriteDesKey(BYTE key_type,int key_n,BYTE key_len,BYTE *key_data,BYTE key_lrc);
 *参数：key_n:    主密钥编号（适用于多个主密钥情形）
 *      key_len:  主密钥长度（8,16,32）
 *      key_data: 主密钥内容
 *      key_lrc:  主密钥校验值
 *返回：
 *=============================================================================*/
int s_WriteDesKey(uchar app_no,uchar key_type,uchar key_no,uchar key_len,uchar *key_data,uchar key_lrc)
{
    unsigned char lrc[8],i; 
    unsigned char MMK[24];
    unsigned char ELRCK[16];
    int offset;
    int iret;
    DES_KEY  des_key;

    if(app_no>PCI_MAX_APPNUM) return PCI_AppNumOver_Err;
    
    if(key_type==FKEY_TYPE)
    {
        if(key_no>=MAX_FK_NUM) return PCI_KeyNo_Err;
    }
    else
    {
        if(key_no>=MAX_MKSK_NUM) return PCI_KeyNo_Err;
    }
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;
    //检测写密钥权限与认证结果
    //先判断是否已锁键盘
    if(gLockStatus) return PCI_Locked_Err;
    
    iret=s_CheckAllKeysIsDiff(key_type,key_no,key_len,key_data);
    if(iret) return iret;
    
    //须对密钥内容进行加密处理； 
    memset(&des_key,0,sizeof(DES_KEY));
    if(s_PciReadMMK(0, (DWORD*)MMK))    return PCI_ReadMMK_Err;
    if(s_PciReadMMK(1, (DWORD*)ELRCK))  return PCI_ReadMMK_Err;
    
#if 0    
    //liantest 13/06/26 !!!
      Lib_AppShareWrite(32,MMK,24);
      Lib_AppShareWrite(32+24,ELRCK,16);
#endif
      
    switch(key_type)
    {
    case PIN_MKEY_TYPE:
        des_key.KeyType=key_type;
        des_key.KeyNo=key_no;
        des_key.KeyLen=key_len;
        //des_key.Lrc[0]=key_lrc; 
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            des_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            //s_UartPrint(COM_DEBUG,"step3=%d\r\n",iret);
            //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
            //s_UartPrint(COM_DEBUG,"lrc=%02x klr=%02x \r\n",lrc,key_lrc);
            return PCI_KeyLrc_Err;
        }

        //s_UartPrint(COM_DEBUG,"step100=%d Keytype=%d\r\n",key_no,key_type);
        //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
#if 0
        //liantest
        if( (key_no>55) )
        {
        trace_debug_printf("PIN_MKEY_TYPE key_no[%d]\n",key_no);

          trace_debug_printf("MMK:",key_no);
        for(i=0;i<24;i++)
          trace_debug_printf("%02x ",MMK[i]);
        trace_debug_printf("\n");

          trace_debug_printf("in  Keydata2:");
          for(i=0;i<des_key.KeyLen;i++)
            trace_debug_printf("%02x ",des_key.KeyData[i]);
          trace_debug_printf("\n");
        }
#endif        
        
        DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,1);
        
#if 0        
        //liantest
        if( (key_no>55) )
        {        
          trace_debug_printf("des KeyData:");
          for(i=0;i<des_key.KeyLen;i++)
            trace_debug_printf("%02x ",des_key.KeyData[i]);
          trace_debug_printf("\n");
        }
#endif        
        
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0;
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(des_key.Lrc,lrc,5); 
        
        //s_UartPrint(COM_DEBUG,"MMK\r\n");
        //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
        //s_UartPrint(COM_DEBUG,"step101\r\n");
        //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY);
        
        iret=s_WriteSysKeyFile(PCI_PIN_MKEY_ADDR_PAGE0+offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));

#if 0        
        //liantest
        if( (key_no>55) )
        {       
        trace_debug_printf("s_WriteSysKeyFile[%d]\n",iret);
        trace_debug_printf("offset[%d]\n",PCI_PIN_MKEY_ADDR+offset);
        }
#endif        
        
        //13/05/27
        if(iret<0)
        {
          //liantest
          trace_debug_printf("s_WriteSysKeyFile[%d]\n",iret);
          
          return PCI_WriteKey_Err;
        }
        
        //if(iret) return PCI_WriteKey_Err;
        break;
    case MACKEY_TYPE:
        des_key.KeyType=key_type;
        des_key.KeyNo=key_no;
        des_key.KeyLen=key_len;
        //des_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            des_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            //s_UartPrint(COM_DEBUG,"step5=%d\r\n",iret);
            //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
            //s_UartPrint(COM_DEBUG,"lrc=%02x klr=%02x \r\n",lrc,key_lrc);
            return PCI_KeyLrc_Err;
        }
        //s_UartPrint(COM_DEBUG,"step200=%d Keytype=%d\r\n",key_no,key_type);
        //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,1); 
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0;
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(des_key.Lrc,lrc,5); 
        
        //s_UartPrint(COM_DEBUG,"MMK\r\n");
        //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
        //s_UartPrint(COM_DEBUG,"step201\r\n");
        //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY);
        iret=s_WriteSysKeyFile(PCI_MACKEY_ADDR+offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));
        //if(iret) return PCI_WriteKey_Err;
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        break;
    case PINKEY_TYPE:
        des_key.KeyType=key_type;
        des_key.KeyNo=key_no;
        des_key.KeyLen=key_len;
        //des_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            des_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            return PCI_KeyLrc_Err;
        }
        
#if 0 
        //liantest
        if( (key_no>55) )
        {
        trace_debug_printf("PINKEY_TYPE key_no[%d]\n",key_no);

          trace_debug_printf("MMK:",key_no);
        for(i=0;i<24;i++)
          trace_debug_printf("%02x ",MMK[i]);
        trace_debug_printf("\n");

          trace_debug_printf("in  Keydata:");
          for(i=0;i<des_key.KeyLen;i++)
            trace_debug_printf("%02x ",des_key.KeyData[i]);
          trace_debug_printf("\n");
        }
#endif        
        
        DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,1); 
        
#if 0        
        //liantest
        if( (key_no>55) )
        {        
          trace_debug_printf("des KeyData:");
          for(i=0;i<des_key.KeyLen;i++)
            trace_debug_printf("%02x ",des_key.KeyData[i]);
          trace_debug_printf("\n");
        }
#endif        
        
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0;
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(des_key.Lrc,lrc,5); 
        offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY);
        iret=s_WriteSysKeyFile(PCI_PINKEY_ADDR_PAGE2+offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));
#if 0
        //liantest
        if( (key_no>55) )
        {
          trace_debug_printf("s_WriteSysKeyFile[%d]\n",iret);
          trace_debug_printf("offset[%d]\n",PCI_PIN_MKEY_ADDR+offset);
        }
#endif        
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        //if(iret) return PCI_WriteKey_Err;
        break;
    case MAC_MKEY_TYPE:
        des_key.KeyType=key_type;
        des_key.KeyNo=key_no;
        des_key.KeyLen=key_len;
        //des_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            des_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            return PCI_KeyLrc_Err;
        } 
        DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,1); 
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0;
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(des_key.Lrc,lrc,5); 
        offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY);
        iret=s_WriteSysKeyFile(PCI_MAC_MKEY_ADDR+offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        //if(iret) return PCI_WriteKey_Err;
        break;
        
     case FKEY_TYPE:
        des_key.KeyType=key_type;
        des_key.KeyNo=key_no;
        des_key.KeyLen=key_len;
        //des_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            des_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            return PCI_KeyLrc_Err;
        } 
        DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,1); 
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0;
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(des_key.Lrc,lrc,5); 
        offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY);
        iret=s_WriteSysKeyFile(PCI_FKEY_ADDR+offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));
        
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        //if(iret) return PCI_WriteKey_Err;
        break;
        
    default:
        memset(MMK,0,sizeof(MMK));
        return PCI_KeyType_Err;
     }
     memset(MMK,0,sizeof(MMK));
     return 0;
}
//13/07/11
int _s_CheckPinKey(unsigned char *sKey)
{
     unsigned char MMK[24];
     unsigned char ELRCK[16];
     unsigned char i;
     unsigned char edc,lrc[8],keylen;

     DES_KEY des_key;

     memcpy( (unsigned char *)&des_key,sKey,sizeof(DES_KEY) );   
     //test
     /*
     for(i=0;i<32;i++)
       trace_debug_printf("%02x ",sKey[i]);
     trace_debug_printf("\n");
     */
     
     if(des_key.KeyType==0)//save pin pinmk
       return PCI_KeyType_Err;
     keylen=des_key.KeyLen;
     if((keylen!=8)&&(keylen!=16)&&(keylen!=24))
      return PCI_NoKey_Err;
     
     if(s_PciReadMMK(0, (DWORD*)MMK))   return PCI_ReadMMK_Err;
     if(s_PciReadMMK(1, (DWORD*)ELRCK)) return PCI_ReadMMK_Err;
     DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,0x81);
     for(i=0;i<keylen;i++)
          edc^=des_key.KeyData[i];
      
     memset(lrc,0,sizeof(lrc));  
     lrc[0]=edc;
     lrc[1]=des_key.KeyType;
     lrc[2]=des_key.KeyNo;
     lrc[3]=des_key.KeyLen;

     //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
     Lib_DES3_16(lrc,ELRCK,1);

     //if(edc!=des_key.Lrc)
     if(memcmp(lrc,des_key.Lrc,5))
     {
          memset(MMK,0,sizeof(MMK));
          //s_UartPrint(COM_DEBUG,"step4=%02x %02x\r\n",edc,des_key.Lrc);
          return PCI_KeyLrc_Err;
     }
     
     memset(MMK,0,sizeof(MMK));
     
     return 0;
}

/*==============================================================================
 *  读主密钥　
 *     int  s_ReadDesKey(BYTE key_type,int key_n,BYTE *key_len,BYTE *key_data);
 *=============================================================================*/
int s_ReadDesKey(uchar app_no,uchar key_type,uchar key_no,uchar *key_len,uchar *key_data)
{
     unsigned char edc,lrc[8],keylen;
     unsigned char MMK[24];
     unsigned char ELRCK[16];
     DES_KEY des_key;
     int iret,i;
     int offset;

    //须对密钥内容进行加密处理；
    if(app_no>PCI_MAX_APPNUM) return PCI_AppNumOver_Err;
     
    if(key_type==FKEY_TYPE)
    {
        if(key_no>=MAX_FK_NUM) return PCI_KeyNo_Err;
    }
    else
    {
        if(key_no>=MAX_MKSK_NUM) return PCI_KeyNo_Err;
    }
    
    
     //先判断是否已锁键盘
     if(gLockStatus) return PCI_Locked_Err; 
     if(s_PciReadMMK(0, (DWORD*)MMK))   return PCI_ReadMMK_Err;
     if(s_PciReadMMK(1, (DWORD*)ELRCK)) return PCI_ReadMMK_Err;
     switch(key_type)
     {
     case PIN_MKEY_TYPE:
          offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY)+PCI_PIN_MKEY_ADDR_PAGE0;
          break;

     case PINKEY_TYPE:
          offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY)+PCI_PINKEY_ADDR_PAGE2;
          break;

     case MACKEY_TYPE:
          offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY)+PCI_MACKEY_ADDR;
          break;

     case MAC_MKEY_TYPE:
          offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY)+PCI_MAC_MKEY_ADDR;
          break;
     
     case FKEY_TYPE:
          offset=(int)app_no*PCI_APPKEY_SPACE+(int)key_no*sizeof(DES_KEY)+PCI_FKEY_ADDR;
          break;
          
     default:
          memset(MMK,0,sizeof(MMK));
          return PCI_KeyType_Err;
     }
     iret=s_ReadSysKeyFile(offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));

     if(iret<0)
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
#ifdef UARTPRINF_DEBUG          
          //test
    Lib_LcdPrintxy(0,8*1,0x00,"s_ReadSysKeyFile[%d]",iret);
   Lib_KbGetCh();
#endif          
          return PCI_NoKey_Err;
     }
     if(des_key.KeyType!=key_type )
     {
       
#ifdef UARTPRINF_DEBUG       
          //test
    Lib_LcdPrintxy(0,8*1,0x00,"KeyType[%d][%d]",des_key.KeyType,key_type);
   Lib_KbGetCh();
#endif       
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          return PCI_NoKey_Err;
     } 
     
     
     edc=0;
     keylen=des_key.KeyLen;
     if((keylen!=8)&&(keylen!=16)&&(keylen!=24))
     {
#ifdef UARTPRINF_DEBUG       
          //test
    Lib_LcdPrintxy(0,8*1,0x00,"keylen[%d]",keylen);
   Lib_KbGetCh();
#endif       
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          return PCI_NoKey_Err;
     }

     //s_UartPrint(COM_DEBUG,"step300=%d Keytype=%d\r\n",key_no,key_type);
     //for(i=0;i<keylen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);
     DES_TDES(MMK,24,des_key.KeyData,des_key.KeyLen,0x81);
     //s_UartPrint(COM_DEBUG,"MMK\r\n");
     //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
     //s_UartPrint(COM_DEBUG,"step301\r\n");
     //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);


     for(i=0;i<keylen;i++)
          edc^=des_key.KeyData[i];
      
     memset(lrc,0,sizeof(lrc));  
     lrc[0]=edc;
     lrc[1]=des_key.KeyType;
     lrc[2]=des_key.KeyNo;
     lrc[3]=des_key.KeyLen;

     //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
     Lib_DES3_16(lrc,ELRCK,1);

     //if(edc!=des_key.Lrc)
     if(memcmp(lrc,des_key.Lrc,5))
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          //s_UartPrint(COM_DEBUG,"step4=%02x %02x\r\n",edc,des_key.Lrc);
          return PCI_KeyLrc_Err;
     }
     *key_len=keylen;
     memcpy(key_data,des_key.KeyData,keylen);
     
#if 0     
    //test
    Lib_LcdPrintxy(0,8*1,0x00,"keylen[%d][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]",keylen,
                   key_data[0],key_data[1],key_data[2],key_data[3],key_data[4],key_data[5],key_data[6],key_data[7]);
   Lib_KbGetCh();
#endif     
     memset(MMK,0,sizeof(MMK));
     return 0;
}  

int Lib_PciWorkMKeyDes(uchar mkey_no, uchar mkey_part, uchar *indata, uchar *outdata, uchar mode)
{
    int iret=0;
    uchar   mkeybuf[24], keybuf[8], buf[8], mkeylen;

	iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
	if(iret)
	{
		memset(mkeybuf,0,sizeof(mkeybuf));
		return iret;
	}
	memcpy(buf, indata, sizeof(buf));
	switch(mkey_part) 
	{
		case 0x00:
			iret=DES_TDES(mkeybuf,mkeylen,buf,8,mode);
			break;
		case 0x01:
			memcpy(keybuf, &mkeybuf[0], 8);
			Lib_DES(buf, keybuf, mode);
			break;
		case 0x02:
			if (mkeylen < 16)
				return PCI_KeyMode_Err;
			memcpy(keybuf, &mkeybuf[1*8], 8);
			Lib_DES(buf, keybuf, mode);
			break;
		case 0x03:
			if (mkeylen < 24)
				return PCI_KeyMode_Err;
			memcpy(keybuf, &mkeybuf[2*8], 8);
			Lib_DES(buf, keybuf, mode);
			break;
		default:
			return PCI_KeyMode_Err;
	}

	memcpy(outdata, buf, 8);

	return iret;
}


//写加密PINKEY的主密钥　
int Lib_PciWriteMKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode)
{
    uchar keylrc;
    uchar i;
    int iret;
    uchar   mkeybuf[24],mkeylen;

    //printf("PCI_WritePIN_MKey(%02x %02x %02x%02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode);
    switch(mode)
    {
    case 0x00:
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=key_data[i];
        iret=s_WriteDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,key_len,key_data,keylrc);
        return iret;
    case 0xff:
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,&mkeylen,mkeybuf);
        if(iret) return iret;
        if(mkeylen!=key_len)  return PCI_KeyLen_Err;
        for(i=0;i<mkeylen;i++) mkeybuf[i]^=key_data[i];
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=mkeybuf[i];
        iret=s_WriteDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,key_len,mkeybuf,keylrc);
        return iret;

    }
    return PCI_KeyMode_Err;
}


//写加密PINKEY的主密钥　
int Lib_PciWritePIN_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode)
{
    uchar keylrc;
    uchar i;
    int iret;
    uchar   mkeybuf[24],mkeylen;

	return 0; //改用Lib_PciWriteWORK_MKey接口
	
    //printf("PCI_WritePIN_MKey(%02x %02x %02x%02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode);
    switch(mode)
    {
    case 0x00:
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=key_data[i];
        iret=s_WriteDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,key_len,key_data,keylrc);
        return iret;
    case 0xff:
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,&mkeylen,mkeybuf);
        if(iret) return iret;
        if(mkeylen!=key_len)  return PCI_KeyLen_Err;
        for(i=0;i<mkeylen;i++) mkeybuf[i]^=key_data[i];
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=mkeybuf[i];
        iret=s_WriteDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,key_len,mkeybuf,keylrc);
        return iret;

    }
    return PCI_KeyMode_Err;
}


//写加密MACKEY的主密钥　
int Lib_PciWriteMAC_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode)
{
    uchar keylrc;
    uchar i;
    int iret;
    uchar   mkeybuf[24],mkeylen;

    //printf("PCI_WriteMAC_MKey(%02x %02x %02x%02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode);
 
	return 0; //改用Lib_PciWriteWORK_MKey接口

    switch(mode)
    {
    case 0x00:
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=key_data[i];
        iret=s_WriteDesKey(g_byCurAppNum,MAC_MKEY_TYPE,key_no,key_len,key_data,keylrc);
        return iret;
    case 0xff:
        iret=s_ReadDesKey(g_byCurAppNum,MAC_MKEY_TYPE,key_no,&mkeylen,mkeybuf);
        if(iret) return iret;
        if(mkeylen!=key_len)  return PCI_KeyLen_Err;
        for(i=0;i<mkeylen;i++) mkeybuf[i]^=key_data[i];
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=mkeybuf[i];
        iret=s_WriteDesKey(g_byCurAppNum,MAC_MKEY_TYPE,key_no,key_len,mkeybuf,keylrc);
        return iret;

    }
    return PCI_KeyMode_Err;
}



//写加密WORKKEY的主密钥　
int Lib_PciWriteWORK_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i;
    int iret;
    uchar   mkeybuf[24], keybuf[24], mkeylen;
    
#if 0
        //liantest
        if( (key_no>55) )
        {
          trace_debug_printf("\n");
          trace_debug_printf("\nLib_PciWriteWORK_MKey\n");
        
          trace_debug_printf("in  Keydata1:");
          for(i=0;i<key_len;i++)
            trace_debug_printf("%02x ",key_data[i]);
          trace_debug_printf("\n");
        
        }
#endif
        
    //printf("PCI_WritePIN_MKey(%02x %02x %02x%02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode);
	memcpy(keybuf,key_data,key_len);
	switch(mode)
    {
	    case 0x00:
			break;
		case 0x01:
		case 0x81:
			iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
			if(iret)
			{
				memset(mkeybuf,0,sizeof(mkeybuf));
				memset(keybuf,0,sizeof(keybuf));
				return iret;
			}
				
			iret=DES_TDES(mkeybuf,mkeylen,keybuf,key_len,mode);
			if(iret)
			{
				memset(mkeybuf,0,sizeof(mkeybuf));
				memset(keybuf,0,sizeof(keybuf));
				return iret;
			}
			break;
		default: return PCI_KeyMode_Err;

    }

	keylrc=0;
	for(i=0;i<key_len;i++) keylrc^=keybuf[i];
        
        
        
	iret=s_WriteDesKey(g_byCurAppNum,PIN_MKEY_TYPE,key_no,key_len,keybuf,keylrc);
	memset(mkeybuf,0,sizeof(mkeybuf));
	memset(keybuf,0,sizeof(keybuf));

	return iret;
}


//写DESKEY密钥　
int Lib_PciWriteDesKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode)
{
    uchar keylrc;
    uchar i;
    int iret;
    uchar   mkeybuf[24],mkeylen; 
    //printf("PCI_WriteDesKey(%02x %02x %02x%02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode);
 
    switch(mode)
    {
    case 0x00:
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=key_data[i];
        iret=s_WriteDesKey(g_byCurAppNum,FKEY_TYPE,key_no,key_len,key_data,keylrc);
        return iret;
    case 0xff:
        iret=s_ReadDesKey(g_byCurAppNum,FKEY_TYPE,key_no,&mkeylen,mkeybuf);
        if(iret) return iret;
        if(mkeylen!=key_len)  return PCI_KeyLen_Err;
        for(i=0;i<mkeylen;i++) mkeybuf[i]^=key_data[i];
        keylrc=0;
        for(i=0;i<key_len;i++) keylrc^=mkeybuf[i];
        iret=s_WriteDesKey(g_byCurAppNum,FKEY_TYPE,key_no,key_len,mkeybuf,keylrc);
        return iret;

    }
    return PCI_KeyMode_Err;
}
#if 1
//写PIN密钥
int Lib_PciWritePinKey(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i,j;
    int iret;
    uchar mkeybuf[24],pinkeybuf[24],mkeylen;
    uchar sk_mack[24],sk_mack_len,skeycrc[8+24],macbuf[8];

	return 0; //改用Lib_PciWriteWorkKey接口

    //printf("PCI_WritePinKey(%02x %02x %02x%02x %02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode,mkey_no);
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;
        
	#ifdef __BCTC_CRC_VERIFY__
	if (key_crc)
	{
    memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    { 
        return iret;
    }    
    memset(skeycrc,0,sizeof(skeycrc)); 
    memset(macbuf,0,sizeof(macbuf));
    skeycrc[0]=key_no;
    skeycrc[1]=key_len;
    skeycrc[2]=mkey_no; 
    memcpy(&skeycrc[8],key_data,key_len); 
    s_DesMacWithTCBC(skeycrc,8+key_len,sk_mack, sk_mack_len,macbuf); 
    if(memcmp(macbuf,key_crc,4))
    {
        memset(sk_mack,0,sizeof(sk_mack)); 
        return PCI_KeyLrc_Err;
    } 
	}
 	#endif
    //if(key_len<25)  
    memcpy(pinkeybuf,key_data,key_len);
    switch(mode)
    {
    case 0:
        //break;
    case 0x01:
    case 0x81:
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
        //printf("readdeskey(%d)\r\n",iret);        
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        iret=DES_TDES(mkeybuf,mkeylen,pinkeybuf,key_len,mode);
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        break;
    default: return PCI_KeyMode_Err;
    }
    keylrc=0;
    for(i=0;i<key_len;i++) keylrc^=pinkeybuf[i]; 
     
   /*  
    memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return iret;
    }  
    
    memset(skeycrc,0,sizeof(skeycrc)); 
    skeycrc[0]=key_no;
    skeycrc[1]=key_len;
    skeycrc[2]=mkey_no; 
    memcpy(&skeycrc[8],pinkeybuf,key_len);
    for(i=1;i<4;i++)
    {
        for(j=0;j<8;j++) skeycrc[j]=skeycrc[j]^skeycrc[i*8+j];
    } 
    iret=DES_TDES(sk_mack,sk_mack_len,skeycrc,8,1); 
    if(memcmp(skeycrc,key_crc,4))
    {
        memset(sk_mack,0,sizeof(sk_mack));
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return PCI_KeyLrc_Err;
    } 
   */ 
   
    iret=s_WriteDesKey(g_byCurAppNum,PINKEY_TYPE,key_no,key_len,pinkeybuf,keylrc); 
    
    
    memset(sk_mack,0,sizeof(sk_mack));
    memset(mkeybuf,0,sizeof(mkeybuf));
    memset(pinkeybuf,0,sizeof(pinkeybuf));
    
    return iret;

}
#endif


#if 0
//写PIN密钥
int Lib_PciWritePinKey(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i,j;
    int iret;
    uchar mkeybuf[24],pinkeybuf[24],mkeylen;
    uchar sk_mack[24],sk_mack_len,skeycrc[8+24];

    //printf("PCI_WritePinKey(%02x %02x %02x%02x %02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode,mkey_no);
 
    if(key_len<25)  memcpy(pinkeybuf,key_data,key_len);
    switch(mode)
    {
    case 0:
        //break;
    case 0x01:
    case 0x81:
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
        //printf("readdeskey(%d)\r\n",iret);        
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        iret=DES_TDES(mkeybuf,mkeylen,pinkeybuf,key_len,mode);
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        break;
    default: return PCI_KeyMode_Err;
    }
    keylrc=0;
    for(i=0;i<key_len;i++) keylrc^=pinkeybuf[i]; 
     
    /*memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return iret;
    }  
    
    memset(skeycrc,0,sizeof(skeycrc));
    //if(key_crc[0]==0x00 && key_crc[1]==0x00 && key_crc[2]==0x00 && key_crc[3]==0x00)
    //{
    //}
    //else
    //{
        skeycrc[0]=key_no;
        skeycrc[1]=key_len;
        skeycrc[2]=mkey_no; 
        memcpy(&skeycrc[8],pinkeybuf,key_len); 
        
        for(i=1;i<4;i++)
        {
            for(j=0;j<8;j++) skeycrc[j]=skeycrc[j]^skeycrc[i*8+j];
        } 
      
        iret=DES_TDES(sk_mack,sk_mack_len,skeycrc,8,1);  
        
        if(memcmp(skeycrc,key_crc,4))
        {
            memset(sk_mack,0,sizeof(sk_mack));
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return PCI_KeyLrc_Err;
        }*/
        
    //} 
    iret=s_WriteDesKey(g_byCurAppNum,PINKEY_TYPE,key_no,key_len,pinkeybuf,keylrc); 
    
    
    memset(sk_mack,0,sizeof(sk_mack));
    memset(mkeybuf,0,sizeof(mkeybuf));
    memset(pinkeybuf,0,sizeof(pinkeybuf));
    
    return iret;

}
#endif

int Lib_PciWritePinKeyForEMV(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i,j;
    int iret;
    uchar mkeybuf[24],pinkeybuf[24],mkeylen;
    uchar sk_mack[24],sk_mack_len,skeycrc[8+24];

    //printf("PCI_WritePinKey(%02x %02x %02x%02x %02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode,mkey_no);
 
    if(key_len<25)  memcpy(pinkeybuf,key_data,key_len);
    switch(mode)
    {
    case 0:
        //break;
    case 0x01:
    case 0x81:
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
        //printf("readdeskey(%d)\r\n",iret);        
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        iret=DES_TDES(mkeybuf,mkeylen,pinkeybuf,key_len,mode);
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        break;
    default: return PCI_KeyMode_Err;
    }
    keylrc=0;
    for(i=0;i<key_len;i++) keylrc^=pinkeybuf[i]; 
     
    /*memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return iret;
    }  
    
    memset(skeycrc,0,sizeof(skeycrc));
    //if(key_crc[0]==0x00 && key_crc[1]==0x00 && key_crc[2]==0x00 && key_crc[3]==0x00)
    //{
    //}
    //else
    //{
        skeycrc[0]=key_no;
        skeycrc[1]=key_len;
        skeycrc[2]=mkey_no; 
        memcpy(&skeycrc[8],pinkeybuf,key_len); 
        
        for(i=1;i<4;i++)
        {
            for(j=0;j<8;j++) skeycrc[j]=skeycrc[j]^skeycrc[i*8+j];
        } 
      
        iret=DES_TDES(sk_mack,sk_mack_len,skeycrc,8,1);  
        
        if(memcmp(skeycrc,key_crc,4))
        {
            memset(sk_mack,0,sizeof(sk_mack));
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return PCI_KeyLrc_Err;
        }*/
        
    //} 
    iret=s_WriteDesKey(g_byCurAppNum,PINKEY_TYPE,key_no,key_len,pinkeybuf,keylrc); 
    
    
    memset(sk_mack,0,sizeof(sk_mack));
    memset(mkeybuf,0,sizeof(mkeybuf));
    memset(pinkeybuf,0,sizeof(pinkeybuf));
    
    return iret;

}


 // 写MAC密钥　
int Lib_PciWriteMacKey(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i,j;
    int iret;
    uchar mkeybuf[24],mackeybuf[24],mkeylen;
    uchar sk_mack[24],sk_mack_len,skeycrc[8+24],macbuf[8];

	return 0; //改用Lib_PciWriteWorkKey接口

    //printf("PCI_WriteMacKey(%02x %02x %02x%02x %02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode,mkey_no);
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;

 	#ifdef __BCTC_CRC_VERIFY__
	if (key_crc)
	{
	    memset(sk_mack,0,sizeof(sk_mack));
	    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
	    if(iret)
	    { 
	        return iret;
	    }    
	    memset(skeycrc,0,sizeof(skeycrc)); 
	    memset(macbuf,0,sizeof(macbuf));
	    skeycrc[0]=key_no;
	    skeycrc[1]=key_len;
	    skeycrc[2]=mkey_no; 
	    memcpy(&skeycrc[8],key_data,key_len); 
	    s_DesMacWithTCBC(skeycrc,8+key_len,sk_mack, sk_mack_len,macbuf); 
	    if(memcmp(macbuf,key_crc,4))
	    {
	        memset(sk_mack,0,sizeof(sk_mack)); 
	        return PCI_KeyLrc_Err;
	    } 
	}
    #endif
    //if(key_len<25) 
    memcpy(mackeybuf,key_data,key_len);
    switch(mode)
    {
    case 0:
        //break;
    case 0x01:
    case 0x81:
        iret=s_ReadDesKey(g_byCurAppNum,MAC_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
        //s_UartPrint(COM_DEBUG,"step1=%d\r\n",iret);
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(mackeybuf,0,sizeof(mackeybuf));
            return iret;
        }
        
        iret=DES_TDES(mkeybuf,mkeylen,mackeybuf,key_len,mode);
        if(iret)
        {
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(mackeybuf,0,sizeof(mackeybuf));
            return iret;
        }
        
        break;
    default: return PCI_KeyMode_Err;
    }
    keylrc=0;
    for(i=0;i<key_len;i++) keylrc^=mackeybuf[i];
    
   /*
    memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(mackeybuf,0,sizeof(mackeybuf));
        return iret;
    } 
    
    memset(skeycrc,0,sizeof(skeycrc)); 
    skeycrc[0]=key_no;
    skeycrc[1]=key_len;
    skeycrc[2]=mkey_no;
    memcpy(&skeycrc[8],mackeybuf,key_len);
    for(i=1;i<4;i++)
    {
        for(j=0;j<8;j++) skeycrc[j]=skeycrc[j]^skeycrc[i*8+j];
    }
        
        
    iret=DES_TDES(sk_mack,sk_mack_len,skeycrc,8,1); 
    if(memcmp(skeycrc,key_crc,4))
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(mackeybuf,0,sizeof(mackeybuf));
        memset(sk_mack,0,sizeof(sk_mack));
        return PCI_KeyLrc_Err;
    } 
    //s_UartPrint(COM_DEBUG,"\r\nklrc=%02x \r\n",keylrc);
    //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
    */

    iret=s_WriteDesKey(g_byCurAppNum,MACKEY_TYPE,key_no,key_len,mackeybuf,keylrc);
    //s_UartPrint(COM_DEBUG,"step2=%d\r\n",iret);
    
    memset(sk_mack,0,sizeof(sk_mack));
    memset(mkeybuf,0,sizeof(mkeybuf));
    memset(mackeybuf,0,sizeof(mackeybuf));
    
    return iret;
}

int Lib_PciWriteWorkKey(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no)
{
    uchar keylrc;
    uchar i,j;
    int iret;
    uchar mkeybuf[24],pinkeybuf[24],mkeylen;
    uchar sk_mack[24],sk_mack_len,skeycrc[8+24],macbuf[8];

    //printf("PCI_WritePinKey(%02x %02x %02x%02x %02x %02x)\r\n",key_no,key_len,key_data[0],key_data[1],mode,mkey_no);
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;
        
	#ifdef __BCTC_CRC_VERIFY__
	if (key_crc)
	{
    memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    { 
        return iret;
    }    
    memset(skeycrc,0,sizeof(skeycrc)); 
    memset(macbuf,0,sizeof(macbuf));
    skeycrc[0]=key_no;
    skeycrc[1]=key_len;
    skeycrc[2]=mkey_no; 
    memcpy(&skeycrc[8],key_data,key_len); 
    s_DesMacWithTCBC(skeycrc,8+key_len,sk_mack, sk_mack_len,macbuf); 
    if(memcmp(macbuf,key_crc,4))
    {
        memset(sk_mack,0,sizeof(sk_mack)); 
        return PCI_KeyLrc_Err;
    } 
	}
 	#endif
    //if(key_len<25)  
    memcpy(pinkeybuf,key_data,key_len);
    switch(mode)
    {
    case 0:
        break;
    case 0x01:
    case 0x81:
        
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_no,&mkeylen,mkeybuf);
#if 0
        //liantest
        if( (key_no>55) )
        {

          trace_debug_printf(" \n");
          trace_debug_printf(" \nLib_PciWriteWorkKey\n");
          trace_debug_printf("mkey[%d]  data:",mkey_no);
          for(i=0;i<mkeylen;i++)
            trace_debug_printf("%02x ",mkeybuf[i]);
          trace_debug_printf("\n");
          
          trace_debug_printf("in  data:");
          for(i=0;i<key_len;i++)
            trace_debug_printf("%02x ",pinkeybuf[i]);
          trace_debug_printf("\n");
          
        }
#endif        
        
        //printf("readdeskey(%d)\r\n",iret);        
        if(iret)
        {
    //liantest

      trace_debug_printf("s_ReadDesKey[%d]\n",iret);
          
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
        
        iret=DES_TDES(mkeybuf,mkeylen,pinkeybuf,key_len,mode);
        if(iret)
        {
    //liantest
      trace_debug_printf("DES_TDES[%d]\n",iret);
          
            memset(mkeybuf,0,sizeof(mkeybuf));
            memset(pinkeybuf,0,sizeof(pinkeybuf));
            return iret;
        }
#if 0        
        //liantest
        if( (key_no>55) )
        {
          trace_debug_printf("des data:");
          for(i=0;i<key_len;i++)
            trace_debug_printf("%02x ",pinkeybuf[i]);
          trace_debug_printf("\n");
        }
#endif        
        
        break;
    default: return PCI_KeyMode_Err;
    }
    keylrc=0;
    for(i=0;i<key_len;i++) keylrc^=pinkeybuf[i]; 
     
   /*  
    memset(sk_mack,0,sizeof(sk_mack));
    iret=s_ReadSK_MACK(sk_mack,&sk_mack_len);
    if(iret)
    {
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return iret;
    }  
    
    memset(skeycrc,0,sizeof(skeycrc)); 
    skeycrc[0]=key_no;
    skeycrc[1]=key_len;
    skeycrc[2]=mkey_no; 
    memcpy(&skeycrc[8],pinkeybuf,key_len);
    for(i=1;i<4;i++)
    {
        for(j=0;j<8;j++) skeycrc[j]=skeycrc[j]^skeycrc[i*8+j];
    } 
    iret=DES_TDES(sk_mack,sk_mack_len,skeycrc,8,1); 
    if(memcmp(skeycrc,key_crc,4))
    {
        memset(sk_mack,0,sizeof(sk_mack));
        memset(mkeybuf,0,sizeof(mkeybuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return PCI_KeyLrc_Err;
    } 
   */ 
   
   
    iret=s_WriteDesKey(g_byCurAppNum,PINKEY_TYPE,key_no,key_len,pinkeybuf,keylrc); 
    
    //liantest
    if(iret!=0)
      trace_debug_printf("s_WriteDesKey[%d]\n",iret);
    
    memset(sk_mack,0,sizeof(sk_mack));
    memset(mkeybuf,0,sizeof(mkeybuf));
    memset(pinkeybuf,0,sizeof(pinkeybuf));
    
    return iret;

}

int DES_TDES(uchar *key,uchar keylen,uchar *dat,uchar datalen,uchar mode)
{
    uchar chmode;
    chmode=((keylen/8)<<4)+(datalen/8);
    //s_UartPrint(COM_DEBUG,"chmode=%02x\r\n",chmode);
    if(mode!=0x01 && mode!=0x81 && mode!=0) return PCI_KeyMode_Err;
    switch(chmode)
    {
    case 0x11:
        if(mode==0x01)
            Lib_DES(dat,key,1);
        else
            Lib_DES(dat,key,0);
        break;
    case 0x21:
        if(mode==0x01)
        {
            Lib_DES(dat,key,1);
            Lib_DES(dat,&key[8],0);
            Lib_DES(dat,key,1);
        }
        else
        {
            Lib_DES(dat,key,0);
            Lib_DES(dat,&key[8],1);
            Lib_DES(dat,key,0);
        }
        break;
    case 0x31:
        if(mode==0x01)
        {
            Lib_DES(dat,key,1);
            Lib_DES(dat,&key[8],0);
            Lib_DES(dat,&key[16],1);
        }
        else
        {
            Lib_DES(dat,&key[16],0);
            Lib_DES(dat,&key[8],1);
            Lib_DES(dat,key,0);
        }
        break;
    case 0x22:
        if(mode==0x01)
        {
            Lib_DES(dat,key,1);
            Lib_DES(dat,&key[8],0);
            Lib_DES(dat,key,1);
            Lib_DES(&dat[8],key,1);
            Lib_DES(&dat[8],&key[8],0);
            Lib_DES(&dat[8],key,1);
        }
        else
        {
            Lib_DES(dat,key,0);
            Lib_DES(dat,&key[8],1);
            Lib_DES(dat,key,0);
            Lib_DES(&dat[8],key,0);
            Lib_DES(&dat[8],&key[8],1);
            Lib_DES(&dat[8],key,0);
        }
        break;
    //case 0x23:
    //    if(mode==0x01)
    //    {
    //        Lib_DES(dat,key,1);
    //        Lib_DES(dat,&key[8],0);
    //        Lib_DES(dat,key,1);
    //        Lib_DES(&dat[8],key,1);
    //        Lib_DES(&dat[8],&key[8],0);
    //        Lib_DES(&dat[8],key,1);
    //        Lib_DES(&dat[16],key,1);
    //        Lib_DES(&dat[16],&key[8],0);
    //        Lib_DES(&dat[16],key,1);
    //    }
    //    else
    //    {
    //        Lib_DES(dat,key,0);
    //        Lib_DES(dat,&key[8],1);
    //        Lib_DES(dat,key,0);
    //        Lib_DES(&dat[8],key,0);
    //        Lib_DES(&dat[8],&key[8],1);
    //        Lib_DES(&dat[8],key,0);
    //        Lib_DES(&dat[16],key,0);
    //        Lib_DES(&dat[16],&key[8],1);
    //        Lib_DES(&dat[16],key,0);
    //    }
    //    break;
    case 0x32:
        if(mode==0x01)
        {
            Lib_DES(dat,key,1);
            Lib_DES(dat,&key[8],0);
            Lib_DES(dat,&key[16],1);
            Lib_DES(&dat[8],key,1);
            Lib_DES(&dat[8],&key[8],0);
            Lib_DES(&dat[8],&key[16],1);
        }
        else
        {
            Lib_DES(dat,&key[16],0);
            Lib_DES(dat,&key[8],1);
            Lib_DES(dat,key,0);
            Lib_DES(&dat[8],&key[16],0);
            Lib_DES(&dat[8],&key[8],1);
            Lib_DES(&dat[8],key,0);
        }
        break;
    case 0x33:
        if(mode==0x01)
        {
            Lib_DES(dat,key,1);
            Lib_DES(dat,&key[8],0);
            Lib_DES(dat,&key[16],1);
            Lib_DES(&dat[8],key,1);
            Lib_DES(&dat[8],&key[8],0);
            Lib_DES(&dat[8],&key[16],1);
            Lib_DES(&dat[16],key,1);
            Lib_DES(&dat[16],&key[8],0);
            Lib_DES(&dat[16],&key[16],1);
        }
        else
        {
            Lib_DES(dat,&key[16],0);
            Lib_DES(dat,&key[8],1);
            Lib_DES(dat,key,0);
            Lib_DES(&dat[8],&key[16],0);
            Lib_DES(&dat[8],&key[8],1);
            Lib_DES(&dat[8],key,0);
            Lib_DES(&dat[16],&key[16],0);
            Lib_DES(&dat[16],&key[8],1);
            Lib_DES(&dat[16],key,0);
        }
        break;
    default : return PCI_KeyMode_Err;
    }
    return 0;
}

int s_DeriveKeySub(uchar app_n,uchar mkey_n,uchar wkey_n1,uchar wkey_n2,uchar mode,uchar wkey_type)
{

     DES_KEY mkey,wkey1;
     uchar   mkeybuf[24],wkeybuf[24],mkeylen,wkeylen,keymode;
     uchar   keylrc;
     uchar   i;
     int     iret;
     if(wkey_type==PINKEY_TYPE)
        iret=s_ReadDesKey(g_byCurAppNum,PIN_MKEY_TYPE,mkey_n,&mkeylen,mkeybuf);
     else
        iret=s_ReadDesKey(g_byCurAppNum,MAC_MKEY_TYPE,mkey_n,&mkeylen,mkeybuf);
     if(iret) return iret;
     iret=s_ReadDesKey(g_byCurAppNum,wkey_type,wkey_n1,&wkeylen,wkeybuf);
     if(iret) return iret;

     iret=DES_TDES(mkeybuf,mkeylen,wkeybuf,wkeylen,mode);
     if(iret) return iret;

     keylrc=0;
     for(i=0;i<wkeylen;i++) keylrc^=wkeybuf[i];
     iret=s_WriteDesKey(g_byCurAppNum,wkey_type,wkey_n2,wkeylen,wkeybuf,keylrc);
     return iret;
}



 // 发散PIN密钥　
int Lib_PciDerivePinKey(BYTE mkey_n,BYTE pinkey_n1,BYTE pinkey_n2,BYTE mode)
{ 
    return s_DeriveKeySub(g_byCurAppNum,mkey_n,pinkey_n1,pinkey_n2,mode,PINKEY_TYPE);

}

 // 发散MAC密钥　
int Lib_PciDeriveMacKey(BYTE mkey_n,BYTE mackey_n1,BYTE mackey_n2,BYTE mode)
{
 
    return s_DeriveKeySub(g_byCurAppNum,mkey_n,mackey_n1,mackey_n2,mode,MACKEY_TYPE);
}

#if 0
//  获取加密PIN（X9.8 X3.92）　
int Lib_PciGetPin(BYTE pinkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec)
{

	uchar i, def_max_len;
     uchar temp[24],one[9],two[9],three[9];
     uchar pin_len,len;
     uchar pinkeybuf[24], pinkeylen,pinbuf[24];
     int   iret;

     //printf("PCI_GetPin(%02x %02x %02x %02x %d)\r\n",pinkey_n,min_len,max_len,mode,waittime_sec);
 

     //if(k_LockStatus) return PCI_Locked_Err;
     memset(pin_block,0,8);

#ifdef  __NEW_PCI_REQUIRE__
	if(s_PciCheckCallInteralMacro())
		return PCI_CallTimeInte_Err;
#else
     if(g_PinInputIntervalCtrl)
     {
          if(!s_CheckTimer(0))
		  {
               s_SetTimer(0,PCI_TIME_OUT);
               return PCI_CallTimeInte_Err;
          }
     }
     else
	 {
		g_PinInputIntervalCtrl=1;
		s_SetTimer(0,PCI_TIME_OUT);
     }
#endif

     def_max_len =14;
     if (min_len>max_len || max_len>def_max_len)
     {
#ifdef  __NEW_PCI_REQUIRE__
		 DelInvalidCall();
#else
          s_SetTimer(0,PCI_TIME_OUT);
#endif
          return PCI_InputLen_Err;
     }
     //if((pinkey_n & 0x7f)>=50)
     if(pinkey_n >= 50)
     {
#ifdef  __NEW_PCI_REQUIRE__
		 DelInvalidCall();
#else
          s_SetTimer(0,PCI_TIME_OUT);
#endif
          return PCI_KeyNo_Err;
     }

     //if(pinkey_n & 0x80)
     //{  //增加20070515：为了支持直联的工作密钥方式。
     //     iret=s_ReadDesKey(g_byCurAppNum,DESKEY_TYPE,pinkey_n&0x7f,&pinkeylen,pinkeybuf);
     //}
     //else
          iret=s_ReadDesKey(g_byCurAppNum,PINKEY_TYPE,pinkey_n,&pinkeylen,pinkeybuf);
     if(iret) return iret;


     if(mode & 0x80)  gNoPinEnterDisenable=1;
     else             gNoPinEnterDisenable=0;

     memset(pinbuf,0,sizeof(pinbuf));
     iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
#ifdef  __NEW_PCI_REQUIRE__
	 ;
#else
          s_SetTimer(0,PCI_TIME_OUT);
#endif
     if(iret)
     {
#ifdef  __NEW_PCI_REQUIRE__
		 DelInvalidCall();
#endif 
          memset(pinbuf,0,sizeof(pinbuf));
          memset(pinkeybuf,0,sizeof(pinkeybuf));
          return iret;
     }

     pin_len =strlen((char *)pinbuf);
     if(pin_len ==0)
     {
          memset(pinbuf,0,sizeof(pinbuf));
          memset(pinkeybuf,0,sizeof(pinkeybuf));
          return PCI_NoInput_Err;
     }

     memset(temp,0,sizeof(temp));
     strcpy((char *)temp,"0000");
     memcpy(&temp[4], card_no+4, 12);
     vTwoOne(temp, 16, one);

     memset(temp,0,sizeof(temp));
     sprintf((char *)temp, "%02d", (int)pin_len);
     strcpy((char *)&temp[2], (char *)pinbuf);
     i=strlen((char *)temp);
     memset(&temp[i], 'F', 16-i);
     vTwoOne(temp,16,two);
     two[0]=pin_len;
     for(i=0;i<8;i++)  three[i]=one[i] ^ two[i];
     DES_TDES(pinkeybuf,pinkeylen,three,8,1);
     memcpy(pin_block, three, 8);
     memset(one,0,sizeof(one));
     memset(two,0,sizeof(two));
     memset(three,0,sizeof(three));
     memset(pinbuf,0,sizeof(pinbuf));
     memset(pinkeybuf,0,sizeof(pinkeybuf));
     return 0;

}
#endif


//  获取加密PIN（X9.8 X3.92）　
int Lib_PciGetPin(BYTE pinkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec)
{
	uchar i, def_max_len;
	uchar temp[24],one[9],two[9],three[9];
	uchar pin_len,len;
	uchar pinkeybuf[24], pinkeylen,pinbuf[24],strict_buf[8];
	int   iret;
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];
 
	
	memset(pin_block,0,8);
	
	def_max_len =14;
	if (min_len>max_len || max_len>def_max_len)
	{
		return PCI_InputLen_Err;
	}
	if(pinkey_n >= PCI_MAX_KEYNUM)
	{
		return PCI_KeyNo_Err;
	}
//////////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
	 //get current time in seconds
	 //Lib_GetCurSecond(&timebefore);
	 Lib_GetDateTime(cur_time);
	 timebefore=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
     if(strict_buf[0] == 0xff && strict_buf[1] == 0xff && strict_buf[2] == 0xff)//first time
     {
		 memset(strict_buf,0x00,sizeof(strict_buf));
		 ByteToDword(strict_buf,8,dwCount);
		 s_PciWritePinCount(0,dwCount[0]);
		 s_PciWritePinCount(1,dwCount[1]);
     }

     //read the count
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
	 count = strict_buf[0];
     if(count == 120)//the max count in max time
     {
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
         timetemp = (strict_buf[4]*256+strict_buf[5])*65535 + strict_buf[6]*256 + strict_buf[7];
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
		 //Lib_GetCurSecond(&dwCount[0]);
		 Lib_GetDateTime(cur_time);
		 dwCount[0]=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
         interval = dwCount[0]-timetemp+strict_buf[2]*256+strict_buf[1];//read the interval time
         if(interval < 3600)//the max time value
         {
             //Lib_GetCurSecond(&timemiddle);
			 Lib_GetDateTime(cur_time);
			 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
			 s_PciWritePinCount(1,timemiddle);
             return PCI_CallTimeInte_Err;
         }
         else
         {
             count = 0;
             interval = 0;
			 memset(strict_buf,0,sizeof(strict_buf));
			 ByteToDword(strict_buf,4,&dwCount[0]);
			 s_PciWritePinCount(0,dwCount[0]);
         }
     }
#endif
//////////////////////////////////////////////////
	iret=s_ReadDesKey(g_byCurAppNum,PINKEY_TYPE,pinkey_n,&pinkeylen,pinkeybuf);
	if(iret) return iret;
	
	
	if(mode & 0x80)  gNoPinEnterDisenable=1;
	else             gNoPinEnterDisenable=0;
	
	memset(pinbuf,0,sizeof(pinbuf));
	iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
	if(iret)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(pinkeybuf,0,sizeof(pinkeybuf));
		return iret;
	}
	
	pin_len =strlen((char *)pinbuf);
	if(pin_len ==0)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(pinkeybuf,0,sizeof(pinkeybuf));
		return PCI_NoInput_Err;
	}
///////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
     //Lib_GetCurSecond(&timemiddle);
	 Lib_GetDateTime(cur_time);
	 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciWritePinCount(1,timemiddle);
     count++;
     //save the count
     //Lib_GetCurSecond(&timeafter);//get current time
	 Lib_GetDateTime(cur_time);
	 timeafter=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
     interval = timeafter - timebefore;
	 s_PciReadPinCount(0,&dwCount[0]);
	 DwordToBytes(&dwCount[0],1,strict_buf);
     interval = interval + strict_buf[2]*256 + strict_buf[1];//accumulate the interval time
	 strict_buf[0] = count;
	 strict_buf[1] = interval%256;
	 strict_buf[2] = interval/256;
	 ByteToDword(strict_buf,4,&dwCount[0]);
	 s_PciWritePinCount(0,dwCount[0]);
#endif
///////////////////////////////////////////////
	memset(temp,0,sizeof(temp));
	strcpy((char *)temp,"0000");
	memcpy(&temp[4], card_no+4, 12);
	vTwoOne(temp, 16, one);
	
	memset(temp,0,sizeof(temp));
	sprintf((char *)temp, "%02d", (int)pin_len);
	strcpy((char *)&temp[2], (char *)pinbuf);
	i=strlen((char *)temp);
	memset(&temp[i], 'F', 16-i);
	vTwoOne(temp,16,two);
	two[0]=pin_len;
	for(i=0;i<8;i++)  three[i]=one[i] ^ two[i];
#if 0        
        //test
    Lib_LcdPrintxy(0,8*1,0x00,"keylen[%d] [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]",
                   pinkeylen,pinkeybuf[0],pinkeybuf[1],pinkeybuf[2],pinkeybuf[3],
                   pinkeybuf[4],pinkeybuf[5],pinkeybuf[6],pinkeybuf[7]);
   Lib_KbGetCh();
   /*
   pinkeylen = 24;//
        //memcpy(pinkeybuf,"\x33\x33\x33\x33\x33\x33\x33\x33",8);
        //memcpy(pinkeybuf,"\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",16);
        memcpy(pinkeybuf,"\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32\x32",24);
   */     
#endif        
	DES_TDES(pinkeybuf,pinkeylen,three,8,1);
	memcpy(pin_block, three, 8);
	memset(one,0,sizeof(one));
	memset(two,0,sizeof(two));
	memset(three,0,sizeof(three));
	memset(pinbuf,0,sizeof(pinbuf));
	memset(pinkeybuf,0,sizeof(pinkeybuf));
	return 0;
	
}

//  获取加密PIN（X9.8 X3.92）　
int Lib_PciGetPinAuto(BYTE pinkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec)
{

	uchar i,j, def_max_len;
     uchar temp[24],one[9],two[9],three[9];
     uchar pin_len,len;
     uchar pinkeybuf[24], pinkeylen,pinbuf[24];
     int   iret;
	 BYTE  rand[16];
     //printf("PCI_GetPin(%02x %02x %02x %02x %d)\r\n",pinkey_n,min_len,max_len,mode,waittime_sec);
 

     //if(k_LockStatus) return PCI_Locked_Err;
     memset(pin_block,0,8);


     def_max_len =14;
     if (min_len>max_len || max_len>def_max_len)
     {
          return PCI_InputLen_Err;
     }
     //if((pinkey_n & 0x7f)>=50)
     if(pinkey_n >= PCI_MAX_KEYNUM)
     {
          return PCI_KeyNo_Err;
     }

	 iret=s_ReadDesKey(g_byCurAppNum,PINKEY_TYPE,pinkey_n,&pinkeylen,pinkeybuf);
     if(iret) return iret;


     if(mode & 0x80)  gNoPinEnterDisenable=1;
     else             gNoPinEnterDisenable=0;

     memset(pinbuf,0,sizeof(pinbuf));
     iret = s_PciInputPinAuto(pinbuf);//iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
     if(iret)
     {
          memset(pinbuf,0,sizeof(pinbuf));
          memset(pinkeybuf,0,sizeof(pinkeybuf));
          return iret;
     }

     pin_len =strlen((char *)pinbuf);
     if(pin_len ==0)
     {
          memset(pinbuf,0,sizeof(pinbuf));
          memset(pinkeybuf,0,sizeof(pinkeybuf));
          return PCI_NoInput_Err;
     }
	 //开始加密pin之前beep
	 Lib_Beep();

//     memset(temp,0,sizeof(temp));
//     strcpy((char *)temp,"0000");
//     memcpy(&temp[4], card_no+4, 12);
//     vTwoOne(temp, 16, one);

//format 1:
	//the plain text  PIN field
     memset(temp,0,sizeof(temp));
	 temp[0] = 0x01;
	 temp[1] = pin_len;
     strcpy((char *)&temp[2], (char *)pinbuf);
     i=strlen((char *)temp);
     Lib_PciGetRnd(rand);
	 Lib_PciGetRnd(&rand[8]);

	 for (j = 0 ;j < 16-i; j++)
	 {
		 temp[i+j] = (rand[j]%15);
	 }
     vTwoOne(temp,16,one);
	 //transaction field
	 memset(temp,0,sizeof(temp));
	 for (j = 0 ; j < (56-pin_len*4)/8; j++);
	 {
		 temp[i] = rand[j];
	 }
	 memcpy(&one[8],temp,(56-pin_len*4)/8);
     DES_TDES(pinkeybuf,pinkeylen,one,(8+(56-pin_len*4)/8),1);
     memcpy(pin_block, one, 8);

	 //清除临时缓冲区
     memset(one,0,sizeof(one));
//     memset(two,0,sizeof(two));
//     memset(three,0,sizeof(three));
     memset(pinbuf,0,sizeof(pinbuf));
     memset(pinkeybuf,0,sizeof(pinkeybuf));
     return 0;

}


int  Lib_PciGetOfflinePin(BYTE min_len,BYTE max_len,BYTE *encpin,unsigned short waittime_sec)
{
    uchar i, j,def_max_len,ret;
    uchar temp[24],one[9],two[9],three[9],rand[16];
	uchar pin_len,len;
	uchar fixkeybuf[24], fixkeylen,pinbuf[24],strict_buf[8];
	int   iret;
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];
	
	
	
	def_max_len =14;
	if (min_len>max_len || max_len>def_max_len)
	{
		return PCI_InputLen_Err;
	}

	iret=s_ReadDesKey(g_byCurAppNum,FKEY_TYPE,0,&fixkeylen,fixkeybuf); 
	if(iret) return iret;
	
	
	//if(mode & 0x80)  gNoPinEnterDisenable=1;
	//else             
		gNoPinEnterDisenable=0;
	
	memset(pinbuf,0,sizeof(pinbuf));
	iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
	if(iret)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(fixkeybuf,0,sizeof(fixkeybuf));
		return iret;
	}
	
	pin_len =strlen((char *)pinbuf);
	if(pin_len ==0)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(fixkeybuf,0,sizeof(fixkeybuf));
		return PCI_NoInput_Err;
	}

	pin_len =strlen((char *)pinbuf);
	temp[0]=0x20+pin_len;
	memset(pinbuf+pin_len, 'F', 14);
	vTwoOne(pinbuf, 14, temp+1);
	
	DES_TDES(fixkeybuf,fixkeylen,temp,8,1);
	
	memcpy(encpin, temp, 8);
	memset(temp,0,sizeof(temp));
	memset(pinbuf,0,sizeof(pinbuf));
	memset(fixkeybuf,0,sizeof(fixkeybuf));
	
	return 0;
}


/*--------------------------------------------------------
 首8个字节做1次DES加密，结果再与下一组数据（8个字节）
 异或，再做1次DES加密，……直到最后一组数据作完1次
 DES加密，得出MAC运算结果（8个字节）。
 ----------------------------------------------------------*/
//void DesMac(uchar *key,uchar *mdat,uchar length)
void DesMac(uchar *key,uchar *mdat,ushort length)
{
    ushort i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)  mdat[j] ^= mdat[j+8*i];
        Lib_DES(mdat,key,1);
    }
}

/*--------------------------------------------------------
 首8个字节做1次3DES加密，结果再与下一组数据（8个字节）
 异或，再做1次3DES加密，……直到最后一组数据作完1次
 3DES加密，得出MAC运算结果（8个字节）。
 ----------------------------------------------------------*/
void Des3_16Mac(uchar *key,uchar *mdat,ushort length)
{
    ushort i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES3_16(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
        Lib_DES3_16(mdat,key,1);
    }
}
void Des3_24Mac(uchar *key,uchar *mdat,ushort length)
{
    ushort i,j,l;
    l=length/8;
    i =length%8;
    if(i)
    {
        memset(&mdat[length],0x00,8-i);
        l ++;
    }
    Lib_DES3_24(mdat,key,1);
    for(i=1;i<l;i++)
    {
        for(j=0;j<8;j++)        mdat[j] ^= mdat[j+8*i];
        Lib_DES3_24(mdat,key,1);
    }
}

void s_DesMacWithTCBC(unsigned char *pbuInData, unsigned char datalen,unsigned char *pbyKey, unsigned char keylen, unsigned char *pbyOutData)
{ 
	unsigned char abydata[256]; 
    memset(abydata,0,sizeof(abydata));
	memcpy(abydata,pbuInData,datalen); 
    switch(keylen/8)
    {
    case 0x01:
        DesMac(pbyKey,abydata,datalen);
        break;
    case 0x02:
        Des3_16Mac(pbyKey,abydata,datalen);
        break;
    case 0x03:
        Des3_24Mac(pbyKey,abydata,datalen);
        break;
    default: 
        memset(abydata,0,sizeof(abydata));
        break;
    } 
    memcpy(pbyOutData,abydata,8);
    
}
/*--------------------------------------------------------
 首8个字节做1次DES加密，结果再与下一组数据（8个字节）
 异或，再做1次DES加密，……直到最后一组数据作完1次
 3DES加密，得出MAC运算结果（8个字节）。
 ----------------------------------------------------------*/
void Des3_16Mac_2(uchar *key,uchar *mdat,ushort length)
{
     ushort i,j,l;
     l=length/8;
     i =length%8;
     if(i)
     {
          memset(&mdat[length],0x00,8-i);
          l ++;
     }
     if(l<=1)
     {
          Lib_DES3_16(mdat,key,1);
          return;
     }

     Lib_DES(mdat,key,1);
     for(i=1;i<l-1;i++)
     {
          for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
          Lib_DES(mdat,key,1);
     }
     for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
     Lib_DES3_16(mdat,key,1);

}
void Des3_24Mac_2(uchar *key,uchar *mdat,ushort length)
{
     ushort i,j,l;
     l=length/8;
     i =length%8;
     if(i)
     {
          memset(&mdat[length],0x00,8-i);
          l ++;
     }

     if(l<=1)
     {
          Lib_DES3_24(mdat,key,1);
          return;
     }

     Lib_DES(mdat,key,1);
     for(i=1;i<l-1;i++)
     {
          for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
          Lib_DES(mdat,key,1);
     }
     for(j=0;j<8;j++)   mdat[j] ^= mdat[j+8*i];
     Lib_DES3_24(mdat,key,1);
}

//  获取MAC
int Lib_PciGetMac(BYTE mackey_n,WORD inlen,BYTE *indata,BYTE *macout,BYTE mode)
{
    uchar   mackeybuf[24],mackeylen,keymode,datain[2050];

    int     iret;
    ushort temp,i,inLen,j,k; 
    //test
    uchar MMK[24];
    //printf("PCI_GetMac(%02x %d %02x)\r\n",mackey_n,inlen,mode);  

#if 0 
        //liantest
        if( (mackey_n>55) )
        {
          trace_debug_printf("\n");
          trace_debug_printf("\nLib_PciGetMac\n");
          
          trace_debug_printf("mackey_n[%d]  inlen[%d] mode[%02x]",mackey_n,inlen,mode);
          if(memcmp(indata,"\x94\x1d\xa8\xc5\x8d\x56\x14\x1e\x00\x00\x00\x00\x00\x00\x00\x00",inlen))
          {
            trace_debug_printf("!!!NO The same data:");
          for(i=0;i<inlen;i++)
            trace_debug_printf("%02x ",indata[i]);
          trace_debug_printf("\n");
            
          }            
        }
#endif        
    
    //if((mackey_n & 0x7f)>=50)
    if(mackey_n >= PCI_MAX_KEYNUM)
    {
        return PCI_KeyNo_Err;
    }

    //if(mackey_n & 0x80)
    //{  //增加20070515：为了支持直联的工作密钥方式。
    //    iret=s_ReadDesKey(g_byCurAppNum,DESKEY_TYPE,mackey_n&0x7f,&mackeylen,mackeybuf);
    //}
    //else
        iret=s_ReadDesKey(g_byCurAppNum,PINKEY_TYPE,mackey_n,&mackeylen,mackeybuf);
        
        
    if(iret)
    {
//#if 0
      trace_debug_printf("FLASH->CR[%x]\n",FLASH->CR);
      
      //liantest 13/06/25
      trace_debug_printf("s_ReadDesKey[%d]\n",iret);
      //MMK
      s_PciReadMMK(0,(DWORD*)MMK);
      trace_debug_printf("MMK:");
      for(i=0;i<24;i++)
          trace_debug_printf("%02x ",MMK[i]);
      trace_debug_printf("\n");
      //ELRCK
      s_PciReadMMK(1,(DWORD*)MMK);
      trace_debug_printf("ELRCK:");
      for(i=0;i<16;i++)
          trace_debug_printf("%02x ",MMK[i]);
      trace_debug_printf("\n");
      
      //first
      for(j=0;j<4;j++)
      {
        memset(datain,0,sizeof(datain));
        sys_flash_read_operate(j*2048,datain,2048);
        trace_debug_printf("%i page:\n",j);
        for(i=0;i<2048;i++)
          trace_debug_printf("%02x ",datain[i]);
        trace_debug_printf("\n");
      }
      //save  data
      /*
      Lib_AppShareRead(0,datain,32+24+16);
      trace_debug_printf("VANSTONE:");
      for(i=0;i<32;i++);
        trace_debug_printf("%c ",datain[i]);
      trace_debug_printf("\n");  
      */
      
      trace_debug_printf("MMK:");
      for(i=0;i<24;i++);
        trace_debug_printf("%02x ",datain[32+i]);
      trace_debug_printf("\n");  
      trace_debug_printf("ELRCK:");
      for(i=0;i<16;i++);
        trace_debug_printf("%02x ",datain[32+16+i]);
      trace_debug_printf("\n");  
//#endif
      
      return iret;
    }  
    
#if 0
    //liantest
       if( (mackey_n>55) )
        {
           trace_debug_printf("mackeybuf data:");
          for(i=0;i<mackeylen;i++)
            trace_debug_printf("%02x ",mackeybuf[i]);
          trace_debug_printf("\n");
          
        }
#endif      
    
    if(inlen>2048 || inlen==0)
    {
        return PCI_DataLen_Err;
    }

    memset(datain,0,sizeof(datain));
    memcpy(datain,indata,inlen);

     if(mode==1){
        temp=inlen%8;
        if(temp) temp=8-temp;
        memset(&datain[inlen],0,temp);
        k=(ushort)inlen+(ushort)temp;
        k=k/8;
        for(i=1;i<k;i++)
        {
            for(j=0;j<8;j++) datain[j] ^= datain[j+8*i];
        }
        DES_TDES(mackeybuf,mackeylen,datain,8,1);
     }
     else if(mode==2)
     {
          switch(mackeylen/8)
          {
               case 0x01:
                    DesMac(mackeybuf,datain,inlen);
                    break;
               case 0x02:
                    Des3_16Mac_2(mackeybuf,datain,inlen);
                    break;
               case 0x03:
                    Des3_24Mac_2(mackeybuf,datain,inlen);
                    break;
               default: return PCI_KeyLen_Err;
          }
     }

    else{
        switch(mackeylen/8)
        {
        case 0x01:
            DesMac(mackeybuf,datain,inlen);
            break;
        case 0x02:
            Des3_16Mac(mackeybuf,datain,inlen);
            break;
        case 0x03:
            Des3_24Mac(mackeybuf,datain,inlen);
            break;
        default: return PCI_KeyLen_Err;
        }
    }
    memcpy(macout,datain,8);
    
#if 0    
    //liantest
      // if( (mackey_n>55) && (mackey_n<62))
        {
           trace_debug_printf("mackey_n[%d] macout data:",mackey_n);
          for(i=0;i<8;i++)
            trace_debug_printf("%02x ",datain[i]);
          trace_debug_printf("\n");
          
        }
#endif    
    
    memset(mackeybuf,0,sizeof(mackeybuf));
    return 0;
}


 // 获取随机数
int Lib_PciGetRnd (BYTE *rnd8)
{ 
    unsigned char tempkey[8]; 
    unsigned int l0,l1,l2;  
    //l0=s_ReadMipsCpuCount(); 
    l0=SysTick_GetCounter();
    tempkey[0]=(unsigned char)(l0>>24)&0xff;
    tempkey[1]=(unsigned char)(l0>>16)&0xff;
    tempkey[2]=(unsigned char)(l0>>8)&0xff;
    tempkey[3]=(unsigned char)(l0>>0)&0xff;    
    g_PciRandCount++;
    tempkey[4]=(unsigned char)(g_PciRandCount>>24)&0xff;
    tempkey[5]=(unsigned char)(g_PciRandCount>>16)&0xff;
    tempkey[6]=(unsigned char)(g_PciRandCount>>8)&0xff;
    tempkey[7]=(unsigned char)(g_PciRandCount>>0)&0xff;  
    hal_rng_read(&l1);
    rnd8[0]=(unsigned char)(l1>>24)&0xff;
    rnd8[1]=(unsigned char)(l1>>16)&0xff;
    rnd8[2]=(unsigned char)(l1>>8)&0xff;
    rnd8[3]=(unsigned char)(l1>>0)&0xff;
    hal_rng_read(&l2);
    rnd8[4]=(unsigned char)(l2>>24)&0xff;
    rnd8[5]=(unsigned char)(l2>>16)&0xff;
    rnd8[6]=(unsigned char)(l2>>8)&0xff;
    rnd8[7]=(unsigned char)(l2>>0)&0xff; 
    Lib_Des(rnd8,rnd8,tempkey,1); 
    memcpy(Auth_Result.Rand,rnd8,8); 
    return 0;  
}

//  进行修改密钥权限认证　
int  Lib_PciAccessAuth(BYTE *auth_data,BYTE mode)
{
    return 0;
}

/*==============================================================
  函数功能: DES(3DES)计算
  mackey_n  计算DES的密钥(DES密钥区)
  datain    输入数据
  macout    输出数据
  mode=0x01 DES加密
  mode=0x00 DES解密
  ================================================================*/ 
int  Lib_PciDes(BYTE deskey_n, BYTE *indata, BYTE *outdata, BYTE mode)
{
    uchar   deskeybuf[24],deskeylen,keymode,datain[8];
    int     iret;
    uchar   tmpmode=mode;

    if(deskey_n>=MAX_FK_NUM )
    {
        return PCI_KeyNo_Err;
    }
    iret=s_ReadDesKey(g_byCurAppNum,FKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
    if(iret) return iret;
    memset(datain,0,sizeof(datain));
    memcpy(datain,indata,8); 
    switch(deskeylen/8)
    {
    case 0x01:
        if(tmpmode)
        {
            Lib_DES(datain,deskeybuf,1);
        }
        else
        {
            Lib_DES(datain,deskeybuf,0);
        }

        break;
    case 0x02:
        if(tmpmode)
        {
            Lib_DES(datain,deskeybuf,1);
            Lib_DES(datain,&deskeybuf[8],0);
            Lib_DES(datain,deskeybuf,1);
        }
        else
        {
            Lib_DES(datain,deskeybuf,0);
            Lib_DES(datain,&deskeybuf[8],1);
            Lib_DES(datain,deskeybuf,0);
        }
        break;
    case 0x03:
        if(tmpmode)
        {
            Lib_DES(datain,deskeybuf,1);
            Lib_DES(datain,&deskeybuf[8],0);
            Lib_DES(datain,&deskeybuf[16],1);
        }
        else
        {
            Lib_DES(datain,&deskeybuf[16],0);
            Lib_DES(datain,&deskeybuf[8],1);
            Lib_DES(datain,deskeybuf,0);
        }
        break;
    default: return PCI_KeyLen_Err;
    }

    memcpy(outdata,datain,8);
    memset(deskeybuf,0,24);
    return 0;
} 
 

/*------------------------------------------------
         Two ASCII Bytes to One Hex Byte
 ------------------------------------------------*/
void vTwoOne(uchar *in, ushort in_len, uchar *out)
{
    uchar tmp;
    ushort i;

    for(i=0;i<in_len;i+=2)
    {
        tmp = in[i];
        if(tmp > '9')
            tmp = toupper(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        tmp <<= 4;
        out[i/2]=tmp;

        tmp=in[i+1];
        if(tmp>'9')
            tmp = toupper(tmp) - ('A' - 0x0A);
        else
            tmp &= 0x0f;
        out[i/2]+=tmp;
    }
}

/*--------------------------------------------
        One Hex Byte To Two Ascii Bytes
 ----------------------------------------------*/
void vOneTwo(uchar *in, ushort lc, uchar *out)
{
    uchar ucHexToChar[17];
    ushort nCounter;

    memcpy(ucHexToChar,"0123456789ABCDEF",16);
    for(nCounter = 0; nCounter < lc; nCounter++)
    {
        out[2*nCounter]   = ucHexToChar[(in[nCounter] >> 4)];
        out[2*nCounter+1] = ucHexToChar[(in[nCounter] & 0x0F)];
    }
    return;
}


void s_PciDispStr(uchar *inp_buf,uchar len, uchar ly,uchar lx)
{

    uchar y;
    uchar bOffset,tempfonttype;
    uchar disp_buf[130];

	   //Lib_LcdClrLine(2*8,31); //13/01/23
           
    tempfonttype=g_byFontType;

    memset(disp_buf,0,sizeof(disp_buf));
    y =ly;
    memcpy(disp_buf,inp_buf,len);
    disp_buf[len] =0;

    bOffset =0;
    if (len == 0)   return;

    if(tempfonttype)
    {
        if(lx>64){
            if(len>=((lx+1)/8))         bOffset = (lx+1)/8;
            else                        bOffset = len;
            Lib_LcdPrintxy(lx+1-bOffset*8,y,1,"%s",&disp_buf[len-bOffset]);  // 2008-01-18 修改显示错误huangjunbin
        }
        else{
            if(len>=((128-lx)/8))       bOffset = (128-lx)/8;
            else                        bOffset = len;
            Lib_LcdPrintxy(lx,y,1,"%s",&disp_buf[len-bOffset]);
        }
    }
    else
    {
        if(lx>64){
            if(len>=((lx+1)/6))         bOffset = (lx+1)/6;
            else                        bOffset = len;
            Lib_LcdPrintxy(lx+1-bOffset*6,y,0,"%s",&disp_buf[len-bOffset]);
        }
        else{
            if(len>=((128-lx)/6))       bOffset = (128-lx)/6;
            else                        bOffset = len;
            Lib_LcdPrintxy(lx,y,0,"%s",&disp_buf[len-bOffset]);  //
        }
    }
}

int s_PciInputPinAuto(uchar *out_pin)
{
    uchar  i,j;
    uchar  xxxx,yyyy;
    uchar  temp_pin[130],temp_len,rnd[16];
    j=0;
    xxxx=g_byStartX/X_MULTI;
    yyyy=g_byStartY/Y_MULTI;
	
	//随机生成4-12位密码，自动显示密码，停留2s，返回   
	
	Lib_PciGetRnd(rnd);
	temp_len = (rnd[i%16])%9+4;//长度为4到12之间随机生成
	
	for (j = 0 ;j < temp_len; j++)
	{
		temp_pin[j] = (rnd[j])%10 + '0';//密码为0到9之间随机生成
	}
    
    s_PciDispStr(temp_pin,temp_len,yyyy,xxxx);
	//Lib_LcdPrintxy(0,yyyy,0,temp_pin);
	Lib_DelayMs(2000);
	memcpy(out_pin,temp_pin,temp_len);
	memset(temp_pin,0,sizeof(temp_pin));
	memset(rnd,0,sizeof(rnd));
    return 0;
}

int s_PciInputPin(uchar min,uchar max,uchar *out_pin,ushort timeoutsec)
{

    uchar  j;
    char   key;
    uchar  xxxx,yyyy;
    uchar  temp_pin[130],temp_len,k_pin[130];
    ushort timeout=timeoutsec;

    j=0;
    xxxx=g_byStartX;
    yyyy=g_byStartY;
    if(timeout==0)        timeout=60;   //default 60 seconds;
	else if(timeout>=60) timeout=60;  //max time 1 minute.
    s_SetTimer(1,timeout*100);
    while (1)
    {
        if(!s_CheckTimer(1)) return PCI_InputTimeOut_Err;
        if(Lib_KbCheck()) continue;

        key=Lib_KbGetCh();
        s_SetTimer(1,timeout*100);

        if(key==KEYUP || key==KEYDOWN || key==KEYMENU ||
            key==KEYFN || key==KEYALPHA)
            continue;

        if(key==KEYCANCEL)     //如果受到POS的撤消要求时退出
        {
            Lib_Beep();
            return PCI_InputCancel_Err;
        }

        if(key==KEYCLEAR)
        {
            if (j==0)
            {
                Lib_Beep();
                continue;
            }
            memset(k_pin, 0, j);
            temp_len=j;
            memset(temp_pin,' ',j);
            s_PciDispStr(temp_pin,temp_len,yyyy,xxxx);
            j=0;
            continue;
        }
        if(key==KEYBACKSPACE)
        {
            if(j==0)
            {
                Lib_Beep();
                continue;
            }

            memset(temp_pin,0,sizeof(temp_pin));
            temp_len=j;
            //memset(temp_pin,' ',j);
            //memset(temp_pin+1,'*',j-1);
            //temp_pin[j-1]=' ';
            if(xxxx>64)
            {
                memset(temp_pin,' ',j);
                memset(temp_pin+1,'*',j-1);
            }
            else 
            {
                memset(temp_pin,'*',j-1);
                temp_pin[j-1]=' ';
            }
            s_PciDispStr(temp_pin,temp_len,yyyy,xxxx);
            k_pin[j-1]=0x00;
            j--;
            continue;
        }


        if(key==KEYENTER)
        {
            if(0 == gNoPinEnterDisenable)
            {
                if(j==0)
                {
                    k_pin[j]=0x00;
                    break;
                }
            }
            if(j<min) //输入金额时，不到最小长度双音提示
            {
                Lib_Beep();
                continue;
            }
            k_pin[j]=0x00;
            break;
        }
        if (j>=max) //pinpad到最大长度双音提示
        {
            Lib_Beep();
            continue;
        }

        if ((key>='0') && (key<='9'))
        {
            //Lib_Beep();
            k_pin[j]=key;
            j++;
        }
        else
        {
            Lib_Beep();
            continue;
        }


        // 
        //解决对于左对齐方式输入金额时数字错乱显示问题
        //
        memset(temp_pin,0,sizeof(temp_pin));
        temp_len=j;
        memset(temp_pin,'*',j);
        s_PciDispStr(temp_pin,temp_len,yyyy,xxxx);
    } //end while(1){
    memcpy(out_pin,k_pin,j);

    return 0;
}





/*==============================================================================
 *  写主密钥　
 *原型：int s_WriteDesKey(BYTE key_type,int key_n,BYTE key_len,BYTE *key_data,BYTE key_lrc);
 *参数：key_n:    主密钥编号（适用于多个主密钥情形）
 *      key_len:  主密钥长度（8,16,32）
 *      key_data: 主密钥内容
 *      key_lrc:  主密钥校验值
 *返回：
 *=============================================================================*/
int s_WriteAuthKey(uchar *app_name,uchar key_type,uchar key_no,uchar key_len,uchar *key_data,uchar key_lrc)
{
    unsigned char lrc[8],i,j; 
    unsigned char MMK[24];
    unsigned char ELRCK[24];
    unsigned char appname[33];
    int offset;
    int iret,ilen;

    AUTH_KEY  auth_key;
    //if(app_no>24) return PCI_AppNumOver_Err;
    ilen=strlen(app_name);
    if(ilen>=32) ilen=32;
    memset(appname,0,sizeof(appname));
    memcpy(appname,app_name,ilen);

    if(key_no>=50) return PCI_KeyNo_Err;
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;

    //检测写密钥权限与认证结果

    //先判断是否已锁键盘
    if(gLockStatus) return PCI_Locked_Err;
    
    
    iret=s_CheckAllKeysIsDiff(key_type,key_no,key_len,key_data);
    if(iret) return iret;
    
    
    //须对密钥内容进行加密处理； 
    memset(&auth_key,0,sizeof(AUTH_KEY));

    if(s_PciReadMMK(0, (DWORD*)MMK))    return PCI_ReadMMK_Err;
    if(s_PciReadMMK(1, (DWORD*)ELRCK))  return PCI_ReadMMK_Err;
    switch(key_type)
    {
    case AUTHPIN_MK_TYPE:
    case AUTHPINK_TYPE:
    case AUTHMACK_TYPE:
    case AUTHMAC_MK_TYPE:
    case AUTHOFFPINK_TYPE:
    case AUTHAPP_TYPE:
    case AUTHLCD_TYPE:
    case AUTHMDK_TYPE:
    case SK_MACK_TYPE:
        auth_key.KeyType=key_type;
        auth_key.KeyNo=key_no;
        auth_key.KeyLen=key_len;
        //auth_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            auth_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            memset(ELRCK,0,sizeof(ELRCK));
            //s_UartPrint(COM_DEBUG,"step3=%d\r\n",iret);
            //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
            //s_UartPrint(COM_DEBUG,"lrc=%02x klr=%02x \r\n",lrc,key_lrc);
            return PCI_KeyLrc_Err;
        }
        
        
        //s_UartPrint(COM_DEBUG,"step100=%d Keytype=%d\r\n",key_no,key_type);
        //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        
        
        DES_TDES(MMK,24,auth_key.KeyData,auth_key.KeyLen,1); 
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0; 
        for(i=0;i<4;i++)
        {
            for(j=0;j<8;j++)  lrc[j]=lrc[j]^appname[i*8+j];
        }  
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(auth_key.Lrc,lrc,5); 
        //s_UartPrint(COM_DEBUG,"MMK\r\n");
        //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
        //s_UartPrint(COM_DEBUG,"step101\r\n");
        //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        memcpy(auth_key.AppName,appname,32);
        iret=s_WriteAuthKeyFile(appname,key_type,key_no,(unsigned char *)&auth_key,sizeof(AUTH_KEY));
        //if(iret) return PCI_WriteKey_Err;
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        break;
    default:
        memset(MMK,0,sizeof(MMK));
        memset(ELRCK,0,sizeof(ELRCK));
        return PCI_KeyType_Err;
    }
    memset(MMK,0,sizeof(MMK));
    memset(ELRCK,0,sizeof(ELRCK));
    return 0;
}

/*==============================================================================
 *  读主密钥　
 *     int  s_ReadDesKey(BYTE key_type,int key_n,BYTE *key_len,BYTE *key_data);
 *=============================================================================*/
int s_ReadAuthKey(uchar *app_name,uchar key_type,uchar key_no,uchar *key_len,uchar *key_data)
{
     unsigned char edc,keylen,lrc[8];
     unsigned char MMK[24];
     unsigned char ELRCK[16];
     unsigned char appname[33];
     AUTH_KEY auth_key;
     int iret,i,j,ilen;
     int offset;

     //须对密钥内容进行加密处理；
     //if(app_no>24) return PCI_AppNumOver_Err;
     ilen=strlen(app_name);
     if(ilen>=32) ilen=32;
     memset(appname,0,sizeof(appname));
     memcpy(appname,app_name,ilen);
     //先判断是否已锁键盘
     if(gLockStatus) return PCI_Locked_Err; 
     if(s_PciReadMMK(0, (DWORD*)MMK))   return PCI_ReadMMK_Err;
     if(s_PciReadMMK(1, (DWORD*)ELRCK)) return PCI_ReadMMK_Err;
     switch(key_type)
     {
     case AUTHPIN_MK_TYPE:
     case AUTHPINK_TYPE:
     case AUTHMACK_TYPE:
     case AUTHMAC_MK_TYPE:
     case AUTHOFFPINK_TYPE:
     case AUTHAPP_TYPE:
     case AUTHLCD_TYPE:
     case AUTHMDK_TYPE:
     case SK_MACK_TYPE:
          iret=s_ReadAuthKeyFile(appname,key_type,key_no,(unsigned char *)&auth_key,sizeof(AUTH_KEY));
          break;
     default:
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_KeyType_Err;
     }

     if(iret<0)
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     if(auth_key.KeyType!=key_type )
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     edc=0;
     keylen=auth_key.KeyLen;
     if((keylen!=8)&&(keylen!=16)&&(keylen!=24))
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     //s_UartPrint(COM_DEBUG,"step300=%d Keytype=%d\r\n",key_no,key_type);
     //for(i=0;i<keylen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);
     DES_TDES(MMK,24,auth_key.KeyData,auth_key.KeyLen,0x81);
     //s_UartPrint(COM_DEBUG,"MMK\r\n");
     //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
     //s_UartPrint(COM_DEBUG,"step301\r\n");
     //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);
    
    
    for(i=0;i<keylen;i++)
          edc^=auth_key.KeyData[i]; 
    memset(lrc,0,sizeof(lrc));  
    lrc[0]=edc;
    lrc[1]=auth_key.KeyType;
    lrc[2]=auth_key.KeyNo;
    lrc[3]=auth_key.KeyLen; 
    for(i=0;i<4;i++)
    {
        for(j=0;j<8;j++)  lrc[j]=lrc[j]^appname[i*8+j];
    }  
    //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
    Lib_DES3_16(lrc,ELRCK,1); 
    if(memcmp(lrc,auth_key.Lrc,5)) 
    //if(edc!=auth_key.Lrc)
    {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          //s_UartPrint(COM_DEBUG,"step4=%02x %02x\r\n",edc,des_key.Lrc);
          return PCI_KeyLrc_Err;
    }
    *key_len=keylen;
    memcpy(key_data,auth_key.KeyData,keylen);
    memset(MMK,0,sizeof(MMK));
    memset(ELRCK,0,sizeof(ELRCK));
    return 0;
}




int s_WriteMdkKey(uchar *app_name,uchar key_type,uchar key_no,uchar key_len,uchar *key_data,uchar key_lrc)
{
    unsigned char lrc[8],i,j; 
    unsigned char MMK[24];
    unsigned char ELRCK[24];
    unsigned char appname[33];
    int offset;
    int iret,ilen;

    AUTH_KEY  auth_key;
    //if(app_no>24) return PCI_AppNumOver_Err;
    ilen=strlen(app_name);
    if(ilen>=32) ilen=32;
    memset(appname,0,sizeof(appname));
    memcpy(appname,app_name,ilen);

    if(key_no>=50) return PCI_KeyNo_Err;
    if(key_len!=8 && key_len!=16 && key_len!=24) return PCI_KeyLen_Err;

    //检测写密钥权限与认证结果 
    //先判断是否已锁键盘
    if(gLockStatus) return PCI_Locked_Err; 
    iret=s_CheckAllKeysIsDiff(key_type,key_no,key_len,key_data);
    if(iret) return iret;  
    //须对密钥内容进行加密处理； 
    memset(&auth_key,0,sizeof(AUTH_KEY)); 
    if(s_PciReadMMK(0, (DWORD*)MMK))    return PCI_ReadMMK_Err;
    if(s_PciReadMMK(1, (DWORD*)ELRCK))  return PCI_ReadMMK_Err;
    switch(key_type)
    { 
    case AUTHMDK_TYPE:
        auth_key.KeyType=key_type;
        auth_key.KeyNo=key_no;
        auth_key.KeyLen=key_len;
        //auth_key.Lrc=key_lrc;
        lrc[0]=0;
        for(i=0;i<key_len;i++)
        {
            auth_key.KeyData[i]=key_data[i];
            lrc[0]=lrc[0]^key_data[i];
        }
        if(lrc[0]!=key_lrc)
        {
            memset(MMK,0,sizeof(MMK));
            memset(ELRCK,0,sizeof(ELRCK));
            //s_UartPrint(COM_DEBUG,"step3=%d\r\n",iret);
            //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
            //s_UartPrint(COM_DEBUG,"lrc=%02x klr=%02x \r\n",lrc,key_lrc);
            return PCI_KeyLrc_Err;
        }
        
        
        //s_UartPrint(COM_DEBUG,"step100=%d Keytype=%d\r\n",key_no,key_type);
        //for(i=0;i<key_len;i++) s_UartPrint(COM_DEBUG,"%02x ",key_data[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        
        
        DES_TDES(MMK,24,auth_key.KeyData,auth_key.KeyLen,1); 
        lrc[1]=key_type;
        lrc[2]=key_no;
        lrc[3]=key_len;
        lrc[4]=0;
        lrc[5]=0;
        lrc[6]=0;
        lrc[7]=0; 
        for(i=0;i<4;i++)
        {
            for(j=0;j<8;j++)  lrc[j]=lrc[j]^appname[i*8+j];
        }  
        //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
        Lib_DES3_16(lrc,ELRCK,1);
        memcpy(auth_key.Lrc,lrc,5); 
        //s_UartPrint(COM_DEBUG,"MMK\r\n");
        //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
        //s_UartPrint(COM_DEBUG,"step101\r\n");
        //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
        //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",key_lrc);
        memcpy(auth_key.AppName,appname,32);
        iret=s_WriteMdkFile(appname,key_type,key_no,(unsigned char *)&auth_key,sizeof(AUTH_KEY));
        //if(iret) return PCI_WriteKey_Err;
        //13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
        break;
    default:
        memset(MMK,0,sizeof(MMK));
        memset(ELRCK,0,sizeof(ELRCK));
        return PCI_KeyType_Err;
    }
    memset(MMK,0,sizeof(MMK));
    memset(ELRCK,0,sizeof(ELRCK));
    return 0;
}

/*==============================================================================
 *  读主密钥　
 *     int  s_ReadDesKey(BYTE key_type,int key_n,BYTE *key_len,BYTE *key_data);
 *=============================================================================*/
int s_ReadMdkKey(uchar *app_name,uchar key_type,uchar key_no,uchar *key_len,uchar *key_data)
{
     unsigned char edc,keylen,lrc[8];
     unsigned char MMK[24];
     unsigned char ELRCK[16];
     unsigned char appname[33];
     AUTH_KEY auth_key;
     int iret,i,j,ilen;
     int offset;

     //须对密钥内容进行加密处理；
     //if(app_no>24) return PCI_AppNumOver_Err;
     ilen=strlen(app_name);
     if(ilen>=32) ilen=32;
     memset(appname,0,sizeof(appname));
     memcpy(appname,app_name,ilen);
     //先判断是否已锁键盘
     if(gLockStatus) return PCI_Locked_Err; 
     if(s_PciReadMMK(0, (DWORD*)MMK))   return PCI_ReadMMK_Err;
     if(s_PciReadMMK(1, (DWORD*)ELRCK)) return PCI_ReadMMK_Err;
     switch(key_type)
     {
 
     case AUTHMDK_TYPE:
          iret=s_ReadMdkFile(appname,key_type,key_no,(unsigned char *)&auth_key,sizeof(AUTH_KEY));
          break;
     default:
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_KeyType_Err;
     }

     if(iret<0)
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     if(auth_key.KeyType!=key_type )
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     edc=0;
     keylen=auth_key.KeyLen;
     if((keylen!=8)&&(keylen!=16)&&(keylen!=24))
     {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          return PCI_NoKey_Err;
     }
     //s_UartPrint(COM_DEBUG,"step300=%d Keytype=%d\r\n",key_no,key_type);
     //for(i=0;i<keylen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);
     DES_TDES(MMK,24,auth_key.KeyData,auth_key.KeyLen,0x81);
     //s_UartPrint(COM_DEBUG,"MMK\r\n");
     //for(i=0;i<16;i++) s_UartPrint(COM_DEBUG,"%02x ",MMK[i]);
     //s_UartPrint(COM_DEBUG,"step301\r\n");
     //for(i=0;i<des_key.KeyLen;i++) s_UartPrint(COM_DEBUG,"%02x ",des_key.KeyData[i]);
     //s_UartPrint(COM_DEBUG,"lrc=%02x\r\n",des_key.Lrc);
    
    
    for(i=0;i<keylen;i++)
          edc^=auth_key.KeyData[i]; 
    memset(lrc,0,sizeof(lrc));  
    lrc[0]=edc;
    lrc[1]=auth_key.KeyType;
    lrc[2]=auth_key.KeyNo;
    lrc[3]=auth_key.KeyLen; 
    for(i=0;i<4;i++)
    {
        for(j=0;j<8;j++)  lrc[j]=lrc[j]^appname[i*8+j];
    }  
    //Lib_DES(lrc,(uchar *)"\x11\x22\x33\x44\x55\x66\x77\x88",1);
    Lib_DES3_16(lrc,ELRCK,1); 
    if(memcmp(lrc,auth_key.Lrc,5)) 
    //if(edc!=auth_key.Lrc)
    {
          *key_len=0;
          memset(MMK,0,sizeof(MMK));
          memset(ELRCK,0,sizeof(ELRCK));
          //s_UartPrint(COM_DEBUG,"step4=%02x %02x\r\n",edc,des_key.Lrc);
          return PCI_KeyLrc_Err;
    }
    *key_len=keylen;
    memcpy(key_data,auth_key.KeyData,keylen);
    memset(MMK,0,sizeof(MMK));
    memset(ELRCK,0,sizeof(ELRCK));
    return 0;
}



int s_AuthGetRand(uchar *rnd8)
{
    uchar rand[8];
    Lib_GetRand(rand ,8);
    memcpy(Auth_Result.Rand,rand,8);
    memcpy(rnd8,rand,8);
    return 0;
}

int s_AuthKeyCheck(uchar *app_name,uchar key_type,uchar key_no,uchar *authdata)
{
    int iret;
    uchar keylen,authflag;
    uchar keydata[24];
    uchar rnddata[8];
     
    //int i;

    if(g_byCurAppNum<=24)
    {
        gPciAuthTimes[g_byCurAppNum]++;
        if(gPciAuthTimes[g_byCurAppNum]>PCI_AUTHTIMES_MAX)
        {
                //printf("PCI_AuthTimes_Err(%d %d) \r\n",g_byCurAppNum,gPciAuthTimes[g_byCurAppNum]);
                return PCI_AuthTimes_Err;
        } 
    }  
        
    iret=s_ReadAuthKey(app_name,key_type,key_no,&keylen,keydata);
    if(iret)
    {
        Auth_Result.AuthFlag=0;
        //printf("authkeycheck1(%s %02x %02x)=%d\r\n",app_name,key_type,key_no,iret);
        return PCI_NoKey_Err;
    }


    memcpy(rnddata,Auth_Result.Rand,8);
    
    
    DES_TDES(keydata,keylen,rnddata,8,0x01);
    if(memcmp(rnddata,authdata,8))
    {
        Auth_Result.AuthFlag=0;
        //printf("authkeycheck2(%s %02x %02x)=%d\r\n",app_name,key_type,key_no,iret);
       
        //printf("len=%d \r\n",keylen);
        //for(i=0;i<8;i++) printf("r[%d]=%02x\r\n",i,Auth_Result.Rand[i]);
        //for(i=0;i<8;i++) printf("v[%d]=%02x\r\n",i,rnddata[i]);
        //for(i=0;i<8;i++) printf("a[%d]=%02x\r\n",i,authdata[i]);
        //for(i=0;i<keylen;i++) printf("k[%d]=%02x\r\n",i,keydata[i]); 
        return PCI_Auth_Err;
    }
    
    if(g_byCurAppNum<=24)
    {
        gPciAuthTimes[g_byCurAppNum]=0;
    }
    

    authflag=Auth_Result.AuthFlag;
    if(authflag==0x00)
    {
        memcpy(Auth_Result.AppName,app_name,32);
        Auth_Result.KeyType1=key_type;
        Auth_Result.KeyNo1=key_no;
        Auth_Result.AuthFlag=0x01;
    }
    else if(authflag==0x01)
    {
        Auth_Result.KeyType2=key_type;
        Auth_Result.KeyNo2=key_no;
        Auth_Result.AuthFlag=0x03;
    }
    else if(authflag==0x03)
    {
        Auth_Result.KeyType3=key_type;
        Auth_Result.KeyNo3=key_no;
        Auth_Result.AuthFlag=0x07;
    }
    else
    {
        Auth_Result.AuthFlag=0x00;
    }
    
    //printf("authkeycheck-ok(%02x %02x %s %02x %02x)\r\n",authflag,Auth_Result.AuthFlag,app_name,key_type,key_no);
    
    return 0;
}

int s_AuthKeyVerify(uchar key_type,uchar key_no,uchar *authdata)
{ 
    int iret=0; 
    return iret;
}


int   s_PciClearAuthFlag(void)
{
    Auth_Result.AuthFlag=0;
    return 0;
}


int s_AuthLcd(void)
{
    int iret=0; 
    //printf("s_AuthLcd1(%d)\r\n",iret); 
    return iret;
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/*===================================================
             脱机PIN较验接口（含明文与密文方式）
 ===================================================*/
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

void PCI_asc_to_bcd(uchar *bcd, uchar *asc, uchar asc_len);
uchar PCI_char_to_bin(uchar bchar);

void PCI_asc_to_bcd(uchar *bcd, uchar *asc, uchar asc_len)
{
    uchar i,j;
    uchar bOddFlag, bchar, bchar1, bchar2;
    if (asc_len%2)  bOddFlag = 1;
    else            bOddFlag = 0;

    for (i=0,j=0; j<asc_len; i++)
    {
        bchar1 = asc[j++];
        bchar = PCI_char_to_bin(bchar1);
        bchar *= 16;
        if(j==asc_len)
        {
            bcd[i] = bchar+0x0F;
            break;
        }
        bchar1 = asc[j++];
        bchar2 = PCI_char_to_bin(bchar1);
        bcd[i] = bchar + bchar2;
    }
}

uchar PCI_char_to_bin(uchar bchar)
{
    if ((bchar >= '0')&&(bchar <= '9'))
        return(bchar-'0');
    else
    {
        if ((bchar >= 'A')&&(bchar <= 'F'))
            return(bchar-'A'+10);
        if ((bchar >= 'a')&&(bchar <= 'f'))
            return(bchar-'a'+10);
        else
            return(0);
    }
}


/*============================================================
             脱机PIN较验接口（密文方式）
进行脱机密文PIN校验功能。
应用通过卡片读写命令从卡片中获得PIN加密非对称结构公钥，并从卡片中
获取相应的随机数，结合应用终端随机数对输入的PIN进行RSA加密运算，
并返回加密的PIN　BLOCK。
typedef struch
{
     Unsigned int modlen;           //PIN加密公钥模数长
     Unsigned char mod[256];        // PIN加密公钥模数
     Unsigned int explen;            // PIN加密公钥指数长
     Unsigned char exp[4];           // PIN加密公钥指数
     Unsigned char iccrandomlen;     //从卡行取得的随机数长
     Unsigned char iccrandom[8];     //从卡行取得的随机数
     Unsigned int  termrandomlen;    //从终端应用取得的填充数长
     Unsigned char termrandom[256];   //从终端应用取得的填充数
} RSA_PINKEY;
PIN加密公钥 　　　从卡片读取（通过读应用数据过程获得）。
卡片随机数  　　　通过读随机数命令从卡片获得。
终端应用随机数  由终端应用程序产生NIC－17字节的填充数据。
min 　　　　　　PIN最小长度
max 　　　　　　PIN最大长度（＜＝14）
encpin  　　　加密后的PIN　BLOCK数据，长度与公钥模长相同。

明文脱机PIN block按如下格式组织
CN  PP  PP  P/F P/F P/F P/F FF
其中
C   控制域  值为0010的四位二进制数（hex. 2）
N   PIN长度 值在0010到1100之间的4位二进制数（hex. '4'到 'C'）
P   PIN数字 值在0000到1001之间的4位二进制数（hex. '0'到 '9'）
P/F PIN/填充位  由PIN长度决定
F   填充位  值为1111的四位二进制数（hex. 'F'）

使用PIN加密公钥，终端将RSA恢复函数应用到下表指明的数据上，以获得加密PIN数据。

数据头  　　　　　　　　　1 　　　十六进制，值为'7F'    　　　
PIN块   　　　　　　　　　8 　　　以PIN块方式存在的PIN　block
IC卡不可预知的随机数　　8   　　　通过GET CHALLENGE命令从IC卡获得
　　　　　　　　　　　　　　　　　　　的不可预知的随机数
随机填充字节    　　　　　　NIC-17  由终端生成的随机填充字节

返回值：
 ============================================================*/

int Lib_PciOffLineEncPin(RSA_PINKEY *rsa_pinkey,uchar min,uchar max,uchar *encpin,ushort waittime_sec)
{

    uint  i,modlen,explen;
    uchar ret,iccrandomlen;
    uint  termrandomlen;
    uchar mod[256],exp[4],iccrandom[8],termrandom[256],strict_buf[8];
    uchar temp[17],PINDATA[300];
    uchar pin_len;
    uchar pinbuf[24];
    int   iret;
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];

    if (min>max || max>14)
    {
        return PCI_InputLen_Err;
    }

    modlen=rsa_pinkey->modlen;
    explen=rsa_pinkey->explen;
    iccrandomlen=rsa_pinkey->iccrandomlen;
    termrandomlen=rsa_pinkey->termrandomlen;
    if(modlen>256)
	{
        return PCI_RsaKey_Err;
    }
    memset(mod,0,256);
    memcpy(mod,rsa_pinkey->mod,modlen);
    if(explen>4)
	{
        return PCI_RsaKey_Err;
    }
    memset(exp,0,4);
    memcpy(exp,rsa_pinkey->exp,explen);
    if(iccrandomlen>8)
	{
        return PCI_RsaKey_Err;
    }
    memset(iccrandom,0,8);
    memcpy(iccrandom,rsa_pinkey->iccrandom,iccrandomlen);
    if(termrandomlen>256)
	{
        return PCI_RsaKey_Err;
    }

//////////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
	 //get current time in seconds
	 //Lib_GetCurSecond(&timebefore);
	 Lib_GetDateTime(cur_time);
	 timebefore=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
     if(strict_buf[0] == 0xff && strict_buf[1] == 0xff && strict_buf[2] == 0xff)//first time
     {
		 memset(strict_buf,0x00,sizeof(strict_buf));
		 ByteToDword(strict_buf,8,dwCount);
		 s_PciWritePinCount(0,dwCount[0]);
		 s_PciWritePinCount(1,dwCount[1]);
     }

     //read the count
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
	 count = strict_buf[0];
     if(count == 120)//the max count in max time
     {
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
         timetemp = (strict_buf[4]*256+strict_buf[5])*65535 + strict_buf[6]*256 + strict_buf[7];
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
		 //Lib_GetCurSecond(&dwCount[0]);
		 Lib_GetDateTime(cur_time);
		 dwCount[0]=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
         interval = dwCount[0]-timetemp+strict_buf[2]*256+strict_buf[1];//read the interval time
         if(interval < 3600)//the max time value
         {
             //Lib_GetCurSecond(&timemiddle);
			 Lib_GetDateTime(cur_time);
			 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
			 s_PciWritePinCount(1,timemiddle);
             return PCI_CallTimeInte_Err;
         }
         else
         {
             count = 0;
             interval = 0;
			 memset(strict_buf,0,sizeof(strict_buf));
			 ByteToDword(strict_buf,4,&dwCount[0]);
			 s_PciWritePinCount(0,dwCount[0]);
         }
     }
#endif
////////////////////////////////////////////////


    memset(termrandom,0,256);
    memcpy(termrandom,rsa_pinkey->termrandom,termrandomlen);

    memset(pinbuf,0,sizeof(pinbuf));
    iret=s_PciInputPin(min, max, pinbuf,waittime_sec);
    if(iret)
    { 
        return iret;
    }

    
    pin_len =strlen((char *)pinbuf);
    if(pin_len ==0)
    {
        memset(pinbuf,0,sizeof(pinbuf));
        //printf("step11 \r\n");
        return PCI_NoInput_Err;
    }
///////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
     //Lib_GetCurSecond(&timemiddle);
	 Lib_GetDateTime(cur_time);
	 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciWritePinCount(1,timemiddle);
     count++;
     //save the count
     //Lib_GetCurSecond(&timeafter);//get current time
	 Lib_GetDateTime(cur_time);
	 timeafter=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
     interval = timeafter - timebefore;
	 s_PciReadPinCount(0,&dwCount[0]);
	 DwordToBytes(&dwCount[0],1,strict_buf);
     interval = interval + strict_buf[2]*256 + strict_buf[1];//accumulate the interval time
	 strict_buf[0] = count;
	 strict_buf[1] = interval%256;
	 strict_buf[2] = interval/256;
	 ByteToDword(strict_buf,4,&dwCount[0]);
	 s_PciWritePinCount(0,dwCount[0]);
#endif
///////////////////////////////////////////////
    temp[0]=0x20+pin_len;
    for(i=1;i<8;i++) temp[i]=0xFF;
    PCI_asc_to_bcd(&temp[1],pinbuf,pin_len);

    memset(PINDATA,0,300);
    PINDATA[0]=0x7F;
    memcpy(&PINDATA[1],temp,8);
    memcpy(&PINDATA[9],iccrandom,8);
    memcpy(&PINDATA[17],termrandom,termrandomlen);
    
    
    Lib_Rsa(mod,modlen,exp,explen,PINDATA,encpin);
    
#ifdef UARTPRINF_DEBUG    
      //test
    Lib_LcdPrintxy(0,8*1,0x00,"Lib_Rsa end");
   Lib_KbGetCh();
#endif    
    memset(mod,0,256);
    memset(exp,0,4);
    memset(pinbuf,0,sizeof(pinbuf));
    return 0;
}



/*===========================================================
             脱机PIN较验接口（明文方式）

进行脱机密文PIN校验功能。在PED内部生成PIN，并按照应用提供的卡片命令格式与
卡片通道号，将明文PINBLOCK直接发送给卡片。
icc_slot         校验卡片所在的卡座号。
min           PIN最小长度
max           PIN最大长度（＜＝14）
icc_command 发给卡片的校验命令头：（5字节）
               CLA＋INS＋P1＋P2＋LC。
icc_resp         卡片响应的结果。（2字节）
               SW1＋SW2
0x00        成功


明文脱机PIN block按如下格式组织
CN  PP  PP  P/F P/F P/F P/F FF
其中
C   控制域  值为0010的四位二进制数（hex. 2）
N   PIN长度 值在0010到1100之间的4位二进制数（hex. '4'到 'C'）
P   PIN数字 值在0000到1001之间的4位二进制数（hex. '0'到 '9'）
P/F PIN/填充位  由PIN长度决定
F   填充位  值为1111的四位二进制数（hex. 'F'）
 ==============================================================*/

int Lib_PciOffLinePlainPin(uchar icc_slot,uchar min,uchar max,uchar *icc_command,uchar *icc_resp,ushort waittime_sec)
{

    uchar i,temp[17],ret;
    APDU_SEND  ApduSend;
    APDU_RESP  ApduResp;
    uchar pin_len;
    uchar pinbuf[24],strict_buf[8];
    int   iret;
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];

    if (min>max || max>14)
    {
        return PCI_InputLen_Err;;        // illegal length.
    } 

//////////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
	 //get current time in seconds
	 //Lib_GetCurSecond(&timebefore);
	 Lib_GetDateTime(cur_time);
	 timebefore=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
     if(strict_buf[0] == 0xff && strict_buf[1] == 0xff && strict_buf[2] == 0xff)//first time
     {
		 memset(strict_buf,0x00,sizeof(strict_buf));
		 ByteToDword(strict_buf,8,dwCount);
		 s_PciWritePinCount(0,dwCount[0]);
		 s_PciWritePinCount(1,dwCount[1]);
     }
     //read the count
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
	 count = strict_buf[0];
     if(count == 120)//the max count in max time
     {
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
         timetemp = (strict_buf[4]*256+strict_buf[5])*65535 + strict_buf[6]*256 + strict_buf[7];
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
		 //Lib_GetCurSecond(&dwCount[0]);
		 Lib_GetDateTime(cur_time);
		 dwCount[0]=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
         interval = dwCount[0]-timetemp+strict_buf[2]*256+strict_buf[1];//read the interval time
         if(interval < 3600)//the max time value
         {
             //Lib_GetCurSecond(&timemiddle);
			 Lib_GetDateTime(cur_time);
			 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
			 s_PciWritePinCount(1,timemiddle);
             return PCI_CallTimeInte_Err;
         }
         else
         {
             count = 0;
             interval = 0;
			 memset(strict_buf,0,sizeof(strict_buf));
			 ByteToDword(strict_buf,4,&dwCount[0]);
			 s_PciWritePinCount(0,dwCount[0]);
         }
     }
#endif
//////////////////////////////////////////////////


    memset(pinbuf,0,sizeof(pinbuf));
    iret=s_PciInputPin(min, max, pinbuf,waittime_sec);
    if(iret)
    {
        return iret;
    } 
    pin_len =strlen((char *)pinbuf);
    if(pin_len ==0)
    {
        memset(pinbuf,0,sizeof(pinbuf));
        //printf("step6 \r\n");
        return PCI_NoInput_Err;
    }
///////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
     //Lib_GetCurSecond(&timemiddle);
	 Lib_GetDateTime(cur_time);
	 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciWritePinCount(1,timemiddle);
     count++;
     //save the count
     //Lib_GetCurSecond(&timeafter);//get current time
	 Lib_GetDateTime(cur_time);
	 timeafter=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
     interval = timeafter - timebefore;
	 s_PciReadPinCount(0,&dwCount[0]);
	 DwordToBytes(&dwCount[0],1,strict_buf);
     interval = interval + strict_buf[2]*256 + strict_buf[1];//accumulate the interval time
	 strict_buf[0] = count;
	 strict_buf[1] = interval%256;
	 strict_buf[2] = interval/256;
	 ByteToDword(strict_buf,4,&dwCount[0]);
	 s_PciWritePinCount(0,dwCount[0]);
#endif
///////////////////////////////////////////////
    temp[0]=0x20+pin_len;
    for(i=1;i<8;i++) temp[i]=0xFF;
    PCI_asc_to_bcd(&temp[1],pinbuf,pin_len);
    memcpy(ApduSend.Command,icc_command,4);
    ApduSend.Lc=8;
    memcpy(ApduSend.DataIn,temp,8);
    ApduSend.Le=0;
    iret=Lib_IccCommand(icc_slot,&ApduSend,&ApduResp);
    if(iret)
    {
        memset(pinbuf,0,sizeof(pinbuf));
        //printf("step7 \r\n");
        return iret;
    }
    icc_resp[0]=ApduResp.SWA;
    icc_resp[1]=ApduResp.SWB;
    memset(pinbuf,0,sizeof(pinbuf));
    return 0;
}



int Lib_PciAuthForNumberKey(uchar *authdata)
{  
    int iret; 
    //printf("pciauthfornumkey=(%d)\r\n",iret);
	iret=0; 
    g_iAuthForNumberKeyFlag=0; 
    if(iret==0)
    {   
        g_iAuthForNumberKeyFlag=1;
        s_PciClearAuthFlag();
    } 
    return iret; 
    
    
} 








int s_WriteMDK(uchar *mdk)
{
    int   iret;
    uchar i,lrc,appname[33];
    lrc=0;
    for(i=0;i<8;i++) lrc=lrc^mdk[i]; 
    memset(appname,0,sizeof(appname));
    memcpy(appname,"VOS",3);
    iret=s_WriteMdkKey(appname,AUTHMDK_TYPE,0,16,mdk,lrc);
    return iret; 
} 

int s_ReadMDK(uchar *mdk)
{
    int   iret;
    uchar appname[33],len;
    memset(appname,0,sizeof(appname));
    memcpy(appname,"VOS",3);
    iret=s_ReadMdkKey(appname,AUTHMDK_TYPE,0,&len,mdk);
    return iret;
}

int s_CheckAllKeysIsDiff(uchar keytype,uchar keyno,uchar keylen,uchar *keydata)
{
    int fd,i,j,k,iret,ilen,offset,nVal;
    BYTE  buff[24],abyKey[24];  
    DWORD adwMMK[4];  
    unsigned char MMK[24],outdata[100];
    unsigned char enckey[24];    
    AUTH_KEY  auth_key;
    DES_KEY   des_key; 

	#ifndef __CHECK_SAME_KEY__
		return 0;
	#endif
	
    if(keylen!=8 && keylen!=16 && keylen!=24) return 0; 
    memset(enckey,0,sizeof(enckey));
    memcpy(enckey,keydata,keylen);
    if(keylen==16)
    { 
        //i = s_PciReadMMK(2, adwMMK); 
//        i = s_ReadBSK(abyKey);
//        if (0 == i)
//        { 
//            //DwordToBytes(adwMMK, 4, abyKey);
//            if(memcmp(abyKey,keydata,16)==0)
//            {
//                //printf("checkdiff(0)\r\n");
//                return PCI_KeySame_Err;
//            } 
//        }  
        
        i = s_ReadVSK(abyKey);
        if (0 == i)
        { 
            //DwordToBytes(adwMMK, 4, abyKey);
            if(memcmp(abyKey,keydata,16)==0)
            {
                //printf("checkdiff(0)\r\n");
                return PCI_KeySame_Err;
            } 
        }   
    }  
    if(s_PciReadMMK(0, (DWORD*)MMK))    return 0; 
    DES_TDES(MMK,24,enckey,keylen,1);   

    fd=my_open(AUTHKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return 0;
    while(1)
    {
        i=my_FileRead(fd,outdata,sizeof(AUTH_KEY));
        nVal = sizeof(AUTH_KEY);
        
        //if(i<sizeof(AUTH_KEY))
        if(i<nVal)
        {
            //Lib_FileClose(fd);
            //return 0;
            break;
        }
        memcpy((uchar *)&auth_key,outdata,sizeof(AUTH_KEY)); 
        if((auth_key.KeyLen==keylen)&&(memcmp(auth_key.KeyData,enckey,keylen)==0))
        {   
            if((auth_key.KeyType==keytype)&&(auth_key.KeyNo==keyno))
            {
                continue;
            } 
            my_FileClose(fd);
            return PCI_KeySame_Err;
        }
    }
    my_FileClose(fd);
    
    iret=SPF_GetMaxAppNo();
    if(iret<=0) return 0;
    k=iret;
    //printf("checkdiff(1)=%d\r\n",k);
    //for(i=0;i<PCI_MAX_APPNUM;i++)
    for(i=0;i<k;i++)
    {
        for(j=0;j<PCI_MAX_KEYNUM;j++)
        {
            offset=(int)i*PCI_APPKEY_SPACE+(int)j*sizeof(DES_KEY)+PCI_PIN_MKEY_ADDR_PAGE0;
            iret=s_ReadSysKeyFile(offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY)); 
            if(iret<0)
            { 
                return 0;
            }
            if(des_key.KeyLen==keylen)
            {
                if(memcmp(des_key.KeyData,enckey,keylen)==0)
                {
                    //printf("checkdiff(2)=%d %d\r\n",i,j);
                    
                    if((des_key.KeyType!=keytype)||(des_key.KeyNo!=keyno))  
                        return PCI_KeySame_Err;
                }
                
            }
   
            offset=(int)i*PCI_APPKEY_SPACE+(int)j*sizeof(DES_KEY)+PCI_PINKEY_ADDR_PAGE2;
            iret=s_ReadSysKeyFile(offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));
            if(iret<0)
            { 
                return 0;
            }
            if(des_key.KeyLen==keylen)
            {
                if(memcmp(des_key.KeyData,enckey,keylen)==0)
                {
                    //printf("checkdiff(3)=%d %d\r\n",i,j);
                    if((des_key.KeyType!=keytype)||(des_key.KeyNo!=keyno))  
                        return PCI_KeySame_Err;
                }
            }
     
            offset=(int)i*PCI_APPKEY_SPACE+(int)j*sizeof(DES_KEY)+PCI_MACKEY_ADDR;
            iret=s_ReadSysKeyFile(offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));

            if(iret<0)
            { 
                return 0;
            }
            if(des_key.KeyLen==keylen)
            {
                if(memcmp(des_key.KeyData,enckey,keylen)==0)
                {
                    //printf("checkdiff(4)=%d %d\r\n",i,j);
                    if((des_key.KeyType!=keytype)||(des_key.KeyNo!=keyno))  
                        return PCI_KeySame_Err;
                }
            }
    
            offset=(int)i*PCI_APPKEY_SPACE+(int)j*sizeof(DES_KEY)+PCI_MAC_MKEY_ADDR;
            iret=s_ReadSysKeyFile(offset,FILE_SEEK_SET,(unsigned char *)&des_key,sizeof(DES_KEY));

            if(iret<0)
            { 
                return 0;
            }
            if(des_key.KeyLen==keylen)
            {
                if(memcmp(des_key.KeyData,enckey,keylen)==0)
                {
                    //printf("checkdiff(5)=%d %d\r\n",i,j);
                    if((des_key.KeyType!=keytype)||(des_key.KeyNo!=keyno))  
                        return PCI_KeySame_Err;
                }
            }
        } 
    }  
    return 0;
}



int  Lib_PciGetPinDukpt(uchar key_n,uchar min_len,uchar max_len,uchar *card_no,uchar mode,uchar *pin_block,ushort waittime_sec,uchar *out_ksn)
{ 
    uchar i, def_max_len,ret;
    uchar temp[24],one[9],two[9],three[9];
    uchar pin_len,len;
    uchar pinkeybuf[24], pinkeylen,pinbuf[24],strict_buf[8];
    int   iret;  
	uchar tempksn[10]; 	  
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];
    //printf("PCI_GetPin(%02x %02x %02x %02x %d)\r\n",pinkey_n,min_len,max_len,mode,waittime_sec);

 

    //if(k_LockStatus) return PCI_Locked_Err;
    memset(pin_block,0,8);

    def_max_len =14;
    if (min_len>max_len || max_len>def_max_len)
    {
        return PCI_InputLen_Err;
    }
    //if((pinkey_n & 0x7f)>=50)
    if(key_n >= MAX_DUKPT_NUM)
    {
        return PCI_KeyNo_Err;
    } 
//////////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
	 //get current time in seconds
	 //Lib_GetCurSecond(&timebefore);
	 Lib_GetDateTime(cur_time);
	 timebefore=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
     if(strict_buf[0] == 0xff && strict_buf[1] == 0xff && strict_buf[2] == 0xff)//first time
     {
		 memset(strict_buf,0x00,sizeof(strict_buf));
		 ByteToDword(strict_buf,8,dwCount);
		 s_PciWritePinCount(0,dwCount[0]);
		 s_PciWritePinCount(1,dwCount[1]);
     }

     //read the count
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
	 count = strict_buf[0];
     if(count == 120)//the max count in max time
     {
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
         timetemp = (strict_buf[4]*256+strict_buf[5])*65535 + strict_buf[6]*256 + strict_buf[7];
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
		 //Lib_GetCurSecond(&dwCount[0]);
		 Lib_GetDateTime(cur_time);
		 dwCount[0]=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
         interval = dwCount[0]-timetemp+strict_buf[2]*256+strict_buf[1];//read the interval time
         if(interval < 3600)//the max time value
         {
             //Lib_GetCurSecond(&timemiddle);
			 Lib_GetDateTime(cur_time);
			 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
			 s_PciWritePinCount(1,timemiddle);
             return PCI_CallTimeInte_Err;
         }
         else
         {
             count = 0;
             interval = 0;
			 memset(strict_buf,0,sizeof(strict_buf));
			 ByteToDword(strict_buf,4,&dwCount[0]);
			 s_PciWritePinCount(0,dwCount[0]);
         }
     }
#endif
//////////////////////////////////////////////////
	 //iret=s_ReadDesKey(g_byCurAppNum,PINKEY_TYPE,pinkey_n,&pinkeylen,pinkeybuf);
    //if(iret) return iret;
     
    g_bDukpt_CurAppNo=g_byCurAppNum;
	g_bDukpt_CurKeyID=key_n;
	ret=s_ReadDukptKSN(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,tempksn);
	if(ret)
	{
		return ret;
	}
	memcpy(g_bDukpt_KSN,tempksn,10);
	tempksn[7]&=0x1f;
	g_lDukpt_EncryptCnt=bit21_to_ulong(&tempksn[7]);  
	
    if(mode & 0x80)  gNoPinEnterDisenable=1;
    else             gNoPinEnterDisenable=0;

    memset(pinbuf,0,sizeof(pinbuf));
    iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
    if(iret)
    {
        memset(pinbuf,0,sizeof(pinbuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return iret;
    }   
    pin_len =strlen((char *)pinbuf);
    if(pin_len ==0)
    {
        memset(pinbuf,0,sizeof(pinbuf));
        memset(pinkeybuf,0,sizeof(pinkeybuf));
        return PCI_NoInput_Err;
    }  
    iret=s_Dukpt_RequestPinOrMac(0,pinkeybuf,&pinkeylen,out_ksn);

	s_ClearDukptGlobalParameter();

    if(iret) return iret; 
///////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
     //Lib_GetCurSecond(&timemiddle);
	 Lib_GetDateTime(cur_time);
	 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciWritePinCount(1,timemiddle);
     count++;
     //save the count
     //Lib_GetCurSecond(&timeafter);//get current time
	 Lib_GetDateTime(cur_time);
	 timeafter=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
     interval = timeafter - timebefore;
	 s_PciReadPinCount(0,&dwCount[0]);
	 DwordToBytes(&dwCount[0],1,strict_buf);
     interval = interval + strict_buf[2]*256 + strict_buf[1];//accumulate the interval time
	 strict_buf[0] = count;
	 strict_buf[1] = interval%256;
	 strict_buf[2] = interval/256;
	 ByteToDword(strict_buf,4,&dwCount[0]);
	 s_PciWritePinCount(0,dwCount[0]);
#endif
///////////////////////////////////////////////

    memset(temp,0,sizeof(temp));
    strcpy((char *)temp,"0000");
    memcpy(&temp[4], card_no+4, 12);
    vTwoOne(temp, 16, one); 
    memset(temp,0,sizeof(temp));
    sprintf((char *)temp, "%02d", (int)pin_len);
    strcpy((char *)&temp[2], (char *)pinbuf);
    i=strlen((char *)temp);
    memset(&temp[i], 'F', 16-i);
    vTwoOne(temp,16,two);
    two[0]=pin_len;
    for(i=0;i<8;i++)  three[i]=one[i] ^ two[i];
    DES_TDES(pinkeybuf,pinkeylen,three,8,1);
    memcpy(pin_block, three, 8);
    memset(one,0,sizeof(one));
    memset(two,0,sizeof(two));
    memset(three,0,sizeof(three));
    memset(pinbuf,0,sizeof(pinbuf));
    memset(pinkeybuf,0,sizeof(pinkeybuf));
    return 0;
}


int  Lib_PciGetMacDukpt(uchar key_n,ushort inlen,uchar *indata,uchar *macout,uchar mode,uchar *out_ksn)
{
    uchar  mackeybuf[24],mackeylen,keymode,datain[2050]; 
    int    iret;
    ushort temp,i,inLen,j,k;  
	uchar  tempksn[10];  
    //printf("PCI_GetMacDukpt(%02x %d %02x)\r\n",mackey_n,inlen,mode);  
    ///////////////////////////////////////////////////////////////////////////////////////
    if(key_n >= MAX_DUKPT_NUM)
    {
        return PCI_KeyNo_Err;
    }   
	g_bDukpt_CurAppNo=g_byCurAppNum;
	g_bDukpt_CurKeyID=key_n;
	iret=s_ReadDukptKSN(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,tempksn);
	if(iret)return iret; 
	memcpy(g_bDukpt_KSN,tempksn,10);
	tempksn[7]&=0x1f;
	g_lDukpt_EncryptCnt=bit21_to_ulong(&tempksn[7]); 
    
	iret=s_Dukpt_RequestPinOrMac(1,mackeybuf,&mackeylen,out_ksn);

	s_ClearDukptGlobalParameter();

	if(iret) return iret; 
    /////////////////////////////////////////////////////////////////////////////////////
    if(inlen>2048 || inlen==0)
    {
        return PCI_DataLen_Err;
    }

    memset(datain,0,sizeof(datain));
    memcpy(datain,indata,inlen);

     if(mode==1){
        temp=inlen%8;
        if(temp) temp=8-temp;
        memset(&datain[inlen],0,temp);
        k=(ushort)inlen+(ushort)temp;
        k=k/8;
        for(i=1;i<k;i++)
        {
            for(j=0;j<8;j++) datain[j] ^= datain[j+8*i];
        }
        DES_TDES(mackeybuf,mackeylen,datain,8,1);
     }
     else if(mode==2)
     {
          switch(mackeylen/8)
          {
               case 0x01:
                    DesMac(mackeybuf,datain,inlen);
                    break;
               case 0x02:
                    Des3_16Mac_2(mackeybuf,datain,inlen);
                    break;
               case 0x03:
                    Des3_24Mac_2(mackeybuf,datain,inlen);
                    break;
               default: return PCI_KeyLen_Err;
          }
     }

    else{
        switch(mackeylen/8)
        {
        case 0x01:
            DesMac(mackeybuf,datain,inlen);
            break;
        case 0x02:
            Des3_16Mac(mackeybuf,datain,inlen);
            break;
        case 0x03:
            Des3_24Mac(mackeybuf,datain,inlen);
            break;
        default: return PCI_KeyLen_Err;
        }
    }
    memcpy(macout,datain,8);
    memset(mackeybuf,0,24);
    return 0;
}


int s_WriteSK_MACK(uchar *sk_mack,uchar sk_mack_len)
{
    int   iret;
    uchar i,lrc,appname[33];
    lrc=0;
    for(i=0;i<sk_mack_len;i++) lrc=lrc^sk_mack[i]; 
    memset(appname,0,sizeof(appname));
    memcpy(appname,"VOS",3);
    iret=s_WriteAuthKey(appname,SK_MACK_TYPE,0,sk_mack_len,sk_mack,lrc);
    return iret; 
} 

int s_ReadSK_MACK(uchar *sk_mack,uchar *sk_mack_len)
{
    int   iret;
    uchar appname[33],len;
    memset(appname,0,sizeof(appname));
    memcpy(appname,"VOS",3);
    iret=s_ReadAuthKey(appname,SK_MACK_TYPE,0,&len,sk_mack);
    *sk_mack_len=len;
    return iret;
}



//  获取加密PIN（X9.8 X3.92）　
int Lib_PciGetPinFixK(BYTE fixkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec)
{
    uchar i, j,def_max_len,ret;
    uchar temp[24],one[9],two[9],three[9],rand[16];
	uchar pin_len,len;
	uchar fixkeybuf[24], fixkeylen,pinbuf[24],strict_buf[8];
	int   iret;
	DWORD timebefore,timeafter,timemiddle,timetemp;
	WORD  interval=0;
	uchar count=0;
	DWORD dwCount[2];
	BYTE cur_time[7];
	
	memset(pin_block,0,8);
	
	
	def_max_len =14;
	if (min_len>max_len || max_len>def_max_len)
	{
		return PCI_InputLen_Err;
	}
	if(fixkey_n >= MAX_FK_NUM)
	{
		return PCI_KeyNo_Err;
	}
//////////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
	 //get current time in seconds
	 //Lib_GetCurSecond(&timebefore);
	 Lib_GetDateTime(cur_time);
	 timebefore=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
     if(strict_buf[0] == 0xff && strict_buf[1] == 0xff && strict_buf[2] == 0xff)//first time
     {
		 memset(strict_buf,0x00,sizeof(strict_buf));
		 ByteToDword(strict_buf,8,dwCount);
		 s_PciWritePinCount(0,dwCount[0]);
		 s_PciWritePinCount(1,dwCount[1]);
     }

     //read the count
	 s_PciReadPinCount(0,&dwCount[0]);
	 s_PciReadPinCount(1,&dwCount[1]);
	 DwordToBytes(dwCount,2,strict_buf);
	 count = strict_buf[0];
     if(count == 120)//the max count in max time
     {
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
         timetemp = (strict_buf[4]*256+strict_buf[5])*65535 + strict_buf[6]*256 + strict_buf[7];
		 s_PciReadPinCount(0,&dwCount[0]);
		 s_PciReadPinCount(1,&dwCount[1]);
		 DwordToBytes(dwCount,2,strict_buf);
		 //Lib_GetCurSecond(&dwCount[0]);
		 Lib_GetDateTime(cur_time);
		 dwCount[0]=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
         interval = dwCount[0]-timetemp+strict_buf[2]*256+strict_buf[1];//read the interval time
         if(interval < 3600)//the max time value
         {
             //Lib_GetCurSecond(&timemiddle);
			 Lib_GetDateTime(cur_time);
			 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
			 s_PciWritePinCount(1,timemiddle);
             return PCI_CallTimeInte_Err;
         }
         else
         {
             count = 0;
             interval = 0;
			 memset(strict_buf,0,sizeof(strict_buf));
			 ByteToDword(strict_buf,4,&dwCount[0]);
			 s_PciWritePinCount(0,dwCount[0]);
         }
     }
#endif
//////////////////////////////////////////////////	
	iret=s_ReadDesKey(g_byCurAppNum,FKEY_TYPE,fixkey_n,&fixkeylen,fixkeybuf); 
	if(iret) return iret;
	
	
	if(mode & 0x80)  gNoPinEnterDisenable=1;
	else             gNoPinEnterDisenable=0;
	
	memset(pinbuf,0,sizeof(pinbuf));
	iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
	if(iret)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(fixkeybuf,0,sizeof(fixkeybuf));
		return iret;
	}
	
	pin_len =strlen((char *)pinbuf);
	if(pin_len ==0)
	{
		memset(pinbuf,0,sizeof(pinbuf));
		memset(fixkeybuf,0,sizeof(fixkeybuf));
		return PCI_NoInput_Err;
	}
///////////////////////////////////////////////
#ifdef PIN_INPUT_LIMIT
     //Lib_GetCurSecond(&timemiddle);
	 Lib_GetDateTime(cur_time);
	 timemiddle=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
	 s_PciWritePinCount(1,timemiddle);
     count++;
     //save the count
     //Lib_GetCurSecond(&timeafter);//get current time
	 Lib_GetDateTime(cur_time);
	 timeafter=(cur_time[3]*3600)+(cur_time[4]*60)+cur_time[5];
     interval = timeafter - timebefore;
	 s_PciReadPinCount(0,&dwCount[0]);
	 DwordToBytes(&dwCount[0],1,strict_buf);
     interval = interval + strict_buf[2]*256 + strict_buf[1];//accumulate the interval time
	 strict_buf[0] = count;
	 strict_buf[1] = interval%256;
	 strict_buf[2] = interval/256;
	 ByteToDword(strict_buf,4,&dwCount[0]);
	 s_PciWritePinCount(0,dwCount[0]);
#endif
///////////////////////////////////////////////	
	memset(temp,0,sizeof(temp));
	strcpy((char *)temp,"0000");
	memcpy(&temp[4], card_no+4, 12);
	vTwoOne(temp, 16, one);
	
    /*   PIN-BLOCK = FORMAT 0
    memset(temp,0,sizeof(temp));
    mysprintf((char *)temp, "%02d", (int)pin_len);
    strcpy((char *)&temp[2], (char *)pinbuf);
    i=strlen((char *)temp);
    memset(&temp[i], 'F', 16-i);
    vTwoOne(temp,16,two);
    two[0]=pin_len;
    */
    //PIN-BLOCK = FORMAT 3
    Lib_PciGetRnd (rand);
    Lib_PciGetRnd (&rand[8]);
    memset(temp,0,sizeof(temp)); 
    temp[0]='3';
    temp[1]='0';
    strcpy((char *)&temp[2], (char *)pinbuf);
    i=strlen((char *)temp);
    for(j=0;j<(16-i);j++)
    {
        temp[i+j]=(rand[j]%6)+'A';
    } 
    vTwoOne(temp,16,two);
    two[0]=0x30+pin_len;  
	for(i=0;i<8;i++)  three[i]=one[i] ^ two[i];
	DES_TDES(fixkeybuf,fixkeylen,three,8,1);
	memcpy(pin_block, three, 8);
	memset(one,0,sizeof(one));
	memset(two,0,sizeof(two));
	memset(three,0,sizeof(three));
	memset(pinbuf,0,sizeof(pinbuf));
	memset(fixkeybuf,0,sizeof(fixkeybuf));
	return 0;
	
}

#if 0
int Lib_PciGetPinFixK(BYTE fixkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec)
{

    uchar i, j,def_max_len,ret;
    uchar temp[24],one[9],two[9],three[9],rand[16];
     uchar pin_len,len;
     uchar fixkeybuf[24], fixkeylen,pinbuf[24];
     int   iret;

     //printf("PCI_GetPin(%02x %02x %02x %02x %d)\r\n",pinkey_n,min_len,max_len,mode,waittime_sec);
 

     //if(k_LockStatus) return PCI_Locked_Err;
    memset(pin_block,0,8);

	#ifdef  __NEW_PCI_REQUIRE__
	if(s_PciCheckCallInteralMacro())
		return PCI_CallTimeInte_Err;
	#else
    if(g_PinInputIntervalCtrl)
    {
		if(!s_CheckTimer(0))
		{
			s_SetTimer(0,PCI_TIME_OUT);
			return PCI_CallTimeInte_Err;
		}
	}
	else
	{
		g_PinInputIntervalCtrl=1;
		s_SetTimer(0,PCI_TIME_OUT);
	}
	#endif


	def_max_len=14;
	if (min_len>max_len || max_len>def_max_len)
	{
		#ifdef  __NEW_PCI_REQUIRE__
		DelInvalidCall();
		#else
		s_SetTimer(0,PCI_TIME_OUT);
		#endif
		return PCI_InputLen_Err;
     }
     //if((pinkey_n & 0x7f)>=50)
	if(fixkey_n >= MAX_FK_NUM)
	{
		#ifdef  __NEW_PCI_REQUIRE__
		DelInvalidCall();
		#else
		s_SetTimer(0,PCI_TIME_OUT);
		#endif
		return PCI_KeyNo_Err;
	}

     //if(pinkey_n & 0x80)
     //{  //增加20070515：为了支持直联的工作密钥方式。
     //     iret=s_ReadDesKey(g_byCurAppNum,DESKEY_TYPE,pinkey_n&0x7f,&pinkeylen,pinkeybuf);
     //}
     //else
     iret=s_ReadDesKey(g_byCurAppNum,FKEY_TYPE,fixkey_n,&fixkeylen,fixkeybuf); 
     if(iret) return iret;


     if(mode & 0x80)  gNoPinEnterDisenable=1;
     else             gNoPinEnterDisenable=0;

     memset(pinbuf,0,sizeof(pinbuf));
     iret=s_PciInputPin(min_len, max_len, pinbuf,waittime_sec);
	#ifdef  __NEW_PCI_REQUIRE__
	 ;
	#else
          s_SetTimer(0,PCI_TIME_OUT);
	#endif
     if(iret)
     {
	#ifdef  __NEW_PCI_REQUIRE__
		 DelInvalidCall();
	#endif 
          memset(pinbuf,0,sizeof(pinbuf));
          memset(fixkeybuf,0,sizeof(fixkeybuf));
          return iret;
     }

     pin_len =strlen((char *)pinbuf);
     if(pin_len ==0)
     {
          memset(pinbuf,0,sizeof(pinbuf));
          memset(fixkeybuf,0,sizeof(fixkeybuf));
          return PCI_NoInput_Err;
     }

     memset(temp,0,sizeof(temp));
     strcpy((char *)temp,"0000");
     memcpy(&temp[4], card_no+4, 12);
     vTwoOne(temp, 16, one);

    /*   PIN-BLOCK = FORMAT 0
    memset(temp,0,sizeof(temp));
    mysprintf((char *)temp, "%02d", (int)pin_len);
    strcpy((char *)&temp[2], (char *)pinbuf);
    i=strlen((char *)temp);
    memset(&temp[i], 'F', 16-i);
    vTwoOne(temp,16,two);
    two[0]=pin_len;
    */
    //PIN-BLOCK = FORMAT 3
    Lib_PciGetRnd (rand);
    Lib_PciGetRnd (&rand[8]);
    memset(temp,0,sizeof(temp)); 
    temp[0]='3';
    temp[1]='0';
    strcpy((char *)&temp[2], (char *)pinbuf);
    i=strlen((char *)temp);
    for(j=0;j<(16-i);j++)
    {
        temp[i+j]=(rand[j]%6)+'A';
    } 
    vTwoOne(temp,16,two);
    two[0]=0x30+pin_len;  
	for(i=0;i<8;i++)  three[i]=one[i] ^ two[i];
	DES_TDES(fixkeybuf,fixkeylen,three,8,1);
	memcpy(pin_block, three, 8);
	memset(one,0,sizeof(one));
	memset(two,0,sizeof(two));
	memset(three,0,sizeof(three));
	memset(pinbuf,0,sizeof(pinbuf));
	memset(fixkeybuf,0,sizeof(fixkeybuf));
     return 0;

}
#endif

#ifdef __NEW_PCI_REQUIRE__
int s_PciCheckCallInteral(uint uiTimerNo  /* = PCI_TIMER2*/,
						  uint uiInputCnt /* = PCI_INPUT_COUNT*/, 
						  ulong ulTimeout /* = PCI_TIME_OUT*/)
{
	int i,isCQFull;
	static QE QTime   = 0 ; //单位秒
	static QE QTimeOld =0 ;//单位秒
	QE QTotalTime = 0 ;  //单位秒
	QE QTinterval ;			//单位秒 



	 //step1 设置定时器4 60分钟
	if(g_PinInputIntervalCtrl)
	{
		//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin]   mak1\r\n");
		if(!s_CheckTimer(uiTimerNo))
		{
			s_SetTimer(uiTimerNo,PCI_TIME_OUT);//满60分钟 重新计时
			g_PinInputCounterPer60min++;//
			//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin]   mark2\r\n");
		}

	}
	else //第一次调用该函数
	{
		//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin]   mark3\r\n");
		g_PinInputIntervalCtrl=1;
		s_SetTimer(uiTimerNo,PCI_TIME_OUT);
	}

	QTime = PCI_TIME_OUT - s_CheckTimer(uiTimerNo); //本次调用时间
	QTime = QTime/(100) + (g_PinInputCounterPer60min*PCI_TIME_OUT_PER_SEC); //换算成秒
	//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin]    QTime = %d  QTimeOld = %d \r\n",QTime,QTimeOld);
	QTinterval = QTime-QTimeOld;//计算相邻的两次调用时间间隔 
	//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin] QTinterval = %d\r\n",QTinterval);
	QTimeOld = QTime ;

	isCQFull = SaveCurCall(QTinterval);
	if(isCQFull)//队列已满
	{//计算时间
		for(i=0;i<uiInputCnt/*PCI_INPUT_COUNT*/;i++)//队列大小=总计数（满x次）
		{
			QTotalTime += gCQ.QueueData[i];
		}
		s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin]QTotalTime=%d\r\n",QTotalTime);
		//s_UartPrint(COM_DEBUG,"[PCI::Lib_PciGetPin] uiTotalTime = %d\r\n",QTime);
		if(QTotalTime<ulTimeout) //60min之内已经120次
		{
			//g_PinInputCounterPer60min = 0 ; 
			//ClrAllCall();
			return PCI_CallTimeInte_Err;
		}
	}
	return 0;
}

int test_PciGetPin(int rnd)
{
	uchar i, def_max_len;
	uchar temp[24],one[9],two[9],three[9];
	uchar pin_len,len;
	uchar pinkeybuf[24], pinkeylen,pinbuf[24];
	int iret;

	#ifdef  __NEW_PCI_REQUIRE__
	if(s_PciCheckCallInteralMacro())
		return PCI_CallTimeInte_Err;
	#endif
	return 0;

	

}
#endif 


unsigned char CrcPageCmp(unsigned char bpage,unsigned char *sBuf)
{
  unsigned char i;
  unsigned char sCrc[2],sCurCrc[2];
  unsigned short nOffset;
  
    i = bpage;
    
    //13/07/01
    if( (i==0) ||(i==1) ||(i==2)|| (i==3) )
    {
    if(i==0)
    {
       nOffset = PCI_CRC_ADDR_PAGE0;
       memcpy(sCrc,&sBuf[PCI_CRC_ADDR_PAGE0],2);
    }
    else if(i==1)
    {
       nOffset = PCI_CRC_ADDR_PAGE1;
       memcpy(sCrc,&sBuf[PCI_CRC_ADDR_PAGE1],2);
      
    }
    else if(i==2)
    {
       nOffset = PCI_CRC_ADDR_PAGE2;
       memcpy(sCrc,&sBuf[PCI_CRC_ADDR_PAGE2],2);
      
    }
    else if(i==3)
    {
       nOffset = PCI_CRC_ADDR_PAGE3;
       memcpy(sCrc,&sBuf[PCI_CRC_ADDR_PAGE3],2);
      
    }
    memcpy(&sBuf[nOffset],"\x00\x00",2);
    CalulateCRC16(sBuf,2048,sCurCrc);
    if(memcmp(sCurCrc,sCrc,2))
      return -1;
    }
    return 0;    
  
}


unsigned char CrcPageCalc(unsigned char bpage,unsigned char *sBuf)
{
  unsigned char i;
  unsigned char sCrc[2],sCurCrc[2];
  i = bpage;
  
  if(!( (i==0) ||(i==1) ||(i==2)|| (i==3) ))
    return -1;
  if(i==0)
  memcpy(&sBuf[PCI_CRC_ADDR_PAGE0],"\x00\x00",2);
  else if(i==1)
  memcpy(&sBuf[PCI_CRC_ADDR_PAGE1],"\x00\x00",2);
  else if(i==2)
  memcpy(&sBuf[PCI_CRC_ADDR_PAGE2],"\x00\x00",2);
  else if(i==3)
  memcpy(&sBuf[PCI_CRC_ADDR_PAGE3],"\x00\x00",2);
  
  CalulateCRC16(sBuf,2048,sCurCrc);
  if(i==0)
    memcpy(&sBuf[PCI_CRC_ADDR_PAGE0],sCurCrc,2);
  else if(i==1)
    memcpy(&sBuf[PCI_CRC_ADDR_PAGE1],sCurCrc,2);
  else if(i==2)
    memcpy(&sBuf[PCI_CRC_ADDR_PAGE2],sCurCrc,2);
  else if(i==3)
    memcpy(&sBuf[PCI_CRC_ADDR_PAGE3],sCurCrc,2);
  
  
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
    uchar   deskeybuf[24],deskeylen,keymode,datain[8],keytypeL4; 
    int     iret;

    //gCurAppNum=App_Msg.Num;
    gCurAppNum=0;//sxl
    if(deskey_n>=100)
    {
        return PCI_KeyNo_Err;
    }

	if ((indata == NULL) || (outdata == NULL))
		return PCI_NoInput_Err;
	if ((keytype & 0xf0) == 0x10)
		iret=s_ReadDesKey(gCurAppNum,PIN_MKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
	else if ((keytype & 0xf0) == 0x20)
		iret=s_ReadDesKey(gCurAppNum,PINKEY_TYPE,deskey_n,&deskeylen,deskeybuf);
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
