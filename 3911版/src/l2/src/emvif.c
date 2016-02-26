/*
*********************************************************************************************************
*                                                
*                                EMV  4.2.a Leve 2 InterFace
*
*                                 Copyright(c)2002-2009, 
*                                 
*
*                                  V1.0
*
* File : emvif.c
* Author : lian guang ping(lian98@163.com)
* history:
*
*********************************************************************************************************
*/
      

#include "var.h"

//#include "bsp_cipher.h"
#include "rsaapp.h"

#define printf trace_debug_printf

//关于HTTP的
extern int socketfd;
extern int connectFlag;
extern int ConnectServer();

uchar EMVSendAndReceiveByTcp(int nSendLen,uchar *SendBuff,uchar time,uchar bReceFlag,int *pnReceLen,uchar *ReceBuff);

uchar gbPrintSignature=0;

T_EMVTAG stTranReadyForPayIcData[]=
{
	{0x9f02,1},//Amount, Authorised (Numeric) 6
	{0x9f03,1},//Amount, Other (Numeric)      6
	{0x9f1a,1},//Terminal Country Code        2
	{0x95,1},  //Terminal Verification Result(0x95) 5
	{0x5F2a,1},//Transaction Currency Code          2
	{0x9A,1},  //Transaction Date                   3
	{0x9C,1},  //Transaction Type                   1
	{0x9f37,1},//Unpredictable Number               4
	{0x5f34,1},//PAN sequence number                1
	{0x82,1},  //AIP                                2
	{0x9f36,1},//Application transaction counter (ATC) 2
	{0x9f26,1},//ARQC                               8
	{0x9f27,1},//Cryptogram Information Data        1
	{0x9f10,1},//Issuer Application Data            1-32
	{0x00,  0}, //end	 //11/06/08 
};

/*
T_EMVTAG stAuthorisation[]=
{
	{0x82,  1}, //AIP
	{0x9f36,1}, //Application transaction counter (ATC)
	{0x9f26,1}, //ARQC()
	{0x9f27,0}, //Cryptogram Information Data()
	{0x9f34,0}, //CVM Results
	{0x9F1E,0}, //IFD Serial Number()
	{0x9f10,1}, //Issuer Application Data (IAD)(0x9f10)
	{0x9f33,0}, //Terminal Capabilities
	{0x9f35,0}, //Terminal Type
	{0x95,  1}, //Terminal Verification Result(0x95)
	{0x9f37,1}, //Unpredictable Number
	{0x9f01,0},	//Acquirer Identifier
	{0x9f02,1}, //Amount, Authorised (Numeric)
	{0x9f03,1}, //Amount, Other (Numeric)
	{0x5f25,0}, //Application Effective Date
	{0x5f24,0}, //Application Expiration Date
	{0x5a,  1}, //Application PAN
	{0x5f34,1}, //PAN sequence number ()
	{0x99,  0}, //Enciphered PIN Data
	{0x9f15,0}, //Merchant Category Code
	{0x9f16,0}, //Merchant Identifier
	{0x9f39,0}, //POS Entry Mode
	{0x9f1a,1}, //Terminal Country Code()
	{0x9f1c,0}, //Terminal Identifier
	{0x57,  0}, //Track 2 Equivalent Data
	{0x5f2a,1}, //Transaction Currency Code()
	{0x9a,  1}, //Transaction Date(0x9a)
	{0x9f21,1}, //Transaction Time
	{0x9c,  0}, //Transaction Type(0x9c)
	{0x00,  0}, //end	
	
};
 
T_EMVTAG stonlineFinancial[]=
{
	{0x82,  1}, //AIP                                   2
	{0x9f36,1}, //Application transaction counter (ATC) 2
	{0x9f07,0}, //Application Usage Control	            2
	{0x9f26,1}, //ARQC()                                8
	{0x9f27,0}, //Cryptogram Information Data()         1
	{0x8e,  0}, //CVM List                          1-252
	{0x9f34,0}, //CVM Results                           3
	{0x9F1E,0}, //IFD Serial Number()                   8
	{0x9F0D,0}, //Issuer Action Code default            5  
	{0x9F0E,0}, //Issuer Action Code-Denial             5
	{0x9F0F,0}, //Issuer Action Code-Online             5
	{0x9f10,1}, //Issuer Application Data (IAD)(0x9f10) 1-32 
	{0x9f33,0}, //Terminal Capabilities                 3
	{0x9f35,0}, //Terminal Type                         1
	{0x95,  1}, //Terminal Verification Result(0x95)    5
	{0x9f37,1}, //Unpredictable Number                  4
	{0x9f01,0},	//Acquirer Identifier                   6
	{0x9f02,1}, //Amount, Authorised (Numeric)          6
	{0x9f03,1}, //Amount, Other (Numeric)               6
	{0x5f25,0}, //Application Effective Date            3
	{0x5f24,0}, //Application Expiration Date           3
	{0x5a,  1}, //Application PAN                      1-10 
	{0x5f34,1}, //PAN sequence number ()                1
	{0x99,  0}, //Enciphered PIN Data                4-16  
	{0x5f28,0}, //Issuer Country Code                   2
	{0x9f15,0}, //Merchant Category Code                2
	{0x9f16,0}, //Merchant Identifier                  15
	{0x9f39,0}, //POS Entry Mode                        1
	{0x9f1a,1}, //Terminal Country Code()               2
	{0x9f1c,0}, //Terminal Identifier                   8        
	{0x57,  0}, //Track 2 Equivalent Data            1-19  
	{0x81,  1}, //Amount,Authorised (Binary)?           1
	{0x5f2a,1}, //Transaction Currency Code()           2
	{0x9a,  1}, //Transaction Date(0x9a)                3
	{0x9f21,1}, //Transaction Time                      3
	{0x9c,  0}, //Transaction Type(0x9c)                1
	{0x00,  0}, //end	
};

T_EMVTAG stonlineFinancialConfirm[]=
{
	{0xDF31,1}, //Issuer Script Results     1-262
	{0x9F26,1},//TC or AAC                   8
	{0x9f1c,0}, //Terminal Identifier        8
	{0x9f4c,0}, //ICC Dynamic Number   8 //09/09/28
	{0x9B,0},   //Transaction Status Information(TSI) 2 100525
	{0x00,  0}, //end	

};


T_EMVTAG stBatchData[]=
{
	{0x82,  1}, //AIP   4
	{0x9f36,1}, //Application transaction counter (ATC) 5
	{0x9f07,0}, //Application Usage Control	 5
	{0x9f27,0}, //Cryptogram Information Data() 4
	{0x8e,  0}, //CVM List 254, >128,len(2byte)
	{0x9f34,0}, //CVM Results 6
	{0x9F1E,0}, //IFD Serial Number() 11
	{0x9F0D,0}, //Issuer Action Code default 8
	{0x9F0E,0}, //Issuer Action Code-Denial 8
	{0x9F0F,0}, //Issuer Action Code-Online 8
	{0x9f10,1}, //Issuer Application Data (IAD)(0x9f10)  35
	{0xdf31,1}, //Issuer Script Results,var. up to 21, 24  *
	{0x9f33,0}, //Terminal Capabilities 6
	{0x9f35,0}, //Terminal Type  4
	{0x95,  1}, //Terminal Verification Result(0x95) 7
	{0x9f26,1}, //? TC/ARQC or AAC,Application Cryptogram,应用密文 11
	{0x9f37,1}, //Unpredictable Number 7
	{0x9f01,0},	//Acquirer Identifier  9
	{0x9f02,1}, //Amount, Authorised (Numeric) 9
	{0x9f03,1}, //Amount, Other (Numeric)  9
	{0x5f25,0}, //Application Effective Date 6
	{0x5f24,0}, //Application Expiration Date 6
	{0x5a,  1}, //Application PAN    12
	{0x5f34,1}, //PAN sequence number () 4
	{0x89,  1}, //Authorisation Code   8
	{0x8a,  1}, //Authorisation Response code  4
	{0x5f28,0}, //Issuer Country Code  5
	{0x9f15,0}, //Merchant Category Code  4
	{0x9f16,0}, //Merchant Identifier 17
	{0x9f39,0}, //POS Entry Mode   4
	{0x9f1a,1}, //Terminal Country Code() 5
	{0x9f1c,0}, //Terminal Identifier   11
	{0x81,  1}, //Amount,Authorised (Binary)?  6
	{0x5f2a,1}, //Transaction Currency Code() 5
	{0x9a,  1}, //Transaction Date(0x9a) 5
	{0x9f21,1}, //Transaction Time    6
	{0x9c,  0}, //Transaction Type(0x9c) 3
	{0x9B,  0}, //Transaction Status Information(TSI) 4 100525
	
	{0x9f4c,0}, //ICC Dynamic Number   8  //09/09/28
	
	{0x00,  0}, //end	total 552
	
};

T_EMVTAG stAdvice[]=
{
	{0x82,  1}, //AIP  4
	{0x9f36,1}, //Application transaction counter (ATC) 5
	{0x9f27,0}, //Cryptogram Information Data() 4
	{0x9f34,0}, //CVM Results  6
	{0x9F1E,0}, //IFD Serial Number() 11
	{0x9f10,1}, //Issuer Application Data (IAD)(0x9f10) 35
	{0xdf31,1}, //Issuer Script Results,var. up to 21, 24 
	{0x9f33,0}, //Terminal Capabilities 6
	{0x9f35,0}, //Terminal Type   4
	{0x95,  1}, //Terminal Verification Result(0x95) 7
	{0x9f26,1}, //? TC/ARQC or AAC,Application Cryptogram,应用密文 11 
	{0x9f37,1}, //Unpredictable Number 7
	{0x9f01,0},	//Acquirer Identifier 9
	{0x9f02,1}, //Amount, Authorised (Numeric)  9
	{0x5f25,0}, //Application Effective Date  6
	{0x5f24,0}, //Application Expiration Date  6
	{0x5a,  1}, //Application PAN  12
	{0x5f34,1}, //PAN sequence number () 4
	{0x8a,  1}, //Authorisation Response code 4
	{0x9f15,0}, //Merchant Category Code 4
	{0x9f16,0}, //Merchant Identifier  17
	{0x9f39,0}, //POS Entry Mode    4
	{0x9f1a,1}, //Terminal Country Code() 5
	{0x9f1c,0}, //Terminal Identifier 11
	{0x57,  0}, //Track 2 Equivalent Data 21
	{0x81,  1}, //Amount,Authorised (Binary)?  6
	{0x5f2a,1}, //Transaction Currency Code() 5
	{0x9a,  1}, //Transaction Date(0x9a) 5
	{0x9f21,1}, //Transaction Time 6
	{0x9c,  0}, //Transaction Type(0x9c) 3
	{0x00,  0}, //end	   total=252
	
};


T_EMVTAG gtReversalTagList[]=
{
	{0x82,  1}, //AIP  4
	{0x9f36,1}, //Application transaction counter (ATC) 5
	{0x9F1E,0}, //IFD Serial Number() 11
	{0x9f10,1}, //Issuer Application Data (IAD)(0x9f10) 35
	{0xdf31,1}, //Issuer Script Results,var. up to 21, 24
	{0x9f33,0}, //Terminal Capabilities   6
	{0x9f35,0}, //Terminal Type   4
	{0x95,  1}, //Terminal Verification Result(0x95) 7
	{0x9f01,0},	//Acquirer Identifier  9
	{0x5f24,0}, //Application Expiration Date  6
	{0x5a,  1}, //Application PAN  12
	{0x5f34,1}, //PAN sequence number () 4
	{0x8a,  1}, //Authorisation Response code 4
	{0x9f15,0}, //Merchant Category Code 4
	{0x9f16,0}, //Merchant Identifier  17
	            //Original Data Elements,fina format?
	{0x9f39,0}, //POS Entry Mode  4
	{0x9f1a,1}, //Terminal Country Code() 5
	{0x9f1c,0}, //Terminal Identifier  11
	{0x57,  0}, //Track 2 Equivalent Data 21
	{0x81,  1}, //Amount,Authorised (Binary)?  6
	{0x5f2a,1}, //Transaction Currency Code() 5
	{0x9a,  1}, //Transaction Date(0x9a) 5
	{0x9f21,1}, //Transaction Time  6
	{0x9c,  0}, //Transaction Type(0x9c) 3
	{0x00,  0}, //end	     total:218
	
};

T_EMVTAG stTransLog[]=
{
	{0x9c  ,1}, //Transaction Type(0x9c) 3 =tag +value +1?
	{0x9a  ,3}, //Transaction Date(0x9a) 5
	{0x5a  ,11}, //PAN() *12
	{0x5f34,1}, //PAN sequence number () 4
	{0x9f02,6}, //Amount Authorised() 9
	{0x9f26,8}, //ARQC   11
	{0x9f27,1},	//Cryptogram Information Data  4
	{0x9f10,33}, //Issuer Application Data *35
	{0x9f37,4}, //Unpredictable Number  7
	{0x9f36,2}, //Application transaction counter (ATC)() 5
	{0x95  ,5}, //Terminal Verification Result() 7
	{0x5f2a,2}, //Transaction Currency Code() 5
	{0x82,  2}, //Application Interchange Profile() 4
	{0x9f1a,2}, //Terminal Country Code() 5
	{0x9f03,6}, //Amount Other(0x9f03) 9
	{0x9f39,1}, //POS Entry Mode   7
	{0x9f41,4}, //ADD! Transaction Sequence Counter *7
	{0x00,  0}, //end	
};	            //total =132+7
*/

