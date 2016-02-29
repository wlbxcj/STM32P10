/*****************************************************/
/* Dll.h                                             */
/* Define Interface for DLL                          */
/* TIME:09/28/2006                                   */
/*****************************************************/

#ifndef _VOS_DLL_H
#define _VOS_DLL_H




#define DLL_PARAM_ADDR      0x20700010
#define DLL_FUN_ADDR        0x20700000


//Define function types
#define SYS_FUN             0x01
#define KB_FUN              0x02
#define LCD_FUN             0x03
#define ICC_FUN             0x04
#define MCR_FUN             0x05
#define PICC_FUN            0x06
#define COM_FUN             0x07
#define MDM_FUN             0x08
#define USB_FUN             0x09
#define FILE_FUN            0x0a
#define ENCRYPT_FUN         0x0b
#define PCI_FUN             0x0c
#define PRN_FUN             0x0d
#define APP_FUN             0x0e
#define FONT_FUN            0x0f
#define WLS_FUN             0x10
#define VOICE_FUN           0x12

//Define SYS_FUN function

#define BEEP                0x01
#define BEEF                0x02
#define GETTIME             0x03
#define SETTIME             0x04
#define SETLED              0x05
#define SETTIMER            0x06
#define CHECKTIMER          0x07
#define READSN              0x08
#define WRITESN             0x09
#define ERASESN             0x0a
#define DELAYMS             0x0b
#define GETLASTERROR        0x0c
#define SETTIMEREVENT       0x0d
#define KILLTIMEREVENT      0x0e
#define READVERINFO         0x0f
#define CHECKFIRST          0x10

#define STOPTIMER           0x20
#define REBOOT              0x21

#define GETSYSCLK           0x30
#define ONBASE              0x31

#define LEDINIT				0x40
#define LEDON				0x41
#define LEDOFF				0x42
#define LEDSTATE			0x43

//Define Wls function
#define WLSOPEN             0x01
#define WLSCLOSE            0x02
#define WLSIOCTL            0x10

//Define FILE_FUN function
#define FILEOPEN            0x01
#define FILEREAD            0x02
#define FILEWRITE           0x03
#define FILECLOSE           0x04
#define FILESEEK            0x05
#define FILEREMOVE          0x06
#define FILESIZE            0x07
#define FILEFREESIZE        0x08
#define FILETRUNCATE        0x09
#define FILEEXIST           0x0a
#define FILEINFO            0x0b
#define FILEEXOPEN          0x0c
#define FILEGETENV          0x0d
#define FILEPUTENV          0x0e


//usb device and host driver function
#define USBOPEN             0x01
#define USBCLOSE            0x02
#define USBRESET            0x03
#define USBSEND             0x04
#define USBSENDBYTE         0x05
#define USBRECV             0x06
#define USBRECVBYTE         0x07

//encrypt function
#define DES_F               0x01
#define HASH_F              0x02
#define RSA_F               0x03

#define GETRAND             0x10


#define ICCRESET            0x01
#define ICCCLOSE            0x02
#define ICCCOMMAND          0x03
#define ICCSETAUTORESP      0x04
#define ICCDETECT           0x05

//Define COM_FUN function
#define SGETKEY             0x01
#define SUARTPRINT          0x02
#define COMOPEN             0x03
#define COMRESET            0x04
#define COMCLOSE            0x05
#define COMRECV             0x06
#define COMRECVBYTE         0x07
#define COMSEND             0x08
#define COMSENDBYTE         0x09

#define COM_DEBUG_RESET_MDM 0x0a
#define COMCHECKSNED		0x0b


//Define KEYBOARD_FUN function
#define KBHIT               0x01
#define KBFLUSH             0x02
#define KBGETKEY            0x03
#define KBMUTE              0x04
#define KBSOUND             0x05
#define KBGETSTR            0x06
#define KBGETHZSTR          0x07

#define KBUNGETCH           0x10

