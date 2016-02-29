#include "stm32f10x_it.h"
#include "Toolkit.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_lib.h"

#define FRAME_DELAY_M 40  //us
#define RST_DELAY_M   10000  //uS
#define CS_DELAY_M    10  //uS
#define CMD_DELAY_M   4  //us
#define BYTE_DELAY_M  2   //us
#define END_DELAY_M   2   //us

#define s_DelayUs WaitNuS

#define SPI1_NSS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define SPI1_NSS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define SPI3_NSS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define SPI3_NSS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_15)

#define Lib_sm1           Sm1
#define SMSM2InputSign    SM2_InputSign
#define SMSM2LoadIDSign   SM2_LoadIDSign
#define SMSM2Gen          Sm2_Init //sm2_init ->sm2_Init 
#define SMSM2UnLoad       SM2_UnLoad
#define sm3               Gm_Sm3
#define Lib_sm4           Gm_Sm4   //Gm_sm3->Gm_Sm4
#define SMSM2LoadPK       Sm2_LoadPK //Lib_sm2LoadPK ->Sm2_LoadPK
//#define Sm2_Verify        Gm_Sm2Verify
#define SMSM2Verify       Sm2Verify //Lib_sm2Verify->Sm2Verify
#define SMSM2InputEandGetSign Lib_SM2InputEandGetSign
#define SMSM2LoadIDVerify     Lib_SM2LoadIDVerify

#define printf trace_debug_printf //12/11/08

void SPI3_Init(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  //Enable SPI3 clock
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

  //Configure SPI3 pins: NSS, SCK, MISO and MOSI
  /*
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //Configure PA15 as Output push-pull, used as Flash Chip select
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //SPI3 Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI_Direction_2Lines_FullDuplex->SPI_Direction_1Line_Tx
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;//SPI_Mode_Master->SPI_Mode_Slave
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  
//#ifdef TEST_SM_M 
  
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//SPI_CPOL_Low->SPI_CPOL_High 13/05/31
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;//SPI_CPHA_1Edge->SPI_CPHA_2Edge
  //test
  trace_debug_printf("SPI_CPHA_2Edge\n");
/* 
#else
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

  //test
  trace_debug_printf("SPI_CPHA_1Edge\n");
  
#endif  
*/
  
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//SPI_FirstBit_MSB->SPI_FirstBit_LSB
  SPI_Init(SPI3, &SPI_InitStructure);

  SPI3_NSS_HIGH();  
  //SPI3 enable
  SPI_Cmd(SPI3, ENABLE);
}

