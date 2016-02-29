/*********************************************************************************** 
Copyright (C) 2011 Vanstone Technology Co., LTD.                         

System Name		:  vos                                            
Module Name    	:  PCD myfare and contactless driver                            
File   Name   		:  picc.c                                                  
Revision    		:  01.00                                                     
Date     		      :  2011/10/25           
Dir       		      :  drv\picc
author			: 
error code  		: -3500~-3999    
                                                                      
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "picc.h"
#include "picc_stm32f.h"

#include "base.h"
#include "vosapi.h"  

#if 0
#include "exbus.h"
#include "usip.h"
#include <infra/error.h>
#include <hal\usip\hal_spi.h>
#include <hal\usip\hal_intr.h>
#endif

#define PICC_DEBUG
//#undef PICC_DEBUG
#ifdef PICC_DEBUG
//#define dbg_printf(args...) 	s_UartPrint(COM_DEBUG, ##args)
#define dbg_printf   trace_debug_printf
#else
#define dbg_printf(args...)
#endif 
//#define err_printf(args...)		s_UartPrint(COM_DEBUG, ##args)
#define err_printf		trace_debug_printf

//#define info_printf(args...) 	s_UartPrint(COM_DEBUG, ##args)
#define info_printf 	trace_debug_printf

#define DelayMs 	Lib_DelayMs
#define PICC_TIMER         4 

static PICC_INFO picc_info[15];
static PCD_INFO  pcd_info;


const  ushort tab_frame_size[9]={16,24,32,40,48,64,96,128,256};
const  uchar  tab_cycle_count[15][2]=
          {{0x00,0x03},{0x00,0x07},{0x00,0x0f},{0x00,0x1f},{0x00,0x3f},{0x00,0x7f},{0x00,0xff},{0x01,0xff},
          {0x03,0xff},{0x07,0xff},{0x0f,0xff},{0x1f,0xff},{0x3f,0xff},{0x7f,0xff},{0xff,0xff}};
//0-0.302ms-3, 1-0.604ms-7, 2-1.208ms-15,3-2.416ms-31,4-4.833ms-63
//5-9.666ms-127, 6-19.33ms-255, 7-38.66ms-,8-77.32ms,9-154.6ms
//10-309.3ms,11-618.6ms,12-1.237s,13-2.474s,14-4.949s

const uchar tab_max_wtx[15]={59,59,59,59,59,59,59,59,59,32,16,8,4,2,1};


static volatile ushort g_PiccWtxCount; 


uchar WTXM=1;

uchar g_TA430_FLAG; //?

//---------------------------------------------
#define HAVE_CARD			1
#define NO_CARD				0 
#define CHECK_ONE_CARD		0
#define CHECK_A_CARD		1
#define CHECK_B_CARD		2
#define CHECK_M_CARD		3
static volatile int g_PiccCheckCardMode;
static volatile int g_HaveACardFlag;
static volatile int g_HaveBCardFlag;
static volatile int g_WakeUpFlag;
static volatile int g_WakeUpAFlag=0;	//polling or remove.
static volatile int g_WakeUpBFlag=0;	//polling or remove.
static volatile int g_ActiveBFlag=0;	//polling or remove.
static volatile int g_ActiveFlag=0;	//polling or remove.
static volatile int g_CommandFlag=0;	//polling or remove.
static volatile int picc_TimeoutFlag=0;	//polling or remove.
static volatile int PICC_AntiColl_B_Flag=0;	//polling or remove.

static volatile unsigned char  Card_Type_ABM=0;	//polling or remove.

uchar OPENED_FLAG=0;
uchar HAVECARD_FLAG=0;


typedef struct tagRc523RegsSet
{
	uchar VAx_reg_CWGsN_value;
	uchar VAx_reg_ModGsN_value;
	uchar VAx_reg_CWGsP_value;
	uchar VAx_reg_ModGsP_value;
	uchar VAx_reg_RxThreshold_MinLevel_value;
	uchar VAx_reg_RxThreshold_CollLevel_value;
	uchar VAx_reg_RFCfgReg_RxGain_value;
	uchar VAx_reg_DemodReg_value;
	uchar VAx_reg_CWGsN_value_B;
	uchar VAx_reg_ModGsN_value_B;
	uchar VAx_reg_CWGsP_value_B;
	uchar VAx_reg_ModGsP_value_B;
	uchar VAx_reg_RxThreshold_MinLevel_value_B;
	uchar VAx_reg_RxThreshold_CollLevel_value_B;
	uchar VAx_reg_RFCfgReg_RxGain_value_B;
	uchar VAx_reg_DemodReg_value_B;
}RegPara;
 
RegPara g_tReg;
void VAx_recall_reg_xx();
void VAx_save_reg_xx();
//---------------------------------------------- 
int  s_PiccCmdExchange(uchar action,uchar wait_mode,uchar *blk,ushort *tx_rx_len);
void s_PiccHalt(void);
void s_PiccDeselect(void); 
void Lib_PiccHalt(void);
extern int USIP_apm_reset_device( unsigned int devnum );
extern int USIP_apm_set_device_mode( unsigned int devnum, unsigned int mode );
void PICC_RESET_HW(void);
#if 0
#define spi_cs_hi() 	Mips_clr16bit(USIP_SPI_CR1, USIP_SPI_SS3)
#define spi_cs_low() 	Mips_set16bit(USIP_SPI_CR1, USIP_SPI_SS3)

inline static void spi_write(char data)
{
	Mips_write16(USIP_SPI_DR, data);
	Mips_set16bit(USIP_SPI_CR1,USIP_SPI_START); 
	while(Mips_read16(USIP_SPI_SR) & USIP_SPI_BUSY){};

	return;

}

inline static char spi_read(void)
{
	uchar data=0;
	
	while((Mips_read16(USIP_SPI_SR) & USIP_SPI_FFREPY) != 0){};
	data = Mips_read16(USIP_SPI_DR);

	return data;
}



static void s_WriteRegRc523(uchar addr,uchar value)
{

	uchar data[2]={0};

	data[0] = ((addr<<1)&0x7E);
	data[1] = value;
	spi_cs_low();
	spi_write(data[0]);
	spi_write(data[1]);
	spi_cs_hi();

	dbg_printf(" [W%02x]%02x ", addr, value);

	return;
}

static uchar s_ReadRegRc523(uchar addr)
{
	uchar data[2]={0};

	data[0] = ((addr<<1)&0x7E) | 0x80;
	data[1] = 0;

	spi_cs_low();
	spi_write(data[0]);
	spi_write(data[1]);
	data[1] = spi_read();
	spi_cs_hi();

	dbg_printf(" [R%02x]%02x ", addr, data[1]);

	return data[1];
} 

int s_PiccSpiInit(void)
{
	Mips_write16(USIP_SPI_CR0,0);
	//Stop all operations and deselect all device
	Mips_write16(USIP_SPI_CR1,0x0);
	// Clears all interrupts without distinction
	Mips_write16(USIP_SPI_ISR,0); 
	//Set the baud rate divisor
	Mips_write16(USIP_SPI_ISR,0); 
	Mips_write16(USIP_SPI_IER,0); 


	Mips_write16(USIP_SPI_BRR, 0x01);
	// Enable SPI module and set the configuration  
	Mips_set16bit(USIP_SPI_CR0, USIP_SPI_EN | USIP_SPI_MSTR |USIP_SPI_SIZE8 | 
					USIP_SPI_CPOL | USIP_SPI_CPHA | USIP_SPI_FULLD ); 
	//s_DelayMs1(1);  
	Mips_set16bit(USIP_SPI_CR1, USIP_SPI_RSTTXFIFO); 

	printf_spi_regs();

	
	return 0;
}

#endif

inline static void printf_spi_regs(void)
{
	dbg_printf("*****SPI REGS*****\r\n");
#if 0        
	dbg_printf("CR0:%08x\r\n", Mips_read16(USIP_SPI_CR0));
	dbg_printf("CR1:%08x\r\n", Mips_read16(USIP_SPI_CR1));
	dbg_printf("BRR:%08x\r\n", Mips_read16(USIP_SPI_BRR));
	dbg_printf("IER:%08x\r\n", Mips_read16(USIP_SPI_IER));
	dbg_printf("ISR:%08x\r\n", Mips_read16(USIP_SPI_ISR));
	dbg_printf("SR:%08x\r\n",  Mips_read16(USIP_SPI_SR));
#endif        
	dbg_printf("******************\r\n");

	return;
}

inline static ushort s_spi_is_busy(void)
{	
	//return (Mips_read16(USIP_SPI_SR) & HAL_SPI_STATE_BUSY);
        //return SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY);
        return 0;
}

inline static int s_spi_reset_fifo(void)
{
	int ret;
#if 0	
	ret = hal_spi_ioctl(HAL_SPI_IOCTL_RESET_RX_FIFO , 0);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_ioctl RESET_RX err. ret:%d\r\n", ret);
		return ret;
	}
	ret = hal_spi_ioctl(HAL_SPI_IOCTL_RESET_TX_FIFO , 0);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_ioctl RESET_TX err. ret:%d\r\n", ret);
		return ret;
	}

	return ERR_NO;
#endif
        while ((SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == SET)){
            SPI_ReceiveData(SPI1);
        }
        
        return 0;        
}

#if 0
inline static int s_spi_wirte(uchar *pdata, uint *len)
{
	int ret;
	uint status;
	
	ret = hal_spi_write(pdata, len);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_write err. ret:%d\r\n", ret);
		return ret;
	}
	while (1) {
		ret = hal_spi_get_status(&status);
		if (ret != -ERR_NO) {
			err_printf("[picc]hal_spi_get_status err. ret:%d\r\n", ret);
			return ret;
		}
		if ((status & HAL_SPI_STATE_FFTEPY) 
			&& ((status & HAL_SPI_STATE_BUSY) == 0))
			break;
	}

	return ERR_NO;

}

inline static int s_spi_read(uchar *pdata, uint *len)
{
	int ret;
	uint status;
	
	while (1) {
		ret = hal_spi_get_status(&status);
		if (ret != -ERR_NO) {
			err_printf("[picc]hal_spi_get_status err. ret:%d\r\n", ret);
			return ret;
		}
		if ((status & HAL_SPI_STATE_FFREPY) == 0)
			break;
	}

	ret = hal_spi_read(pdata, len);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_read err. ret:%d\r\n", ret);
		return ret;
	}

	return ERR_NO;
}
#endif

#if 1
static void s_WriteRegRc523(uchar addr,uchar value)
{

	uchar data[2]={0};
	int len;
	int ret;

	while (s_spi_is_busy()){};
	s_spi_reset_fifo();

	//hal_spi_ioctl(HAL_SPI_IOCTL_SELECT_DEVICE, HAL_SPI_CS_THREE);
        SPI1_NSS_LOW();
#if 0        
	data[0] = ((addr<<1)&0x7E);
	data[1] = value;
	len = 2;
	s_spi_wirte(data, &len);
#endif        
        SPIDataSendReceive((addr <<1)&0x7F);
        SPIDataSendReceive(value);
        
	//hal_spi_ioctl(HAL_SPI_IOCTL_UNSELECT_DEVICE, HAL_SPI_CS_THREE);
        SPI1_NSS_HIGH();
        
	//dbg_printf(" [W%02x]%02x ", addr, value);

	return;
}

static uchar s_ReadRegRc523(uchar addr)
{
	uchar data[2]={0};
	int len;
	int ret;

	while (s_spi_is_busy()){};
	s_spi_reset_fifo();

	//hal_spi_ioctl(HAL_SPI_IOCTL_SELECT_DEVICE, HAL_SPI_CS_THREE);
        SPI1_NSS_LOW();
#if 0        
	data[0] = ((addr<<1)&0x7E) | 0x80;
	data[1] = 0;
	len = 2;
	s_spi_wirte(data, &len);
#endif        
        SPIDataSendReceive((addr <<1)|0x80);
#if 0        
	data[0] = data[1] =0;
	len = 2;
	s_spi_read(data, &len);
#endif        
        data[1] = SPIDataSendReceive(0x00);
        
	//hal_spi_ioctl(HAL_SPI_IOCTL_UNSELECT_DEVICE, HAL_SPI_CS_THREE);
        SPI1_NSS_HIGH();
        
	//dbg_printf(" [R%02x]%02x ", addr, data[1]);

	return data[1];
} 

void s_SetBitRegRc523(unsigned char regaddr,unsigned char bitmask)
{
   uchar tmp = 0x0;
   tmp = s_ReadRegRc523(regaddr);
   s_WriteRegRc523(regaddr,tmp | bitmask);   
} 
 

void s_ClrBitRegRc523(unsigned char regaddr,unsigned char bitmask)
{
   uchar tmp = 0x0;
   tmp=s_ReadRegRc523(regaddr);
   s_WriteRegRc523(regaddr,tmp & ~bitmask); 
} 


void s_PiccIntProc(void)
{  
}



#define PICC_SPI_BAUDRATE	1500000
int s_PiccSpiInit(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  //Enable SPI1 clock
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  //Configure SPI1 pins: NSS, SCK, MISO and MOSI
  /*
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //Configure PA4 as Output push-pull, used as Flash Chip select
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //SPI1 Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI1_NSS_HIGH();  
  //SPI1 enable
  SPI_Cmd(SPI1, ENABLE);
  return 0;  
#if 0  
	int ret;
	HAL_SPI_CONFIG spi_cfg;

	USIP_apm_reset_device(USIP_APM_SPI);
    USIP_apm_set_device_mode(USIP_APM_SPI, USIP_APM_DEVICE_MODE_ACTIVE);
    hal_interrupt_mask(HAL_INTERRUPT_SPI);

	spi_cfg.baudrate = PICC_SPI_BAUDRATE;
	spi_cfg.fiforxth = 1;
	spi_cfg.wordsize = HAL_SPI_WORD_SIZE_8;
	spi_cfg.exflags = HAL_SPI_FULL_MODE;
	spi_cfg.mode = HAL_SPI_MODE_MASTER;
	ret = hal_spi_init(&spi_cfg);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_init err. ret:%d\r\n", ret);
		return ret;
	}
	ret = hal_spi_ioctl(HAL_SPI_IOCTL_ENABLE, 0);
	if (ret != -ERR_NO) {
		err_printf("[picc]hal_spi_ioctl ENABLE err. ret:%d\r\n", ret);
		return ret;
	}

	printf_spi_regs();

	info_printf("[picc]Spi initial......done.\r\n");
	return ERR_NO;
#endif
        
}
#endif


int s_PiccCheckRC523Exist()
{	
	uchar uVersion;

	uVersion=s_ReadRegRc523(rVersionReg);
	//RETAILMSG(1,(TEXT("%s %d %x \n"),TEXT(__FUNCTION__),__LINE__, ret));

	uVersion=s_ReadRegRc523(rVersionReg);
	//RETAILMSG(1,(TEXT("%s %d %x \n"),TEXT(__FUNCTION__),__LINE__, ret));

	uVersion=s_ReadRegRc523(rVersionReg);
	//RETAILMSG(1,(TEXT("%s %d %x \n"),TEXT(__FUNCTION__),__LINE__, ret));


	if ((uVersion== 0xB1) || (uVersion== 0xB2))
	{
 
		trace_debug_printf("[picc] Read Rc523 uVersion=%02X\r\n",uVersion);
		return 0;
	}

	trace_debug_printf("[picc] Read Rc523 uVersion error(%02x) \r\n",uVersion);
	return -1;
}


// NRSTPWD: NOT REST AND POWER DOWN
// LOW: hard power down; low->high: reset
void PN512_s_vRFReset1(void)
{	


//do nothing,beause reset have do by cpu
    GPIO_InitTypeDef GPIO_InitStructure;

    //GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOA,RFRESET);
   
    DelayMs(2);
    GPIO_ResetBits(GPIOA,RFRESET);
    DelayMs(3);
    GPIO_SetBits(GPIOA,RFRESET);
    DelayMs(2);
    


}

int s_PiccInitFirst()
{
#if 0  //old  
        unsigned int i = 0;
	uchar ucReadVal = 0;
	uchar ucWriteVal = 0;
  
	for(i=0;i<0x4000;i++)
	{
		PN512_s_vRFReadReg(1,0x01,&ucReadVal);  
		if(ucReadVal==0x20)break;
	}
	if(i == 0x4000)
	    return -1;
	
	    
	ucReadVal = 0x00;
  	PN512_s_vRFWriteReg(1,PN512_PAGE_REG,&ucReadVal); //SPI模式,按照平板访问寄存器

	PN512_s_vRFReadReg(1,PN512_TXSEL_REG,&ucReadVal);  
	ucWriteVal = ucReadVal+0x01;
	PN512_s_vRFWriteReg(1,PN512_TXSEL_REG,&ucWriteVal); 
	PN512_s_vRFReadReg(1,PN512_TXSEL_REG,&ucReadVal);  
	if(ucReadVal != ucWriteVal)
	{
		info_printf("SPI ERR_1,%02x/%02x\n",ucWriteVal,ucReadVal);
		return -2;
	}
		
	/*
	ucReadVal=0xff;   
	PN512_s_vRFReadReg(1,PN512_DIVIEN_REG,&ucReadVal);//读取0x03寄存器值，用以区分RC531和PN512
	if( ucReadVal )
	{
		printk("SPI ERR_2,%02x\n",ucReadVal);
		return -3;
	}
	*/
	
	ucReadVal = 0x10;
	PN512_s_vRFWriteReg(1,PN512_TXSEL_REG,&ucReadVal); // Modulation signal (envelope) from the internal coder

	ucReadVal = 0x80;
	PN512_s_vRFWriteReg(1, PN512_COMMIEN_REG,&ucReadVal); // Irq Inv		
	PN512_s_vRFWriteReg(1, PN512_DIVIEN_REG,&ucReadVal);  // close pcd interrupts 2,IRQPushPull=1

	
	PN512_s_vRFReadReg(1,PN512_CONTROL_REG,&ucReadVal);
	ucReadVal |= 0x10;
	PN512_s_vRFWriteReg(1,PN512_CONTROL_REG,&ucReadVal);//配置PN512为主动发起命令者
