#ifndef _PCI_H
#define _PCI_H

//========================================================
//                   PED AND PCI API
//==========================================================
/*
  
 ==============================================================
 ||                                                          ||
 ||                 SysKey  Space                            ||
 ==============================================================
 10字节：文件标识：SYSKEYOK
 6字节保留：
 8字节：　锁键盘标识：LOCKED　UNLOCK
 9字节：　终端管理密码：8字节的PIN－BLOCK加密结果，1字节指示有无初始化标志。 
*/

//typedef struct
//{
//     unsigned char SysKeyFlag[16];  //SYSKEYOK
//     unsigned char LockFlag[8];     //UNLOCK  LOCKED
//     unsigned char Manage_PWD[8];   //Load Manage PWD
//     unsigned char Manage_PWD_Flag; //0=default 1=have pwd 
//}SYSKEY_INFO; 

typedef struct //__attribute__ ((__packed__))
{    
    unsigned char  KeyType;
    unsigned int   KeyNo;
    unsigned int   ModulLen;
    unsigned char  Modul[256];
    unsigned int   ExpLen;
    unsigned char  Exp[256];
    unsigned char  HashResult[20];
    unsigned char  Valid;
}RSA_KEY;

typedef struct //__attribute__ ((__packed__))
{
     unsigned char  KeyType;
     unsigned char  KeyNo;
     unsigned char  KeyLen;
     unsigned char  KeyData[24];
     unsigned char  Lrc[5];
}DES_KEY;

typedef struct //__attribute__ ((__packed__))
{
     unsigned char  AppName[32];
     unsigned char  KeyType;
     unsigned char  KeyNo;
     unsigned char  KeyLen;
     unsigned char  KeyData[24];
     unsigned char  Lrc[5];
}AUTH_KEY;


typedef struct //__attribute__ ((__packed__))
{
     unsigned char  AppName[32];
     unsigned char  KeyType1;
     unsigned char  KeyNo1;
     unsigned char  KeyType2;
     unsigned char  KeyNo2;
     unsigned char  KeyType3;
     unsigned char  KeyNo3;
     unsigned char  AuthFlag; //=0　未认证　＝1通过KEY1　＝3通过KEY1,2；＝7通过KEY1,2，3
     unsigned char  Reserve;
     unsigned char  Rand[8];
}AUTH_RESULT;


#define RSAKEY_SIZE     (sizeof(RSA_KEY))
#define DESKEY_SIZE     (sizeof(DES_KEY))
#define AUTHKEY_SIZE    (sizeof(AUTH_KEY))


#define MMKey_LenAdr      5
#define MMKey_Adr         6
#define MMKey_LrcAdr      22

#define MAX_FK_NUM        10
#define MAX_MKSK_NUM      100
#define MAX_DUKPT_NUM     2

#define BPK_VALIDATE_FLAG  0xaa55bb55

#define LOCKED_STR       "LOCKED"
#define UNLOCK_STR       "UNLOCK"

#define BPK_MMK_CHECK_STRING   "\xf4\xa3\x1e\x59\x36\x4b\x3d\x82"

#define SYSKEY_FILE    "SysKey.Key"
//用于保存MK，PINK，MACK，DESK
#define AUTHKEY_FILE   "AuthKey.Key"
//用于保存每个应用的认证密钥与每个应用的的MK，PINK，MACK，DESK的认证密钥
#define DUKPTKEY_FILE  "DukptKey.Key"
//用于保存DUKPT密钥与KSN。
#define MDK_FILE       "Mdk.Key"
//用于保存磁头的认证密钥MDK。

#define PCI_APPKEY_SPACE     (16*1024)

#define PCI_KEYFILE_FLAG     (0)
#define PCI_LOCK_ADDR        (16)
#define PCI_MANAGEPWD_ADDR   (16+8)
#define PCI_PIN_MKEY_ADDR    (256+3200*0)
#define PCI_PINKEY_ADDR      (256+3200*1)
#define PCI_MACKEY_ADDR      (256+3200*2)
#define PCI_MAC_MKEY_ADDR    (256+3200*3)
#define PCI_FKEY_ADDR        (256+3200*4)


#define PCI_AUTHTIMES_MAX    (5)

#define PCI_MAX_APPNUM       (10)
#define PCI_MAX_KEYNUM       (100)

void vTwoOne(unsigned char *in, unsigned short in_len, unsigned char *out);
void vOneTwo(unsigned char *in, unsigned short lc, unsigned char *out);
void s_PciDispStr(uchar *inp_buf,uchar len, uchar ly,uchar lx);
int  s_PciInputPin(unsigned char min,unsigned char max,unsigned char *out_pin,unsigned short timeoutsec);
int  s_PciInputPinAuto(unsigned char *out_pin);
int  DES_TDES(uchar *key,uchar keylen,uchar *data,uchar datalen,uchar mode);

int  s_InitSysKeyFile(void);
int  s_ReadSysKeyFile(long offset,unsigned char fromwhere,unsigned char *outdata,int max_len);
int  s_WriteSysKeyFile(long offset,unsigned char fromwhere,unsigned char *indata,int max_len);

#endif