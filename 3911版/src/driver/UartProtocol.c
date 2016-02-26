#include "TaxBox.h"
#include "comm.h"
#include "UartProtocol.h"
//#include "rc531\Mifre_TmDef.h"
#include "pn512\Mifre_TmDef.h"

#include "KF701DH.h"
#include <time.h>


#define COMM_PORT   UART1
#define TIME_OUT    2

unsigned char BeepFlag=0; //0->1
unsigned short BeepDelay=0;

#if 0 //12/09/25

#define COMM_BANKUNION

#ifdef COMM_BANKUNION

//#define RECEIVE_BUFF_LEN  1034//512 too small for APDU_SEND  gplian
#define RECEIVE_BUFF_LEN  534//512 too small for APDU_SEND
//帧结构：
/******************************************************************************************************
STX(1B) |Serinal No(1B)|CMD(1B)| DataLen(2B) | Instruction OR status(1B)  | Data(xB) | CRC(2B) | ETX(1B)
*******************************************************************************************************/
//数据帧头长度及其内容
#define COM_STX			0x02		//帧头握手字节
#define COM_ETX			0x03		//帧尾字节

#define SUB_NO      1 //serinal no
#define SUB_INS     2//Instruction

#define SUB_DATA_0  5//First_Data

//send 帧定位
#define SEND_STX        0//STX
#define SEND_NO         1//serinal no
#define SEND_CMD        2// command id
#define SEND_LEN_H      3//SEND LEN HIGH
#define SEND_LEN_L      4//SEND LEN LOW
#define SEND_STATUS     5//STATUS
#define SEND_DATA_0     6//First_Data


unsigned char LcdDatex=0,LcdDatey=0,LcdTimex=0,LcdTimey=0;
unsigned char ShowDateFlag=0,ShowTimeFlag=0;
unsigned int ShowTimeDly=0,ShowDtaeDly=0;



extern unsigned char DisplayType;
extern unsigned char LedSegCode[];

//串行通讯各种命令定义
#define KF322_INS_POLL		0x07

//串口通讯返回码
#define RC_SUCCESS      0x00

static uchar gbPosPiccMode=0;/* 
                           
						0- 初始状态
                				1- POLL
                                		2- 正常
		                                3- 调试优化模式
                                		4- 非接触支付交易
                                		5- 管理
                              */

static uchar gbExAuth;  /*

	0-未进行双向认证
	1-进行双向认证	
                      */
//key define,存在安全的存储环境 ?                    
uchar gsIMEKmdk[16],gsIAEKmdk[16];
uchar gsIMEK[5][16],gsIAEK[5][16];	

static unsigned short gnP_MSG_TIMEOUT; //default:500ms
static unsigned short gnP_SALE_TIMEOUT;//15000ms
static unsigned short gnP_POLL_MSG;    //30s
static unsigned short gnP_BUF_TIMEOUT; //5000ms

//从串口循环队列的数据中获取合法的数据包
s16 PortRecvPack(unsigned char *recvBuf,u16 *pLen)
{
    unsigned char LRC,ch;
    u16 datalen=0,i=0;
    u16 nCRC;
    
    while(1)
    {
        LRC = 0;
        //PortReset(COMM_PORT);//每接收一包均进行复位操作？？？
        
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收STX
        if(ch != COM_STX) continue;
        recvBuf[0] = ch;

        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收serial no
        recvBuf[1] = ch;

        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收cmd
        recvBuf[2] = ch;
        
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收len 高字节
        recvBuf[3] = ch;
        datalen = (u16)(ch<<8);

        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收len 低字节
        recvBuf[4] = ch;
        datalen += ch;
        //if(datalen<1 || datalen>RECEIVE_BUFF_LEN) continue;
        if( datalen>RECEIVE_BUFF_LEN) continue;
        
        for(i=1;i<=datalen;i++){			//接收INS and data
            if(Lib_ComRecvByte(COMM_PORT,&recvBuf[i+4], TIME_OUT)) break;
            //LRC ^= recvBuf[i];
        }
        if(i<=datalen) continue;//receive
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收CRC1
        nCRC = (ch << 8) & 0xFF00;
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收CRC2
        nCRC += ch;
        
        //if( LRC!=ch ) continue;
        if(nCRC!=crc_16(datalen+4,1,&recvBuf[1]) )
        { 
          *pLen = datalen+5;
          return -RC_INVALID_DATA;
          //continue;
        }
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收EXT
        if( ch!=COM_ETX ) continue;
        
        return datalen;
    }
    
    
    
}

// STX+No+Cmd+len+data
static void SendPack(unsigned char * pSendBuf)
{
    u16 nCRC;
    s16 nLen;    
    nLen = (pSendBuf[SEND_LEN_H] << 8) & 0xFF00;
    nLen += pSendBuf[SEND_LEN_L];
    nLen +=5;
    nCRC = crc_16(nLen-1,1,&pSendBuf[1]);
    memcpy(&pSendBuf[nLen],&nCRC,2);
    pSendBuf[ nLen+2 ] = COM_ETX;
        
        //mask recieve int
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    Lib_ComSend(COMM_PORT, pSendBuf, nLen+3);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  
}

//串口发送数据包