#endif

	return 0;	  
  
}

/*
Resource:
SPI - Slave 2
RF_RESET  Ex_CS2 Bit1

*/
int s_PiccInit(void)
{ 


	DWORD  spcr1;
	int ret;

	ret = s_PiccSpiInit();
	if (ret != ERR_NO) {
		return ret;
	}
        
        //RFinterrupt_Init();//现不用中断 gplian 13/01/08
        
        PN512_s_vRFReset1();
#if 0        
	ret = exbus2_init();
 	if (ret != ERR_NO) {
		return ret;
	}
	exbus2_rf_reset();
#endif        
	memset(&picc_info,0x00,sizeof(picc_info));  
	memset(&pcd_info,0x00,sizeof(pcd_info));   
	g_PiccWtxCount=0;    

	picc_info[0].SFGT_need=0;
#if 0	
	PICC_RESET_HW();
#endif
	s_PiccCheckRC523Exist();

	info_printf("[picc]Initial......done.\r\n");
#if 0        
	VAx_recall_reg_xx();
#endif
        s_PiccInitFirst(); //gplian
	return 0;
}

#if 0
int Picc_Open_old(void)
{ 
	uchar tmpc;
	uchar tmps[PICC_RXPOOL_SIZE];
	ushort tn,i; 
	int iret;

	memset(&picc_info,0x00,sizeof(picc_info)); 
	memset(&pcd_info,0x00,sizeof(pcd_info)); 
	 

	//reset PCD
	OkiCLib_set8bit(PO6, RF_RST);
	DelayMs(2);
	OkiCLib_clr8bit(PO6, RF_RST);
	DelayMs(25);
	OkiCLib_set8bit(PO6, RF_RST);
	OkiCLib_clr8bit(PO3, RF_CS);
	DelayMs(5);
	

    s_WriteRegRc523(rCommandReg,PCD_SOFTRESET);//reset the RC523
    DelayMs(100);
    
   
	 
	//disable Crypto1 bit  
	s_ClrBitRegRc523(rStatus2Reg, BIT_MFCRYPTO1ON);   

	//s_SetTimer(PICC_TIMER, 200);
	//while(1)
	//{
	//      tmpc=s_ReadRegRc523(rCommandReg);       
	//      if(!(tmpc&0x3f))break;
	//      printf("rcommand2=%02x \r\n",tmpc);
	//      if(!s_CheckTimer(PICC_TIMER)) return PICC_OpenErr;
	//}  
	tmpc=s_ReadRegRc523(rVersionReg);   //
	printf("rversion(0XB1)=%02x \r\n",tmpc); 

	//s_WriteRegRc523(rComIEnReg,0x7f);//close all pcd interrupts
	s_WriteRegRc523(rComIEnReg,0x80);//close all pcd interrupts
	s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
	//s_WriteRegRc523(rDivIEnReg,0x14);//close all pcd interrupts
	s_WriteRegRc523(rDivIEnReg,0x00);//close all pcd interrupts
	s_WriteRegRc523(rDivIrqReg,0x14);//clear all pcd interrupts 

	g_PiccWtxCount=0;  

	tmpc=s_ReadRegRc523(rStatus1Reg);   
	if(tmpc!=0x21)
	{
		printf("rcommand7=%02x \r\n",tmpc);
		//return PICC_OpenErr;
	}

	tmpc=s_ReadRegRc523(rStatus2Reg); 
	if(tmpc!=0x00)
	{
		printf("rcommand8=%02x \r\n",tmpc);
		//return PICC_OpenErr;
	}    

	DelayUs(100); 
	//s_WriteRegRc523(rClockQControl,0x0);
	//s_WriteRegRc523(rClockQControl,0x40);
	s_WriteRegRc523(rDemodReg,0x6D); //6e
	DelayUs(100);
	//tmpc = s_ReadRegRc523(rClockQControl);
	//s_WriteRegRc523(rClockQControl,tmpc & 0xbf); 

	pcd_info.open=1; 
	DelayMs(10);  

	/*
	while (1)
	{
		s_ReadRegRc523(rStatus1Reg);
		s_WriteRegRc523(rDemodReg,0x6D); //6e
		tmpc=s_ReadRegRc523(rVersionReg);   //
		if(tmpc!=0xb1)
			printf("rversion(0XB1)=%02x \r\n",tmpc); 
		s_WriteRegRc523(rDemodReg,0x6D); //6e
		tmpc=s_ReadRegRc523(rDemodReg);   //
		if(tmpc!=0x6d)
			printf("xxxxx=%02x \r\n",tmpc); 
	}
	*/
	

	return 0;   
}



int Picc_Close_old(void)
{ 
	pcd_info.open=0;

	OkiCLib_clr8bit(PO6, RF_RST);   
 
    return 0;
}
#endif

#define FDT_TYPE_0 0	///n=9	TYPE A (WUPA ANTICOLLISION SELECT)
#define FDT_TYPE_1 1///	//n>=9	TYPE A(OTHERS)
#define FDT_TYPE_2 2///	//	TYPE A (RATS)
#define FDT_TYPE_3 3///	//	TYPE B
#define FDT_TYPE_4 4///	//	TYPE B (WUPB)
static volatile uchar g_FDT_Flag=0;



uchar g_Add = 0;
void set_picc(uchar mode)
{     
	if(mode=='A'||mode=='M')
	{
		//s_WriteRegRc523(rTxControl,0x5b);            //100%ASK,modulated TX 
		s_WriteRegRc523(rTxControlReg,0x83);
		//invTx2RFON  invTx1RFON invTX2RFOff invTx1RFoff TX2CW  RFU  TX2RFEN  TX1RFEN
		s_WriteRegRc523(rTxASKReg,0x40);
		//RFU Force100ASK RFU RFU RFU RFU RFU RFU 
		s_WriteRegRc523(rTxSelReg,0x10);
		//Driversel(B6B5)=01  

		//s_WriteRegRc523(rCwConductance, 0x3f);       //default 0x3f,for operating distance adjust        
		s_WriteRegRc523(rGsNReg, g_tReg.VAx_reg_CWGsN_value<<4 | g_tReg.VAx_reg_ModGsN_value);//0x0f<<4,0x06


		s_WriteRegRc523(rCWGsPReg, g_tReg.VAx_reg_CWGsP_value);
		//s_WriteRegRc523(rModGsPReg, g_tReg.VAx_reg_ModGsP_value); //added by ding

		//s_WriteRegRc523(rCoderControl,0x19);         //coder mode:ISO14443 type A  
		s_WriteRegRc523(rTxModeReg,0x00);
		//TXCRCEN(B7)=0 TXSPEED(B6B5B4)=000: 106k  INVMOD(B3)=0 TXFRAMING(B1B0)=00  ISO14443A


		//--WUPA command is sent as a short frame 
		//s_WriteRegRc523(rBitFraming,0x07);           //TxLastBits=7 for short frame
		s_WriteRegRc523(rBitFramingReg,0x07); 

		//s_WriteRegRc523(rControl,0x00);


		//s_WriteRegRc523(rRxControl1,0x73);           //default:0x73,for rcv gain
		s_WriteRegRc523(rRxModeReg,0x00);
		//RxCRCEn(b7) = 0; RxSpeed(b654) =000: 106; RxNoErr, RxMultiple = 0; RxFraming = 0   
		s_WriteRegRc523(rRFCfgReg,g_tReg.VAx_reg_RFCfgReg_RxGain_value<<4);
		//RXGAIN(B654)=100 :33DB 
		//s_WriteRegRc523(rDecoderControl,0x08);       //default:0x08,decoder mode:ISO14443 type A
		//s_WriteRegRc523(rBitPhase,0xad);             //default:0xad
		//s_WriteRegRc523(rRxThreshold,0xff);          //default:0xff,define rx signal threshold
		s_WriteRegRc523(rRxThresholdReg,((g_tReg.VAx_reg_RxThreshold_MinLevel_value<<4)|g_tReg.VAx_reg_RxThreshold_CollLevel_value)); //0x55

		s_WriteRegRc523(rModWidthReg, 0x26);      /* Modwidth = 0x26 */   
 
		//s_WriteRegRc523(rDemodReg,g_tReg.VAx_reg_DemodReg_value); //added by ding
		//s_WriteRegRc523(rRxControl2,0x41);           //default:0x41,RxAutoPowerDown=1  


		s_WriteRegRc523(rRxSelReg,0x80); //0x84


		if((g_FDT_Flag==FDT_TYPE_0)||(g_FDT_Flag==FDT_TYPE_1)){		//FDTa,min -128/fc		
			s_SetBitRegRc523(rRxSelReg,0x08);
		}
		else{														//FDTb

		}

		
		//s_WriteRegRc523(rChannelRedundancy,0x03);    //no CRC,odd parity,default:0x03 
		s_ClrBitRegRc523(rTxModeReg,0x80);
		s_ClrBitRegRc523(rRxModeReg,0x80);
		s_ClrBitRegRc523(rMfRxReg,0x10); 


		//s_WriteRegRc523(rCRCPresetLSB,0x63);
		//s_WriteRegRc523(rCRCPresetMSB,0x63);
		s_WriteRegRc523(rModeReg,0x01);
		//MSBFIRST(B7) TXWAITRF(5) POLMFIN(B3) CRCPreset(B1B0)=01: 6363

		//s_WriteRegRc523(rFIFOLevel,T_FIFO_LEVEL);    //FIFO warning bytes
		s_WriteRegRc523(rWaterLevelReg,T_FIFO_LEVEL);
		s_WriteRegRc523(rFIFOLevelReg,0x80);   

		//s_WriteRegRc523(rTimerControl,0x06);         //D2-TStopRxBegin,D1-TStartUponTxEnd
		s_WriteRegRc523(rTModeReg,0x80);  

		//------------
		// 需要增加这条
		s_ClrBitRegRc523(rStatus2Reg, BIT_MFCRYPTO1ON); 
	}
	else
	{ 
		//s_WriteRegRc523(rTxControl,0x4b);            //disable 100%ASK,modulated TX 
		s_WriteRegRc523(rTxControlReg,0x83);
		s_WriteRegRc523(rTxASKReg,0x00);
		s_WriteRegRc523(rTxSelReg,0x10);

		//s_WriteRegRc523(rCwConductance,0x3f);        //default 0x3f,for operating distance adjust		
		s_WriteRegRc523(rGsNReg, (g_tReg.VAx_reg_CWGsN_value_B<<4) | g_tReg.VAx_reg_ModGsN_value_B);
		s_WriteRegRc523(rCWGsPReg, g_tReg.VAx_reg_CWGsP_value_B);
		s_WriteRegRc523(rModGsPReg,g_tReg.VAx_reg_ModGsP_value_B);
		s_WriteRegRc523(rRxThresholdReg,((g_tReg.VAx_reg_RxThreshold_MinLevel_value_B<<4)|g_tReg.VAx_reg_RxThreshold_CollLevel_value_B)); //0x55

		//s_WriteRegRc523(rModConductance,0x04);       //0x05,0x04,set modulation index at 12%,type B only
//		s_WriteRegRc523(rModGsPReg,0x04);
 
		//s_WriteRegRc523(rCoderControl,0x20);         //coder mode:ISO14443 type B
		s_WriteRegRc523(rTxModeReg,0x03);
		//s_WriteRegRc523(rTypeBFraming,0x00);         //char space 0,EOF 10ETU_2H,SOF 10ETU_2H
		s_WriteRegRc523(rTypeBReg,0x00|0<<7|1<<6|0<<4); 

		//--all type B commands are sent as an async frame 
		//s_WriteRegRc523(rBitFraming,0x00);           //TxLastBits=0 
		s_WriteRegRc523(rBitFramingReg,0x00);

		s_WriteRegRc523(rRFCfgReg,(g_tReg.VAx_reg_RFCfgReg_RxGain_value_B<<4));

		//s_WriteRegRc523(rRxControl1,0x73);           //default:0x73,for rcv gain
		s_WriteRegRc523(rRxModeReg,0x03);

		//s_WriteRegRc523(rDecoderControl,0x19);       //ISO 14443B Framing,BPSK coding		
		//s_WriteRegRc523(rRxThreshold,0xff);          //Reduced MinLevel & ColLevel.
		//s_WriteRegRc523(rBPSKDemControl,0x3e);       //ignore EOF,High Pass filter on,type B only
		s_WriteRegRc523(rDemodReg,g_tReg.VAx_reg_DemodReg_value_B); 
		//s_WriteRegRc523(rRxControl2,0x01);           //default:0x41,RxAutoPowerDown=0 
		//s_WriteRegRc523(rChannelRedundancy,0x2c);    //CRC,no parity,default:0x03
		s_SetBitRegRc523(rTxModeReg,0x80);
		s_SetBitRegRc523(rRxModeReg,0x80);
		s_SetBitRegRc523(rMfRxReg,0x10);


		//s_WriteRegRc523(rCRCPresetLSB,0xff);
		//s_WriteRegRc523(rCRCPresetMSB,0xff); 
		s_WriteRegRc523(rModeReg,0x03);

		//s_WriteRegRc523(rFIFOLevel,T_FIFO_LEVEL);    //FIFO warning bytes
		s_WriteRegRc523(rWaterLevelReg,T_FIFO_LEVEL);
		s_WriteRegRc523(rFIFOLevelReg,0x80);   
		//s_WriteRegRc523(rTimerControl,0x06);         //D2-TStopRxBegin,D1-TStartUponTxEnd
		s_WriteRegRc523(rTModeReg,0x80);  
	}
	//wait for the carrier to be stable,6000
	DelayMs(6); 
}


