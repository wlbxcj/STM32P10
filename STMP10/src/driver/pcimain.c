/*
PCI接口测试协议： 
命令包与响应包格式： 
命令头（ID）＋长度（高字节）＋长度（低字节）＋数据（LEN＝LEN＿高＊256＋LEN＿低）＋校验值
1字节　　　　　　1字节　　　　　1字节　　　　　　　　　　　　　　　　　　　　　　　　　1字节

 
0x90--0x9F:   PCI 

2009/6/8:V1.2
1) 增加FK测试功能接口。

2009/8/7:V2.2
1) 增加FK的加密PIN功能接口测试。
*/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
 
#include "vosapi.h" 
//#include "dll.h"
#include "pcicmd.h" 

//test
#include "emvcore.h"

#define  TEST_COM   VCOM_DEBUG //COM1   //COM2

#define s_printf trace_debug_printf //12/11/08

//#define USIP_GPIO_DR0           (0xA2005404) //Data register for GPIO[15..0] 
 
const APP_MSG App_Msg={
    "PCI-TEST",
    "PCI-TEST",
    "1.1",
    "VIEWAT",
    "",
    "",
    0,
    0,
    0,
    ""
};
//人民币符号￥
unsigned char g_Display_RMB[]={
0x10,0x10,0x00,0x40,0x40,0xc0,0x40,0x00,0x00,0x00,0x40,0xc0,0x40,0x40,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x08,0x09,0x0a,0xfc,0x0a,0x09,0x08,0x00,0x00,0x00,0x00,0x00,0x00
};
int event_main(SET_EVENT_MSG *msg)
{
    return 0;
} 

int CLib_PciAuth(uchar key_type,uchar key_no,uchar *inrand,uchar *outauth)
{      
	//uchar authkey[24];
	//memset(authkey,0x00,sizeof(authkey));
	//memcpy(authkey,"\x22\x22\x22\x22\x22\x22\x22\x22",8);
	//authkey[22]=key_type;
	//authkey[23]=key_no; 
	//Lib_Des(inrand, outauth,&authkey[0], 1);
    //Lib_Des(outauth, outauth,&authkey[8], 0);
    //Lib_Des(outauth, outauth,&authkey[16], 1); 
	//Lib_Des(inrand, outauth,"\x22\x22\x22\x22\x22\x22\x22\x22", 1);
	//Lib_Des(outauth, outauth,"\x11\x11\x11\x11\x11\x11\x11\x11", 0);
	//Lib_Des(outauth, outauth,"\x11\x11\x11\x11\x11\x11\x11\x11", 1);
    return 0;
} 
 
static uchar c_to_b(uchar bchar)
{
    if ((bchar >='0')&&(bchar <='9'))   return(bchar-'0');
    if ((bchar >='A')&&(bchar <='F'))   return(bchar-'A'+10);
    else                                return(0xf);
}
static void a_to_b(uchar *asc, uint asc_len, uchar *bcd)
{
    uint i,j;
    uchar bOddFlag, bchar; 
    bOddFlag =0;
    if (asc_len%2)      bOddFlag =1; 
    for (i=0,j=0; j<asc_len; i++)
    {
        if ((i==0) && bOddFlag)     
            bcd[i] = c_to_b(asc[j++]);
        else
        {
            bchar = c_to_b(asc[j++]);
            bcd[i] = bchar*16 + c_to_b(asc[j++]);
        }
    }
}

static void b_to_a(uchar *bcd, uchar *asc, uint asc_len)
{
    uint i, j, new_len;
    uchar bOddFlag, bchar; 
    new_len =asc_len/2;
    bOddFlag =0;
    if (asc_len%2)      bOddFlag = 1; 
    for(i=0,j=0; i<new_len; i++)
    {
        bchar = bcd[i]/16;
        asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
        bchar = bcd[i]&0x0f;
        asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
    }
    if (bOddFlag)
    {
        bchar = bcd[i]/16;
        asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
    } 
    asc[j] = 0x0;
} 

void ShowInfo(char *psInfo)
{
    uchar bStrLen;
    bStrLen =strlen(psInfo);
    Lib_LcdClrLine(0, 63);
     if(bStrLen<17)
      Lib_LcdGotoxy((16-bStrLen)*4, 2*8);
    else 
      Lib_LcdGotoxy(0,3*8);
    Lib_Lcdprintf(psInfo);
    //printf(psInfo);

} 

uchar GetEDC(uchar *psData)
{
    uint i,leng;
    uchar bEdc; 
    leng=psData[1]*256+psData[2]+3;
    bEdc=0;
    for(i=0;i<leng;i++)     bEdc ^= psData[i];
    return bEdc;
} 

