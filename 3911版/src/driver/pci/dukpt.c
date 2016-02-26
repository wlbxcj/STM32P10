/*********************************************************************************
  
***********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 

#if 0
#include "KF701DH.h"
#include "stm32f10x_lib.h"
#endif

#include "..\..\inc\FunctionList.h"

#include "base.h"
#include "vosapi.h"
#include "pci.h"
//#include "usip.h"
#include "dukpt.h" 

uchar g_bDukpt_KSN[10];         //current KSN  
uchar g_bDukpt_CurAppNo;
uchar g_bDukpt_CurKeyID;
uchar g_bDukpt_CurFutureKeyID;  //current key unit no(1－21) 
uchar g_bDukpt_CryptReg1[8];    //临时密钥单元1 
uchar g_bDukpt_CryptReg2[8];    //临时密钥单元2 
uchar g_bDukpt_CryptReg3[8];    //临时密钥单元3 
uchar g_bDukpt_KeyReg[24];      //临时密钥单元 
ulong g_lDukpt_EncryptCnt;      //加密计算器，后21位有效 
ulong g_lDukpt_ShiftReg;        //移位寄存器，后21位有效 
 
//static T_DUKPT_KEY DukptKey[DUKPTKEY_MAXNUM];
//extern uchar gFileBuffer[32*1024]; 
  
int  s_Dukpt_NewKey(void);
int  s_Dukpt_NewKey1(void);
int  s_Dukpt_NewKey2(void);
int  s_Dukpt_NewKey3(void);
int  s_Dukpt_NewKey4(uchar mode);
int  s_Dukpt_AddCounter(ulong counter); 
int  s_Dukpt_SetBit(void);
int  s_Dukpt_RequestPinOrMac(uchar mode,uchar *curkey,uchar *curkey_len,uchar *curksn);
int  s_ReadDukptFutureKey(uchar appno,uchar dukptkey_id,uchar futurekey_id,uchar *keylen,uchar *keydata);
int  s_WriteDukptFutureKey(uchar appno,uchar dukptkey_id,uchar futurekey_id,uchar keylen,uchar *keydata);
int  s_ReadDukptKSN(uchar appno,uchar dukptkey_id,uchar *out_ksn);
int  s_WriteDukptKSN(uchar appno,uchar dukptkey_id,uchar *in_ksn);
int  s_WriteDukptKeyFlag(uchar appno,uchar dukptkey_id,uchar mode);
int  s_Dukpt_NRKGP(void);
void s_DukptGetCRC(unsigned char *address, unsigned int size, unsigned char *crcOut);

extern int DES_TDES(uchar *key,uchar keylen,uchar *dat,uchar datalen,uchar mode);

/*============================================================
　功能：清除所有与密钥有关的全局变量；
　说明：在退出GETPIN或GETMAC接口之前，须调用此接口。
 ============================================================*/
void s_ClearDukptGlobalParameter(void)
{
	memset(g_bDukpt_CryptReg1,0,sizeof(g_bDukpt_CryptReg1));
	memset(g_bDukpt_CryptReg2,0,sizeof(g_bDukpt_CryptReg2));
	memset(g_bDukpt_CryptReg3,0,sizeof(g_bDukpt_CryptReg3));
	memset(g_bDukpt_KeyReg,0,sizeof(g_bDukpt_KeyReg));
} 

int s_PciDukptInit(void)
{ 
    memset(g_bDukpt_KSN,0,sizeof(g_bDukpt_KSN));
    s_ClearDukptGlobalParameter();
    s_InitDukptKeyFile();   
    return 0;
}  

int s_InitDukptKeyFile(void)
{
    int iret,i,fd; 
    T_DUKPT_KEY  dukpt_key;
    
    memset(&dukpt_key,0,sizeof(dukpt_key));
    dukpt_key.AppNo=0xff; 
    fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0)
    {
        fd = my_open(DUKPTKEY_FILE, O_CREATE, (uchar *)"\xff\x02"); 
        for(i=0;i<DUKPTKEY_MAXNUM;i++)
	    {
	         my_FileWrite(fd, (uchar *)&dukpt_key, DUKPTKEY_SIZE); 
	    } 
    }
    my_FileClose(fd);
    return 0;
} 


