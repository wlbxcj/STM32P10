/*
* spi driver 
*create by skx on 2010-07-02
*
*/
#include "mifre_tmdef.h"
//#include "MAXQ1850_isr.h"
#include <string.h>

#define RF_SPI_SEND   1
#define RF_SPI_RECV   0


unsigned char RF_pucSendBuf[10];


static volatile unsigned char  RF_ucRegAddr = 0;


unsigned char RF_pucRecvBuf[10];


static volatile unsigned int  RF_ulBufCur;


static volatile unsigned int  RF_ulBufLen;    


static volatile unsigned char RF_ulSPIEnd;


static volatile unsigned char RF_ulSPIMode; 

void s_InitRFSPI(unsigned char spibaudckr)
{
    #if (TERM_TYPE == TERM_KF322)
    //form RF1356.c
    extern void SPI1_Init(void);
    SPI1_Init();
    #endif
    
    #if (TERM_TYPE == TERM_KF311)
    SPICN =0x0;//CLEAR ALL FLAG AND MDD FAILE DETECT DISABLE
    PD1 |=(1<<SPICS);//SET SSEL IS OUTPUT
    PO1 |=(1<<SPICS);//SET SSEL IS HIGH
    SPICN |=0X03;//set spi is master and enable spi and baud rate generator
    SPICF=0X0;//SPI INTERRUPT DISABLE,CHAR LEN IS 8 BIT,POL LOW, CKPHA IS RISING SAMPLED
    SPICK=spibaudckr;//set spi baud rate ,compute:baud rate=(0.5*sysclk)/(spibaudckr+1)
    #endif
}


void s_InitRFGPIO(void)
{
	#if (TERM_TYPE == TERM_KF311)
	  __disable_interrupt();
	  
	  ctl_set_isr(MAX1850_EXT_INT_01_VECTOR, INT_PRI_HIGH, MAXQ1850_ISR_TRIG_NEG_EDGE, s_RFIsr, NULL);
	  PD0 &=~(RFINT);//SET RFINT PIN  IS INPUT
	  EIE &=~(RFINT);//disable RFINT PIN INTERRUPT
	  EIES|=(RFINT);//SET RFINT PIN IS FALLING
	  EIF &=~(RFINT);//CLEAR RF INT FLAG
	  __enable_interrupt();
 	#endif
 	
 	#if (TERM_TYPE == TERM_KF322)
	//form RF1356.c
	extern void RFinterrupt_Init(void);
	RFinterrupt_Init();
#endif
  
} 


void s_vRFReset(void)
{	
	//control by cpu
	#if (TERM_TYPE == TERM_KF322)

	//do nothing,beause reset have do by cpu
	
	#endif

}

void s_vRFSwitchNCS(unsigned char chHigh)
{
	#if (TERM_TYPE == TERM_KF322)
	//form RF1356.c
	extern void PN512_RFSwitchNCS(char chHigh);
	PN512_RFSwitchNCS(chHigh);
	#endif

	#if (TERM_TYPE == TERM_KF311)
	  // ncs 
	  PD1 |=(1<<SPICS);//SET SSEL IS OUTPUT
	  
	  if(chHigh)
	  {
	    PO1 |=(1<<SPICS);//SET SSEL IS HIGH
	  }
	  else 
	  {
	    PO1 &=~(1<<SPICS);//SET SSEL IS low
	  }
  	#endif

}