/******************************************************************************
*******************************************************************************/
int s_PiccDetectTypeB(uchar mode,uchar *CardType,uchar *SerialNo)
{
	int iret, i; 
	uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
	uchar uid[3][5],ATQB[12];
	ushort tn;

	cur_cid=0;
	cc=0;
	memset(tmps,0x00,sizeof(tmps));
	memset(ATQB,0x00,sizeof(ATQB));
	memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));

	set_picc('B');    
	tmps[0]=0x05; //wakeup-B
	tmps[1]=0x00; //AFI
	tmps[2]=0x08;//PARAM--b4=1-->WUPB
	tn=3;
	g_WakeUpFlag = 1;
	//DelayMs(10);
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
	g_WakeUpFlag = 0;
	printf("\r\n----WUPB--iret=%d--Txlen=%d--\r\n", iret, tn);
	for (i=0; i<tn; i++)
	{
		printf("tmps[%d]=0x%02x ", i, tmps[i]);
	}
	printf("\r\n");

	if(iret)   
		return PICC_RequestErr;    
	else
		g_HaveBCardFlag = HAVE_CARD;

	picc_info[cur_cid].status=S_READY;    
	if(tn!=12)               return PICC_RequestErr;
	if(tmps[0]!=0x50)        return PICC_RequestErr;
	if((tmps[10]&0x0f)!=0x01)return PICC_RequestErr;//not compliant with ISO14443-4

	CardType[0]='B';
	if((tmps[10]&0x0f)==0x01)   CardType[1]='C';

	memcpy(ATQB,tmps,12);
	memcpy(uid[0],tmps+1,4);//PUPI
	picc_info[cur_cid].serial_len=4;
	memcpy(picc_info[cur_cid].serial_no,uid[0],4);
	if(SerialNo!=NULL)
	{
		memset(SerialNo,0x00,10);
		SerialNo[0]=4;
		memcpy(&SerialNo[1],uid[0],4);
	}
	//fetch the picc's maximum frame size
	cc=tmps[10]>>4;  //Max_Frame_Size
	if(cc>8)cc=8;
	picc_info[cur_cid].max_frame_size=tab_frame_size[cc];
	picc_info[cur_cid].card_type='B';

	//帧等待时间整数FWI  应用数据编码ADC  PICC支持的帧选项FO
	cc=tmps[11];
	if(cc&0x02)picc_info[cur_cid].nad_enable=1;
	if(cc&0x01)picc_info[cur_cid].cid_enable=1;

	//fetch the picc frame wait time index
	cc=tmps[11]>>4;
	if(cc>14)cc=14;
	picc_info[cur_cid].max_delay_index=cc;

	//B6--send ATTRIB command
	tmps[0]=0x1d;
	memcpy(tmps+1,uid,4);
	tmps[5]=0x00;//D7D6-TR0,D5D4-T1,D4-EOF suppress,D3-SOF suppress
	tmps[6]=0x05;//SPEED/FSDI(0-16,5-64)
	tmps[7]=0x01;//tx protocol type
	tmps[8]=cur_cid;
	tn=9;
	//DelayMs(10);
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
	//printf("send ATTRIB command=%d \r\n", iret);
	//cc=s_ReadRegRc523(rCommand);
	//printf("ATT:%d,%02X%02X%02X%02X%02X %02X ", tn,tmps[0],tmps[1],tmps[2],tmps[3],tmps[4],cc);
	if(iret)            return PICC_AttribErr;
	if(!tn)             return PICC_AttribErr;

	cc=tmps[0]&0x0f;//CID
	if(cc!=cur_cid)     return PICC_AttribErr;

	picc_info[cur_cid].max_buffer_index=tmps[0]>>4;
	picc_info[cur_cid].status=S_ACTIVE;
	picc_info[cur_cid].blk_no=0;

	DelayMs(12);//50->30->20 OK

	return 0;
}

/******************************************************************************
*******************************************************************************/
int s_PiccDetectTypeA(uchar mode,uchar *CardType,uchar *SerialNo)
{
	int iret, i; 
	uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
	uchar cascade,uid_type,uid[3][5],ATQA[2];
	ushort Txlen;
	uchar collBitNo, antiCollBits, antiBackupBits;
	uchar collSnByteNo;		//在返回序列号时，冲突的序列号字节编号(0-3)
	uchar snTempBuf[5];		//保存临时SN

	collSnByteNo = 0;
	collBitNo = 0;
	antiCollBits = 0;
	cur_cid=0;
	cc=0;
	memset(tmps,0x00,sizeof(tmps));
	memset(ATQA,0x00,sizeof(ATQA));
	memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));
	memset(snTempBuf, 0x00, sizeof(snTempBuf));

	if(mode=='A')  set_picc('A');
	else set_picc('M');

	tmps[0]=0x52;  //WUPA-0x52,REQA-0x26
	//tmps[0]=0x26;  //WUPA-0x52,REQA-0x26
	Txlen=1;
	g_WakeUpFlag = 1;
	//DelayMs(10); 	
	iret=s_PiccCmdExchange(0,0,tmps,&Txlen);
	g_WakeUpFlag = 0;


	collBitNo = s_ReadRegRc523(rCollReg) & 0x1f ;	//冲突位
	//printf("\r\ncollBitNo=%d\r\n", collBitNo);	

	//printf("\r\n----WUPA--iret=%d--Txlen=%d--\r\n", iret, Txlen);
	//for (i=0; i<Txlen; i++)
	//{
	//	printf("tmps[%d]=0x%02x ", i, tmps[i]);
	//}
	//printf("\r\n");

	//注释：如果一个A类CPU卡WUPA命令返回“04 00”，另外一张返回“08 00”，这时候会在第3个位处出现位冲突
	//这步暂时不做错误退出处理
	if (iret==PICC_TypeAColl && g_PiccCheckCardMode==CHECK_ONE_CARD)
	{
		printf("PICC_Collision1\r\n"); 
		return PICC_Collision;  //寻到多张A卡
	}
	else if(iret!=0 && iret!=PICC_TypeAColl)   
	{
		printf("WUPA Err!\r\n");     
		printf("iret=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);     
		return PICC_RequestErr;
	}
	else
		g_HaveACardFlag = HAVE_CARD;

	if(Txlen!=2) return PICC_RequestErr;  //WUPA 应答(ATQA),2个字节长度   
	picc_info[cur_cid].status=S_READY;    
	memcpy(ATQA,tmps,2);
	uid_type=tmps[0]>>6; // 04(0000 0100)
	if(uid_type>2)return PICC_RequestErr;  //UID的长度错误

	//send anticollision command 防冲突 
	//s_WriteRegRc523(rBitFraming,0x00);   //TxLastBits=0 for bit oriented frame 
	s_WriteRegRc523(rBitFramingReg,0x00);
	//s_WriteRegRc523(rDecoderControl,0x28); 
	s_ClrBitRegRc523(rCollReg,0x80);  //valueaftercoll
	for(cascade=0;cascade<=uid_type;cascade++)
	{
		if(cascade)
		{ 
			//s_WriteRegRc523(rChannelRedundancy,0x03);  //no CRC,odd parity,default:0x03
			s_ClrBitRegRc523(rMfRxReg ,0x10);  //parity disable
			s_ClrBitRegRc523(rTxModeReg,0x80);
			s_ClrBitRegRc523(rRxModeReg,0x80); 
		}
		if(cascade==0)  tmps[0]=PICC_ANTICOLL1;
		else if(cascade==1) tmps[0]=PICC_ANTICOLL2;
		else tmps[0]=PICC_ANTICOLL3; 

		//发送防冲突命令
		tmps[1]=0x20;  //NVB
		Txlen=2;
		//DelayMs(10); 
		iret=s_PiccCmdExchange(0,0,tmps,&Txlen); //ANTICOLLISION应答(UID CLn)
		//cc=s_ReadRegRc523(rCommandReg);

		if(Txlen!=5)//UID的长度为5字节
		{
			return PICC_RequestErr;		// 防冲突命令 93 20 (95 20 或97 20)返回的UID长度都为5 
			//return PICC_Collision;	//寻到多张A卡
		}

		if (iret==PICC_TypeAColl && g_PiccCheckCardMode==CHECK_ONE_CARD)
		{
			printf("PICC_Collision2\r\n"); 
			return PICC_Collision;  //寻到多张A卡
		}
		else if (iret==PICC_TypeAColl && g_PiccCheckCardMode==CHECK_A_CARD)
		{

			//collBitNo = s_ReadRegRc523(rCollPos);	//冲突位
			collBitNo = s_ReadRegRc523(rCollReg)& 0x1f;
			//printf("\r\ncollBitNo=%d\r\n", collBitNo);	

			//printf("\r\n----NVB20--iret=%d--Txlen=%d--\r\n", iret, Txlen);
			//for (i=0; i<Txlen; i++)
			//{
			//	printf("tmps[%d]=0x%02x ", i, tmps[i]);
			//}
			//printf("\r\n");

			//查找序列号中第几个字节冲突
			for (i=4; i>=0; i--)
			{
				if (tmps[i] != 0) 
					break;
			}
			antiCollBits = tmps[i];		
			collSnByteNo = i;
			//printf("\r\nantiCollBits=%d, collSnByteNo=%d \r\n", antiCollBits, collSnByteNo);

			memcpy(snTempBuf, tmps, collSnByteNo);		//保存未冲突的SN字节

			s_WriteRegRc523(rBitFramingReg, (collBitNo<<4 | collBitNo));

			if (cascade == 0)  
				tmps[0] = PICC_ANTICOLL1;
			else if (cascade ==1 ) 
				tmps[0] = PICC_ANTICOLL2;
			else 
				tmps[0] = PICC_ANTICOLL3; 

			tmps[1] = 0x20 + collBitNo;  //NVB
			tmps[2] = antiCollBits & (~(1 << (collBitNo-1) ));
			antiBackupBits = tmps[2];
			Txlen=3;
			//DelayMs(10); 
			iret=s_PiccCmdExchange(0,0,tmps,&Txlen);		//ANTICOLLISION应答(UID CLn)
			//printf("\r\n----NVB20+coll --iret=%d--Txlen=%d--\r\n", iret, Txlen);

			//for (i=0; i<Txlen; i++)
			//{
			//	printf("tmps[%d]=0x%02x ", i, tmps[i]);
			//}
			//printf("\r\n");	
			tmps[0] = tmps[0] | antiBackupBits;		

			memcpy(snTempBuf+collSnByteNo, tmps, 5-collSnByteNo);
			memcpy(tmps, snTempBuf, 5);
		}
		else if(iret != 0)   
		{
			printf("iret1=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);     
			return PICC_AntiCollErr;
		}



		for(cc=0,i=0;i<5;i++) cc^=tmps[i];		//卡校验	
		if(cc)
		{
			printf("UidCRCErr=%02x\r\n",cc);
			return PICC_UidCRCErr;							//error crc check of UID
		}       
		memcpy(uid[cascade],tmps,5);

		//A3--send select command,cascade level n SELECT命令 
		//s_WriteRegRc523(rChannelRedundancy,0x0F);    //RxCRC,TxCRC,Odd Parity 
		s_SetBitRegRc523(rTxModeReg,0x80);
		s_SetBitRegRc523(rRxModeReg,0x80);
		s_ClrBitRegRc523(rMfRxReg,0x10);

		if(cascade==0)  tmps[0]=PICC_SELECT1;
		else if(cascade==1) tmps[0]=PICC_SELECT2;
		else tmps[0]=PICC_SELECT3;         

		tmps[1]=0x70;  //NVB
		memcpy(tmps+2,uid[cascade],5);
		Txlen=7;
		//DelayMs(10); 
		iret=s_PiccCmdExchange(0,0,tmps,&Txlen); //SELECT应答(SAK)(长度为1字节)
		//cc=s_ReadRegRc523(rCommandReg);
		if (iret==PICC_TypeAColl && g_PiccCheckCardMode==CHECK_ONE_CARD)
		{
			printf("PICC_Collision3\r\n"); 
			return PICC_Collision;  //寻到多张A卡
		}
		else if(iret != 0)   
		{
			printf("iret2=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);     
			return PICC_AntiCollErr;
		}

		if(!Txlen)  //SAK的长度错误
		{
			//printf("tnanticollerr(6)=%d\r\n",tn);
			return PICC_AntiCollErr;
		}        
	}

	if(tmps[0]&0x04)    return PICC_AntiCollErr;//UID not complete    

	if(!uid_type)
	{
		picc_info[cur_cid].serial_len=4;
		memcpy(picc_info[cur_cid].serial_no,uid[0],4);
	}
	else if(uid_type==1)
	{
		picc_info[cur_cid].serial_len=7;
		memcpy(picc_info[cur_cid].serial_no,uid[0]+1,3);
		memcpy(picc_info[cur_cid].serial_no+3,uid[1],4);
	}
	else
	{
		picc_info[cur_cid].serial_len=10;
		memcpy(picc_info[cur_cid].serial_no,uid[0]+1,3);
		memcpy(picc_info[cur_cid].serial_no+3,uid[1]+1,3);
		memcpy(picc_info[cur_cid].serial_no+6,uid[2],4);
	}

	if(SerialNo!=NULL)
	{
		memset(SerialNo,0x00,10);
		SerialNo[0]=picc_info[cur_cid].serial_len;
		memcpy(&SerialNo[1],picc_info[cur_cid].serial_no,picc_info[cur_cid].serial_len);
		CardType[0]='A';
		if(tmps[0]&0x20)
		{
			CardType[1]='C';
		}
		else CardType[1]='M';	 	
	}

	if (mode=='M' && CardType[1]=='C')		
	{
		return PICC_CardTyepErr;	//卡类型错误，20080428 
	}

	if(mode=='M')    picc_info[cur_cid].card_type='M';	  
	else             picc_info[cur_cid].card_type='A';	
	picc_info[cur_cid].status=S_ACTIVE;    
	return 0;
}



int PICC_CARRIER_ON(void)
{	
          s_WriteRegRc523(rTxControlReg,(0x83<<0));            //100%ASK,modulated TX


//w_regRC632(rCwConductance, 0x3f);	//default 0x3f,for operating distance adjust

//w_regRC632(rCoderControl,0x19); 		//coder mode:ISO14443 type A

}

int PICC_CARRIER_OFF(void)
{
	s_ClrBitRegRc523(rTxControlReg,(0x83<<0)); 
}



void PICC_RESET2(void)
{	
	PICC_CARRIER_OFF();
	DelayMs(6);
	PICC_CARRIER_ON();
	DelayMs(2);
}

#if 0
void PICC_RESET_HW(void)
{	//reset PCD
//	OkiCLib_set8bit(PO6, RF_RST);
//	DelayMs(2);
#if 0
	OkiCLib_clr8bit(PO6, RF_RST);
	DelayMs(5);
	OkiCLib_set8bit(PO6, RF_RST);
	DelayMs(2);
#endif
	exbus2_rf_reset();
	
	DelayMs(100);
	return;
}
#endif


int PICC_WUPA(uchar *atqa)
{

    int iret, i;
    uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
    uchar cascade,uid_type,uid[3][5],ATQA[2];
    ushort Txlen;
	uchar collBitNo, antiCollBits, antiBackupBits;
	uchar collSnByteNo;		//在返回序列号时，冲突的序列号字节编号(0-3)
	uchar snTempBuf[5];		//保存临时SN

	collSnByteNo = 0;
	collBitNo = 0;
	antiCollBits = 0;
    cur_cid=0;
    cc=0;
    memset(tmps,0x00,sizeof(tmps));
    memset(ATQA,0x00,sizeof(ATQA));
    memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));
	memset(snTempBuf, 0x00, sizeof(snTempBuf));

	g_FDT_Flag=FDT_TYPE_0;


	set_picc('A');
	
	tmps[0]=0x52;  //WUPA-0x52,REQA-0x26
	//tmps[0]=0x26;  //WUPA-0x52,REQA-0x26
	Txlen=1;
	//g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
	
	g_WakeUpFlag = 1;
	iret=s_PiccCmdExchange(0,0,tmps,&Txlen);
	g_WakeUpFlag = 0;
	
//printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
        
	if (iret == PICC_KeyCancel)	
	{
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_KeyCancel;
	}
	
	if (iret==PICC_TypeAColl)
	{
		printf("PICC_Collision1\r\n");
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_Collision;  //寻到多张A卡
	}
    else if(iret!=0 && iret!=PICC_TypeAColl)
    {
//		printf("WUPA Err!\r\n");
//        printf("iret=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);
if(iret!=PICC_SOFErr){															//TA204
	if(iret!=PICC_RxTimerOut)
			printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
	        return iret;
}
    }
	else{
	//	printf("ok :%s %d\r\n",__FUNCTION__,__LINE__);
//		g_HaveACardFlag = HAVE_CARD;
	}
	if(Txlen!=2){
		printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;  //WUPA 应答(ATQA),2个字节长度
	}
{
	uchar i,j,k;
	
	k=0;
	j=tmps[0]&0x1f;
	for(i=0;i<5;i++)
		if(j&(1<<i))
			k++;
	if(k!=1)
		return PICC_Collision;
}	
	if(atqa){
		atqa[0]=tmps[0];
		atqa[1]=tmps[1];
	}
return PICC_OK; 
}



