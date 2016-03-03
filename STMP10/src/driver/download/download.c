
#include "vosapi.h"
#include "dll.h"
#include "FlashOperate.h"
#include "download.h"
#include "pci.h"
#include "des.h"
#include "..\..\inc\FunctionList.h"

#include <time.h>
#include "stm32f10x_lib.h"
#include "Toolkit.h"

int      g_LocalDownMode=UART_DOWN_MODE;

/****************************************************************************
  函数名     :  int Down_RecvByte(uchar *recv_byte, int waitms)
  描述       :  本地下载接收单个字节函数
  输入参数   :  1、int waitms：
  输出参数   :  2、uchar *recv_byte：
  返回值     :  0：接收成功，1：下载通讯模式错误，-6503：非法通道号，
                -6502：串口未打开，-6504：接收超时
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Down_RecvByte(uchar *recv_byte, int waitms)
{
    switch (g_LocalDownMode)
    {
    case USBD_DOWN_MODE:
//        return s_UsbDeviceRecv(0, recv_byte, 1, &iTemp, waitms);
		return 1;
    case UART_DOWN_MODE:
        return Lib_ComRecvByte(LOAD_COM, recv_byte, waitms);
    default:
        return 1;
    }
}

int CheckKeyHandShack()
{
    int iRet;
    BYTE abyTemp[2];

    iRet = Down_RecvByte(abyTemp, 20);
    if ((0==iRet) && ('W'==abyTemp[0]))
    {
        Lib_Beep();
        return 0;
    }
    return 1;
}

int Down_SendByte(uchar send_byte)
{
    int iRet = -1;
    switch (g_LocalDownMode)
    {
    case USBD_DOWN_MODE:
//        iRet = s_UsbDeviceSend(0, &send_byte, 1);
        if (0 != iRet)
        {
            return iRet;
        }
//        (void)s_UsbStartSend();
        return 0;
    case UART_DOWN_MODE:
        return Lib_ComSendByte(LOAD_COM, send_byte);
    default:
        return 1;
    }
}

WORD g_awhalfCrc16CCITT[16]={ /* CRC 16bit余式表 */
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

