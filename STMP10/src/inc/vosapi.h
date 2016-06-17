/*******************************************************/
/* VPP202API.h                                        */
/* Define the Application Program Interface            */
/* for view100 EMV test terminals                      */
/*******************************************************/

#ifndef  _VOS_API_H
#define  _VOS_API_H

//Standard header files
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>

//TODO:  Add other header files

#ifndef BYTE
#define BYTE   unsigned char
#endif
#ifndef WORD
#define WORD   unsigned short
#endif
#ifndef DWORD
#define DWORD  unsigned long
#endif
#ifndef uchar
#define uchar  unsigned char
#endif
#ifndef uint
#define uint   unsigned int
#endif
#ifndef ulong
#define ulong  unsigned long
#endif
#ifndef ushort
#define ushort unsigned short
#endif

#define BPK_DEBUG
#define DLL_NOT_SWI
//#undef   DLL_NOT_SWI

/*defined by Makefile*/
//#define SENSOR_ENABLE
//#undef  SENSOR_ENABLE

#define VOSSIGN_ENABLE
#undef  VOSSIGN_ENABLE

#define VOSTRANSKEY_INIT
//#undef  VOSTRANSKEY_INIT

#define SELFCHECK_ENABLE
#undef  SELFCHECK_ENABLE

#define DEBUG_ADJUST_REGS
#undef DEBUG_ADJUST_REGS

#define  PRODUCT_TYPE          "V10S"
#define  PRODUCT_TYPE_LEN      7

#define MMKEY_TYPE        0x00
#define PIN_MKEY_TYPE     0x01  //用于加密PINKEY的MK
#define PINKEY_TYPE       0x02
#define MACKEY_TYPE       0x03
#define MAC_MKEY_TYPE     0x04  //用于加密MACKEY的MK 
#define FKEY_TYPE         0x05  //脱机pin传输上的加密
#define PTK_TYPE          0x0A
#define TMACK_TYPE        0x0B


#define AUTHPIN_MK_TYPE   0x11
#define AUTHPINK_TYPE     0x12
#define AUTHMACK_TYPE     0x13
#define AUTHMAC_MK_TYPE   0x14
#define AUTHOFFPINK_TYPE  0x15

#define AUTHAPP_TYPE      0x20   //用于APP签名认证
#define AUTHVOS_TYPE      0x30   //
#define AUTHLCD_TYPE      0x40   //用于LCD应用认证
#define AUTHMDK_TYPE      0x50

#define SK_MACK_TYPE      0x60  //用于计算PINK或MACK的MAC

#define DUKPTK_TYPE       0x80

/*
16M sdram :   c0000000-c0800000  : vos     (8m)
              20800000-20c00000  : app     (4m)
              20c00000-20e00000  : app man (2m)
              20e00000-20f00000  : reserve (1m)
              20f00000-21000000  : font and dll (1m)

8M flash  :   D0000000-D0800000  : vos +file (8m)

*/


#define  APP_APPMAN_FLAG_SPACE  0x20700008
#define  APP_RUN_SPACE          0x20500000
#define  APPMAN_RUN_SPACE       0x20400000
#define  FONTLIB_STARTADDR      0xC0310000

#define VOS_SAVE_ADDR    0xD0010000
#define VOS_MINI_LEN     102400
#define VOS_TAIL_LEN     30

//=================================================
//               System functions
//==================================================
int    s_SystemInit(void);
int    s_SystemTestSelf(void);
void   s_DelayUs(unsigned long usec);

int    Lib_AppInit(void);
void   Lib_DelayMs(int ms);
int    Lib_GetLastError(void);
//for speak
void   s_BeepInit(void);
void   Lib_Beep(void);
void   Lib_Beef(BYTE mode,int DlyTimeMs);

//for real time
#define TIME_FORMAT_ERR         (-900)
#define TIME_YEAR_ERR           (-901)
#define TIME_MONTH_ERR          (-902)
#define TIME_DAY_ERR            (-903)
#define TIME_HOUR_ERR           (-904)
#define TIME_MINUTE_ERR         (-905)
#define TIME_SECOND_ERR         (-906)
#define TIME_WEEK_ERR           (-907)
#define TIME_SET_ERR            (-908)
#define TIME_GET_ERR            (-909)
#define TIME_RAMADDR_OVER       (-910)
#define TIME_RAMLEN_OVER        (-911)


void   s_InitRtc(void);
int    Lib_SetDateTime(uchar *datetime);
int    Lib_GetDateTime(uchar *datetime);

void   s_TimeInit(void);
//int    s_ReadDS1302(uchar start_addr,uchar read_len,uchar *read_data);
//int    s_WriteDS1302(uchar start_addr,uchar write_len,uchar *write_data);