/*============================================================
　功能：将3字节数组转换为长整型数据
　说明：高字节在前，低字节在后。
 ============================================================*/
ulong bit21_to_ulong(uchar *indata)
{
	ulong  temp1,temp2; 
	temp1=0;
	temp2=1;
	temp1+=indata[2]*temp2;
	temp2<<=8;
	temp1+=indata[1]*temp2;
	temp2<<=8;
	temp1+=indata[0]*temp2;
    //temp1=temp1&0x001FFFFF;
	return temp1;	
}

/*============================================================
　功能：将长整型数据3字节数组转换为3字节数组
　说明：高字节在前，低字节在后。
 ============================================================*/
void ulong_to_bit21(ulong indata,uchar *outdata)
{
	ushort nHigh, nLow; 
	//temp=indata&0x001FFFFF;	
	nHigh =(unsigned short)(indata>>16);
	nLow  =(unsigned short)(indata&0xffff);
	outdata[0] =nHigh&0x1F;
	outdata[1] =nLow>>8;
	outdata[2] =nLow&0xff;
}

/*===================================================================
　功能：根据＃21单元的未来密钥与KSN生成21个新未来密钥。
　说明：APPNO：当前应用序号
　　　　DUKPTKEY－ID：当前应用中所支持的DUKPT密钥索引号
　　　　INKSN：初始KSN
 ==================================================================*/
int s_Dukpt_StartInitial(uchar appno,uchar dukptkey_id,uchar *inksn)
{
	int ret; 
	memcpy(g_bDukpt_KSN,inksn,10); 
	g_bDukpt_CurAppNo=appno;
	g_bDukpt_CurKeyID=dukptkey_id;
	g_bDukpt_CurFutureKeyID=21;
	g_lDukpt_EncryptCnt=0;
	g_lDukpt_ShiftReg=((ulong)1<<20); 
	while(1)
	{
		ret=s_Dukpt_NewKey3();
		if(ret) return ret; 
		ret=s_Dukpt_NewKey1();
		if(ret==0) continue;
		if(ret>1)  return ret; 
		ret=s_Dukpt_NewKey4(0);
		if(ret) return ret; 
		ret=s_Dukpt_NewKey2();
		if(ret) return ret;
		break;		
	}	
	return DUKPT_OK;
}
 

//===================================================================
//
//RET=0:NEED CALL NEWKEY1
//RET=1:NEED CALL NEWKEY2
//===================================================================
int s_Dukpt_NewKey(void)
{
	uchar i,j;
	ulong l;
	uchar tempkey[24];	 
	j=0;
	l=g_lDukpt_EncryptCnt;
	for(i=0;i<21;i++)
	{
		if(l&0x00000001) j++;
		l=l>>1;
	}
	if(j<10) return NEEDCALL_NEWKEY1; 
	memset(tempkey,0,sizeof(tempkey));		
	s_WriteDukptFutureKey(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,g_bDukpt_CurFutureKeyID,0,tempkey); 
	s_Dukpt_AddCounter(g_lDukpt_ShiftReg);
	return NEEDCALL_NEWKEY2;	
}

//===================================================================
//1) Shift the Shift Register right one bit(end-off),(A zero is 
//   shifted into position #1,the left-most bit of the register.)
//2) if the shift register now contains all zeros(i.e.,the single
//   "one" was shifted off),goto "newkey4) else goto "newkey3";//
//RET=0: NEED CALL NEWKYE3
//RET=1: NEED CALL NEWKEY4
//======================================================================
int s_Dukpt_NewKey1(void)
{	
	g_lDukpt_ShiftReg=g_lDukpt_ShiftReg>>1;
	if(g_lDukpt_ShiftReg==0) return NEEDCALL_NEWKEY4;
	return NEEDCALL_NEWKEY3;	
}