/****************************************************************************
  函数名     :  void Crc16CCITT(BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[2])
  描述       :  用移位的方法计算一组数字的16位CRC-CCITT校验值
  输入参数   :  1、BYTE *pbyDataIn : 要进行16位CRC-CCITT计算的数
                2、WORD dwDataLen : pbyDataIn数组的长度最大值65535
  输出参数   :  1、BYTE abyCrcOut[2] : 16位CRC-CCITT计算的结果
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-08-30  V1.0         创建
****************************************************************************/
void Crc16CCITT(const BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[2])
{
    WORD wCrc = 0;
    BYTE byTemp;

    while (dwDataLen-- != 0)
    {
        byTemp = ((BYTE)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= g_awhalfCrc16CCITT[byTemp^(*pbyDataIn/16)];
        byTemp = ((BYTE)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= g_awhalfCrc16CCITT[byTemp^(*pbyDataIn&0x0f)];
        pbyDataIn++;
    }
    abyCrcOut[0] = wCrc/256;
    abyCrcOut[1] = wCrc%256;
}


/****************************************************************************
  函数名     :  static int Crc16RecvPacket(BYTE *pbyOutPacket, WORD *pwOutLen,
                      WORD *pwCmd, DWORD dwTimeOutMs)
  描述       :  本地下载数据包接收函数
  输入参数   :  1、DWORD dwTimeOutMs：超时时间，单位是毫秒
  输出参数   :  1、BYTE *pbyOutPacket：数据包内容，包括包头和包尾
                2、WORD *pwOutLen：输出的数据包内容长度，不包括包头包尾
                3、WORD *pwCmd：命令字
  返回值     :  0：成功，1：下载通讯模式错误，2：CRC校验错误，3：接收的数据包
                长度非法，-6503：非法通道号，-6502：串口未打开，-6504：接收超时
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-28   V1.0        创建
****************************************************************************/
static int Crc16RecvPacket(BYTE *pbyOutPacket, WORD *pwOutLen, WORD *pwCmd, DWORD dwTimeOutMs)
{
    int i, iRet;
    BYTE abyCrc16[2];

    s_SetTimer(1, dwTimeOutMs);
    while (1)
    {
        if (0==s_CheckTimer(1))
        {
            return 1;
        }
        iRet = Down_RecvByte(pbyOutPacket, 30);
        if ((0==iRet) && (0x02==pbyOutPacket[0]))
        {
            break;
        }
    }
    for (i=0; i<4; i++)
    {
        iRet = Down_RecvByte(&pbyOutPacket[i+1], 50);
        if (0 != iRet)
        {
            return iRet;
        }
    }
    *pwCmd = pbyOutPacket[1]*256 + pbyOutPacket[2];
    *pwOutLen = pbyOutPacket[3]*256 + pbyOutPacket[4];
    if (*pwOutLen > 8900)
    {
        return 3;
    }
    for (i=0; i<(*pwOutLen+2); i++)
    {
        iRet = Down_RecvByte(&pbyOutPacket[i+5], 50);
        if (0 != iRet)
        {
            return iRet;
        }
    }
    Crc16CCITT(&pbyOutPacket[1], *pwOutLen+4, abyCrc16);
    if (0 != memcmp(abyCrc16, &pbyOutPacket[*pwOutLen+5], 2))
    {
        return 2;
    }
    return 0;
}

int Down_Send(uchar *send_data, int send_len)
{
    int iRet = -1;
    switch (g_LocalDownMode)
    {
    case USBD_DOWN_MODE:
//        iRet = s_UsbDeviceSend(0, send_data, send_len);
        if (0 != iRet)
        {
            return iRet;
        }
        //(void)s_UsbStartSend();
        return 0;

    case UART_DOWN_MODE:
        return Lib_ComSend(LOAD_COM, send_data, send_len);

    default:
        return 1;
    }
}


/****************************************************************************
  函数名     :  static void Crc16SendPacket(WORD wCmd, BYTE*pbySendBuff, WORD wSendLen)
  描述       :  本地下载数据包发送函数
  输入参数   :  1、WORD wCmd：命令码
                2、BYTE*pbySendBuff：要发送的数据内容
                3、WORD wSendLen：发送数据内容的长度
  输出参数   :  无
  返回值     :  无
  修改历史
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-28   V1.0        创建
****************************************************************************/
static void Crc16SendPacket(WORD wCmd, BYTE*pbySendBuff, WORD wSendLen)
{
    BYTE g_DownSendBuff[512];//add 
    
    g_DownSendBuff[0] = 0x02;
    g_DownSendBuff[1] = wCmd/256;
    g_DownSendBuff[2] = wCmd%256;
    g_DownSendBuff[3] = wSendLen/256;
    g_DownSendBuff[4] = wSendLen%256;
    memcpy(&g_DownSendBuff[5], pbySendBuff, wSendLen);
    Crc16CCITT(&g_DownSendBuff[1], wSendLen+4, &g_DownSendBuff[wSendLen+5]);
    (void)Down_Send(g_DownSendBuff, wSendLen+7);
}

int CheckHandShack(void)
{
    int iRet;
    BYTE abyTemp[2];

    iRet = Down_RecvByte(abyTemp, 120);//120
    if ((0==iRet) && ('L'==abyTemp[0]))
    {
        Lib_Beep();
        return 0;
    }
    //else if(iRet==0)
    //{
    //    Down_SendByte(abyTemp[0]);
    //}

    return 1;
}

int hal_sys_get_serial_number(unsigned char *nUniqueID)
{
  u32 Dev_Serial0,Dev_Serial1,Dev_Serial2;
  
	//获取芯片ID
  Dev_Serial0=*(vu32*)(0x1ffff7e8);
  Dev_Serial1=*(vu32*)(0x1ffff7ec);
  Dev_Serial2=*(vu32*)(0x1ffff7f0);
  memcpy(nUniqueID,(unsigned char *)Dev_Serial0,4);
  memcpy(&nUniqueID[4],(unsigned char *)Dev_Serial1,4);
  memcpy(&nUniqueID[8],(unsigned char *)Dev_Serial2,4);
  return 0;  
}

/****************************************************************************
  函数名     :  void WriteLog(char *str, int iWriteTimeFlag, int iAminister)
  描述       :  记录日志函数
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 汤冬勇     2010-06-07  V1.0         创建
****************************************************************************/
void WriteLog(char *str, int iWriteTimeFlag, int iAminister)
{
#if 0 //12/10/28
    int fd;
    long lLen;
    BYTE abyDateTime[10];
    char strTemp[30];

    lLen = Lib_FileSize(OPERATE_LOG_FILE_NAME);
    if (lLen > (10*1024))
    {
    	#if 0
        Lib_LcdCls();
       /* Lib_LcdGotoxy(0, 0);
        Lib_Lcdprintf("Log file is out memory!");
        Lib_LcdGotoxy(0, 32/2);
        Lib_Lcdprintf("Pls send log to PC.");*/
		Lib_LcdPrintxy(0,8*0,0x00,"Log file is out memory!");
		Lib_LcdPrintxy(0,8*2,0x00,"Pls send log to PC.");
        Lib_KbGetCh();
		#endif

		Lib_FileRemove(OPERATE_LOG_FILE_NAME);
        return ;
    }
    lLen = Lib_FileFreeSize();
    if (lLen < 1024)
    {
    	#if 0
        Lib_LcdCls();
       /* Lib_LcdGotoxy(0, 0);
        Lib_Lcdprintf("no free space to save log!");
        Lib_LcdGotoxy(0, 32/2);
        Lib_Lcdprintf("Pls send log to PC.");*/
		Lib_LcdPrintxy(0,8*0,0x00,"no free space to save log!");
		Lib_LcdPrintxy(0,8*2,0x00,"Pls send log to PC.");
        Lib_KbGetCh();
		#endif
		
        return ;
    }
    fd = Lib_FileOpen(OPERATE_LOG_FILE_NAME, O_RDWR);
    if (fd < 0)
    {
        fd = Lib_FileOpen(OPERATE_LOG_FILE_NAME, O_CREATE);
        if (fd < 0)
        {
            return ;
        }
    }
    (void)Lib_FileSeek(fd, 0, FILE_SEEK_END);
    if (0 != iWriteTimeFlag)
    {
        Lib_GetDateTime(abyDateTime);
        if (abyDateTime[0]<50)
        {
            strcpy(strTemp, "20");
        }
        else
        {
            strcpy(strTemp, "19");
        }
        strTemp[2] = (abyDateTime[0]>>4)+'0';
        strTemp[3] = (abyDateTime[0]&0x0f)+'0';
        strTemp[4] = '/';
        strTemp[5] = (abyDateTime[1]>>4)+'0';
        strTemp[6] = (abyDateTime[1]&0x0f)+'0';
        strTemp[7] = '/';
        strTemp[8] = (abyDateTime[2]>>4)+'0';
        strTemp[9] = (abyDateTime[2]&0x0f)+'0';
        strTemp[10] = ' ';
        strTemp[11] = (abyDateTime[3]>>4)+'0';
        strTemp[12] = (abyDateTime[3]&0x0f)+'0';
        strTemp[13] = ':';
        strTemp[14] = (abyDateTime[4]>>4)+'0';
        strTemp[15] = (abyDateTime[4]&0x0f)+'0';
        strTemp[16] = ':';
        strTemp[17] = (abyDateTime[5]>>4)+'0';
        strTemp[18] = (abyDateTime[5]&0x0f)+'0';
        strTemp[19] = ' ';
        strTemp[20] = 0;
        (void)Lib_FileWrite(fd, strTemp, strlen(strTemp));
    }
    if ((0==iAminister) || (1==iAminister))
    {
        if (0 == iAminister)
        {
            strcpy(strTemp, "Administer A ");
        }
        else
        {
            strcpy(strTemp, "Administer B ");
        }
        (void)Lib_FileWrite(fd, strTemp, strlen(strTemp));
    }
    (void)Lib_FileWrite(fd, str, strlen(str));
    (void)Lib_FileClose(fd);
#endif
    
}


void WriteLoadLog(BYTE byKeyType, BYTE byFlag)
{
    switch (byKeyType)
    {
    case CHANGE_BAUDRATE:
        WriteLog("Change Baudrate ", 1, 0xff);
        break ;
    case AUTHENTICATE:
        WriteLog("Authenticate ", 1, 0xff);
        break ;
    case GET_CPU_SN:
        WriteLog("Get CPU SN ", 1, 0xff);
        break ;
    case DELETE_FILE:
        WriteLog("Delete File ", 1, 0xff);
        break ;
    case SET_POS_TIME:
        WriteLog("Set POS Time ", 1, 0xff);
        break ;
    case CREATE_FS:
        WriteLog("Create File System ", 1, 0xff);
        break ;
    case GET_SN:
        WriteLog("Get Sn ", 1, 0xff);
        break ;
    case SET_SN:
        WriteLog("Set Sn ", 1, 0xff);
        break ;
	case DELETE_APP_NAME:
		WriteLog("Delete App Name ",1,0xff);
		break;
    case AUTHMDK_TYPE:
        WriteLog("Load MDK ", 1, 0xff);
        break ;
	case SK_MACK_TYPE:
		WriteLog("Load SK_MACK ",1,0xff);
		break;
	case AUTHVOS_TYPE:
		WriteLog("Load VSK ",1,0xff);
		break;
	case KM_APP_SIGN_KEY:
		WriteLog("Load ASK ",1,0xff);
		break;
	case KM_PCI_MAIN_PIN_KEY:
		WriteLog("Load PIN_MK ",1,0xff);
		break;
 	case KM_PCI_MAIN_MAC_KEY:
		WriteLog("Load MAC_MK ",1,0xff);
		break;
	case KM_PCI_FKEY:
		WriteLog("Load FixK ",1,0xff);
		break;
	case DUKPTK_TYPE:
		WriteLog("Load Dukptk ",1,0xff);
		break;
	default:
        break;
    }
    if (0 == byFlag)
    {
        WriteLog("OK.\n", 0, 0xff);
    }
    else
    {
        WriteLog("Failed!\n", 0, 0xff);
    }
}



BYTE s_KM_GetLrc(BYTE *pbyDataIn, int iDataLen)
{
    BYTE byLrc;
    int i;

    byLrc = 0;
    for (i=0; i<iDataLen; i++)
    {
        byLrc ^= pbyDataIn[i];
    }
    return byLrc;
}

DWORD s_KM_SaveAppKey(BYTE *pbyKeyDataIn)
{
    int iRet;
    BYTE byKeyLen, byLrc;

    //byMainKeyIndex = pbyKeyDataIn[0];
    //byKeyIndex     = pbyKeyDataIn[1];
    byKeyLen       = pbyKeyDataIn[2];
    //if (KM_DEFAULT_KEY_LEN != byKeyLen)
    if(8!=byKeyLen && 16!=byKeyLen && 24!=byKeyLen)
    {
        /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE("  密钥长度错误  ", "Key length error"); */
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"Key length error");
        //printf("save(0)=%d %02x\r\n",byKeyLen,byKeyLen); 
        return KM_APP_KEY_LEN_ERROR;
    }
    pbyKeyDataIn[59] = 0;
    iRet = SPF_GetAppNO((char *)&pbyKeyDataIn[27]);
    if (iRet < 0)
    {
       /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE("  应用不存在   ", "App. not exist  ");*/
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"App. not exist  ");
//        Lib_Lcdprintf("as=%s,",&pbyKeyDataIn[27]); 
        //printf("save(1)=%d %02x\r\n",iRet,iRet);
        return KM_APP_NOT_EXIST;
    }
    g_byCurAppNum = iRet;
    
    
    switch (pbyKeyDataIn[0])
    {
    case KM_APP_SIGN_KEY:  // 应用程序签名密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHAPP_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
//        dwRet = AM_SaveAppSignKey(&pbyKeyDataIn[5]);
//        iRet = dwRet;
        break;
    case KM_APP_AUTH_MAIN_PIN_KEY:   // 应用程序PCI接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHPIN_MK_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_APP_AUTH_PIN_KEY:   // 应用程序PCI接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHPINK_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_APP_AUTH_MAIN_MAC_KEY:   // 应用程序PCI接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHMAC_MK_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_APP_AUTH_MAC_KEY:   // 应用程序PCI接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHMACK_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_APP_AUTH_OFFPIN_KEY:   // 应用程序PCI接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHOFFPINK_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_APP_AUTH_LCD_KEY:   // 应用程序LCD接口调用认证密钥
        byLrc = s_KM_GetLrc(&pbyKeyDataIn[3], pbyKeyDataIn[2]);
        iRet = s_WriteAuthKey(&pbyKeyDataIn[27], KM_AUTHLCD_TYPE,
            pbyKeyDataIn[1], pbyKeyDataIn[2], &pbyKeyDataIn[3], byLrc);
        break;
    case KM_PCI_MAIN_PIN_KEY:  // PIN主密钥
 //       iRet = Lib_PciWritePIN_MKey(pbyKeyDataIn[1], pbyKeyDataIn[2],
 //           &pbyKeyDataIn[3], 0x00);
         iRet = Lib_PciWriteWORK_MKey(pbyKeyDataIn[1]%50, pbyKeyDataIn[2],
            &pbyKeyDataIn[3], 0x00, 0, 0);
        break;
    case KM_PCI_PIN_KEY:       // PIN密钥
        //iRet = Lib_PciWritePinKey(pbyKeyDataIn[1], pbyKeyDataIn[2],
        //    &pbyKeyDataIn[3], 0x00, pbyKeyDataIn[1]);
        break;
    case KM_PCI_MAIN_MAC_KEY:  // MAC主密钥
//        iRet = Lib_PciWriteMAC_MKey(pbyKeyDataIn[1], pbyKeyDataIn[2],
//            &pbyKeyDataIn[3], 0x00);
        iRet = Lib_PciWriteWORK_MKey(pbyKeyDataIn[1]%50+50, pbyKeyDataIn[2],
            &pbyKeyDataIn[3], 0x00, 0, 0);

        break;
    case KM_PCI_MAC_KEY:       // MAC密钥
        //iRet = Lib_PciWriteMacKey(pbyKeyDataIn[2], pbyKeyDataIn[3],
        //    &pbyKeyDataIn[4], 0x00, pbyKeyDataIn[1]);
        break;
	case KM_PCI_FKEY:  // FIXED 密钥
        iRet = Lib_PciWriteDesKey(pbyKeyDataIn[1], pbyKeyDataIn[2],
            &pbyKeyDataIn[3], 0x00);
        break;
    default:
        g_byCurAppNum = 0xff;
        return KM_APP_KEY_TYPE_ERROR;
    }
    g_byCurAppNum = 0xff;
    if (0 != iRet)
    {
        Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
       /* Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE(" 密钥写入失败   ", "Key Save failed "); */
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"Key Save failed ");
        //printf("save(2)=%d %02x\r\n",iRet,iRet); 
        return KM_APP_SAVE_KEY_ERROR;
    }
    return KM_SUCCESS;
}