u8 SPI3DataSendReceive(u8 DATA)
//vu8 SPIDataSendReceive(u8 DATA)
{
    /* Loop while DR register in not emplty */
    while(SPI_GetFlagStatus(SPI3, SPI_FLAG_TXE) == RESET);
    /* Send byte through the SPI1 peripheral */
    SPI_SendData(SPI3, DATA);
    /* Wait to receive a byte */
    while(SPI_GetFlagStatus(SPI3, SPI_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
    return SPI_ReceiveData(SPI3);
}

unsigned char calc_lrc(const unsigned char * buf, unsigned int len)
{
	unsigned int i;
	unsigned int lrc = 0;
	for(; len>0; len--, buf++)
	{
		lrc ^= *buf;
	}
	return (lrc&0xff);
}

int SMSendApdu(unsigned int nLen,unsigned char *sData)
{
  unsigned char bLrc;
  unsigned int i;
  static unsigned char bTestFlag=0;
  
#if 0
  if(bTestFlag==0)
  {
    SelfTest();
    bTestFlag = 1;
  }
#endif
  
#if 0
  //test
  printf("send:");
  for(i=0;i<nLen;i++)
    printf("%02x ",sData[i]);
  printf("\r\n");
#endif
  
  
  bLrc = calc_lrc(sData,nLen)^0xff;
  bLrc ^= nLen&0xff;
  bLrc ^= (nLen>>8)&0xff;
  // Send apdu
  s_DelayUs(FRAME_DELAY_M);
  SPI3_NSS_LOW();
  s_DelayUs( CS_DELAY_M);
  

  SPI3DataSendReceive(0x02);
  s_DelayUs( CMD_DELAY_M);

  
  SPI3DataSendReceive((nLen>>8)&0xff);
  SPI3DataSendReceive(nLen&0xff);
  for(i=0;i<nLen;i++)
    SPI3DataSendReceive(sData[i]);
  SPI3DataSendReceive(bLrc);
  s_DelayUs(END_DELAY_M);
  SPI3_NSS_HIGH();  
  return 0;
}

int SMCheck()
{
  unsigned char bVal;
  s_DelayUs(FRAME_DELAY_M);
  SPI3_NSS_LOW();
  s_DelayUs(CS_DELAY_M);
  SPI3DataSendReceive(0x05);
  s_DelayUs(CMD_DELAY_M);
  
  bVal = SPI3DataSendReceive(0x00);
  
  SPI3_NSS_HIGH();  
  return bVal;
  
}

int SMGetResponse(unsigned int *pnRecLen,unsigned char *sRecBuf)
{
  unsigned int nLen,i;
  unsigned char bLrc;
  
  s_DelayUs(FRAME_DELAY_M);
  SPI3_NSS_LOW();
  s_DelayUs(CS_DELAY_M);
  SPI3DataSendReceive(0x03);
  s_DelayUs(CMD_DELAY_M);
  
  nLen = SPI3DataSendReceive(0x00);
  nLen <<=8;

  nLen |= SPI3DataSendReceive(0x00);
  nLen &=0x7ff;
  for(i=0;i<nLen;i++)
    sRecBuf[i] = SPI3DataSendReceive(0x00);
  bLrc = SPI3DataSendReceive(0x00);
  
  SPI3_NSS_HIGH();  

  bLrc ^= (nLen&0xff);
  bLrc ^= (nLen>>8)&0xff;
  bLrc ^= calc_lrc(sRecBuf, nLen)^0xff;
  
  if(bLrc != 0) 
  {
  //printf("calc lrc failed: %02x\n", lrc);
  // here can try again to get response or return error
  return -1;

  }
  *pnRecLen = nLen;
  return 0;
  
}

int SMGetRand(unsigned int nLc,unsigned char *sRand)
{
  
  unsigned char sBuf[1024+5];
  int nRet;
  unsigned int nLen;
  memcpy(sBuf,"\xb0\xa0\x0\x0",4);
  if(nLc>0xff)
  {
    sBuf[4] = 0;
    sBuf[5] = nLc/0x100;
    sBuf[6] = nLc%0x100;
    SMSendApdu(5+2,sBuf);
    
  }
  else
  {
    sBuf[4] = (unsigned char)nLc;
    SMSendApdu(5,sBuf);
  }
  
  while(1)
  {
     if(SMCheck()==0)
       break;
     delay_ms(2);//10->2
  }

  nLen = 0;
  nRet = SMGetResponse(&nLen,sBuf);  
  if(nRet==0 )
  {
    if(memcmp(&sBuf[nLen-2],"\x90\x00",2)==0)
    memcpy(sRand,sBuf,nLc);
    else
      return -2;
  }
  return nRet;
}


//加密初始化
/*
mode:1-ENCRYPT

*/
int SMSYMINIT(unsigned char *smkey,int mode)
{
     unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
     //test
     int i;
     //加密初始化
     memcpy(sBuf,"\xB0\x8E\x00\x00\x00",5);
     sBuf[3] =(unsigned char)mode;
     
#if 0
     if(mode==ENCRYPT)
      sBuf[3] = 0x83;
     else
      sBuf[3] = 0x03;
#endif
     
     sBuf[4] = 16;
     memcpy(&sBuf[5],smkey,16);
     
#if 0
     //test
     printf("send:");
     for(i=0;i<5+16;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
     
     SMSendApdu(5+16,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nRet = SMGetResponse(&nLen,sBuf);
     
     if(nRet==0)
     {
       if(memcmp(sBuf,"\x90\x00",2)==0)
       {
#if 0
     //test
     printf("resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
         return 0;
       }
       else
         return -2;
     }
     
     
     return nRet;      
}

 //加密中间值
int SMSYMMIDD(unsigned char *input,unsigned int input_len,unsigned char *output)
{
     unsigned char sBuf[1024+10];
     int nRet;
     unsigned int nLen;
    //test
     int i;
     memset(sBuf,0,sizeof(sBuf));
     memcpy(sBuf,"\xB0\x8E\x01\x00\x00",5);
     if(input_len>0xff)
     {
       sBuf[4] = 0;
       sBuf[5] = input_len/0x100;
       sBuf[6] = input_len%0x100;
       
       //sBuf[4] = (unsigned char)input_len;
       memcpy(&sBuf[7],input,input_len);
     //sBuf[5+input_len] = 16;//le
     
     
      SMSendApdu(5+2+input_len,sBuf);
     }
     else
     {
      sBuf[4] = (unsigned char)input_len;
      memcpy(&sBuf[5],input,input_len);
      //sBuf[5+input_len] = 16;//le
     
     
      SMSendApdu(5+input_len,sBuf);
       
     }
#if 0
     //test
     printf("send:");
     for(i=0;i<5+input_len;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
     
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nLen = 0;
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet==0)
     {
#if 0
     //test
     printf("11 resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
       if(memcmp(&sBuf[nLen-2],"\x90\x00",2)==0)
       {
         memcpy(output,sBuf,nLen);
         return 0;
       }
       else
       {
#if 0
         //test
         printf("nLen[%d] sw[%02x %02x]\r\n",nLen,sBuf[0],sBuf[1]);
#endif    
         return -2;
       }
     }
     
     
     return nRet;        
}

int SMSYMEND(void)
{
     unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
     //test
     int i;
     
     //加密结束
     memcpy(sBuf,"\xB0\x8E\x02\x00\x00",5);
     SMSendApdu(5,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet==0)
     {
#if 0
     //test
     printf("resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif  
       if(memcmp(sBuf,"\x90\x00",2)==0)
       {
         
         return 0;
       }
       else
         return -2;
     }
     
     
     return nRet;         
}

/*
mode:
  0-SHA1
  1-SHA256
  2-SM3
  3-MD5
*/
int SMHashINIT(int mode)
{
     unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
     //test
     int i;
     //杂凑初始化
     memcpy(sBuf,"\xB0\x8C\x00\x00\x00",5);
     sBuf[3] = mode;

     
 
     
     SMSendApdu(5,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nRet = SMGetResponse(&nLen,sBuf);
     
     if(nRet==0)
     {
       if(memcmp(sBuf,"\x90\x00",2)==0)
       {
#if 0
     //test
     printf("resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
         return 0;
       }
       else
         return -2;
     }
     
     
     return nRet;      
}

//杂凑中间值
int SMHashMIDD(unsigned char *input,unsigned int input_len)
{
     unsigned char sBuf[1024+10];
     int nRet;
     unsigned int nLen;
    //test
     int i;
     memset(sBuf,0,sizeof(sBuf));
     memcpy(sBuf,"\xB0\x8C\x01\x00\x00",5);
     if(input_len>0xff)
     {
       sBuf[4] = 0;
       sBuf[5] = input_len/0x100;
       sBuf[6] = input_len%0x100;
       
       //sBuf[4] = (unsigned char)input_len;
       memcpy(&sBuf[7],input,input_len);
     //sBuf[5+input_len] = 16;//le
     
     
      SMSendApdu(5+2+input_len,sBuf);
     }
     else
     {
      sBuf[4] = (unsigned char)input_len;
      memcpy(&sBuf[5],input,input_len);
      //sBuf[5+input_len] = 16;//le
     
     
      SMSendApdu(5+input_len,sBuf);
       
     }
#if 0
     //test
     printf("send:");
     for(i=0;i<5+input_len;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
     
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nLen = 0;
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet==0)
     {
#if 0
     //test
     printf("11 resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif
       if(memcmp(&sBuf[0],"\x90\x00",2)==0)
       {
         //memcpy(output,sBuf,nLen);
         return 0;
       }
       else
       {
#if 0
         //test
         printf("nLen[%d] sw[%02x %02x]\r\n",nLen,sBuf[0],sBuf[1]);
#endif    
         return -2;
       }
     }
     
     
     return nRet;        
}

int SMHashEND(unsigned char *sOutput )
{
     unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
     //test
     int i;
     
     //杂凑结束
     memcpy(sBuf,"\xB0\x8C\x02\x00\x00",5);
     sBuf[4] = 32;
     SMSendApdu(5,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet==0)
     {
#if 0
     //test
     printf("resp:");
     for(i=0;i<nLen;i++)
       printf("%02x ",sBuf[i]);
     printf("\r\n");
#endif  
       if(memcmp(&sBuf[32],"\x90\x00",2)==0)
       {
         memcpy(sOutput,sBuf,32);
         return 0;
       }
       else
         return -2;
     }
     
     
     return nRet;         
}

/****************************************************************************
  函数名     : Lib_sm1
  描述       : SM1加解密
  输入参数   :
			 input:最多为1024字节
			 mode-1为加密
			      0为解密
  输出参数   :
			output-输出16字节
  返回值     :
             0-成功
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/

int Sm1(unsigned char *input,unsigned int input_len,unsigned char *output,unsigned char *smkey,int mode)
{
  
     //unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
     
    // Lib_KbGetCh();
     
     //printf("Lib_sm1 begin\r\n");

     //加密初始化
     if(mode==1)
      nRet = SMSYMINIT(smkey,0x83);
     else
      nRet = SMSYMINIT(smkey,0x03);
     
     if(nRet)
     {
       return nRet;
     }
     
     //加密中间值
     nRet = SMSYMMIDD(input,input_len,output);

#if 0
     
     printf("SMSYMMIDD[%d]\r\n",nRet);
     if(nRet)
     {
       
       return nRet;
     }
#endif
     
     //加密结束
     return SMSYMEND();
}

/****************************************************************************
  函数名     :Lib_sm2init
  描述       :sm2 初始化
  输入参数   :
  输出参数   :
  返回值     :
              0-成功
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/

int Sm2_Init()
{
  unsigned char sBuf[100];
  int nRet;
  unsigned int nLen;

  printf("Gm_Sm2Init\r\n");
  memcpy(sBuf,"\xB0\xB0\x00\x00\x00",5); //

  SMSendApdu(5,sBuf);
  while(1)
  {
      if(SMCheck()==0)
        break;
      delay_ms(1);
  }
  nRet = SMGetResponse(&nLen,sBuf);
  if(nRet==0)
  {
    if(memcmp(sBuf,"\x90\x00",2)==0)
      return 0;
    else
      return -2;
  }
  return nRet;
  
}

/****************************************************************************
  函数名     :Lib_sm2LoadPK
  描述       :sm2 导入公钥对
  输入参数   :
  输出参数   :
  返回值     :
              0-成功
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Sm2_LoadPK(unsigned char *sPx,unsigned char *sPy)
{
     unsigned char sBuf[100];
     int nRet;
     unsigned int nLen;

	 printf("Gm_Sm2LoadPK\r\n");
     //memcpy(sBuf,"\xB0\xB4\x00\x02\x20\x39\x88\x36\xF2\xA2\xC5\x1E\xEB\xB8\x37\x06\x0D\x1F\x34\x07\x66\x57\x71\xB4\xD6\x0E\x1F\x5E\x19\x61\x82\x6E\xBB\x9B\x48\xD3\x3D",37);
     memcpy(sBuf,"\xB0\xB4\x00\x02\x20",5);
     memcpy(&sBuf[5],sPx,32);
     SMSendApdu(37,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet)
       return nRet;
     if(memcmp(sBuf,"\x90\x00",2))
       return -2;
     
     //y
     //memcpy(sBuf,"\xB0\xB4\x00\x03\x20\x42\x91\x74\xC5\x7F\xAB\x0A\x9C\xDD\xD4\xE7\x48\xE3\x37\x11\xA9\x8A\x66\x0C\x49\x77\xFF\xC2\x3F\xC5\x0A\xB0\x92\xD8\x17\x46\xAD",37);
     memcpy(sBuf,"\xB0\xB4\x00\x03\x20",5);
     memcpy(&sBuf[5],sPy,32);
     SMSendApdu(37,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf);
     
     if(nRet)
       return nRet;
     if(memcmp(sBuf,"\x90\x00",2))
       return -2;
     
     return nRet;     
     
}
//sxl
int Sm2_Load_PrivateKey(unsigned char *d)
{
     unsigned char sBuf[100];
     int nRet;
     unsigned int nLen;
	 printf("Sm2_Load_PrivateKey\r\n");
     memcpy(sBuf,"\xB0\xB4\x00\x01\x20",5);
     memcpy(&sBuf[5],d,32);
     SMSendApdu(37,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet)
       return nRet;
     if(memcmp(sBuf,"\x90\x00",2))
       return -2;
     return nRet;     
}
//sxl
int Sm2_GetZ(unsigned char *id, int id_len, unsigned char *Z)
{
  unsigned char sBuf[100];
  int nRet;
  unsigned int nLen, count;
  count = 0;
  memcpy(sBuf,"\xB0\x74\x01\x00",4); 
  count += 4;
  sBuf[4] = id_len;
  count += 1;
  memcpy(&sBuf[count], id, id_len);
  count += id_len;  
  SMSendApdu(count,sBuf);
  while(1)
  {
      if(SMCheck()==0)
        break;
      delay_ms(1);
  }
  nRet = SMGetResponse(&nLen,sBuf);
  if(nRet==0)
  {
    if(memcmp(&sBuf[nLen-2],"\x90\x00",2)==0) {
	  memcpy(Z, sBuf, 32);
      return 0;
    }
    else
      return -2;
  }
  return nRet;	
}
static const unsigned char sm2_param[128] = {
0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
0x28,0xE9,0xFA,0x9E,0x9D,0x9F,0x5E,0x34,0x4D,0x5A,0x9E,0x4B,0xCF,0x65,0x09,0xA7,
0xF3,0x97,0x89,0xF5,0x15,0xAB,0x8F,0x92,0xDD,0xBC,0xBD,0x41,0x4D,0x94,0x0E,0x93,
0x32,0xC4,0xAE,0x2C,0x1F,0x19,0x81,0x19,0x5F,0x99,0x04,0x46,0x6A,0x39,0xC9,0x94,
0x8F,0xE3,0x0B,0xBF,0xF2,0x66,0x0B,0xE1,0x71,0x5A,0x45,0x89,0x33,0x4C,0x74,0xC7,
0xBC,0x37,0x36,0xA2,0xF4,0xF6,0x77,0x9C,0x59,0xBD,0xCE,0xE3,0x6B,0x69,0x21,0x53,
0xD0,0xA9,0x87,0x7C,0xC6,0x2A,0x47,0x40,0x02,0xDF,0x32,0xE5,0x21,0x39,0xF0,0xA0,
}; 
int Sm2_GetE(const unsigned char *id, unsigned int idLen, 
   const unsigned char *publicKey, const unsigned char *msg, unsigned int msgLen, 
   unsigned char *e)
{
	unsigned char buf[1024], h[32];
	int totalLen;
	int resLen;
	unsigned char *p;
	 
	//第一次sm3: id的BIT长度（2字节) + id + sm2的那些参数128字节+公钥（0x40字节)
	buf[0] = ((idLen * 8) >> 8) & 0xff;
	buf[1] = (idLen * 8) & 0xff;
	memcpy(buf + 2, id, idLen);
	totalLen = 2 + idLen;
	memcpy(buf + totalLen, sm2_param, 128);
	totalLen += 128;
	memcpy(buf + totalLen, publicKey, 0x40);
	totalLen += 0x40;
	if(Gm_Sm3(buf, totalLen, h) != 0)
	{
		printf("sm3 c1error");
		return -1;
	}	 
	//第二次Sm3: 第一次算出的哈希值+要验证的消息数据
	memcpy(buf, h, 0x20);
	totalLen = 0x20;
	memcpy(buf + totalLen, msg, msgLen);
	totalLen += msgLen;
	if(Gm_Sm3(buf, totalLen, h) != 0)
	{
		printf("sm3 c2error");
		return -2;
	}
	memcpy(e, h, 0x20);
	return 0;
}

/****************************************************************************
  函数名     :Lib_sm2Verify
  描述       :对输入的杂凑值E与 (R,S)验签
  输入参数   :
              E-32 字节
  输出参数   :
  返回值     :
              0-成功
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Sm2Verify(unsigned char *E,unsigned char * R,unsigned char * S)
{
     unsigned char sBuf[200];
     int nRet;
     unsigned int nLen;
  
     //memcpy(sBuf,"\xB0\x74\x00\x00\x60\xDC\xF5\x5F\x9B\x30\xED\xC1\x30\x79\x60\x6D\x62\x80\xB7\x52\x58\x7B\xDB\xA6\x2F\x47\x1A\x3D\xA7\x7A\xA3\x49\x73\xAA\x3E\x47\x14\x9F\x8E\x28\xEE\x47\x05\x7B\x7B\xD0\x01\xB6\x11\x39\x1D\x80\xDF\x5D\xA7\x20\x3F\x3D\x30\x0E\xEC\xFB\x32\x43\xC4\x49\x8B\xA1\x0C\xFE\x3F\xB9\x1A\xB1\x5A\xA0\x37\x62\x10\xA3\xF6\x75\x11\x93\x0C\x2E\xF0\x32\x97\x01\x71\x67\xB5\x2A\xE5\x06\x03\xCA\x34\xDF\xED",101);
	 printf("Gm_Sm2Verify\r\n");
	 memcpy(sBuf,"\xB0\x74\x00\x00\x60",5);
     
     memcpy(&sBuf[5],E,32);
     memcpy(&sBuf[5+32],R,32);
     memcpy(&sBuf[5+64],S,32);
     
     SMSendApdu(101,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      //delay_ms(10);
      delay_ms(1);
      
     }
     nRet = SMGetResponse(&nLen,sBuf);
     if(nRet==0)
     {
       if(memcmp(sBuf,"\x90\x00",2)==0)
         return 0;
       else
         return -2;
     }
     return nRet;
}

//int Gm_Sm2Verify(unsigned char *user_id, int userid_len,const 
//unsigned char * public_key,unsigned char * signed_data, unsigned char *msg, int msg_len)
Sm2_Verify(unsigned char *user_id, int userid_len,const 
  unsigned char * public_key,unsigned char * signed_data, unsigned char *msg, int msg_len)
{
	unsigned char px[32], py[32];
	unsigned char E[32], R[32], S[32];
	if (Sm2_Init() != 0)
		return -1;
	memcpy(px, public_key, 32);
	memcpy(py, &public_key[32], 32);	
    if (Sm2_LoadPK(px, py) != 0)
		return -2;
	if (Sm2_GetE(user_id, userid_len, public_key, msg, msg_len, E) != 0)
		return -3;
	memcpy(R, signed_data, 32);
	memcpy(S, &signed_data[32], 32);
	if (Sm2Verify(E,R,S) != 0)
		return -4;

	return 0;
}

//sxl 14/3/25
int Sm2_ExportPK(unsigned char bPara,unsigned char *sOutput)
{
  unsigned char sBuf[100];
  int nRet;
  unsigned int nLen;
  memcpy(sBuf,"\xB0\xB2\x00\x00\x20",5); //
  sBuf[3] = bPara;
  SMSendApdu(5,sBuf);
  while(1)
  {
      if(SMCheck()==0)
        break;
      delay_ms(1);
  }
  nRet = SMGetResponse(&nLen,sBuf);
  if(nRet==0)
  {
    if(memcmp(&sBuf[32],"\x90\x00",2)==0)
    {
      memcpy(sOutput,sBuf,32);
      return 0;
    }
    else
      return -2;
  }
  return nRet;  
}
//sxl
int Lib_SM2GetSign(unsigned char *sOutputE,unsigned char *sOutputR,unsigned char *sOutputS)
{
  unsigned char sBuf[200];
  int nRet;
  unsigned int nLen=0;
  memcpy(sBuf,"\xB0\x72\x03\x01\x60",5); //00->01
  SMSendApdu(5,sBuf);
  while(1)
  {
      if(SMCheck()==0)
        break;
      delay_ms(1);
  }
  nRet = SMGetResponse(&nLen,sBuf);
  if(nRet==0)
  {
    if(memcmp(&sBuf[32*3],"\x90\x00",2)==0)
    {
      memcpy(sOutputE,sBuf,32);
      memcpy(sOutputR,&sBuf[32],32);
      memcpy(sOutputS,&sBuf[32*2],32);
      return 0;
    }
    else
    {
      printf("SW[%02x %02x]\r\n",sBuf[64],sBuf[65]);
      return -2;
    }
  }
  return nRet;  
}
//sxl
int Lib_SM2LoadIDSign(unsigned char bInputLen,unsigned char *sInputID)
{
     unsigned char sBuf[100];
     int nRet;
     unsigned int nLen;
     //memcpy(sBuf,"\xB0\x72\x01\x00\x08\x11\x22\x33\x44\x55\x66\x77\x88\x20",14); //
     memcpy(sBuf,"\xB0\x72\x01\x01\x08",5); //00->01
     sBuf[4] = bInputLen;
     memcpy(&sBuf[5],sInputID,bInputLen);
     sBuf[5+bInputLen] = 0x20;
     //SMSendApdu(14,sBuf);
     SMSendApdu(5+bInputLen+1,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf); 
    if(nRet==0)
    {
      if(memcmp(&sBuf[32],"\x90\x00",2)==0)
        return 0;
      else
      {
        printf("SW[%02x %02x]\r\n",sBuf[0],sBuf[1]);
        return -2;
      }
    }
    return nRet;
}
//sxl
int Lib_SM2InputEandGetSign(unsigned char *sInputE,unsigned char *sOutputR,unsigned char *sOutputS)
{
  unsigned char sBuf[100];
  int nRet;
  unsigned int nLen;
  memcpy(sBuf,"\xB0\x72\x00\x00\x20",5); //
  memcpy(&sBuf[5],sInputE,32);
  sBuf[5+32] = 0x40;
  SMSendApdu(5+32+1,sBuf);
  while(1)
  {
      if(SMCheck()==0)
        break;
      delay_ms(1);
  }
  nRet = SMGetResponse(&nLen,sBuf);
  if(nRet==0)
  {
    if(memcmp(&sBuf[64],"\x90\x00",2)==0)
    {
      memcpy(sOutputR,sBuf,32);
      memcpy(sOutputS,&sBuf[32],32);
      return 0;
    }
    else
    {
      printf("SW[%02x %02x]\r\n",sBuf[64],sBuf[65]);
      return -2;
    }
  }
  return nRet;  
}
//sxl
int Lib_SM2InputSign(unsigned char bInputLen,unsigned char *sInput)
{
     unsigned char sBuf[100];
     int nRet;
     unsigned int nLen;
     memcpy(sBuf,"\xB0\x72\x02\x01\x00",5); //00->01
     sBuf[4] = bInputLen;
     memcpy(&sBuf[5],sInput,bInputLen);
     //SMSendApdu(14,sBuf);
     SMSendApdu(5+bInputLen,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf); 
    if(nRet==0)
    {
      if(memcmp(&sBuf[0],"\x90\x00",2)==0)
        return 0;
      else
        return -2;
    }
    return nRet;
}
//sxl
int Lib_SM2LoadIDVerify(unsigned char bInputLen,unsigned char *sInputID)
{
     unsigned char sBuf[100];
     int nRet;
     unsigned int nLen;
     memcpy(sBuf,"\xB0\x74\x01\x00\x08",5); //
     sBuf[4] = bInputLen;
     memcpy(&sBuf[5],sInputID,bInputLen);
     sBuf[5+bInputLen] = 0x20;
     SMSendApdu(5+bInputLen+1,sBuf);
     while(1)
     {
      if(SMCheck()==0)
        break;
      delay_ms(1);
     }
     nRet = SMGetResponse(&nLen,sBuf); 
    if(nRet==0)
    {
      if(memcmp(&sBuf[32],"\x90\x00",2)==0)
        return 0;
      else
        return -2;
    }
    return nRet;
}
//sxl
int Gm_Sm2Sign(unsigned char *user_id, int userid_len, const unsigned char *public_key,
	unsigned char *private_Key, unsigned char *sign, unsigned char *msg,int msg_len)
{
	unsigned char px[32], py[32], d[32];
	unsigned char E[32], R[32], S[32];
	int i;
	if (Sm2_Init() != 0)
		return -1;
	memcpy(d, private_Key, 32); 
	if (Sm2_Load_PrivateKey(d) != 0)
		return -2;
	memcpy(px, public_key, 32);
	memcpy(py, &public_key[32], 32);
    if (Sm2_LoadPK(px, py) != 0)
		return -3;
	#if 0
	if (Lib_SM2InputSign(msg_len, msg)!=0)
		return -4;
	if (Lib_SM2GetSign(E,R,S))
		return -5;
	if(Sm2Verify(E,R,S))
		s_UartPrint("verify err in sing\r\n");
	#else
	if (Sm2_GetE(user_id, userid_len, public_key, msg, msg_len, E))
		return -4;
	if (Lib_SM2InputEandGetSign(E, R, S)!=0)
		return -6;
	#endif
	memcpy(sign, R, 32);
	memcpy(sign+32, S, 32);
	return 0;
}
static unsigned char sm2_public_key[] = 
{
	0xDF,0xEF,0x37,0x7B,0xE5,0x2F,0x4D,0x03,0x09,0xC7,0x6E,0xEC,0x6F,0x15,0xF6,0xD6,
	0x5C,0xBE,0x12,0x82,0xD2,0xAB,0x84,0x41,0x31,0xE2,0x5A,0x38,0xC0,0x8C,0xF4,0x25,
	0x29,0x39,0xEB,0xC6,0x1A,0x62,0xBF,0x6F,0x39,0xF9,0x2B,0x44,0x59,0xBE,0x2E,0xE4,
	0x17,0x13,0x38,0x43,0xE2,0x47,0xF7,0xC8,0xAC,0x5F,0x53,0xB1,0x35,0x80,0x63,0xA5
};
static unsigned char sm2_private_key[] =
{
	0x87,0x21,0x44,0x1C,0xC2,0x7D,0x73,0x97,0x1F,0x35,0x17,0xAE,0x33,0xCE,0xF5,0xC7,
	0x7B,0x6F,0xD3,0x37,0x5F,0x3F,0x40,0x4F,0x4D,0x17,0x09,0xF3,0x0A,0xEA,0xFB,0xFF
};
static unsigned char sm2_signer_id[] =
{
	0x30,0xD2,0xF3,0xB2,0xD2,0x51,0x06,0xB4,0x16,0x98,0x5A,0x5A,0xE7,0xD6,0xB6,0xC7,
	0x47,0x76,0x1A,0x29,0xAF,0xD6,0xB9,0x98,0x64,0x7E,0x3D,0x64,0x06,0x21,0x34,0xB0
};
static unsigned char sm2_data_len = 0x20;
static unsigned char sm2_data[] = 
{
	0x9F,0x8A,0xFD,0x39,0x77,0xED,0xBE,0xFD,0xFD,0x65,0x33,0x1D,0x23,0x67,0xF1,0x15,
	0x08,0x80,0x2F,0x50,0xEB,0x65,0x74,0x71,0xB5,0x34,0x7F,0x7E,0x04,0x21,0x08,0xB3
};
static unsigned char sm2_signed_data[] = 
{
	0x29,0xF0,0x66,0x2E,0xEE,0xD2,0x75,0xAB,0x17,0xBC,0xA4,0xE9,0x04,0x3D,0x1F,0xFC,
	0xEE,0xF9,0x90,0xF2,0xE9,0x06,0xAE,0x3C,0x46,0x39,0x85,0x74,0x42,0x1A,0x21,0x8D,
	0x30,0x9D,0x8E,0xE5,0x19,0xAA,0x9A,0x7D,0x77,0xEA,0xF5,0xE4,0x36,0xBC,0xC6,0xCC,
	0x68,0x59,0x4A,0x3A,0x9D,0xC1,0x86,0xD2,0x33,0xC2,0xCD,0xD5,0xAD,0x11,0xD5,0xE6
};
/****************************************************************************
  函数名     :Lib_sm3
  描述       :杂凑值
  输入参数   :
  输出参数   :
  返回值     :
           
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/

int Gm_Sm3( unsigned char *input, int ilen, unsigned char *output)
{
  int nRet;

  nRet = SMHashINIT(2);
  if(nRet)
	return nRet;

  nRet = SMHashMIDD(input,ilen);
  if(nRet)
	return nRet;

  nRet = SMHashEND(output);
  if(nRet)
	return nRet;

  return 0;
}

/****************************************************************************
  函数名     : Lib_sm4
  描述       : SM4加解密
  输入参数   :
			 input:最多为1024字节
			 mode-1为加密
			      0为解密
  输出参数   :
			output-输出16字节
  返回值     :
           0-成功
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Gm_Sm4(unsigned char *input,unsigned int input_len,unsigned char *output,unsigned char *smkey,int mode)
{
     int nRet;
     unsigned int nLen;
     if(mode)
         nRet = SMSYMINIT(smkey,0x85);
     else
         nRet = SMSYMINIT(smkey,0x05);
     if(nRet)
         return nRet;
     //加密中间值
     nRet = SMSYMMIDD(input,input_len,output);
	 if (nRet)
	     return nRet;
	 nRet = SMSYMEND();
	 if (nRet)
	     return nRet;
#if 0
     
     printf("SMSYMMIDD[%d]\r\n",nRet);
     if(nRet)
     {
       
       return nRet;
     }
#endif
     
     //加密结束
     return 0;  
}

#if 1
static const unsigned char publicKey[0x40] =  //公钥
{
	0x92,0xE1,0x4A,0xCD,0x01,0xE3,0x70,0x75,0xC8,0x34,0x9C,0x60,0xAE,0x98,0xFA,0x9E,
	0x43,0xA0,0x6A,0x96,0xF6,0x80,0xD2,0x08,0xB0,0x4B,0x3A,0x2D,0x63,0x97,0xE5,0xFC,
	0xFE,0x9E,0x37,0x29,0x9D,0x3C,0x6F,0x33,0xC0,0x6E,0xB6,0xF4,0xB7,0x4E,0xAC,0xAC,
	0x89,0x53,0x41,0x36,0x12,0x19,0x09,0xB4,0x16,0x13,0x42,0x76,0x37,0x88,0xF6,0x5D
};
 
static unsigned char id[16] = //用户ID
{
	 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 
};
 
static unsigned char msg[14 + 0x40] = //验证的消息
{
	0x12,0x62,0x22,0x35,0xFF,0x12,0x30,0x00,0x00,0x03,0x04,0x00,0x11, 0x40,
	0xC8,0x4C,0x27,0xE5,0x85,0xC6,0x5B,0xEB,0xFE,0x8A,0xDC,0x04,0x44,0x67,0x1C,0x59,
	0x36,0x10,0x2F,0xFC,0x4B,0x89,0x41,0xFF,0x14,0x57,0xCF,0x73,0xAB,0x3E,0xE4,0xAA,
	0x16,0xAF,0xC2,0x6B,0x5D,0x20,0x85,0xC2,0xF1,0x23,0x86,0x83,0xD8,0x3C,0x81,0x09,
	0x10,0xE5,0xFE,0x6A,0x3A,0xCC,0xDE,0x64,0x01,0xE6,0xA7,0x75,0xC3,0x90,0x8B,0xDD
};
 
static unsigned char sign[0x40] = //对数据的签名
{
	0x58,0x32,0x55,0x8B,0x8E,0x42,0x73,0x46,0xAE,0x8F,0xCD,0xD1,0x18,0x8F,0x3D,0xDE,
	0xD3,0xAE,0x68,0x7E,0x4A,0x77,0x8D,0xD6,0x69,0x7F,0xC5,0x71,0x5A,0xC2,0x27,0x8B,
	0x86,0x66,0x99,0x2D,0xB8,0x87,0xCE,0x43,0x57,0x09,0x4D,0xA8,0x35,0x9C,0x3A,0x44,
	0x00,0x60,0x1C,0x72,0x1A,0x59,0x6D,0xB1,0x16,0x34,0x59,0xC8,0x4C,0x11,0x4C,0x84
};

static unsigned char sm4_data[16] =
{
	0xAA,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88

};

static unsigned char sm4_key[16] =
{
	0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88

};
void gm_test(void)
{
	unsigned char out[16];

	printf("********Sm4 Test********\r\n");
	Gm_Sm4(sm4_data,16, out, sm4_key, 1);

	printf("********Sm2 Test********\r\n");
	//Gm_Sm2Verify(id, 16, publicKey, sign, msg, sizeof(msg));
        Sm2_Verify(id, 16, publicKey, sign, msg, sizeof(msg));
	printf("********Finish********\r\n");

	Kb_GetKey();
}
#endif


int SMSM1SelfTest(void)
{
  unsigned char sKey[16];
  unsigned char sInput[16];
  unsigned char sOutput[16],sBuf[100];
  memcpy(sKey,"\xD6\x1D\xA6\x8C\xE0\xC4\x26\x61\x0A\x97\x8E\x68\xEB\xC2\xA9\x8E",16);
  memcpy(sInput,"\xA6\x2A\x0A\x3E\xBC\xA8\x85\x36\xAA\x0A\x56\xA7\xB6\x81\x48\x3E",16);
  memcpy(sOutput,"\x8E\x4B\x87\x01\x0B\xCD\xE1\x13\x22\x09\x12\x7F\xEB\x06\xAA\x70",16);
  Lib_sm1(sInput,16,sBuf,sKey,1); 
  if(memcmp(sBuf,sOutput,16))
    return 1;
  Lib_sm1(sOutput,16,sBuf,sKey,0); 
  if(memcmp(sBuf,sInput,16))
    return 1;
  return 0;
}

int SMSM2SelfTest(void)
{
    int iret;
  
    iret = SMSM2Gen();
    printf("SMSM2Gen[%d]\r\n",iret);
    //iret = SMSM2LoadIDVerify(32,"\x1D\x18\x8B\x6F\x97\x9D\xB6\x3D\xF1\x5D\x93\x33\x54\x3D\xA9\x39\x69\x25\x4B\xD4\x30\x70\x7B\x62\x9D\xE2\x1D\x3F\x29\xAD\x49\x23");
    iret = SMSM2LoadIDVerify(8,"\x11\x22\x33\x44\x55\x66\x77\x88");
    printf("SMSM2LoadIDVerify[%d]\r\n",iret);
    
#if 0
    //iret = SMSM2Load(2,"\x1D\x18\x8B\x6F\x97\x9D\xB6\x3D\xF1\x5D\x93\x33\x54\x3D\xA9\x39\x69\x25\x4B\xD4\x30\x70\x7B\x62\x9D\xE2\x1D\x3F\x29\xAD\x49\x23");    
    iret = SMSM2Load(2,"\x39\x88\x36\xF2\xA2\xC5\x1E\xEB\xB8\x37\x06\x0D\x1F\x34\x07\x66\x57\x71\xB4\xD6\x0E\x1F\x5E\x19\x61\x82\x6E\xBB\x9B\x48\xD3\x3D");    
    printf("SMSM2Load[%d]\r\n",iret);
    //iret = SMSM2Load(3,"\x5C\xF7\x4F\x72\x80\x44\x46\xC1\x48\x40\x80\xB7\x78\xF8\xD7\xDD\x6B\x1D\x32\x11\x28\x35\x20\x1C\xC8\xBA\xE0\x6A\xA8\xDB\x0C\xD7");    
    iret = SMSM2Load(3,"\x42\x91\x74\xC5\x7F\xAB\x0A\x9C\xDD\xD4\xE7\x48\xE3\x37\x11\xA9\x8A\x66\x0C\x49\x77\xFF\xC2\x3F\xC5\x0A\xB0\x92\xD8\x17\x46\xAD");    
#endif
    
    iret = SMSM2LoadPK("\x39\x88\x36\xF2\xA2\xC5\x1E\xEB\xB8\x37\x06\x0D\x1F\x34\x07\x66\x57\x71\xB4\xD6\x0E\x1F\x5E\x19\x61\x82\x6E\xBB\x9B\x48\xD3\x3D",
                       "\x42\x91\x74\xC5\x7F\xAB\x0A\x9C\xDD\xD4\xE7\x48\xE3\x37\x11\xA9\x8A\x66\x0C\x49\x77\xFF\xC2\x3F\xC5\x0A\xB0\x92\xD8\x17\x46\xAD");
    
    
    printf("SMSM2Load[%d]\r\n",iret);
    //iret = SMSM2InputVerify(32,w"\xB1\xD5\xB8\x72\xCC\x3C\x37\x5F\x9E\xAD\xFB\x06\x67\x03\xA8\x0F\x58\x2C\x0E\xA3\xDF\x9F\x6B\xD1\x08\xBC\x14\x58\xF9\x76\xEB\x23");
    //printf("SMSM2InputVerify[%d]\r\n",iret);
    
    //iret = SMSM2VerifyNoE("\xED\x17\x9B\x31\x65\x04\xD1\xDF\xB4\xEC\xDF\x4C\xB7\xDC\x0C\x20\xDD\x75\x66\x5A\xFE\x4A\x9B\x6A\xB6\x08\x1C\x1E\x27\xD3\x29\x2E",
    //                      "\x67\x02\x64\xB9\x02\xF7\x6C\x67\xEC\x02\xB9\x1E\x39\xCE\xA0\x8D\xA7\xBF\x7E\xAC\x80\xEB\xF2\xFA\xDE\xE1\xC5\xFF\x64\x45\xA8\xDF");
    iret = SMSM2Verify("\xDC\xF5\x5F\x9B\x30\xED\xC1\x30\x79\x60\x6D\x62\x80\xB7\x52\x58\x7B\xDB\xA6\x2F\x47\x1A\x3D\xA7\x7A\xA3\x49\x73\xAA\x3E\x47\x14",
                       "\x9F\x8E\x28\xEE\x47\x05\x7B\x7B\xD0\x01\xB6\x11\x39\x1D\x80\xDF\x5D\xA7\x20\x3F\x3D\x30\x0E\xEC\xFB\x32\x43\xC4\x49\x8B\xA1\x0C",
                       "\xFE\x3F\xB9\x1A\xB1\x5A\xA0\x37\x62\x10\xA3\xF6\x75\x11\x93\x0C\x2E\xF0\x32\x97\x01\x71\x67\xB5\x2A\xE5\x06\x03\xCA\x34\xDF\xED");
    return iret;
}

int SMSM3SelfTest(void)
{
  unsigned char sOutput[32];
  sm3("\x55",1,sOutput);
  if(memcmp(sOutput,"\x65\x7A\x94\xD4\xF1\x56\x95\x63\x30\x05\xEC\x90\x9A\xC5\x65\x51\x11\x59\x9B\xCD\x82\x78\xB7\x5D\x18\x3B\x86\xFE\xEB\x77\x8B\xFF",32))
    return 1;
  return 0;
}

int SMSM4SelfTest(void)
{
  unsigned char sKey[16];
  unsigned char sInput[16];
  unsigned char sOutput[16],sBuf[100];
  memcpy(sKey,"\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88",16);
  memcpy(sInput,"\xAA\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88",16);
  memcpy(sOutput,"\x95\x0F\xA2\xC8\x8C\x42\xAB\x1C\xE0\xD6\xFC\x25\xC1\xB5\x48\x51",16);
  Lib_sm4(sInput,16,sBuf,sKey,1); 
  if(memcmp(sBuf,sOutput,16))
    return 1;
  Lib_sm4(sOutput,16,sBuf,sKey,0); 
  if(memcmp(sBuf,sInput,16))
    return 1;
  return 0;
}