//for timer event
void   s_TimerInit(void);
int    SetTimerEvent(ushort uElapse10ms, void (*TimerFunc)(void));
int    s_SetTimerEvent(ushort uElapse10ms, void (*TimerFunc)(void));
void   KillTimerEvent(int handle);
void   s_KillTimerEvent(int handle);
void   s_SetTimer(uchar TimerNo, ulong count);
ulong  s_CheckTimer(uchar TimerNo);


void   Lib_SetTimer(uchar TimerNo, ushort Cnt100ms);
ushort Lib_CheckTimer(uchar TimerNo);
void   Lib_StopTimer(uchar TimerNo);

//for all hardware and software version
int    Lib_ReadSN(BYTE *SerialNo);
int	   Lib_WriteSN(BYTE *SerialNo);

int    Lib_EraseSN(void);
//uchar  GetMagHardVer(void);  //AN1
//uchar  GetMainHardVer(void);//AN0
int    Lib_ReadVerInfo(uchar *VerInfo);

void   Lib_Reboot(void);
void   Lib_GetRand(uchar *out_rand ,uchar  out_len);

#define OFF_BASE_ERR         (-800)

// int    Lib_OnBase(void);
uint   Lib_GetSysClk(void);
// int    Lib_GetBatteryVolt(void);

#define  LCD_LED1       ((unsigned short)1<<0)
#define  LCD_LED2       ((unsigned short)1<<1)
#define  LCD_LED3       ((unsigned short)1<<2)
#define  LCD_LED4       ((unsigned short)1<<3)
void Lib_LedInit(void);
void Lib_LedOn(WORD mask);
void Lib_LedOff(WORD mask);
WORD Lib_LedState(void);

//=================================================
//               KeyBoard functions
//==================================================
#define KEYF1            0x01
#define KEYF2            0x02
#define KEYF3            0x03
#define KEYF4            0x04
#define KEYF5            0x09
#define KEYF6            0x0a

#define KEY1             0x31
#define KEY2             0x32
#define KEY3             0x33
#define KEY4             0x34
#define KEY5             0x35
#define KEY6             0x36
#define KEY7             0x37
#define KEY8             0x38
#define KEY9             0x39
#define KEY0             0x30
#define KEYCLEAR         0x08
#define KEYALPHA         0x07
#define KEYUP            KEYF1//0x05
#define KEYDOWN          KEYF2//0x06
#define KEYFN            0x15
#define KEYMENU          0x14
#define KEYENTER         0x0d
#define KEYCANCEL        0x1b
#define KEYBACKSPACE     0x1c
#define KEYPRINTERUP     0x1d
#define NOKEY            0xff

#define KEYBANK          0x1e
#define KEYTEL           0x1f
#define KEYREDAIL        0x20
#define KEYMIANTI        0x21
#define KEYHOTLINE       0x22
       

#define KB_NoKey_Err        (-1000)
#define KB_InputPara_Err    (-1010)
#define KB_InputTimeOut_Err (-1011)
#define KB_UserCancel_Err   (-1012)
#define KB_UserEnter_Exit   (-1013)

int    s_KbInit(void);
uchar  Lib_KbGetCh(void);
uchar  Lib_KbUnGetCh(void);
int    Lib_KbCheck(void);
void   Lib_KbFlush(void);
int    Lib_KbSound(uchar mode,ushort DlyTimeMs);
int    Lib_KbMute(uchar mode);
int    Lib_KbGetStr(uchar *str,uchar minlen, uchar maxlen,uchar mode,ushort timeoutsec);
int    Lib_KbGetHzStr(uchar *outstr, uchar max, ushort TimeOut);

//=================================================
//               LCD Display functions
//==================================================
#define ASCII       0x00
#define CFONT       0x01
#define REVER       0x80

#define GRAY_MAX_VALUE      60
#define GRAY_MIN_VALUE      40
#define DEFAULT_GRAY_BASE   40 //52  

#define GRAY_MAX_LEVEL	20
#define GRAY_MIN_LEVEL	1
#define DEFAULT_GRAY_LEVEL	 10


#define  LCD_SetFont_Err   (-1500)
#define  LCD_GetFont_Err   (-1501)


#ifndef RGB_TYPE
#define RGB_TYPE unsigned short
#endif 

#ifndef ASCII_FONT
#define ASCII_FONT		0
#endif

#ifndef GB2312_FONT
#define GB2312_FONT		1
#endif

#define X_MULTI			1
#define Y_MULTI			1  
 
#define DEFAULT_ASCII_FONT_HEIGH	8
#define DEFAULT_EXT_FONT_HEIGH		16
 