DWORD s_KM_SaveDukptKey(BYTE *pbyKeyDataIn)
{
    int iRet;
    BYTE byKeyIndex, byKeyLen,byKsnLen;
	BYTE bdkksnbuf[100]; 

    //byKeyType      = pbyKeyDataIn[0];
	byKsnLen       = pbyKeyDataIn[1];
    byKeyIndex     = pbyKeyDataIn[2];
    byKeyLen       = pbyKeyDataIn[3];
    //if (KM_DEFAULT_KEY_LEN != byKeyLen)
    if(16!=byKeyLen && 24!=byKeyLen)
    {
        /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE("  密钥长度错误  ", "Key length error"); */
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"Key length error");
        //printf("save(0)=%d %02x\r\n",byKeyLen,byKeyLen);  
        return KM_APP_KEY_LEN_ERROR;
    }

	if(10!=byKsnLen)
    {
       /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE("  KSN长度错误  ", "KSN length error"); */
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"KSN length error");
        //printf("save(0)=%d %02x\r\n",byKsnLen,byKsnLen);  
        return KM_APP_KEY_LEN_ERROR;
    }

    pbyKeyDataIn[59+16+1] = 0;
    iRet = SPF_GetAppNO((char *)&pbyKeyDataIn[27+16+1]);
    if (iRet < 0)
    {
        /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE("  应用不存在    ", "App. not exist  ");*/
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"App. not exist  ");
//      Lib_Lcdprintf("as=%s,",&pbyKeyDataIn[27]); 
        //printf("save(1)=%d %02x\r\n",iRet,iRet);
        return KM_APP_NOT_EXIST;
    }
    g_byCurAppNum = iRet;

    memset(bdkksnbuf,0,sizeof(bdkksnbuf));
	memcpy(bdkksnbuf,&pbyKeyDataIn[4],byKeyLen);
	memcpy(bdkksnbuf+byKeyLen,&pbyKeyDataIn[4+24],byKsnLen);

	iRet=s_DukptLoadKey(g_byCurAppNum,byKeyIndex,byKeyLen,byKsnLen,bdkksnbuf); 
     
    g_byCurAppNum = 0xff;
    if (0 != iRet)
    {
       /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 24/2);
        Lib_LcdPrintfCE(" 密钥写入失败   ", "Key Save failed "); */
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*2,0x00,"Key Save failed ");
        //printf("save(2)=%d %02x\r\n",iRet,iRet); 
        return KM_APP_SAVE_KEY_ERROR;
    }
    return KM_SUCCESS;
}