void s_RF_SPIProc(void)
{
	#if (TERM_TYPE == TERM_KF311)
  unsigned char Rec_First=0;
  signed int i=0;
  
  while(SPICN & 0x80) ;// Wait until SPI busy flag is cleared by .
  
  if (RF_ulSPIMode == RF_SPI_SEND)
  {
  		//printf("in  1\n");
      SPIB = RF_ucRegAddr;
  }
  else
  {
  		//printf("in  2\n");
      SPIB = RF_ucRegAddr|(unsigned char)0x80;
  }
  
  while(!(SPICN & 0x40)) ;// Wait for the transfer to complete.
  SPICN &=~(1<<6);//clear transfer complete flag by software must

  Rec_First=SPIB & (0x0ff);//First byte is ignore
  
  
  while (RF_ulSPIEnd)
  {
      if (RF_ulSPIMode == RF_SPI_SEND)
      {
        if (RF_ulBufCur == RF_ulBufLen)
        {
          s_vRFSwitchNCS(1);
          RF_ulSPIEnd = 0;
        }
        else
        {
          SPIB = RF_pucSendBuf[RF_ulBufCur];
          RF_ulBufCur++;

          while(!(SPICN & 0x40)) ;// Wait for the transfer to complete.
          SPICN &=~(1<<6);//clear transfer complete flag by software must
          Rec_First=SPIB;//write mode ignore read byte
        }
      }
      else//read
      {
      	//printf("in read 1\n");
        if (RF_ulBufCur > 0)
        {
          RF_pucRecvBuf[RF_ulBufCur-1] = SPIB;
          if (RF_ulBufCur == RF_ulBufLen)
          {
          	//printf("in read 4\n");
            s_vRFSwitchNCS(1);
            RF_ulSPIEnd = 0;
          }
        }
      
        if (RF_ulBufCur < RF_ulBufLen)
        {
          if (RF_ulBufCur == (RF_ulBufLen - 1))
          {
          	//printf("in read 3\n");
            SPIB = 0;
            while(!(SPICN & 0x40));// Wait for the transfer to complete.
            SPICN &=~(1<<6);//clear transfer complete flag by software must
          }
          else
          {
          	//printf("in read 2\n");
            SPIB = RF_ucRegAddr;
            while(!(SPICN & 0x40));// Wait for the transfer to complete.
            SPICN &=~(1<<6);//clear transfer complete flag by software must
          }
          RF_ulBufCur++;
        }
     }
  }
  SPICN &= ~( (1<<3)|(1<<4)|(1<<5)|(1<<6) );//maual clear rec over,write coll,mode fault,trans complete
  #endif
}


signed int s_vRFReadReg(unsigned int ulLen, unsigned char ucRegAddr, unsigned char* pucData)
{ 
	#if (TERM_TYPE == TERM_KF311)
    if (ulLen == 0)
    return 0xFF;
    s_vRFSwitchNCS(0);
    RF_ulSPIMode = RF_SPI_RECV;
    memset(RF_pucRecvBuf, 0, sizeof(RF_pucRecvBuf));
    RF_ucRegAddr = (ucRegAddr<<1)&(unsigned char)0x7E;
    
    RF_ulBufCur = 0;
    RF_ulBufLen = ulLen;
    RF_ulSPIEnd = 1;       
    
    s_RF_SPIProc();  
    memcpy(pucData, RF_pucRecvBuf, ulLen);
    return 0;
    #endif
    
    #if (TERM_TYPE == TERM_KF322)
    extern uchar PN512GetReg(uchar  RegAddr);
    if (ulLen != 1)
		return 0xFF;
    *pucData = PN512GetReg(ucRegAddr);
    return 0;
    #endif
}


signed int s_vRFWriteReg(unsigned int ulLen, unsigned char ucRegAddr, unsigned char* pucData)
{ 
	#if (TERM_TYPE == TERM_KF311)
    if (ulLen == 0)
    return 0xFF; 
    s_vRFSwitchNCS(0);
    RF_ulSPIMode = RF_SPI_SEND; 
    memset(RF_pucSendBuf, 0, sizeof(RF_pucSendBuf));
    memcpy(RF_pucSendBuf,pucData, ulLen);
    RF_ulBufCur = 0;
    RF_ulBufLen = ulLen;
    RF_ulSPIEnd = 1;
    RF_ucRegAddr = (ucRegAddr<<1)&(unsigned char)0x7E;

    s_RF_SPIProc();
    
    return 0;
    #endif
    
    #if (TERM_TYPE == TERM_KF322)
    extern void PN512SetReg(uchar RegAddr,uchar RegValue);
    if (ulLen != 1)
		return 0xFF;
    PN512SetReg(ucRegAddr,*pucData);
    return 0;
    #endif
}