#define  ICON_PHONE      1    // phone 电话
#define  ICON_SIGNAL     2    // wireless signal 信号
#define  ICON_PRINTER    3    // printer 打印机
#define  ICON_ICCARD     4    // smart card IC卡
#define  ICON_LOCK       5    // lock 锁
#define  ICON_SPEAKER    6    // speeker 扬声器
#define  ICON_UP         7    // up 向上
#define  ICON_DOWN       8    // down 向下
#define  CLOSEICON       0    // 关闭图标[针对所有图标]
#define  OPENICON        1    // 显示图标[针对打印机、IC卡、锁、电池、向上、向下]


#define LCD_WIDTH_MINI	122
#define LCD_HIGH_MINI	32

extern void DrawThreeLine(void);
unsigned long Lib_LCDVerValue(void);
int   s_LcdInit(void);
void  Lib_LcdCls(void);
void  Lib_LcdClrLine(BYTE startline, BYTE endline);
void  Lib_LcdSetBackLight(BYTE mode);
void  Lib_LcdSetGray(BYTE level);
void  Lib_LcdGotoxy(BYTE x, BYTE y);
int   Lib_LcdSetFont(BYTE AsciiFontHeight, BYTE ExtendFontHeight, BYTE Zoom);
int   Lib_LcdGetFont(BYTE *AsciiFontHeight, BYTE *ExtendFontHeight, BYTE *Zoom);
BYTE  Lib_LcdSetAttr(BYTE attr);
//void  Lib_LcdSetFontColor(RGB_TYPE FontColor, RGB_TYPE BackGroundColor);
int   Lib_Lcdprintf(char *fmt,...);
void  Lib_LcdPrintxy(BYTE x, BYTE y, BYTE mode, char *str,...);
void  Lib_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color);
void  Lib_LcdDrawLogo(BYTE *pDataBuffer);
void  Lib_LcdDrawBox(BYTE x1,BYTE y1,BYTE x2,BYTE y2);
int   Lib_LcdRestore(BYTE mode);
//void Lib_LcdSetIcon(int byIconNo, int byMode);

void  Lib_LcdGetSize(BYTE * x, BYTE *y);
void  Lib_LcdDrawLine(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE byColor);
BYTE  Lib_LcdGetSelectItem(BYTE *pbyItemString, BYTE byItemNumber, BYTE byExitMode);
BYTE  Lib_LcdGetSelectItemCE(BYTE *pbyChnItemString, BYTE *pbyEngItemString,
                     BYTE byItemNumber, BYTE byExitMode);
void  Lib_LcdPrintfCE(char *pCHN , char *pEN);
void  Lib_LcdPrintxyCE(BYTE col, BYTE row, BYTE mode, char *pCHN , char *pEN);
void  Lib_LcdPrintxyExtCE(BYTE col, BYTE row, BYTE mode, char * pCHN, char * pEN, int iPara);
BYTE  Lib_LcdShowItem(BYTE *pbyItemString, BYTE byItemNumber, BYTE byShowPage);
//=================================================
//               sign pad functions
//==================================================
int Lib_PadOpen(void); 
int Lib_PadClose(void); 
int Lib_PadStatus(void); 
int Lib_PadSign(unsigned char * pcode,unsigned char TimeOut); 
int Lib_PadRead(unsigned char * pbuf); 
int Lib_Padcalibration(void);

//extern calibration cal;



//========================================
//     Encrypt and decrypt functions
//=========================================
#define ENCRYPT 1
#define DECRYPT 0

void  s_DesInit(void);
void  Lib_Des(uchar *input, uchar *output,uchar *deskey, int mode);
void  Lib_DES(uchar *dat,uchar *key,int mode);
void  Lib_DES3_16(uchar *dat,uchar *key,int mode);
void  Lib_DES3_24(uchar *dat,uchar *key,int mode);
void  Lib_Rsa(uchar* Modul,uint ModulLen,uchar* Exp,uint ExpLen,uchar* DataIn,uchar* DataOut);
void  Lib_Hash(uchar* DataIn, uint DataInLen, uchar* DataOut);
//===========================================================
// Defined for swapping data between terminal and smart card
//===========================================================

//#pragma pack(1)
//如果PACK（1）表1字节对齐，如果为（2）则2字节对齐

typedef struct //__attribute__ ((__packed__))
{
     uchar  Command[4];
     ushort Lc;
     uchar  DataIn[512];
     ushort Le;
}APDU_SEND;

//#pragma pack()

typedef struct //__attribute__ ((__packed__))
{
     ushort LenOut;
     uchar  DataOut[512];
     uchar  SWA;
     uchar  SWB;
}APDU_RESP;