uchar WaitKey(unsigned int iTimeInt);
/*
int  EMVMMI_MenuAppSel(int TryCnt, char  *List[], int AppNum)
{
	return ShowMenu( (char *)List,AppNum);
}
*/
int  EMVMMI_InputAmount(unsigned char *AuthAmt, unsigned char *CashBackAmt)
{
	char sTemp[20],sDisp[100],sAmount[100];
	uchar bValue;
	memset(sAmount,0,sizeof(sAmount) );
        #ifdef KF311_M
          strcpy((char*)AuthAmt,"000");
        #else
        {
	if(AuthAmt!=NULL)
	{

 		AppGetAmount(1,0xf0,sAmount);
		strcpy((char*)AuthAmt,sAmount);
		//KeyInputAmount("输入金额",AuthAmt);
	}
	//cash back
	if(CashBackAmt!=NULL)
	{
		//KeyInputAmount("输入找钱金额",CashBackAmt);
 		AppGetAmount(1,0xf1,sAmount);
		strcpy( (char*)CashBackAmt,sAmount);

	}
        #endif
	return 0;
}



/*
TryFlag:
         bit
          8: 0-ciphertext(x9.8) 1-plaintext
         7-5: try number
           
*/
#ifndef KF311_M
int  EMVMMI_InputPIN(unsigned char Flag, int RemainCnt, unsigned char *pin)
{
	int nRet; 
	unsigned char bLocFlag,sTmp[20];
	unsigned char i;
	uchar sCardNo[16];
	ClrBelow(1);
	nRet = PPWriteMainKey(1,"\x11\x11\x11\x11\x11\x11\x11\x11",8,0); //master
//	printf("PPWriteMainKey nRet=%02x\n",nRet);
	
	nRet = PPWriteDesKey(1,1,"\x22\x22\x22\x22\x22\x22\x22\x22",8,0); //des
//	printf("PPWriteDesKey nRet=%02x\n",nRet);
	
	nRet = PPWritePinKey(1,1,"\x33\x33\x33\x33\x33\x33\x33\x33",8,0); //pin
	TraceDisp("PPWritePinKey nRet=%02x\n",nRet);

//	bLocFlag =(Flag & 0xf0)|0X02;//中英显示
//	bLocFlag =(Flag & 0xf0)|0X00;//中英显示
	bLocFlag =(Flag & 0xf8)|0X00;//中英显示 100520
	nRet = PPPrmptPos(2,0,bLocFlag);
//	printf("PPPrmptPos nRet=%02x\n",nRet);
	 
//	memset(sTmp,0,sizeof(sTmp));

	//for test
	if( (Flag &0x80)==0) //encry
	{
		memset(sCardNo,'0',sizeof(sCardNo) );
		i = strlen(gtPosCom.szCardNo);
		memcpy(&sCardNo[4],&gtPosCom.szCardNo[i-13],12);
		
		nRet = PPGetPwd(1,4,12,sCardNo,sTmp,0,0xff);

	}
	else
		nRet = PPGetPwd(1,4,12,"\x00\x00\x12\x34",sTmp,0,0xff);
//	printf("PPGetPwd nRet=%02x\n",nRet);

if(nRet)
	return nRet;

	//liantest
	//strcpy(sTmp,"1234");
	
	if(Flag&0x80) // plain text
	{
	    pin[0] = strlen(sTmp);
    
    	memset(&sTmp[pin[0] ], 'F', 10);
	//    memcpy(&tTransInq.PinData[1], "\x06\x12\x2f\xff\xff\xff\xff\xff", 7);

		dat_asctobcd(&pin[1],&sTmp[0], 14 );
		pin[0] = 0x20|pin[0];


	}
	else 
	{
		
		//pin[0] =8;
		memcpy(pin,sTmp,8);
	}
//	printf("out:");
//	for(i=0;i<strlen(sTmp);i++)
//		printf("%02x",sTmp[i]);
//	printf("\n");
     
	return 0;
	//keyboard
	
	//PED
	
}
#endif

void tolowerstr(char *sour,char *dest,uchar bLen)
{
	uchar i;
	for(i=0;i<bLen;i++)
	{
		*dest = tolower(*sour++);
		dest++;	
	}
}

#if 0
int EMVIF_GetPreferenceLanguage(unsigned char bLen,unsigned char *sLanguagePrefer )
{
	unsigned char sBuf[8],i,bLocLen,bValue;
	char sDisp[100];
	
//	bLen = (unsigned char)pDataDict[10].len;
	//memcpy(sBuf,pDataDict[10].pContent,bLen);
	tolowerstr(sLanguagePrefer,sBuf,bLen);//070425 
	bLocLen =bLen/2;
	
	for(i=0;i<bLocLen;i++)
	{
		//if(memcmp(&sBuf[2*i],"en",2)==0)
		if(memcmp(&sBuf[2*i],gltLanguage[LOCALE].sAbbr,2)==0)
			return LOCALE;
		else if(memcmp(&sBuf[2*i],gltLanguage[SUPPORT].sAbbr,2)==0)
			return SUPPORT;

	}

//	if(glbLanguage<0)
	{
		for(;;)
		{
			ClrBelow(1);
			printf(sDispPleaseSelectLanguage[glbLanguage]);
			lcdshow(1,MIDDLE,0,sDispPleaseSelectLanguage[glbLanguage]);
			sprintf(sDisp,"1.%s",gltLanguage[LOCALE].sName);
			lcdshow(2,LEFT,0,sDisp);
			sprintf(sDisp,"2.%s",gltLanguage[SUPPORT].sName);
			lcdshow(3,LEFT,0,sDisp);
			
			
			printf(sDispEnter1Or2[glbLanguage]);
			bValue =press_key();
			if(bValue==0x31 || bValue==0x32)
			{
				glbLanguage = bValue-0x30;
				break;
			}
		}
		ClrBelow(2);

	}
	
	return glbLanguage;
    //return -1; 	
	 
}

 
int  EMVIF_CertVerify(void)
{
	int		iRet, iLength;
	uchar	ucKey, ucCertType, szCertNo[50];

	iRet = EMV_GetTLVData(0x9F62, &ucCertType, &iLength);
	if( iRet!=EMV_OK )
	{
		return -1;
	}

	memset(szCertNo, 0, sizeof(szCertNo));
	iRet = EMV_GetTLVData(0x9F61, szCertNo, &iLength);
	if( iRet!=EMV_OK )
	{
		return -1;
	}
	if( iLength>40 )
	{
		return -1;
	}

	//ScrClrLine(2, 7);
	//Api_DispString((uchar *)"请核对持卡人证件", LINE3, LEFT_DISP, NORMAL_DISP);
	
	ClrBelow(0);
	lcdshow(0, MIDDLE, 0, "请核对持卡人证件");
	switch( ucCertType ){
	case 0x00:
		//Api_DispString((uchar *)"身份证:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "身份证:");
		break;

	case 0x01:
		//Api_DispString((uchar *)"军官证:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "军官证:");		
		break;

	case 0x02:
		//Api_DispString((uchar *)"护照:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "护照:");		
		break;

	case 0x03:
		//Api_DispString((uchar *)"入境证:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "入境证:");		
		break;

	case 0x04:
		//Api_DispString((uchar *)"临时身份证:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "临时身份证:");		
		break;

	case 0x05:
		//Api_DispString((uchar *)"台胞证:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "台胞证:");		
		break;

	default:
		//Api_DispString((uchar *)"其它证件:", LINE5, LEFT_DISP, NORMAL_DISP);
		lcdshow(1, LEFT, 0, "其它证件:");		
		break;
	}
	
	//ScrPrint(0, LINE7, ASCII, "%.40s", szCertNo);
	
	lcdshow(2,LEFT,0,szCertNo);
	lcdshow(3,RIGHT,0,"1.批准 2.拒绝");
	while( 1 )
	{
		//ucKey = Api_WaitKey(stPosParam.ucOprtLimitTime);
		//ucKey = WaitKey(gtPosParameter.ucOprtLimitTime);
		ucKey = getkey();
		if( ucKey==0x32 )
		{
			return -1;
		}
		if( ucKey==0x31 )
		{
			sprintf((char *)&gtPosCom.szCertData, "%02X%-20.20s", ucCertType, szCertNo);
			
			return 0;
		}
	}	 
}


int EMVIF_ReferProc(void)  //only for disp,ex boolean EmvIF_GetReferralResults(EMVLanguages *pelsLangs,boolean blRemove)
{
	uchar bValue=0,lc,TranData[200],OutLen=0,bRet;
	uchar outdata[200],status[2];
//	uchar IssuerLen,IssuerAuthdata[16],i,CryInf,bCID;
	char sDisp[100];
	
	ClrBelow(1);

	sprintf(sDisp,"%s",glTStandMess[glbLanguage].sCallYourBank);
	lcdshow(1,MIDDLE,1,sDisp);
		
	printf(sDispPAN[glbLanguage]);
	lcdshow(2,MIDDLE,1,sDispPAN[glbLanguage]);
	//dat_bcdtoasc(outdata,pDataDict[4].pContent,pDataDict[4].len*2);
	strcpy(outdata,gtPosCom.szCardNo);
	/*
	for(i=0;i<pDataDict[4].len;i++)
	{
		printf("%02x",pDataDict[4].pContent[i]);
	}
	*/
	printf("%s\n",outdata);
	lcdshow(2,RIGHT,0,outdata);
	
	printf(sDispSelect[glbLanguage]);
	/*  //remove refer su42
	if(Flag ==CARD)
	{
		// at the beginning of the transaction-->force online.?
		printf(sDispReferralCard[glbLanguage]);   
		lcdshow(3,LEFT,1,sDispReferralCard[glbLanguage]);       
		bValue=press_key();
		if(bValue=='1')
		{
			//AAR-->ARQC 	
			//send an authorization or financial request message
			glTVR[3] = glTVR[3] | 0x08;
			if(bRet =OnlineProcessing())
				return bRet;
			return 0;	
		}
	}
	bValue -=1;
	*/
	
	/*
	if(Flag ==ISSUER)
	{
		//Issuer Authentication Data
		IssuerLen =pDataDict[36].len;
		if(IssuerLen)
		{
			memcpy(IssuerAuthdata,pDataDict[36].pContent,IssuerLen);
			EMVExternalAuthenticate(IssuerLen,IssuerAuthdata,status);
			glTSI[0] =glTSI[0] | 0x10;		
			if(!(memcmp(status,"\x90\x00",2)==0 ||
			   memcmp(status,"\x62\x83",2)==0 ||
			   (status[0]==0x63 && (status[1]&0xc0)==0xc0)
			  ))
			{
			//if(memcmp(status,"\x90\x00",2)	!=0)//6985 terminate要区别出03/03/18
				glTVR[4] = glTVR[4] | 0x40;
				return 1;
			}

			//if(memcmp(status,"\x90\x00",2)	!=0)//6985 terminate 要区别出03/03/18
			//	glTVR[4] = glTVR[4] | 0x40;

		}	
	}
	*/
	 
//	if(Flag ==ISSUER)
	while(1)
	{
	//printf("%s\n","Select 1.Approved  2.Declined  ");
		printf(sDispReferralIssuer[glbLanguage]);
		lcdshow(3,LEFT,1,sDispReferralIssuer[glbLanguage]);
		bValue=press_key();
		if(bValue=='1' || bValue=='2')
			break;
	}
	
	if(bValue=='1')
		return REFER_APPROVE;
	else
		return REFER_DENIAL;
	/*
	switch(bValue)
	{
		case '1': // approved 
			//Authorization Response Code
			//memcpy(pDataDict[30].pContent,"09",2);
			// not go online
			if(Flag ==CARD)
				memcpy(pDataDict[30].pContent,"Y2",2);
			// online decline
			//memcpy(pDataDict[30].pContent,"00",2);
			if(Flag ==ISSUER)
				IssuertoCardScriptProcessing(1);

	//second generate AC--cdol2
	BuildTDOL();
	buildDOL(0x8d);
	lc = (uchar )pDataDict[ 32].len;
	memcpy(TranData,pDataDict[ 32].pContent,lc);

			if(EMVGenerateAC(0x40,lc,TranData,&OutLen,outdata,status,0 )!=0)//tc	   	
				return 0xfe;// terminal tranaction 
			if(Flag ==ISSUER)
				IssuertoCardScriptProcessing(2);
			CryInf =gltDictContent.CryData;
			if((gltDictContent.AIP[ 0] & 0x01)==0x01 && 
				(gltDictContent.TermCapa[2] & 0x08)==0x08 &&
				( (glTVR[0] & 0x04)==0 ) && //geticcpk OK 09/04/21
				(CryInf &0xc0) ==0x40
			  )
			{
				if( (bRet=offlineCombineDSV(2,&bCID)))
				{
					//liantest
					printf("aofflineCombineDSV[%02x]\n",bRet);
					
					glTVR[0] = glTVR[0] | 0x04;
				}
			}

			break;
				
		case '2': // declined 
			//memcpy(pDataDict[30].pContent,"10",2);
			//not online
			if(Flag ==CARD)
				memcpy(pDataDict[30].pContent,"Z2",2);
			//online decline
			//memcpy(pDataDict[30].pContent,"51",2);
			if(Flag ==ISSUER)
				IssuertoCardScriptProcessing(1);

	//second generate AC--cdol2
	BuildTDOL();
	buildDOL(0x8d);
	lc = (uchar )pDataDict[ 32].len;
	memcpy(TranData,pDataDict[ 32].pContent,lc);
			
			if(EMVGenerateAC(0x00,lc,TranData,&OutLen,outdata,status,0 )!=0)//aac	   	
				return 0xfe;// terminate
			if(Flag ==ISSUER)
				IssuertoCardScriptProcessing(2);

			break;
	}
	
	return 0;	
	*/
	
}

#endif
/*
	transtype
	  0- Authorisation
	  1- Financial Transaction Request
	  2- Financial Transaction Confirmation
      3- Batch Data Capture
            -financail
      4- Batch Data Capture
            -offline advice
      5- Online Advice
      6- Reversal

      7- swipe proc
      8- Financial Record
	  
*/

uchar SetOnlineData(T_EMVTAG *pstList, uchar *psTLVData, int *piDataLen)
{
	int		i, iRet, iLength;
	uchar	*psTemp, sBuff[300];

	*piDataLen = 0;
	psTemp     = psTLVData;

	//psTemp++;// add transtype
	
/*
//liantest
iRet = EMV_GetTLVData(pstList[0].nTag, sBuff, &iLength);
printf("iRet[%x] len[%i] [%i]\n",iRet,iLength,TAG_NULL_1);
 
for(i=0;i<36;i++)
{
//	iRet = EMV_GetTLVData(stonlineFinancial[i].nTag, sBuff, &iLength);
	iRet = EMV_GetTLVData(pstList[i].nTag, sBuff, &iLength);
//liantest
//	printf("i=%i\n",i);

	printf("tag[%x] len[%i]\n ",pstList[i].nTag,iLength);
	press_key();

}
*/

	for(i=0; pstList[i].nTag!=TAG_NULL_1; i++)
//	for(i=0; i<36; i++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		//iRet = EMV_GetTLVData(pstList[i].nTag, sBuff, &iLength);
		iRet = QPboc_GetTLVData(pstList[i].nTag, sBuff, &iLength);
		
//		iRet = EMV_GetTLVData(stonlineFinancial[i].nTag, sBuff, &iLength);

		if( iRet==EMV_OK )
		{
			BuildTLVString(pstList[i].nTag, sBuff, iLength, &psTemp);
//			BuildTLVString(stonlineFinancial[i].nTag, sBuff, iLength, &psTemp);
		}
		/* //09/07/14
		else if( pstList[i].bOption==DE55_MUST_SET )
		{	// 如果必须存在的TAG不存在,交易失败
			if(pstList[i].nTag==0x9f37) 
			{   //在测试ADVT 17case时需要设置的TAG
				memset(sBuff,0,sizeof(sBuff));
				BuildTLVString(pstList[i].nTag, sBuff, 4, &psTemp);
				continue;
			}
			return E_TRANS_FAIL;
		}
		*/
	}
	*piDataLen = (psTemp-psTLVData);


	return OK_MIS;
}