//Define LCD_FUN function
#define LCDCLS              0x01
#define LCDCLRLINE          0x02
#define LCDSETGRAY          0x03
#define LCDSETBACKLIGHT     0x04
#define LCDGOTOXY           0x05
#define LCDSETFONT          0x06
#define LCDSETATTR          0x07
#define LCDPLOT             0x08
#define LCDPRINT            0x09
#define LCDDRLOGO           0x0a
#define LCDSETICON          0x0b
#define LCDFPUTC            0x0c
//#define PRINTF            0x0c
#define LCDDRLOGOXY         0x0d
#define LCDRESTORE          0x0e
#define LCDDRAWBOX          0x0f
#define LCDPRINTF           0x10

#define LCDSETFONTCOLOR     0x11

#define LCDGETSIZE          0x20
#define LCDDRAWLINE         0x21
#define LCDGETFONT          0x22

#define LCD_PRINTF_CE           0x30
#define LCD_PRINTXY_CE          0x31
#define LCD_PRINTXY_EXT_CE      0x32
#define LCD_GET_SELECT_ITEM_CE  0x33
#define LCD_GET_SELECT_ITEM     0x34


//Define MAG_FUN function
#define MCRRESET            0x01
#define MCROPEN             0x02
#define MCRCLOSE            0x03
#define MCRCHECK            0x04
#define MCRREAD             0x05
#define MCRGETANTIFLAG      0x06

//Define APP_FUN function
#define APPREADINFO         0x01
#define APPWRITEINFO        0x02
#define APPREADSTATE        0x03
#define APPSETACTIVE        0x04
#define APPRUN              0x05
#define APPRUNEVENT         0x06

#define APPGETNAME          0x07


#define PCIGETRND           0x01
#define PCIACCESSAUTH       0x02
#define PCIWRITEMKEY        0x03
#define PCIWRITEPINKEY      0x04
#define PCIWRITEMACKEY      0x05
#define PCIWRITEDESKEY      0x06
#define PCIDERIVEPINKEY     0x07
#define PCIDERIVEMACKEY     0x08
#define PCIDERIVEDESKEY     0x09
#define PCIGETPIN           0x0a
#define PCIGETMAC           0x0b
#define PCIDES              0x0c

#define PCIWRITEPINMKEY     0x0d
#define PCIWRITEMACMKEY     0x0e
#define PCIGETPINAUTO       0x0f


#define PCIOFFLINEPLAINPIN  0x10
#define PCIOFFLINEENCPIN    0x11
#define PCIAUTHFORNUMKEY    0x12
#define PCIGETOFFLINEPIN    0x13

#define PCIGETAUTHRND       0x20
#define PCIVERIFYAUTH       0x21

#define PCIGETPINDUKPT      0x30
#define PCIGETMACDUKPT      0x31 

#define PCIGETPINFIXK       0x40
#define PCIWRITEPINKEYFOREMV  0x50

#define PCIWRITEWORKMKEY	0x60
#define PCIWRITEWORKKEY		0x61
#define PCIWORKMKEYDES		0x62

//Define COMM_FUN function
#define MODEMDIAL           0x01
#define MODEMCHECK          0x02
#define MODEMTXD            0x03
#define MODEMRXD            0x04
#define ONHOOK              0x05
#define ASMODEMRXD          0x06
#define MODEMRESET          0x07
#define SMODEMINFO          0x08
#define MODEMEXCMD          0x09



//Define PRINT_FUN function
#define PRNSETFONT          0x01
#define PRNSPACESET         0x02
#define PRNLEFTINDENT       0x03
#define PRNSTEP             0x04
#define PRNSTR              0x05
#define PRNLOGO             0x06
#define PRNINIT             0x07
#define PRNSTART            0x08
#define PRNCHECK            0x09

#define PRNGETFONT          0x20
#define PRN_SET_GRAY        0x21
#define PRN_SET_SPEED       0x22