int   Lib_IccCommand(uchar slot,APDU_SEND * ApduSend,APDU_RESP * ApduResp);
int   Lib_IccCheck(uchar slot);
int   Lib_IccOpen(uchar slot,uchar VCC_Mode,uchar *ATR);
int   Lib_IccClose(uchar slot);
int   Lib_IccSetAutoResp(uchar slot,uchar autoresp);
int   Lib_IccPPS(uchar slot,uchar *pps);
int   s_IccInit(void);


//=============================================
//     MagCard Reader Functions
//=============================================

#define  MCR_NOTSWIPED  (-3000)
#define  MCR_CMDSENDERR (-3001)
#define  MCR_CMDRECVERR (-3002)


int   s_McrInit(void);
void  Lib_McrOpen(void);
void  Lib_McrClose(void);
void  Lib_McrReset(void);
int   Lib_McrCheck(void);
int   Lib_McrRead(uchar *track1, uchar *track2, uchar *track3);

//=============================================
//     Asynchronism communication functions
//
//=============================================
#define VCOM1       1
#define VCOM2       2 
#define VCOM_DEBUG  0


#define RS232A     VCOM1
#define RS232B     VCOM2
#define COM1       VCOM1
#define COM2       VCOM2 
//#define COM_DEBUG  VCOM_DEBUG 
#define  COMM_MAX   3

#define  COM_INVALID_PARA     (-6501)
#define  COM_NOT_OPEN         (-6502)
#define  COM_INVALID_PORTNO   (-6503)
#define  COM_RX_TIMEOUT       (-6504)
#define  COM_TX_TIMEOUT       (-6505)
#define  COM_TX_MEMOVER       (-6506)
#define  COM_NO_EMPTYPORT     (-6507)

void s_ComInit(void);
int  Lib_ComReset(uchar port);
int  Lib_ComWrite(uchar port,uchar *writebyte,int write_len);
int  Lib_ComRecvByte(uchar port,uchar *recv_byte,int waitms);
int  Lib_ComRecv(uchar port,uchar *recv_data,int max_len,int *recv_len,int waitms);
int  Lib_ComSendByte(uchar port,uchar send_byte);
int  Lib_ComSend(uchar port,uchar *send_data,int send_len);
int  Lib_ComClose(uchar port);
int  Lib_ComOpen(uchar port, char *ComPara); 
//===========================================
//        Defined for file system
//
//============================================
#define   FILE_EXIST               (-8001)
#define   FILE_NOTEXIST            (-8002)
#define   FILE_MEM_OVERFLOW        (-8003)
#define   FILE_TOO_MANY_FILES      (-8004)
#define   FILE_INVALID_HANDLE      (-8005)
#define   FILE_INVALID_MODE        (-8006)
#define   FILE_NO_FILESYS          (-8007)
#define   FILE_NOT_OPENED          (-8008)
#define   FILE_OPENED              (-8009)
#define   FILE_END_OVERFLOW        (-8010)
#define   FILE_TOP_OVERFLOW        (-8011)
#define   FILE_NO_PERMISSION       (-8012)
#define   FILE_FS_CORRUPT          (-8013)
#define   FILE_INVALID_PARA        (-8014)
#define   FILE_WRITE_ERR           (-8015)
#define   FILE_READ_ERR            (-8016)
#define   FILE_NO_MATCH_RECORD     (-8017)

#define   O_RDWR                   0x01
#define   O_CREATE                 0x02

#define   FILE_SEEK_CUR            0
#define   FILE_SEEK_SET            1
#define   FILE_SEEK_END            2


#define   FLASH_CMD_DELAY          80
#define   WRITE_FLASH_CMD_DELAY    10

typedef struct //__attribute__ ((__packed__))
{
     uchar  fid;
     uchar  attr;
     uchar  type;
     char   name[17];
     ulong  length;
} FILE_INFO;

#define APP_DOWNLOAD_OK_FLAG  "DOWNLOADOK"

extern int errno;

//File operation functions
void s_FileInit(void);
int  Lib_FileOpen(char *filename, uchar mode);
int  Lib_FileRead(int fid, uchar *dat, int len);
int  Lib_FileWrite(int fid, uchar *dat, int len);
int  Lib_FileClose(int fid);
int  Lib_FileSeek(int fid, long offset, uchar fromwhere);
long Lib_FileSize(char *filename);
long Lib_FileFreeSize(void);
int  Lib_FileTruncate(int fid,long len);
int  Lib_FileExist(char *filename);
int  Lib_FileInfo(FILE_INFO* finfo);
int  Lib_FileExOpen(char *filename, uchar mode,uchar* attr);
int  Lib_FileRemove(const char *filename);
int  Lib_FileGetEnv(char *name, uchar *value);
int  Lib_FilePutEnv(char *name, uchar *value);