uchar ConvReturnCode(int ireturn)
{
    uchar rc=0;
    switch(ireturn)
    {
    case 0:
        rc=0;
        break; 
    case PCI_Locked_Err: 
        rc=PCI_LockedErr;
        break;
    case PCI_KeyType_Err: 
        rc=PCI_KeyTypeErr;
        break;
    case PCI_KeyLrc_Err: 
        rc=PCI_KeyLrcErr;
        break;
    case PCI_KeyNo_Err: 
        rc=PCI_KeyNoErr;
        break;
    case PCI_KeyLen_Err: 
        rc=PCI_KeyLenErr;
        break;
    case PCI_KeyMode_Err: 
        rc=PCI_KeyModeErr;
        break;
    case PCI_InputLen_Err: 
        rc=PCI_InputLenErr;
        break;
    case PCI_InputCancel_Err: 
        rc=PCI_InputCancelErr;
        break;
    case PCI_InputNotMatch_Err: 
        rc=PCI_InputNotMatchErr;
        break;
    case PCI_InputTimeOut_Err: 
        rc=PCI_InputTimeOutErr;
        break;
    case PCI_CallTimeInte_Err:
        rc=PCI_CallTimeInteErr;
        break;
    case PCI_NoKey_Err: 
        rc=PCI_NoKeyErr;
        break;
    case PCI_WriteKey_Err: 
        rc=PCI_WriteKeyErr;
        break;
    case PCI_ReadKey_Err: 
        rc=PCI_ReadKeyErr;
        break;
    case PCI_RsaKeyHash_Err: 
        rc=PCI_RsaKeyHashErr;
        break;
    case PCI_DataLen_Err:
        rc=PCI_DataLenErr;
        break;
    case PCI_NoInput_Err:
        rc=PCI_NoInputErr;
        break;
    case PCI_AppNumOver_Err: 
        rc=PCI_AppNumOverErr;
        break;
    case PCI_ReadMMK_Err:
        rc=PCI_ReadMMKErr;
        break;
    case PCI_WriteMMK_Err: 
        rc=PCI_WriteMMKErr;
        break;
    case PCI_Auth_Err: 
        rc=PCI_AuthErr;
        break;
    case PCI_RsaKey_Err: 
        rc=PCI_RsaKeyErr;
        break; 
    case PCI_KeySame_Err:
        rc=PCI_KeySameErr;
        break; 

	case DUKPT_NoKey:
        rc=DUKPT_NoKeyErr;
        break;
	case DUKPT_CounterOverFlow:
        rc=DUKPT_CounterOverFlowErr;
        break;
	case DUKPT_NoEmptyList:
        rc=DUKPT_NoEmptyListErr;
        break;
	case DUKPT_InvalidAppNo:
        rc=DUKPT_InvalidAppNoErr;
        break;
	case DUKPT_InvalidKeyID:
        rc=DUKPT_InvalidKeyIDErr;
        break;
	case DUKPT_InvalidFutureKeyID:
        rc=DUKPT_InvalidFutureKeyIDErr;
        break;
	case DUKPT_InvalidCrc:
        rc=DUKPT_InvalidCrcErr;
        break;
	case DUKPT_InvalidBDK:
        rc=DUKPT_InvalidBDKErr;
        break;
	case DUKPT_InvalidKSN:
        rc=DUKPT_InvalidKSNErr;
        break;
	case DUKPT_InvalidMode:
        rc=DUKPT_InvalidModeErr;
        break;
	case DUKPT_NotFound:
        rc=DUKPT_NotFoundErr;
        break; 
    }
    return rc; 
} 

int RecvHostCmd(uchar *psCmdStr, uchar *bRecvLen)
{
    //uchar  nRetVal;
    uint   rx_len,bOffset;
    uchar  bEdc;
    int iret; 
    bOffset =0; 
    iret =Lib_ComRecvByte(TEST_COM, &psCmdStr[bOffset], 500);
    //ShowInfo("等待接收数据...");
    //ShowInfo("Receiving...");
    if (iret)   return iret; 
    if (psCmdStr[bOffset]<0x90 || psCmdStr[bOffset]>0xb0)
        return -1; 
    
    bOffset ++;
    iret =Lib_ComRecvByte(TEST_COM, &psCmdStr[bOffset], 500);
    if (iret) return iret;
    rx_len=psCmdStr[bOffset]*256;
    bOffset ++;
    iret =Lib_ComRecvByte(TEST_COM, &psCmdStr[bOffset], 500);
    if (iret) return iret;
    rx_len += psCmdStr[bOffset];
    bOffset ++; 
    switch(psCmdStr[0])
    {
    case PCIREADVER_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIGETRND_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIACCESSAUTH_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIWRITEPINMKEY_CMD:
	case PCIWRITEMACMKEY_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIWRITEPINKEY_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIWRITEMACKEY_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    //case PCIWRITEDESKEY_CMD:
        //if (rx_len!=0)    return -1;           
    //    break;          
   // case PCIDERIVEPINKEY_CMD:
        //if (rx_len!=0)    return -1;           
  //      break;          
    case PCIDERIVEMACKEY_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    //case PCIDERIVEDESKEY_CMD:
        //if (rx_len!=0)    return -1;           
    //    break;          
    case PCIGETPIN_CMD:
        //if (rx_len!=0)    return -1;           
        break;
	//case PCIGETPINAUTO_CMD:
	//	break;
             
    case PCIGETMAC_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIDES_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIOFFLINEPLAINPIN_CMD:
        //if (rx_len!=0)    return -1;           
        break;          
    case PCIOFFLINEENCPIN_CMD:
        //if (rx_len!=0)    return -1;           
        break; 
    
    case DUKPTGETPIN_CMD:
        //if (rx_len!=0)    return -1;           
        break; 
             
    case DUKPTGETMAC_CMD:
        //if (rx_len!=0)    return -1;           
        break;  

	case PCIGETPINFIXK_CMD:
        //if (rx_len!=0)    return -1;           
        break; 
	case PCIWRITEDESKEY_CMD:
		break;
	case PCIGETOFFLINEPIN_CMD:
		break;
        
    default:
        return -1;
    }
    while(1)
    {
        iret =Lib_ComRecvByte(TEST_COM, &psCmdStr[bOffset], 500);
        if (iret)   break;
        if (bOffset ==(rx_len+3)) break;
        bOffset ++;
    }
    if (bOffset !=(rx_len+3))
        return -1;

    bEdc =GetEDC(psCmdStr);
    if (bEdc !=psCmdStr[rx_len+3])
        return -1;

    *bRecvLen =bOffset;
    return 0;
}