int PICC_WUPB(uchar *atqb)
{

    int iret, i;
    uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
    uchar uid[3][5],ATQB[12];
    ushort tn;
	uchar retry_count=0;

	g_FDT_Flag=FDT_TYPE_4;


    cur_cid=0;
    cc=0;
    memset(tmps,0x00,sizeof(tmps));
    memset(ATQB,0x00,sizeof(ATQB));
    memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));


	set_picc('B');
RETRY:	
	tmps[0]=0x05; //wakeup-B
	tmps[1]=0x00; //AFI
	tmps[2]=0x08;//PARAM--b4=1-->WUPB
	tn=3;
//	g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
	g_WakeUpFlag = 1;
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
	g_WakeUpFlag = 0;

	if (iret == PICC_KeyCancel)	
	{
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_KeyCancel;
	}

    if(iret){
		if(iret!=-3505)
			printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
		/*

		if(iret==PICC_CRCErr)
			return iret;
*/
		if(iret==PICC_RxTimerOut)
		if(PICC_AntiColl_B_Flag){
			retry_count++;
			if(retry_count>3){
				printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
				return iret;
			}
			goto RETRY;
		}

		
		if(iret!=-3505)
			printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
		return iret;
	}
	else{
	//	printf("ok :%s %d\r\n",__FUNCTION__,__LINE__);
//		g_HaveBCardFlag = HAVE_CARD;
	}

//    picc_info[cur_cid].status=S_READY;
    if((tn!=12)&&(tn!=13)) {
		printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,tn);
		return PICC_RequestErr;
	}
    if(tmps[0]!=0x50) {
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}
/*    if((tmps[10]&0x0e)!=0x00){
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}//not compliant with ISO14443-4
*/
	if(atqb){
		memcpy(atqb,tmps,tn);
		if(tn==12){
			atqb[12]=0|0;	//rfu?
		}
	}

	return PICC_OK; 
}

#define FSDI 8
#define CID 0


int PICC_WUPA_RATS(void)
{

	int iret, i;
	uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
	uchar cascade,uid_type,uid[3][5],ATQA[2];
	ushort Txlen;
	uchar collBitNo, antiCollBits, antiBackupBits;
	uchar collSnByteNo; 	//在返回序列号时，冲突的序列号字节编号(0-3)
	uchar snTempBuf[5]; 	//保存临时SN

	uchar retry_count=0;

	collSnByteNo = 0;
	collBitNo = 0;
	antiCollBits = 0;
	cur_cid=0;
	cc=0;
	memset(tmps,0x00,sizeof(tmps));
	memset(ATQA,0x00,sizeof(ATQA));
//	memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));
	memset(snTempBuf, 0x00, sizeof(snTempBuf));

g_FDT_Flag=FDT_TYPE_2;


//	set_picc('A');
	
	//standard frame crc.
		s_WriteRegRc523(rBitFramingReg,0x00);
	
		s_SetBitRegRc523(rTxModeReg,0x80);
		s_SetBitRegRc523(rRxModeReg,0x80);
		s_ClrBitRegRc523(rMfRxReg,0x10);
RETRY:
	
	tmps[0]=0xE0;  //WUPA-0x52,REQA-0x26
	tmps[1]=(FSDImin<<4)|CID;//0x80; 
	Txlen=2;
	//g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
		printf(":%d %d\r\n",tmps[1],__LINE__);
g_ActiveFlag=1;
	iret=s_PiccCmdExchange(0,0,tmps,&Txlen);
g_ActiveFlag=0;
	
	printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);

	if((iret==PICC_RxTimerOut)){
		if(retry_count>2){
			printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
			return PICC_RatsErr;
		}
		retry_count++;
		goto RETRY;
	}
	else if (iret==PICC_TypeAColl)
	{
		printf("PICC_Collision1\r\n");
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_Collision;	//寻到多张A卡
	}
	else if(iret!=0 && iret!=PICC_TypeAColl)
	{
		printf("WUPA Err!\r\n");
		printfx("iret=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);
		printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}
	else{
		printf("ok :%s %d\r\n",__FUNCTION__,__LINE__);
//		g_HaveACardFlag = HAVE_CARD;
		#if 0
		{
			int i,j;
			i=Txlen;
				printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
			for(j=0;j<i;j++)
			{
				printfx("%02x ", tmps[j]);
			}
			printfx("\r\n");
		}
		#endif


		if(Txlen<(1)){	//ONLY TL is ok.
				printfx("error :%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
			return PICC_RatsErr;
		}

		if(Txlen!=tmps[0]){
				printfx("error :%s %d %d %d\r\n",__FUNCTION__,__LINE__,Txlen,tmps[0]);
			return PICC_RatsErr;
		}

		if(Txlen<(1+4+15))
			memcpy(picc_info[0].ATS,tmps,Txlen);
		else
			memcpy(picc_info[0].ATS,tmps,1+4+15);

		if(Txlen==1){
			picc_info[0].ATS[1]=2;
		}

		uchar i=0;
			
		if(picc_info[0].ATS[1]&0x10){	//TA
			picc_info[0].TA=picc_info[0].ATS[2+i];
			i++;
		}
		else{
			picc_info[0].TA=1<<7|(0x00);
		}
		
		if(picc_info[0].ATS[1]&0x20){	//TB
			picc_info[0].TB=picc_info[0].ATS[2+i];
			i++;
		}else{
			picc_info[0].TB=(4<<4)|(0<<0);
		}
			
		if(picc_info[0].ATS[1]&0x40){	//TC
			picc_info[0].TC=picc_info[0].ATS[2+i];
			i++;
		}else{
			picc_info[0].TC=(0x00);
		}
	}
	picc_info[0].blk_no=0;

	g_FDT_Flag=FDT_TYPE_1;

	picc_info[0].SFGT_need=1;

		
	return PICC_OK; 
}




int PICC_WUPB_ATTRIB(void)
{

	int iret, i;
	uchar cc,cur_cid,tmps[PICC_RXPOOL_SIZE];
	uchar uid[3][5],ATQB[12];
	ushort tn;
	uchar retry_count=0;


	g_FDT_Flag=FDT_TYPE_3;

	cur_cid=0;
	cc=0;
	memset(tmps,0x00,sizeof(tmps));
//	memset(ATQB,0x00,sizeof(ATQB));
//	memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));


RETRY:

//	set_picc('B');
	
	tmps[0]=0x1D; //wakeup-B
	//tmps[1]=picc_info[cur_cid].PUPI; 
	memcpy(tmps+1,&(picc_info[cur_cid].ATQB[1]),4);
	tmps[5]=0x00;//P1

	const uchar t[]={0,1,2,0,3};
	tmps[6]=FSDImin|((t[(picc_info[cur_cid].ATQB[9]>>4)&(0x7)]<<6)|(t[(picc_info[cur_cid].ATQB[9])&(0x7)]<<4));//P2

	tmps[7]=0x01;//P3
	tmps[8]=0x00;//P4
	tn=9;
//	g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
	g_ActiveBFlag=1;
g_ActiveFlag=1;
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
g_ActiveFlag=0;
	g_ActiveBFlag=0;

	if(iret){
		
		printf("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
		
	//	if(iret==PICC_CRCErr)
	//		return iret;
		
		if((iret==PICC_RxTimerOut)||(iret==PICC_CRCErr))
		{
			retry_count++;
			if(retry_count>2){
				printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
				return PICC_RatsErr;
			}
			goto RETRY;
		}
		printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
		return PICC_RequestErr;
	}
	else{
		printf("ok :%s %d\r\n",__FUNCTION__,__LINE__);
//		g_HaveBCardFlag = HAVE_CARD;
	}
/*
//	  picc_info[cur_cid].status=S_READY;
	if(tn!=12) {
		printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}
	if(tmps[0]!=0x50) {
		printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}
	if((tmps[10]&0x0f)!=0x01){
		printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
		return PICC_RequestErr;
	}//not compliant with ISO14443-4

*/
	if(tn==0){
		
		retry_count++;
		if(retry_count>2){
			printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
			return PICC_RatsErr;
		}
		goto RETRY;

	}

	if((tmps[0]&0x0f)){
//	if((tmps[0]&0x0f)&&(tmps[0]!=0x1d)){

		printfx("error:%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
//		printfx("error:%s %d %d %02x %02x\r\n",__FUNCTION__,__LINE__,iret,tmps[0],tn);

		return PICC_RequestErr;
	}

	picc_info[0].blk_no=0;

	picc_info[0].SFGT_need=1;


	return PICC_OK; 
}

static uchar TYPE_A,TYPE_B;


int Picc_HLTA(void);
#define  PICC_REGFILE_NAME "picc_reg"
void VAx_save_reg_xx(void)
{
	BYTE attr[2] = {0xff,0x02};
	int fd;
	fd=Lib_FileOpen(PICC_REGFILE_NAME, O_RDWR);
	if (fd<0)
	{
		trace_debug_printf("VAx_save_reg_xx return %d",fd);
		return ;
	}
	
	Lib_FileWrite(fd,(uchar*)&g_tReg,sizeof(RegPara) );
	Lib_FileClose(fd);
}

void VAx_recall_reg_xx()
{
	BYTE attr[2] = {0xff,0x02};
	int fd;
	int iret ;

	g_tReg.VAx_reg_CWGsN_value=0xd;
	g_tReg.VAx_reg_ModGsN_value=0x06;//0x0b;
	g_tReg.VAx_reg_CWGsP_value=0x3f;
	g_tReg.VAx_reg_ModGsP_value=0x20;
	g_tReg.VAx_reg_RxThreshold_MinLevel_value= 0x0a;//0xe;
	g_tReg.VAx_reg_RxThreshold_CollLevel_value=0x5;
	g_tReg.VAx_reg_RFCfgReg_RxGain_value= 0x4;//0x06;//0x4;
	g_tReg.VAx_reg_DemodReg_value=0x4d;
	g_tReg.VAx_reg_CWGsN_value_B=0xF;
	g_tReg.VAx_reg_ModGsN_value_B=0x06;//0x02;//0x02;
	g_tReg.VAx_reg_CWGsP_value_B=0x3e;
	g_tReg.VAx_reg_ModGsP_value_B=0x04;//0x08;//0x08;//0x03;
	g_tReg.VAx_reg_RxThreshold_MinLevel_value_B=0xd;
	g_tReg.VAx_reg_RxThreshold_CollLevel_value_B=0x5;
	g_tReg.VAx_reg_RFCfgReg_RxGain_value_B=0x05;//0x4;
	g_tReg.VAx_reg_DemodReg_value_B=0x6e;//0x4d;

	fd=Lib_FileOpen(PICC_REGFILE_NAME, O_RDWR);
	//s_UartPrint(COM_DEBUG,"VAx_recall_reg_xx return %d",fd);
	if (fd<0)
	{
		fd = Lib_FileOpen(PICC_REGFILE_NAME, O_CREATE);
		//s_UartPrint(COM_DEBUG,"Lib_FileOpen (O_CREATE) return %d",fd);
		if (fd)
		{
			Lib_FileWrite(fd,(uchar*)&g_tReg,sizeof(RegPara) );
		}
		
	}
	else
	{
		iret = Lib_FileRead(fd,(uchar*)&g_tReg,sizeof(RegPara) );
		
	}
	Lib_FileClose(fd);

 
}


void VAx_printf_reg_xx(void)
{

	info_printf("\r\n--------------------------------------------------\r\n");
	info_printf("A CWGsN%02x					a+  z-\r\n",g_tReg.VAx_reg_CWGsN_value);
	info_printf("A ModGsN:%02x					s+  x-\r\n",g_tReg.VAx_reg_ModGsN_value);
	info_printf("A CWGsP:%02x					d+  c-\r\n",g_tReg.VAx_reg_CWGsP_value);
	info_printf("A ModGsP:%02x	  				f+  v-\r\n",g_tReg.VAx_reg_ModGsP_value);
	info_printf("A RxThreshold_MinLevel:%02x			g+  b-\r\n",g_tReg.VAx_reg_RxThreshold_MinLevel_value);
	info_printf("A RxThreshold_CollLevel:%02x			q+  w-\r\n",g_tReg.VAx_reg_RxThreshold_CollLevel_value);
	info_printf("A RFCfgReg_RxGain:%02x				1+  2-\r\n",g_tReg.VAx_reg_RFCfgReg_RxGain_value);
	info_printf("A VAx_reg_DemodReg_value:%02x			5+  6-\r\n",g_tReg.VAx_reg_DemodReg_value);

	info_printf("B CWGsN:%02x 		 			h+  n-\r\n",g_tReg.VAx_reg_CWGsN_value_B);
	info_printf("B ModGsN:%02x	  				j+  m-\r\n",g_tReg.VAx_reg_ModGsN_value_B);
	info_printf("B CWGsP:%02x 	 				k+  ,-\r\n",g_tReg.VAx_reg_CWGsP_value_B);
	info_printf("B ModGsP:%02x	 	 			l+  -\r\n",g_tReg.VAx_reg_ModGsP_value_B);
	info_printf("B RxThreshold_MinLevel:%02x			;+  /-\r\n",g_tReg.VAx_reg_RxThreshold_MinLevel_value_B);
	info_printf("B RxThreshold_CollLevel:%02x			[+  ]-\r\n",g_tReg.VAx_reg_RxThreshold_CollLevel_value_B);

	info_printf("B RFCfgReg_RxGain:%02x	  			3+ 4-\r\n",g_tReg.VAx_reg_RFCfgReg_RxGain_value_B);
	info_printf("B VAx_reg_DemodReg_value:%02x	  		7+  8-\r\n",g_tReg.VAx_reg_DemodReg_value_B);

}

 
void set_rc523reg()
{
	static uchar one=0;

	
	if(0==one)
	{
		one=1;
		//s_Kb_Flush();
		//s_UartInit();
		VAx_printf_reg_xx();
	}
	
	if(s_hitkey(COM_DEBUG)==0)
	{
		
		uchar key = s_getkey(COM_DEBUG);
		s_flushkey(COM_DEBUG);
		info_printf("Key=%c\r\n",key);
		switch (key)
		{
			case 'a':
				if(g_tReg.VAx_reg_CWGsN_value!=0xf)
					g_tReg.VAx_reg_CWGsN_value++;
				break;
			case 'z':
				if(g_tReg.VAx_reg_CWGsN_value!=0x0)
					g_tReg.VAx_reg_CWGsN_value--;
				break;
			case 's':
				if(g_tReg.VAx_reg_ModGsN_value!=0xf)
					g_tReg.VAx_reg_ModGsN_value++;
				break;
			case 'x':
				if(g_tReg.VAx_reg_ModGsN_value!=0x0)
					g_tReg.VAx_reg_ModGsN_value--;
				break;
			case 'd':
				if(g_tReg.VAx_reg_CWGsP_value!=0x3f)
					g_tReg.VAx_reg_CWGsP_value++;
				break;
			case 'c':
				if(g_tReg.VAx_reg_CWGsP_value!=0x0)
					g_tReg.VAx_reg_CWGsP_value--;
				break;
			case 'f':
				if(g_tReg.VAx_reg_ModGsP_value!=0x3f)
					g_tReg.VAx_reg_ModGsP_value++;
				break;
			case 'v':
				if(g_tReg.VAx_reg_ModGsP_value!=0x0)
					g_tReg.VAx_reg_ModGsP_value--;
				break;
			case 'g':
				if(g_tReg.VAx_reg_RxThreshold_MinLevel_value!=0xf)
					g_tReg.VAx_reg_RxThreshold_MinLevel_value++;
				break;
			case 'b':
				if(g_tReg.VAx_reg_RxThreshold_MinLevel_value!=0x0)
					g_tReg.VAx_reg_RxThreshold_MinLevel_value--;
				break;
			case 'q':
				if(g_tReg.VAx_reg_RxThreshold_CollLevel_value!=0x7)
					g_tReg.VAx_reg_RxThreshold_CollLevel_value++;
				break;
			case 'w':
				if(g_tReg.VAx_reg_RxThreshold_CollLevel_value!=0x0)
					g_tReg.VAx_reg_RxThreshold_CollLevel_value--;
				break;

			case '1':
				if(g_tReg.VAx_reg_RFCfgReg_RxGain_value!=0x7)
					g_tReg.VAx_reg_RFCfgReg_RxGain_value++;
				break;
			case '2':
				if(g_tReg.VAx_reg_RFCfgReg_RxGain_value!=0x0)
					g_tReg.VAx_reg_RFCfgReg_RxGain_value--;
				break;

			case '5':
				if(g_tReg.VAx_reg_DemodReg_value!=0xff)
					g_tReg.VAx_reg_DemodReg_value++;
				break;
			case '6':
				if(g_tReg.VAx_reg_DemodReg_value!=0x0)
					g_tReg.VAx_reg_DemodReg_value--;
				break;
			case 'h':
				if(g_tReg.VAx_reg_CWGsN_value_B!=0xf)
					g_tReg.VAx_reg_CWGsN_value_B++;
				break;
			case 'n':
				if(g_tReg.VAx_reg_CWGsN_value_B!=0x0)
					g_tReg.VAx_reg_CWGsN_value_B--;
				break;
			case 'j':
				if(g_tReg.VAx_reg_ModGsN_value_B!=0xf)
					g_tReg.VAx_reg_ModGsN_value_B++;
				break;
			case 'm':
				if(g_tReg.VAx_reg_ModGsN_value_B!=0x0)
					g_tReg.VAx_reg_ModGsN_value_B--;
				break;
			case 'k':
				if(g_tReg.VAx_reg_CWGsP_value_B!=0x3f)
					g_tReg.VAx_reg_CWGsP_value_B++;
				break;
			case ',':
				if(g_tReg.VAx_reg_CWGsP_value_B!=0x0)
					g_tReg.VAx_reg_CWGsP_value_B--;
				break;
			case 'l':
				if(g_tReg.VAx_reg_ModGsP_value_B!=0x3f)
					g_tReg.VAx_reg_ModGsP_value_B++;
				break;
			case '.':
				if(g_tReg.VAx_reg_ModGsP_value_B!=0x0)
					g_tReg.VAx_reg_ModGsP_value_B--;
				break;

			case ';':
				if(g_tReg.VAx_reg_RxThreshold_MinLevel_value_B!=0xf)
					g_tReg.VAx_reg_RxThreshold_MinLevel_value_B++;
				break;
			case '/':
				if(g_tReg.VAx_reg_RxThreshold_MinLevel_value_B!=0x0)
					g_tReg.VAx_reg_RxThreshold_MinLevel_value_B--;
				break;
			case '[':
				if(g_tReg.VAx_reg_RxThreshold_CollLevel_value_B!=0x7)
					g_tReg.VAx_reg_RxThreshold_CollLevel_value_B++;
				break;
			case ']':
				if(g_tReg.VAx_reg_RxThreshold_CollLevel_value_B!=0x0)
					g_tReg.VAx_reg_RxThreshold_CollLevel_value_B--;
				break;

			case '3':
				if(g_tReg.VAx_reg_RFCfgReg_RxGain_value_B!=0x7)
					g_tReg.VAx_reg_RFCfgReg_RxGain_value_B++;
				break;
			case '4':
				if(g_tReg.VAx_reg_RFCfgReg_RxGain_value_B!=0x0)
					g_tReg.VAx_reg_RFCfgReg_RxGain_value_B--;
				break;

			case '7':
				if(g_tReg.VAx_reg_DemodReg_value_B!=0xff)
					g_tReg.VAx_reg_DemodReg_value_B++;
				break;
			case '8':
				if(g_tReg.VAx_reg_DemodReg_value_B!=0x0)
					g_tReg.VAx_reg_DemodReg_value_B--;
				break;

 			
			case 0x1b:
				printf("[%s]%d polling return\r\n",__FUNCTION__,__LINE__);
				return  ;

			default :
				break;
			}


		VAx_printf_reg_xx();
	}
	//info_printf("set_rc523reg44\r\n");
	
}
int PICC_POLLING(void)
{
	int ret;
	TYPE_A=0;
	TYPE_B=0;
	
	while(1){
		if(TYPE_A!=0)
			break;
		DelayMs(2);//wait tp 5.1ms~10ms
		g_WakeUpAFlag=1;
		ret=PICC_WUPA(0);
		//info_printf("PICC_WUPA=%d\r\n",ret);
		g_WakeUpAFlag=0;
//		 if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			TYPE_A=1;
			if(TYPE_B==0)								//tb303
				Picc_HLTA();//hlta
		 }
		 
		if(TYPE_B!=0)
			break;
		DelayMs(2);//wait tp
		g_WakeUpBFlag=1;
		ret=PICC_WUPB(0);
		//info_printf("PICC_WUPB=%d\r\n",ret);
		g_WakeUpBFlag=0;
//		if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			TYPE_B=1;
		 }
		set_rc523reg();
	}
	
	return PICC_OK; 
}

static uchar HARDWARE_TEST_BREAK=0; 
int PICC_POLLING_A(void)
{
	int ret;
	TYPE_A=0;
	TYPE_B=0;

	printfx(":%s %d\r\n",__FUNCTION__,__LINE__);

	while(1)
	{
		if(TYPE_A!=0)
			break;
		DelayMs(2);//wait tp 5.1ms~10ms
		g_WakeUpAFlag=1;
		//g_Flag_A=1;
		ret=PICC_WUPA(0);
		//g_Flag_A=
		g_WakeUpAFlag=0;
		//		 if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			TYPE_A=1;

			break ;

			if(TYPE_B==0)								//tb303
				Picc_HLTA();//hlta
		}


		if(TYPE_B!=0)
			break;
		DelayMs(2);//wait tp
		g_WakeUpBFlag=1;
		//	ret=PICC_WUPB(0);
		g_WakeUpBFlag=0;
		//		if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			;//		TYPE_B=1;
		}

#if 1		//rf antennat test
		{

			static uchar one=0;
			if(0==one)
			{
				one=1;
				//Lib_KbFlush();
				//s_UartInit();
				VAx_printf_reg_xx();
			}
			if(s_hitkey(COM_DEBUG)==0){
				uchar key = s_getkey(COM_DEBUG);
				if (key == KEYCANCEL)
				{
					printf("[%s]Line(%d): polling return\r\n",__FUNCTION__,__LINE__);
					HARDWARE_TEST_BREAK=1; 
					return 0;
				}

				VAx_printf_reg_xx();
			}
		}
#endif		 




	}

	return PICC_OK; 
}