/*
#define PICCOPEN            0x01
#define PICCCLOSE           0x02
#define PICCCHECK           0x03
#define PICCCOMMAND         0x04
#define PICCHALT            0x05
#define PICCRESET           0x06
#define PICCREMOVE          0x07
#define PICCM1AUTH          0x10
#define PICCM1READ          0x11
#define PICCM1WRITE         0x12
#define PICCM1OPERATE       0x13*/

#define PICC_OPEN            0x01
#define PICC_CHECK           0x02
#define PICC_COMMAND         0x03
#define PICC_REMOVE          0x04
#define PICC_HALT            0x05
#define PICC_RESET           0x06
#define PICC_CLOSE           0x07
#define PICC_M1_AUTHORITY    0x08
#define PICC_M1_READ_BLOCK   0x09
#define PICC_M1_WRITE_BLOCK  0x0a
#define PICC_M1_OPERATE      0x0b


#define FONTFILECHECK       0x01
#define FONTGETCHARSET      0x02
#define FONTGETSTYLE        0x03
#define FONTGETHEIGHT       0x04
#define FONTGETDOTMATRIX    0x05

//Define VOICE_FUN function
#define SETVOICE            0x01


typedef struct //__attribute__ ((__packed__))
{
     int    FuncType;
     int    FuncNo;
     int    AppNum;

     void   *Addr1;
     void   *Addr2;

     char   char1;
     char   char2;
     char   *str1;
     char   *str2;

     short  short1;
     short  short2;
     short  *p_short1;
     short  *p_short2;

     int    int1;
     int    int2;
     int    int3;
     int    int4;
     int    *p_int1;
     int    *p_int2;

     long   long1;
     long   long2;
     long   *p_long1;
     long   *p_long2;

     unsigned char  u_char1;
     unsigned char  u_char2;
     unsigned char  u_char3;
     unsigned char  u_char4;
     unsigned char  *u_str1;
     unsigned char  *u_str2;
     unsigned char  *u_str3;
     unsigned char  *u_str4;

     unsigned short u_short1;
     unsigned short u_short2;
     unsigned short u_short3;
     unsigned short u_short4;
     unsigned short *up_short1;
     unsigned short *up_short2;
     unsigned short *up_short3;
     unsigned short *up_short4;

     unsigned int   u_int1;
     unsigned int   u_int2;
     unsigned int   *up_int1;
     unsigned int   *up_int2;

     unsigned long  u_long1;
     unsigned long  u_long2;
     unsigned long  *up_long1;
     unsigned long  *up_long2;

}PARAM_STRUCT;


//Define process functions
void  FuncProc(void);
void  SysFun(void);
void  ComFun(void);
void  FileFun(void);
void  LcdFun(void);
void  UsbFun(void);
void  IccFun(void);
void  McrFun(void);
void  PciFun(void);
void  KbFun(void);
void  AppFun(void);
void  EncryptFun(void);
void  MdmFun(void);
void  PrnFun(void);
void  PiccFun(void);
void  FontFun(void);
void  WlsFun(void);
void  VoiceFun(void);

//for manage
#define MANAGE_NUMBER       2
#define AM_MANAGE_PWD_LEN  8

#define MA_MMK_INDEX    0

enum AM_ERROR_TYPE
{
    AM_SUCCESS = 0,
    AM_APP_NOT_EXIST,
    AM_MA_KEY_NOT_EXIST,       // 密钥不存在
    AM_WRITE_FLASH_ERROR,      // 写flash错误
    AM_ERASE_FLASH_ERROR,      // 擦除flash错误
    AM_CRC_CHECK_ERROR,        // 校验错误
    AM_INVALIDATE,             // 密钥无效
    AM_MANAGE_NOT_EXIST,
    AM_READ_MMK_ERROR,
    AM_USER_PRESS_CANCEL,
    AM_USER_INPUT_ERROR,
    AM_USER_LOGIN_FAILED,
    AM_INPUT_TIME_OUT
};
#define AM_MANAGE_NO1       0
#define AM_MANAGE_NO2       1