void SendErrorData(uchar bCmdHead, uchar bErrCode)
{
    uchar sSendBuf[10];//i, 
    uint len;// j,
    
    if (bErrCode ==0)
    {
        sSendBuf[0] =bCmdHead;
        sSendBuf[1] =0;
        sSendBuf[2] =0;
        sSendBuf[3] =GetEDC(sSendBuf);
    }
    else
    {
        sSendBuf[0] =bCmdHead&0x7f;
        sSendBuf[1] =0;
        sSendBuf[2] =1;
        sSendBuf[3] =bErrCode;
        sSendBuf[4] =GetEDC(sSendBuf);
    }
    len=sSendBuf[1]*256+sSendBuf[2]+4;
    Lib_ComSend(TEST_COM, sSendBuf,len);
    //for(j=0; j<len; j++)
    //    Lib_ComSendByte(TEST_COM, sSendBuf[j]);
}

void SendBackData(uchar bCmdHead, uint slen, uchar *sdata)
{
    uchar sSendBuf[4096];
    uint i, xlen;//,j,len;
    slen=slen%4096; 
    xlen=0;
    sSendBuf[xlen++] =bCmdHead;
    sSendBuf[xlen++] =slen/256;
    sSendBuf[xlen++] =slen%256;
    for(i=0;i<slen;i++)
        sSendBuf[xlen++]=sdata[i];
    sSendBuf[xlen] =GetEDC(sSendBuf); 
    Lib_ComSend(TEST_COM, sSendBuf,slen+4);
    //for(i=0; i<(slen+4); i++)
    //    Lib_ComSendByte(TEST_COM, sSendBuf[i]);
} 

uchar Pci_ReadVerTest(uchar *psCmdStr)
{ 
    uchar rc=0;
    uchar Ver[8]; 
    memset(Ver, 0, sizeof(Ver));
    Lib_ReadVerInfo(Ver);
    rc=0;
    if(rc==0)   SendBackData(psCmdStr[0],8, Ver);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
}
  
uchar Pci_GetOfflinePinTest(uchar *psCmdStr)
{
    int iret;
    uchar rc; 
    uchar  min_len,max_len;//,key_n,mode;
    uchar pinblock[8];//cardno[16],
    ushort wait_sec;  
	uchar tryflag,remaincnt;

	tryflag = psCmdStr[3];
	remaincnt = psCmdStr[4];
    min_len=psCmdStr[5];     
    max_len=psCmdStr[6];
    wait_sec=psCmdStr[7]*256+psCmdStr[8];


     if (tryflag==0 && remaincnt != 1)
     {
         Lib_Beep(); 
		 Lib_LcdCls(); 
		 Lib_LcdPrintxyCE(2,0*8,0x01,"请输入脱机密码","PLS INPUT PIN:");
		 Lib_LcdPrintxyCE(2,4*8,0x01,"无密码按确认键 ","No Pwd,Press Enter");
      }
     else//if (tryflag!=0)
     {
        if(remaincnt == 1)
        {
			Lib_Beep(); 
			Lib_LcdCls(); 
			Lib_LcdPrintxyCE(2,0*8,0x01,"最后一次输入","Last Time Input:");
        }
        else
        {
			Lib_Beep(); 
			Lib_LcdCls(); 
			Lib_LcdPrintxyCE(2,0*8,0x01,"密码错,请重试","Pwd Err,Retry:");
        }
     }

    Lib_LcdGotoxy(2,4*8);
    iret=Lib_PciGetOfflinePin(min_len,max_len,pinblock,wait_sec);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],8, pinblock);
    else        SendErrorData(psCmdStr[0], rc);
    return rc; 

}