int  s_open(char *filename, uchar mode,uchar* attr);
int  s_remove(char *filename, unsigned char *attr);  

//========================================================
//     MultiApplication functions,called by AppManager.
//
//==========================================================
typedef struct //__attribute__ ((__packed__))
{
     uchar Name[32];
     uchar AID[16];
     uchar Version[16];
     uchar Provider[32];
     uchar Descript[64];
     uchar LoadTime[14];
     ulong MainEntry;
     ulong EventEntry;
     uchar Num;
     uchar Reserve[73];
}APP_MSG;  //

#define MAGCARD_MSG   0x01
#define ICCARD_MSG    0x02
#define KEYBOARD_MSG  0x03
#define USER_MSG      0x04

typedef struct //__attribute__ ((__packed__))
{
     uchar RetCode;
     uchar track1[256];
     uchar track2[256];
     uchar track3[256];
}SET_MAGCARD;

typedef struct //__attribute__ ((__packed__))
{
     int MsgType;           //MAGCARD_MSG,ICCARD_MSG,KEYBOARD_MSG,USER_MSG
     SET_MAGCARD MagMsg;    //MAGCARD_MSG
     uchar KeyValue;        //ICCARD_MSG
     uchar IccSlot;         //KEYBOARD_MSG
     void *UserMsg;         //USER_MSG
}SET_EVENT_MSG;

int Lib_AppReadInfo(uchar AppNo, APP_MSG* ai);
int Lib_AppReadState(uchar AppNo);
int Lib_AppSetActive(uchar AppNo, uchar flag);
int Lib_AppRun(uchar AppNo);
int Lib_AppRunEvent(uchar AppNo, SET_EVENT_MSG *msg);  // ????????


//============================================================
//       USB Host and Device Operation Functions
//============================================================
#define  USBD     0
#define  USBH     1

int  UsbDeviceTest(void);
int  s_UsbDeviceCheck(void);
int  s_InitUsbDevice(void);
int  Usb_Open(uchar port);
int  Usb_Close(uchar port);
int  Usb_Reset(uchar port);
int  Usb_Send(uchar port,uchar *send_data, int send_len);
int  Usb_SendByte(uchar port,uchar send_byte);
int  Usb_Recv(uchar port,uchar *recv_data,int max_len, int *recv_len,int  waitms);
int  Usb_RecvByte(uchar port,uchar *recv_byte,int waitms);


//========================================================
//                   PED AND PCI API
//==========================================================

typedef struct
{
     unsigned int  modlen;           //PIN加密公钥模数长
     unsigned char mod[256];        // PIN加密公钥模数
     unsigned int  explen;            // PIN加密公钥指数长
     unsigned char exp[4];           // PIN加密公钥指数
     unsigned char iccrandomlen;     //从卡行取得的随机数长
     unsigned char iccrandom[8];     //从卡行取得的随机数
     unsigned int  termrandomlen;    //从终端应用取得的填充数长
     unsigned char termrandom[256];   //从终端应用取得的填充数
} RSA_PINKEY;


int  Lib_PciWritePIN_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode);
int  Lib_PciWriteMAC_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode);
int  Lib_PciWriteWORK_MKey(uchar key_no,uchar key_len,uchar *key_data,uchar *key_crc,uchar mode, uchar mkey_no);
int  Lib_PciWorkMKeyDes(uchar mkey_no, uchar mkey_part, uchar *indata, uchar *outdata, uchar mode);


//int  Lib_PciWritePinKey(uchar key_no,uchar key_len,uchar *key_data, uchar mode, uchar mkey_no);
//int  Lib_PciWriteMacKey(uchar key_no,uchar key_len,uchar *key_data, uchar mode, uchar mkey_no); 
int  Lib_PciWritePinKey(uchar key_no,uchar key_len,uchar *key_data,uchar *key_crc,uchar mode, uchar mkey_no);
int  Lib_PciWriteMacKey(uchar key_no,uchar key_len,uchar *key_data,uchar *key_crc,uchar mode, uchar mkey_no);
int  Lib_PciWriteDesKey(uchar key_no,uchar key_len,uchar *key_data,uchar mode);
int  Lib_PciWriteWorkKey(uchar key_no,uchar key_len,uchar *key_data, uchar *key_crc,uchar mode, uchar mkey_no);


int  Lib_PciDerivePinKey(uchar mkey_n,uchar pinkey_n1,uchar pinkey_n2,uchar mode);
int  Lib_PciDeriveMacKey(uchar mkey_n,uchar mackey_n1,uchar mackey_n2,uchar mode);
 