typedef struct _MANAGE_KEY_{
BYTE abyFlag[2];  // 0xffff表示管理员密码未初始化，0x5555表示有效，其他值是错误的
BYTE byKeyLen;       // 密钥长度
BYTE byResv;         // 保留
BYTE abyKeyData[24]; // 密文存储
BYTE abyCrc32[4];    // 前面28字节的Lrc校验码
}MANAGE_KEY; // 32字节

typedef struct _MAN_APP_KEY_{
MANAGE_KEY asManKey[MANAGE_NUMBER]; // 两个管理员的登录密码
//APP_SIGN_KEY asAppKey[25];          // 25个应用的签名密钥
BYTE m_abyLock[4];
}MAN_APP_KEY;     // 32*2字节+ 64*25字节 = 1664->32*2+4 

//for download
#define CHANGE_BAUDS_REQUEST        0xb000
#define CHANGE_BAUDS_RESPOND        0xb001


#define AUTHENTICATE_REQUEST        0xb100
#define AUTHENTICATE_STEP2          0xb101
#define AUTHENTICATE_STEP3          0xb102
#define AUTHENTICATE_RESPOND        0xb103


#define FILE_DOWNLOAD_REQUEST       0xb200
#define FILE_DOWNLOAD_RESPOND       0xb201
#define FILE_DATA_DOWNLOAD_REQUEST  0xb202
#define FILE_DATA_DOWNLOAD_RESPOND  0xb203
#define FILE_SAVE_REQUEST           0xb204
#define FILE_SAVE_RESULT            0xb205


#define APP_LIST_REQUEST            0xb300
#define APP_LIST_RESPOND            0xb301
#define APPEND_FILE_LIST_REQUEST    0xb302
#define APPEND_FILE_LIST_RESPOND    0xb303
#define GET_APP_INFO_REQUEST        0xb304
#define GET_APP_INFO_RESPOND        0xb305
#define FONT_INFO_REQUEST           0xb306
#define FONT_INFO_RESPOND           0xb307
#define DELETE_FILE_REQUEST         0xb308
#define DELETE_FILE_RESPOND         0xb309
#define SET_VPOS_TIME_REQUEST       0xb30a
#define SET_VPOS_TIME_RESPOND       0xb30b
#define CREATE_FILE_SYSTEM_REQUEST  0xb30c
#define CREATE_FILE_SYSTEM_RESPOND  0xb30d
#define GET_SERIAL_NUMBER_REQUEST   0xb30e
#define GET_SERIAL_NUMBER_RESPOND   0xb30f
#define SET_SERIAL_NUMBER_REQUEST   0xb310
#define SET_SERIAL_NUMBER_RESPOND   0xb311
#define DELETE_FILE_REQUEST_NAME    0xb312
#define DELETE_FILE_RESPOND_NAME    0xb313

#define LOAD_VOS_SIGN_KEY_REQUEST   0xb320
#define LOAD_VOS_SIGN_KEY_RESPOND   0xb321
#define LOAD_APP_KEY_REQUEST        0xb322
#define LOAD_APP_KEY_RESPOND        0xb323
#define LOAD_MAG_KEY_REQUEST        0xb324
#define LOAD_MAG_KEY_RESPOND        0xb325
#define LOAD_GET_CPU_SN_REQUEST     0xb326
#define LOAD_GET_CPU_SN_RESPOND     0xb327

#define LOAD_SK_MACK_REQUEST        0xb328
#define LOAD_SK_MACK_RESPOND        0xb329

#define LOAD_APP_DUKPTKEY_REQUEST   0xb330
#define LOAD_APP_DUKPTKEY_RESPOND   0xb331

#define LOAD_BOOT_SIGN_KEY_REQUEST  0xb332
#define LOAD_BOOT_SIGN_KEY_RESPOND  0xb333

