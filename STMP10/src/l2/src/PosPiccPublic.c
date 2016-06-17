#include "TaxBox.h"
//#include "rc531\Mifre_TmDef.h"
//#include "pn512\Mifre_TmDef.h"

#include"PosPiccPublic.h"
#include "var.h"

#include "vosapi.h"

#define  picc_cmdexchange PiccCmdExchange

/*
  空间限制,bFlag只为1byte
  content(2 Byte)
  P_LANGUAGE(8 Byte)
*/
int PosPiccGetPara(unsigned char bFlag,unsigned char *pbLen,unsigned char *pBuf)
{
  return 0;
}

/*
0 - successful
RC_NO_PARAMETER
*/

int PosPiccSetPara(int nParaLen,unsigned char *psPara)
{
  
	return 0;  
}





void RemoveTailChars(uchar *pszString, uchar ucRemove)
{
	int		i, iLen;

	iLen = strlen((char *)pszString);
	for(i=iLen-1; i>0; i--)
	{
		if( pszString[i]!=ucRemove )
		{
			break;
		}
		pszString[i] = 0;
	}
}


//#ifdef QPBOC_M

int  Contactless_SetTLVData(unsigned short usTag, uchar *pucDtIn, int nDtLen)
{
	return QPboc_SetTLVData(usTag,pucDtIn, nDtLen);
}

int Contactless_BatchSetTLVData(unsigned char bCount,unsigned char *sTLVBuf)
{
	unsigned char i;
	unsigned int nTag,nLen;
	unsigned char *p;
	unsigned char *q;

	
	extern unsigned char *_cl_SeparateBER_TLV (  unsigned char *p,  unsigned int *tag,  unsigned int *len);
	p = sTLVBuf;
	for(i=0;i<bCount;i++)
	{
		q=_cl_SeparateBER_TLV(p,&nTag,&nLen);
		if(q!=NULL)
		{
			Contactless_SetTLVData(nTag,q,nLen);
		}
		p = q+nLen;
	}
        return 0;
}

int  Contactless_GetTLVData(unsigned short Tag, uchar *DtOut, int *nDtLen)
{
	return QPboc_GetTLVData(Tag,DtOut,nDtLen);
}
//#endif

/*********************************************************************
 NAME: uchar dat_bytetobcd(uchar bchar)
 FUNC: convert one uchar to bcd(bchar<100)
 IN  : bchar:to be converted
 OUT : 
 RTRN: the result converted
 NOTE:for example: convert 12 ->  0x18 ( because 0x12=18)
*********************************************************************/
uchar dat_bytetobcd(uchar bchar)
{
	unsigned char bcd_code, btmp;

	btmp = bchar/10;
	bcd_code = btmp*16;
	bcd_code += bchar-btmp*10;
	return(bcd_code);
}


unsigned char dat_bintobcd(uchar *bcd, unsigned long  bin, uchar len)
{
	long lbin;
	int iCount,j;
	uchar sbuf[6];
    
	lbin = 1;
	for(iCount=0; iCount<len; iCount++)
    lbin *= 100;
	if (bin/lbin)
    return(1);
	lbin = bin;
	for (iCount=5; iCount>=0; iCount--)
	{
		sbuf[iCount] = (uchar)(lbin%100);
		lbin /= 100;
	}
	for (iCount=0,j=6-len; j<6; iCount++,j++)
	{
		bcd[iCount] = dat_bytetobcd(sbuf[j]);
	}
	return(0);
}


/*********************************************************************
 NAME: unsigned long dat_hextoul(uchar *bcd, uchar len) 
 FUNC: hex(0 low) to ul
 IN  : hex 
       len
 OUT : 
 RTRN: result:unsigned long
 NOTE: bcd ="\x11\x22\x33" , len =3
       result = 0x112233
*********************************************************************/

unsigned long dat_hextoul(uchar * hex,uchar len)
{
	uchar i;
	unsigned long result=0;
	for(i=0;i<len;i++)
	{
		result *=256;
		result +=hex[i];

	}
	return result;
}


/*********************************************************************
 NAME: void dat_asctobcd(unsigned char *bcd,unsigned char *asc, unsigned short asc_len )
 FUNC: convert ascii data to bcd data 
 IN  : asc: asc data 
       asc_len:the length of ascii data to be converted
 OUT : asc: bcddata point
 RTRN: 
 NOTE: for example: 
        bcd ="\x11\x22\x33"
        dat_bcdtoasc(asc,bcd,6)
        so asc="112233"
*********************************************************************/
void dat_asctobcd(unsigned char *bcd,unsigned char *asc, unsigned short asc_len )
{
   unsigned char is_high, by;

   is_high = !(asc_len % 2);
   *bcd = 0x00;
   while(asc_len-->0)     
   {
	 by = *asc++;

	 if ( !(by&0x10)&&(by>0x30)) by += 9;
	 /*对字母和空格的处理,小写变大写,空格变0*/
     if (is_high)  *bcd = by << 4;
     else
	   {
	   by&=0x0f;
	   *bcd++ |= by;
	   }
	 is_high = !is_high;
	 }

}