uchar Pci_OffLineEncPinTest(uchar *psCmdStr)
{
    uchar rc,Ret_data[300];//,atr[40];
    uchar i,j,k;//,l;
    ushort wait_sec;    
    int iret; 
    RSA_PINKEY RSA_pinkey; 
    memset(Ret_data,0,300);
    RSA_pinkey.modlen=psCmdStr[3];
    i=psCmdStr[3];
    memcpy(RSA_pinkey.mod,&psCmdStr[4],i); 
    RSA_pinkey.explen=psCmdStr[4+i];
    j=psCmdStr[4+i];
    memcpy(RSA_pinkey.exp,&psCmdStr[5+i],j); 
    RSA_pinkey.iccrandomlen=8;
    memcpy(RSA_pinkey.iccrandom,&psCmdStr[6+i+j],8); 
    RSA_pinkey.termrandomlen=psCmdStr[14+i+j];
    k=psCmdStr[14+i+j];
    memcpy(RSA_pinkey.termrandom,&psCmdStr[15+i+j],k); 
    wait_sec=psCmdStr[17+i+j+k]*256+psCmdStr[18+i+j+k];  
    /*
    //waiting for insert ic card
    iret=Lib_IccCheck(0);
    if(iret)
    {
        //Lib_LcdClrLine(2*8,79);
        //Lib_LcdPrintxy(0,4*8,1,"    Pls Insert Card  "); 
        ShowInfo("Pls Insert Card!");
    }
    while(1)
    {
        iret=Lib_IccCheck(0);
        if(!iret)
        {
            break;
        } 
    } 
    
    //reset card
    iret=Lib_IccOpen(0,1,atr);
    if(iret)
    {
        rc=ConvReturnCode(iret);
        //if(rc==0)   SendBackData(psCmdStr[0],2, Ret_data);
        //else        
        SendErrorData(psCmdStr[0], rc);
        return rc;
    } 
    
    */
    //display input pin:
    Lib_Beep(); 
    Lib_LcdClrLine(0,63);
    Lib_LcdPrintxy(2,0*8,0x01,"PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    //printf("Pls input pin:\r\n"); 
   
    
    iret=Lib_PciOffLineEncPin(&RSA_pinkey,psCmdStr[15+i+j+k],psCmdStr[16+i+j+k],Ret_data,wait_sec);
    
    //test
    Lib_LcdPrintxy(0,8*1,0x00,"Lib_PciOffLineEncPin[%d]",iret);
   Lib_KbGetCh();
    
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],i, Ret_data);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_OffLinePlainPinTest(uchar *psCmdStr)
{
    uchar rc,Ret_data[2],atr[40];
    ushort wait_sec;  
    int iret;
    memset(Ret_data,0,2);
    wait_sec=psCmdStr[6]*256+psCmdStr[7];
    
   
    //waiting for insert ic card
    iret=Lib_IccCheck(0);
    if(iret)
    {
        //Lib_LcdClrLine(2*8,79);
        //Lib_LcdPrintxy(0,4*8,1,"    Pls Insert Card  "); 
        ShowInfo("Pls Insert Card!");
    }
    while(1)
    {
        iret=Lib_IccCheck(0);
        if(!iret)
        {
            break;
        } 
        if(!Lib_KbCheck())
        {
            if(Lib_KbGetCh()==KEYCANCEL) 
            {
                rc=PCI_InputCancelErr;
                SendErrorData(psCmdStr[0], rc);
                return rc;
            } 
        }
    } 
    
    //reset card
    iret=Lib_IccOpen(0,1,atr);
    if(iret)
    {
        rc=ConvReturnCode(iret);
        if(rc==0)   SendBackData(psCmdStr[0],2, Ret_data);
        else        SendErrorData(psCmdStr[0], rc);
        return rc;
    } 
    //display input pin:
    Lib_Beep(); 
    Lib_LcdClrLine(0,63);
    Lib_LcdPrintxy(2,0*8,0x01,"PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    //printf("Pls input pin:\r\n"); 
    
    
    iret=Lib_PciOffLinePlainPin(psCmdStr[3],psCmdStr[4],psCmdStr[5],&psCmdStr[8],Ret_data,wait_sec);
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],2, Ret_data);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
} 
 
uchar Pci_GetPinTest(uchar *psCmdStr)
{
    
    int iret;
    uchar rc; 
    uchar  key_n,min_len,max_len,mode;
    uchar cardno[16],pinblock[8];
    ushort wait_sec;  
    key_n=psCmdStr[3];
    min_len=psCmdStr[4];     
    max_len=psCmdStr[5];
    memcpy(cardno,&psCmdStr[6],16);
    mode=psCmdStr[22]; 
    wait_sec=psCmdStr[23]*256+psCmdStr[24];
    Lib_Beep(); 
    Lib_LcdCls(); 
    Lib_LcdPrintxyCE(2,0*8,0x01,"请输入联机密码","PLS INPUT PIN:");
	Lib_LcdPrintxyCE(2,4*8,0x01,"无密码按确认键 ","No Pwd,Press Enter");
    Lib_LcdGotoxy(2,2*8);
    iret=Lib_PciGetPin(key_n,min_len,max_len,cardno,mode,pinblock,wait_sec);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],8, pinblock);
    else        SendErrorData(psCmdStr[0], rc);
    