int PICC_POLLING_B(void)
{
	int ret;
	TYPE_A=0;
	TYPE_B=0;
	printfx(":%s %d\r\n",__FUNCTION__,__LINE__);

	while(1){
		if(TYPE_A!=0)
			break;
		DelayMs(2);//wait tp 5.1ms~10ms
		g_WakeUpAFlag=1;
		//	ret=PICC_WUPA(0);
		g_WakeUpAFlag=0;
		//		 if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			//		TYPE_A=1;
			//		if(TYPE_B==0)								//tb303
			//			Picc_HLTA();//hlta
		}


		if(TYPE_B!=0)
			break;
		DelayMs(2);//wait tp
		g_WakeUpBFlag=1;
		ret=PICC_WUPB(0);
		g_WakeUpBFlag=0;
		//		if((ret==PICC_OK)||(ret==PICC_Collision)){
		if((ret!=PICC_RxTimerOut)){
			TYPE_B=1;
			break ;
		}
#if 1		//rf antennat test
		{

			static uchar one=0;
			if(0==one)
			{
				one=1;
				//Lib_KbFlush();
				//s_UartInit();
				VAx_printf_reg_xx();
			}
			if(s_hitkey(COM_DEBUG)==0)
			{
				uchar key = s_getkey(COM_DEBUG);
				if(key == KEYCANCEL)
					HARDWARE_TEST_BREAK=1; 
				return 0;

				VAx_printf_reg_xx();
			}
		}
#endif		 




	}

	return PICC_OK; 
}

int PICC_POLLING2(uint count)
{
	int ret=0;
	TYPE_A=0;
	TYPE_B=0;
	
	while(count--){
		if(TYPE_A!=0)
			return PICC_OK; 
		DelayMs(2);//wait tp 5.1ms~10ms
		g_WakeUpAFlag=1;
		if((Card_Type_ABM==0)||(Card_Type_ABM=='A')||(Card_Type_ABM=='M'))
			ret=PICC_WUPA(0);
		else
			ret=PICC_RxTimerOut;
		g_WakeUpAFlag=0;
//		 if((ret==PICC_OK)||(ret==PICC_Collision)){
		if (ret == PICC_KeyCancel)
			return PICC_KeyCancel;
		if((ret!=PICC_RxTimerOut)){
			TYPE_A=1;
			if(TYPE_B==0)								//tb303
				Picc_HLTA();//hlta
		 }

		 
		if(TYPE_B!=0)
			return PICC_OK; 
		DelayMs(2);//wait tp
		g_WakeUpBFlag=1;
		if((Card_Type_ABM==0)||(Card_Type_ABM=='B'))
			ret=PICC_WUPB(0);
		else
			ret=PICC_RxTimerOut;
		g_WakeUpBFlag=0;
//		if((ret==PICC_OK)||(ret==PICC_Collision)){
		if (ret == PICC_KeyCancel)
			return PICC_KeyCancel;

		if((ret!=PICC_RxTimerOut)){
			TYPE_B=1;
		 }
	}
	return PICC_HaveCard; 
}



