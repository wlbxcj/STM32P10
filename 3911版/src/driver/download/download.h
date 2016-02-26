/***************************************************************************
* Copyright (c) 2006-2009, ShenZhen Viewat Technology CO.,Ltd.
* All rights reserved.
*
* 文件名称：  download.h
* 功能描述：  本地下载和对拷模块头文件
* 其它描述:   // 其它补充内容的说明
*
* 作 者：黄俊斌
* 描述 :
* 版本：  V1.0
* 完成日期：2007年10月
*
* 版本修改历史信息
*
* 作者 ：// 输入修改者名字
* 版本：1.1
* 修改内容 ：
* 完成日期：200X年XX月XX日
****************************************************************************/
#ifndef _DOWN_LOAD_H
#define _DOWN_LOAD_H

#define LRC_CHECK_TYPE     0
#define CRC16_CHECK_TYPE   1
#define CRC32_CHECK_TYPE   2


#define  SHAKE_HAND         0x00
#define  CHECK_PWD          0x01
#define  GET_FILE_INFO      0x02
#define  SET_TIME           0x03
#define  CREATE_FILESYS     0x04
#define  OPEN_FILE          0x05
#define  SEEK_FILE          0x06
#define  READ_FILE          0x07
#define  WRITE_FILE         0x08
#define  SET_FILE_SIZE      0x09
#define  FILE_SIZE          0x12
#define  CLOSE_FILE         0x0a
#define  REMOVE_FILE        0x0b
#define  GET_PRODUCT_INFO   0x0d
#define  QUERY_COMPRESS     0x0f
#define  LOAD_END           0x0e

#define  USER_AUTH          0x20
#define  GET_RANDOM         0x21
#define  WRITE_KEY          0x22
#define  CLEAR_KEY          0x23


#define  SYS_QUERYSHAKE_CMD        0xf0
#define  SYS_READTYPE_CMD          0xf1
#define  SYS_AUTHV100_CMD          0xf2
#define  SYS_AUTHPC_CMD            0xf3
#define  SYS_AUTHVOS_CMD           0xf4
#define  SYS_READVER_CMD           0xf5
#define  SYS_READFLAG_CMD          0xf6
#define  SYS_SETFLAG_CMD           0xf7
#define  SYS_READRECORD_CMD        0xf8
#define  SYS_READSN_CMD            0xf9
#define  SYS_SETTIME_CMD           0xfa
#define  SYS_GETTIME_CMD           0xfb
#define  SYS_DELRECORD_CMD         0xfc

#define  SYS_V100AUTHVOS_CMD       0xfd
#define  SYS_VOSAUTHV100_CMD       0xfe


#define RECV_PACKET_MAX_LEN 5050

#define SUCCESS              0

#define RECV_PACKET_ERROR       101
#define SEND_PACKET_ERROR       102
#define PACKET_LEN_ERROR        103
#define RECV_TIMEOUT            104
#define PACKET_LEN_TOO_LONG     107
#define PACKET_CHECK_TYPE_ERROR 108

#define CRC_CHECK_ERROR             120
#define LRC_CHECK_ERROR             121

#define LOADTABLE_FILE_NAME   "LoadTable"
#define LOADTABLE_LINE_MAX_LEN      2500


#define  LOAD_COM     VCOM_DEBUG  //RS232A //VCOM_DEBUG //RS232A     //VCOM2

#define OPERATE_LOG_FILE_NAME  "Log.log"

#define CMD_SHACK_HAND           0x03
#define CMD_GET_LOG_FILE_LENGTH  0x04
#define CMD_GET_LOG_DATA         0x05
#define CMD_DELETE_LOG_FILE      0x06
#define SEND_FINISH              0x07

#define CHANGE_BAUDRATE          0xA1
#define AUTHENTICATE             0xA2   
#define GET_CPU_SN               0xA3
#define DELETE_FILE              0xA4
#define SET_POS_TIME             0xA5
#define CREATE_FS                0xA6
#define GET_SN                   0xA7
#define SET_SN                   0xA8
#define DELETE_APP_NAME          0xA9 


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


#define KM_INPUT_KEY_LEN           8

#define KM_DEFAULT_KEY_LEN         16


#define ALL_FINISHED                0xb400

#define MAX_PACKET_LEN              (4096+7)

//#define  DOWN_RECV_BUFFLEN      ((4*1024)+12)  //12/10/28 (16*1024)->(4*1024+12)
//#define  DOWN_SEND_BUFFLEN      (1*1024)  //12/10/28  8*1024-> 1*1024

#ifndef BYTE
#define BYTE unsigned char
#endif

typedef struct
{
    BYTE byCreateFileSystem;
    BYTE byAllApp;
    BYTE byAllParamFile;
    BYTE byAllDataFile;
    BYTE byFontFile;
    BYTE byVosAppendFile;
}REPLICATE_CONFIGURE;

extern int g_iComBauds;
extern int      g_LocalDownMode;
extern int g_iChineseFontFlag;

void Crc16CCITT(const BYTE *pbyDataIn, DWORD dwDataLen, BYTE abyCrcOut[2]);
int ChangeComBauds(int iBauds);
int BufferToApp(BYTE *pbyBuffer, DWORD dwBufferLen, BYTE *pbyAppNo);
void GetAppEntrySub(unsigned char *AppHead, unsigned long *MainEntry, unsigned long *EventEntry, unsigned long *AppMsgEntry);
int DownloadFileProcess(BYTE *pbyRecvPacket);
void WriteLoadLog(BYTE byKeyType, BYTE byFlag);
int hal_sys_get_serial_number(unsigned char *nUniqueID);
void SendAppListInfo(void);
void SendAppInfo(BYTE byAppNo);
void SendAppendFileInfo(BYTE byAppNo);
void LocalCreateFileSystem(void);


#endif