#if 0    
    Lib_LcdPrintxy(0,8*1,0x00,"rc[%d] [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]",
                   rc,pinblock[0],pinblock[1],pinblock[2],pinblock[3],
                   pinblock[4],pinblock[5],pinblock[6],pinblock[7]);
   Lib_KbGetCh();
#endif
   
    return rc; 



/*
    int iret;
    uchar rc; 
    uchar  key_n,min_len,max_len,mode;
    uchar cardno[16],pinblock[8],money[14];
    ushort wait_sec;  
	memset(money,0,sizeof(money));
	money[14] = '\0';
    key_n=psCmdStr[3];
    min_len=psCmdStr[4];     
    max_len=psCmdStr[5];
    memcpy(cardno,&psCmdStr[6],16);
    mode=psCmdStr[22]; 
    wait_sec=psCmdStr[23]*256+psCmdStr[24];
    Lib_Beep(); 
    Lib_LcdCls(); 
	if (psCmdStr[25] == 1)
	{
		memcpy(money,&psCmdStr[27],psCmdStr[26]);
		Lib_LcdGotoxy(0,0);
		Lib_LcdDrawLogo(g_Display_RMB);
		Lib_LcdGotoxy(16,0);
		Lib_Lcdprintf(money);
	}
    Lib_LcdPrintxyCE(2,0*8,0x01,"请输入联机密码","PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    iret=Lib_PciGetPin(key_n,min_len,max_len,cardno,mode,pinblock,wait_sec);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],8, pinblock);
    else        SendErrorData(psCmdStr[0], rc);
    return rc; */

}

uchar Pci_GetPinAutoTest(uchar *psCmdStr)
{
    int iret;
    uchar rc; 
    uchar  key_n,min_len,max_len,mode;
    uchar cardno[16],pinblock[8];
    ushort wait_sec;  
    key_n=psCmdStr[3];
    min_len=psCmdStr[4];     
    max_len=psCmdStr[5];
    memcpy(cardno,&psCmdStr[6],16);
    mode=psCmdStr[22]; 
    wait_sec=psCmdStr[23]*256+psCmdStr[24];
    Lib_Beep(); 
    Lib_LcdCls(); 
    Lib_LcdPrintxy(2,0*8,0x01,"PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    //printf("Pls input pin:\r\n");
    iret=Lib_PciGetPinAuto(key_n,min_len,max_len,cardno,mode,pinblock,wait_sec);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],8, pinblock);
    else        SendErrorData(psCmdStr[0], rc);
    return rc; 
}