//=====================================================================
//1) The Shift Register,right justified in 64bits,padded to the left wiht
//   zeros, OR'ed withe the 64 right-most bits of the KSN register,is 
//   transferred into Crypto Register1;
//2) Copy ![Current Key Pointer] into Key Register.
//3) Call the subroutine "Non-reversible Key Generation Process".
//4) Store the contents of Crypto Register1 into the left half of the Future 
//   Key Register indicated by the position of the single "one" bit in the Shift
//   Register;
//5) Store the contents of Crypto Register2 into the right half of the Future 
//   Key Register indicated by the position of the single "one" bit in the Shift
//   Register;
//6) Generate and store the LRC on this Future Key Register;
//7) Go to "NEW KEY1"
//======================================================================
int s_Dukpt_NewKey3(void)
{
	uchar shiftbuf[8];
	uchar i,j,KeyLen;
	uchar tempkey[24];
	ulong l;
	int   ret; 
	memset(shiftbuf,0,sizeof(shiftbuf));
	ulong_to_bit21(g_lDukpt_ShiftReg,&shiftbuf[5]);
	for(i=0;i<8;i++) g_bDukpt_CryptReg1[i]=shiftbuf[i]|g_bDukpt_KSN[i+2]; 
	ret=s_ReadDukptFutureKey(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,g_bDukpt_CurFutureKeyID,&KeyLen,g_bDukpt_KeyReg);
	if(ret) return ret; 
	s_Dukpt_NRKGP();
	memcpy(tempkey,g_bDukpt_CryptReg1,8);
	memcpy(&tempkey[8],g_bDukpt_CryptReg2,8); 
	l=g_lDukpt_ShiftReg;
	j=0;
	for(i=0;i<21;i++)
	{
		if(l&0x00100000)
		{
			j=i+1;
			break;
		}
		l=l<<1;
	} 
	ret=s_WriteDukptFutureKey(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,j,16,tempkey);
	if(ret) return ret; 
	return DUKPT_OK;
}

//=====================================================================
//Add counter to the Encryption Counter
//======================================================================
int s_Dukpt_AddCounter(ulong counter)
{
	uchar buf[3];
	int   ret; 
	g_lDukpt_EncryptCnt=g_lDukpt_EncryptCnt+counter;
	ulong_to_bit21(g_lDukpt_EncryptCnt,buf);
	g_bDukpt_KSN[7]=(g_bDukpt_KSN[7]&0xE0)|buf[0];
	g_bDukpt_KSN[8]=buf[1];
	g_bDukpt_KSN[9]=buf[2];	
	ret=s_WriteDukptKSN(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,g_bDukpt_KSN);
	return ret;	
}

//=====================================================================
//1) Erase the key at ![current key pointer]
//2) Set the Lrc for ![current key pointer] to an invalid value(e.g.increment 
//   the LRC by one);
//3) Add one to the Encryption Counter
//4) goto "NEW KEY2"
//mode=0: no erase current future key
//mode=1: erase current future key
//======================================================================
int s_Dukpt_NewKey4(uchar mode)
{
	int ret;
	uchar tempkey[24];	 
	if(mode)
	{
		memset(tempkey,0,sizeof(tempkey));		
		ret=s_WriteDukptFutureKey(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,g_bDukpt_CurFutureKeyID,0,tempkey);
		if(ret) return ret;	
	}
	ret=s_Dukpt_AddCounter(1);
	return ret;	
}

//=====================================================================
//FUN:if the Encryption Counter Contains all zero,cease operation.(The Pin
//    Entry Device is now inoperative,having encrypted more than 1million pins)
//    if not all zeros,go to "exit";
//======================================================================
int s_Dukpt_NewKey2(void)
{ 
	if((g_lDukpt_EncryptCnt&0x001fffff)==0)
	{
		s_WriteDukptKeyFlag(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,2);
		return  DUKPT_CounterOverFlow;		
	}
	return DUKPT_OK;	
}