//大约4K的栈空间，注意栈大小的分配
u16 Protocol_Pro(unsigned char* UART_RecBuffer,s16 DataLen)//协议处理部分
{
	//uchar var_Buf1[512],var_Buf2[512],var_char1,var_char2,var_char3;
	unsigned char sendRet;
    u16     SendLen=0,i=0,j=0,k=0,nCRC,nRet,nOffset;
    //#define UART_MAX_SIZE (1024+20)
    #define UART_MAX_SIZE (512+20)
    unsigned char UART_SendBuffer[UART_MAX_SIZE];
    unsigned int var_int1=0,var_int2=0;//for cmdexchange
        
//    PICC_PARA picc_para;//FOR RF SETUP disable gplian

//    APDU_SEND ApduSend;
//    APDU_RESP ApduRecv;
	unsigned char bVal,sBuf1[512],sBuf2[200];
	static unsigned char sRandR[16],sRandB[16];
	struct tm sTime;
/*
// #pragma pack(1) 
	typedef struct aa
	{
		unsigned short a;
                unsigned char c;
                unsigned char d;
                
		int b;
                 
	} aa;
// #pragma pack()	
*/

        
        memset(UART_SendBuffer,0,UART_MAX_SIZE);//清空发送缓冲区
        UART_SendBuffer[SEND_LEN_H] = 0;
        UART_SendBuffer[SEND_LEN_L] = 1;
	UART_SendBuffer[SEND_CMD] = UART_RecBuffer[SUB_INS];
	switch(UART_RecBuffer[SUB_INS])//分析指令码
	{
          case CMD_POLL_POLL:
            UART_SendBuffer[SEND_CMD] = CMD_POLL_POLL;
            
            if(gbExAuth)
              UART_SendBuffer[SEND_STATUS] = RC_POLL_A;
            else
              UART_SendBuffer[SEND_STATUS] = RC_POLL_P;
            
            break;
            
          case  CMD_POLL_ECHO:
            UART_SendBuffer[SEND_STATUS] = RC_SUCCESS;
            memcpy(UART_SendBuffer,UART_RecBuffer,DataLen);
             
            break;
         
          case CMD_DEBUG_ENTER:
            if(UART_RecBuffer[SEND_STATUS]==0) //enter debug
              gbPosPiccMode = POSPICC_DEBUG_M;
            else if(UART_RecBuffer[SEND_STATUS]==1) //enter normal
              gbPosPiccMode = POSPICC_NORMAL_M;
            UART_SendBuffer[SEND_STATUS] = RC_SUCCESS;
            break;
          case CMD_DEBUG_SETPARAMETER: 
            if(gbPosPiccMode != POSPICC_DEBUG_M)
            {
              UART_SendBuffer[SEND_STATUS] = RC_FAILURE;
              break;
            }
            nRet=PosPiccSetPara(DataLen,&UART_RecBuffer[SEND_STATUS]);
            if(nRet==RC_NO_PARAMETER)
              UART_SendBuffer[SEND_STATUS] = RC_NO_PARAMETER;
            else if(nRet==RC_SUCCESS)
            {
              UART_SendBuffer[SEND_STATUS] = RC_SUCCESS;
            }
            else
              UART_SendBuffer[SEND_STATUS] = RC_FAILURE;
            break;
          case CMD_AUTH_COMM_INIT:
		  	bVal = UART_RecBuffer[SUB_DATA_0];//key type
		  	EMVIF_GetRandNumber(sBuf1);
			memcpy(sRandR,sBuf1,16);
			memcpy(sRandB,&UART_RecBuffer[SUB_DATA_0+2],16);
			TripleDes_Enc2(bVal,UART_RecBuffer[SUB_DATA_0+1],sRandB,sBuf1,sBuf2);
			
			if(TripleDes_Enc1(bVal,UART_RecBuffer[SUB_DATA_0+1],&UART_RecBuffer[SUB_DATA_0+2],sBuf1,sBuf2)==0)
			{
				UART_RecBuffer[SEND_STATUS] = 0;
				nOffset= SEND_DATA_0;
				memcpy(&UART_SendBuffer[nOffset],&UART_RecBuffer[SUB_DATA_0],2);
				nOffset+=2;
				memcpy(&UART_SendBuffer[nOffset],sBuf1,8);
				nOffset+=8;
				memcpy(&UART_SendBuffer[nOffset],sBuf2,16);
				nOffset+=16;
		        UART_SendBuffer[SEND_LEN_L] = nOffset-SEND_STATUS;

			}
			else
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;

			}
            break;
          case  CMD_AUTH_BOTH:
		  	bVal = UART_RecBuffer[SUB_DATA_0];//key type
		  	if(TripleDes_Dec2(bVal,UART_RecBuffer[SUB_DATA_0+1],sBuf1,sBuf2,&UART_RecBuffer[SUB_DATA_0+2] ))
		  	{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;
		  	}
			if(memcmp(sBuf1,sRandR,16) || memcmp(sBuf2,sRandB,16) )
				UART_SendBuffer[SEND_STATUS] = RC_AUTH_FAILURE;
		  	break;
		  case CMD_AUTH_KEY_GENERATE:
		  	bVal = UART_RecBuffer[SUB_DATA_0];//key type
		  	if( TripleDes_Dec1(bVal,UART_RecBuffer[SUB_DATA_0+1],sBuf1,&UART_RecBuffer[SUB_DATA_0+2]) )
				UART_SendBuffer[SEND_STATUS] = RC_AUTH_NOT_PERFORMED;
		case CMD_AUTH_DISABLE_READER:				
		  	break;
		case CMD_TRAN_READYFORPAY:
			bVal = 0;
			if(ContactlessCardProc(&UART_RecBuffer[SUB_DATA_0],&bVal)==0)
			{
				if(bVal==RC_ONLINE)
					UART_SendBuffer[SEND_STATUS] = RC_ONLINE;
				else
					UART_SendBuffer[SEND_STATUS] = RC_DATA;
				MakeTranReadyForPayICData(sBuf1,&i);
				memcpy(&UART_SendBuffer[SEND_DATA_0],sBuf1,i);
				UART_SendBuffer[SEND_LEN_L] = (i+1)%0x100;
				UART_SendBuffer[SEND_LEN_H] = (i+1)/0x100;
				
				picc_close();
				break;
			}
			picc_close();
			
			UART_SendBuffer[SEND_STATUS] = bVal;
			break;
		case CMD_TRAN_RESET:
			break;
		case CMD_TRAN_DISP_STATUS:
			break;
		case CMD_TRAN_ONLINE:
			
			break;
		case CMD_TRAN_UPCARD:
			break;
		case CMD_MANAGER_ENTER:
			memcpy(&UART_SendBuffer[SEND_DATA_0],"VANSTONE",8);//制造商标识
			memcpy(&UART_SendBuffer[SEND_DATA_0+8],"V001",4);//固件版本号
			memcpy(&UART_SendBuffer[SEND_DATA_0+12],"    ",4);//RFU
			UART_SendBuffer[SEND_LEN_L] = 16+1;
			break;
		case CMD_MANAGER_GET_CAPA:
			break;
		case CMD_MANAGER_SET_CAPA:
			break;
		case CMD_MANAGER_GET_TIMEDATE:
			gettime(&sTime);
			sprintf(sBuf1,"20%02d%02d%02d%02d%02d%02d",sTime.tm_year,sTime.tm_mon,sTime.tm_mday,
		     sTime.tm_hour,sTime.tm_min,sTime.tm_sec);
			dat_asctobcd(sBuf2,sBuf1,14);
			memcpy(&UART_SendBuffer[SEND_DATA_0],sBuf2,7);
			UART_SendBuffer[SEND_LEN_L] = 8;	
			break;
		case CMD_MANAGER_SET_TIMEDATE:
			if(DataLen!=7)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_COMMAND;
				break;
			}
			
			memcpy(sBuf1,&UART_RecBuffer[SUB_DATA_0],7);
			ChangeDate(&sBuf1[0]); //YYYYMMDD
			ChangeTime(&sBuf1[4]);//HHMMSS
			break;
		case CMD_MANAGER_GET_PARAMETER:
			if(DataLen!=2)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;

			}
			//disable high byte
			sendRet = UART_RecBuffer[SUB_DATA_0+1];
			if(PosPiccGetPara(sendRet,&bVal,sBuf1)!=0)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_PARAM;
				break;

			}
			memcpy(&UART_SendBuffer[SEND_DATA_0],sBuf1,bVal);
			UART_SendBuffer[SEND_LEN_L] = bVal+1;
			break;
		case CMD_MANAGER_GET_COMM_BPS:
			UART_SendBuffer[SEND_DATA_0] = 0; //115200bps
			UART_SendBuffer[SEND_LEN_L] = 2;
			break;
		case CMD_MANAGER_SET_COMM_BPS:
			if(DataLen!=1)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;

			}
			//??setup new bps
			break;
		case CMD_MANAGER_RESET_ACQUIRE_KEY:
			//?? clear MEK AEK
			if(DataLen!=2)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;

			}
			break;
		case CMD_MANAGER_RESTORE_READER:
			//开启读卡器的寻卡功能??
			break;
		case CMD_MANAGER_GET_TAG:
			if(DataLen>2 || (DataLen==0) )
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;

			}
			if(DataLen==1)
				i= UART_RecBuffer[SUB_DATA_0];
			else
				i= UART_RecBuffer[SUB_DATA_0]<<8|UART_RecBuffer[SUB_DATA_0+1];
			if(Contactless_GetTLVData(i,&UART_SendBuffer[SEND_DATA_0],&var_int1)!=0)//not find.  X (1) -无标签
				var_int1 = 1;
			UART_SendBuffer[SEND_LEN_L] = (unsigned char)var_int1+1;
			break;
		case CMD_MANAGER_SET_TAG:
			Contactless_BatchSetTLVData(UART_RecBuffer[SUB_DATA_0],&UART_RecBuffer[SUB_DATA_0+1]);
			break;
		case CMD_MANAGER_GET_DISP:
			//get disp from file??
			UART_SendBuffer[SEND_LEN_L] = 2; //now set 无提示信息
			break;
		case CMD_MANAGER_SET_DISP:
			//?? set disp into file
			break;
		case CMD_MANAGER_GET_CVM:
			Contactless_GetTLVData(0x9f66,sBuf1,&var_int1);
			bVal =0;
			nOffset=SEND_DATA_0+1;
			if(sBuf1[0] &0x04) //support online pin
			{
				memcpy(&UART_SendBuffer[nOffset],"\x11\x00",2);
				nOffset+=2;
				bVal++;
			}
			if(sBuf1[0] &0x02) //support sign
			{
				memcpy(&UART_SendBuffer[nOffset],"\x10\x00",2);
				nOffset+=2;
				bVal++;
			}
			UART_SendBuffer[SEND_DATA_0] = bVal;
			UART_SendBuffer[SEND_LEN_L] = bVal*2+2;
			break;
		case CMD_MANAGER_SET_CVM:
			break;
		case CMD_MANAGER_SET_CAPK:
			if(ManagerSetCapk(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		case CMD_MANAGER_FIND_CAPK:
			if(DataLen!=6)
			{
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				break;

			}
			
			ManagerGetCAPK(&UART_RecBuffer[0],UART_RecBuffer[5],&i,&UART_RecBuffer[SUB_DATA_0]); //RID+index
			UART_SendBuffer[SEND_LEN_H] = i>>8;
			UART_SendBuffer[SEND_LEN_L] = i&0xff;
			break;
		case CMD_MANAGER_SET_REVOCLIST:

			if(ManagetSetRevocList(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
				
			

			break;
		case CMD_MANAGER_FIND_REVOCLIST:
			memcpy(sBuf1,&UART_RecBuffer[SUB_DATA_0],9);

			if(QPboc_FindRevocList(sBuf1[5],sBuf1,&sBuf1[6]))
				UART_SendBuffer[SEND_STATUS] = RC_FAILURE;
			break;
		case CMD_MANAGER_SET_EXCEPTIONLIST:
			if(ManagerSetExceptionList(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		case CMD_MANAGER_FIND_EXCEPTIONLIST:
			if(ManagerFindExceptionList(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		case CMD_MANAGER_SET_PBOC_PARAMETER:
			if(ManagerSetPbocParameter(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		case CMD_MANAGER_SET_AID:
			if(ManagerSetAid(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		case CMD_MANAGER_SET_ALL_AID:
			if(ManagerSetAllAid(DataLen,&UART_RecBuffer[SUB_DATA_0]))
				UART_SendBuffer[SEND_STATUS] = RC_INVALID_DATA;
			break;
		default:  //11/06/07
			UART_SendBuffer[SEND_STATUS] = RC_INVALID_COMMAND;
	
			break;
			
			
        }
 
//组装发送包结构
	UART_SendBuffer[SEND_STX]=COM_STX;//STX
        if(UART_RecBuffer[SUB_INS]!=CMD_POLL_POLL)
          UART_SendBuffer[SEND_NO] = UART_RecBuffer[ SUB_NO]+1;
        
        //UART_SendBuffer[SEND_LEN_H] = (SendLen>>8)&0x0ff;
        //UART_SendBuffer[SEND_LEN_L] = SendLen&0x0ff;
        
	//UART_SendBuffer[SEND_STATUS];
	//UART_SendBuffer[SEND_STATUS+SendLen]=GetXorCheck(&UART_SendBuffer[SEND_STATUS], SendLen);//LRC
        
        SendPack(UART_SendBuffer);
        /*
        nCRC = crc_16(SendLen+SEND_LEN_L,1,&UART_SendBuffer[1]);
        memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],&nCRC,2);
        
	UART_SendBuffer[SEND_STATUS+SendLen+2]=COM_ETX;//ETX
        
        //mask recieve int
        USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        PortSends(COMM_PORT, UART_SendBuffer, SendLen+8);
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
        */
	//TXD_EN = 0;	//使能中兴2.4G模块
	return (SendLen+8);        
        
        
}


u16 Uart_Protocol(void)
{
    unsigned char UART_RecBuffer[RECEIVE_BUFF_LEN];
    u16 i=0,nLen;
    s16 j=0;
    while(1)
    {
    
      
      j=PortRecvPack(UART_RecBuffer,&nLen);
      
      /*
      //test
      PortReset(COMM_PORT);//此时可以做复位操作
      delay_ms(1);//等待正确包后面的脏数据全部发完以免影响发送包
      UART_RecBuffer[0] =0x02;
      UART_RecBuffer[1] =0x00;
      UART_RecBuffer[2] =0x07;
      UART_RecBuffer[3] =0x00;
      UART_RecBuffer[4] =0x01;
      UART_RecBuffer[5] =0x00;
      SendPack( UART_RecBuffer);
      continue;
      */
      /*
      if(j==-RC_INVALID_DATA)
      { 
        if(UART_RecBuffer[SEND_CMD] != CMD_POLL_POLL)
        {
          UART_RecBuffer[SEND_NO] += 1;
          UART_RecBuffer[SEND_STATUS] = RC_INVALID_DATA;
          
        }
        
        SendPack(UART_RecBuffer);
        
        delay_ms(1);//等待正确包后面的脏数据全部发完以免影响发送包
        continue;
        
      }
      if( j<0 ) continue;
      */
      Lib_ComReset(COMM_PORT);//此时可以做复位操作
      delay_ms(5);//等待正确包后面的脏数据全部发完以免影响发送包
      i=Protocol_Pro(UART_RecBuffer,j);//执行协议处理							 
    }
}



void PosPiccInit()
{
	uchar sBuf[100];
	gbPosPiccMode = 0;
	gbExAuth = 0;
	if(GetPara(P_MSG_TIMEOUT,sBuf)==0)
		gnP_MSG_TIMEOUT = sBuf[0]*0x100+sBuf[1];
	else
		gnP_MSG_TIMEOUT = 500;//ms

	if(GetPara(P_SALE_TIMEOUT,sBuf)==0)
		gnP_SALE_TIMEOUT = sBuf[0]*0x100+sBuf[1];
	else
		gnP_SALE_TIMEOUT = 15000;//ms

	if(GetPara(P_POLL_MSG,sBuf)==0)
		gnP_POLL_MSG = sBuf[0]*0x100+sBuf[1];
	else
		gnP_POLL_MSG = 30;//s

	if(GetPara(P_BUF_TIMEOUT,sBuf)==0)
		gnP_BUF_TIMEOUT = sBuf[0]*0x100+sBuf[1];
	else
		gnP_BUF_TIMEOUT = 5000;//ms


		
}
#else

//串行通讯各种命令定义
#define KF322_INS_SETDATE		0x02
#define KF322_INS_SETTIME		0x03
#define KF322_INS_LCDASCII		0x04
#define KF322_INS_LCDESHOW		0x05
#define KF322_INS_SHOWDATE		0x07
#define KF322_INS_SHOWTIME		0x08
#define KF322_INS_BEEP		        0x09
#define KF322_INS_OFFSHOW		0x0A
#define KF322_INS_LIGHT		        0x0B
#define KF322_INS_LEDSHOW		0x0C

//RF相关接口
#define KF322_INS_OPEN		        0x10
#define KF322_INS_SETUP		        0x11
#define KF322_INS_DETECT		0x12
#define KF322_INS_ISOCOMMAND		0x13
#define KF322_INS_REMOVE		0x14
#define KF322_INS_CLOSE		        0x15
#define KF322_INS_CMDEXCHANGE		0x16


//串口通讯返回码
#define DATA_LEN_ERR			0xF8
#define DATA_INS_ERR			0xF9

//串口错误码定义
#define UART_RET_OK		        0x00
#define UART_RET_ERR		        0x01


//帧结构：
/******************************************************************************************************
STX(1B) | DataLen(2B) | Instruction OR status(1B)  | Data(xB) | LRC(1B) | ETX(1B)
*******************************************************************************************************/
//数据帧头长度及其内容
#define COM_STX			0x04		//帧头握手字节
#define COM_ETX			0x05		//帧尾字节

//recv帧定位
#define SUB_STX     0//STX

#define SUB_INS     1//Instruction

#define SUB_DATA_0  2//Frist_Data

//send 帧定位
#define SEND_STX        0//STX
#define SEND_LEN_H      1//SEND LEN HIGH
#define SEND_LEN_L      2//SEND LEN LOW
#define SEND_STATUS     3//STATUS
#define SEND_DATA_0     4//Frist_Data


//#define COMM_PORT   COM3
//#define TIME_OUT    2

#define LEDTYPE                0x01
#define LCDTYPE                0x00

#define RECEIVE_BUFF_LEN  600//512 too small for APDU_SEND
unsigned char UART_RecBuffer[RECEIVE_BUFF_LEN];
unsigned char DisplayArr[40];//store lcd/led data and time value
//for lcd 0-10 for date 11-21 for time;for led 0-7 for date 8-15 for time
unsigned char LcdDatex=0,LcdDatey=0,LcdTimex=0,LcdTimey=0;
unsigned char ShowDateFlag=0,ShowTimeFlag=0;
unsigned int ShowTimeDly=0,ShowDtaeDly=0;



extern unsigned char DisplayType;
extern unsigned char LedSegCode[];

//计算一串数据的异或校验值
unsigned char GetXorCheck(unsigned char *Buffer, unsigned int Len)
{
    unsigned char value=0;
    while(Len--)
    {
            value ^= *Buffer;
            Buffer++;
    }
    return value;
} 



//从串口循环队列的数据中获取合法的数据包
s16 PortRecvPack(unsigned char *recvBuf)
{
    unsigned char LRC,ch;
    u16 datalen=0,i=0;

    while(1)
    {
        LRC = 0;
        //PortReset(COMM_PORT);//每接收一包均进行复位操作？？？
        
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收STX
        if(ch != COM_STX) continue;
        recvBuf[0] = ch;

        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收len 高字节
        datalen = (u16)(ch<<8);

        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收len 低字节
        datalen += ch;
        if(datalen<1 || datalen>RECEIVE_BUFF_LEN) continue;
        
        for(i=1;i<=datalen;i++){			//接收INS and data
            if(Lib_ComRecvByte(COMM_PORT,&recvBuf[i], TIME_OUT)) break;
            LRC ^= recvBuf[i];
        }
        if(i<=datalen) continue;//receive
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收LRC
        if( LRC!=ch ) continue;
        
        if(Lib_ComRecvByte(COMM_PORT,&ch, TIME_OUT)) continue;//接收EXT
        if( ch!=COM_ETX ) continue;
        
        return datalen;
    }
}

//串口发送数据包

//大约4K的栈空间，注意栈大小的分配
u16 Protocol_Pro(unsigned char* UART_RecBuffer,s16 DataLen)//协议处理部分
{
	uchar var_Buf1[512],var_Buf2[512],var_char1,var_char2,var_char3;
	unsigned char sendRet;
        u16     SendLen=0,i=0,j=0,k=0;
        #define UART_MAX_SIZE (1024+20)
        unsigned char UART_SendBuffer[UART_MAX_SIZE];
        unsigned int var_int1=0,var_int2=0;//for cmdexchange
        
        PICC_PARA picc_para;//FOR RF SETUP
        APDU_SEND ApduSend;
        APDU_RESP ApduRecv;
	memset(UART_SendBuffer,0,UART_MAX_SIZE);//清空发送缓冲区

	switch(UART_RecBuffer[SUB_INS])//分析指令码
	{
	case KF322_INS_SETDATE:	//set date
		if ( DataLen !=6 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  ChangeDate(&UART_RecBuffer[SUB_DATA_0]);
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
		}
                SendLen=1;
		break;
	case KF322_INS_SETTIME:	//set time
		if ( DataLen !=4 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  ChangeTime(&UART_RecBuffer[SUB_DATA_0]);
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
		}
                SendLen=1;
		break;
	case KF322_INS_LCDASCII://LCD SHOW ASCII
		if ( DataLen <5 || DisplayType!=LCDTYPE )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  ShowDateFlag=0;
                  ShowTimeFlag=0;
                  var_char1 = UART_RecBuffer[SUB_DATA_0];
                  var_char2 = UART_RecBuffer[SUB_DATA_0+1];
                  var_char3 = UART_RecBuffer[SUB_DATA_0+2];
                  UART_RecBuffer[SUB_DATA_0+3+var_char3]=0x00;//复用接收缓冲
                  Lcdstring_On(var_char1,var_char2,&UART_RecBuffer[SUB_DATA_0+3]);                     
		}
                SendLen=1;
		break;

	case KF322_INS_LCDESHOW://LCD E show
		if ( DataLen <5 || DisplayType!=LCDTYPE )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  ShowDateFlag=0;
                  ShowTimeFlag=0;
                  var_char1 = UART_RecBuffer[SUB_DATA_0];
                  var_char2 = UART_RecBuffer[SUB_DATA_0+1];
                  var_char3 = UART_RecBuffer[SUB_DATA_0+2];
                  Lcdarray_On(var_char1,var_char2,var_char3, &UART_RecBuffer[SUB_DATA_0+3]);                     
		}
                SendLen=1;
		break;
        case KF322_INS_SHOWDATE://LCD/LED show DATE
		if ( DataLen <3 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  ShowDtaeDly=100*60*60;
                  
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  ShowDateFlag=1;
                  LcdDatex = UART_RecBuffer[SUB_DATA_0];
                  LcdDatey = UART_RecBuffer[SUB_DATA_0+1];
                  DateGet(UART_RecBuffer);//复用UART_RecBuffer
                  
                  if(DisplayType==LCDTYPE)//LCD
                  {
                    DisplayArr[0] =   UART_RecBuffer[0] + 0x30;                   
                    DisplayArr[1] =   UART_RecBuffer[1] + 0x30;                   
                    DisplayArr[2] =  UART_RecBuffer[2] + 0x30;                   
                    DisplayArr[3] =  UART_RecBuffer[3] + 0x30;                   
                    DisplayArr[4] =  '-';                   
                    DisplayArr[5] =  UART_RecBuffer[4] + 0x30;                   
                    DisplayArr[6] =  UART_RecBuffer[5] + 0x30;                   
                    DisplayArr[7] =  '-';                    
                    DisplayArr[8] =  UART_RecBuffer[6] + 0x30;                   
                    DisplayArr[9] =  UART_RecBuffer[7] + 0x30;                     
                    DisplayArr[10] =  0x00;
                    Lcdstring_On(LcdDatex,LcdDatey,&DisplayArr[0]);
                  }
                  
                  else//led
                  {
                    for(i=0;i<8;i++) DisplayArr[i] = LedSegCode[UART_RecBuffer[i]];                   
                    DisplayArr[i+3] &= 0x7F;
                    DisplayArr[i+5] &= 0x7F;
                    Ledarray_On( &DisplayArr[0] );
                  }
		}
                SendLen=1;
		break;
        case KF322_INS_SHOWTIME://LCD/LED show TIME
		if ( DataLen <3 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  ShowTimeDly=99;
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  ShowTimeFlag=1;
                  LcdTimex = UART_RecBuffer[SUB_DATA_0];
                  LcdTimey = UART_RecBuffer[SUB_DATA_0+1];
                  TimeGet(UART_RecBuffer);//复用UART_RecBuffer
                  
                  if(DisplayType==LCDTYPE)//LCD
                  {
                    DisplayArr[11] =   UART_RecBuffer[2] + 0x30;                   
                    DisplayArr[12] =   UART_RecBuffer[3] + 0x30;                   
                    DisplayArr[13] =   ':';                   
                    DisplayArr[14] =  UART_RecBuffer[4] + 0x30;  
                    DisplayArr[15] =  UART_RecBuffer[5] + 0x30;
                    DisplayArr[16] =  ':';                   
                                       
                    DisplayArr[17] =  UART_RecBuffer[6] + 0x30;
                    DisplayArr[18] =  UART_RecBuffer[7] + 0x30;                 
                    DisplayArr[19] =  0x00;
                    Lcdstring_On(LcdTimex,LcdTimey,&DisplayArr[11]);
                  }
                  
                  else//led
                  {
                    for(i=0;i<8;i++) DisplayArr[i] = LedSegCode[UART_RecBuffer[i]]; 
                    DisplayArr[0] = 0xFF;
                    DisplayArr[1] = 0xFF;
                    Ledarray_On( &DisplayArr[0] );
                  }
		}
                SendLen=1;
		break;
        case KF322_INS_BEEP://BEEP
		if ( DataLen <5 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  BeepFlag=1;
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  i  = UART_RecBuffer[SUB_DATA_0]*0x100 + UART_RecBuffer[SUB_DATA_0+1];
                  BeepDelay = UART_RecBuffer[SUB_DATA_0+2]*0x100 + UART_RecBuffer[SUB_DATA_0+3];
                  Buzzer_Ring(i);                  
                  //delay_ms(j);                   
                  //Buzzer_Off();  
		}
                SendLen=1;
		break;
        case KF322_INS_OFFSHOW://Shut down show
		if ( DataLen <1 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  ShowDateFlag=0;
                  ShowTimeFlag=0;
                  if(DisplayType==LCDTYPE)//LCD
                  {
                    Lcdarray_clear(0x0F);
                  }
                  else
                  {
                    Ledarray_Off();
                  }    
		}
                SendLen=1;
		break;
        case KF322_INS_LIGHT://Light Ctl
		if ( DataLen <2 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  if(UART_RecBuffer[SUB_DATA_0]&0x01)
                    ledred_on(); 
                  else
                    ledred_off(); 
                  if(UART_RecBuffer[SUB_DATA_0]&0x02)
                    ledgreen_on(); 
                  else
                    ledgreen_off();                   
                  if(UART_RecBuffer[SUB_DATA_0]&0x04)
                    ledorange_on(); 
                  else
                    ledorange_off();    
                  if(UART_RecBuffer[SUB_DATA_0]&0x80)
                    LcdBL_Control(1);   
                  else
                    LcdBL_Control(0); 
		}
                SendLen=1;
		break;
                
        case KF322_INS_LEDSHOW://LED show
		if ( DataLen <9 || DisplayType==LCDTYPE )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
		else
		{
                  UART_SendBuffer[SEND_STATUS] = UART_RET_OK;
                  ShowDateFlag=0;
                  ShowTimeFlag=0;  
                  memset(DisplayArr,0,sizeof(DisplayArr));
                  memcpy(DisplayArr,&UART_RecBuffer[SUB_DATA_0],8);
                  Ledarray_On(DisplayArr);
		}
                SendLen=1;
		break;
                
             //以下为RF相关操作
        case KF322_INS_OPEN:// open rf
                if ( DataLen <1 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;			
		}
                else
                {
                  UART_SendBuffer[SEND_STATUS]=picc_open();
                }
                SendLen=1;
                break;
        case KF322_INS_SETUP:// setup
                if ( DataLen != sizeof(PICC_PARA)+2 )//校验数据长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                  SendLen=1;
		}
                else
                {
                  memset(&picc_para,0,sizeof(PICC_PARA));
                  var_char1=0;
                  memcpy(&picc_para,&UART_RecBuffer[SUB_DATA_0+1],sizeof(PICC_PARA));//serial
                  
                  UART_SendBuffer[SEND_STATUS] = picc_setup(UART_RecBuffer[SUB_DATA_0],&picc_para);
                  if(UART_SendBuffer[SEND_STATUS] || 
                     UART_RecBuffer[SUB_DATA_0]=='W' || UART_RecBuffer[SUB_DATA_0]=='w')
                  {
                    SendLen = 1;//执行出错或为写模式则直接返回执行状态码
                  }
                  else//成功的读模式
                  {
                    SendLen = 1;
                    memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],&picc_para,sizeof(PICC_PARA));//serial
                    SendLen+=sizeof(PICC_PARA);
                  }	
                }
                break;
        case KF322_INS_DETECT:// detect
                if ( DataLen!=2  )//校验数据长度,只需要接收寻卡模式
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                  SendLen=1;
		}
                else
                {
                  memset(var_Buf1,0,sizeof(var_Buf1));
                  memset(var_Buf2,0,sizeof(var_Buf2));
                  var_char1=0;var_char2=0;
                  UART_SendBuffer[SEND_STATUS] = picc_detect(UART_RecBuffer[SUB_DATA_0],&var_char1,var_Buf1,&var_char2,var_Buf2);
                  if(UART_SendBuffer[SEND_STATUS])
                  {
                    SendLen =1;//执行出错则直接返回错误码
                  }
                  else
                  {
                    SendLen=1;
                    UART_SendBuffer[SEND_STATUS+SendLen]=var_char1;//type
                    SendLen+=1;
                    memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],var_Buf1,var_Buf1[0]+1);//serial
                    SendLen+=var_Buf1[0]+1;
                    UART_SendBuffer[SEND_STATUS+SendLen]=var_char2;//CID
                    SendLen+=1;
                    memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],var_Buf2,var_Buf2[0]+1);//other
                    SendLen+=var_Buf2[0]+1;//all long
                  }
                }
                break;
                
        case KF322_INS_ISOCOMMAND:// iso command
                if (DataLen != (2+sizeof(APDU_SEND)))//校验传输长度
		{
                  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                  SendLen=1;
		}
                else
                {
                  memset(&ApduSend,0,sizeof(APDU_SEND));
                  memset(&ApduRecv,0,sizeof(APDU_RESP));
                  memcpy(&ApduSend,&UART_RecBuffer[SUB_DATA_0+1],sizeof(APDU_SEND));	
                  UART_SendBuffer[SEND_STATUS] = picc_isocommand(UART_RecBuffer[SUB_DATA_0],&ApduSend,&ApduRecv);
                  if(UART_SendBuffer[SEND_STATUS])
                  {
                    SendLen = 1;//执行出错则直接返回错误码
                  }
                  else
                  {
                    SendLen=1;
                    memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],&ApduRecv,sizeof(APDU_RESP));
                    SendLen+=sizeof(APDU_RESP);
                  }
                }
		
                break;
		
        case KF322_INS_REMOVE:// REMOVE
                if (DataLen != 3)//校验传输长度
		{
	          UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                  SendLen=1;
		}
                else
                {
                  UART_SendBuffer[SEND_STATUS] = picc_remove(UART_RecBuffer[SUB_DATA_0], UART_RecBuffer[SUB_DATA_0+1]);
                  SendLen = 1;//执直接返回状态码
                }
                break;
        case KF322_INS_CLOSE:// close
                if ( DataLen != 1 )//校验传输长度
		{
		  UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                  SendLen=1;
		}
                else
                {
                  picc_close();
		  UART_SendBuffer[SEND_STATUS]=0;//always return ok
		  SendLen = 1;
                }
                break;
         case KF322_INS_CMDEXCHANGE:// cmdexchange
                if ( DataLen < 5 || DataLen>512)//校验传输长度
		{
		    UART_SendBuffer[SEND_STATUS] = DATA_LEN_ERR;	
                    SendLen=1;
		} 
		memset(var_Buf1,0,sizeof(var_Buf1));
		memset(var_Buf2,0,sizeof(var_Buf2));
		var_int1=0;var_int2=0;
		//var_int1=(unsigned int)(UART_RecBuffer[SUB_DATA_0]<<24|UART_RecBuffer[SUB_DATA_0+1]<<16|UART_RecBuffer[SUB_DATA_0+2]<<8|UART_RecBuffer[SUB_DATA_0+3]<<0);//uiSendLen	
		
                memcpy(var_Buf1,&UART_RecBuffer[SUB_DATA_0],DataLen);//paucInData
		UART_SendBuffer[SEND_STATUS] = picc_cmdexchange(DataLen, var_Buf1, &var_int2, var_Buf2);
		if(UART_SendBuffer[SEND_STATUS])
		{
			SendLen = 1;//执行出错则直接返回错误码	
		}
		else
		{
			SendLen=1;
			memcpy(&UART_SendBuffer[SEND_STATUS+SendLen],var_Buf2,var_int2);
                        SendLen+=var_int2;
		}
		break;
	default:	//未定义的命令
		UART_SendBuffer[SEND_STATUS]= DATA_INS_ERR;
		SendLen=1;
		break;
	}
	//组装发送包结构
	UART_SendBuffer[SEND_STX]=COM_STX;//STX
        
        UART_SendBuffer[SEND_LEN_H] = (SendLen>>8)&0x0ff;
        UART_SendBuffer[SEND_LEN_L] = SendLen&0x0ff;
	//UART_SendBuffer[SEND_STATUS];
	UART_SendBuffer[SEND_STATUS+SendLen]=GetXorCheck(&UART_SendBuffer[SEND_STATUS], SendLen);//LRC
	UART_SendBuffer[SEND_STATUS+SendLen+1]=COM_ETX;//ETX
        
        //mask recieve int
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        Lib_ComSend(COMM_PORT, UART_SendBuffer, SendLen+5);
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//TXD_EN = 0;	//使能中兴2.4G模块
	return (SendLen+5);
} 


u16 Uart_Protocol(void)
{
    u16 i=0;
    s16 j=0;
    while(1)
    {
      j=PortRecvPack(UART_RecBuffer);
      if( j<0 ) continue;
      Lib_ComReset(COMM_PORT);//此时可以做复位操作
      delay_ms(1);//等待正确包后面的脏数据全部发完以免影响发送包
      i=Protocol_Pro(UART_RecBuffer,j);//执行协议处理							 
    }
}
#endif

#endif