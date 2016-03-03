#ifndef _DUKPT_H_
#define _DUKPT_H_ 

#define DUKPTKEY_MAXNUM     10 //->2 but pci.c(#define MAX_DUKPT_NUM     2)

typedef struct
{
    unsigned char KeyLen;       //key len,0,8,16,24 etc. 
    unsigned char Key[24];      //content 
    unsigned char CrcCheck[2];       //check 
    unsigned char Reserve;
}T_FUTURE_KEY;             //28 bytes 

//DUKPT key struct 
typedef struct
{   
    unsigned char AppNo;            //application number.
    unsigned char KeyID;            //key index no 0-9 
    unsigned char KeyFlag;          //status:0-not initialized,1-initialized,2-overflowed 
    unsigned char KSN[10];   	    //content 
    unsigned char KsnCrc[2];        //KSN check--不包含KSN最右端21位内容 
    unsigned char bps;              //reserve bytes.
    T_FUTURE_KEY  FutureKey[21];    //future 21 keys 
} T_DUKPT_KEY;                      //604 bytes  
 
 
#define DUKPTKEY_FILE  "DukptKey.Key"
//用于保存DUKPT密钥与KSN。


#define DUKPTKEY_SIZE      (sizeof(T_DUKPT_KEY))
#define FUTUREKEY_SIZE     (sizeof(T_FUTURE_KEY))

 
 
#define NEEDCALL_NEWKEY1          0
#define NEEDCALL_NEWKEY2          1
#define NEEDCALL_NEWKEY3          0
#define NEEDCALL_NEWKEY4          1




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

ulong bit21_to_ulong(uchar *indata);
int  s_ReadDukptKSN(uchar appno,uchar dukptkey_id,uchar *out_ksn);
int s_Dukpt_RequestPinOrMac(uchar mode,uchar *curkey,uchar *curkey_len,uchar *curksn);

#endif