/*********************************************************************
 NAME:BYTE low_4bit(BYTE inchar)
 FUNC:get low 4 bit of  one uchar
 IN  :inchar:input uchar
 OUT : 
 RTRN:result 
 NOTE: 
*********************************************************************/
uchar low_4bit(uchar inchar)
{
	return(inchar&0x0f);
}

/*********************************************************************
 NAME: uchar high_4bit(uchar inchar)
 FUNC:get high 4 bit of  one uchar
 IN  :inchar:input uchar
 OUT : 
 RTRN:result 
 NOTE: 
*********************************************************************/
uchar high_4bit(uchar inchar)
{
	return(inchar/16);
}


/*********************************************************************
 NAME: unsigned long dat_bcdtobin(uchar *bcd, uchar len)
 FUNC: convert bcd data to a unsigned long digit
 IN  : bcd:bcd data
       len:bcd len (unit is uchar) 
 OUT : 
 RTRN: the result of unsigned long digit
 NOTE: 
*********************************************************************/
unsigned long dat_bcdtobin(uchar *bcd, uchar len)
{
	int iCount;
	unsigned char bLow, bHigh;
	unsigned long ulBin;

	ulBin = 0;
	for (iCount=0; iCount<len; iCount++)
	{
		bHigh = high_4bit(bcd[iCount]);
		bLow = low_4bit(bcd[iCount]);
		ulBin *= 100;
		ulBin += bHigh*10 + bLow;
	}
	return(ulBin);
}
/*********************************************************************
 NAME: uchar dat_ultohex(uchar *hex, unsigned long ulData)
 FUNC: convert unsigned long data to 4uchar hex data(uchar by uchar)
 IN  : ulData:
 OUT : hex:result
 RTRN: 0
 NOTE: the first uchar is  high ,the last uchar is low
*********************************************************************/
uchar dat_ultohex(uchar *hex, unsigned long ulData)
{
	unsigned long ulTmp;

	hex[3] = (uchar)(ulData%256);
	ulTmp = ulData/256;
	hex[2] = (uchar)(ulTmp%256);
	ulTmp /= 256;
	hex[1] = (uchar)(ulTmp%256);
	ulTmp /= 256;
	hex[0] = (uchar)(ulTmp%256);
	return 0;
}
void ConvBcdToBin(uchar *bcd,uchar *bin,uchar len)
{
	unsigned long ul;
	unsigned char buff[6];
	
	ul =dat_bcdtobin(bcd,len);
	dat_ultohex(buff, ul);
	memcpy(bin,buff,4);
}


/*********************************************************************
 NAME: BYTE dat_bcdadd(uchar *bcd, uchar *added_bcd, uchar len) 
 FUNC: add two bcd string
 IN  : bcd:the first  bcd string 
       added_bcd: the second bcd string
 OUT : bcd:the result
 RTRN: 1:overflow(>100),0:not overflow
 NOTE: bcd ="\x11\x22\x33" , added_bcd="\x33\x33\x33",len=3
       result = "\x44\x55\x66"
*********************************************************************/
uchar  dat_bcdadd(uchar *bcd, uchar *added_bcd, uchar len) // +added_bcd
{
	char i, carry_bit;
	short num, num_add;

	carry_bit = 0;
	
//负数，要为int型(linux)	
	for (i=len; i>0; i--)
	{
		num = (short)dat_bcdtobin(&bcd[i-1], 1);
		
		num_add = (short)dat_bcdtobin(&added_bcd[i-1], 1);
		num += num_add;
		num += carry_bit;
		if (num >= 100)
		{
			carry_bit = 1;
			num -= 100;
		}
		else
			carry_bit = 0;
			
		bcd[i-1] = dat_bytetobcd((uchar)num);
		
	}
	
	return(carry_bit);
}


int Picc_command_APDU_sim(struct scc_cmd *command, unsigned char *rbuf, unsigned int *rlen)
{
#if 0    //for mifre
  unsigned char bRet;
  
  bRet = picc_cmdexchange((unsigned short)command->len,(unsigned char *)command->data,(unsigned short*)rlen,(unsigned char *)rbuf);
  
  return bRet;
#endif
  
  APDU_SEND tApduSend;
  APDU_RESP tApduResp;
  unsigned char bRet;

	
  memcpy(tApduSend.Command,command->data,4);
  tApduSend.Le=0;
  if(command->len<4)
    return 0xfe;
  else if(command->len==4)//case 1
  {
    tApduSend.Lc=0;
    tApduSend.Le=0;
  }
  else if(command->len==5)
  {
    tApduSend.Lc=0;
    if(command->data[4]==0)
	tApduSend.Le=256;//expire return the max data
    else		
	tApduSend.Le=command->data[4];

  }
  else
  {
    tApduSend.Lc = command->data[4];
    memcpy(tApduSend.DataIn,&command->data[5],tApduSend.Lc);
    if(command->len > (tApduSend.Lc+5) )
    {
	if(	command->data[command->len-1]==0)
  	  tApduSend.Le = 256;
	else		
	  tApduSend.Le = command->data[command->len-1];
    }
  }
#if 0 //for mifre  
  if(bRet = picc_isocommand(0,&tApduSend,&tApduResp))
#endif
  if(bRet = Lib_PiccCommand( &tApduSend,&tApduResp))   
  {
    s_printf("Picc_isocommand[%02x]\n",bRet);
    return bRet;
  }
  memcpy(rbuf,tApduResp.DataOut,tApduResp.LenOut);
	
  s_printf("tApduResp len[%i]\n",tApduResp.LenOut);
	
  *rlen = tApduResp.LenOut+2;
  rbuf[*rlen-2] = tApduResp.SWA;
  rbuf[*rlen-1] = tApduResp.SWB;
  
  return bRet;
  
}