#define KM_APP_SIGN_KEY             0x00 // 应用程序签名密钥
#define KM_APP_AUTH_LCD_KEY         0x01 // LCD接口调用认证密钥
#define KM_APP_AUTH_MAIN_PIN_KEY    0x02 // PIN主密钥接口调用认证密钥
#define KM_APP_AUTH_PIN_KEY         0x03 // PIN密钥接口调用认证密钥
#define KM_APP_AUTH_MAIN_MAC_KEY    0x04 // MAC主密钥接口调用认证密钥
#define KM_APP_AUTH_MAC_KEY         0x05 // MAC密钥接口调用认证密钥
#define KM_APP_AUTH_OFFPIN_KEY      0x06 // PIN密钥接口调用认证密钥 
#define KM_PCI_MAIN_PIN_KEY         0x10 // PIN主密钥
#define KM_PCI_PIN_KEY              0x11 // PIN密钥
#define KM_PCI_MAIN_MAC_KEY         0x12 // MAC主密钥
#define KM_PCI_MAC_KEY              0x13 // MAC密钥
#define KM_PCI_FKEY                 0x14 // FIXED KEY

#define KM_AUTHPIN_MK_TYPE   0x11
#define KM_AUTHPINK_TYPE     0x12
#define KM_AUTHMACK_TYPE     0x13
#define KM_AUTHMAC_MK_TYPE   0x14
#define KM_AUTHOFFPINK_TYPE  0x15

#define KM_AUTHAPP_TYPE      0x20
#define KM_AUTHLCD_TYPE      0x40


#define KM_SUPPORT_MAX_KEYNUM    (100)


#define FILE_TYPE_APPLICATION    2
#define FILE_TYPE_PARAM          3
#define FILE_TYPE_DATA           4
#define FILE_TYPE_FONT           5

#define ALL_FINISHED                0xb400

#define KM_SUCCESS                 0
#define KM_USER_PRESS_CANCEL       1
#define KM_USER_INPUT_ERROR        2
#define KM_APP_KEY_TYPE_ERROR      3
#define KM_APP_KEY_LEN_ERROR       4
#define KM_APP_NOT_EXIST           5
#define KM_APP_SAVE_KEY_ERROR      6
#define KM_WAIT_TIMEOUT_ERROR      7
#define KM_AUTH_ERROR              8
#define KM_PTK_ERROR               9

#define  LOAD_COM     VCOM_DEBUG  //RS232A //VCOM_DEBUG //RS232A     //VCOM2

//systempara
#define SPF_SUCCESS               0    // 成功
#define SPF_CREATE_FILE_ERROR    -1    // 创建文件失败
#define SPF_PARA_ERROR           -2    // 参数错误
#define SPF_APP_NOEXIST          -3    // 应用程序不存在
#define SPF_NO_FREE_SPACE        -4    // 没有空闲的空间
#define SPF_APP_INACTIVE         -5    // 应用程序未激活

#define APP_MAX_NUM            25  // 应用程序最大个数

#define SERIAL_NUMBER_LEN     40  //32->40

typedef struct _SYSTEM_PARA
{
  BYTE bSN[40];//序列号  //32->40 13/01/28
  BYTE bVer[4];//版本号
  BYTE bAttrib;//应用程序i的属性
  /*
        bit0：0－未装入，  1－已装入
        bit1：0－未激活，  1－已激活
        bit2：0－未运行过，1－已运行过
        bit3: 0- 未设lock  1- 设lock 下载

        bit3-bit7：RFU. 保留
  
  */
  char AppName[33];//应用程序名称
  BYTE Lib_GetDateTime[6];  // 应用程序下载时间
  BYTE sSize[4]; //应用程序大小
  BYTE sDownloadOK[10];//下载是否成功标志(DOWNLOADOK)
}T_SYSTEM_PATA;

#endif