uchar SetTransLogData(T_EMVTAG *pstList, uchar *psTLVData, int *piDataLen)
{
	int		i, iRet, iLength;
	uchar	*psTemp, sBuff[128];

	*piDataLen = 0;
	psTemp     = psTLVData;
	for(i=0; pstList[i].nTag!=TAG_NULL_1; i++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMV_GetTLVData(pstList[i].nTag, sBuff, &iLength);
		
		if( iRet==EMV_OK )
		{
			if(iLength<pstList[i].bOption) //Var len
			{
				*psTemp = iLength;  //len only for <=0xff
				memcpy(psTemp+1,sBuff,pstList[i].bOption-1);//以最大长度增充
			}
			else
			memcpy(psTemp,sBuff,pstList[i].bOption);//以最大长度增充
			//BuildTLVString(pstList[i].uiEmvTag, sBuff, iLength, &psTemp);
		}
		psTemp+=pstList[i].bOption;
		/*
		else if( pstList[i].ucOption==DE55_MUST_SET )
		{	// 如果必须存在的TAG不存在,交易失败
			if(pstList[i].uiEmvTag==0x9f37) 
			{   //在测试ADVT 17case时需要设置的TAG
				memset(sBuff,0,sizeof(sBuff));
				BuildTLVString(pstList[i].uiEmvTag, sBuff, 4, &psTemp);
				continue;
			}
			return E_TRANS_FAIL;
		}
		*/
	}
	*piDataLen = (psTemp-psTLVData);

	return OK_MIS;
}

