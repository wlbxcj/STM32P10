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
#define SYSKEY_FILE_ADDR  0x00
#define SYSKEY_FILE_SIZE  16256  //48256->16256
//size:256+3*16000=48256 byte==>size:256+1*16000=16256

#define AUTHKEY_FILE   "AuthKey.Key"
#define AUTHKEY_FILE_ADDR 0x3F80     //size 64*3=192 byte->64*1,0xbc80->0x3F80
#define AUTHKEY_FILE_SIZE 64   //192->64

//用于保存每个应用的认证密钥与每个应用的的MK，PINK，MACK，DESK的认证密钥
#define DUKPTKEY_FILE  "DukptKey.Key"
//用于保存DUKPT密钥与KSN。
#define DUKPTKEY_FILE_ADDR 0x3FC0     //size (16+21*28)*2=1208.0xbd40=>0x3FC0
#define DUKPTKEY_FILE_SIZE 1208       // not support dukpt.1208=>0->1208

#define MDK_FILE       "Mdk.Key"
//用于保存磁头的认证密钥MDK。
#define MDK_FILE_ADDR      0x4478    //size  64*3 = 192->0 0xC1F8=>0x3FC0->4478
#define MDK_FILE_SIZE      0     //192->0

#define PCI_APPKEY_SPACE     (16*1024)

#define STORAGE_WORD_COUNT 16

#define PCI_KEYFILE_FLAG     (0)
#define PCI_LOCK_ADDR        (16)
#define PCI_MANAGEPWD_ADDR   (16+8)
#define PCI_CRC_ADDR_PAGE0         (16+8+10) //13/07/01 2byte

#define PCI_LIST_ADDR         (16+8+10+2) //13/07/11 yy-mm-dd hh:MM:SS xxxx 12+6+4+1byte

//#define PCI_PIN_MKEY_ADDR    (256+3200*0)  //size 32*100(100组)
#define PCI_PIN_MKEY_ADDR    (128+3200*0)  //size 32*100(100组)

#define PCI_PIN_MKEY_ADDR_PAGE0 (128+3200*0) //60组
#define PCI_CRC_ADDR_PAGE1      (40*32+2)//2 byte 
/*
128+60*32 --key page0
128+2048+40*32+2--key page1

*/
#define PCI_PINKEY_ADDR       (256+3200*1)

#define PCI_CRC_ADDR_PAGE2    (2) //2 Byte

#define PCI_PINKEY_ADDR_PAGE2 (2048*2+4)  //63 组
#define PCI_CRC_ADDR_PAGE3    (37*32+2) //2 byte

/*
128+2048+40*32+2+23*32--key page1
128+2048*2+2+63*32--key page2
128+2048*3+2+14*32+2--key page3
*/

//MACKEY MAC_MKEY not use,备份PIN_MKEY PINKEY 13/07/09

#define PCI_BACK_ADDR     (4*2048)


#define PCI_MACKEY_ADDR      (256+3200*2)
//#define PCI_MACKEY_ADDR      (256+2048*4)

#define PCI_MAC_MKEY_ADDR    (256+3200*3)
//#define PCI_MAC_MKEY_ADDR    (256+2048*4+3200)

#define PCI_FKEY_ADDR        (256+3200*4)
//#define PCI_FKEY_ADDR        (256+2048*4+3200*2)


#define PCI_AUTHTIMES_MAX    (5)

#define PCI_MAX_APPNUM       (1)   //10->3->1
#define PCI_MAX_KEYNUM       (100)

void vTwoOne(unsigned char *in, unsigned short in_len, unsigned char *out);
void vOneTwo(unsigned char *in, unsigned short lc, unsigned char *out);
void s_PciDispStr(uchar *inp_buf,uchar len, uchar ly,uchar lx);
int  s_PciInputPin(unsigned char min,unsigned char max,unsigned char *out_pin,unsigned short timeoutsec);
int  s_PciInputPinAuto(unsigned char *out_pin);
int  DES_TDES(uchar *key,uchar keylen,uchar *data,uchar datalen,uchar mode);

int  s_InitSysKeyFile(unsigned char bFlag);
int  s_ReadSysKeyFile(long offset,unsigned char fromwhere,unsigned char *outdata,int max_len);
int  s_WriteSysKeyFile(long offset,unsigned char fromwhere,unsigned char *indata,int max_len);

#endif