//读卡器CAPK,qpboc交易时，再放进qpboctermcapk
//原来应建一个中间文件，但空间有限，现EMV的CAPK和中间文件合为一个。而且每个记录为264(284-20),略去CheckSum[20]
int   EMV_AddCAPK(T_EMV_TERM_CAPK  *capk )
{
	int fid,len,nStruLen;
	int num,i;
	T_EMV_TERM_CAPK tTermCapk;
	
	fid=open(EMV_TERM_CAPK_FILE_M,O_RDWR);
	if(fid<0)
	{
		fid = creat(EMV_TERM_CAPK_FILE_M,0777);
		//fid=open(EMV_TERM_CAPK_FILE_M, O_RDWR | O_CREAT | O_SYNC);
		if(fid<0)
			//return fid;
			return EMV_FILE_ERR;
	}

	len=EMVIF_FileSize(EMV_TERM_CAPK_FILE_M);
	nStruLen = sizeof(T_EMV_TERM_CAPK)-20;
	//num=len/sizeof(T_EMV_TERM_CAPK);
	num=len/nStruLen;
	//first check??
	for(i=0;i<num;i++)
	{
		lseek(fid,i*nStruLen,SEEK_SET);
		read(fid,(unsigned char*)&tTermCapk,nStruLen);
		//if( (!memcmp(capk.RID,tTermCapk.RID,5)) && tTermCapk.Index==capk.Index )
		if( memcmp(	capk->RID,tTermCapk.RID,5)==0 && (tTermCapk.Index==capk->Index) )
		{	
			break;
		}
	}	
	
	if(i==num) //not found
	{
		lseek(fid,0,SEEK_END);
		write(fid,(unsigned char*)capk,nStruLen);
		
	}
	else
	{   
		lseek(fid,i*nStruLen,SEEK_SET);
		write(fid,(unsigned char*)capk,nStruLen);
		
	}
	close(fid);
	return 0;
}

int   EMV_GetCAPK(int Index, T_EMV_TERM_CAPK  *capk )
{
	int fid,nStruLen;
	int len,num;	
	fid=open(EMV_TERM_CAPK_FILE_M,O_RDWR);
	if(fid<0) 
		//return fid;
		return EMV_FILE_ERR; //20100624

	nStruLen = sizeof(T_EMV_TERM_CAPK)-20;
	
	len=EMVIF_FileSize(EMV_TERM_CAPK_FILE_M);
	num=len/nStruLen;
	
	if(	(Index>=num) || num==0)
		return EMV_NOT_FOUND;
		
	lseek(fid,Index*nStruLen,SEEK_SET);
	read(fid,(unsigned char*)capk,nStruLen);
	close(fid);
	
	return 0;
}

int   EMV_DelCAPK(unsigned char KeyID, unsigned char *RID)
{
	int fid,len,nStruLen;
	int num,i;
	T_EMV_TERM_CAPK tTermCapk;
	
	fid=open(EMV_TERM_CAPK_FILE_M,O_RDWR);
	if(fid<0) 
		//return fid;
		return EMV_FILE_ERR;//20100624

	nStruLen = sizeof(T_EMV_TERM_CAPK)-20;
	
	len=EMVIF_FileSize(EMV_TERM_CAPK_FILE_M);
	num=len/nStruLen;
	//first check??
	for(i=0;i<num;i++)
	{
		lseek(fid,i*nStruLen,SEEK_SET);
		read(fid,(unsigned char*)&tTermCapk,nStruLen);
		if( (!memcmp(RID,tTermCapk.RID,5)) && tTermCapk.Index==KeyID )
		{	
			break;
		}
	}	
	
	if(i<num)//find
	{
		//replace find with the end capk
		lseek(fid,(num-1)*nStruLen,SEEK_SET);
		read(fid,(unsigned char*)&tTermCapk,nStruLen);
		lseek(fid,i*nStruLen,SEEK_SET);
		write(fid,(unsigned char*)&tTermCapk,nStruLen);
		ftruncate(fid,len-nStruLen);
	}
	else //20100624
	{
		close(fid);
		return EMV_NOT_FOUND;
	}
	close(fid);
	return 0;	
}