#if 0
uchar GenSendData(int nSendLen,uchar * sBuff,unsigned int * nLen)
{

	unsigned int nOffset=0,nTmp,i;
	unsigned char bLrc=0x02;
	unsigned char sLocBuf[1200];

	//liantest
	printf("GenSendData[%i]\n",nSendLen);

	memcpy(sLocBuf,sBuff,nSendLen);
	//ARQC(0x9f26)
	sBuff[0] =0x02;

	sBuff[1] = (nSendLen)/0x100;
	sBuff[2] = (nSendLen)%0x100;
	nOffset = nSendLen +2;
	memcpy(&sBuff[3],sLocBuf,nSendLen);

	//liantest
	printf("offset[%i]\n",nOffset);
	
	for(i=0;i<nOffset;i++)
	{
		bLrc ^=sBuff[i+1];
		
	}
	nOffset++; //add head
	sBuff[nOffset++] = bLrc;
	*nLen = nOffset;

	//liantest
	printf("*nLen[%i]\n",*nLen);

	return 0;
	
}
#endif

uchar EMVSendAndReceiveForBatch(int nSendLen,uchar *SendBuff,uchar time,uchar bReceFlag,int *pnReceLen,uchar *ReceBuff)
{
	uchar tmpBuff[700],bValue=0;
	int i,nRecNum;
	int nTotalLen;
	int nHeadTime;
      
#ifdef KF311_M
        return 0;
#else
	// use Authorisation aip+type??
//liantest
TraceDisp("open com\n");
	
	//if need reverse,create reverse  090529
	if( open_com( 3, 115200, 5 ) )   //5s
	{
		TraceDisp("Com1 init err");
		lcdshow(2,MIDDLE,1,"Com1 init err");
		getch();
	}

//liantest
TraceDisp("open com OK[%i] \n",nSendLen);
	 
	
	//send buff
	GenSendData(nSendLen,SendBuff,&nTotalLen);
//	nRecNum = glRecNum;
	//test
//	printf("RecNum[%i]",nRecNum);
//	getch();

//liantest
TraceDisp("send len [%i]\n",nTotalLen);

	if(time==0)
		nHeadTime = 1;
	else
		nHeadTime = time;
		 
	for(;;)
	{
		//for(i=0;i<nHeadTime;i++)
		for(;;)	
		{
			//sendcomm(0x06);
			tmpBuff[0] = 0x06;
//liantest
TraceDisp("send data\n");
			send_str_com(1,tmpBuff);
			
			tmpBuff[0] = 0;
			rece_s(tmpBuff);
//		 	delay_us(200);
//liantest
TraceDisp("rece data\n");
			if(tmpBuff[0] ==0x06)
				break;
			//if((bValue=rececomm(5000))==0x06)
			//{
			//	break;
			//}
			if(getkey()==KEY_ESC)
				return 0xf0;
		
		}
		//for(i=0;i<nLen;i++)
		//	sendcomm(SendBuff[i]);
			
		send_str_com(nTotalLen,SendBuff);
		
		break;
		
		// batch capture data 	
		//if(nRecNum)
		//{
		//	GenSendDataLog(SendBuff,&nLen,nRecNum-1);
		//	nRecNum--;
		//}
		//else
		//	break;
		
    }
	//batch data clear
	//glRecNum =0; //disable 03/12/09

	if(bReceFlag)
	{
	for(i=0;i<5;i++)
	{
		//sendcomm(0x05);
		tmpBuff[0] = 0x05;
		send_str_com(1,tmpBuff);
		
		//if((bValue=rececomm(5000))==0x05)
		//{
		//	break;
		//}
		tmpBuff[0] = 0;
		rece_s(tmpBuff);
		if(tmpBuff[0] ==0x05)
			break;
		
	}
	if(i==5)
	{
		//close_com();
		return 0xf1;
	}
	//tmpBuff[0] =rececomm(5000);
	//if(tmpBuff[0]!=0x02)
	//	return 2;
	tmpBuff[0] = 0;
	if(read_com(1,tmpBuff)!=0)
	{
		//close_com();
		return 0xf1;
	}
	if(tmpBuff[0]!=0x02)
	{
		//close_com();
		return 0xf2;
	}
		 
	//tmpBuff[1] =rececomm(5000);
	//tmpBuff[2] =rececomm(5000);
	if(read_com(2,&tmpBuff[1])!=0)
	{
		//close_com();
		return 0xf1;
	}
	nTotalLen = tmpBuff[1]*0x100+tmpBuff[2];


	//for(i=0;i<nLen;i++)
	//{
	//	tmpBuff[3+i] = rececomm(5000);
	//	if(COMERR)
	//		return 3;
	//}
	if(read_com(nTotalLen,&tmpBuff[3])!=0)
	{
		//close_com();
		return 0xf3;
	}
	*pnReceLen = nTotalLen;
	memcpy(ReceBuff,&tmpBuff[3],nTotalLen);
	
	//tmpBuff[3+i] = rececomm(5000);
	if(read_com(1,&tmpBuff[3+nTotalLen])!=0)
	{
		//close_com();
		return 0xf3;
	} 

	bValue = tmpBuff[0];
	for(i=0;i<nTotalLen+3;i++)
		bValue^=tmpBuff[i+1];
	if(bValue!=0)
	{
		//close_com();
		return 0xf4;//lrc err
	} 

	}

 
//liantest
//printf("\nret[%i] host len[%i] script1[%i]",i,nLen,pDataDict[17].len);
//for(i=0;i<nLen+4;i++)
//	printf("%02x ",tmpBuff[i]);

//	close_com(); 
	TraceDisp("com ok\n");
	return 0;
#endif	
}