uchar Pci_GetPinFixKTest(uchar *psCmdStr)
{
    
    int iret;
    uchar rc; 
    uchar  key_n,min_len,max_len,mode;
    uchar cardno[16],pinblock[8];
    ushort wait_sec;  
    key_n=psCmdStr[3];
    min_len=psCmdStr[4];     
    max_len=psCmdStr[5];
    memcpy(cardno,&psCmdStr[6],16);
    mode=psCmdStr[22]; 
    wait_sec=psCmdStr[23]*256+psCmdStr[24];
    Lib_Beep(); 
    Lib_LcdCls(); 
    Lib_LcdPrintxy(2,0*8,0x01,"PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    //printf("Pls input pin:\r\n");
    iret=Lib_PciGetPinFixK(key_n,min_len,max_len,cardno,mode,pinblock,wait_sec);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    if(rc==0)   SendBackData(psCmdStr[0],8, pinblock);
    else        SendErrorData(psCmdStr[0], rc);
    return rc; 
}

uchar Pci_GetMacTest(uchar *psCmdStr)
{ 
    int iret; 
    uchar rc; 
    uchar  key_n,mode;
    uchar buf[1024],mac[8];
    ushort len;  
    key_n=psCmdStr[3];
    len=psCmdStr[4]*256+psCmdStr[5];  
    memset(buf,0,sizeof(buf)); 
    memcpy(buf,&psCmdStr[6],len);
    mode=psCmdStr[6+len];  
    iret=Lib_PciGetMac(key_n,len,buf,mac,mode);
    rc=ConvReturnCode(iret);  
    if(rc==0)   SendBackData(psCmdStr[0],8, mac);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
}



extern int  Lib_PciDes(BYTE deskey_n, BYTE *indata, BYTE *outdata, BYTE mode);
uchar Pci_DesTest(uchar *psCmdStr)
{ 
    int iret; 
    uchar rc; 
    uchar  key_n,mode;
    uchar buf[1024],mac[8]; 
    key_n=psCmdStr[3]; 
    memset(buf,0,sizeof(buf)); 
    memcpy(buf,&psCmdStr[4],8);
    mode=psCmdStr[4+8];  
    iret=Lib_PciDes(key_n,buf,mac,mode);
    rc=ConvReturnCode(iret);  
    if(rc==0)   SendBackData(psCmdStr[0],8, mac);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
}


uchar Pci_DukptGetPinTest(uchar *psCmdStr)
{
    
    int iret;
    uchar rc; 
    uchar  key_n,min_len,max_len,mode;
    uchar cardno[16],pinblock[8],ksn[10];
    uchar retbuf[100];
    ushort wait_sec;  
    key_n=psCmdStr[3];
    min_len=psCmdStr[4];     
    max_len=psCmdStr[5];
    memcpy(cardno,&psCmdStr[6],16);
    mode=psCmdStr[22]; 
    wait_sec=psCmdStr[23]*256+psCmdStr[24];
    Lib_Beep(); 
    Lib_LcdCls(); 
    Lib_LcdPrintxy(2,0*8,0x01,"PLS INPUT PIN:");
    Lib_LcdGotoxy(2,4*8);
    //printf("Pls input pin:\r\n");
    iret=Lib_PciGetPinDukpt(key_n,min_len,max_len,cardno,mode,pinblock,wait_sec,ksn);
    Lib_LcdCls();  
    rc=ConvReturnCode(iret);
    memset(retbuf,0,sizeof(retbuf));
    memcpy(retbuf,pinblock,8);
    memcpy(retbuf+8,ksn,10);
    if(rc==0)   SendBackData(psCmdStr[0],8+10, retbuf);
    else        SendErrorData(psCmdStr[0], rc);
    return rc; 
}

uchar Pci_DukptGetMacTest(uchar *psCmdStr)
{ 
    int iret; 
    uchar rc; 
    uchar  key_n,mode;
    uchar buf[1024],mac[8],ksn[10],retbuf[100];
    ushort len;  
    key_n=psCmdStr[3];
    len=psCmdStr[4]*256+psCmdStr[5];  
    memset(buf,0,sizeof(buf)); 
    memcpy(buf,&psCmdStr[6],len);
    mode=psCmdStr[6+len];  
    iret=Lib_PciGetMacDukpt(key_n,len,buf,mac,mode,ksn);
    rc=ConvReturnCode(iret);
    memset(retbuf,0,sizeof(retbuf));
    memcpy(retbuf,mac,8);
    memcpy(retbuf+8,ksn,10); 
    if(rc==0)   SendBackData(psCmdStr[0],8+10, retbuf);
    else        SendErrorData(psCmdStr[0], rc);
    return rc;
}
 
uchar Pci_DerivePinKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc;  
    uchar  mkey_n,pinkey_n1,pinkey_n2,mode;  
    mkey_n=psCmdStr[3];
    pinkey_n1=psCmdStr[4];     
    pinkey_n2=psCmdStr[5];
    mode=psCmdStr[6];  
    iret=Lib_PciDerivePinKey(mkey_n,pinkey_n1,pinkey_n2,mode); 
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_DeriveMacKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc;  
    uchar  mkey_n,mackey_n1,mackey_n2,mode; 
    mkey_n=psCmdStr[3];
    mackey_n1=psCmdStr[4];     
    mackey_n2=psCmdStr[5];
    mode=psCmdStr[6];  
    iret=Lib_PciDeriveMacKey(mkey_n,mackey_n1,mackey_n2,mode); 
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
} 

uchar Pci_WriteMacKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc; //32
    uchar  key_no,key_len,mode,mkey_no;
    uchar  tmpbuf[24],tmpcrc[4]; 
    key_no=psCmdStr[3];
    key_len=psCmdStr[4];     
    memcpy(tmpbuf,&psCmdStr[5],key_len);
    mkey_no=psCmdStr[5+key_len];     
    mode=psCmdStr[6+key_len];  

	memcpy(tmpcrc,&psCmdStr[7+key_len],4); 

    //iret=Lib_PciWriteMacKey(key_no,key_len,tmpbuf,tmpcrc,mkey_no);//mode,mkey_no);  
    iret=Lib_PciWriteMacKey(key_no,key_len,tmpbuf,tmpcrc,mode,mkey_no);  
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}


uchar Pci_WriteDesKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc; //32
    uchar  key_no,key_len,mode;//,mkey_no;
    uchar  tmpbuf[24];//,tmpcrc[4]; 
    key_no=psCmdStr[3];
    key_len=psCmdStr[4];     
    memcpy(tmpbuf,&psCmdStr[5],key_len);
    mode=psCmdStr[5+key_len];     
    //mode=psCmdStr[6+key_len];  

	//memcpy(tmpcrc,&psCmdStr[7+key_len],4); 

//	Lib_LcdCls();
//	Lib_Lcdprintf("writing");
//	Lib_DelayMs(2000);
    iret=Lib_PciWriteDesKey(key_no,key_len,tmpbuf,mode);//mode,mkey_no);  