DWORD KM_LoadKey(void)
{
    WORD wCmd, wPacketLen;
    int iCount = 0, iRet;
    BYTE abyTemp[300], abyAuthKey[25],byKeyLen, byKeyType, byKeyIndex,byKsnLen,abyTMacKey[25];
    BYTE macdata[300];
    BYTE abyHandShackKey[25];
    int iStep = 0;
    int iKeyNumCnt=0;
    DWORD dwRet;
    
    BYTE g_DownRecvBuff[300];
    
    
    memset(abyHandShackKey, 0x11, 8);
    memset(&abyHandShackKey[8], 0x22, 8);
    memset(&abyHandShackKey[16], 0x33, 8);
	memset(abyTMacKey,0,sizeof(abyTMacKey));
#ifdef VOSTRANSKEY_INIT
    memset(abyAuthKey,0x33,24);
    dwRet=KM_SUCCESS;
#else
    dwRet = s_KM_GetLoadCertifyKey(abyAuthKey);
#endif
    if (KM_SUCCESS != dwRet)
    {
        WriteLog("Input Transact Key Failed", 1, 0xff);
        return dwRet;
    }
	WriteLog("Input Transact Key OK", 1, 0xff);

#ifdef VOSTRANSKEY_INIT
    memset(abyTMacKey,0x44,24);
    dwRet=KM_SUCCESS;
#else
    dwRet = s_KM_GetT_MacKey(abyTMacKey);
#endif
    if (KM_SUCCESS != dwRet)
    {
        WriteLog("Input T-Mac Key Failed", 1, 0xff);
        return dwRet;
    }
    WriteLog("Input T-Mac Key OK", 1, 0xff);

    
////////////////ptk and tmack can't be same
	if (memcmp(abyAuthKey,abyTMacKey,24) == 0)
	{
       /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
        Lib_LcdGotoxy(0, 16/2);
		Lib_LcdGotoxy(0, 2*8/2);
		Lib_LcdPrintfCE(" PTK和TMACK相同  ", " PTK And TMACK \n  Same Err!");*/
		Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
		Lib_LcdPrintxy(0,8*1,0x00," PTK And TMACK \n  Same Err!");
		Lib_DelayMs(2000);
        WriteLog("PTK And TMACK Cant't Be Same!!!\n", 1, 0xff);
        return PCI_KeySame_Err;
	}
        
        
////////////////
    Lib_LcdCls();
    /*(void)Lib_LcdSetAttr(1);
    Lib_LcdGotoxy(0, 0);
    Lib_LcdPrintfCE("    下载密钥    ", "    Load Key    ");*/
	Lib_LcdPrintxy(0,8*0,0x80,"      Load Key      ");
    (void)Lib_LcdSetAttr(0);

    s_SetTimer(0,6000);//modify 30 seconds to 1 minute

reloadkey:
    /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
    Lib_LcdGotoxy(0, 32/2);
    Lib_LcdPrintfCE(" 正在连接...  ", "Connecting... ");*/
	Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
	Lib_LcdPrintxy(0,8*2,0x00,"Connecting... ");
    iStep = 0;

    while (1)
    {
      
        if(!s_CheckTimer(0))
        {
            /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE("    等待超时    ", "TimeOut For Load");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*2,0x00,"TimeOut For Load");
            WriteLog("TimeOut For Load Key!\n", 1, 0xff);
            return KM_WAIT_TIMEOUT_ERROR;
        }

        if(!s_CheckTimer(3))
        {
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 32/2);
            Lib_LcdPrintfCE("    等待超时    ", "TimeOut Quit");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*2,0x00,"TimeOut Quit");
            WriteLog("TimeOut For Load Key!\n", 1, 0xff);
            return KM_WAIT_TIMEOUT_ERROR;
        }
        if (0 == CheckKeyHandShack())
        {
            (void)Down_SendByte('B');
            break;
        }
        if (0 == Lib_KbCheck())
        {
            if (KEYCANCEL == Lib_KbGetCh())
            {
                return KM_USER_PRESS_CANCEL;
            }
        }
        iCount++;
    }

    while (1)
    {
        wCmd = 0;
        iRet = Crc16RecvPacket(g_DownRecvBuff, &wPacketLen, &wCmd, 500);  //300
        if (0 != iRet)
        {
            /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE(" 接收失败   ", "Recv. error ");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00,"Recv. error ");
            WriteLog("Receive Failed!\n", 1, 0xff);
            goto reloadkey;
        }
        s_SetTimer(0,6000);//modify 30seconds to 1mins