/*
uchar EMVSendAndReceiveByTcp(int nSendLen,uchar *SendBuff,uchar time,uchar bReceFlag,int *pnReceLen,uchar *ReceBuff)
{
	uchar tmpBuff[700],bValue=0;
	int i,nRecNum;
	int nTotalLen;
	int nHeadTime;
	int ret;
	

	*pnReceLen = 0;

	 
	GenSendData(nSendLen,SendBuff,&nTotalLen);
	printf("send len [%i]\n",nTotalLen);

	if(time==0)
		nHeadTime = 1;
	else
		nHeadTime = time;
	if(gbTimeout>nHeadTime)
		nHeadTime = gbTimeout;
	
	if(connectFlag != 0)
	{
		if(!ConnectServer())   //初始化HTTP连接		
		{
		printf("http连接不成功");
		lcdshow(2,MIDDLE,1,"http连接错误");
		press_key();
		return 0xff;
		}
	}
	
	for(;;)
	{
		for(i=0;i<1552;i++) //超过1550次就超时退出
		{
			tmpBuff[0] = '0x06';
			ret = send(socketfd, &tmpBuff[0], 1, 0);
			printf("send data kk_1 [%c] && ret = [%d]\n",tmpBuff[0], ret);

			
			tmpBuff[0] = 0;
			ret = recv(socketfd, &tmpBuff[0], 1, 0);
			printf("rece data kk_2 [%02x] && ret = [%d]\n",tmpBuff[0], ret);

			
			if(tmpBuff[0] =='6')
				{
					printf("NOW BREAK\n"); // 握手结束
					break;
				}
				
			if(getkey()==KEY_ESC)
				return -1;

			if(i == 1550)
				{
					ClearAndWriteLCD("通讯超时");
					press_key();
					return 2;
				}
			
			if(ret == 0)
				{
					connectFlag = 1;
					close(socketfd);//关闭socket
					ClearAndWriteLCD("网络失去连接");
					press_key();
					return 2;
				}
			
		}	
			printf("run here 5 ===握手成功\n");
			lcdshow(3, MIDDLE, 0, "握手成功");		
		//send_str_com(nTotalLen,SendBuff);
		ret = send(socketfd, SendBuff, nTotalLen, 0);
		printf("send data kk_3 [%02x] && ret = [%d]\n",SendBuff, ret);
		
		break;
				
    }

	if(bReceFlag)
	{
	for(i=0;i<1052;i++) //超过1050次就超时退出
	{
		//sendcomm(0x05);
		//tmpBuff[0] = 0x05;
		//send_str_com(1,tmpBuff);

		tmpBuff[0] = '0x05';
		ret = send(socketfd, &tmpBuff[0], 1, 0);
		printf("send data kk_4 [%c] && ret = [%d]\n",tmpBuff[0], ret);
			
		

		tmpBuff[0] = 0;
		//rece_s(tmpBuff);

		ret = recv(socketfd, &tmpBuff[0], 1, 0);
		printf("recv data kk_5 [%02x] && ret = [%d]\n",tmpBuff[0], ret);
			
		if(tmpBuff[0] =='5')
			{
				printf("break\n");
				break;
			}

		if(getkey()==KEY_ESC)
				return -1;
			if(ret == 0)
				{
					connectFlag = 1;
					close(socketfd);//关闭socket
					ClearAndWriteLCD("网络失去连接");
					press_key();
					return 2;
				}
		
	}
	if(i==5)
	{
		//close_com();
		return 0xf1;
	}

	tmpBuff[0] = 0;
	//if(read_com(1,tmpBuff)!=0)
	if(read_http(1,tmpBuff)!=0)
	{
		//close_com();
		return 0xf1;
	}
	if(tmpBuff[0]!=0x02)
	{
		//close_com();
		return 0xf2;
	}
		
	//if(read_com(2,&tmpBuff[1])!=0)
	if(read_http(2,&tmpBuff[1])!=0)
	{
		//close_com();
		return 0xf1;
	}
	nTotalLen = tmpBuff[1]*0x100+tmpBuff[2];

	//if(read_com(nTotalLen,&tmpBuff[3])!=0)
	if(read_http(nTotalLen,&tmpBuff[3])!=0)
	{
		//close_com();
		return 0xf3;
	}
	*pnReceLen = nTotalLen;
	memcpy(ReceBuff,&tmpBuff[3],nTotalLen);
	
	//tmpBuff[3+i] = rececomm(5000);
	
	//if(read_com(1,&tmpBuff[3+nTotalLen])!=0)
	if(read_http(1,&tmpBuff[3+nTotalLen])!=0)
	{
		//close_com();
		return 0xf3;
	} 

	bValue = tmpBuff[0];
	for(i=0;i<nTotalLen+3;i++)
		bValue^=tmpBuff[i+1];
	if(bValue!=0)
	{
		//close_com();
		return 0xf4;//lrc err
	} 

	}  
 
	printf("com ok\n");
	return 0;
}
*/

uchar EMVSendAndReceive(int nSendLen,uchar *SendBuff,uchar time,uchar bReceFlag,int *pnReceLen,uchar *ReceBuff)
{
  return 0;
#if 0
//本函数用于使用COM口下载参数，在本函数下面的函数用于在HTTP下载参数
	uchar tmpBuff[700],bValue=0;
	int i,nRecNum;
	int nTotalLen;
	int nHeadTime;


	//显示选择分支
	clear_lcd(); //清除LCD内容
	lcdshow(1,MIDDLE,0,"请选择下载方式:");
	lcdshow(2,MIDDLE,0,"(1):TCP方式");
	lcdshow(3,MIDDLE,0,"(2):串口方式");
	printf("等待用于输入..\n");
	while(1)
	{
		bValue=press_key();
		if(getkey()==KEY_ESC)
				return -1;
		if(bValue==0x31 || bValue==0x32)
			break;
		else
			continue;
	}
	switch(bValue-0x30)
	{
		case 1:  
		ClearAndWriteLCD("等待后台发送数据...");
		EMVSendAndReceiveByTcp(nSendLen, SendBuff, time, bReceFlag, pnReceLen, ReceBuff);//跳转
		return 0; //返回
			break;
		case 2:  //直接运行下面
			break;
		default:
			break;
	}

	// use Authorisation aip+type??
//liantest
printf("open com\n");
	*pnReceLen = 0;
	//if need reverse,create reverse  090529
	if( open_com( 3, 115200, 5 ) )   //5s

	{
		printf("Com1 init err");
		lcdshow(2,MIDDLE,1,"Com1 init err");
		getch();
	}

//liantest
printf("open com OK[%i] \n",nSendLen);
	 
	
	//send buff
	GenSendData(nSendLen,SendBuff,&nTotalLen);
//	nRecNum = glRecNum;
	//test
//	printf("RecNum[%i]",nRecNum);
//	getch();

//liantest
printf("send len [%i]\n",nTotalLen);

	if(time==0)
		nHeadTime = 1;
	else
		nHeadTime = time;
	if(gbTimeout>nHeadTime)
		nHeadTime = gbTimeout;
//liantest
printf("nHeadTime[%i]\n",nHeadTime);
nHeadTime=1;//test!!!
	for(;;)
	{
		for(i=0;i<nHeadTime;i++)
		{
			//sendcomm(0x06);
			tmpBuff[0] = 0x06;
//liantest
printf("send data\n");
			send_str_com(1,tmpBuff);
			
			tmpBuff[0] = 0;
			rece_s(tmpBuff);
//liantest
printf("rece data\n");
			if(tmpBuff[0] ==0x06)
				break;
			//if((bValue=rececomm(5000))==0x06)
			//{
			//	break;
			//}
			if(getkey()==KEY_ESC)
				return 0xf0;
		//	delay_us(2);
		}
		if(i==nHeadTime)
		{
			//close_com();		
			return 1;
		}	
		//for(i=0;i<nLen;i++)
		//	sendcomm(SendBuff[i]);
		delay_ms(100);	
		send_str_com(nTotalLen,SendBuff);
		
		break;
		
		// batch capture data 	
		//if(nRecNum)
		//{
		//	GenSendDataLog(SendBuff,&nLen,nRecNum-1);
		//	nRecNum--;
		//}
		//else
		//	break;
		
    }
	//batch data clear
	//glRecNum =0; //disable 03/12/09

	if(bReceFlag)
	{
	for(i=0;i<5;i++)
	{
		//sendcomm(0x05);
		tmpBuff[0] = 0x05;
		delay_ms(100);	
		send_str_com(1,tmpBuff);
		
		//if((bValue=rececomm(5000))==0x05)
		//{
		//	break;
		//}
		tmpBuff[0] = 0;
		rece_s(tmpBuff);
		if(tmpBuff[0] ==0x05)
		{
			break;
		}
		delay_ms(100);	

	}
	if(i==5)
	{
		//close_com();
		return 0xf1;
	}
	//tmpBuff[0] =rececomm(5000);
	//if(tmpBuff[0]!=0x02)
	//	return 2;
	tmpBuff[0] = 0;
	if(read_com(1,tmpBuff)!=0)
	{
		//close_com();
		return 0xf1;
	}
	if(tmpBuff[0]!=0x02)
	{
		//close_com();
		return 0xf2;
	}
		
	//tmpBuff[1] =rececomm(5000);
	//tmpBuff[2] =rececomm(5000);
	if(read_com(2,&tmpBuff[1])!=0)
	{
		//close_com();
		return 0xf1;
	}
	nTotalLen = tmpBuff[1]*0x100+tmpBuff[2];


	//for(i=0;i<nLen;i++)
	//{
	//	tmpBuff[3+i] = rececomm(5000);
	//	if(COMERR)
	//		return 3;
	//}
	if(read_com(nTotalLen,&tmpBuff[3])!=0)
	{
		//close_com();
		return 0xf3;
	}
	*pnReceLen = nTotalLen;
	memcpy(ReceBuff,&tmpBuff[3],nTotalLen);
	
	//tmpBuff[3+i] = rececomm(5000);
	if(read_com(1,&tmpBuff[3+nTotalLen])!=0)
	{
		//close_com();
		return 0xf3;
	} 

	bValue = tmpBuff[0];
	for(i=0;i<nTotalLen+3;i++)
		bValue^=tmpBuff[i+1];
	if(bValue!=0)
	{
		//close_com();
		return 0xf4;//lrc err
	} 
	}  
 
 
//liantest
//printf("\nret[%i] host len[%i] script1[%i]",i,nLen,pDataDict[17].len);
//for(i=0;i<nLen+4;i++)
//	printf("%02x ",tmpBuff[i]);


	//close_com();
	printf("com ok\n");
	return 0;
#endif
}