//===================================================
//FUN:Set to "one" that bit in the shift register that
//    corresponds to the right-most "one" bit in the 
//    Encryption counter,making all other bits in the
//    shift register equal zero.
//
//if Encryption Counter= 0 0010 1100 1101 1100 0011
//then Shift Register  = 0 0000 0000 0000 0000 0001
//if Encryption Counter= 0 0010 1100 1101 1100 0000
//then shift Register  = 0 0000 0000 0000 0100 0000
//====================================================
int s_Dukpt_SetBit(void)
{
	uchar i,j;
	ulong l; 
	l=g_lDukpt_EncryptCnt;
	j=0;
	for(i=0;i<21;i++)
	{
		if(l&0x00000001)
		{
			j=i;
			break;
		}
		l=l>>1;
	} 
	g_lDukpt_ShiftReg=(1<<j);
	g_bDukpt_CurFutureKeyID=(21-j);
	return DUKPT_OK;	
}

/*===================================================
// 功能：根据当前的KSN，获取当前的PIN或MAC加密密钥
// 说明：MODE＝0　获取PIN加密密钥
// 　　　MODE＝1　获取MAC加密密钥
// 　　　WORKKEY，返回加密密钥内容
//       CURKSN,  返回当前交易时的KSN。　　　　
====================================================*/
int s_Dukpt_RequestPinOrMac(uchar mode,uchar *curkey,uchar *curkey_len,uchar *curksn)
{
	uchar i,buf[24];
	uchar KeyLen;
	int   ret; 
	s_Dukpt_SetBit();
	ret=s_ReadDukptFutureKey(g_bDukpt_CurAppNo,g_bDukpt_CurKeyID,g_bDukpt_CurFutureKeyID,&KeyLen,g_bDukpt_KeyReg);
	if(ret==DUKPT_OK)
	{
		if(mode==0) memcpy(buf,"\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff",16);
		else        memcpy(buf,"\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xff\x00",16);
		for(i=0;i<16;i++) g_bDukpt_KeyReg[i]=g_bDukpt_KeyReg[i]^buf[i];
		memcpy(curksn,g_bDukpt_KSN,10);
		memcpy(curkey,g_bDukpt_KeyReg,16); 
		*curkey_len=KeyLen;
		ret=s_Dukpt_NewKey();
		if(ret==NEEDCALL_NEWKEY1)
		{
			ret=s_Dukpt_NewKey1();
			if(ret==NEEDCALL_NEWKEY3)
			{
				while(1)
				{
					ret=s_Dukpt_NewKey3();
					if(ret) return ret;		
					ret=s_Dukpt_NewKey1();
					if(ret==NEEDCALL_NEWKEY3) continue;
					if(ret>NEEDCALL_NEWKEY4)  return ret;
					break;	
				}				
			} 
			ret=s_Dukpt_NewKey4(1);
			if(ret) return ret; 
			ret=s_Dukpt_NewKey2();		
			return ret;			
		}				
		ret=s_Dukpt_NewKey2();		
	}
	return ret;	
}