//        if (   (2!=iStep)
//            && (AUTHENTICATE_REQUEST!=wCmd)
//            && (AUTHENTICATE_STEP3!=wCmd)
//            && (ALL_FINISHED!=wCmd))
//        {
//            continue;
//        }
        switch (wCmd)
        {
        case AUTHENTICATE_REQUEST:
            if ((0!=iStep) || (8!=wPacketLen))
            {
                break;
            }
            Lib_LcdClrLine(16, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16);
            //Lib_LcdPrintfCE("   认证         ", " Authenticate   ");
            Lib_Lcdprintf(" Authenticate   ");
            iStep++;
            memset(abyTemp, 0, sizeof(abyTemp));
            Lib_Des24(&g_DownRecvBuff[5], abyTemp, abyHandShackKey, ENCRYPT);
            Lib_GetRand(&abyTemp[8], 8);  // 认证随机数
            strcpy((char *)&abyTemp[16], "VPOS306");  // 机型
            Crc16SendPacket(AUTHENTICATE_STEP2, abyTemp, 32);
            break;
        case AUTHENTICATE_STEP3:
            if ((1!=iStep) || (9!=wPacketLen))
            {
                break;
            }
            Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            //Lib_LcdPrintfCE("   认证2        ", " Authenticate 2 ");
            Lib_Lcdprintf(" Authenticate 2 ");
            if (0 != g_DownRecvBuff[5])
            {
                Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 24/2);
                //Lib_LcdPrintfCE("   认证失败     ", " Auth  failed   ");
                Lib_Lcdprintf(" Auth  failed   ");
                return KM_AUTH_ERROR;
            }
            Lib_Des24(&g_DownRecvBuff[6], &abyTemp[20], abyHandShackKey, DECRYPT);
            if (0 != memcmp(&abyTemp[20],&abyTemp[8],8))
            {
                abyTemp[0] = 1;
                Crc16SendPacket(AUTHENTICATE_RESPOND, abyTemp, 1);
                Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 24/2);
                //Lib_LcdPrintfCE("   认证失败1    ", " Auth  failed1  ");
                Lib_Lcdprintf(" Auth  failed1  ");
                return KM_AUTH_ERROR;
            }
            else
            {
                abyTemp[0] = 0;
                Crc16SendPacket(AUTHENTICATE_RESPOND, abyTemp, 1);
            }
            iStep++;
            Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            //Lib_LcdPrintfCE(" 认证完成   ", "Authenticate OK");
            Lib_Lcdprintf("Authenticate OK");
            
            iKeyNumCnt++;
            
            break;
        case LOAD_GET_CPU_SN_REQUEST:
            /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE("   读取CPU序号 ", " Read CPU SN");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00," Read CPU SN");
            memset(abyTemp, 0, 33);
            iRet = hal_sys_get_serial_number(&abyTemp[1]);
            if (0 != iRet)
            {
                abyTemp[0] = 1;
            }