int PICC_AntiColl_A()
{
	int ret,iret;
	uchar ATQA[2],uid[3][5];
	uchar uid_type,cascade,tmps[PICC_RXPOOL_SIZE];
	ushort Txlen,cc,cur_cid;
	uchar i;
	uchar retry_count=0;

	cur_cid=0;
	memset(&picc_info[cur_cid],0x00,sizeof(picc_info[0]));
	
	retry_count=0;
RETRY0:
	
	//wait t(p)
	ret=PICC_WUPA(ATQA);
	
	if(ret==PICC_RxTimerOut){
		if(retry_count>2){
			printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
			return PICC_Collision;
		}
		retry_count++;
		goto RETRY0;
	}

	
//	if(ret==PICC_Collision){
	if(ret){
		
		return PICC_Collision;
	}
	
	uid_type=ATQA[0]>>6; // 04(0000 0100)
	if(uid_type>2){
		
		return PICC_RequestErr;  //UID的长度错误
	}
		
	
	//send anticollision command 防冲突 
	//s_WriteRegRc523(rBitFraming,0x00);   //TxLastBits=0 for bit oriented frame 
	s_WriteRegRc523(rBitFramingReg,0x00);
	//s_WriteRegRc523(rDecoderControl,0x28); 
	s_ClrBitRegRc523(rCollReg,0x80);  //valueaftercoll
	for(cascade=0;cascade<=uid_type;cascade++)
	{
		if(cascade)
		{ 
			//s_WriteRegRc523(rChannelRedundancy,0x03);  //no CRC,odd parity,default:0x03
			s_ClrBitRegRc523(rMfRxReg ,0x10);  //parity disable
			s_ClrBitRegRc523(rTxModeReg,0x80);
			s_ClrBitRegRc523(rRxModeReg,0x80); 
		}
		
		retry_count=0;
RETRY1:
		
		if(cascade==0)  
			tmps[0]=PICC_ANTICOLL1;
		else if(cascade==1) 
			tmps[0]=PICC_ANTICOLL2;
		else 
			tmps[0]=PICC_ANTICOLL3; 
		
		g_FDT_Flag=FDT_TYPE_0;

		//发送防冲突命令
		tmps[1]=0x20;  //NVB
		Txlen=2;
		//DelayMs(10); 
		iret=s_PiccCmdExchange(0,0,tmps,&Txlen); //ANTICOLLISION应答(UID CLn)
		//cc=s_ReadRegRc523(rCommandReg);
		
		if(iret==PICC_RxTimerOut){
			if(retry_count>2){
				printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
				return PICC_Collision;
			}
			retry_count++;
			goto RETRY1;
		}

		if(Txlen!=5)//UID的长度为5字节
		{
			return PICC_RequestErr;		// 防冲突命令 93 20 (95 20 或97 20)返回的UID长度都为5 
			//return PICC_Collision;	//寻到多张A卡
		}

		if (iret==PICC_TypeAColl)
		{
			printf("PICC_Collision2\r\n"); 
			return PICC_Collision;  //寻到多张A卡
		}
		else if(iret != 0)   
		{
			printf("iret1=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);     
			return PICC_AntiCollErr;
		}

		for(cc=0,i=0;i<5;i++) cc^=tmps[i];		//卡校验	
		if(cc)
		{
			printf("UidCRCErr=%02x\r\n",cc);
			return PICC_UidCRCErr;							//error crc check of UID
		}    

		if(0==uid_type){
			if(0==cascade){
				if(0x88==tmps[0])
					return PICC_AntiCollErr;
			}
		}
		else if(1==uid_type){
			if(0==cascade){
				if(0x88!=tmps[0])
					return PICC_AntiCollErr;
			}
			if(1==cascade){
				if(0x88==tmps[0])
					return PICC_AntiCollErr;
			}

		}
		else if(2==uid_type){
			if(0==cascade){
				if(0x88!=tmps[0])
					return PICC_AntiCollErr;
			}
			if(1==cascade){
				if(0x88!=tmps[0])
					return PICC_AntiCollErr;
			}

		}

		memcpy(uid[cascade],tmps,5);

		//A3--send select command,cascade level n SELECT命令 
		//s_WriteRegRc523(rChannelRedundancy,0x0F);    //RxCRC,TxCRC,Odd Parity 
		s_SetBitRegRc523(rTxModeReg,0x80);
		s_SetBitRegRc523(rRxModeReg,0x80);
		s_ClrBitRegRc523(rMfRxReg,0x10);
		
		retry_count=0;
RETRY2:
		if(cascade==0)  
			tmps[0]=PICC_SELECT1;
		else if(cascade==1) 
			tmps[0]=PICC_SELECT2;
		else 
			tmps[0]=PICC_SELECT3;         

		tmps[1]=0x70;  //NVB
		memcpy(tmps+2,uid[cascade],5);
		Txlen=7;
		//DelayMs(10); 
		iret=s_PiccCmdExchange(0,0,tmps,&Txlen); //SELECT应答(SAK)(长度为1字节)
		//cc=s_ReadRegRc523(rCommandReg);

		if(iret==PICC_RxTimerOut){
			if(retry_count>2){
				printf("error :%s %d\r\n",__FUNCTION__,__LINE__);
				return PICC_Collision;
			}
			retry_count++;
			goto RETRY2;
		}
		
	
		if (iret==PICC_TypeAColl )
		{
			printf("PICC_Collision3\r\n"); 
			return PICC_Collision;  //寻到多张A卡
		}
		else if(iret != 0)   
		{
			printf("iret2=%d:%d,%02X %02X %02X \r\n",iret,Txlen,tmps[0],tmps[1],cc);     
			return PICC_AntiCollErr;
		}

		if(!Txlen)  //SAK的长度错误
		{
			//printf("tnanticollerr(6)=%d\r\n",tn);
			return PICC_AntiCollErr;
		}        
   
		
	}


	if(tmps[0]&0x04)
		return PICC_AntiCollErr;//UID not complete    

	if(!uid_type)
	{
		picc_info[cur_cid].serial_len=4;
		memcpy(picc_info[cur_cid].serial_no,uid[0],4);
	}
	else if(uid_type==1)
	{
		picc_info[cur_cid].serial_len=7;
		memcpy(picc_info[cur_cid].serial_no,uid[0]+1,3);
		memcpy(picc_info[cur_cid].serial_no+3,uid[1],4);
	}
	else
	{
		picc_info[cur_cid].serial_len=10;
		memcpy(picc_info[cur_cid].serial_no,uid[0]+1,3);
		memcpy(picc_info[cur_cid].serial_no+3,uid[1]+1,3);
		memcpy(picc_info[cur_cid].serial_no+6,uid[2],4);
	}


	g_FDT_Flag=FDT_TYPE_1;

	
	return PICC_OK;
}

int PICC_AntiColl_B()
{
	int iret;
	uchar ATQB[13];
	//wait t(p)
	
	PICC_AntiColl_B_Flag=1;
	iret=PICC_WUPB(ATQB);
	PICC_AntiColl_B_Flag=0;
	
	if(iret)   {
		printf("PICC_AntiColl_B %d \r\n",iret);
		return PICC_RequestErr;    

	}
	memcpy(picc_info[0].ATQB,ATQB,13);
	return PICC_OK;
}


int PICC_AntiColl(void)
{
	if((TYPE_A==1)&&(TYPE_B==1)){
		// <t(ressetdelay)
		//reset;
		return PICC_RequestErr;
	}
	
	if((TYPE_A==1)){
		return PICC_AntiColl_A();
	}
	else if((TYPE_B==1)){
		return PICC_AntiColl_B();
	}
}


static volatile uchar g_HLTAFlag=0;


int Picc_HLTA(void)
{
	uchar wait_mode,cid,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret;

//DelayMs(1);
//DelayUs(200);
	
//standard frame crc.
	s_WriteRegRc523(rBitFramingReg,0x00);

	s_SetBitRegRc523(rTxModeReg,0x80);
	s_SetBitRegRc523(rRxModeReg,0x80);
	s_ClrBitRegRc523(rMfRxReg,0x10);

	cid=0;
	dn=2;
	memset(tmps,0x00,sizeof(tmps));
	tmps[0]=PICC_HALT;
	tmps[1]=0x00;
	wait_mode=1;
	wait_mode=0;
	g_WakeUpFlag = 1;
	g_HLTAFlag=1;
	g_FDT_Flag=FDT_TYPE_0;

	iret=s_PiccCmdExchange(0,wait_mode,tmps,&dn);
		
	g_HLTAFlag=0;
	g_WakeUpFlag = 0;
//	printf("H1:%d,%d %02X \r\n",dn,iret,tmps[0]);
	picc_info[cid].status=S_IDLE;
	return iret;
}





int PICC_Remove_A(void)
{
	int iret;
	PICC_RESET2();//reset
	DelayMs(2);//wait tp
	while(1){
		g_WakeUpAFlag=1;
		iret=PICC_WUPA(0);
		g_WakeUpAFlag=0;
		if(iret==PICC_RxTimerOut){
			g_WakeUpAFlag=1;
			iret=PICC_WUPA(0);
			g_WakeUpAFlag=0;
			if(iret==PICC_RxTimerOut){
				g_WakeUpAFlag=1;
				iret=PICC_WUPA(0);
				g_WakeUpAFlag=0;
				if(iret==PICC_RxTimerOut){
					break;
				}
			}
		}
//		else{
			Picc_HLTA();
	//		DelayMs(7);//wait tp
		//	DelayMs(2);//wait tp
//		}
	//	printf(":%s %d\r\n",__FUNCTION__,__LINE__);
	}

	
	return PICC_OK;
}

int PICC_Remove_B(void)
{
	int iret;
	PICC_RESET2();//reset
	DelayMs(2);//wait tp
	while(1){
		g_WakeUpBFlag=1;
		iret=PICC_WUPB(0);
		g_WakeUpBFlag=0;
		if(iret==PICC_RxTimerOut){
			g_WakeUpBFlag=1;
			iret=PICC_WUPB(0);
			g_WakeUpBFlag=0;
			if(iret==PICC_RxTimerOut){
				g_WakeUpBFlag=1;
				iret=PICC_WUPB(0);
				g_WakeUpBFlag=0;
				if(iret==PICC_RxTimerOut){
					break;
				}
			}
		}
		else{
			DelayMs(2);//wait tp
		}
	}

	return PICC_OK;
}




int PICC_main_loop(void)
{
	int iret;
	
	while(1){
		PICC_POLLING();
		iret=PICC_AntiColl();
		if(iret != PICC_OK){
			PICC_RESET2();
			printf("%d\r\n",iret);
			continue;
		}
		
		//activate picc
		if(TYPE_A){
			iret=PICC_WUPA_RATS();
		}
		else{
			iret=PICC_WUPB_ATTRIB();
		}
		if(iret != PICC_OK){
			PICC_RESET2();
			printf("%d\r\n",iret);
			continue;
		}
		//process picc
		return PICC_OK;

		//remove picc

		//reset <t(reset)
		
		//wait <t(pause)

	}
	
}


int Picc_Open2(void)
{
	PICC_RESET2();
	PICC_main_loop();
	pcd_info.open=1;



}

int Picc_Close2(void)
{
	if(TYPE_A){
		PICC_Remove_A();
	}
	else if(TYPE_B){
		PICC_Remove_B();
	}

	PICC_RESET2();



	pcd_info.open=0;

}

int Picc_Command2(APDU_SEND *ApduSend, APDU_RESP *ApduResp) 
{}

int Lib_PiccReset(void)
{
	return 0;
}

int Lib_PiccOpen(void)
{
	PICC_RESET2();
	set_picc('A');
	PICC_CARRIER_ON();
	pcd_info.open=1;

	OPENED_FLAG=1;

	return 0;
}


int Lib_PiccClose(void)
{
	PICC_CARRIER_OFF();
	pcd_info.open=0;
	OPENED_FLAG=0;

	return 0;
}



#if 0  //?
int Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo, uint count)
#endif
int Lib_PiccCheck1(uchar mode,uchar *CardType,uchar *SerialNo, uint count)
{
		int iret;
	
		if(0==OPENED_FLAG){
			return PICC_NotOpen;
		}
	
		if(mode == 0x0a)	mode = 'A';
		if(mode == 0x0b)	mode = 'B';
		if(mode == 'a') 	mode = 'A';
		if(mode == 'b') 	mode = 'B';
		if(mode == 'm') 	mode = 'M';
		if(mode!='A' && mode!='M'&& mode!='B'&& mode!=0) return PICC_ParameterErr;
	
		Card_Type_ABM=mode;
	
	//	while(count--)
			{
			iret = PICC_POLLING2(count);
			if (iret == PICC_KeyCancel)
				return PICC_KeyCancel;
			if(PICC_OK!=iret)
			{
				return PICC_CardStateErr;
			}	
			iret=PICC_AntiColl();
			if(iret != PICC_OK){
				PICC_RESET2();
				printf("%d\r\n",iret);
	//			continue;
				return PICC_AntiCollErr;
			}
			
			//activate picc
			if(TYPE_A){
				iret=PICC_WUPA_RATS();
			}
			else{
				iret=PICC_WUPB_ATTRIB();
			}
			if(iret != PICC_OK){
				PICC_RESET2();
				printf("%d\r\n",iret);
				return PICC_AttribErr;
			}
			//process picc
			return PICC_OK;
	}

}



int Picc_Check2(uchar mode,uchar *CardType,uchar *SerialNo,uint count)

{
	int iret;

	if(0==OPENED_FLAG){
		return PICC_NotOpen;
	}

	if(mode == 0x0a)    mode = 'A';
	if(mode == 0x0b)    mode = 'B';
	if(mode == 'a')     mode = 'A';
	if(mode == 'b')     mode = 'B';
	if(mode == 'm')     mode = 'M';
	if(mode!='A' && mode!='M'&& mode!='B'&& mode!=0) return PICC_ParameterErr;

	Card_Type_ABM=mode;

//	while(count--)
		{
		iret = PICC_POLLING2(count);
		if (iret == PICC_KeyCancel)
			return PICC_KeyCancel;
		if(PICC_OK!=iret)
		{
			return PICC_CardStateErr;
		}	
		iret=PICC_AntiColl();
		if(iret != PICC_OK){
			PICC_RESET2();
			printf("%d\r\n",iret);
//			continue;
			return PICC_AntiCollErr;
		}
		
		//activate picc
		if(TYPE_A){
			iret=PICC_WUPA_RATS();
		}
		else{
			iret=PICC_WUPB_ATTRIB();
		}
		if(iret != PICC_OK){
			PICC_RESET2();
			printf("%d\r\n",iret);
			return PICC_AttribErr;
		}
		//process picc
		return PICC_OK;

		//remove picc

		//reset <t(reset)
		
		//wait <t(pause)

	}
//		return PICC_CardStateErr;
	
}

int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
{
  return Lib_PiccCheck1(mode,CardType,SerialNo,3);
}  

int Lib_PiccRemove(void)
{
if(0==OPENED_FLAG){
	return PICC_NotOpen;
}
	Picc_Close2();
	return 0;
}


/*************************************************************************************
功能      : 按指定的模式搜寻PICC卡片,搜到卡片后,将其选中和激活.

mode         : －输入参数;用于指定卡片搜寻模式;
'a'或'A' -- 只搜寻A型卡一次;感应区内不允许有多于1张的A型卡
'b'或'B' -- 只搜寻B型卡一次;感应区内不允许有多于1张的B型卡
'm'或'M' -- 只搜寻M1卡一次;感应区内不允许有多于1张的M1卡
其它值-- 保留

CardType    : －输入参数; 返回卡类型

SerialNo     : －输出参数; 指向存取卡片序列号信息的缓冲区首址
该信息依次包含了序列号长度和序列号内容等两项内容.
B型卡和M1卡的序列号均为4字节;
A型卡的序列号一般为4字节,也有7字节或10字节的.
采用字节SerialNo[0]指示序列号的长度,
SerialNo[1~9]保存序列号(左对齐).
若需要读取序列号、且是A型卡,则要使用和判断长度字节
*************************************************************************************/
int Picc_Check_old(uchar mode,uchar *CardType,uchar *SerialNo)
{

	int iret; 
	uchar type[16],snnum[128];


	g_HaveACardFlag = NO_CARD;
	g_HaveBCardFlag = NO_CARD;
	memset(type,0x00,sizeof(type));
	memset(snnum,0x00,sizeof(snnum));
	if(!pcd_info.open) return PICC_NotOpen;//not powered on     

	if(mode == 0x0a)    mode = 'A';
	if(mode == 0x0b)    mode = 'B';
	if(mode == 'a')     mode = 'A';
	if(mode == 'b')     mode = 'B';
	if(mode == 'm')     mode = 'M';
	if(mode!='A' && mode!='M'&& mode!='B'&& mode!=0) return PICC_ParameterErr;
	switch(mode)
	{
	case  'B':
		g_PiccCheckCardMode = CHECK_B_CARD;
		iret=s_PiccDetectTypeB(mode,CardType,SerialNo);
		break;
	case 'A':
	case 'M':
		g_PiccCheckCardMode = CHECK_A_CARD;
		iret=s_PiccDetectTypeA(mode,CardType,SerialNo);
		break;
	case  0:
		g_PiccCheckCardMode = CHECK_ONE_CARD;
		iret=s_PiccDetectTypeA('A',type,snnum);
		if(iret==PICC_Collision)  break;   //多张A卡
		if(iret==0)  //有A卡,寻B卡
		{
			printf("Have A card.\r\n");
			Lib_PiccHalt();
			iret=s_PiccDetectTypeB('B',type,snnum);

			if (g_HaveACardFlag==HAVE_CARD && g_HaveBCardFlag==HAVE_CARD)	
			{//当发送A和B类卡唤醒命令的时候，如果A和B类卡都有正确的返回就认为感应区中有A和B两张卡存在
				printf("Have A and B cards!\r\n");
				return PICC_Collision;
			}

			if(iret==0)  
			{
				printf("Have B card.\r\n");
				return PICC_Collision;  //寻到B卡
			}
			else   //无B卡
			{
				iret=s_PiccDetectTypeA('A',CardType,SerialNo);
				return iret; 		
			}
		}
		else   //无A卡,寻B卡
		{
			printf("No A card, find B card\r\n");
			iret=s_PiccDetectTypeB('B',CardType,SerialNo);
			return iret;  //寻到B卡			
		}
		break;
	default : break;
	}
	return iret;

} 