uchar SeparateData(int nRecLen,uchar *ReceBuff,unsigned char *RspCode, unsigned char *AuthCode,unsigned int *AuthCodeLen, unsigned char *IAuthData,unsigned int *IAuthDataLen, unsigned char *script71, int *ScriptLen71,unsigned char *script72,unsigned int *ScriptLen72)
{
	unsigned int tag,len,LengthTmp;
	uchar * pDataOut,*pDataInTmp;
	extern unsigned char *SeparateBER_TLV (  unsigned char *p,  unsigned int *tag,  unsigned int *len);
	
	
	LengthTmp = nRecLen;
	pDataInTmp=ReceBuff;
	while(LengthTmp)
	{
		pDataOut =SeparateBER_TLV (pDataInTmp,&tag,&len);
		if(pDataOut==NULL && tag ==0)
			return 0;
		else if(pDataOut==NULL)
		{
			  
			//if(DataNum==1)//Application Label, 070427 disable book1 12.2.5
			//	return 0;
			return 1;
		}
		LengthTmp = LengthTmp-(pDataOut-pDataInTmp);
		
        pDataInTmp = pDataOut + len;
	
		switch(tag)
		{
			case 0x8a:
				memcpy(RspCode,pDataOut,2);
				break;
			case 0x89:
				memcpy(AuthCode,pDataOut,6);
				*AuthCodeLen =6;
				break;
			case 0x91:
				memcpy(IAuthData,pDataOut,len);
				*IAuthDataLen = len;
				break;
			case 0x71:
				memcpy(script71,pDataOut,len);
				*ScriptLen71 = len;
				break;
			case 0x72:
				memcpy(script72,pDataOut,len);
				*ScriptLen72 = len;
				break;
			default:
				break;
		}
		LengthTmp -= len;
		
	}      

}  


void MakeTranReadyForPayICData(unsigned char *pBuf,unsigned short *pnLen)
{
	unsigned short nOffset;
	unsigned char *p;
	struct tm sTime;
	unsigned char sBuf[200];
	unsigned char sOtherBuf[100];
	int nLen;
	
	nOffset = 0;
	p = pBuf;
	*p = 0x91;//QPBOC ?? MSD when set --方案标识号
	p+=1;
	gettime(&sTime);
	sprintf(sBuf,"20%02d%02d%02d%02d%02d%02d",sTime.tm_year,sTime.tm_mon,sTime.tm_mday,
		     sTime.tm_hour,sTime.tm_min,sTime.tm_sec);
	dat_asctobcd(p, sBuf, 14);
	p+=7;
	memcpy(p,"\xd1\x00",2); //track1 no exist
	p+=2;

	Contactless_GetTLVData(0x57,sBuf,&nLen);//track2
	*p++=0xd2;
	*p++=(uchar)nLen;
	memcpy(p,sBuf,nLen);
	p+=nLen;
	//芯片数据
	SetOnlineData(stTranReadyForPayIcData,sBuf,&nLen);
	*p++=0xd3;
	//not more than 0x7f
	*p++=nLen;
	memcpy(p,sBuf,nLen);
	p+=nLen;
	
	//其它数据
	Contactless_GetTLVData(0x9f74,sBuf,&nLen);//电子现金发行者授权码
	memcpy(sOtherBuf,"\x9f\x74",2);
	sOtherBuf[2] = nLen;
	memcpy(&sOtherBuf[3],sBuf,nLen);
	nOffset = nLen+3;
	nLen=0;
	Contactless_GetTLVData(0x9f5d,sBuf,&nLen);//有效的脱机交易金额
	if(nLen)
	{
	memcpy(&sOtherBuf[nOffset],"\x9f\x5d",2);
	nOffset +=2;
	sOtherBuf[nOffset++] = nLen;
	memcpy(&sOtherBuf[nOffset],sBuf,nLen);
	nOffset += nLen;
	}
	
	memcpy(&sOtherBuf[nOffset],"\x99\x01\x00",3);// not support online pin
	nOffset +=3;
	memcpy(&sOtherBuf[nOffset],"\x55\x00",2); //未签名
	nOffset +=2;
	memcpy(&sOtherBuf[nOffset],"\x44\x00",2); //not support offline pin
	nOffset +=2;
	memcpy(&sOtherBuf[nOffset],"\x01\x00",2); //DDA 成功
	nOffset +=2;
	
	*p++=0xd4;
	*p++=nOffset;
	memcpy(p,sOtherBuf,nOffset);
	p+=nOffset;
    
	*pnLen = p-pBuf;
	
	
}



#if 0
int  EMVIF_OnlineProc(unsigned char *RspCode, unsigned char *AuthCode,unsigned int *AuthCodeLen, unsigned char *IAuthData,unsigned int *IAuthDataLen, unsigned char *script71,unsigned int *ScriptLen71,unsigned char *script72,unsigned int *ScriptLen72)
{
	unsigned char SendBuff[1000],ReceBuff[1000];
	int nLen,nRecLen,fid;
	uchar bRet;
	uchar nLocRspCode[2];
	unsigned int nLenAuthCode,nLenIAuthData,nLen71,nLen72;
	
	uchar i;
//liantest
//printf("EMVIF_OnlineProc sOptions:");
//for(i=0;i<10;i++)
//	printf("%02x ",gtEmvParameter.sOptions[i]);
//printf("\n");

   
	if(EMV_OPTIONS(gtEmvParameter.sOptions,OFFSET_OnlineCapture_M))//Financial
	{
		//liantest
		printf("online capture\n");
		SendBuff[0] = 1;//type
  
		
		bRet = SetOnlineData(stonlineFinancial,SendBuff,&nLen);
		printf("\nbRet = %i\n",bRet);
	}
	else  
	{
		//liantest
		printf("batch  capture\n");
		SendBuff[0] = 0;//type
		
		SetOnlineData(stAuthorisation,SendBuff,&nLen);
		
	}
 

	bRet = EMVSendAndReceive(nLen,SendBuff,5,1,&nRecLen,ReceBuff);

//liantest
printf("nRecLen[%i]\n",nRecLen);

	if( (nRecLen==2) || (bRet>0xf1) ) //not rece data or rece not all,reverse,unable to go online)
	{
		//creat(REVERSAL_FILE,0777);
		fid=open(REVERSAL_FILE, O_RDWR | O_CREAT | O_SYNC);
		close(fid);
		return ONLINE_FAILED;

		//return 0xf0;
	}
	
	SeparateData(nRecLen,ReceBuff, RspCode, AuthCode,AuthCodeLen, IAuthData,IAuthDataLen, script71,ScriptLen71,script72,ScriptLen72);
  
	//liantest
//	printf("RspCode[%02x][%02x]\n",RspCode[0],RspCode[1]);
	memcpy(nLocRspCode,RspCode,2);
	if(memcmp(nLocRspCode,"\x60\x60",2)==0)
	{
		 memcpy(_gnRespCode,nLocRspCode,2); //09/08/12
		//creat(REVERSAL_FILE,0777);
		fid=open(REVERSAL_FILE, O_RDWR | O_CREAT | O_SYNC);
		close(fid);
		
		//liantest
		printf("return 0xf0");
		return ONLINE_FAILED;
		//return 0xf0;          
	}
	memset(_gnRespCode,0,sizeof(_gnRespCode) );
	
//	*AuthCodeLen = nLenAuthCode;
//	*IAuthDataLen = nLenIAuthData;
//	*ScriptLen71 = nLen71;
//	*ScriptLen72 = nLen72;

	return bRet;
	
}
#endif

#if 0
int EMVIF_FinancialConfirm(void)
{
	unsigned char SendBuff[1024],ReceBuff[1000];
	int nLen,nRecLen;
	uchar bRet;

	SendBuff[0] = 2; //type
	bRet = SetOnlineData(stonlineFinancialConfirm,SendBuff,&nLen);

	
	bRet = EMVSendAndReceive(nLen,SendBuff,5,0,&nRecLen,ReceBuff);
	
  	return bRet; //100601
}

