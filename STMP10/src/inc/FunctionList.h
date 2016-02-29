#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

//#include "..\vos\app\SystemPara.h"
#ifndef  _FUNCTION_LIST_H
#define  _FUNCTION_LIST_H

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef BYTE
#define BYTE   unsigned char
#endif

#ifndef WORD
#define WORD   unsigned short
#endif

#ifndef DWORD
#define DWORD  unsigned long
#endif

#ifndef UINT
#define UINT  unsigned int
#endif

extern void  AM_ChangePassword(void);
extern void  AM_InitAll(void);
extern void  ByteToDword(BYTE *pbyIn, int iLen, DWORD *pdwOut);
extern int   CheckHandShack();
extern int   CheckVosIsFirstRun(void);
extern void  ClearIDCache(void);
extern void  ConvertBcdTimeToStr(const BYTE *pbyInBcdTime, BYTE *pbyTimeStr);
extern int   Decompress(BYTE *srcbuf, BYTE *dstbuf, DWORD srclen, DWORD *dstlen);
extern void  DrawThreeLine(void);
extern void  GetAppEntry(BYTE *AppHead, DWORD *MainEntry, DWORD *EventEntry, DWORD *AppMsgEntry);
extern int   GetCharDotMatrix(const BYTE *str, BYTE byCharSet, BYTE byFontHeight,
                     BYTE *pbyCharDotMatrix);
extern int   GetLcdCharDotMatrix(const BYTE *str, BYTE byCharSet, BYTE byFontHeight, BYTE *pbyCharDotMatrix, DWORD dwZoom);
extern void  Lib_CreateFileSys(void);
extern void  Lib_Des16(BYTE *input,BYTE *output,BYTE *deskey,int mode);
extern int   Lib_WriteSN(BYTE *SerialNo);
extern void  LoadApp(void);
extern DWORD KM_LoadKey(void);
extern DWORD ManageLogin(void);
extern void  SendLogStart(void);
extern void WriteLog(char *str, int iWriteTimeFlag, int iAminister);
extern void  memcpy_byte(BYTE * startaddr, BYTE *buf,int len);
extern void  MyLocalDownload();
extern int   myvsprintf(char *outbuf, const char *fmt, va_list ap);
extern void  ProcFunc(void);
extern void  QuickCrc32(BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[4]);
extern int   s_AuthGetRand(BYTE *rnd8);
extern int   s_AuthKeyVerify(BYTE key_type,BYTE key_no,BYTE *authdata);
extern int   s_AuthLcd(void);
extern void  s_Beep(int mode);
extern void  s_DelayMs1(int ms);
extern void  s_EraseSector(DWORD Addr);
extern void  s_GPIO_Int(void);
extern long  s_filesize(char *filename, BYTE *attr);
extern int   s_GetBackupDotMatrix(const BYTE *str, BYTE byFontHeight, BYTE *pbyCharDotMatrix);
extern BYTE  s_getkey(int port);
extern int   s_GetRand(BYTE *pbyOut);
extern void  s_InitDll(void);
extern int   s_Lcdprintf(char *str);
extern void  s_Lib_LcdPrintxy(BYTE x, BYTE y, BYTE mode, char *str);
extern int   s_PciClearAppKey(uchar app_no);
extern int   s_PciInitMMK(void);
extern int   s_PciMMKCheck(void);
extern int   s_PciReadMMK(int iIndex, DWORD *pdwOut);
extern int   s_PciWriteMMK(int iIndex, DWORD *pdwIn);
extern int   s_PowerSet(int mode);
extern int   s_ReadAuthKey(BYTE *app_name, BYTE key_type, BYTE key_no, BYTE *key_len, BYTE *key_data);
//extern int   s_RecreateSysKeyFile(void);
extern int   s_rtc_write_reg(UINT which, UINT value );
extern int   s_TestAll(void);
extern int   s_UartPrintSub(char *str);
extern int   s_WaitForReleaseONOFF(void);
extern int   s_WriteAuthKey(BYTE *app_name,BYTE key_type,BYTE key_no,BYTE key_len,BYTE *key_data,BYTE key_lrc);
extern int  s_DukptLoadKey(uchar AppNo,uchar KeyId,uchar BdkLen,uchar KsnLen,uchar *BdkKsn);

extern int   s_WriteFlash(WORD *pwAddr, WORD *pwSrc, int wlen);
extern int   ShowSystemMenu(void);
extern void  SPF_CheckApp(void);
extern int   SPF_EraseAllAppInfo();
extern int   switch_to_user_mode(UINT code_addr);
extern int   SystemTest(void);
extern int   USIP_apm_reset_device(UINT devnum );
extern int   USIP_apm_set_device_mode(UINT devnum, UINT mode );
  
extern int   s_SignBootWithVSK(uchar *signdata,uchar *outmac);  
//extern int   s_CheckBootSign(void);
extern int   s_WriteBootSign(void);
extern int   s_EraseBootSigh(void);
extern int   s_PciClearAuthFlag(void);


extern int   s_CheckVosSign(void);
extern int   s_CheckSDRAM(void);
extern int   s_CheckAllKeys(void);
#endif