int   EMV_DelAllCAPK() //2011/01/07 add
{
	int fid;
	fid=open(EMV_TERM_CAPK_FILE_M,O_RDWR);
	if(fid<0) return fid;
	ftruncate(fid,0);
	close(fid);
	return 0;

}



	
static void FromTimeToYYMMDD(struct tm * tTime,uchar * CurrYYMMDD)
{
	uchar sDate[4];//,pNewDate[4];

	sDate[0] =dat_bytetobcd(tTime->tm_year);
	sDate[1] =dat_bytetobcd(tTime->tm_mon);
	sDate[2] =dat_bytetobcd(tTime->tm_mday);
	memcpy(CurrYYMMDD,sDate,3);
}

int EMV_FindCAPK(unsigned char *RID,unsigned char *CAPKIndex,T_EMV_TERM_CAPK * pTermCapk)
{

	struct tm  timestru;
	uchar sDate[10];
	int fid,len,nStruLen;
	int num,i;
	T_EMV_TERM_CAPK tTermCapk;

	nStruLen = sizeof(T_EMV_TERM_CAPK)-20;
	
	gettime(&timestru);
	//sprintf((char*)sDate,"%02d%02d%02d",timestru.tm_year,timestru.tm_mon,timestru.tm_mday);
	FromTimeToYYMMDD(&timestru,sDate);//yymmdd

	
	fid=open(EMV_TERM_CAPK_FILE_M,O_RDWR);
	if(fid<0) 
		//return fid;
		return EMV_FILE_ERR; //20100624

	len=EMVIF_FileSize(EMV_TERM_CAPK_FILE_M);
	num=len/nStruLen;
	
	
	//VerifyCAPK and check expire date
	for(i=0;i<num;i++)
	{
		lseek(fid,i*nStruLen,SEEK_SET);
		read(fid,(unsigned char*)&tTermCapk,nStruLen);
		if( (memcmp(RID,tTermCapk.RID,5)==0) && (tTermCapk.Index==*CAPKIndex) )
		{	
			break;
		}
	}
	close(fid);
	if(i==num) //not find 
		return 1;

	/* ? gplian
	if(memcmp(&sDate[0],tTermCapk.ExpDate,3)>0) //yymmdd 09/12/20
	{
		return 2;
	}
	*/
	
	memcpy(pTermCapk,&tTermCapk,nStruLen );
	
	return 0;



}

unsigned char ManagerSetCapk(unsigned short DataLen,unsigned char *UART_RecBuffer)
{
	unsigned char bState;
	unsigned short nOffset = 0;//nLen,
	T_EMV_TERM_CAPK tCapk;
	
	bState= UART_RecBuffer[0];

	switch(bState)
	{
		case 0x11: //add
			memcpy(&tCapk.RID,&UART_RecBuffer[1],6);//RID+index
			nOffset=7;
			//nLen = 
            UART_RecBuffer[nOffset++]*0x100+UART_RecBuffer[nOffset++];
			memcpy(&tCapk.HashIndicator,&UART_RecBuffer[nOffset],3);//(Hash+CAPK)Indicator+len
			nOffset+=3;
			memcpy(&tCapk.Modulus,&UART_RecBuffer[nOffset],tCapk.Len);
			nOffset+=tCapk.Len;
			tCapk.ExponentLen = UART_RecBuffer[nOffset++];
			memcpy(&tCapk.Exponent,&UART_RecBuffer[nOffset],tCapk.ExponentLen);
			nOffset+=tCapk.ExponentLen;
			memcpy(&tCapk.CheckSum,&UART_RecBuffer[nOffset],20);
			if(EMV_AddCAPK(&tCapk)!=0)
				return 2;
			break;
		case 0x21: //del a
		    if(DataLen<6)
				return 1;
			EMV_DelCAPK(UART_RecBuffer[1],&UART_RecBuffer[6]);
			break;
		case 0x22: //del all
			if(DataLen!=1)
				return 1;
			EMV_DelAllCAPK();
			break;
		default:
			return 1;
	}
	
	return 0;
}


int ManagerGetCAPK(unsigned char KeyID, unsigned char *RID,unsigned short *pnLen,uchar *psBuf)
{
	T_EMV_TERM_CAPK tCapk;
	int nRet;
	unsigned short nLen;
	uchar *p;
	if (nRet=EMV_FindCAPK(RID,&KeyID,&tCapk))
		return nRet;
	p = psBuf;
	p+=2; //len:2byte
	memcpy(p,&tCapk.HashIndicator,3);
	p+=3;
	memcpy(p,tCapk.Modulus,tCapk.Len);
	p+=tCapk.Len;
	*p++= tCapk.ExponentLen;
	memcpy(p,tCapk.Exponent,tCapk.ExponentLen);
	p+=tCapk.ExponentLen;
	memcpy(p,tCapk.CheckSum,20);
	p+=20;
	nLen = p-psBuf-2;
	*psBuf++= nLen/0x100;
	*psBuf = nLen%0x100;

	*pnLen = nLen+2;
	
	return 0;
}