#if 0            
	    WriteLoadLog(GET_CPU_SN,abyTemp[0]);
#endif            
            Crc16SendPacket(LOAD_GET_CPU_SN_RESPOND, abyTemp, 33);
            break;
        /*case LOAD_BOOT_SIGN_KEY_REQUEST:
            Lib_LcdClrLine(16, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16);
            Lib_LcdPrintfCE("   下载BSK   ", "  Load BSK ");
            Lib_Des24(&g_DownRecvBuff[5], abyTemp, abyAuthKey, DECRYPT);
            Lib_Des24(&g_DownRecvBuff[13], &abyTemp[8], abyAuthKey, DECRYPT); 
			 
           
            memset(macdata, 0, sizeof(macdata));
            memcpy(&macdata[0], &abyTemp[0], 16);
            Des3_24Mac(abyTMacKey,macdata,16); 
            if(memcmp(macdata,&g_DownRecvBuff[13+8+8],4)==0)
            {
                //ByteToDword(abyTemp, 16, adwTemp);
                iRet = s_WriteBSK(abyTemp); 
                //Lib_LcdCls();
                //Lib_Lcdprintf("wbsk:iret=%d %02x%02x",iRet,abyTemp[0],abyTemp[1]);
                //Lib_KbGetCh();
            }
            else
            {
                iRet = 1;
            } 


            if (0 == iRet)
            {
                abyTemp[0] = 0;
                //s_WriteBootSign();
                Crc16SendPacket(LOAD_BOOT_SIGN_KEY_RESPOND, abyTemp, 1);
            }
            else
            {
                abyTemp[0] = 1;
                Crc16SendPacket(LOAD_BOOT_SIGN_KEY_RESPOND, abyTemp, 1); 
                Lib_LcdClrLine(16, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 32);
                Lib_LcdPrintfCE("  PTK/MAC错误!  ", "  MAC/PTK Error! "); 
                return KM_PTK_ERROR;
            }
            //Crc16SendPacket(LOAD_VOS_SIGN_KEY_RESPOND, abyTemp, 1);
            iStep = 0;
            break;*/
            
		case LOAD_VOS_SIGN_KEY_REQUEST:
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE("   下载VSK   ", "  Load VSK ");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			//Lib_LcdPrintxy(0,8*1,0x00,"  Load VSK ");
			Lib_LcdPrintxy(0,8*1,0x00,"not support Load VSK ");