int Lib_PiccCommand(APDU_SEND *ApduSend, APDU_RESP *ApduResp) 
{

	uchar cid,wait_mode,tx_pool[PICC_TXPOOL_SIZE],rx_pool[PICC_RXPOOL_SIZE];
	uchar tx_chained,rx_chained,cc,pcb,blk_type,blk_no,d_offset;
	ushort i,bsize,bn,tlen,dn,sn,nak_retries,tx_retries,wtx_retries;

	uchar SendCmd[PICC_TXPOOL_SIZE]; 
	ushort len;

	uchar LastRx_chain=0;
	uchar e_tx_retries=0;
	uchar I_tx_retries=0;

	int iret;    

	if((ApduSend==NULL)||(ApduResp==NULL))
		return PICC_ParameterErr;
	//将ApduSend中Command字段拷到发送命令缓冲区中 
	len=0;
	memset(SendCmd,0,sizeof(SendCmd));
	memcpy(SendCmd,ApduSend->Command,4);
	len=len+4;
	//将ApduSend中Lc和Le字段拷到发送命令缓冲区中 

	//Lib_DelayMs(10);		//delay >2MS is ok, 两个COMMAND命令的间隔，
	if((ApduSend->Lc==0)&&(ApduSend->Le==0))
	{
		SendCmd[len++]=0x00;
	}         
	if(ApduSend->Lc)
	{
		if(ApduSend->Lc>250)    return PICC_ParameterErr;
		else
		{
			SendCmd[len++]=(unsigned char)(ApduSend->Lc);
			memcpy(&SendCmd[len],ApduSend->DataIn,(unsigned char)ApduSend->Lc);                  
			len+=(unsigned char)(ApduSend->Lc);
		}              
	}
	if(ApduSend->Le)
	{              
		if(ApduSend->Le>=256)    SendCmd[len++]=0x00;
		else                     SendCmd[len++]=(unsigned char)(ApduSend->Le);
	}         

	cid=0;
	//tlen为SendCmd中有效数据的长度
	tlen=len;
	if(!tlen)           return PICC_ParameterErr;

	if(!pcd_info.open)  return PICC_NotOpen;//not powered on
	//type-A中没有使用max_buffer_index变量
	if(picc_info[cid].max_buffer_index)
	{
		//如果命令长度大于max_buffer_size,出错
		bn=picc_info[cid].max_buffer_index-1;
		for(dn=1,i=0;i<bn;i++)dn*=2;
		bn=picc_info[cid].max_frame_size*dn;

		if(tlen>bn)    return PICC_ParameterErr;//too long data
	}
	//max_frame_size表示PCD能够发送的最大帧长度,也即卡片能够接收的最大帧长度
	//s_UartPrint(COM_DEBUG,"TLEN:%d ",tlen);
	//	bsize=picc_info[cid].max_frame_size-3;//excluding CRC,PCB,CID/NAD(optional)
	
	{
		const ushort fsc_table[]={16,24,32,40,48,64,96,128,256};
		uchar i;
		if(TYPE_A){						//FSC
			i=picc_info[0].ATS[1]&0x0f;
			if(i>8)
				i=8;
			bsize=fsc_table[i]-3;//excluding CRC,PCB,CID/NAD(optional)
		}
		else{
			i=picc_info[0].ATQB[10]>>4;
			if(i>8)
				i=8;
			bsize=fsc_table[i]-3;//excluding CRC,PCB,CID/NAD(optional)
		}
	}
	//bsize=6;//for debug only
	bn=tlen/bsize;
	if(tlen%bsize)bn++;
	//此时bn表示PCD发送的数据帧的个数
	//Lib_DelayMs(2);		//delay >2MS is ok, 两个COMMAND命令的间隔，

	//--perform sending loop
	wait_mode=0x10+cid;//low 4 bits are CID
	picc_info[cid].delay_multiple=0;//reset WTX
	for(i=0;i<bn;i++)
	{
		nak_retries=0;
		tx_retries=0;
		I_tx_retries=0;

TX_RETRY:
		I_tx_retries++;
        printf("%d:%d\r\n",__LINE__,I_tx_retries);
		
		if(I_tx_retries>3){		//ta401.15
			return PICC_ProtocolErr;
		}
		if(i==bn-1)
		{
			dn=tlen-bsize*i;
			tx_chained=0;
		}
		else
		{
			dn=bsize;
			if(bn>1)tx_chained=1;
			else tx_chained=0;
		}

		//PCB--D7D6(00:I-Block),D5(0),D4:chain,D3:CID,D2:NAD,D1(1),D0:blk_no
		tx_pool[0]=0x02+picc_info[cid].blk_no;
		if(tx_chained)tx_pool[0]|=0x10;
		//if(picc_info[cid].cid_enable)tx_pool[0]|=0x08;
		memcpy(tx_pool+1,SendCmd+i*bsize,dn);
		dn+=1;//including PCB,CID/NAD(optional)

		wtx_retries=0;
		sn=dn;
		e_tx_retries=0;
TX_BEGIN: 

	//	Lib_DelayMs(8);//1->5->10ms   about 20etu    

		//s_UartPrint(COM_DEBUG, "s1:pcb=%02x %d\r\n", tx_pool[0],dn);
		printf("s1:pcb=%02x %d\r\n", tx_pool[0],dn);
	
		g_CommandFlag=1;
		iret=s_PiccCmdExchange(0,wait_mode,tx_pool,&dn);
        g_CommandFlag=0;
		//cc=s_ReadRegRc523(rCommandReg);
		//s_UartPrint(COM_DEBUG,"A:%d,%02X-%02X TN:%d ",
		//s_UartPrint(COM_DEBUG,"A:%d,%02X-%02X SN:%d ",
		//             dn,cc,tmpc,sn);
		//--process the RBLOCK chain response

		if(g_TA430_FLAG == 1){
			
			dn=21;
			memcpy(tx_pool,"\x02\x00\xA4\x04\x00\x0C\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x00\x90\x00",21);
			
			iret=PICC_OK;
			g_TA430_FLAG++;
		
		}
		else if(g_TA430_FLAG == 2){
	
			dn=21;//0300A404000C9192939495969798999A9B9C009000
			memcpy(tx_pool,"\x03\x00\xA4\x04\x00\x0C\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x00\x90\x00",21);
			
			iret=PICC_OK;
			g_TA430_FLAG++;

		}

		pcb=tx_pool[0];
		blk_type=pcb>>6;
		blk_no=pcb&0x01;
		trace_debug_printf( "r1:pcb=%02x dn=%d  %02x %d\r\n", tx_pool[0],dn,blk_type,blk_no);


		if(!iret)
		{
			/*
			WTXM=1;
			picc_info[cid].delay_multiple=0;   //reset WTX

			if(!dn)                            return PICC_ApduErr;
			if(blk_type==0x01)                 return PICC_ApduErr;//undefined block type
			if(tx_chained && !blk_type)        return PICC_ApduErr;//I block received for chain req
			//if(!tx_chained && blk_type==2)     return PICC_ApduErr;//R block received for no-chain req
			if(blk_type==3 && (pcb&0x30)!=0x30)return PICC_ApduErr;//invalid S block
			*/
			WTXM=1;
			picc_info[cid].delay_multiple=0;   //reset WTX

			if(!dn)                            return PICC_ApduErr1;
			if(blk_type==0x01)                 return PICC_ProtocolErr;//undefined block type
			if(tx_chained && !blk_type)        return PICC_ApduErr2;//I block received for chain req
			if((!tx_chained && blk_type==2)&&(picc_TimeoutFlag==0))     return PICC_ApduErr3;//R block received for no-chain req
//				picc_TimeoutFlag=0;
			if(blk_type==3 && (pcb&0x30)!=0x30)return PICC_ApduErr4;//invalid S block
			if(dn>FSDmin)return PICC_ProtocolErr;//invalid S block
			
			if(blk_type==0x00) {
				if((pcb&(1<<1))!=(1<<1)){
					return PICC_ProtocolErr;
				}
			}
			
			if(blk_type==0x10) {
				if((pcb&((1<<5)|(1<<2)))!=((1<<5)|(0<<2))){
					return PICC_ProtocolErr;
				}
			}
			
			if(blk_type==0x11) {
				if((pcb&(1<<2))!=(0<<2)){
					return PICC_ProtocolErr;
				}
			}
		}
		else//invalid blocks,tx error, or timeout
		{/*
			//s_UartPrint(COM_DEBUG, "invalid blocks=%d\r\n", iret);
			wtx_retries=0;

			//if(!tx_chained)
			//{
			//     tx_retries++;
			//     if(tx_retries>=3)   return PICC_ApduErr;
			//     Lib_DelayMs(10);//10ms
			//     goto TX_RETRY;
			//}

			nak_retries++;
			if(nak_retries>3)        return PICC_ApduErr;

			//--fill the R_NAK pack
			tx_pool[0]=0xb2+picc_info[cid].blk_no;//R_NAK
			//if(picc_info[cid].cid_enable)tx_pool[0]|=0x08;
			tx_pool[1]=cid;
			dn=1;

			goto TX_BEGIN;*/

			//s_UartPrint(COM_DEBUG, "invalid blocks=%d\r\n", iret);
			wtx_retries=0;

			//if(!tx_chained)
			//{
			//     tx_retries++;
			//     if(tx_retries>=3)   return PICC_ApduErr;
			//     Lib_DelayMs(10);//10ms
			//     goto TX_RETRY;
			//}

			if(iret==PICC_RxTimerOut){	
				picc_TimeoutFlag=1;

#if 0			//?	
				ssssss=1;

#endif

				if(wtx_retries>3)	return PICC_ApduErr23;


				
				if(LastRx_chain){			//13.3.5.8
					e_tx_retries++;
					if(e_tx_retries>3)	
						return PICC_RxTimerOut;

	//				goto TX_ACK;
				}
				else{					//13.3.5.5
					e_tx_retries++;
					if(e_tx_retries>2)	
						return PICC_FastOut;

	//				goto TX_ACK;					
				}
			}else{
/*			
					e_tx_retries++;
					if(e_tx_retries>3)	
						return PICC_FastOut;
*/
/*			
if(){
			nak_retries++;
			if(nak_retries>3)        return PICC_ApduErr;
}		*/
			nak_retries++;
			if(nak_retries>2)        return PICC_ApduErr5;	//ta403.1
			}

			//--fill the R_NAK pack
			tx_pool[0]=0xb2+picc_info[cid].blk_no;//R_NAK
			//if(picc_info[cid].cid_enable)tx_pool[0]|=0x08;
			tx_pool[1]=cid;
			dn=1;

printf("%d:%d\r\n",__LINE__,iret);

			goto TX_BEGIN;
		}

		if(!blk_type)  break;
		else if(blk_type==2)//R_block
		{

			trace_debug_printf( "blkno=%d %d %d\r\n", blk_no,picc_info[cid].blk_no,nak_retries);

			if(pcb&(1<<4)){	//R(NAK)
					return PICC_ProtocolErr;
			}
			
			if(0==(pcb&(1<<5))){	//R()
					return PICC_ProtocolErr;
			}
			
			if((pcb&(1<<3))){	//R()
					return PICC_ProtocolErr;
			}
			
			if((pcb&(1<<2))){	//R()
					return PICC_ProtocolErr;
			}

			if(blk_no!=picc_info[cid].blk_no)
			{
				tx_retries++;
				if(tx_retries>3)    return PICC_ApduErr6;
				goto TX_RETRY;
			}

			picc_TimeoutFlag=0;

			if(nak_retries && !tx_chained)    //add 2008/6/24 for TYPEB MOBILE
			{//无链的情形下收到一个ACK包，且上次发送的为R包则须重发I包
				nak_retries=0;
				tx_retries++;
				if(tx_retries>3)    return PICC_ApduErr22;
				goto TX_RETRY;
			}

			picc_info[cid].blk_no=(blk_no+1)%2;
		}
		else if(blk_type==3)//S_block(WTX) during TX
		{
			if(tx_pool[0]==0)
				return PICC_ProtocolErr;
			
			if((pcb&(1<<2)))
				return PICC_ProtocolErr;
			
			if((pcb&(1<<3)))
				return PICC_ProtocolErr;
			
			wtx_retries++;
		//	if(wtx_retries>3)   return PICC_ApduErr;

			d_offset=1;
			if(pcb&0x08)d_offset++;//cid follows
			if(dn<=d_offset)    return PICC_ApduErr21;

			cc=tx_pool[d_offset]&0x3f;
			if(!cc)             return PICC_ApduErr20;//WTX cannot be 0
			if(cc>=59)cc=59;
			picc_info[cid].delay_multiple=cc;

			//--fill the S_WTX response pack
			tx_pool[d_offset]&=0x3f;

			WTXM=tx_pool[d_offset]&0x3f;
			if(WTXM>59){
				WTXM=59;
			}
            printf("%d:%d\r\n",__LINE__,WTXM);

			goto TX_BEGIN;
		}
		else      return PICC_ApduErr8;
	}//for(i)

	picc_TimeoutFlag=0;


	//--perform receiving loop
	i=0;//reset receive pointer
	while(2)
	{
		tx_retries=0;
		//wtx_retries=0;
		if(blk_type)   return PICC_ApduErr9;
		if(dn<2)       return PICC_ApduErr10;

		rx_chained=pcb&0x10;
		LastRx_chain=pcb&0x10;

		d_offset=1;
		if(pcb&0x08)d_offset++;//cid follows
		if(pcb&0x04)d_offset++;//nad follows
		if(dn<=d_offset)return PICC_ApduErr11;

		if(dn>(FSDmin-1)){			/////////////////////////////256?  255? ta404.4
//printfx("%d:%d\r\n",__LINE__,iret);
			return PICC_Fsderror;
		}

		if(blk_no==picc_info[cid].blk_no)
		{
			//正确流程分支 
			picc_info[cid].blk_no=(blk_no+1)%2;
		}
		else
		{
			//错误流程分支 
			return PICC_ApduErr12;


			tx_retries++;
			if(tx_retries>=2)   return PICC_ApduErr13;
			tx_pool[0]=0xb2+picc_info[cid].blk_no;//R_NAK
			tx_pool[1]=cid;
			dn=1;
			printfx("%d:%d\r\n",__LINE__,iret);
			goto RX_BEGIN;
		}
		//将I block中的数据拷贝到rx_pool中 
		dn-=d_offset;
		if(i+dn>PICC_RXPOOL_SIZE)    return PICC_ApduErr14;//rx buffer overflows
		memcpy(rx_pool+i,tx_pool+d_offset,dn);
		i+=dn;

		if(!rx_chained)break;

		//tx_retries=0;
TX_ACK:
		tx_pool[0]=0xa2+picc_info[cid].blk_no;//R_ACK
		//if(picc_info[cid].cid_enable)tx_pool[0]|=0x08;
		tx_pool[1]=cid;
		dn=1;

		wtx_retries=0;
RX_BEGIN:
		trace_debug_printf( "s2:pcb=%02x %d\r\n", tx_pool[0],dn);

//		Lib_DelayMs(8);//1->5->10ms   about 20-50etu

g_CommandFlag=1;
		iret=s_PiccCmdExchange(0,wait_mode,tx_pool,&dn);
g_CommandFlag=0;

		if(iret)
		{
			trace_debug_printf( "r2:err=%d\r\n", iret);

		/*	tx_retries++;
			if(tx_retries>=3)   return PICC_ApduErr;

			goto TX_ACK;*/

			if(iret==PICC_RxTimerOut){	
				
				if(wtx_retries>3)	return PICC_ApduErr24;
				
				if(LastRx_chain){			//13.3.5.8
					tx_retries++;
					if(tx_retries>=3)   
						return PICC_RxTimerOut;

					goto TX_ACK;
				}
				else{					//13.3.5.5
					tx_retries++;
					if(tx_retries>=3)   
						return PICC_RxTimerOut;

					goto TX_ACK;					
				}
			}

			tx_retries++;
			if(tx_retries>=3)   return PICC_ApduErr15;

			goto TX_ACK;
		}

		tx_retries=0;
		WTXM=1;


		pcb=tx_pool[0];
		blk_type=pcb>>6;
		blk_no=pcb&0x01;
		picc_info[cid].delay_multiple=0;//reset WTX

		trace_debug_printf( "r2:pcb=%02x dn=%d  %02x %d\r\n", tx_pool[0],dn,blk_type,blk_no);

        if(blk_type==0){	//I
			LastRx_chain=pcb&0x10;
		}
		else if(blk_type==2){	//R
//			LastRx_chain=0;

		}
		else
		if((pcb&0xf0)==0xf0)//S_block(WTX) during RX
		{
			wtx_retries++;
		//	if(wtx_retries>3)   return PICC_ApduErr;

			d_offset=1;
			if(pcb&0x08)d_offset++;//cid follows
			if(dn<=d_offset)    return PICC_ApduErr17;

			cc=tx_pool[d_offset]&0x3f;
			if(!cc)             return PICC_ApduErr18;//WTX cannot be 0
			if(cc>=59)cc=59;
			picc_info[cid].delay_multiple=cc;

			//--fill the S_WTX response pack
			tx_pool[d_offset]&=0x3f;

			WTXM=tx_pool[d_offset]&0x3f;
			if(WTXM>59){
				WTXM=59;
			}
			goto RX_BEGIN;
		}//if(S_WTX)
	}//while(2)


	if(i>=2)
	{
		ApduResp->LenOut=i-2;
		memcpy(ApduResp->DataOut,rx_pool,ApduResp->LenOut);
		ApduResp->SWA=rx_pool[i-2];
		ApduResp->SWB=rx_pool[i-1];
		return 0;
	}
	return PICC_ApduErr19;   

}

int analysis_requestcmd(uchar action,uchar *birqen,uchar *bdiven,uchar *bwaitfor,uchar *bneedrecv)
{
	switch(action)
	{
	case PCD_IDLE: 
		*birqen   = 0x00;
		*bwaitfor = 0x00;

		*bdiven   = 0x00;
		break;    
		//case PCD_WRITEE2:
		//	*birqen   = 0x11;
		//	*bwaitfor = 0x10; 
		//	*bdiven   = 0x00; 
		//	break;
		//case PCD_READE2:
		//	*birqen   = 0x07;
		//	*bwaitfor = 0x04; 
		//	*bdiven   = 0x00; 
		//	*bneedrecv=1;
		//	break;          
		//case PCD_LOADCONFIG :
		//case PCD_LOADKEYE2:           
	case PCD_AUTHENT:
		*birqen   = 0x05;
		*bwaitfor = 0x04;

		*bdiven   = 0x00;

		break;       
	case PCD_CALCCRC:
		*birqen   = 0x11;
		*bwaitfor = 0x10;

		*bdiven   = 0x00;

		break;
		//case PCD_AUTHENT2:
		//	*birqen   = 0x04;
		//	*bwaitfor = 0x04; 
		//	*bdiven   = 0x00; 
		//	break;
	case PCD_RECEIVE:
		*birqen   = 0x06;
		*bwaitfor = 0x04;
		*bneedrecv=1;

		*bdiven   = 0x00;

		break;
		//case PCD_LOADKEY:
		//	*birqen   = 0x05;
		//	*bwaitfor = 0x04; 
		//	*bdiven   = 0x00; 
		//	break;
	case PCD_TRANSMIT:
		*birqen   = 0x05;
		*bwaitfor = 0x04; 
		*bdiven   = 0x00; 
		break;
	case PCD_TRANSCEIVE:
		*birqen   = 0x77;//3d    
		*bwaitfor = 0x10;//04
		*bneedrecv=1; 
		*bdiven   = 0x00; 
		break;
	default:
		//     action=MI_UNKNOWN_COMMAND;
		break;      
	}
	return 0;     
}