int ManagetSetRevocList(unsigned short nLen,unsigned char *sBuf)
{
	unsigned char bState;
	//unsigned char bStruLen;
	
	bState = sBuf[0];
	//bStruLen = sizeof(T_EMV_REVOCATIONLIST);
	
	switch(bState)
	{
		case 0x11:   //add
			//if(nLen<bStruLen+1) //11/06/10
			//	return 1;
			if(QPboc_AddRevocList((T_EMV_REVOCATIONLIST*) &sBuf[1]))
				return 2;
			break;
		case 0x21: //del a
			//if(nLen<bStruLen+1) //11/06/10
			//	return 1;
			if(QPboc_DelRevocList(sBuf[6],&sBuf[1],&sBuf[7]))
				return 2;
			break;
		case 0x22: //delall
			QPboc_DelAllRevocList();
			break;
                default:
			return 1;
	}
	return 0;
}


//move to app
uchar QPboc_SearchExceptionList(unsigned char bLen ,unsigned char *rdPAN,unsigned char  *rdPANSeq)
{
	int fid,len;
	int num,i;
	T_TERMEXCEPTIONFILE tTermException;
	
	fid=open(QPBOC_EXCEPTION_FILE_M,O_RDWR);
	if(fid<0) return fid;
	len=EMVIF_FileSize(QPBOC_EXCEPTION_FILE_M);
	num=len/sizeof(T_TERMEXCEPTIONFILE);
	//first check??
	for(i=0;i<num;i++)
	{
		lseek(fid,i*sizeof(T_TERMEXCEPTIONFILE),SEEK_SET);
		read(fid,(uchar*)&tTermException,sizeof(T_TERMEXCEPTIONFILE));
		if( (!memcmp(rdPAN,tTermException.PAN,tTermException.bLen)) && 
			 (tTermException.bLen==bLen) &&
			( (*rdPANSeq)==tTermException.PANSeqNum) )
			break;
	}	
	 
	if(i==num) //not found
		return 1;
	return 0;	
}


int  QPboc_AddExceptionList(T_TERMEXCEPTIONFILE  *ptException )
{
	int fid,len;
	int num,i;
	T_TERMEXCEPTIONFILE tTermException;
	
	fid=open(QPBOC_EXCEPTION_FILE_M,O_RDWR);
	if(fid<0)
	{
		//creat(FIEL_EXCEPTION_M,0777);
		fid=open(QPBOC_EXCEPTION_FILE_M,O_RDWR | O_CREAT | O_SYNC);
		if(fid<0)
			return fid;
	}
	len=EMVIF_FileSize(QPBOC_EXCEPTION_FILE_M);
	num=len/sizeof(T_TERMEXCEPTIONFILE);

	//first check??
	for(i=0;i<num;i++)
	{
		lseek(fid,i*sizeof(T_TERMEXCEPTIONFILE),SEEK_SET);
		read(fid,(uchar*)&tTermException,sizeof(T_TERMEXCEPTIONFILE));
		if( (!memcmp(ptException->PAN,tTermException.PAN,tTermException.bLen))
			&& (tTermException.bLen==ptException->bLen)
			&& (ptException->PANSeqNum==tTermException.PANSeqNum) )
			break;
	}	

	if(i==num) //not found
	{
		lseek(fid,0,SEEK_END);
		write(fid,(uchar*)ptException,sizeof(T_TERMEXCEPTIONFILE));
		
	}
	else
	{   
		lseek(fid,i*sizeof(T_TERMEXCEPTIONFILE),SEEK_SET);
		write(fid,(uchar*)ptException,sizeof(T_TERMEXCEPTIONFILE));
		
	}
	close(fid);	
	return 0;
}

int  QPboc_DelExceptionList(T_TERMEXCEPTIONFILE  *ptException )
{
	int fid,i,k,num,len,lenStruct;
	//T_EMV_REVOCATIONLIST bufStruct;
	//T_TERMEXCEPTIONFILE tTermException;
	uchar sBuf[100];
	uchar sFileBuf[100];
	
	lenStruct=sizeof(T_TERMEXCEPTIONFILE);
	memcpy(sBuf,&ptException->bLen,lenStruct);
	
//	memcpy((byte*)&msgStruct,pRevocList,lenStruct);
	fid=open(QPBOC_EXCEPTION_FILE_M,O_RDWR);
	if(fid<0) 
		//return fid;
		return EMV_FILE_ERR;//20100624
	len=EMVIF_FileSize(QPBOC_EXCEPTION_FILE_M);
	num=len/lenStruct;
	
	for(i=0;i<num;i++)
	{
		lseek(fid,i*lenStruct,SEEK_SET);
		read(fid,sFileBuf,lenStruct);
		if(memcmp(sFileBuf,sBuf,lenStruct)==0)
			break;
	}
	k=i;	
	if(i<num)
	{
		lseek(fid,(num-1)*lenStruct,SEEK_SET); //read the end record
		read(fid,sFileBuf,lenStruct);
		lseek(fid,k*lenStruct,SEEK_SET);
		write(fid,sFileBuf,lenStruct);
		ftruncate(fid,len-lenStruct);
		
	}
	else
	{
		close(fid);
		return EMV_NOT_FOUND;	
		
	}		
	close(fid);
	return EMV_OK;
}

int  QPboc_DelAllExceptionList(void)
{
	int fid;
	fid=open(QPBOC_EXCEPTION_FILE_M,O_RDWR);
	if(fid<0) return fid;
	ftruncate(fid,0);
	close(fid);
	return 0;
}