int  Lib_PciGetPin(uchar pinkey_n,uchar min_len,uchar max_len,uchar *card_no,uchar mode,uchar *pin_block,ushort waittime_sec);
int  Lib_PciGetPinAuto(uchar pinkey_n,uchar min_len,uchar max_len,uchar *card_no,uchar mode,uchar *pin_block,ushort waittime_sec);
int  Lib_PciGetPinFixK(BYTE fixkey_n,BYTE min_len,BYTE max_len,BYTE *card_no,BYTE mode,BYTE *pin_block,ushort waittime_sec);
int  Lib_PciGetMac(uchar mackey_n,ushort inlen,uchar *indata,uchar *macout,uchar mode);

int  Lib_PciGetRnd (uchar *rnd8);
int  Lib_PciAccessAuth(uchar *auth_data,uchar mode);
int  Lib_PciGetOfflinePin(BYTE min_len,BYTE max_len,BYTE *encpin,unsigned short waittime_sec);
int  Lib_PciOffLineEncPin(RSA_PINKEY *rsa_pinkey,uchar min,uchar max,uchar *encpin,ushort waittime_sec);
int  Lib_PciOffLinePlainPin(uchar icc_slot,uchar min,uchar max,uchar *icc_command,uchar *icc_resp,ushort waittime_sec);
int  Lib_PciGetPinDukpt(uchar key_n,uchar min_len,uchar max_len,uchar *card_no,uchar mode,uchar *pin_block,ushort waittime_sec,uchar *out_ksn);
int  Lib_PciGetMacDukpt(uchar key_n,ushort inlen,uchar *indata,uchar *macout,uchar mode,uchar *out_ksn);
 
int  Lib_PciAuthForNumberKey(uchar *authdata);

int  s_PciInit(void);

void vTwoOne(uchar *in, ushort in_len, uchar *out);
int  s_GetLockState(void);

int  s_ReadMDK(uchar *mdk);
int  s_WriteMDK(uchar *mdk); 

extern BYTE g_byCurAppNum;

#define PCI_Locked_Err        (-7000)
#define PCI_KeyType_Err       (-7001)
#define PCI_KeyLrc_Err        (-7002)
#define PCI_KeyNo_Err         (-7003)
#define PCI_KeyLen_Err        (-7004)
#define PCI_KeyMode_Err       (-7005) 
#define PCI_InputLen_Err      (-7006)
#define PCI_InputCancel_Err   (-7007)
#define PCI_InputNotMatch_Err (-7008)
#define PCI_InputTimeOut_Err  (-7009)
#define PCI_CallTimeInte_Err  (-7010) 
#define PCI_NoKey_Err         (-7011)
#define PCI_WriteKey_Err      (-7012)
#define PCI_ReadKey_Err       (-7013)
#define PCI_RsaKeyHash_Err    (-7014)
#define PCI_DataLen_Err       (-7015) 
#define PCI_NoInput_Err       (-7016)
#define PCI_AppNumOver_Err    (-7017) 
#define PCI_ReadMMK_Err       (-7020)
#define PCI_WriteMMK_Err      (-7021) 
#define PCI_Auth_Err          (-7030)
#define PCI_RsaKey_Err        (-7031) 
#define PCI_AuthTimes_Err     (-7032) 
#define PCI_KeySame_Err       (-7040)


#define DUKPT_OK                  0
#define DUKPT_NoKey               (-7050)
#define DUKPT_CounterOverFlow     (-7051)
#define DUKPT_NoEmptyList         (-7052)
#define DUKPT_InvalidAppNo        (-7053)
#define DUKPT_InvalidKeyID        (-7054)
#define DUKPT_InvalidFutureKeyID  (-7055)
#define DUKPT_InvalidCrc          (-7056)
#define DUKPT_InvalidBDK          (-7057)
#define DUKPT_InvalidKSN          (-7058)
#define DUKPT_InvalidMode         (-7059)
#define DUKPT_NotFound            (-7060)



//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
#define PICC_OK                (0)
#define PICC_ChipIDErr         (-3500)
#define PICC_OpenErr           (-3501)
#define PICC_NotOpen           (-3502)
#define PICC_ParameterErr      (-3503)
#define PICC_TxTimerOut        (-3504)
#define PICC_RxTimerOut        (-3505)
#define PICC_RxDataOver        (-3506)
#define PICC_TypeAColl         (-3507)
#define PICC_FifoOver          (-3508)
#define PICC_CRCErr            (-3509)
#define PICC_SOFErr            (-3510)
#define PICC_ParityErr         (-3511)
#define PICC_KeyFormatErr      (-3512)