#if 0		 
            memset(macdata, 0, sizeof(macdata));
            memcpy(&macdata[0], &g_DownRecvBuff[5], 24);
            Des3_24Mac(abyTMacKey,macdata,24); 
            if(memcmp(macdata,&g_DownRecvBuff[13+8+8],4)==0)
            {
				Lib_Des24(&g_DownRecvBuff[5], abyTemp, abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[13], &abyTemp[8], abyAuthKey, DECRYPT); 
				iRet = s_WriteVSK(abyTemp);
            }
            else
            {
                iRet = 1;
            } 

			WriteLoadLog(AUTHVOS_TYPE,abyTemp[0]);
            if (0 == iRet)
            {
                abyTemp[0] = 0;
                //s_WriteBootSign();
                Crc16SendPacket(LOAD_VOS_SIGN_KEY_RESPOND, abyTemp, 1);
            }
            else
#endif              
            {
                abyTemp[0] = 1;
                Crc16SendPacket(LOAD_VOS_SIGN_KEY_RESPOND, abyTemp, 1); 
                /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 24/2);
                Lib_LcdPrintfCE("  PTK/MAC错误!  ", "  MAC/PTK Error! "); */
				Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0,8*2,0x00,"  MAC/PTK Error! ");
                return KM_PTK_ERROR;
            }
            //Crc16SendPacket(LOAD_VOS_SIGN_KEY_RESPOND, abyTemp, 1);

            //iStep = 0;
            //break;
        case LOAD_MAG_KEY_REQUEST:
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE("   下载MDK   ", "  Load MDK ");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00,"  Load MDK ");
            Lib_Des24(&g_DownRecvBuff[5], abyTemp, abyAuthKey, DECRYPT);
            Lib_Des24(&g_DownRecvBuff[13], &abyTemp[8], abyAuthKey, DECRYPT);
            Lib_Des24(&g_DownRecvBuff[21], &abyTemp[16], abyAuthKey, DECRYPT);
            memcpy(macdata, abyTemp, 24);
			/*
            for (i=1; i<3; i++)
            {
                for (j=0; j<8; j++)
                {
                    macdata[j] ^= macdata[i*8+j];
                }
            }
            Lib_Des24(macdata, mac, abyAuthKey, ENCRYPT);
			*/
			Des3_24Mac(abyTMacKey,macdata,24);  
            if (0 == memcmp(macdata, &g_DownRecvBuff[29],4))
            {
                iRet = s_WriteMDK(abyTemp);
            }
            else
            {
                iRet = 1;
            }
            iRet = 0;
            if (0 == iRet)
            {
                abyTemp[0] = 0;
            }
            else
            {
                abyTemp[0] = 1;
            }
			WriteLoadLog(AUTHMDK_TYPE,abyTemp[0]);
            Crc16SendPacket(LOAD_MAG_KEY_RESPOND, abyTemp, 1);
            iStep = 0;
            break;
         case LOAD_SK_MACK_REQUEST:
            /*Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE("  下载SK_MACK  ", "  Load SK_MACK ");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00,"  Load SK_MACK ");
      
            byKeyLen   = g_DownRecvBuff[5]; 
            abyTemp[0] = byKeyLen; 
            
            
            memset(macdata, 0, sizeof(macdata)); 
            memcpy(macdata, &g_DownRecvBuff[5], 25);//abyTemp
			Des3_24Mac(abyTMacKey,macdata,25);  
            
            if (0 == memcmp(macdata, &g_DownRecvBuff[30],4))
            { 
		        Lib_Des24(&g_DownRecvBuff[6], &abyTemp[1], abyAuthKey, DECRYPT);
		        Lib_Des24(&g_DownRecvBuff[14], &abyTemp[9], abyAuthKey, DECRYPT);
		        Lib_Des24(&g_DownRecvBuff[22], &abyTemp[17], abyAuthKey, DECRYPT); 
                iRet = s_WriteSK_MACK(&abyTemp[1],abyTemp[0]);
            }
			else
            {
                iRet = 1;;
            }
            
            
            //Lib_LcdCls();
            //Lib_Lcdprintf("iret=%d",iRet);
            //Lib_KbGetCh();
			if (0 == iRet)
            {
                abyTemp[0] = 0;
            }
            else
            {
                abyTemp[0] = 1;
            }
			WriteLoadLog(SK_MACK_TYPE,abyTemp[0]);
            Crc16SendPacket(LOAD_SK_MACK_RESPOND, abyTemp, 1);
            iKeyNumCnt++;
            if(iKeyNumCnt> KM_SUPPORT_MAX_KEYNUM)
            {
               /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 32/2);
                Lib_LcdPrintfCE(" 下载密钥数超限 ", "Key Numbers Over"); */
				Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0,8*2,0x00,"Key Numbers Over");
                return KM_SUCCESS;
            }
            iStep = 0;
            break;
        case LOAD_APP_KEY_REQUEST:
            if (64 != wPacketLen)
            {
                break;
            }
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE("  下载APP密钥   ", " Load App Key");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00," Load App Key");

            byKeyType  = g_DownRecvBuff[5];
            byKeyIndex = g_DownRecvBuff[7];
            byKeyLen   = g_DownRecvBuff[8];
            abyTemp[0] = byKeyType;
            abyTemp[1] = byKeyIndex;
            abyTemp[2] = byKeyLen;
            memset(macdata, 0, sizeof(macdata));
            memcpy(&macdata[0], &g_DownRecvBuff[5], 4+32+24);
            //memcpy(&macdata[4], &abyTemp[3], 32+24);

			
			Des3_24Mac(abyTMacKey,macdata,4+32+24); 
            if(0 == memcmp(macdata,&g_DownRecvBuff[64+1],4))
            {
				Lib_Des24(&g_DownRecvBuff[8+1],  &abyTemp[3],  abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[16+1], &abyTemp[11], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[24+1], &abyTemp[19], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[32+1], &abyTemp[27], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[40+1], &abyTemp[35], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[48+1], &abyTemp[43], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[56+1], &abyTemp[51], abyAuthKey, DECRYPT);
                dwRet = s_KM_SaveAppKey(abyTemp);
                
            }
            else
            {
                dwRet = 1;
            }

            abyTemp[0] = (BYTE)(dwRet&0xff);
			WriteLoadLog(byKeyType, abyTemp[0]);

            Crc16SendPacket(LOAD_APP_KEY_RESPOND, abyTemp, 1);
            iStep = 0;
            if (KM_SUCCESS != dwRet)
            {  
                return dwRet;
            }
            iKeyNumCnt++;
            if(iKeyNumCnt> KM_SUPPORT_MAX_KEYNUM)
            {
               /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 24/2);
                Lib_LcdPrintfCE(" 下载密钥数超限 ", "Key Numbers Over"); */
				Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0,8*2,0x00,"Key Numbers Over");
                return KM_SUCCESS;
            }
            
            break;
		case LOAD_APP_DUKPTKEY_REQUEST:

			/*
            共80字节:1字节类型+1字节KSN长度+1字节DUKPT索引号+1字节BDK长度
			         +24字节BDK+16字节KSN+32字节的应用名+4字节MAC
            */
            if (80 != wPacketLen)
            {
                break;
            }
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 16/2);
            Lib_LcdPrintfCE(" 下载DUKPT密钥 ", " Load Dukpt Key");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*1,0x00," Load Dukpt Key");

            byKeyType  = g_DownRecvBuff[5];
			byKsnLen   = g_DownRecvBuff[6];
            byKeyIndex = g_DownRecvBuff[7]; 
            byKeyLen   = g_DownRecvBuff[8];
            abyTemp[0] = byKeyType;
			abyTemp[1] = byKsnLen;
            abyTemp[2] = byKeyIndex;
            abyTemp[3] = byKeyLen;
            memset(macdata, 0, sizeof(macdata));
            memcpy(&macdata[0], &g_DownRecvBuff[5], 4+32+24+16);
            //memcpy(&macdata[4], &abyTemp[3+1], 32+24+16);

			Des3_24Mac(abyTMacKey,macdata,4+32+24+16); 
            if(0 == memcmp(macdata,&g_DownRecvBuff[79+1+1],4))
            {
				Lib_Des24(&g_DownRecvBuff[8+1],  &abyTemp[3+1],  abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[16+1], &abyTemp[11+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[24+1], &abyTemp[19+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[32+1], &abyTemp[27+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[40+1], &abyTemp[35+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[48+1], &abyTemp[43+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[56+1], &abyTemp[51+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[64+1], &abyTemp[59+1], abyAuthKey, DECRYPT);
				Lib_Des24(&g_DownRecvBuff[72+1], &abyTemp[67+1], abyAuthKey, DECRYPT);
                dwRet = s_KM_SaveDukptKey(abyTemp);
            }
            else
            {
                dwRet = 1;
            }
			
            abyTemp[0] = (BYTE)(dwRet&0xff);
			WriteLoadLog(DUKPTK_TYPE, abyTemp[0]);
            Crc16SendPacket(LOAD_APP_DUKPTKEY_RESPOND, abyTemp, 1);
            iStep = 0;
            if (KM_SUCCESS != dwRet)
            {  
                return dwRet;
            }
			iKeyNumCnt++;            
            if(iKeyNumCnt> KM_SUPPORT_MAX_KEYNUM)
            {
               /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
                Lib_LcdGotoxy(0, 24/2);
                Lib_LcdPrintfCE(" 下载密钥数超限 ", "Key Numbers Over"); */
				Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
				Lib_LcdPrintxy(0,8*2,0x00,"Key Numbers Over");
                return KM_SUCCESS;
            }
            
            break;
        case ALL_FINISHED:
           /* Lib_LcdClrLine(16/2, LCD_HIGH_MINI-1);
            Lib_LcdGotoxy(0, 24/2);
            Lib_LcdPrintfCE(" 下载完成   ", " Download OK");*/
			Lib_LcdClrLine(8*1, LCD_HIGH_MINI-1);
			Lib_LcdPrintxy(0,8*2,0x00," Download OK");
            abyTemp[0] = 0;
            Crc16SendPacket(ALL_FINISHED, abyTemp, 1);
 			WriteLog("Download OK\n",1, 0xff);
           return KM_SUCCESS;
        default:
            break;
        }
    }
}