int EMVIF_ReversalProc()
{
	unsigned char SendBuff[1024],ReceBuff[1000];
	int nLen,nRecLen;
	uchar bRet;
	extern uchar _gbTransResult;

	if( fexist(REVERSAL_FILE)<0 )
	{
		return OK_MIS;
	}
   
	if(!EMV_OPTIONS(gtEmvParameter.sOptions,OFFSET_OnlineCapture_M))
	{
		//create advcie //09/08/12
		EMVIF_AdviceProc();
		//return OK;
	}
	memset(SendBuff,0,sizeof(SendBuff) );
	     
	SendBuff[0] = 6;
         
	bRet = SetOnlineData(gtReversalTagList, SendBuff, &nLen);

//	delay_us(300);

	//bRet = EMVSendAndReceive(nLen,SendBuff,5,0,&nRecLen,ReceBuff);
	bRet = EMVSendAndReceiveForBatch(nLen,SendBuff,5,0,&nRecLen,ReceBuff); //09/8/19
	
	remove(REVERSAL_FILE);

	//send final record
	if(_gbTransResult==APPROVE_M)
	{

		memset(SendBuff,0,sizeof(SendBuff) );
		SendBuff[0] = 8;  //Financial Record 
		SetOnlineData(stBatchData,SendBuff,&nLen);
//		delay_us(300);

		//bRet = EMVSendAndReceive(nLen,SendBuff,5,0,&nRecLen,ReceBuff);
		bRet = EMVSendAndReceiveForBatch(nLen,SendBuff,5,0,&nRecLen,ReceBuff);

	} 
	  
}

void EMVIF_BatchProc(void)
{
	unsigned char SendBuff[2000];
	int nLen;
	memset(SendBuff,0,sizeof(SendBuff) );
	SendBuff[0] = 3;
	SetOnlineData(stBatchData,SendBuff,&nLen);


	SaveBatchFile(nLen,SendBuff);


}

void EMVIF_BatchFinancialRecordProc(void)  //100601
{
	unsigned char SendBuff[2000];
	int nLen;
	memset(SendBuff,0,sizeof(SendBuff) );
	SendBuff[0] = 8;
	SetOnlineData(stBatchData,SendBuff,&nLen);


	SaveBatchFile(nLen,SendBuff);


}


void EMVIF_AdviceProc(void)
{
	unsigned char SendBuff[2000];
	int nLen,nReceLen;
	uchar nReceBuff[100];
	uchar bRet;
	SetOnlineData(stAdvice,SendBuff,&nLen);
	if(EMV_OPTIONS(gtEmvParameter.sOptions,OFFSET_OnlineCapture_M))//online data capture
	{
		SendBuff[0] = 5;//type-online advice
		EMVSendAndReceive(nLen,SendBuff,5,0,&nReceLen,nReceBuff); 
		printf("online data capture[%i]\n",nLen);
		/*
		if(EMVSendAndReceive(nLen,SendBuff,0,0,&nReceLen,nReceBuff)) 
		{
			memset(SendBuff,0,sizeof(SendBuff) );
			SetOnlineData(stBatchData,SendBuff,&nLen);
			SaveBatchFile(nLen,SendBuff);
		}
		*/ 
	}
	else
	{
		SetOnlineData(stBatchData,SendBuff,&nLen);
		
		SendBuff[0] = 4;//type offline advice(batch data capture)
		bRet=SaveAdviceFile(nLen,SendBuff);	
	}
	
}

void EMVMMI_VerifyPINOK(unsigned char bFlag)
{
    ClrBelow(2);
	if(bFlag)
		lcdshow(2,MIDDLE,0,glTStandMess[glbLanguage].sIncorrectPIN);
	else
		lcdshow(2,MIDDLE,0,glTStandMess[glbLanguage].sPINOK);
	WaitKey(2);
	
}

#endif

//处理DOL的过程中，EMV库遇到不识别的TAG时会调用该回调函数
int  EMVIF_UnknowTLVData(unsigned short Tag, unsigned char *pDat, int * pnLen)
{
	switch( Tag )
	{
	case 0x9F53:	// Transaction Category Code (TCC) - Master Card 
		*pDat = 0x52;
		*pnLen = 1;
		break;

	default:
		return -1;
	}

	return EMV_OK;	
}


void EMVIF_ClearDsp(void)
{
	return;
#if 0
	clear_lcd();
#endif
}

 
uchar WaitKey(unsigned int iTimeInt)
{
	unsigned long s_time, ul_time;
	unsigned char bKey;
        
        return 0;
/*
	time(&s_time);
	
	while(1)
	{ 
			
		if(bKey = getkey())
			return bKey;
		time(&ul_time);
		if( ul_time - s_time >= iTimeInt ) 
			break;
	}		
	return KEY_ESC;
 */
}

uchar WaitKeyTest(unsigned int iTimeInt)
{
	unsigned long s_time, ul_time;
	unsigned char bKey;
        
        return 0;
#if 0
	//fortest
	return getkey();
	
	time(&s_time);
	
	while(1)
	{ 
			
		if(bKey = getkey())
			return bKey;
		time(&ul_time);
		if( ul_time - s_time >= iTimeInt ) 
			break;
	}		
	return KEY_ESC;
#endif
}

#if 0
int  EMVIF_DisplayWait(int line_num, int place, int clear_line_flag, unsigned char *str,int iTimeInt)
{

	lcdshow(  line_num,  place, clear_line_flag, str );
	WaitKey(iTimeInt);
	return 0;
}
#endif

//int filesize(char *sName)
int EMVIF_FileSize(char *sName)	
{
	struct stat tFileStat;
        extern int stat(const char * file_name,struct stat *buf);
	memset(&tFileStat,0,sizeof(struct stat));
	if(stat(sName,&tFileStat)<0)
		return -1;
	return tFileStat.st_size;
	

}