#define PICC_RequestErr        (-3513)
#define PICC_AntiCollErr       (-3514)
#define PICC_UidCRCErr         (-3515)
#define PICC_SelectErr         (-3516)
#define PICC_RatsErr           (-3517)
#define PICC_AttribErr         (-3518)
#define PICC_HaltErr           (-3519)
#define PICC_OperateErr        (-3520)
#define PICC_WriteBlockErr     (-3521)
#define PICC_ReadBlockErr      (-3522)
#define PICC_AuthErr           (-3523)
#define PICC_ApduErr           (-3524)
#define PICC_HaveCard          (-3525)   //有卡
#define PICC_Collision         (-3526) //多卡
#define PICC_CardTyepErr       (-3527) //A 卡发送RATS 
#define PICC_CardStateErr      (-3528)



int  s_PiccInit(void);
int  Lib_PiccOpen(void);
int  Lib_PiccClose(void);
int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo);
int  Lib_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp);
int  Lib_PiccM1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo);
int  Lib_PiccM1ReadBlock(uchar BlkNo,uchar *BlkValue);
int  Lib_PiccM1WriteBlock(uchar BlkNo,uchar *BlkValue);
int  Lib_PiccM1Operate(uchar Type,uchar BlkNo,uchar *Value,uchar UpdateBlkNo);
void Lib_PiccHalt(void);
int  Lib_PiccReset(void);
int  Lib_PiccRemove(void);
//========================================================
//                   Local Download API
//========================================================== 
#define  DOWN_RECV_BUFFLEN      (16*1024)
#define  DOWN_SEND_BUFFLEN      (8*1024) 

#define  USBD_DOWN_MODE     0
#define  UART_DOWN_MODE     1

int  Down_RecvByte(uchar *recv_byte,int waitms);
int  Down_Recv(uchar *recv_data,int max_len,int *recv_len,int waitms);
int  Down_SendByte(uchar send_byte);
int  Down_Send(uchar *send_data,int send_len);
int  Down_Reset(void);
int  Down_Open(void);
int  Down_Close(void); 

#define FONT_NOEXISTERR        (-1600)
#define FONT_NOSUPPORTERR      (-1601)

//font api
void InitFontLib(void);
int  Lib_FontFileCheck(void);
int  Lib_FontGetCharSet(BYTE *bCharSet, BYTE *bNum);
int  Lib_FontGetHeight(BYTE bCharSet, BYTE *bHeight, BYTE * bHeightNum);
//int  Lib_FontGetStyle(VFONT_LIB_STRUCT *style_info); 
//usb api
void s_UdpInit(void);   

extern int mysprintf(char * buf, const char *fmt, ...);



//========================================================
//                   VOICE API
//==========================================================
#define VOICE_PLS_INPUT_PWD					1		//请输入密码
#define VOICE_PLS_INPUT_PWD_AGAIN		    2		//请重新输入密码
#define VOICE_PWD_ERR						3		//密码错误
#define VOICE_PLS_SWIPE_MAGCARD				4		//请刷卡
#define VOICE_PLS_INSERT_ICCARD				5		//请插入IC卡
#define VOICE_TRADE_SUCCESS					6		//交易成功
#define VOICE_TRADE_FAILER					7		//交易失败
#define VOICE_WELCOME						8		//欢迎光临
#define VOICE_THANKS_PATRONAGE				9		//谢谢惠顾
#define VOICE_THANKS_USE					10		//谢谢使用
#define VOICE_COMM_FAILER					11		//通信失败
#define VOICE_BALANCE_LACK					12		//余额不足

#define VOICE_ID_ERR					    (-920)	//声音编号错误

int SetVoice(uchar voiceID);

//========================================================
//                   USB Device API
//========================================================== 

#define USBD_SUCCESS          (0)
#define USB_INVALID_PORT      (-6300)
#define USBD_NOT_CONFIG       (-6301)
#define USB_SEND_OVERFLOW     (-6302)
#define USB_RECV_TIMEOUT      (-6303)
#define USB_RECV_OVERFLOW     (-6304)
#define USB_SEND_TIMEOUT      (-6305)
#define USB_NOT_CONNECT       (-6306) 

int Lib_UsbOpen(uchar port); 
int Lib_UsbClose(uchar port);
int Lib_UsbReset(uchar port);
int Lib_UsbSend(uchar port,uchar *send_data, int send_len);
int Lib_UsbSendByte(uchar port,uchar send_byte);
int Lib_UsbRecv(uchar port,uchar *recv_data,int max_len, int *recv_len,int  waitms);
int Lib_UsbRecvByte(uchar port,uchar *recv_byte,int waitms);