uchar sPcdInit(void)
{
    uchar ret=0;

    ret=s_RF_Init();

    sHal_Module_Disable();

    return ret;
}

extern PICC_PARA c_para;
extern RF_WORKSTRUCT RF_WorkInfo;
extern const uchar PCD_VER[5];
extern const uchar PCD_DATE[12];
int s_RF_Init(void)
{

      unsigned long i = 0;
      uchar ucReadVal = 0;
      uchar ucWriteVal = 0;
      uchar card_param[8];
      int paramlen = 0;
      
      s_InitRFSPI(SPIBAUDCKR);
      s_InitRFGPIO();
      s_vRFReset();

	ucReadVal = 0x00;
	
	s_vRFWriteReg(1,RF_REG_PAGE,&ucReadVal);
        
	ucWriteVal=0x13;
	
	s_vRFWriteReg(1, RF_REG_MODWIDTH, &ucWriteVal);
        s_vRFReadReg(1, RF_REG_MODWIDTH, &ucReadVal);
    
	//s_vRFReadReg(1,RF_REG_PRIMARYSTATUS,&ucReadVal);
	if( ucReadVal != ucWriteVal )
	{       
		//debug_printf(0,0,0,"rc531 spi comm err!%02x\n",ucReadVal);
		return -3;
	}
        
	//clear int 
	ucReadVal = 0x3F;
	s_vRFWriteReg(1,RF_REG_INTERRUPTEN,&ucReadVal); 
	s_vRFWriteReg(1,RF_REG_INTERRUPTRQ,&ucReadVal);
	
	//config irq pin
	//ucReadVal = 0x03;//config irq pin is inverted with respect to bit IRq and a standard CMOS output
	//s_vRFWriteReg(1,RF_REG_IRQPINCONFIG,&ucReadVal); 

	//init struct
	memset((uchar *)&RF_WorkInfo, 0x00, sizeof(RF_WORKSTRUCT));
	memset(&c_para,0x00,sizeof(c_para));
	memset(card_param,0,sizeof(card_param));
	
	c_para.a_conduct_val = 0x3f;
	c_para.b_modulate_val = 0x0a;//0x0c;//0x08;
	c_para.card_RxThreshold_val=0x70;
	c_para.card_buffer_val = 32;	
	strcpy((char *)c_para.drv_ver, (char *)PCD_VER);
	strcpy((char *)c_para.drv_date,(char *)PCD_DATE);
	
	return 0;	

}


uchar sHal_Module_Disable(void)
{
	sHal_MaskCpuInt();//disable int and clear
	s_vRFSwitchNCS(1);//CS H

	// NRSTPWD HIGH
	//contorl by cpu
	return 0;
}


void sHal_EnCpuInt(void)//enable CPU int
{
	#if (TERM_TYPE == TERM_KF322)
	// Enable the Interrupt
	//from stm3210x_exti.c
	extern void EXTI_EnableITBit(unsigned int EXTI_Line);
	EXTI_EnableITBit(RFINT);	
	#endif
	
	#if (TERM_TYPE == TERM_KF311)
	//IC|=(1<<0);
	EIE |=(RFINT);//ENABLE RFINT PIN INTERRUPT
	#endif
	//enable rf int
}


void sHal_MaskCpuInt(void)//disable CPU int
{
	#if (TERM_TYPE == TERM_KF311)
	EIE &=~(RFINT);//DISABLE RFINT PIN INTERRUPT
	EIF &=~(RFINT);//CLEAR RF INT FLAG
	#endif
	
	#if (TERM_TYPE == TERM_KF322)
	// Enable the Interrupt
	//from stm3210x_exti.c
        extern void EXTI_DisableITBit(unsigned int EXTI_Line);
        extern void EXTI_ClearITPendingBit(unsigned int EXTI_Line);
        //EXTI_DisableITBit(EXTI_INTERRUPT_LINE);
        EXTI_ClearITPendingBit(RFINT);
	#endif
}