//	Lib_LcdCls();
//	Lib_Lcdprintf("iret is %d",iret);
//	Lib_DelayMs(2000);
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_WritePinKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc;    //32
    uchar  key_no,key_len,mode,mkey_no;
    uchar  tmpbuf[24],tmpcrc[4]; 

    key_no=psCmdStr[3];
    key_len=psCmdStr[4];     
    memcpy(tmpbuf,&psCmdStr[5],key_len);
    mkey_no=psCmdStr[5+key_len];
    mode=psCmdStr[6+key_len]; 

	memcpy(tmpcrc,&psCmdStr[7+key_len],4);

    //iret=Lib_PciWritePinKey(key_no,key_len,tmpbuf,tmpcrc,mkey_no);//mode,mkey_no);  
    //iret=Lib_PciWritePinKey(key_no,key_len,tmpbuf,tmpcrc,mode,mkey_no);  
    iret=Lib_PciWriteWorkKey( key_no,key_len,tmpbuf, tmpcrc,mode, mkey_no); //FOR APP

    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_WritePIN_MKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc;  //31
    uchar  key_no,key_len,mode;
    uchar  tmpbuf[24]; 
    key_no=psCmdStr[3];
    key_len=psCmdStr[4];     
    mode=psCmdStr[5+key_len];
    
    memcpy(tmpbuf,&psCmdStr[5],key_len); 
    //int  Lib_PciWriteMKey(BYTE key_no,BYTE key_len,BYTE *key_data,BYTE mode)
    //iret=Lib_PciWritePIN_MKey(key_no,key_len,tmpbuf);//,mode);  
    iret=Lib_PciWritePIN_MKey(key_no,key_len,tmpbuf,mode);  
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_WriteMAC_MKeyTest(uchar *psCmdStr)
{
    int iret;
    uchar rc;  //31
    uchar  key_no,key_len,mode;
    uchar  tmpbuf[24]; 
    key_no=psCmdStr[3];
    key_len=psCmdStr[4];     
    mode=psCmdStr[5+key_len];
    memcpy(tmpbuf,&psCmdStr[5],key_len); 
    //int  Lib_PciWriteMKey(BYTE key_no,BYTE key_len,BYTE *key_data,BYTE mode)
    //iret=Lib_PciWriteMAC_MKey(key_no,key_len,tmpbuf);//,mode);  
    iret=Lib_PciWriteMAC_MKey(key_no,key_len,tmpbuf,mode);  
    rc=ConvReturnCode(iret);
    SendErrorData(psCmdStr[0], rc);
    return rc;
}

uchar Pci_GetRandTest(uchar *psCmdStr)
{
    //uchar rc;
    uchar Random[2049];
    int i;
    memset(Random, 0, sizeof(Random));
    for(i=0;i<256;i++)
    {  
        Lib_GetRand(&Random[i*8] ,8); 
    }
    
    //for(i=0;i<8;i++) printf("%02x ",Random[i]);
    SendBackData(psCmdStr[0],2048, Random);
    return 0;
}  