uchar EMVIF_SearchExceptionList(unsigned char bLen ,unsigned char *rdPAN,unsigned char  *rdPANSeq)
{
	int fid,len;
	int num,i;
	T_TERMEXCEPTIONFILE tTermException;
	
	fid=open(FIEL_EXCEPTION_M,O_RDWR);
	if(fid<0) return fid;
	len=EMVIF_FileSize(FIEL_EXCEPTION_M);
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

int  EMVIF_AddExceptionList(T_TERMEXCEPTIONFILE  *ptException )
{
	int fid,len;
	int num,i;
	T_TERMEXCEPTIONFILE tTermException;
	
	fid=open(FIEL_EXCEPTION_M,O_RDWR);
	if(fid<0)
	{
		//creat(FIEL_EXCEPTION_M,0777);
		fid=open(FIEL_EXCEPTION_M,O_RDWR | O_CREAT | O_SYNC);
		if(fid<0)
			return fid;
	}
	len=EMVIF_FileSize(FIEL_EXCEPTION_M);
	num=len/sizeof(T_TERMEXCEPTIONFILE);
//liantest
TraceDisp("len%i num%i\n",len,num);

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
//liantest
TraceDisp("i=%i num=%i\n",i,num);
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


#if 0
uchar EMVIF_GetsumLogByPAN(unsigned char bLen,unsigned char *sPAN,unsigned int *nAmount)
{
	T_EMVTRANSREC tEmvTransRec;
	unsigned int nNum,i;
	int iLogFile,iRet;
	
	*nAmount = 0;
	nNum = gtCtrlParameter.nTransNum;
	if(nNum==0)
	{
		return 0;
	}
	iLogFile = open(TERM_LOG_FILE_NAME, O_RDWR);
	if( iLogFile<0 )
	{
		DispFileErrInfo();
		return E_FILE_OPEN;
	}
	
	for(i=0;i<nNum;i++)
	{
		iRet = lseek(iLogFile, (long)(i*LOG_RECORD_LEN), SEEK_SET);
		if( iRet<0 ) 
		{
			DispFileErrInfo();
			close(iLogFile);
			return E_FILE_SEEK;
		}
		read(iLogFile,(unsigned char*)&tEmvTransRec,LOG_RECORD_LEN);
		if(memcmp(sPAN,tEmvTransRec.Pan,bLen)) continue;//not match PAN
			(*nAmount)+=dat_bcdtobin(tEmvTransRec.sAmount,6); //convert?
					
		
	}
	return 0;
}


unsigned char TranAddLog(void)
{
	unsigned char SendBuff[2000];
	int nLen;
	memset(SendBuff,0,sizeof(SendBuff) );
	
	SetTransLogData(stTransLog,SendBuff,&nLen);
	memcpy(&SendBuff[nLen],"\x02\x00",2);
	nLen +=2;
	SaveTransLogFile(nLen,SendBuff);			
	
}


void EMVIF_GetRandNumberByNum(unsigned char bLen,unsigned char *prdRand) //09/08/17
{
#define RNG_IOC_MAGIC 'N'
#define RNG_WORD 		_IO(RNG_IOC_MAGIC, 0)
	  
	int fd_rng;
	int result;
	uchar i;  
//	fd_rng = open("/dev/rng", O_RDWR);  //move to platorm
        

	if (gnfd_rng < 0) {
		printf("%s - Can't open RNG!\n", __FUNCTION__);
		return ;
	}	

	if((result=ioctl(gnfd_rng,RNG_WORD, 0)) == -1) {
			/* calling ioctl failed so return */
			//perror("IOCTL: RNG_WORD failed \n");
			printf("IOCTL: RNG_WORD failed \n");
			return ;
	} 
	 
	//if (read(gnfd_rng,prdRand,sizeof(unsigned int)) == -1) {
	if (read(gnfd_rng,prdRand,bLen) == -1) {
			/* calling read failed so return */
			printf("Read Failed! \n");
			return ;
	}	
       
	//liantest
	//printf("random[%02x][%02x][%02x][%02x]\n",prdRand[0],prdRand[1],prdRand[2],prdRand[3]);
//	printf("random:");
//	for(i=0;i<bLen;i++)
//		printf("%02x",prdRand[i]);
//	printf("\n");
	
	//close(fd_rng);	
}

#endif

void EMVIF_GetRandNumber(unsigned int *prdRand)
{
#if 0  
#define RNG_IOC_MAGIC 'N'
#define RNG_WORD 		_IO(RNG_IOC_MAGIC, 0)
#endif	  
	int fd_rng;
	int result;

	srand(RTC_GetCounter());	
	//11/06/13
	//TraceDisp("rand RTC_GetCounter[%x]\n",RTC_GetCounter());
	
	*prdRand = rand();
#if 0        
//	fd_rng = open("/dev/rng", O_RDWR);  //move to platorm
	if (gnfd_rng < 0) {
		printf("%s - Can't open RNG!\n", __FUNCTION__);
		return ;
	}	

	if((result=ioctl(gnfd_rng,RNG_WORD, 0)) == -1) {
			/* calling ioctl failed so return */
			//perror("IOCTL: RNG_WORD failed \n");
			printf("IOCTL: RNG_WORD failed \n");
			return ;
	}
	 
	if (read(gnfd_rng,prdRand,sizeof(unsigned int)) == -1) {
			/* calling read failed so return */
			printf("Read Failed! \n");
			return ;
	}	
	//liantest
	printf("random[%02x][%02x][%02x][%02x]\n",prdRand[0],prdRand[1],prdRand[2],prdRand[3]);
#endif
	//close(fd_rng);	
}

#if 0
void EmvIF_Hash(unsigned char * DataIn,unsigned int DataInLen,unsigned char* DataOut)
{
	int fd_sha;
	int pos,result;
	fd_sha = open("/dev/sha", O_RDWR);
	if (fd_sha < 0) {
		printf("%s - Can't open SHA!\n", __FUNCTION__);
		return ;
	}

	pos = DataInLen;
	result = write (fd_sha, DataIn, pos);
	if (result == -ENOMEM) printf("ERROR: Could not allocate Memory\n");
	if (result == -EFAULT) printf("ERROR: Invalid Pointer Used\n");
	if (result < pos) printf("ERROR: %d bytes added, should be %d\n",result,pos); 
	
	result = read (fd_sha, DataOut, 20);
	if (result = 0) {
		printf("ERROR: No hash to recieve or read() failed");
	} else {
		if (result < 20) printf("ERROR: %d bytes copies to digest, should be 20",result);
	}


	close(fd_sha);


}


int EmvIF_RSAPublicKeyDecrypt(unsigned char *output, unsigned int *outputLen, unsigned char *input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKey)
{

	int	fCphr,Len;
	unsigned int 	Size = publicKey->bits;
	BSP_RSA_KEY		RsaKey;
	uchar sPubExp[256];
	
	fCphr = open("/dev/rsa", O_RDWR);
	if( fCphr < 0 )
	{
		printf("Unable to acquire RSA cipher\r\n");
		return fCphr;
	}

	/*
	 * Make a BSP_RSA_KEY structure.
	 */
	RsaKey.pMod      = publicKey->modulus;
	RsaKey.ModLen    = Size;
	
	memset(sPubExp,0,sizeof(sPubExp) );
	memcpy(&sPubExp[inputLen-3],publicKey->exponent,3);
	RsaKey.pPubExp   = sPubExp;
	RsaKey.PubLen    = Size;


	//ioctl( fCphr, CPHR_SET_KEY, (BSP_CPHR_KEY*)&RsaKey ); //guohonglv  原函数是这样的，不过ioctl这个函数的原型不知道该怎么写，就先写成下面的样子
        ioctl( fCphr, CPHR_SET_KEY, (int)&RsaKey ); //guohonglv
	
	/*
		 * RSA transform using Public key to verify the data.
		 */
	ioctl( fCphr, CPHR_SET_MODE, BSP_CPHR_MODE_ENCRYPT );
    printf("Calling write (public verify)\r\n");
	write( fCphr, input, inputLen );
	Len = read( fCphr, output, inputLen );
	printf( "RSA Read Len %u, data\r\n", Len );
	//CphrDisplay( EncodedData, Len );
   
	close( fCphr );
	*outputLen = inputLen;
	return 0;
}


void  EmvIF_des(unsigned char* data, unsigned char* key,unsigned short encrypt)
{
	int fCphr,Len;
	
	fCphr = open("/dev/des0", O_RDWR);
	if(fCphr<0)
	{
		printf("Unable to acquire DES cipher\r\n");
		//return fCphr;  //guohonglv //为什么会有返回值呢？是这里写错了，还是函数的类型写错了？
	}

	// Set Key
    //printf("Setting DES key...\r\n" );
	//ioctl( fCphr, CPHR_SET_KEY, (BSP_CPHR_KEY *)key ); //guohonglv 现在不知道ioctl该怎么写，就先写成下面的样子了
        ioctl( fCphr, CPHR_SET_KEY, (int)key ); //guohonglv

	if(encrypt==ENC_M)
	{
		// Encrypt
		ioctl( fCphr, CPHR_SET_MODE, BSP_CPHR_MODE_ENCRYPT );
		write( fCphr, data, 8 );
		Len = read( fCphr, data, 8 );
		printf( "DES Read Len %u, data\r\n", Len );
		//CphrDisplay( DesData, Len );
	}
	else //DEC_M
	{
		// Decrypt
		ioctl( fCphr, CPHR_SET_MODE, BSP_CPHR_MODE_DECRYPT );
		write( fCphr, data, 8 );
		Len = read( fCphr, data, 8 );
		printf( "DES Read Len %u, data\r\n", Len );
		//CphrDisplay( DesData, Len );
	}
	close( fCphr );
	//return 0;	 //guohonglv //为什么会有返回值呢？是这里写错了，还是函数的类型写错了？
}




int EMV_CardAuth(void)
{
	uchar bRet,sBuf[10];
	
	if((bRet=EMV_OfflineDataAuth()))
	{
			 
		//return CancelTrans("DDA intaut err");
		TraceDisp("EMV_OfflineDataAuth[%02x]",bRet);
		return EMV_DATA_ERR;
	}
	return 0;
}

int EMVProcTrans(void)
{
	uchar bRet,sBuf[10],sAIP[2];
	int nLen;
	
//liantest
printf("ProcessingRestrictions\n");
       
    if(bRet=EMV_ProcessRestrictions())
	{
		printf("ProcessingRestrictions[%02x]",bRet);
		//return CancelTrans(DispBuff);
		return EMV_DENIAL;
	}
     
 //liantest
//printf("CardholderVerification AIP[%02x]\n",gltDictContent.AIP[ 0]);
    EMV_GetTLVData(0x82,sBuf,&nLen); //AIP
	memcpy(sAIP,sBuf,2);
	if(sBuf[ 0] & 0x10)
	{
		bRet =EMV_CardholderVerification();
		//0xff with no Cardholder Verification Rules,2cl.044
		
			//test
		if(bRet)
		{
			printf("\nCardholderVerification[%02x]",bRet);
			//press_key();
		}
 	
		if(bRet ==0xfe || bRet ==0xff)
			//return CancelTrans("CardholderVerification err");
			return EMV_USER_CANCEL;
 
	} 
	else
	{
		memset(sBuf,0,sizeof(sBuf) );
		EMV_GetTLVData(0x9f34,sBuf,&nLen);//Cardholder Verification Method (CVM) Results
		sBuf[0] = 0x3f; //'3F' if no CVM is performed
		nLen = 3;
		EMV_SetTLVData(0x9f34,sBuf,nLen);
	}
	
//	if(gltDictContent.AIP[0] & 0x08)
//	if(sAIP[0] &0x08)  //10/02/02 ICS support TRM irrespective of AIP setting 
	{
		if(EMV_TerminalRiskManagement()==0xfe)
			//return CancelTrans("TerminalRiskManagement err");
			return EMV_DENIAL;
	}
	
	//liantest
	printf("TerminalRiskManagement finish\n");
	   
	if((bRet=EMV_TerminalAndCardActionAnalysis()))
	{
		//sprintf(DispBuff,"TerminalActionAnalysis[%02x]",bRet);
		//return CancelTrans("Declined");
		printf("TerminalActionAnalysis CancelTrans[%02x]\n",bRet);
	//	press_key();
		if(bRet==EMV_ONLINE)
		{
			if(EMV_OnlineProcessing()==0xfe)
				return EMV_USER_CANCEL;
		}
		else
		//return CancelTrans("Terminate");
			return EMV_USER_CANCEL;
	}
//liantest   
printf("TerminalActionAnalysis ok\n");

	return 0;
}


void EMVIF_PrintSignature()
{
	unsigned short unRet;

	gbPrintSignature = 1; 
	return ;  //for bctest
	
	print_b("");
	print_b("Signature:___________________");
	
	StartPrn();	

	if((unRet=prt_status())==0)
	{
		lcdshow(3,MIDDLE,1,"Print OK");
	}
	else if(unRet==PRINTER_NO_PAPER)
		lcdshow(3,MIDDLE,1,"no paper");
//	else if(unRet==PRINTER_HIGH_TEMP);
//		lcdshow(3,MIDDLE,1,"High Temperature");
	WaitKeyTest(2);
}

#endif