extern void dat_bcdtoasc( uchar  *asc, uchar  *bcd, unsigned short asc_len);

int ManagerSetExceptionList(unsigned short nLen,unsigned char *sBuf)
{
	uchar bState,bLen,bStruLen;
	char sStr[100]={0};
	T_TERMEXCEPTIONFILE tTermException;

	bStruLen = sizeof(T_TERMEXCEPTIONFILE);
	bState = sBuf[0];
	memcpy(&tTermException.PAN,&sBuf[2],11);
	if(bState==0x11 || bState==0x21)
	{
		if(nLen!= (bStruLen+1))
			return 1;
		dat_bcdtoasc((unsigned char *)sStr, tTermException.PAN, 20);
		RemoveTailChars((unsigned char *)sStr,'F');
		bLen = strlen(sStr);
		tTermException.bLen = (bLen+1)/2;
	}
	switch(bState)
	{
		case 0x11://add
			if(QPboc_AddExceptionList(&tTermException))
				return 2;
			break;
		case 0x21://del
			if(QPboc_DelExceptionList(&tTermException))
				return 2;
			
			break;
		case 0x22://del all
			if(QPboc_DelAllExceptionList())
				return 2;
			break;
	}
	
	return 0;
	
	
}

int ManagerFindExceptionList(unsigned short nLen,unsigned char *sBuf)
{
	uchar sPan[20],sPanSeq[10],sPanLen;
	uchar sStr[100]={0};
	if(nLen!=11)
		return 1;
	memcpy(sPan,sBuf,10);
	memcpy(sPanSeq,&sBuf[10],1);
	dat_bcdtoasc(sStr, sPan, 20);
	RemoveTailChars(sStr,'F');
	sPanLen = strlen((char *)sStr);
	
	if(QPboc_SearchExceptionList((sPanLen+1)/2,sPan,sPanSeq))
		return 2;
	return 0;
}

/*
sBuf:
X(2) – 商户分类码(PBOC标签9F15) 
X(6) – 收单行标识(PBOC标签9F01) 
X(2) – 终端国家代码(PBOC标签9F1A) 
X(2) – 终端交易货币代码(PBOC标签5F2A) 
X(1) – 终端交易货币指数 (PBOC标签5F36) 
X(1) –终端交易类别代码
X(2) –商户类别代码(PBOC标签9F15)
X(2) – RFU长度
X(n) – RFU数据
*/
int ManagerSetPbocParameter(unsigned short nLen,unsigned char *sBuf)
{
	#pragma pack(1)
	typedef struct
	{
		uchar MerchCatCode[2]; //9F15
		uchar AcquirerID[6];   //9F01
		uchar TmCntrCode[2];   //9F1A
		uchar TmTransCur[2];   //5F2A
		uchar TmTransCurExp;   //5F36
		uchar TranType;        //9C
		uchar MerchantID[15];  //9F16
		uchar RFU[100];
	}T_PBOC_PARAMETER;
	typedef struct
	{
		unsigned short nTag;
		unsigned char  bLen;
		unsigned char *pData;

	}T_TAGDATA;

	#pragma pack()
	
	T_PBOC_PARAMETER tPbocPara;
	T_TAGDATA tTagData[]=
	{
		{0x9F15,2,tPbocPara.MerchCatCode},
		{0x9F01,6,tPbocPara.AcquirerID},
		{0x9F1A,2,tPbocPara.TmCntrCode},
		{0x5F2A,2,tPbocPara.TmTransCur},
		{0x5F36,1,&tPbocPara.TmTransCurExp},
		{0x9C,  1,&tPbocPara.TranType},
		{0x9F16,15,tPbocPara.MerchantID},
		{0x00,0,NULL},
	};
	unsigned short i;

    if(nLen<sizeof(T_PBOC_PARAMETER)-100)
		return 1;
	memcpy(&tPbocPara,sBuf,sizeof(T_PBOC_PARAMETER) );
	i=0;
	for(;;)
	{
		if(tTagData[i].nTag==0)
			break;
		if(Contactless_SetTLVData(tTagData[i].nTag,tTagData[i].pData,tTagData[i].bLen))
			return 2;
		i++;
	}

    return 0;
}