//int main(void)
int pcimain(void)
{
    int iret;
    uchar rc, bRecvLen;//i, 
    uchar CmdStr[2100],  sTemp[200];//temppwd[8],gplian disable RespStr[2100] 2100->1024
    //ulong templ;
    //test
    //uchar sTempData[2048],sTempData2[2048];

    
    //int j;
    
    iret = Lib_AppInit();
    
    
        
#if 0    
    //test
    
    memset(sTempData,0x55,2048);
    iret=sys_flash_write_operate(1023,sTempData,2048);
    trace_debug_printf("sys_flash_write_operate[%x]",iret);
    
    iret =sys_flash_read_operate(1023,sTempData2,2048);
    trace_debug_printf("sys_flash_read_operate[%x]",iret);
    
    if(memcmp(  sTempData,sTempData2,2048)==0)
      Lib_LcdPrintxy(0, 2*8, 0x00, "write&read same%d", iret);
    else
    {
      Lib_LcdPrintxy(0, 2*8, 0x00, "write&read not same");
      for(j=0;j<2048;j++)
        trace_debug_printf("%02x",sTempData2[j]);
      
    }
    
    for(;;);
    
    s_PciInitMMK();
#endif
    
    
    
    s_printf("appinit()=%d \r\n",iret); 
    //for(i=0;i<3;i++)
    //{
    //    Lib_Beep();
    //    Lib_DelayMs(100); 
    //}  
    Lib_LcdCls();
    //Lib_LcdPrintxy(0,0,0x81,"   PED认证测试  ");
    Lib_LcdPrintxy(0,0,0x82,"    PCI TEST   ");
    
    
    s_printf("   PED认证测试  \r\n");  
    rc =Lib_ComOpen(TEST_COM,"115200,8,n,1");
    if (rc)
    {
         
        ShowInfo("Open Port Fail!");
        Lib_KbGetCh();
		for(;;);
        //return 0;
    }  
    ShowInfo("Receiving...");
    while(1)
    {
        memset(CmdStr, 0, sizeof(CmdStr));
        iret =RecvHostCmd(CmdStr, &bRecvLen);
        if (iret)
        {   
            if(!Lib_KbCheck())
            {
                if(Lib_KbGetCh()==KEYCANCEL)
                    return 0;
            } 
            //ShowInfo("Receiving...");           
            continue;
        }
        
        switch(CmdStr[0])
        { 
        case PCIREADVER_CMD:
            rc =Pci_ReadVerTest(CmdStr);
            break; 
        case PCIGETRND_CMD:
            rc =Pci_GetRandTest(CmdStr);
            break;      
        case PCIWRITEPINMKEY_CMD:
            rc =Pci_WritePIN_MKeyTest(CmdStr);
            break;
        case PCIWRITEPINKEY_CMD:
            rc =Pci_WritePinKeyTest(CmdStr);
            break;
        case PCIWRITEMACKEY_CMD:
            rc =Pci_WriteMacKeyTest(CmdStr);
            break; 
        case PCIWRITEMACMKEY_CMD:
            rc =Pci_WriteMAC_MKeyTest(CmdStr);
            break;
        /*case PCIDERIVEPINKEY_CMD:
            rc =Pci_DerivePinKeyTest(CmdStr);
            break;*/
		case PCIWRITEDESKEY_CMD:
//			Lib_LcdCls();
//			Lib_Lcdprintf("Write Des Key");
//			Lib_DelayMs(2000);
			rc = Pci_WriteDesKeyTest(CmdStr);
			break;
        case PCIDERIVEMACKEY_CMD:
            rc =Pci_DeriveMacKeyTest(CmdStr);
            break;
        //case PCIDERIVEDESKEY_CMD:
        //    rc =Pci_DeriveDesKeyTest(CmdStr);
        //    break;
        case PCIGETMAC_CMD:
            rc =Pci_GetMacTest(CmdStr);
            break;      
        case PCIGETPIN_CMD:
            rc =Pci_GetPinTest(CmdStr);
            break; 
		/*case PCIGETPINAUTO_CMD:
			rc =Pci_GetPinAutoTest(CmdStr);
			break;*/
		case PCIGETOFFLINEPIN_CMD:
			rc = Pci_GetOfflinePinTest(CmdStr);
			break;
        case PCIOFFLINEENCPIN_CMD:
            rc =Pci_OffLineEncPinTest(CmdStr);
            break;
        case PCIOFFLINEPLAINPIN_CMD:
            rc =Pci_OffLinePlainPinTest(CmdStr);
            break; 

		case PCIDES_CMD:
			rc =Pci_DesTest(CmdStr);
			break;
            
        case DUKPTGETMAC_CMD:
            rc =Pci_DukptGetMacTest(CmdStr);
            break;      
        case DUKPTGETPIN_CMD:
            rc =Pci_DukptGetPinTest(CmdStr);
            break;
            
        case PCIGETPINFIXK_CMD:
            rc =Pci_GetPinFixKTest(CmdStr);
            break;      
        default:
            SendErrorData(CmdStr[0], 0xa0); 
            ShowInfo("Command Error");
            //Lcdprintf("%x %x %x %x ", CmdStr[0],CmdStr[1],CmdStr[2],CmdStr[3]);
            continue;
        }

        switch(rc)
        {
        case 0: 
            ShowInfo("Handle Success");
            break;
        case PCI_DataLenErr: 
            ShowInfo("Data Length Err");
            break;
        case PCI_KeyNoErr: 
            ShowInfo("Key Index Err");
            break;
        case PCI_KeyModeErr: 
            ShowInfo("Mode Err");
            break;
        case PCI_InputLenErr: 
            ShowInfo("Input StrLen Err");
            break;  
        case PCI_InputCancelErr: 
            ShowInfo("User CancelInput");
            break;
        case PCI_InputTimeOutErr: 
            ShowInfo("Input TimeOut");
            break; 
        case PCI_InputNotMatchErr: 
            ShowInfo("Different PIN");
            break;
        case PCI_NoInputErr: 
            ShowInfo("Not Input PIN");
            break;
        case PCI_NoKeyErr: 
            ShowInfo("Key Nonexistence");
            break;   
        case PCI_KeyLrcErr: 
            ShowInfo("Compare Key Fail");
            break;  
        case PCI_AppNumOverErr: 
            ShowInfo("Total Number Over");
            break; 
        case PCI_AuthErr: 
            ShowInfo("ExternalAuth Fail");
            break; 
        case PCI_KeyTypeErr: 
            ShowInfo("KeyType Error");
            break;  
        case PCI_CallTimeInteErr: 
            ShowInfo("IntervalTime Less than 30sec");
            break;  
        case PCI_LockedErr: 
            ShowInfo("PED Locked");
            break; 
		case PCI_KeySameErr: 
            ShowInfo("Key Same Error");
            break; 
        default: 
            sprintf((char *)sTemp, "Other Error[%02x]", rc);
            ShowInfo((char *)sTemp);
            break;
        }
        //getkey();
    }
} 

 