//========================================================
//                   Convert
//========================================================== 
#define SetTimer			Lib_SetTimer
#define CheckTimer			Lib_CheckTimer
#define DelayMs				Lib_DelayMs
#define Beep				Lib_Beep
#define Beef				Lib_Beef

#define Lcd_Cls				Lib_LcdCls
#define Lcd_ClrLine(start, end) \
		Lib_LcdClrLine((start)*8/2, (end)*8/2)
#define Lcd_SetBackLight	Lib_LcdSetBackLight
#define Lcd_SetGray			Lib_LcdSetGray
#define Lcd_Gotoxy(x, y) \
		Lib_LcdGotoxy((x)*8/2, (y)*8/2)
#define Lcd_SetFont			Lib_LcdSetFont
#define Lcd_GetFont			Lib_LcdGetFont
#define Lcd_SetAttr			Lib_LcdSetAttr
#define Lcd_printf			Lib_Lcdprintf
//#define Lcd_Printxy(x, y, mode, arg...)	\	
#if 0
#define Lcd_Printxy(x, y, mode, arg)	\		
		Lib_LcdPrintxy((x)*8/2, (y)*8/2, mode, arg)
#endif
                  
#define Lcd_DrawPlot		Lib_LcdDrawPlot
#define Lcd_DrawLogo		Lib_LcdDrawLogo

#define  Kb_GetKey			Lib_KbGetCh
#define  Kb_Hit				Lib_KbCheck
#define  Kb_Flush			Lib_KbFlush

#define Picc_Open			Lib_PiccOpen
#define Picc_Close			Lib_PiccClose
#define	Picc_Check			Lib_PiccCheck
#define	Picc_Command		Lib_PiccCommand
#define	Picc_Halt			Lib_PiccHalt
#define	Picc_Reset			Lib_PiccReset
#define	Picc_Remove			Lib_PiccRemove

                  //Address:0-(66*1024-1)  size:66k                  
int Lib_AppShareRead(ulong Address,uchar * buff,ulong len);                  
int Lib_AppShareWrite(ulong Address,uchar * buff,ulong len);
//Address:66*1024-101 - 66*1024 -1
int Lib_TscRead(ulong Address,uchar * buff,ulong len);                  
int Lib_TscWrite(ulong Address,uchar * buff,ulong len);

int Lib_sm1(unsigned char *input,unsigned int input_len,unsigned char *output,unsigned char *smkey,int mode);

int Sm2_Init();
int Sm2_LoadPK(unsigned char *sPx,unsigned char *sPy);
int Sm2_Load_PrivateKey(unsigned char *d);
int Sm2_GetZ(unsigned char *id, int id_len, unsigned char *Z);
int Sm2_GetE(const unsigned char *id, unsigned int idLen, 
   const unsigned char *publicKey, const unsigned char *msg, unsigned int msgLen, 
   unsigned char *e);
//int Gm_Sm2Verify(unsigned char *user_id, int userid_len,const 
//unsigned char * public_key,unsigned char * signed_data, unsigned char *msg, int msg_len);
int Sm2_Verify(unsigned char *user_id, int userid_len,const 
  unsigned char * public_key,unsigned char * signed_data, unsigned char *msg, int msg_len);
int Sm2_ExportPK(unsigned char bPara,unsigned char *sOutput);
int Lib_SM2GetSign(unsigned char *sOutputE,unsigned char *sOutputR,unsigned char *sOutputS);
int Lib_SM2LoadIDSign(unsigned char bInputLen,unsigned char *sInputID);
int Lib_SM2InputEandGetSign(unsigned char *sInputE,unsigned char *sOutputR,unsigned char *sOutputS);
int Lib_SM2InputSign(unsigned char bInputLen,unsigned char *sInput);
int Lib_SM2LoadIDVerify(unsigned char bInputLen,unsigned char *sInputID);
int Gm_Sm2Sign(unsigned char *user_id, int userid_len, const unsigned char *public_key,
    unsigned char *private_Key, unsigned char *sign, unsigned char *msg,int msg_len);

int Gm_Sm3( unsigned char *input, int ilen, unsigned char *output);
int Gm_Sm4(unsigned char *input,unsigned int input_len,unsigned char *output,unsigned char *smkey,int mode);

int SMGetRand(unsigned int nLc,unsigned char *sRand);

#define Sm3 Gm_Sm3
#define Sm4 Gm_Sm4

int Hid_Send(unsigned char *pucData, unsigned int ulSendLen);
int Hid_Rec(unsigned char *pucData);
unsigned int Hid_GetStatus(void);


#endif