//==============================================================
//FUN: Non-reversible Key Generation Process
//1) Crypto-Register1 XORed with the right half of the key Register
//   goes to Crypto-Register2;
//2) Crypto-Register2 DEA-encrypted using,as the key ,the left half of
//   the key Register goes to Crypto-Register2;
//3) Crypto-Register2 XORed with the right half of the key Register 
//   goes to Crypto-Register2; 
//4) XOR the Key Register with hex"C0C0C0C000000000C0C0C0C000000000";
//5) Crypto-Register1 XORed with the right half of the Key Register 
//   goes to Crypto-Register1;
//6) Crypto-Register1 DEA-encrypted using,as the key,the left half of 
//   the Key Register goes to Crypto-Register1;
//7) Crypto-Register1 XORed with the right half of the Key Register 
//   goes to Crypto-Register1;
//8) return from subroutine;
//==============================================================
int s_Dukpt_NRKGP(void)
{
	uchar i,buf[24]; 
	for(i=0;i<8;i++) g_bDukpt_CryptReg2[i]=g_bDukpt_CryptReg1[i]^g_bDukpt_KeyReg[i+8];
	//DES_TDES(g_bDukpt_KeyReg,16,g_bDukpt_CryptReg2,8,1);
	DES_TDES(g_bDukpt_KeyReg,8,g_bDukpt_CryptReg2,8,1);
	for(i=0;i<8;i++) g_bDukpt_CryptReg2[i]=g_bDukpt_CryptReg2[i]^g_bDukpt_KeyReg[i+8];
	memcpy(buf,(uchar *)"\xC0\xC0\xC0\xC0\x00\x00\x00\x00\xC0\xC0\xC0\xC0\x00\x00\x00\x00",16);
	for(i=0;i<16;i++) g_bDukpt_KeyReg[i]=buf[i]^g_bDukpt_KeyReg[i];
	for(i=0;i<8;i++) g_bDukpt_CryptReg1[i]=g_bDukpt_CryptReg1[i]^g_bDukpt_KeyReg[i+8];
	//DES_TDES(g_bDukpt_KeyReg,16,g_bDukpt_CryptReg1,8,1);
	DES_TDES(g_bDukpt_KeyReg,8,g_bDukpt_CryptReg1,8,1);
	for(i=0;i<8;i++) g_bDukpt_CryptReg1[i]=g_bDukpt_CryptReg1[i]^g_bDukpt_KeyReg[i+8]; 
	return DUKPT_OK;
}

 
 

 

/*===================================================================
//功能：读、写未来密钥内容
===================================================================*/
int s_ReadDukptFutureKey(uchar appno,uchar dukptkey_id,uchar futurekey_id,uchar *keylen,uchar *keydata)
{
 
    //uchar MMK[24];
    //uchar ELRCK[8];
	uchar CRC[2];
	T_FUTURE_KEY future_key; 
	T_DUKPT_KEY  dukpt_key;
    int iret,i,fd; 
     
	if(appno>24) return DUKPT_InvalidAppNo;
	//if((dukptkey_id<1)||(dukptkey_id>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID;
	if(dukptkey_id>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID;
	if((futurekey_id<1)||(futurekey_id>21))            return DUKPT_InvalidFutureKeyID;	
	
	memset(&future_key,0,sizeof(future_key));
	//memset(&dukpt_key,0,sizeof(dukpt_key));
	 
    fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoKey;
	    } 
		if(dukpt_key.AppNo==appno && dukpt_key.KeyID==dukptkey_id)
		{
			break;
		}
	}
	my_FileClose(fd); 
	if(i>=DUKPTKEY_MAXNUM) return DUKPT_NoKey;  
	
	future_key=dukpt_key.FutureKey[futurekey_id-1];
	if(future_key.KeyLen==0)      return DUKPT_NoKey; 
	s_DukptGetCRC(future_key.Key, (unsigned int)future_key.KeyLen,CRC);
	if(memcmp(CRC,future_key.CRC,2))
	{
		memset(&future_key,0,sizeof(future_key));
		return DUKPT_InvalidCrc;
	} 
	*keylen=future_key.KeyLen;
	memcpy(keydata,future_key.Key,future_key.KeyLen);  
	memset(&future_key,0,sizeof(future_key));
	return DUKPT_OK;
}