/*
sBuf:
X(1) – AID长度 
X(16) – AID数据
X(1) – 部分选择标志（0-不支持，1-支持） 
X(1) – 终端类型（PBOC标签9F35） 
X(3) – 终端性能 (PBOC标签9F33) 
X(5) – 终端附加性能 (PBOC标签9F40) 
X(2) – 终端应用版本号(PBOC标签9F09)
X(11) – 随机选择阈值 
X(3) –随机选择目标百分数
X(3) –随机选择最大目标百分数
X(5) –终端行为代码TAC－拒绝
X(5) –终端行为代码TAC－联机
X(5) –终端行为代码TAC－默认
X(11) –非接触交易限额
X(11) –非接触脱机限额
X(11) –非接触CVM限额
X(2) – RFU长度
X(n) – RFU数据

*/
int ManagerSetAid(unsigned short nLen,unsigned char *sBuf)
{
	#pragma pack(1)
	typedef struct
	{
		uchar bAidLen;
		uchar sAid[16];
		uchar bSelectIndicator;  //与原来的相反，要转换
		uchar TermType;       //9F35  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar TermCapa[3];    //9f33  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar AddTermCapa[5]; //9F40  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar TermAVM[2];     //9F09  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar ThresholdValue[11]; //十进制 ASCII  PBOC ? 暂不支持
		uchar TargetPercentage[3];//十进制 ASCII  PBOC ? 暂不支持
		uchar MaxTargetPercentage[3];//十进制 ASCII PBOC ? 暂不支持
		uchar TACDenial[5];     // PBOC ? 暂不支持
		uchar TACOnline[5];     // PBOC ? 暂不支持
		uchar TACDefault[5];    //PBOC ? 暂不支持
		uchar ulRdClssTxnLmt[11];//十进制 ASCII
		uchar ulRdClssFLmt[11];  //十进制 ASCII
		uchar ulRdCVMLmt[11];    //十进制 ASCII
		uchar sRFU[100];
	}T_ALLAID;
	#pragma pack()
	
	T_ALLAID tAllAid;
	T_Clss_PreProcInfo tPreProcInfoIn;
	unsigned short nStruLen;
	uchar bSelFlg,bKeynType;
	uchar sStr[100];
	
	nStruLen = sizeof(T_ALLAID);
	if(nLen<nStruLen-100)
		return 1;
	memcpy(&tAllAid,sBuf,nStruLen);
	if(Entry_GetAidList(tAllAid.sAid, tAllAid.bAidLen, &bSelFlg, &bKeynType))
	{
		bSelFlg = tAllAid.bSelectIndicator;
		bKeynType = KERNTYPE_DEF; //default
	}
	bSelFlg = tAllAid.bSelectIndicator; //11/06/06
	if(bSelFlg)
		bSelFlg = PART_MATCH;
	else 
		bSelFlg = FULL_MATCH;
		
	Entry_AddAidList(tAllAid.sAid, tAllAid.bAidLen,bSelFlg,bKeynType);
	
	Entry_GetPreProcInfo(tAllAid.bAidLen,tAllAid.sAid,&tPreProcInfoIn);
	memset(sStr,0,sizeof(sStr));
	memcpy(sStr,tAllAid.ulRdClssTxnLmt,11);
	tPreProcInfoIn.ulRdClssTxnLmt = atoi((char *)sStr);

	memset(sStr,0,sizeof(sStr));
	memcpy(sStr,tAllAid.ulRdClssFLmt,11);
	tPreProcInfoIn.ulRdClssFLmt= atoi((char *)sStr);
	
	memset(sStr,0,sizeof(sStr));
	memcpy(sStr,tAllAid.ulRdCVMLmt,11);
	tPreProcInfoIn.ulRdCVMLmt= atoi((char *)sStr);

	tPreProcInfoIn.ulTermFLmt = dat_hextoul(tAllAid.sRFU,4);
	tPreProcInfoIn.ucStatusCheckFlg = tAllAid.sRFU[4];

	tPreProcInfoIn.ucZeroAmtNoAllowed = tAllAid.sRFU[5];
	
	//if(tAllAid.sRFU[5])
	//	tPreProcInfoIn.ucZeroAmtNoAllowed = 0;
	//else
	//	tPreProcInfoIn.ucZeroAmtNoAllowed = 1;
	
	tPreProcInfoIn.ucTermFLmtFlg = tAllAid.sRFU[6];
	tPreProcInfoIn.ucRdClssTxnLmtFlg = tAllAid.sRFU[7];
	tPreProcInfoIn.ucRdCVMLmtFlg= tAllAid.sRFU[8];
	tPreProcInfoIn.ucRdClssFLmtFlg= tAllAid.sRFU[9];
		
	Entry_SetPreProcInfo(&tPreProcInfoIn);
	return 0;
}