uint exp2_(uchar en)
{
     switch(en){
     case 0:  return 1;
     case 1:  return 2;
     case 2:  return 4;
     case 3:  return 8;
     case 4:  return 16;
     case 5:  return 32;
     case 6:  return 64;
     case 7:  return 128;
     case 8:  return 256;
     case 9:  return 512;
     case 10:  return 1024;
     case 11:  return 2048;
     case 12:  return 4096;
     case 13:  return 8192;
     case 14:  return 16384;
     }
     return 0xff;
}

/****************************************************************************
功能描述:   

*****************************************************************************/
int s_PiccCmdExchange(uchar action,uchar wait_mode,uchar *blk,ushort *tx_rx_len)
{
	uchar tmpc,cid,rx_pool[256],cc;
	uchar irqEn,divEn,waitFor,lastBits,needRecv;
	ushort tx_len,i,n;
	int iresp,j;   
	ushort dn,cur_pn;     
	//uchar myTemp;   
	uchar FWI;
	uchar SFGI;
	int k;
	
	needRecv=0;   //所需要接收标志1=需要接收,0=不需要接收
	irqEn=0x00;    //使能和禁止中断请求通过的控制位
	divEn=0x00;
	waitFor=0x00;
	iresp=0;
	tx_len=*tx_rx_len;  //接收数据的长度
	//	if(!tx_len && action!=PCD_AUTHENT2) return PICC_ParameterErr;  
	if(!tx_len && action!=PCD_AUTHENT) return PICC_ParameterErr;  

	if(!action) action=PCD_TRANSCEIVE;

	analysis_requestcmd(action,&irqEn,&divEn,&waitFor,&needRecv); 


	s_WriteRegRc523(rCommandReg,0x00);//clear command


	s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 


	s_WriteRegRc523(rFIFOLevelReg,0x80);
	s_SetBitRegRc523(rControlReg ,0x80);



	if(1==picc_info[0].SFGT_need){
		
			if(TYPE_A){
				SFGI=picc_info[0].TB&0x0f;
			}
			else{
				SFGI=picc_info[0].ATQB[9+3]>>4;
			}
			

		DelayMs((((((Xsfgt+256*16)*exp2_(SFGI))/(13560)))>5)?(((Xsfgt+256*16)*exp2_(SFGI))/(13560)-5):((((Xsfgt+256*16)*exp2_(SFGI))/(13560))));
		picc_info[0].SFGT_need=0;
	}else{
		Lib_DelayMs(1);
	}
	
	g_PiccWtxCount=0;

	if(!wait_mode)	
		s_WriteRegRc523(rModeReg ,0x80);    //timer cycle T=2048/6.78us  


	if(g_FDT_Flag==FDT_TYPE_0){
										//(n*128+20)/fc  (n*128+84)/fc   86.67~91.15
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_WriteRegRc523(rTPrescalerReg ,0x00);	   
		s_WriteRegRc523(rTReloadRegH ,(9*128+84)/0x100);	
		s_WriteRegRc523(rTReloadRegL ,(9*128+84)%0x100); 

	}
	else if((g_FDT_Flag==FDT_TYPE_1)||(g_FDT_Flag==FDT_TYPE_3)) {

		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_SetBitRegRc523(rTModeReg, 0x03);
		s_WriteRegRc523(rTPrescalerReg ,0xff);	   

		if(TYPE_A){
			FWI=picc_info[0].TB>>4;
		}
		else{
			FWI=picc_info[0].ATQB[9+2]>>4;
		}

		if(FWI>14)
			FWI=14;

		s_WriteRegRc523(rTReloadRegH ,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1))/0x100);	
		s_WriteRegRc523(rTReloadRegL ,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1))%0x100+1); 
		
		printf("xxx:%d %x %x %d\r\n",FWI,s_ReadRegRc523(rTReloadRegH ),s_ReadRegRc523(rTReloadRegL ),WTXM);

	}
	else if(g_FDT_Flag==FDT_TYPE_2) {
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_SetBitRegRc523(rTModeReg, 0x03);
		s_WriteRegRc523(rTPrescalerReg ,0xff);	   
		
		s_WriteRegRc523(rTReloadRegH ,((FWTactivation+128*3)/(0x3ff*2+1))/0x100);
		s_WriteRegRc523(rTReloadRegL ,((FWTactivation+128*3)/(0x3ff*2+1))%0x100); 
		
		printfx("xxx:%d %x %x %d\r\n",FWI,s_ReadRegRc523(rTReloadRegH ),s_ReadRegRc523(rTReloadRegL ),WTXM);
	}
	else if(g_FDT_Flag==FDT_TYPE_4) {
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_SetBitRegRc523(rTModeReg, 0x00);
		s_WriteRegRc523(rTPrescalerReg ,0x00);	   
		
		s_WriteRegRc523(rTReloadRegH ,((FWTatqb+128*3))/0x100);
		s_WriteRegRc523(rTReloadRegL ,((FWTatqb+128*3))%0x100); 
	}
	else
	{ 
		cid=wait_mode&0x0f;
		i=picc_info[cid].max_delay_index;
		//s_WriteRegRc523(rTimerClock,tab_cycle_count[i][0]);
		//s_WriteRegRc523(rTimerReload,tab_cycle_count[i][1]); 
		s_WriteRegRc523(rModeReg ,0x80);    //
		s_SetBitRegRc523(rTModeReg, 0x01);		// 0x01ff, 每一节拍： 512/6.78us = 75.5us
		s_WriteRegRc523(rTPrescalerReg ,0xff);   
		s_WriteRegRc523(rTReloadRegH ,tab_cycle_count[i][0]);
		s_WriteRegRc523(rTReloadRegL ,tab_cycle_count[i][1]);
		//s_WriteRegRc523(rTReloadRegL ,tab_cycle_count[i][1]);

		g_PiccWtxCount=picc_info[cid].delay_multiple;
		if(g_PiccWtxCount>tab_max_wtx[i])g_PiccWtxCount=tab_max_wtx[i];
	}

	printf("Send:");
	cur_pn=(tx_len<=DEF_FIFO_LEVEL)?tx_len:DEF_FIFO_LEVEL;
	for(i=0;i<cur_pn;i++) {
		s_WriteRegRc523(rFIFODataReg,blk[i]); 
		printf("%02x ", blk[i]);
	}
	printf("\r\n");
	dn=s_ReadRegRc523(rFIFOLevelReg);//FIFO empty length
 
	if(dn!=cur_pn)
	{
		printf("WRITE ERROR:%d-%d \r\n",dn,cur_pn);
	} 

	s_SetTimer(PICC_TIMER, 50); 

	
	s_WriteRegRc523(rCommandReg,action); 
	if(action== PCD_TRANSCEIVE) 
		s_SetBitRegRc523(rBitFramingReg,0x80);

	//send the remaining packs
	for(i=cur_pn;i<tx_len;)
	{
		while(1)
		{
			//check if byte count of FIFO reaches the low threshold
			tmpc=s_ReadRegRc523(rStatus1Reg);
			if(tmpc&0x01) break;
			if(!s_CheckTimer(PICC_TIMER))
			{
				s_WriteRegRc523(rComIEnReg,0x80);//close all pcd interrupts
				s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
				//s_WriteRegRc523(rDivIEnReg,0x14);//close all pcd interrupts
				s_WriteRegRc523(rDivIEnReg,0x00);//close all pcd interrupts
				s_WriteRegRc523(rDivIrqReg,0x14);//clear all pcd interrupts 
				printf("TX Timeout....\r\n");
				return PICC_TxTimerOut;
			}            
		}
		dn=DEF_FIFO_LEVEL-s_ReadRegRc523(rFIFOLevelReg);//FIFO empty length
		cur_pn=((tx_len-i)<=DEF_FIFO_LEVEL)?(tx_len-i):dn;
		for(j=0;j<cur_pn;j++)  s_WriteRegRc523(rFIFODataReg,blk[i+j]);
		i+=cur_pn;
	}

	i=0;

Rx_again:


	irqEn   |= 0x01;
	waitFor |= 0x01; 
	while(1)
	{
		cc=s_ReadRegRc523(rComIrqReg);
		if(cc&0x30){
			printfx("%d:%02x\r\n",__LINE__,cc);
				break;
		}
		if(cc&0x02)
		{
			printfx("%d:%02x\r\n",__LINE__,cc);
			if(g_WakeUpFlag==1)
				iresp=PICC_TypeAColl;
			else{
				iresp=PICC_RxErr;
			if(1)
			{
				uchar i=200;
				uchar ret;
				while(i--){
					ret=s_ReadRegRc523(rStatus2Reg);
					printfx("error:%d: %d %02x\r\n",__LINE__,i,ret);
					if(0x06!=(ret&0x07)){
						break;
					}
				}
				if(i==255){
					printfx("error:%d:%02x\r\n",__LINE__,s_ReadRegRc523(rStatus2Reg));
				}
			}

			}
			goto X_TAIL;
		} 
		
		if(cc&0x01){
			uchar bb;
			bb=s_ReadRegRc523(rStatus2Reg);
			if((bb&0x07)==0x05){
				printf("Rx timeout.\r\n");
				iresp=PICC_RxTimerOut;
				goto X_TAIL;
			} 
			else{
				if(g_WakeUpAFlag==1){
					iresp=PICC_TypeAColl;
					goto X_TAIL;
				}
			
				if(g_WakeUpBFlag==1){
					iresp=PICC_TypeAColl;
					printfx("%d:%02x\r\n",__LINE__,bb);
					goto X_TAIL;
				}
				printfx("%d:%02x\r\n",__LINE__,bb);
			}
		}
		if(cc&0x08){	//hialert
			cc=s_ReadRegRc523(rFIFOLevelReg);

			if(cc<DEF_FIFO_LEVEL)
			{
				iresp=PICC_RxDataOver;
				printf("Rx Data over...111\r\n");
				goto X_TAIL;
			}
	 
			
			n=(cc-DEF_FIFO_LEVEL)+i;

			if(n>256){	//FSDmin 256
				iresp=PICC_RxOverFlow;
				goto X_TAIL;
			}
			
			for(;i<n;i++)
				rx_pool[i]=s_ReadRegRc523(rFIFODataReg);
			
		}
	}

	tmpc=s_ReadRegRc523(rErrorReg);
	iresp=0;        
	if(needRecv)
	{        
		n=cc=s_ReadRegRc523(rFIFOLevelReg);
		if(cc>64)
		{
			iresp=PICC_RxDataOver;
			//printf("Rx Data over...\r\n");
			goto X_TAIL;
		}
		n=i+n;


		if(n>256){	//FSDmin 256
			iresp=PICC_RxOverFlow;
			goto X_TAIL;
		}

		
		for(;i<n;i++)
		{
			rx_pool[i]=s_ReadRegRc523(rFIFODataReg);
			printf("%02x ", rx_pool[i]);
		}
		printf("\r\n");
	}

	if(tmpc&0x08)
	{
		iresp=PICC_TypeAColl;
		printfx("Type A coll... %02x %d\r\n",tmpc,i);

		goto X_TAIL;
	}//bit collision detected for type A  4
	if(tmpc&0x10)
	{
		iresp=PICC_FifoOver;
		printf("Fifo over...\r\n");

		goto X_TAIL;
	}//FIFO overflow  5
	if(tmpc&0x04)
	{
		iresp=PICC_CRCErr;
		printfx("CRC err...  %02x %d\r\n",tmpc,i);
		if(i==0)
			iresp=PICC_CRCErr2;
		
		if((g_ActiveBFlag==1)&&(tmpc&0x01))
			iresp=PICC_SOFErr;
	
		goto X_TAIL;
	}//CRC error  6
	if(g_ActiveBFlag==1) {
		if(tmpc&0x01)
		{
			iresp=PICC_SOFErr;
	//		printfx("SOF err...[%02x] %d\r\n", tmpc,i);
			printfx("SOF err...[%02x] %d %02x %02x\r\n", tmpc,i,s_ReadRegRc523(rControlReg),s_ReadRegRc523(rStatus2Reg));
			goto X_TAIL;
		}//SOF incorrect  7
	}
	if(tmpc&0x02)
	{
		iresp=PICC_ParityErr;
		printf("Parity err...\r\n");
		goto X_TAIL;
	}


X_TAIL:
	*tx_rx_len=i;
	if(i)
		memcpy(blk,rx_pool,i);
	printf("Recv:");
	for (k=0; k<*tx_rx_len; k++) printf("%02x ", rx_pool[k]);
	printf("\r\n");
	if((iresp!=PICC_OK)&&(iresp!=PICC_RxTimerOut)&&((TYPE_A==1)
		||((iresp!=PICC_CRCErr)&&(iresp!=PICC_CRCErr2)))) {
		if(g_ActiveFlag==1){
			int ret_p;
			
			s_SetBitRegRc523(rControlReg ,0x80);
			printf("%d:%d %d %02x %02x %02x\r\n",__LINE__,iresp,i,s_ReadRegRc523(rControlReg),s_ReadRegRc523(rErrorReg),s_ReadRegRc523(rFIFOLevelReg));

			if((i>=4)||(s_ReadRegRc523(rFIFOLevelReg)>=4)){	//rx_len>=4
				ret_p=s_ReadRegRc523(rControlReg);
				if((ret_p&0x07)==0){	//LastBit 8.
					ret_p=s_ReadRegRc523(rErrorReg);
					if(ret_p&((1<<2)|(1<<1))){	//
						printfx("%d:%d\r\n",__LINE__,iresp);
						goto out;
					}

				}
			}

			s_WriteRegRc523(rCommandReg,0x00);//clear command
			s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
			s_WriteRegRc523(rFIFOLevelReg,0x80);
			s_SetBitRegRc523(rControlReg ,0x60);
			s_WriteRegRc523(rCommandReg,PCD_RECEIVE); 
	
			goto Rx_again;
		}
	}
	if(g_CommandFlag==1) {
		if((iresp!=PICC_OK)&&(iresp!=PICC_RxTimerOut))
		{
			
			int ret_p;

			s_SetBitRegRc523(rControlReg ,0x80);
			
			if(1){										//Not frame Error.
				ret_p=s_ReadRegRc523(rErrorReg);
				if(ret_p&((1<<2)|(1<<1))){	//crc or parity
					if(i>=4){
						ret_p=s_ReadRegRc523(rControlReg);
						if((ret_p&0x07)==0){	//LastBit 8.
								goto out;
						}
					}
				}
			}
			

			s_WriteRegRc523(rCommandReg,0x00);//clear command
			s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
			s_WriteRegRc523(rFIFOLevelReg,0x80);
			s_SetBitRegRc523(rControlReg ,0x40);
			
			printfx("%d:%d\r\n",__LINE__,iresp);



			s_WriteRegRc523(rCommandReg,PCD_RECEIVE); 

			
			goto Rx_again;

		}
	}

out:


	s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
	s_WriteRegRc523(rCommandReg,0X00);
	s_ClrBitRegRc523(rBitFramingReg,0x80);
	
	if((iresp!=PICC_OK)&&(iresp!=PICC_RxTimerOut))
		printfx("CmdExchange err: %d\r\n",iresp);
		

	return iresp;
}