int  s_WriteDukptFutureKey(uchar appno,uchar dukptkey_id,uchar futurekey_id,uchar keylen,uchar *keydata)
{
	 
	T_FUTURE_KEY future_key;
	T_DUKPT_KEY  dukpt_key;
    int iret,i,fd; 
    long offset;
    
	if(appno>24) return DUKPT_InvalidAppNo;
	//if((dukptkey_id<1)||(dukptkey_id>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID;
	if(dukptkey_id>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID;
	if((futurekey_id<1)||(futurekey_id>21))            return DUKPT_InvalidFutureKeyID;
	 
	memset(&future_key,0,sizeof(future_key));
	//memset(&dukpt_key,0,sizeof(dukpt_key));
	s_DukptGetCRC(keydata, (unsigned int)keylen,future_key.CRC);
	future_key.KeyLen=keylen; 
	memcpy(future_key.Key,keydata,8); 
	memcpy(future_key.Key+8,keydata+8,8);  
	 
    fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoKey;
	    } 
	    
		if(dukpt_key.AppNo==appno && dukpt_key.KeyID==dukptkey_id)
		{
			break;
		}
	}
	
	if(i>=DUKPTKEY_MAXNUM)
	{
	    my_FileClose(fd); 
	    return DUKPT_NoKey;  
	}
	
	dukpt_key.FutureKey[futurekey_id-1]=future_key; 
    offset=DUKPTKEY_SIZE;
    offset=0-offset;
    my_FileSeek(fd,offset,FILE_SEEK_CUR); 
    offset=16+(futurekey_id-1)*FUTUREKEY_SIZE;
    my_FileSeek(fd,offset,FILE_SEEK_CUR); 
    iret=my_FileWrite(fd,(uchar *)&future_key,FUTUREKEY_SIZE); 
	
	my_FileClose(fd); 
	
	//13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
	return DUKPT_OK;	
}

/*===================================================================
//功能：读、写KSN信息
===================================================================*/
int  s_ReadDukptKSN(uchar appno,uchar dukptkey_id,uchar *out_ksn)
{
	uchar keyflag;
	uchar CRC[2]; 
	T_DUKPT_KEY  dukpt_key;
    int iret,i,fd; 
    
	if(appno>24) return DUKPT_InvalidAppNo;
	//if((dukptkey_id<1)||(dukptkey_id>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID; 
	if(dukptkey_id>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID; 
	//memset(&dukpt_key,0,sizeof(dukpt_key));
	 
    fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoKey;
	    } 
		if(dukpt_key.AppNo==appno && dukpt_key.KeyID==dukptkey_id)
		{
			break;
		}
	}
	my_FileClose(fd); 
	if(i>=DUKPTKEY_MAXNUM) return DUKPT_NoKey;   
	
	keyflag=dukpt_key.KeyFlag;
	if(keyflag==0) return  DUKPT_NoKey;
	if(keyflag>1)  return  DUKPT_CounterOverFlow;  
	s_DukptGetCRC(dukpt_key.KSN,10,CRC);
	if(memcmp(CRC,dukpt_key.KsnCrc,2))
		return DUKPT_InvalidCrc;

	memcpy(out_ksn,dukpt_key.KSN,10);
	return DUKPT_OK;
}

int  s_WriteDukptKSN(uchar appno,uchar dukptkey_id,uchar *in_ksn)
{
 
	T_DUKPT_KEY  dukpt_key;
    int iret,i,fd; 
    long offset;
    
    
	if(appno>24) return DUKPT_InvalidAppNo;
	//if((dukptkey_id<1)||(dukptkey_id>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID;
    if(dukptkey_id>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID;
	//memset(&dukpt_key,0,sizeof(dukpt_key)); 
	fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoKey;
	    } 
	    
		if(dukpt_key.AppNo==appno && dukpt_key.KeyID==dukptkey_id)
		{
			break;
		}
	}
	
	if(i>=DUKPTKEY_MAXNUM)
	{
	    my_FileClose(fd); 
	    return DUKPT_NoKey;  
	}  
	  
	s_DukptGetCRC(in_ksn,10,dukpt_key.KsnCrc);
	dukpt_key.KeyFlag=1;
	memcpy(dukpt_key.KSN,in_ksn,10); 
	  
    offset=DUKPTKEY_SIZE;
    offset=0-offset;
    my_FileSeek(fd,offset,FILE_SEEK_CUR);  
    iret=my_FileWrite(fd,(uchar *)&dukpt_key,16);  
	my_FileClose(fd);  
        
	//13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
	return DUKPT_OK;
}