int ManagerSetAllAid(unsigned short nLen,unsigned char *sBuf)
{
	#pragma pack(1)
	typedef struct
	{
		uchar bAidLen;
		uchar sAid[16];
		uchar bSelectIndicator;  //与原来的相反，要转换
		uchar TermType;       //9F35  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar TermCapa[3];    //9f33  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar AddTermCapa[5]; //9F40  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar TermAVM[2];     //9F09  Terminal through Contactless_SetTLVData? 每个应用不一样
		uchar ThresholdValue[11]; //十进制 ASCII  PBOC ? 暂不支持
		uchar TargetPercentage[3];//十进制 ASCII  PBOC ? 暂不支持
		uchar MaxTargetPercentage[3];//十进制 ASCII PBOC ? 暂不支持
		uchar TACDenial[5];     // PBOC ? 暂不支持
		uchar TACOnline[5];     // PBOC ? 暂不支持
		uchar TACDefault[5];    //PBOC ? 暂不支持
		uchar ulRdClssTxnLmt[11];//十进制 ASCII
		uchar ulRdClssFLmt[11];  //十进制 ASCII
		uchar ulRdCVMLmt[11];    //十进制 ASCII
		uchar sRFU[100];
	}T_ALLAID;
	#pragma pack()
	
	T_ALLAID tAllAid;
	T_Clss_PreProcInfo tPreProcInfoIn;
	unsigned short nStruLen;
	uchar bSelFlg,bKeynType,bSelFlgSet;
	uchar sStr[100],i;
	
	nStruLen = sizeof(T_ALLAID);
	if(nLen<nStruLen-100)
		return 1;

	memcpy(&tAllAid,sBuf,nStruLen);

	bSelFlgSet = tAllAid.bSelectIndicator;

	if(bSelFlgSet)
		bSelFlgSet = PART_MATCH;
	else
		bSelFlgSet = FULL_MATCH;
		
	i=0;
	for(;;)
	{
		if(Entry_GetAidListByNum(i++,tAllAid.sAid, &tAllAid.bAidLen, &bSelFlg, &bKeynType)) //the end file
			break;
	/*	
	if(Entry_GetAidList(tAllAid.sAid, tAllAid.bAidLen, &bSelFlg, &bKeynType))
	{
		bSelFlg = tAllAid.bSelectIndicator;
		bKeynType = KERNTYPE_DEF; //default
	}
	*/
	
		if(bSelFlg)
			bSelFlg = PART_MATCH;
		else 
			bSelFlg = FULL_MATCH;
		
		Entry_AddAidList(tAllAid.sAid, tAllAid.bAidLen,bSelFlgSet,bKeynType);//11/06/06
	
		Entry_GetPreProcInfo(tAllAid.bAidLen,tAllAid.sAid,&tPreProcInfoIn);
		memset(sStr,0,sizeof(sStr));
		memcpy(sStr,tAllAid.ulRdClssTxnLmt,11);
		tPreProcInfoIn.ulRdClssTxnLmt = atoi((char *)sStr);

		memset(sStr,0,sizeof(sStr));
		memcpy(sStr,tAllAid.ulRdClssFLmt,11);
		tPreProcInfoIn.ulRdClssFLmt= atoi((char *)sStr);
	
		memset(sStr,0,sizeof(sStr));
		memcpy(sStr,tAllAid.ulRdCVMLmt,11);
		tPreProcInfoIn.ulRdCVMLmt= atoi((char *)sStr);

		tPreProcInfoIn.ulTermFLmt = dat_hextoul(tAllAid.sRFU,4);
		tPreProcInfoIn.ucStatusCheckFlg = tAllAid.sRFU[4];

		tPreProcInfoIn.ucZeroAmtNoAllowed = tAllAid.sRFU[5];

		
		//if(tAllAid.sRFU[5])
		//	tPreProcInfoIn.ucZeroAmtNoAllowed = 0;
		//else
		//	tPreProcInfoIn.ucZeroAmtNoAllowed = 1;
		
		tPreProcInfoIn.ucTermFLmtFlg = tAllAid.sRFU[6];
		tPreProcInfoIn.ucRdClssTxnLmtFlg = tAllAid.sRFU[7];
		tPreProcInfoIn.ucRdCVMLmtFlg= tAllAid.sRFU[8];
		tPreProcInfoIn.ucRdClssFLmtFlg= tAllAid.sRFU[9];
		
		Entry_SetPreProcInfo(&tPreProcInfoIn);

	}
	return 0;
}


/****************************************************************************
 Function:      从磁道中取卡号
 Param In:
	track2 2磁道数据
	track3 3磁道数据
 Param Out:		szCardNo 卡号，带结束符
 Return Code:   E_ERR_SWIPE 磁道信息错；
				OK 成功
****************************************************************************/
uchar GetCardFromTrack(uchar *szCardNo,uchar *track2,uchar *track3)
{
	int i;

	if(track2!=NULL)
		track2[37] = 0;
	if(track3!=NULL)
		track3[104] = 0;
	
	/* POS端不存卡表，从2磁道开始到‘＝’*/
	if(strlen((char *)track2) != 0)
	{	
		i = 0;
		while (track2[i] != '=')
		{
			if(i > 19) return E_ERR_SWIPE;
			i++;
		}
		if( i < 13 || i > 19) return E_ERR_SWIPE;
		memcpy(szCardNo, track2, i);		
		szCardNo[i] = 0;

		return 0;	// [4/10/2006 ]
	}
	else if(strlen((char *)track3 )!= 0) 
	{
		i = 0;
		while(track3[i] != '=') {
			if(i > 21) return E_ERR_SWIPE;
			i++;
		}			    
		if( i < 15 || i > 21)
			return E_ERR_SWIPE;		
		memcpy(szCardNo, track3+2 ,i-2);		
		szCardNo[i-2]=0;
		return 0;	// [4/10/2006 ]
	}
	
	return E_ERR_SWIPE;		
//	return OK;	// [4/10/2006 ]
}
   
uchar AppSetClss_capk(uchar index,uchar *rid)
{
	//int fd;
	T_EMV_TERM_CAPK capk;

	if(EMV_FindCAPK(rid,&index,&capk))
		return 1;
	QPboc_AddCAPK(&capk);
	
	return 0;
}	