//MODE=0: NOT INITIALED
//MODE=1: HAVE INITIALED
//MODE=2: OVERFLOWS
int  s_WriteDukptKeyFlag(uchar appno,uchar dukptkey_id,uchar mode)
{
	T_DUKPT_KEY  dukpt_key;
    int iret,i,fd; 
    long offset;
	if(appno>24) return DUKPT_InvalidAppNo;
	//if((dukptkey_id<1)||(dukptkey_id>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID; 
	if(dukptkey_id>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID; 
	//memset(&dukpt_key,0,sizeof(dukpt_key)); 
	fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoKey;
	    } 
	    
		if(dukpt_key.AppNo==appno && dukpt_key.KeyID==dukptkey_id)
		{
			break;
		}
	} 
	if(i>=DUKPTKEY_MAXNUM)
	{
	    my_FileClose(fd); 
	    return DUKPT_NoKey;  
	}  
	dukpt_key.KeyFlag=mode;   
    offset=DUKPTKEY_SIZE;
    offset=0-offset;
    my_FileSeek(fd,offset,FILE_SEEK_CUR);  
    iret=my_FileWrite(fd,(uchar *)&dukpt_key,16);  
	my_FileClose(fd);  
        
	//13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
	return DUKPT_OK;
}
 
//================================================================
// func: clear respective DUKPT keys
// desc: must be called when deleting certain application 
// ================================================================== 
int  s_DukptEraseKey(uchar AppNo,uchar KeyId,uchar Mode)
{
    T_DUKPT_KEY  dukpt_key;
	uchar found;
	int iret,i,fd,nVal;
    long offset;
 
	if(AppNo>24) return DUKPT_InvalidAppNo;
	if(Mode>1)	 return DUKPT_InvalidMode;
	//if(Mode && (KeyId<1 || KeyId>DUKPTKEY_MAXNUM) ) return DUKPT_InvalidKeyID; 
	if(Mode && (KeyId>=DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID; 
	found=0;
	
	fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
            nVal = DUKPTKEY_SIZE;
	    if(iret<nVal)
	    {
	         break;
	    }  
		if(dukpt_key.AppNo!=AppNo)continue;

		if(!Mode || (dukpt_key.KeyID==KeyId && Mode==1) )
		{  
			memset(&dukpt_key,0x00,sizeof(T_DUKPT_KEY)); 
			dukpt_key.AppNo=0xff;
			dukpt_key.KeyID=0x00; 
            offset=DUKPTKEY_SIZE;
            offset=0-offset;
            my_FileSeek(fd,offset,FILE_SEEK_CUR);  
            iret=my_FileWrite(fd,(uchar *)&dukpt_key,16);  
			found=1;
			if(Mode==1) break;
		}
	} 
	my_FileClose(fd); 
	if(!found)return DUKPT_NotFound;
        
	//13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
	return 0;
}

//================================================================
//  func: initialize DUKPT keys
//  desc: according to downloaded BDK and KSN, generate new 21 keys.
//		  must be done under dual security control.
// ================================================================== 
int  s_DukptLoadKey(uchar AppNo,uchar KeyId,uchar BdkLen,uchar KsnLen,uchar *BdkKsn)
{
	uchar tempksn[10];
	uchar buf[24],buf1[24],buf2[24]; 
	T_DUKPT_KEY  dukpt_key; 
    int iret,i,j,k,fd; 
    long offset;

	if(AppNo>24) return DUKPT_InvalidAppNo;
	//if((KeyId<1)||(KeyId>DUKPTKEY_MAXNUM)) return DUKPT_InvalidKeyID;
	if(KeyId>=DUKPTKEY_MAXNUM) return DUKPT_InvalidKeyID;
	if(BdkLen!=16) return DUKPT_InvalidBDK;
	if(KsnLen>10)  return DUKPT_InvalidKSN;  
	 
	//memset(&dukpt_key,0,sizeof(dukpt_key)); 
	fd=my_open(DUKPTKEY_FILE,O_RDWR,(unsigned char *)"\xff\x02");
    if(fd<0) return DUKPT_NoKey;
   
    my_FileSeek(fd,0,FILE_SEEK_SET);   
    for(i=0;i<DUKPTKEY_MAXNUM;i++)
	{
	    memset(&dukpt_key,0,sizeof(dukpt_key));
	    iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	    if(iret<DUKPTKEY_SIZE)
	    {
	         my_FileClose(fd);
	         return DUKPT_NoEmptyList;
	    } 
		if(dukpt_key.AppNo==AppNo && dukpt_key.KeyID==KeyId)
		{ 
			break;
		} 
	} 
	if(i>=DUKPTKEY_MAXNUM)
	{
	    my_FileSeek(fd,0,FILE_SEEK_SET);   
        for(i=0;i<DUKPTKEY_MAXNUM;i++)
	    {
	        memset(&dukpt_key,0,sizeof(dukpt_key));
	        iret=my_FileRead(fd,(uchar *)&dukpt_key,DUKPTKEY_SIZE);
	        if(iret<DUKPTKEY_SIZE)
	        {
	            my_FileClose(fd);
	            return DUKPT_NoEmptyList;
	        }  
		    if(dukpt_key.AppNo==0xff && dukpt_key.KeyID==0)
		    { 
			    break;
		    } 
	    }  
	    if(i>=DUKPTKEY_MAXNUM)
	    {
	        my_FileClose(fd);
	        return DUKPT_NoEmptyList;
        } 
	}   
	
	dukpt_key.AppNo=AppNo;
	dukpt_key.KeyID=KeyId;  
	dukpt_key.KeyFlag=0;   
    offset=DUKPTKEY_SIZE;
    offset=0-offset;
    my_FileSeek(fd,offset,FILE_SEEK_CUR);  
    iret=my_FileWrite(fd,(uchar *)&dukpt_key,16);  
	my_FileClose(fd);  

	//13/05/27
        if(iret<0)
          return PCI_WriteKey_Err;
        
	j=KsnLen%10;
	if(j) k=10-j;
	else  k=0; 
	if(k) memset(tempksn,0xff,k);
	memcpy(tempksn+k,BdkKsn+BdkLen,KsnLen);
	tempksn[9]=0x00;
	tempksn[8]=0x00;
	tempksn[7]&=0xe0;
	s_WriteDukptKSN(AppNo,KeyId,tempksn); 
	memcpy(buf1,BdkKsn,16); 
	iret=s_WriteDukptFutureKey(AppNo,KeyId,21,16,buf1);
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	memset(buf2,0,sizeof(buf2));
	if(iret)return iret; 
	iret=s_Dukpt_StartInitial(AppNo,KeyId,tempksn);
	return iret;
}  
 

static unsigned short crctableA[16] =
{
0x0000,0x1081,0x2102,0x3183,0x4204,0x5285,0x6306,0x7387,
0x8408,0x9489,0xA50A,0xB58B,0xC60C,0xD68D,0xE70E,0xF78F
};

static unsigned short crctableB[16]=
{
0x0000,0x1189,0x2312,0x329B,0x4624,0x57AD,0x6536,0x74BF,
0x8C48,0x9DC1,0xAF5A,0xBED3,0xCA6C,0xDBE5,0xE97E,0xF8F7
};

void s_DukptGetCRC(unsigned char *address, unsigned int size, unsigned char *crcOut)
{
	unsigned char Sdata;
	unsigned short crc;

	 crc=0x1021;
	 for(;size>0;size--)
	 {
		Sdata = *address++;
		Sdata ^= crc;
		crc = ((crctableA[(Sdata & 0xF0) >> 4] ^ crctableB[Sdata & 0x0F]) ^ (crc >> 8));
	 }
	crcOut[0]=(unsigned char)(crc>>8);
	crcOut[1]=(unsigned char)crc;
}








 