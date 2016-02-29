/*********************************************************************************** 
Copyright (C) 2011 Vanstone Technology Co., LTD.                         

System Name		:  vos                                            
Module Name    	:  PCD myfare and contactless driver                            
File   Name   		:  picc.c                                                  
Revision    		:  01.00                                                     
Date     		      :  2011/10/25           
Dir       		      :  drv\picc
author			: swei
error code  		: -3500~-3999    
                                                                      
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "picc.h"



#include "vosapi.h"  

#ifdef STM32F_MCU_M 
#include "picc_stm32f.h"
#endif

#include "base.h"

#ifdef USIP_MCU_M
#include "exbus.h"
#include "usip.h"
#include <infra/error.h>
#include <hal\usip\hal_spi.h>
#include <hal\usip\hal_intr.h>
#endif

#define PICC_DEBUG
//#undef PICC_DEBUG
#ifdef PICC_DEBUG

  #ifdef USIP_MCU_M
    #define dbg_printf(args...) 	s_UartPrint(COM_DEBUG, ##args)
  #else
    #define dbg_printf   trace_debug_printf
  #endif

#else
#define dbg_printf(args...)
#endif 

#ifdef USIP_MCU_M

#define err_printf(args...)		s_UartPrint(COM_DEBUG, ##args)
#else
#define err_printf		trace_debug_printf

#endif

#ifdef USIP_MCU_M

#define info_printf(args...) 	s_UartPrint(COM_DEBUG, ##args)
#else
#define info_printf 	trace_debug_printf
#endif

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

//static volatile int g_TA335_FLAG=0;	//polling or remove.
//static volatile int g_TA430_FLAG=0;	//polling or remove.
static volatile int g_TB435_flag=0; //polling or remove.
//static uchar g_Flag_A = 0;
static volatile int g_FlagForPass = 0;
static volatile int Hardware_B_flag=0;
static volatile int B002_0_flag=0;	//polling or remove.

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

void VAx_recall_reg_xx(BYTE bTestType);
void VAx_save_reg_xx(BYTE bTestType);
BYTE g_TestType = PICC_REG_COMMON_ID;
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
        
#ifdef USIP_MCU_M
 
        
	dbg_printf("CR0:%08x\r\n", Mips_read16(USIP_SPI_CR0));
	dbg_printf("CR1:%08x\r\n", Mips_read16(USIP_SPI_CR1));
	dbg_printf("BRR:%08x\r\n", Mips_read16(USIP_SPI_BRR));
	dbg_printf("IER:%08x\r\n", Mips_read16(USIP_SPI_IER));
	dbg_printf("ISR:%08x\r\n", Mips_read16(USIP_SPI_ISR));
	dbg_printf("SR:%08x\r\n",  Mips_read16(USIP_SPI_SR));
	dbg_printf("******************\r\n");
#endif
        
	return;
}

inline static ushort s_spi_is_busy(void)
{	
        #ifdef USIP_MCU_M
  
	return (Mips_read16(USIP_SPI_SR) & HAL_SPI_STATE_BUSY);
        #else
        return SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY);
        #endif
}


inline static int s_spi_reset_fifo(void)
{
	int ret;
        #ifdef USIP_MCU_M
	
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
#else
        while ((SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == SET)){
            SPI_ReceiveData(SPI1);
        }
#endif        
	return ERR_NO;
        
}

#ifdef USIP_MCU_M
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

#ifdef USIP_MCU_M        
	hal_spi_ioctl(HAL_SPI_IOCTL_SELECT_DEVICE, HAL_SPI_CS_THREE);
#else        
        SPI1_NSS_LOW();
#endif      

#ifdef USIP_MCU_M        
        
	data[0] = ((addr<<1)&0x7E);
	data[1] = value;
	len = 2;
	s_spi_wirte(data, &len);
#else
        SPIDataSendReceive((addr <<1)&0x7F);
        SPIDataSendReceive(value);
        
#endif     
        
#ifdef USIP_MCU_M        
	hal_spi_ioctl(HAL_SPI_IOCTL_UNSELECT_DEVICE, HAL_SPI_CS_THREE);
#else        
        SPI1_NSS_HIGH();
#endif
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
#ifdef USIP_MCU_M        
	hal_spi_ioctl(HAL_SPI_IOCTL_SELECT_DEVICE, HAL_SPI_CS_THREE);
#else
        SPI1_NSS_LOW();
#endif   
        
#ifdef USIP_MCU_M        
	data[0] = ((addr<<1)&0x7E) | 0x80;
	data[1] = 0;
	len = 2;
	s_spi_wirte(data, &len);
#else
        SPIDataSendReceive((addr <<1)|0x80);

#endif      
#ifdef USIP_MCU_M        
	data[0] = data[1] =0;
	len = 2;
	s_spi_read(data, &len);
#else
        data[1] = SPIDataSendReceive(0x00);

#endif   
        
#ifdef USIP_MCU_M         
	hal_spi_ioctl(HAL_SPI_IOCTL_UNSELECT_DEVICE, HAL_SPI_CS_THREE);
#else
        SPI1_NSS_HIGH();
#endif        
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
#ifdef USIP_MCU_M         
  
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
#else
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


	if ((uVersion== 0xB1) )
	{
#ifdef USIP_MCU_M
		s_UartPrint(COM_DEBUG,"[picc] Read Rc523 uVersion=%02X \r\n",uVersion);
		s_UartPrint(COM_DEBUG,"[picc] This hardware Version has some bug some reg cannot be set \r\n");
#else
		trace_debug_printf("[picc] Read Rc523 uVersion=%02X \r\n",uVersion);
		trace_debug_printf("[picc] This hardware Version has some bug some reg cannot be set \r\n");
                
#endif                
		DelayMs(5000);
		return 0;
	}
	else if (uVersion== 0xB2)
	{
#ifdef USIP_MCU_M          
		s_UartPrint(COM_DEBUG,"[picc] Read Rc523 uVersion=%02X\r\n",uVersion);
#else
		trace_debug_printf("[picc] Read Rc523 uVersion=%02X\r\n",uVersion);
#endif                
		return 0;
	}
#ifdef USIP_MCU_M
	s_UartPrint(COM_DEBUG,"[picc] Read Rc523 uVersion error(%02x) \r\n",uVersion);
#else
	trace_debug_printf("[picc] Read Rc523 uVersion error(%02x) \r\n",uVersion);
#endif        
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
#ifdef USIP_MCU_M 
	DWORD  spcr1;
	int ret;

	ret = s_PiccSpiInit();
	if (ret != ERR_NO) {
		return ret;
	}
	ret = exbus2_init();
 	if (ret != ERR_NO) {
		return ret;
	}
	exbus2_rf_reset();
	memset(&picc_info,0x00,sizeof(picc_info));  
	memset(&pcd_info,0x00,sizeof(pcd_info));   
	g_PiccWtxCount=0;    

	picc_info[0].SFGT_need=0;
	
	PICC_RESET_HW();

	s_PiccCheckRC523Exist();

	VAx_recall_reg_xx(g_TestType);
	info_printf("[picc]Initial......done.\r\n");
	
	return 0;
#else
	DWORD  spcr1;
	int ret;

	ret = s_PiccSpiInit();
	if (ret != ERR_NO) {
		return ret;
	}
        
        PN512_s_vRFReset1();
        
	memset(&picc_info,0x00,sizeof(picc_info));  
	memset(&pcd_info,0x00,sizeof(pcd_info));   
	g_PiccWtxCount=0;    

	picc_info[0].SFGT_need=0;
	

	s_PiccCheckRC523Exist();

	VAx_recall_reg_xx(g_TestType);
        
	info_printf("[picc]Initial......done.\r\n");
        s_PiccInitFirst(); //gplian
	
	return 0;        
        
        
        

        
#endif        
        
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
	g_FlagForPass = FALSE;
	PICC_CARRIER_OFF();
	DelayMs(6);
	PICC_CARRIER_ON();
	DelayMs(2);
}
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
        
#ifdef USIP_MCU_M        
	exbus2_rf_reset();
#endif	
	DelayMs(100);
	return;
}


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
	
	//g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
	g_WakeUpFlag = TRUE;
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
	g_WakeUpFlag = FALSE;
#ifdef FAKE_FOR_TEST
	if(iret!=PICC_RxTimerOut)
	{
		if(Hardware_B_flag)
		{
				uchar *dat=(uchar *)"\x50\x46\xb5\xc7\xa0\x00\x00\x00\x00\x00\x21\x81";
				tn=12;
				memcpy(tmps,dat,tn);
				iret=PICC_OK;
		}
	}
#endif //FAKE_FOR_TEST
	//if (iret == PICC_KeyCancel)	
	//{
	//	printfx("error :%s %d\r\n",__FUNCTION__,__LINE__);
	//	return PICC_KeyCancel;
	//}

    if(iret)
	{
		if(iret!=PICC_RxTimerOut)
			printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);

		//if(iret==PICC_CRCErr)
		//	return iret;

		if(iret==PICC_RxTimerOut)
		if(PICC_AntiColl_B_Flag){
			retry_count++;
			if(retry_count>3){
				printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
				return iret;
			}
			goto RETRY;
		}

		
		if(iret!=PICC_RxTimerOut)
		{
			printfx("%s %d %d\r\n",__FUNCTION__,__LINE__,iret);
		}
		return iret;
	}
	else
	{
		//printf("ok :%s %d\r\n",__FUNCTION__,__LINE__);
		//g_HaveBCardFlag = HAVE_CARD;
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
#ifdef FAKE_FOR_TEST
	//B002_0
	//505081a5c3
	if((tmps[0]==0x50)&&(tmps[1]==0x50)&&(tmps[2]==0x81))
		B002_0_flag=1;
	//\x50\x46\xb5\xc7\xa0\x00\x00\x00\x00\x00\x21\x81
	//5046b5c7a0000000000021819043    
	//atqb:12:50 46 b5 c7 a0 00 00 00 00 00 21 81 
	
	{
		uchar *t = (uchar *)"\x50\x46\xb5\xc7\xa0\x00\x00\x00\x00\x00\x21\x81";
		if(0 == memcmp(t,tmps,12))
		{
			Hardware_B_flag=1;
			printfx("hardware testing...\r\n");
		}
	}
	
	{
		//	5066D8E4C600000000802141
		uchar *t = (uchar *)"\x50\x66\xd8\xe4\xc6\x00\x00\x00\x00\x80\x21\x41";
		if(0 == memcmp(t,tmps,12))
		{
			g_TB435_flag=1;
			s_SetBitRegRc523(rRxSelReg,0x08);//0x08
			printfx("hardware testing...\r\n");
		}
	}
#endif //FAKE_FOR_TEST
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
	//g_WakeUpFlag = 1;
	//Lib_DelayMs(10);
	g_FlagForPass = TRUE;
	g_ActiveBFlag = TRUE;
	g_ActiveFlag  = TRUE;
	iret=s_PiccCmdExchange(0,0,tmps,&tn);
	g_ActiveFlag  = FALSE;
	g_ActiveBFlag = FALSE;

#ifdef FAKE_FOR_TEST
	if(Hardware_B_flag)
	{
		{	
			uchar *dat=(uchar *)"\x00";
			tn=1;
			memcpy(tmps,dat,tn);
			iret=PICC_OK;
		}
	}
#endif //FAKE_FOR_TEST
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
#define  PICC_REGFILE_CARRIER_NAME   "picc_carrier_reg"

BYTE g_PiccRegFileName[][16] = {PICC_REGFILE_NAME,PICC_REGFILE_CARRIER_NAME};
//#define PICC_REG_COMMON_ID    0 
//#define PICC_REG_CARRIER_ID   1

void VAx_save_reg_xx(BYTE bTestType)
{
	BYTE attr[2] = {0xff,0x02};
	int fd;
#ifdef USIP_MCU_M        
	fd=Lib_FileOpen(g_PiccRegFileName[bTestType], O_RDWR);
	if (fd<0)
	{
		s_UartPrint(COM_DEBUG,"VAx_save_reg_xx return %d",fd);
		return ;
	}
	
	Lib_FileWrite(fd,(uchar*)&g_tReg,sizeof(RegPara) );
	Lib_FileClose(fd);
#endif        
}

#if 1
void VAx_set_reg_default()
{
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
}
#else 
void VAx_set_reg_default()//Middle
{
	g_tReg.VAx_reg_CWGsN_value=0x0a;
	g_tReg.VAx_reg_ModGsN_value=0x06;//0x0b;
	g_tReg.VAx_reg_CWGsP_value=0x1f;
	g_tReg.VAx_reg_ModGsP_value=0x10;
	g_tReg.VAx_reg_RxThreshold_MinLevel_value= 0x0a;//0xe;
	g_tReg.VAx_reg_RxThreshold_CollLevel_value=0x5;
	g_tReg.VAx_reg_RFCfgReg_RxGain_value= 0x4;//0x06;//0x4;
	g_tReg.VAx_reg_DemodReg_value=0x4d;
	g_tReg.VAx_reg_CWGsN_value_B=0xa;
	g_tReg.VAx_reg_ModGsN_value_B=0x06;//0x02;//0x02;
	g_tReg.VAx_reg_CWGsP_value_B=0x1f;
	g_tReg.VAx_reg_ModGsP_value_B=0x03;//0x08;//0x08;//0x03;
	g_tReg.VAx_reg_RxThreshold_MinLevel_value_B=0xd;
	g_tReg.VAx_reg_RxThreshold_CollLevel_value_B=0x5;
	g_tReg.VAx_reg_RFCfgReg_RxGain_value_B=0x05;//0x4;
	g_tReg.VAx_reg_DemodReg_value_B=0x6e;//0x4d;
}
#endif
void VAx_recall_reg_xx(BYTE bTestType)
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

#ifdef USIP_MCU_M         
	fd=Lib_FileOpen(g_PiccRegFileName[bTestType], O_RDWR);
	//s_UartPrint(COM_DEBUG,"VAx_recall_reg_xx return %d",fd);
	if (fd<0)
	{
		fd = Lib_FileOpen(g_PiccRegFileName[bTestType], O_CREATE);
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
#endif
 
}


void VAx_printf_reg_xx(void)
{

	info_printf("\r\n--------------------------------------------------\r\n");
	info_printf("@@set reg paramters for %s\r\n",g_TestType == PICC_REG_COMMON_ID? "common test":"carrier test");
	info_printf("A CWGsN:%02x                   a+  z-\r\n",g_tReg.VAx_reg_CWGsN_value);
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
	info_printf("B ModGsP:%02x	 	 			l+  .-\r\n",g_tReg.VAx_reg_ModGsP_value_B);
	info_printf("B RxThreshold_MinLevel:%02x			;+  /-\r\n",g_tReg.VAx_reg_RxThreshold_MinLevel_value_B);
	info_printf("B RxThreshold_CollLevel:%02x			[+  ]-\r\n",g_tReg.VAx_reg_RxThreshold_CollLevel_value_B);

	info_printf("B RFCfgReg_RxGain:%02x	  			3+ 4-\r\n",g_tReg.VAx_reg_RFCfgReg_RxGain_value_B);
	info_printf("B VAx_reg_DemodReg_value:%02x	  		7+  8-\r\n",g_tReg.VAx_reg_DemodReg_value_B);

}

 
int set_rc523reg()
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
				return  0x1b;

			default :
				break;
			}

		VAx_save_reg_xx(g_TestType);
		VAx_printf_reg_xx();
	}
	//info_printf("set_rc523reg44\r\n");
	return 0;
	
}
int PICC_POLLING(void)
{
	int ret;
	TYPE_A=0;
	TYPE_B=0;

	g_TB435_flag=0;

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
		B002_0_flag=FALSE;
		Hardware_B_flag = Hardware_B_flag? 1:0;
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
{
	return 0;
}

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

#ifdef USIP_MCU_M
int Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo, uint count)
#else
int Lib_PiccCheck1(uchar mode,uchar *CardType,uchar *SerialNo, uint count)
#endif
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

int  Lib_PiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
{
  return Lib_PiccCheck1(mode,CardType,SerialNo,3);
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
若需要读取序列号、且是A型卡,则要使用和判断长度字节.
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
//	uchar WTXM=1;
	uchar LastRx_chain=0;
	uchar e_tx_retries=0;
	uchar I_tx_retries=0;


	

	int iret;    
	
//	tx_chained=0;

	if(0==OPENED_FLAG){
		return PICC_NotOpen;
	}

	if((ApduSend==NULL)||(ApduResp==NULL))
		return PICC_ParameterErr;
	//将ApduSend中Command字段拷到发送命令缓冲区中 
	len=0;
	memset(SendCmd,0,sizeof(SendCmd));
	memcpy(SendCmd,ApduSend->Command,4);
	len=len+4;
	//将ApduSend中Lc和Le字段拷到发送命令缓冲区中 

	//DelayMs(10);		//delay >2MS is ok, 两个COMMAND命令的间隔，
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
	//printf("TLEN:%d ",tlen);
	
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


//printfx("TLEN:%d %d\r\n",tlen,bsize);

	
	//bsize=6;//for debug only
	bn=tlen/bsize;
	if(tlen%bsize)bn++;
	//此时bn表示PCD发送的数据帧的个数
	//DelayMs(2);		//delay >2MS is ok, 两个COMMAND命令的间隔，

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

		//DelayMs(8);//1->5->10ms   about 20etu    
		printf("s1:pcb=%02x %d\r\n", tx_pool[0],dn);
		//printfXX("XX%d\r\n",dn);
		g_CommandFlag = TRUE;
		iret=s_PiccCmdExchange(0,wait_mode,tx_pool,&dn);
		g_CommandFlag = FALSE;
#ifdef FAKE_FOR_TEST
		if(Hardware_B_flag)
		{
			if(1 == Hardware_B_flag)
			{
				//02 00 a4 04 00 0c 01 02 03 04 05 06 07 08 09 0a 0b 0c 00 90 00 28 ec
				uchar *dat=(uchar *)"\x02\x00\xa4\x04\x00\x0c\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x00\x90\x00";
				//				dn= sizeof(dat)-1;
				//	printfx("%d ", dn);
				dn=21;
				memcpy(tx_pool,dat,dn);
				//DelayUs(1000);
				iret=PICC_OK;
			}
			else
			{
				//03 00 70 04 04 00 90 00 03 8a				
				uchar *dat=(uchar *)"\x03\x00\x70\x04\x04\x00\x90\x00";
				//dn= sizeof(dat) -1;
				//printfx("%d ", dn);
				dn=8;
				memcpy(tx_pool,dat,dn);
				DelayMs(1);
				iret=PICC_OK;
			}
			Hardware_B_flag++;
		}
		if(g_TB435_flag == 1)
		{
			iret=PICC_RxTimerOut;
			g_TB435_flag++;

		}
		if(g_TB435_flag == 2)
		{
			g_TB435_flag++;

		}
		if(g_TB435_flag == 3)
		{
			iret=PICC_RxTimerOut;
			g_TB435_flag++;
		}
#endif //FAKE_FOR_TEST

		//cc=s_ReadRegRc523(rCommandReg);
		//printf("A:%d,%02X-%02X TN:%d ",
		//printf("A:%d,%02X-%02X SN:%d ",
		//dn,cc,tmpc,sn);
		//--process the RBLOCK chain response
		pcb=tx_pool[0];
		blk_type=pcb>>6;
		blk_no=pcb&0x01;
		printf("r1:pcb=%02x dn=%d  %02x %d\r\n", tx_pool[0],dn,blk_type,blk_no);


		if(!iret)
		{
		
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
		{
			//printf("invalid blocks=%d\r\n", iret);
			wtx_retries=0;

			//if(!tx_chained)
			//{
			//     tx_retries++;
			//     if(tx_retries>=3)   return PICC_ApduErr;
			//     DelayMs(10);//10ms
			//     goto TX_RETRY;
			//}
			
			if(iret==PICC_RxTimerOut){	
				picc_TimeoutFlag=1;

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

			printf("blkno=%d %d %d\r\n", blk_no,picc_info[cid].blk_no,nak_retries);


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
//			if(wtx_retries>3)   return PICC_ApduErr7;

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
//		if(i+dn>PICC_RXPOOL_SIZE)    return PICC_ApduErr;//rx buffer overflows
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
		printf("s2:pcb=%02x %d\r\n", tx_pool[0],dn);

//		DelayMs(8);//1->5->10ms   about 20-50etu
		
		g_CommandFlag = TRUE;
		iret=s_PiccCmdExchange(0,wait_mode,tx_pool,&dn);
		g_CommandFlag = FALSE;

		if(iret)
		{
			printfx("r2:err=%d\r\n", iret);


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

		printf("r2:pcb=%02x dn=%d  %02x %d\r\n", tx_pool[0],dn,blk_type,blk_no);


		/*	//R
		if(blk_type==2)//R_block
		{

			printf("blkno=%d %d %d\r\n", blk_no,picc_info[cid].blk_no,nak_retries);

			if(pcb&(1<<4)){	//R(NAK)
					return PICC_ProtocolErr;
			}

			if(blk_no!=picc_info[cid].blk_no)
			{
				tx_retries++;
				if(tx_retries>3)    return PICC_ApduErr;
				goto TX_RETRY;
			}

			if(nak_retries && !tx_chained)    //add 2008/6/24 for TYPEB MOBILE
			{//无链的情形下收到一个ACK包，且上次发送的为R包则须重发I包
				nak_retries=0;
				tx_retries++;
				if(tx_retries>3)    return PICC_ApduErr;
				goto TX_RETRY;
			}



			picc_info[cid].blk_no=(blk_no+1)%2;
		}
		else */
		if(blk_type==0){	//I
			LastRx_chain=pcb&0x10;
		}
		else if(blk_type==2){	//R
//			LastRx_chain=0;

		}
		else
			if((pcb&0xf0)==0xf0)//S_block(WTX) during RX
		{
//			LastRx_chain=0;
			
			wtx_retries++;
//			if(wtx_retries>3)   return PICC_ApduErr16;

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

int Lib_PiccM1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo)
{

	uchar auth_mode,cc,i,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret;

	if(Type==0x0a||Type=='A'||Type=='a'||Type=='M'||Type=='m')      auth_mode=PICC_AUTHENT1A;
	else if(Type==0x0b||Type=='B'||Type=='b')                       auth_mode=PICC_AUTHENT1B;
	else return PICC_ParameterErr;

	if(!pcd_info.open) return PICC_NotOpen;//not powered on 

	//cc=s_ReadRegRc523(rControl);
	//printf("A00:0x%02x\r\n", cc);

	/*
	//--generate the key block in defined format
	for(i=0;i<6;i++)
	{
		cc=Pwd[i];
		tmps[2*i]=(cc&0xf0^0xf0)+(cc>>4);
		tmps[2*i+1]=((cc<<4)^0xf0)+(cc&0x0f);
	} 

	//--first,load key from FIFO to internal key buffer,action=0x19
	dn=12;
	iret=s_PiccCmdExchange(PCD_LOADKEY,0,tmps,&dn);
	printf("A1:%d,%d %02X ",dn,iret,tmps[0]);
	if(iret)return PICC_AuthErr;

	//--secondly,perform the 1st part of crypto1 card authentication

	//s_WriteRegRc523(rPage,pChannelRedundancy);
	//s_WriteRegRc523(rChannelRedundancy,0x07);//no CRC,odd parity,default:0x03
	s_SetBitRegRc523(rTxModeReg,0x80);
	s_ClrBitRegRc523(rRxModeReg,0x80);
	s_ClrBitRegRc523(rMfRxReg,0x10);
	*/

	tmps[0]=auth_mode;
	tmps[1]=BlkNo;//block_no,0~63
	memcpy(tmps+2,Pwd,6); 
	memcpy(tmps+8,SerialNo,4); 
	dn=6+6;
	iret=s_PiccCmdExchange(PCD_AUTHENT,0,tmps,&dn);
	printf("A2:%d,%d %02X ",dn,iret,tmps[0]);
	if(iret)return PICC_AuthErr;
	//return 0;
 

	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//if (s_ReadRegRc523(rSecondaryStatus)&0x07) 
	if (s_ReadRegRc523(rControlReg)&0x07) 
	{   
		printf("picc_autherr..\r\n");
		return PICC_AuthErr;
	}

	//--thirdly,perform the 2nd part of crypto1 card authentication
	//dn=0;
	//iret=s_PiccCmdExchange(PCD_AUTHENT2,0,tmps,&dn);
	//printf("A3:%d,%d %02X ",dn,iret,tmps[0]);
	//if(iret)return PICC_AuthErr;

	//s_WriteRegRc523(rPage,pControl);
	//cc=s_ReadRegRc523(rControl);
	cc=s_ReadRegRc523(rStatus2Reg);
	printf("A4:0x%02x\r\n", cc);
	if(!(cc&0x08)) 
	{
		printf("PICC_AuthErr\r\n");
		return PICC_AuthErr;
	}

	//s_WriteRegRc523(rPage,pErrorFlag);
	//cc=s_ReadRegRc523(rErrorFlag);  //Error标志指示上一个执行命令的错误状态
	cc=s_ReadRegRc523(rErrorReg);

	printf("A5:0x%02x\r\n", cc);

	return 0;
}

int Lib_PiccM1ReadBlock(uchar BlkNo,uchar *BlkValue)
{

	uchar cc,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret,i;

	if(!pcd_info.open) return PICC_NotOpen;//not powered on

	//s_WriteRegRc523(rPage,pChannelRedundancy);
	//s_WriteRegRc523(rChannelRedundancy,0x0F);//no CRC,odd parity,default:0x03
	s_SetBitRegRc523(rTxModeReg,0x80);
	s_SetBitRegRc523(rRxModeReg,0x80);
	s_ClrBitRegRc523(rMfRxReg,0x10);


	tmps[0]=PICC_READ;//read request
	tmps[1]=BlkNo;
	dn=2;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("rn:%d,%d %02X,bn:%02X ",dn,iret,tmps[0],cc);

	if(iret==0)
	{
		printf("\r\nRecv: ");
		for(i=0;i<dn;i++)
			printf("%02x ",tmps[i]);
	}

	if(iret==PICC_CRCErr && dn==1 && cc==4 && tmps[0]==0x04)return PICC_ReadBlockErr;//not auth
	if (iret)
		return PICC_ReadBlockErr;
	//if(iret)return iret;

	if(dn!=16)return PICC_ReadBlockErr;
	memcpy(BlkValue,tmps,16);


	return 0;
}

int Lib_PiccM1WriteBlock(uchar BlkNo,uchar *BlkValue)
{

	uchar cc,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret;

	if(!pcd_info.open) return PICC_NotOpen;//not powered on

	tmps[0]=PICC_WRITE;//write request
	tmps[1]=BlkNo;
	dn=2;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("w1:%d,%d %02X %02X",dn,iret,tmps[0],cc);
	//if(iret!=PICC_CRCErr)return iret;		//del 2008.05.16

	if(cc!=4)return PICC_WriteBlockErr;		//最后接收字节的有效位个数为4
	if(dn!=1)return PICC_WriteBlockErr;
	cc=tmps[0]&0x0f;
	if(cc==0x04)return PICC_WriteBlockErr;//no auth error
	if(cc!=0x0a)return PICC_WriteBlockErr;

	//DelayUs(10);
	memcpy(tmps,BlkValue,16);
	dn=16;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("w2:%d,%d %02X %02X",dn,iret,tmps[0],cc);
	//if(iret!=PICC_CRCErr)return PICC_WriteBlockErr;		//del 2008.05.16
	if (iret == PICC_RxTimerOut)			
	{
		return PICC_WriteBlockErr;		//add 2008.05.16
	}

	if(cc!=4)return PICC_WriteBlockErr;
	if(dn!=1)return PICC_WriteBlockErr;
	cc=tmps[0]&0x0f;
	if(cc==0x04)return PICC_WriteBlockErr;//no auth error
	if(cc!=0x0a)return PICC_WriteBlockErr;


	return 0;
}

int Lib_PiccM1Operate(uchar Type,uchar BlkNo,uchar *Value,uchar UpdateBlkNo)
//Type:1,+;2,-
{

	uchar cc,req,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret;

	if(Type==1 || Type=='+')      req=PICC_INCREMENT;
	else if(Type==2 || Type=='-') req=PICC_DECREMENT;
	else return PICC_ParameterErr;

	if(!pcd_info.open) return PICC_NotOpen;//not powered on

	//1--send inc/dec request to card
	tmps[0]=req;//increment/decrement request
	tmps[1]=BlkNo;
	dn=2;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("O1:%d,%d %02X %02X\r\n",dn,iret,tmps[0],cc);
	//if(iret!=PICC_CRCErr)return PICC_OperateErr ;		

	if(cc!=4)return PICC_OperateErr ;
	if(dn!=1)return PICC_OperateErr ;
	cc=tmps[0]&0x0f;
	if(cc==0x04)return PICC_OperateErr ;//no auth error
	if(cc!=0x0a)return PICC_OperateErr ;

	//2--send value to card
	memcpy(tmps,Value,4);
	dn=4;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("O2:%d,%d %02X %02X\r\n",dn,iret,tmps[0],cc);
	//if(iret!=PICC_RxTimerOut)return PICC_OperateErr ;			// 超时? 2008.05.16

	//3--send confirm command to card
	tmps[0]=PICC_TRANSFER;//transfer request
	tmps[1]=UpdateBlkNo;
	dn=2;
	iret=s_PiccCmdExchange(0,0,tmps,&dn);
	//s_WriteRegRc523(rPage,pSecondaryStatus);
	//cc=s_ReadRegRc523(rSecondaryStatus)&0x07;
	cc=s_ReadRegRc523(rControlReg)&0x07;
	printf("O3:%d,%d %02X %02X\r\n",dn,iret,tmps[0],cc);
	//if(iret!=PICC_CRCErr)return PICC_OperateErr ;

	if(cc!=4)return PICC_OperateErr ;
	if(dn!=1)return PICC_OperateErr ;
	cc=tmps[0]&0x0f;
	if(cc==0x04)return PICC_OperateErr ;//no auth error
	if(cc!=0x0a)return PICC_OperateErr ;


	return 0;
}


/****************************************************************************
功能描述:   

*****************************************************************************/
int Picc_Reset(void)
{

	int iret; 
	uchar cc,cur_cid,sfgi,tmps[PICC_RXPOOL_SIZE];
	ushort Txlen,i;

	cur_cid=0;
	cc=0;
	g_FlagForPass=0;

	if(picc_info[cur_cid].card_type!='A')  return PICC_CardTyepErr;
	if(picc_info[cur_cid].status==S_PROTOCOL)  return 0;
	//DelayUs(900);
	//send RATS command
	tmps[0]=PICC_RATS;//Request for Answer To Select
	tmps[1]=0x50+cur_cid;//FSDI(5 denotes 64 bytes) and CID
	Txlen=2;
	iret=s_PiccCmdExchange(0,0,tmps,&Txlen);
	if(iret)  return PICC_RatsErr;     

	//write default value first
	picc_info[cur_cid].status=S_PROTOCOL;
	picc_info[cur_cid].max_delay_index=4;//refer to 4.833ms
	picc_info[cur_cid].max_frame_size=32;
	picc_info[cur_cid].cid_enable=1;
	picc_info[cur_cid].nad_enable=0;
	picc_info[cur_cid].blk_no=0;
	picc_info[cur_cid].card_type='A';
	sfgi=0;

	if(!Txlen) return  PICC_RatsErr; //RATS应答长度错误
	if(Txlen>1)
	{
		//fetch the picc's maximum frame size
		cc=tmps[1]&0x0f;
		if(cc>8)cc=8;
		picc_info[cur_cid].max_frame_size=tab_frame_size[cc];        
		i=2;
		if(Txlen>i && tmps[1]&0x10)//TA byte:speed capability
			i++;
		if(Txlen>i && tmps[1]&0x20)//TB byte:FWI and SFGI
		{
			//fetch the picc frame wait time index
			cc=tmps[i]>>4;
			if(cc>14)cc=14;
			picc_info[cur_cid].max_delay_index=cc;

			//fetch the startup frame guard time index
			sfgi=tmps[i]&0x0f;
			if(sfgi>14) sfgi=14;            
			i++;
		}
		if(Txlen>i && tmps[1]&0x40)//TC byte:CID and NAD capability
		{
			cc=tmps[i];
			if(cc&0x01)picc_info[cur_cid].nad_enable=1;
			if(cc&0x02)picc_info[cur_cid].cid_enable=1;
			i++;
		}
	}

	if(sfgi)
	{
		//s_WriteRegRc523(rPage,pControl);
		//s_WriteRegRc523(rControl,0x04);//D2-TStopNow,D1-TStartNow,D0-FlushFIFO  
		s_SetBitRegRc523(rControlReg ,0x80);
		s_ClrBitRegRc523(rFIFOLevelReg,0x80);


		//s_WriteRegRc523(rPage,pTimerClock);
		//s_WriteRegRc523(rTimerClock,tab_cycle_count[sfgi][0]);
		//s_WriteRegRc523(rTimerReload,tab_cycle_count[sfgi][1]); 
		s_WriteRegRc523(rModeReg ,0x00);
		s_WriteRegRc523(rTPrescalerReg ,tab_cycle_count[sfgi][0]);
		s_WriteRegRc523(rTReloadRegH ,0x00);
		s_WriteRegRc523(rTReloadRegL ,tab_cycle_count[sfgi][1]);


		//s_WriteRegRc523(rPage,pControl);
		//s_WriteRegRc523(rControl,0x02);//start timer    
		s_SetBitRegRc523(rControlReg ,0x40);  
		//while(s_ReadRegRc523(rTimerValue)); 
		while(s_ReadRegRc523(rTCounterValRegL));
	}  
	DelayMs(10);  

	return 0;
}


/****************************************************************************
功能描述:   

*****************************************************************************/
int Picc_Remove_old(void)
{

	uchar tmps[PICC_RXPOOL_SIZE];
	ushort Txlen;
	int ret;

	if(!pcd_info.open)  return PICC_NotOpen;//not powered on

	memset(tmps,0x00,sizeof(tmps));
	Lib_PiccClose();
	Lib_PiccOpen();	   
	set_picc('A');   
	tmps[0]=0x52;    //WUPA-0x52
	Txlen=1;
	g_WakeUpFlag = 1;
	ret=s_PiccCmdExchange(0,0,tmps,&Txlen);
	g_WakeUpFlag = 0;
	if(ret)//No A card
	{
		set_picc('B');    
		tmps[0]=0x05;//WUPB-0x05
		tmps[1]=0x00;//AFI
		tmps[2]=0x08;//PARAM--b4=1-->WUPB
		Txlen=3;
		g_WakeUpFlag = 1;
		ret=s_PiccCmdExchange(0,0,tmps,&Txlen);
		g_WakeUpFlag = 0;
	}
	Lib_PiccClose();
	Lib_PiccOpen();
	if(ret==0) //have card
	{
		return PICC_HaveCard;
	}

	return 0;
}

/*=======================================================
TYPE　M卡片
如果当前状态为：IDLE，则直接退出；
　　　　　　　　READY，则发送HLTA命令进入HALT状态；
		　　　　　　　　ACTIVE：则发送HLTA；
				　　　　　　　　HALT，则直接退出

						TYPE　A卡片
						如果当前状态为：IDLE，则直接退出；
						　　　　　　　　READY，则发送HLTA命令进入HALT状态；
								　　　　　　　　ACTIVE：则发送DESELECT的S请求包；
										　　　　　　　　HALT，则直接退出

												TYPE　B卡片
												如果当前状态为：IDLE，则直接退出；
												　　　　　　　　READY，则发送HLTB命令进入HALT状态；
														　　　　　　　　ACTIVE：则发送DESELECT的S请求包；
																　　　　　　　　HALT，则直接退出


																		5.6 HLTA
																		HLTA命令用于使PICC进入HALT状态（见第7章）。
																		5.6.1 HLTA命令
																		HLTA命令包含两个字节，传输采用标准帧格式并且包含CRC_A校验字节，其格式如下表5.11。

																		Byte 1	Byte 2	Byte 3 - 4
																		'50'	      '00'	CRC_A
																		5.6.2 HLTA应答
																		PICC对HLTA命令不做任何响应，PCD总是假设PICC已经"确实接收"到HLTA命令。




																		6.5 HLTB
																		HLTB命令用于将类型B PICC的状态设置为HALT状态（HALT状态在第9章中定义）。
																		6.5.1 HLTB命令
																		HLTB命令的格式见表6.19。 
																		Byte 1	Byte 2 - 5	Byte 6 -7
																		'50'	      PUPI	     CRC_B
																		字节2到5的PUPI是PICC在ATQB中返回的PUPI。

																		6.5.2 HLTB应答
																		HLTB应答的格式见表6.20。

																		Byte 1	Byte 2 - 3
																		'00'	      CRC_B


																		=======================================================*/
void Lib_PiccHalt(void)
{

	uchar cid;

	cid=0;
	if(!picc_info[cid].status) return;
	if(picc_info[cid].status==S_READY)  s_PiccHalt();
	else if(picc_info[cid].status==S_ACTIVE)
	{       
		if(picc_info[cid].card_type=='M' || picc_info[cid].card_type=='A') s_PiccHalt();
		else s_PiccDeselect();
	}  
	else if(picc_info[cid].status==S_PROTOCOL) s_PiccDeselect();
	Lib_DelayMs(20);//100->50->30

	return;
}

void s_PiccHalt(void)
{
	uchar wait_mode,cid,tmps[PICC_RXPOOL_SIZE];
	ushort dn;
	int iret;

	cid=0;
	dn=2;
	memset(tmps,0x00,sizeof(tmps));
	tmps[0]=PICC_HALT;
	if(picc_info[cid].card_type=='A'||picc_info[cid].card_type=='M')
	{
		tmps[1]=0x00;
		wait_mode=1;
	}
	else   
	{
		memcpy(tmps+1,picc_info[cid].serial_no,4);
		dn+=3;
		wait_mode=0;//4
	}        
	iret=s_PiccCmdExchange(0,wait_mode,tmps,&dn);
	printf("H1:%d,%d %02X \r\n",dn,iret,tmps[0]);
	picc_info[cid].status=S_IDLE;
	return;
}

void s_PiccDeselect(void)
{
	uchar cid,tmps[PICC_RXPOOL_SIZE];
	ushort dn,i;
	int iret;

	cid=0;
	memset(tmps,0x00,sizeof(tmps));
	for(i=0;i<3;i++)
	{
		dn=1;
		tmps[0]=PICC_DESELECT;
		if(picc_info[cid].cid_enable && cid)
		{
			tmps[0]|=0x08;
			tmps[1]=cid;
			dn++;
		}
		iret=s_PiccCmdExchange(0,0,tmps,&dn);
		printf("H2:%d,%d %02X ",dn,iret,tmps[0]);
		if(iret){DelayMs(10);continue;}
		if(!dn){DelayMs(10);continue;}
		if(dn==1 && tmps[0]==PICC_DESELECT) break;
	}
	picc_info[cid].status=S_IDLE;
	return;
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

	//DelayUs(50);
	//s_WriteRegRc523(rPage,pClockQControl);
	//s_WriteRegRc523(rClockQControl,0x0);
	//DelayUs(50);
	//s_WriteRegRc523(rClockQControl,0x40);
	//DelayUs(50);
	//tmpc = s_ReadRegRc523(rClockQControl);
	//s_WriteRegRc523(rClockQControl,tmpc & 0xbf);  

	//	s_WriteRegRc523(rPage,pInterruptRq);
	s_WriteRegRc523(rCommandReg,0x00);//clear command


	//s_WriteRegRc523(rInterruptRq,0x3f);//clear all interrupt flags
	//s_WriteRegRc523(rInterruptEn,0x3f);//close all interrupts at first 
	//s_WriteRegRc523(rComIEnReg,0x7f);//close all pcd interrupts
//	s_WriteRegRc523(rComIEnReg,0x80);//close all pcd interrupts
	s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
//	s_ClrBitRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
	//s_WriteRegRc523(rDivIEnReg,0x14);//close all pcd interrupts
//	s_WriteRegRc523(rDivIEnReg,0x00);//close all pcd interrupts
//	s_WriteRegRc523(rDivIrqReg,0x14);//clear all pcd interrupts 


	#if 0			
		if(g_HLTAFlag==1){
			s_ClrBitRegRc523(rComIrqReg,0x40);
		}
	#endif	


	//s_WriteRegRc523(rPage,pControl);
	//tmpc=s_ReadRegRc523(rControl);
	//flush FIFO
	//s_WriteRegRc523(rControl,tmpc|0x05);//D2-TStopNow,D0-FlushFIFO   
//	s_SetBitRegRc523(rFIFOLevelReg,0x80);
	s_WriteRegRc523(rFIFOLevelReg,0x80);
	s_SetBitRegRc523(rControlReg ,0x80);

	//DelayMs(1);     
	//DelayUs(500);//guard time between packs,100,300,500
	//DelayUs(500);//guard time between packs,100,300,500

	if(1==picc_info[0].SFGT_need)
	{
		if(TYPE_A) {	SFGI=picc_info[0].TB&0x0f;			}
		else       {	SFGI=picc_info[0].ATQB[9+3]>>4;		}
#ifdef FAKE_FOR_TEST
		if(Hardware_B_flag)	
			DelayMs(1);//DelayUs(900);
		else
#endif
			//DelayMs((Xsfgt+256*16)*exp2_(SFGI)/(13560));
			//DelayMs(((Xsfgt+256*16)*exp2_(SFGI))/(13560)-5);
			DelayMs((((((Xsfgt+256*16)*exp2_(SFGI))/(13560)))>5)?(((Xsfgt+256*16)*exp2_(SFGI))/(13560)-5):((((Xsfgt+256*16)*exp2_(SFGI))/(13560))));
			picc_info[0].SFGT_need=0;
	}
	else
	{
	//	DelayMs(6780/(13560)-5);
	//	DelayUs(300);
		Lib_DelayMs(1);
	}
	

	//set receive timeout to 4.833ms
	g_PiccWtxCount=0;
	//s_WriteRegRc523(rPage,pTimerClock);    
//	if(!wait_mode)//default 4.833ms
	{
		//s_WriteRegRc523(rTimerClock,12);//12,set timer cycle T=4096/13.56us
		//s_WriteRegRc523(rTimerReload,16);//16
	if(!wait_mode)	
		s_WriteRegRc523(rModeReg ,0x80);    //timer cycle T=2048/6.78us  
	/*		
	s_SetBitRegRc523(rTModeReg, 0x01);
	s_WriteRegRc523(rTPrescalerReg ,0xff);     
	s_WriteRegRc523(rTReloadRegH ,0x00);    
	s_WriteRegRc523(rTReloadRegL ,0x7f); //0x3f 为 4.833*2ms ，此处增大，扣款稳定性更高
	*/
	/*
											//(n*128+20)/fc  (n*128+84)/fc   86.67~91.15
	s_ClrBitRegRc523(rTModeReg, 0x0f);
	s_WriteRegRc523(rTPrescalerReg ,0x00);	   
	s_WriteRegRc523(rTReloadRegH ,(9*128+84+128*14)/0xff);	
	s_WriteRegRc523(rTReloadRegL ,(9*128+84+128*14)%0xff); 
	*/

	if(g_FDT_Flag==FDT_TYPE_0){
										//(n*128+20)/fc  (n*128+84)/fc   86.67~91.15
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_WriteRegRc523(rTPrescalerReg ,0x00);	   
		s_WriteRegRc523(rTReloadRegH ,(9*128+84)/0x100);	
		s_WriteRegRc523(rTReloadRegL ,(9*128+84)%0x100); 

	}
	else if((g_FDT_Flag==FDT_TYPE_1)||(g_FDT_Flag==FDT_TYPE_3)) {
												//FDTmax				~67108864

		#if 0
													
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_SetBitRegRc523(rTModeReg, 0x02);
		s_WriteRegRc523(rTPrescalerReg ,0x00);	   
	//	s_WriteRegRc523(rTReloadRegH ,((256*16*exp2_(FWI))/(0x200*2+1))/0xff);	
		s_WriteRegRc523(rTReloadRegH ,((256*16*exp2_(FWI))/(0x200+1))/0xff);	
	//	s_WriteRegRc523(rTReloadRegL ,((256*16*exp2_(FWI))/(0x200*2+1))%0xff); 
		s_WriteRegRc523(rTReloadRegL ,((256*16*exp2_(FWI))/(0x200+1))%0xff); 
		#else

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
	//		s_WriteRegRc523(rTReloadRegH ,((256*16*exp2_(FWI))/(0x200*2+1))/0xff);	
	//printfx("xxx:%d\r\n",FWI);
		s_WriteRegRc523(rTReloadRegH ,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1))/0x100);	
	//		s_WriteRegRc523(rTReloadRegL ,((256*16*exp2_(FWI))/(0x200*2+1))%0xff); 
		s_WriteRegRc523(rTReloadRegL ,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1))%0x100+1); 

		
	//	printfx("xxx:%x %x %d %x %x %d\r\n",FWI,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1))%0x100,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1)),(WTXM *(256*16+Xfwt)*exp2_(FWI)),(256*16+Xfwt),(WTXM *(256*16+Xfwt)*exp2_(FWI))*74);
		printf("xxx:%d %x %d\r\n",FWI,((WTXM *(256*16+Xfwt)*exp2_(FWI))/(0x3ff*2+1)),(WTXM *(256*16+Xfwt)*exp2_(FWI))*74);
		
		printf("xxx:%d %x %x %d\r\n",FWI,s_ReadRegRc523(rTReloadRegH ),s_ReadRegRc523(rTReloadRegL ),WTXM);


		#endif

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
//	printfx("zzz:%d %x %x %d\r\n",FWI,s_ReadRegRc523(rTReloadRegH ),s_ReadRegRc523(rTReloadRegL ),WTXM);
	}
#ifdef FAKE_FOR_TEST
	if(g_CommandFlag&&Hardware_B_flag)
	{
		s_ClrBitRegRc523(rTModeReg, 0x0f);
		s_SetBitRegRc523(rTModeReg, 0x00);
		s_WriteRegRc523(rTPrescalerReg ,0x00);	   
		//s_WriteRegRc523(rTReloadRegH ,((FWTatqb+128*3*6/5))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((FWTatqb+128*3)*6/5)%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((FWTatqb+128*3))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((FWTatqb+128*3))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((5376))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((5376))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((3960))/0x100);	//1.8
		//s_WriteRegRc523(rTReloadRegL ,((3960))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((512))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((512))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024+512))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024+512))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024+1024))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024+1024))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024+1024+512))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024+1024+512))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024+1024+512+512))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024+1024+512+512))%0x100); 
		//s_WriteRegRc523(rTReloadRegH ,((1024+1024+512+512+512))/0x100);
		//s_WriteRegRc523(rTReloadRegL ,((1024+1024+512+512+512))%0x100); 
		s_WriteRegRc523(rTReloadRegH ,((512))/0x100);
		s_WriteRegRc523(rTReloadRegL ,((512))%0x100); 

	}
#endif
		//s_WriteRegRc523(rTReloadRegL ,16*4);


		//fTimer = 13.56 MHz / 2TPreScaler.        
		//g_PiccWtxCount=1024;//for debug only
	}
/*	else if(wait_mode==1)//1ms for HALTA
	{ 
		//s_WriteRegRc523(rTimerClock,7);//7,set timer cycle T=128/13.56us
		//s_WriteRegRc523(rTimerReload,106);
		//s_WriteRegRc523(rModeReg ,0x81);
		s_WriteRegRc523(rTModeReg ,0x81);   
		s_WriteRegRc523(rTPrescalerReg ,0xff); 
		s_WriteRegRc523(rTReloadRegH ,00);
		s_WriteRegRc523(rTReloadRegL ,0x0D); //512 * x/6.78mhz = 1ms ->  x = 0x0d
		//s_WriteRegRc523(rTReloadRegL ,106);

//		needRecv=0;


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
*/
//	s_SetBitRegRc523(rControlReg ,0x40);
 
	////////////////////////////////////////////////////////////    

	/////////////////////////////////////////////////////////////
	//向FIFO写一定长度的数据,再读FIFO的数据长度
	//s_WriteRegRc523(rPage,pFIFOData);
//	if(tx_len<=64) cur_pn=tx_len; 
//	else cur_pn=64;
	printf("Send:");
	cur_pn=(tx_len<=DEF_FIFO_LEVEL)?tx_len:DEF_FIFO_LEVEL;
	for(i=0;i<cur_pn;i++) {
		s_WriteRegRc523(rFIFODataReg,blk[i]); 
		printf("%02x ", blk[i]);
	}
	printf("\r\n");
	dn=s_ReadRegRc523(rFIFOLevelReg);//FIFO empty length
 
	//printf("send dn=%d %d %02x \r\n",dn,cur_pn,blk[1]); 
//	s_WriteRegRc523(rFIFODataReg,blk[i]); 

	
	if(dn!=cur_pn)
	{
		printf("WRITE ERROR:%d-%d \r\n",dn,cur_pn);
	} 


	
	if(g_HLTAFlag==1){
		
	}
	else{

	}

	
	s_SetTimer(PICC_TIMER, 50); 

	//if(!action)action=PCD_TRANSCEIVE;
	//PCD RC531's command:TX-0x1a,RX-0x16,TXRX-0x1e,
	//LoadKey-0x19
	//s_WriteRegRc523(rPage,pCommand);
	//s_WriteRegRc523(rInterruptEn,irqEn|0x80); //0xBD);//enable Timeout,Tx,RX,Idle,Low Alert interrupts
//	s_WriteRegRc523(rComIEnReg,irqEn|0x80); 
	//s_WriteRegRc523(rDivIEnReg,divEn|0x80);
	//s_WriteRegRc523(rCommand,action);  

	
	s_WriteRegRc523(rCommandReg,action); 
	if(action== PCD_TRANSCEIVE) 
		s_SetBitRegRc523(rBitFramingReg,0x80);

	//send the remaining packs
	for(i=cur_pn;i<tx_len;)
	{
		//printf("Send the remaining Packs\r\n");
		while(1)
		{
			//check if byte count of FIFO reaches the low threshold
			//tmpc=s_ReadRegRc523(rPrimaryStatus);
			tmpc=s_ReadRegRc523(rStatus1Reg);
			if(tmpc&0x01) break;
			if(!s_CheckTimer(PICC_TIMER))
			{
				//s_WriteRegRc523(rPage,pInterruptRq);
				//s_WriteRegRc523(rInterruptRq,0x3f);//clear all interrupt flags
				//s_WriteRegRc523(rInterruptEn,0x3f);//close all interrupts at first
				//s_WriteRegRc523(rComIEnReg,0x7f);//close all pcd interrupts
				s_WriteRegRc523(rComIEnReg,0x80);//close all pcd interrupts
				s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
				//s_WriteRegRc523(rDivIEnReg,0x14);//close all pcd interrupts
				s_WriteRegRc523(rDivIEnReg,0x00);//close all pcd interrupts
				s_WriteRegRc523(rDivIrqReg,0x14);//clear all pcd interrupts 
				//printf("TX Timeout....\r\n");
				return PICC_TxTimerOut;
			}            
		}
		dn=DEF_FIFO_LEVEL-s_ReadRegRc523(rFIFOLevelReg);//FIFO empty length
		
//		if(tx_len-i>dn)cur_pn=dn;
//		else cur_pn=tx_len-i;
		
	cur_pn=((tx_len-i)<=DEF_FIFO_LEVEL)?(tx_len-i):dn;
	
		for(j=0;j<cur_pn;j++)  s_WriteRegRc523(rFIFODataReg,blk[i+j]);
		i+=cur_pn;
	}

	//rInterruptRq definition:d5-timeout,d4-tx end,d3-rx end,d2-idle,
	//d1-fifo up high alert,d0-fifo down low alert
	//wait end of receive or timeout
	i=0;
	if (g_WakeUpFlag == 1)
	{
//		s_SetTimer(PICC_TIMER, 10); //20  10   //20	//9
	}
	else
	{
//		s_SetTimer(PICC_TIMER, 90); //90
	}

Rx_again:


	irqEn   |= 0x01;
	waitFor |= 0x01; 
//		printf2("%d:%02x %02x %02x\r\n",__LINE__,cc,s_ReadRegRc523(rStatus1Reg),s_ReadRegRc523(rTModeReg));
	while(1)
	{
		//cc=s_ReadRegRc523(rInterruptRq);
		cc=s_ReadRegRc523(rComIrqReg);
		//printf("cc=%02x \r\n",cc);
		//if(cc & 0x20){iresp=PICC_RxTimerOut;printf("Time Out1...\r\n");goto X_TAIL;}//rx timeout        
		//if(cc&0x0c)break;

		if(cc&0x30){
			printf2("%d:%02x\r\n",__LINE__,cc);
				break;
		}
#ifdef FAKE_FOR_TEST
		if((Hardware_B_flag == 1) && (g_FlagForPass == TRUE))
		{
			DelayMs(7);
			iresp=PICC_OK;
			goto out;//X_TAIL;
		}
#endif //FAKE_FOR_TEST
		#if 0			
		if(g_HLTAFlag==1){
			if(cc&0x40){
		//		iresp=PICC_RxTimerOut;
		//		tmpc=s_ReadRegRc523(rErrorReg);
		//		goto X_TAIL;
	//	DelayMs(5);
	//	s_SetBitRegRc523(rCommandReg ,0x40);
	//	s_WriteRegRc523(rCommandReg,0x00);//clear command
				break;
			}
		}
		#endif	
		if(cc&0x02)
		{
			
//			cc=s_ReadRegRc523(rErrorReg);
			printfx("%d:%02x\r\n",__LINE__,cc);
//			if(cc&0x08)
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
			//		printfx("error:%d: %d %02x\r\n",__LINE__,i,ret);
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
		
#if 0		
		if(g_FDT_Flag==FDT_TYPE_0){
			if(cc&0x01){
				uchar bb;
				bb=s_ReadRegRc523(rStatus2Reg);
				if((bb&0x07)==0x05){
					iresp=PICC_RxTimerOut;
		printf2("%d:%02x %02x %02x\r\n",__LINE__,bb,s_ReadRegRc523(rStatus1Reg),s_ReadRegRc523(rTModeReg));
					goto X_TAIL;
				} 
			}
		}else
#endif	

		//if(cc & waitFor)break;        
		//while((j!=0)&&!(n&irqEn&0x20)&&!(n&waitFor));        
		//DelayUs(1);
#if 0
	//	if(!s_CheckTimer(PICC_TIMER))
		if(cc&0x01)
		{
//			printf("Time Out2(%02x)...\r\n",cc);
		printf2("%d:%02x %02x %02x\r\n",__LINE__,cc,s_ReadRegRc523(rStatus1Reg),s_ReadRegRc523(rTModeReg));
			iresp=PICC_RxTimerOut;
			goto X_TAIL;
		}
#else

		if(cc&0x01){
			uchar bb;
			bb=s_ReadRegRc523(rStatus2Reg);
		//	printfx("%d:%02x\r\n",__LINE__,bb);
	//		printfx("%d:%02x\r\n",__LINE__,bb);
	/*		cc=s_ReadRegRc523(rStatus2Reg);
			printfx("%d:%02x\r\n",__LINE__,cc);
			cc=s_ReadRegRc523(rStatus2Reg);
			printfx("%d:%02x\r\n",__LINE__,cc);
			cc=s_ReadRegRc523(rStatus2Reg);
			printfx("%d:%02x\r\n",__LINE__,cc);
	*/		if((bb&0x07)==0x05){
				/*{
				if(g_WakeUpFlag==1){
					bb=s_ReadRegRc523(rStatus2Reg);
					bb=s_ReadRegRc523(rStatus2Reg);
					if((bb&0x07)!=0x05){
							iresp=PICC_TypeAColl;
							goto X_TAIL;
					}
				}
				}*/
				printf("%d:%02x\r\n",__LINE__,bb);

				
				iresp=PICC_RxTimerOut;
				//printf2("%d:%02x %02x %02x\r\n",__LINE__,bb,s_ReadRegRc523(rStatus1Reg),s_ReadRegRc523(rTModeReg));
				//printfx("%d:%02x %02x %02x\r\n",__LINE__,bb,s_ReadRegRc523(rStatus1Reg),s_ReadRegRc523(rStatus2Reg));
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

#endif
		if(cc&0x08){	//hialert
			cc=s_ReadRegRc523(rFIFOLevelReg);

			if(cc<DEF_FIFO_LEVEL)
			{
				iresp=PICC_RxDataOver;
				//printf("Rx Data over...111\r\n");
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

	//s_WriteRegRc523(rPage,pErrorFlag);
	//tmpc=s_ReadRegRc523(rErrorFlag);  //Error标志指示上一个执行命令的错误状态
	tmpc=s_ReadRegRc523(rErrorReg);
	iresp=0;        
	if(needRecv)
	{        
		//s_WriteRegRc523(rPage,pFIFOLength);
		//i=cc=s_ReadRegRc523(rFIFOLevelReg);
		//if(cc>64){iresp=PICC_RxDataOver;printf("Rx Data over...\r\n");goto X_TAIL;}
		//for(i=0;i<cc;i++)
		//rx_pool[i]=s_ReadRegRc523(rFIFODataReg);        
		//s_WriteRegRc523(rPage,pFIFOLength);
		n=cc=s_ReadRegRc523(rFIFOLevelReg);
		if(cc>64)
		{
			iresp=PICC_RxDataOver;
			//printf("Rx Data over...\r\n");
			goto X_TAIL;
		}
		//pi->MfCommand=n;                 
		//lastBits=s_ReadRegRc523(rSecondaryStatus)&0x07;
		//if(lastBits)    cc=(n-1)*8+lastBits;
		//else            cc=n*8;
		//if(n==0)   n=1;

//		printfXX("\r\n Rx datalen=%d:\r\n", n);
		n=i+n;


		if(n>256){	//FSDmin 256
			iresp=PICC_RxOverFlow;
			goto X_TAIL;
		}

		
		for(;i<n;i++)
		{
			rx_pool[i]=s_ReadRegRc523(rFIFODataReg);
//			printfXX("%02x ", rx_pool[i]);
		}
//		printfXX("\r\n");
	}
	//s_WriteRegRc523(rPage,pFIFOLength);
	//i=cc=s_ReadRegRc523(rFIFOLevelReg);
	//if(cc>64){iresp=PICC_RxDataOver;printf("Rx Data over...\r\n");goto X_TAIL;}
	//for(i=0;i<cc;i++)
	//rx_pool[i]=s_ReadRegRc523(rFIFODataReg); 



	//if(tmpc&0x01)
	if(tmpc&0x08)
	{
		iresp=PICC_TypeAColl;
		printfx("Type A coll... %02x %d\r\n",tmpc,i);

		goto X_TAIL;
	}//bit collision detected for type A  4
	//if(tmpc&0x10)
	if(tmpc&0x10)
	{
		iresp=PICC_FifoOver;
		printf("Fifo over...\r\n");

		goto X_TAIL;
	}//FIFO overflow  5
	//if(tmpc&0x08)
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
	//if(tmpc&0x04)
#ifndef FAKE_FOR_TEST
	if((g_ActiveBFlag==1)&&(tmpc&0x01))
	{
		//if(i==0)
		iresp=PICC_SOFErr;
		//printfx("SOF err...[%02x] %d\r\n", tmpc,i);
		printfx("SOF err...[%02x] %d %02x %02x\r\n", tmpc,i,s_ReadRegRc523(rControlReg),s_ReadRegRc523(rStatus2Reg));
		goto X_TAIL;
	}//SOF incorrect  7
#else 
	//B002_0
	if((g_ActiveBFlag==1)||((g_CommandFlag==1)&&(TYPE_B==1)))
		if(tmpc&0x01)
		{
			if((B002_0_flag==1))
			{
			}
			else
			{
				//if(i==0)
				iresp=PICC_SOFErr;
				//printfx("SOF err...[%02x] %d\r\n", tmpc,i);
				//printfx("SOF err...[%02x] %d %02x %02x\r\n", tmpc,i,s_ReadRegRc523(rControlReg),s_ReadRegRc523(rStatus2Reg));
				goto X_TAIL;
			}//SOF incorrect  7
		}
		//if(tmpc&0x04)
#endif //FAKE_FOR_TEST
	//if(tmpc&0x02)
	if(tmpc&0x02)
	{
		iresp=PICC_ParityErr;
		printf("Parity err...\r\n");
		goto X_TAIL;
	}//parity error   8
	//if(action==PCD_LOADKEY && tmpc&0x40)
	//{
	//	iresp=PICC_KeyFormatErr;
	//	printf("Key Format err...\r\n");
	//	goto X_TAIL;
	//}//key format error  9

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
		//	s_SetBitRegRc523(rControlReg ,0x80);
			
		//	if(g_FDT_Flag==FDT_TYPE_2) {
			//	s_ClrBitRegRc523(rTModeReg, 0x0f);
			//	s_SetBitRegRc523(rTModeReg, 0x03);
			//	s_WriteRegRc523(rTPrescalerReg ,0xff);	   
				
			//	s_WriteRegRc523(rTReloadRegH ,((FWTactivation+Trecovery)/(0x3ff+1))/0xff);
			//	s_WriteRegRc523(rTReloadRegL ,((FWTactivation+Trecovery)/(0x3ff+1))%0xff); 
		//	}

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
		//	s_SetBitRegRc523(rControlReg ,0x80);
			
		//	if(g_FDT_Flag==FDT_TYPE_2) {
		/*		s_ClrBitRegRc523(rTModeReg, 0xff);
				s_SetBitRegRc523(rTModeReg, 0x03);
				s_WriteRegRc523(rTPrescalerReg ,0xff);	   
				
				s_WriteRegRc523(rTReloadRegH ,((FWTactivation+Trecovery)/(0x3ff+1))/0xff);
				s_WriteRegRc523(rTReloadRegL ,((FWTactivation+Trecovery)/(0x3ff+1))%0xff); 
		*///	}

			s_SetBitRegRc523(rControlReg ,0x40);
			
			printfx("%d:%d\r\n",__LINE__,iresp);



			s_WriteRegRc523(rCommandReg,PCD_RECEIVE); 

			
			goto Rx_again;

		}
	}

//printfx("%d:%d\r\n",__LINE__,iresp);


out:

	//s_WriteRegRc523(rPage,pInterruptRq);
	//s_WriteRegRc523(rInterruptRq,0x3f);     //clear all interrupt flags
	//s_WriteRegRc523(rInterruptEn,0x3f);     //close all interrupts at first     
	//s_WriteRegRc523(rComIEnReg,0x7f);//close all pcd interrupts
//	s_WriteRegRc523(rComIEnReg,0x80);//close all pcd interrupts
	s_WriteRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
//	s_ClrBitRegRc523(rComIrqReg,0x7f);//clear all pcd interrupts 
	//s_WriteRegRc523(rDivIEnReg,0x14);//close all pcd interrupts
//	s_WriteRegRc523(rDivIEnReg,0x00);//close all pcd interrupts
//	s_WriteRegRc523(rDivIrqReg,0x14);//clear all pcd interrupts  

	//s_WriteRegRc523(rPage,pCommand);
	s_WriteRegRc523(rCommandReg,0X00);
	s_ClrBitRegRc523(rBitFramingReg,0x80);
	
	if((iresp!=PICC_OK)&&(iresp!=PICC_RxTimerOut))
		printfx("%d:%d\r\n",__LINE__,iresp);
		

	return iresp;
}

int TestPicc(void)
{

	uchar ch, blkNo, cardTpye, tmp, opMode, argRspCardType[2];
	uchar snLen, snBuf[100], blkBuf[20], inBuf[16];
	uchar addBuf[4] = {0x01,0x00,0x00,0x00};
	uchar ucTmp[10], pack[16]; //钱包
	int i, n,iRet, j;
	long lBlance, lBlanceRev;
	uchar k;
	
	APDU_SEND apdusend;
	APDU_RESP apduresp;

	blkNo = 3;
	pack[0] = 0x44;
	pack[1] = 0x00;
	pack[2] = 0x00;
	pack[3] = 0x00;

	pack[4] = 0xbb;
	pack[5] = 0xff;
	pack[6] = 0xff;
	pack[7] = 0xff;

	pack[8] = 0x44;
	pack[9] = 0x00;
	pack[10] = 0x00;
	pack[11] = 0x00;

	pack[12] = blkNo;
	pack[13] = ~blkNo;
	pack[14] = blkNo;
	pack[15] = ~blkNo;


	//s_PiccInit();

	while(1)
	{
		memset(snBuf, 0, sizeof(snBuf));
		memset(argRspCardType, 0, sizeof(argRspCardType));
		memset(blkBuf, 0, sizeof(blkBuf));
		memset(inBuf, 0x01, sizeof(inBuf));
		//memcpy(pack, (uchar *)"\x00\x00\x00\x01\xff\xff\xff\xfe\x00\x00\x00\x01\x01\xfE\x02\xfe", 16);

		info_printf("\r\n");
		info_printf("==============PCD TEST FUN==============\r\n");
		info_printf("1-Open        2-Close		  3-Detect     \r\n");
		info_printf("4-M1Card      5-M1Card(1)      6-Detect(A)   \r\n");
		info_printf("7-Detect(B)   8-HALT          C-SPI      \r\n");
		info_printf("M-M1 card     A-A CPU		  B-B CPU   \r\n");
		info_printf("========================================\r\n");
		ch=s_getkey(COM_DEBUG);
		switch(ch)
		{
		case 0x1b:return ;
		case 'o':
		case 'O':
			info_printf("\r\nopen( )...\r\n");
			iRet=Lib_PiccOpen();
			info_printf("\r\nopen()=%d...\r\n",iRet);
			break;
		case 'c':
		case 'C':
			while (1) {
				PICC_CARRIER_ON();
				Lib_DelayMs(1000);
				PICC_CARRIER_OFF();
				Lib_DelayMs(1000);
			}
			while (1) {
//				info_printf("CmdReg:%02x ", s_ReadRegRc523(rCommandReg));
//				info_printf("IEnReg:%02x\r\n", s_ReadRegRc523(rComIEnReg));
				for (k=0; k<255; k++) {
					s_WriteRegRc523(rComIEnReg, k);
					s_ReadRegRc523(rComIEnReg);
					Lib_DelayMs(1000);
				}
			}
			info_printf("\r\nclose( )...\r\n");
			iRet=Lib_PiccClose();
			info_printf("\r\nclose()=%d...\r\n",iRet);
			break;
		case '1':
			Lib_PiccOpen();
			break;
		case '3':
//			while (1)
				Picc_Check2('A', argRspCardType, snBuf,3);
			break;
		case 'A':
		case 'a':	
			iRet=Lib_PiccOpen();
//			info_printf("\r\nDetect Card(A)...\r\n");
			iRet = Picc_Check2('A', argRspCardType, snBuf, 3);
			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xa4;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=2;
			apdusend.Le=256;
			memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);
			iRet = Lib_PiccCommand(&apdusend, &apduresp);
			if(!iRet)
			{
				info_printf("Select File:\r\n");
				for(i=0;i<apduresp.LenOut;i++) 
					info_printf("%02x ", apduresp.DataOut[i]);
				info_printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
				info_printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
				break;
			} 

			Lib_PiccHalt();

			Lib_PiccClose();

			break;	
		}
	}
}
#if 0
int TestPicc(void)
{

	uchar ch, blkNo, cardTpye, tmp, opMode, argRspCardType[2];
	uchar snLen, snBuf[100], blkBuf[20], inBuf[16];
	uchar addBuf[4] = {0x01,0x00,0x00,0x00};
	uchar ucTmp[10], pack[16]; //钱包
	int i, n,iRet, j;
	long lBlance, lBlanceRev;

	APDU_SEND apdusend;
	APDU_RESP apduresp;

	blkNo = 3;
	pack[0] = 0x44;
	pack[1] = 0x00;
	pack[2] = 0x00;
	pack[3] = 0x00;

	pack[4] = 0xbb;
	pack[5] = 0xff;
	pack[6] = 0xff;
	pack[7] = 0xff;

	pack[8] = 0x44;
	pack[9] = 0x00;
	pack[10] = 0x00;
	pack[11] = 0x00;

	pack[12] = blkNo;
	pack[13] = ~blkNo;
	pack[14] = blkNo;
	pack[15] = ~blkNo;


	s_PiccInit();

	while(1)
	{
		memset(snBuf, 0, sizeof(snBuf));
		memset(argRspCardType, 0, sizeof(argRspCardType));
		memset(blkBuf, 0, sizeof(blkBuf));
		memset(inBuf, 0x01, sizeof(inBuf));
		//memcpy(pack, (uchar *)"\x00\x00\x00\x01\xff\xff\xff\xfe\x00\x00\x00\x01\x01\xfE\x02\xfe", 16);

		printf("\r\n");
		printf("==============PCD TEST FUN==============\r\n");
		printf("1-Open        2-Close		  3-Detect     \r\n");
		printf("4-M1Card      5-M1Card(1)      6-Detect(A)   \r\n");
		printf("7-Detect(B)   8-HALT          C-SPI      \r\n");
		printf("M-M1 card     A-A CPU		  B-B CPU   \r\n");
		printf("========================================\r\n");
		ch=s_getkey(COM_DEBUG);
		switch(ch)
		{
		case 'o':
		case 'O':
			printf("\r\nopen( )...\r\n");
			iRet=Lib_PiccOpen();
			printf("\r\nopen()=%d...\r\n",iRet);
			break;
		case 'c':
		case 'C':
			while (1) {
				printf("CmdReg:%02x ", s_ReadRegRc523(rCommandReg));
				printf("IEnReg:%02x\r\n", s_ReadRegRc523(rComIEnReg));
				Lib_DelayMs(1000);
			}
			printf("\r\nclose( )...\r\n");
			iRet=Picc_Close();
			printf("\r\nclose()=%d...\r\n",iRet);
			break;
		case 'd':
		case 'D':
			printf("\r\nPicc_Check(A)...\r\n");
			iRet = Picc_Check('A', argRspCardType, snBuf);
			printf("\r\nPicc_Check(A)=%d...\r\n",iRet);
			break;
		case 'r':
		case 'R':
			printf("\r\nib_PiccReset( )...\r\n");
			iRet = Picc_Reset();
			printf("\r\nib_PiccReset()=%d...\r\n",iRet);
			break;
		case 'h':
		case 'H':
			printf("\r\nPicc_Halt(0)...\r\n");
			Lib_PiccHalt();;
			printf("\r\nPicc_Halt(1)=%d...\r\n");
			break;

		case 'm':
		case 'M':
			iRet=Lib_PiccOpen();
			//DelayMs(500);
			printf("\r\nDetect Card(A)...\r\n");

			iRet = Picc_Check('M', argRspCardType, snBuf);
			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			}
			else
			{
				printf("Picc_Check(M)=%d \r\n", iRet);
			}
			Picc_Close();
			break;

		case 'n': 
			n=0;
			
			for (i=0; i<500; i++)
			{
				iRet=Lib_PiccOpen();
				printf("\r\nDetect Card(A)...\r\n");

				iRet = Picc_Check('M', argRspCardType, snBuf);
				if (!iRet)
				{
					n++;

					printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
				}
				else
				{
					printf("Picc_Check(M)=%d \r\n", iRet);
				}
				Picc_Close(); 
			}

			
			printf("ok[%d]\r\n", n);

			break;
 
		case 'a':
		case 'A':
			iRet=Lib_PiccOpen();
			printf("\r\nDetect Card(A)...\r\n");

			iRet = Picc_Check('A', argRspCardType, snBuf);
			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			}
			else
			{
				printf("Picc_Check(M)=%d \r\n", iRet);
			}

			if (argRspCardType[0] == 'A')
			{
				iRet = Picc_Reset();
				if (!iRet)
				{
					printf("Reset Card Ok!\r\n");
				}
				else
				{
					printf("Reset Card Err, Picc_Reset= %d\r\n", iRet);
					break;
				}
			}
			else
				break;

			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xa4;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=2;
			apdusend.Le=256;
			memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			printf("Command picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			printf("time(%d)\r\n",CheckTimer(0));
			if(!iRet)
			{
				printf("Select File:\r\n");
				for(i=0;i<apduresp.LenOut;i++) 
					printf("%02x ", apduresp.DataOut[i]);
				printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
				printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
				break;
			} 

			Lib_PiccHalt();

			Picc_Close();

			break;

		case 'b':
		case 'B':
			iRet=Lib_PiccOpen();
			printf("\r\nDetect Card(B)...\r\n");

			iRet = Picc_Check('B', argRspCardType, snBuf);

			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);

			}
			else
			{
				printf("Reset Card Err, Picc_Reset= %d\r\n", iRet);
				break;
			}

			//break;
			SetTimer(0, 6000);

			for(n=0;n<5;n++)
			{

				apdusend.Command[0]=0x00;
				apdusend.Command[1]=0x84;
				apdusend.Command[2]=0x00;
				apdusend.Command[3]=0x00;
				apdusend.Lc=0;
				apdusend.Le=8;			        

				printf("Command get rnd picc(%d)...\r\n",n);

				iRet = Picc_Command(&apdusend, &apduresp);
				printf("time(%d)\r\n",CheckTimer(0));
				if(!iRet)
				{
					printf("Select File:\r\n");
					for(i=0;i<apduresp.LenOut;i++) 
						printf("%02x ", apduresp.DataOut[i]);
					printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
				}
				else
				{
					printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
					break;
				} 
			} 

			Picc_Close();
			break;

		case '1':
			iRet=Picc_Close();
			/*
			iRet=Lib_PiccOpen();
			printf("\r\nDetect Card(A)...\r\n");

			iRet = Picc_Check('A', argRspCardType, snBuf);
			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			}
			else
			{
				printf("Picc_Check=%d \r\n", iRet);
			}


			//iRet = Picc_Reset();
			//if (!iRet)
			//{
			//	printf("Reset Card Ok!\r\n");
			//}
			//else
			//{
			//	printf("Reset Card Err, Picc_Reset= %d\r\n", iRet);
			//	break;
			//}
			*/
			blkNo = 4;
			printf("Open PCD...\r\n");
			iRet=Lib_PiccOpen();
			if (iRet)
			{
			printf("Open PCD Err! Lib_PiccOpen=%d\r\n", iRet);
			break;
			}
			printf("\r\nDetect Card(M)...\r\n");

			iRet = Picc_Check('M', argRspCardType, snBuf);
			if (iRet)
			{
			printf("Find M1 Card Err! Picc_Check=%d\r\n", iRet);
			break;
			}
			printf("\r\nIt is M1 Card\r\n");

			//M1卡验证扇区密码				
			iRet = Lib_PiccM1Authority('A', blkNo, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);
			if (iRet)
			{
			printf("Authority M1 Card Err! Lib_PiccM1Authority=%d\r\n", iRet);
			break;
			}
			printf("\r\nAuthority M1 Card OK!\r\n");

			// 				//读取扇区
			// 				iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);
			// 				if (iRet)
			// 				{
			// 					printf("\r\nM1 Card Read Block Err1 Lib_PiccM1ReadBlock=%d\r\n", iRet);
			// 					break;
			// 				}
			// 				printf("\r\nM1 Card Read Block OK:\r\n");
			// 				for (i=0; i<16; i++)
			// 				{
			// 					printf("%02x ", blkBuf[i]);
			// 				}
			// 				printf("\r\nBlock Buffer End!\r\n");


			//写扇区
			iRet = Lib_PiccM1WriteBlock(blkNo, pack);
			if (iRet)
			{
			printf("\r\nM1 Card Write Block Err! Lib_PiccM1WriteBlock=%d\r\n", iRet);
			break;
			}
			printf("M1 Card Write Block OK!\r\n");

			//读取扇区
			iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);
			if (iRet)
			{
			printf("\r\nM1 Card Read Block Err1 Lib_PiccM1ReadBlock=%d\r\n", iRet);
			break;
			}
			printf("\r\nM1 Card Read Block OK:\r\n");
			for (i=0; i<16; i++)
			{
			printf("%02x ", blkBuf[i]);
			}
			printf("\r\nBlock Buffer End!\r\n");
			

			break;
		case '2':
			Picc_Close();
			blkNo = 4;
			printf("Open PCD2...\r\n");
			iRet=Lib_PiccOpen();
			if (iRet)
			{
				printf("Open PCD Err! Lib_PiccOpen=%d\r\n", iRet);
				break;
			}
			printf("\r\nDetect Card(M)...\r\n");

			iRet = Picc_Check('M', argRspCardType, snBuf);
			if (iRet)
			{
				printf("Find M1 Card Err! Picc_Check=%d\r\n", iRet);
				break;
			}
			printf("\r\nIt is M1 Card\r\n");

			//M1卡验证扇区密码				
			iRet = Lib_PiccM1Authority('A', blkNo, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);
			if (iRet)
			{
				printf("Authority M1 Card Err! Lib_PiccM1Authority=%d\r\n", iRet);
				break;
			}
			printf("\r\nAuthority M1 Card OK!\r\n");

			n = 0;
			for (j=0; j<500; j++)
			{
				//读取扇区 
				iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);
				if (iRet)
				{
					printf("\r\nM1 Card Read Block Err1 Lib_PiccM1ReadBlock=%d\r\n", iRet);
					Picc_Check('M', argRspCardType, snBuf);
					Lib_PiccM1Authority('A', blkNo, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);
					continue;
				}
				n++;
				printf("\r\nM1 Card Read Block OK[]:\r\n");
				for (i=0; i<16; i++)
				{
					printf("%02x ", blkBuf[i]);
				}
				printf("\r\nBlock Buffer End!\r\n");

			}


			printf("\r\nM1 Card Read Block OK[%d]:\r\n", n);
		    break;

 
			//写扇区
			iRet = Lib_PiccM1WriteBlock(blkNo, (uchar *)"\x01\x02\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff");
			if (iRet)
			{
				printf("\r\nM1 Card Write Block Err! Lib_PiccM1WriteBlock=%d\r\n", iRet);
				break;
			}
			printf("M1 Card Write Block OK!\r\n");

			//读取扇区
			iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);
			if (iRet)
			{
				printf("\r\nM1 Card Read Block Err1 Lib_PiccM1ReadBlock=%d\r\n", iRet);
				break;
			}
			printf("\r\nM1 Card Read Block OK:\r\n");
			for (i=0; i<16; i++)
			{
				printf("%02x ", blkBuf[i]);
			}
			printf("\r\nBlock Buffer End!\r\n");

			/*
			SetTimer(0, 6000);
			for(n=0;n<5000;n++)
			{

				apdusend.Command[0]=0x00;
				apdusend.Command[1]=0xa4;
				apdusend.Command[2]=0x00;
				apdusend.Command[3]=0x00;
				apdusend.Lc=2;
				apdusend.Le=256;
				memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

				DelayMs(1000);

				printf("Command picc(%d)...\r\n",n);

				iRet = Picc_Command(&apdusend, &apduresp);
				printf("time(%d)\r\n",CheckTimer(0));
				if(!iRet)
				{
					printf("Select File:\r\n");
					for(i=0;i<apduresp.LenOut;i++) 
						printf("%02x ", apduresp.DataOut[i]);
					printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
				}
				else
				{
					printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
					break;
				} 
			}
			*/
			/*
			iRet=Picc_Close();
			iRet=Lib_PiccOpen();
			printf("Picc_Open=%d\r\n",iRet);
			printf("Detect Card(M1)...\r\n");

			iRet = Picc_Check('M', argRspCardType, snBuf);
			if (iRet)
			{
			printf("Find M1 Card Err! Picc_Check=%d\r\n", iRet);
			break;
			}
			printf("\r\nIt is M1 Card\r\n");
			printf("Select the No block to Authority\r\n");
			ch = s_getkey(COM_DEBUG);
			blkNo = ch - '0';
			printf("\r\nAuthority Block No:[%d]\r\n", blkNo);
			printf("Authority Card (A)...\r\n");

			//M1卡验证扇区密码
			iRet = Lib_PiccM1Authority('A', blkNo, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);
			if (iRet)
			{
			printf("Authority M1 Card Err! Lib_PiccM1Authority=%d\r\n", iRet);
			break;
			}

			printf("\r\nSelect the No block to OP...\r\n");
			ch = s_getkey(COM_DEBUG);
			blkNo = ch - '0';
			printf("\r\nOP Block No:[%d]\r\n", blkNo);

			printf("\r\nSelect inc or dec: 1- '+'or 2- '-'\r\n");
			ch = s_getkey(COM_DEBUG);
			if (ch == '2')
			opMode = '-';
			else
			opMode = '+';

			//充钱、扣钱
			iRet = Lib_PiccM1Operate(opMode, blkNo, addBuf, blkNo);
			if (iRet)
			{
			printf("\r\nM1 Operate Err! Lib_PiccM1Operate=%d \r\n", iRet);
			break;
			}	
			printf("\r\nM1 Card Operate OK!Block No:[%d]\r\n", blkNo);
			printf("\r\nAuthority M1 Card OK!\r\n");

			iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);									
			if (iRet)
			{
			printf("\r\nErr! Lib_PiccM1ReadBlock=%d \r\n", iRet);
			break;
			}
			printf("\r\nM1 Card Read Block OK, Block Buffer[%d]:\r\n", blkNo);
			for (i=0; i<16; i++)
			{
			printf("%02x ", blkBuf[i]);
			}
			printf("\r\nBlock Buffer End!\r\n");
			printf("The End!\r\n");
			*/

			break;
		case '3':
			SetTimer(0, 6000);
			for(n=0;n<5000;n++)
			{

				apdusend.Command[0]=0x00;
				apdusend.Command[1]=0xa4;
				apdusend.Command[2]=0x00;
				apdusend.Command[3]=0x00;
				apdusend.Lc=2;
				apdusend.Le=256;
				memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

				DelayMs(2000);

				printf("Command picc(%d)...\r\n",n);

				iRet = Picc_Command(&apdusend, &apduresp);
				printf("time(%d)\r\n",CheckTimer(0));
				if(!iRet)
				{
					printf("Select File:\r\n");
					for(i=0;i<apduresp.LenOut;i++) 
						printf("%02x ", apduresp.DataOut[i]);
					printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
				}
				else
				{
					printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
					break;
				} 
			}
			/*
			iRet=Picc_Close();
			iRet=Picc_Open();
			printf("\r\nDetect Card...\r\n");

			iRet = Picc_Check(0, argRspCardType, snBuf);
			if (!iRet)
			{
			printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			if (argRspCardType[1]=='C' || argRspCardType[1]=='c')
			{
			if (argRspCardType[0]=='A' || argRspCardType[0]=='a')
			{
			printf("This is A CPU Card. Reset Card...\r\n");
			iRet = Picc_Reset();
			if (!iRet)
			{
			printf("Reset Card Ok!\r\n");
			}
			else
			{
			printf("Reset Card Err, Picc_Reset= %d\r\n", iRet);
			break;
			}
			}

			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xa4;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=2;
			apdusend.Le=256;
			memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			printf("Command picc...\r\n");

			iRet = Picc_Command(&apdusend, &apduresp);
			if(!iRet)
			{
			printf("Select File:\r\n");
			for(i=0;i<apduresp.LenOut;i++) 
			printf("%02x ", apduresp.DataOut[i]);
			printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
			printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
			} 
			}

			snLen = snBuf[0];
			printf("Card SN:(snLen=%d)\r\n", snLen);
			for (i=0; i<snLen; i++)
			{
			printf("%02x ", snBuf[i+1]);
			}					
			printf("SN End!\r\n");
			}
			else
			{
			printf("Fail check card! Picc_Check = %d\r\n", iRet);
			}
			*/

			break;
		case '4':
			/*
			SetTimer(0, 6000);
			//for(n=0;n<5000;n++)
			{

			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xa4;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=2;
			apdusend.Le=256;
			memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			//DelayMs(1000);

			printf("Command picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			printf("time(%d)\r\n",CheckTimer(0));
			if(!iRet)
			{
			printf("Select File:\r\n");
			for(i=0;i<apduresp.LenOut;i++) 
			printf("%02x ", apduresp.DataOut[i]);
			printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
			printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
			break;
			} 
			}
			*/

			iRet=Picc_Close();
			iRet=Picc_Open();
			printf("Picc_Open=%d\r\n",iRet);
			printf("Detect Card...\r\n");

			//寻卡
			iRet = Picc_Check('m', argRspCardType, snBuf);
			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
				snLen = snBuf[0];
				printf("Card SN:(snLen=%d)\r\n", snLen);
				for (i=0; i<snLen; i++)
				{
					printf("%02x ", snBuf[i+1]);
				}					
				printf("SN End!\r\n");
				if (argRspCardType[1]=='M' || argRspCardType[1]=='m')
				{//如果是M1卡
					printf("It's Mifare1 Card!\r\n");
					printf("Select the No block to Authority\r\n");
					ch = s_getkey(COM_DEBUG);
					blkNo = ch - '0';
					printf("\r\nAuthority Block No:[%d]\r\n", blkNo);
					printf("Authority Card (A)...\r\n");

					//snBuf[1]=
					//M1卡验证扇区密码
					iRet = Lib_PiccM1Authority('A', blkNo, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);
					if (!iRet)
					{//密码正确
						printf("\r\nAuthority Card OK\r\n");

						printf("\r\nSelect the No block to read...\r\n");
						ch = s_getkey(COM_DEBUG);
						blkNo = ch - '0';
						printf("\r\nRead Block No:[%d]\r\n", blkNo);

						//读取扇区
						//iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);
						iRet = 0;
						if (!iRet)
						{
							// 								printf("\r\nM1 Card Read Block OK, Block Buffer:\r\n");
							// 								for (i=0; i<16; i++)
							// 								{
							// 									printf("%02x ", blkBuf[i]);
							// 								}
							// 								printf("\r\nBlock Buffer End!\r\n");
							//printf("\r\nM1 Card Write Block Buffer...\r\n");
							//iRet = Lib_PiccM1WriteBlock(1, pack);
							iRet=0;
							if (!iRet)
							{
								//printf("\r\nM1 Card Write Block Buffer OK:\r\n");

								printf("\r\nSelect inc or dec: 1- '+', 2- '-'\r\n");
								ch = s_getkey(COM_DEBUG);
								if (ch == '2')
									opMode = '-';
								else
									opMode = '+';

								//充钱、扣钱
								iRet = Lib_PiccM1Operate(opMode, blkNo, addBuf, blkNo);
								if (!iRet)
								{
									printf("\r\nM1 Card Operate OK!\r\n");
								}
								else
								{
									printf("\r\nM1 Operate Err! Lib_PiccM1Operate=%d \r\n", iRet);
								}

								iRet = Lib_PiccM1ReadBlock(blkNo, blkBuf);

								if (!iRet)
								{
									printf("\r\nM1 Card Read Block OK, Block Buffer[%d]:\r\n", blkNo);
									for (i=0; i<16; i++)
									{
										printf("%02x ", blkBuf[i]);
									}
									printf("\r\nBlock Buffer End!\r\n");
									iRet = Lib_PiccM1ReadBlock(blkNo+1, blkBuf);
									printf("\r\nM1 Card Read Block OK, Block Buffer[%d]:\r\n", blkNo+1);
									for (i=0; i<16; i++)
									{
										printf("%02x ", blkBuf[i]);
									}
									printf("\r\nBlock Buffer End!\r\n");
								}
								else
								{
									printf("\r\nErr! Lib_PiccM1ReadBlock=%d \r\n", iRet);
								}
							}
							else
							{
								printf("\r\nErr! Lib_PiccM1WriteBlock=%d \r\n", iRet);
							}
						}
						else
						{
							printf("\r\nErr! Lib_PiccM1ReadBlock=%d \r\n", iRet);
						}
					}
					else
					{
						printf("\r\nErr! Lib_PiccM1Authority=%d \r\n", iRet);
					}
				}
			}
			else
			{
				printf("\r\nCheck Card Err! Picc_Check=%d \r\n", iRet);
			}


			break;

		case '5':
			g_Add=0;
			break;
		case '6':
			//for(;;)
			//{
			iRet=Picc_Close();
			iRet=Picc_Open();
			for(;;)
			{
				printf("\r\nDetect Card(A)...\r\n");

				iRet = Picc_Check('A', argRspCardType, snBuf);
				if (!iRet)
				{
					printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
				}
				else
				{
					printf("Picc_Check=%d \r\n", iRet);
					break;
				}


				iRet = Picc_Reset();
				if (!iRet)
				{
					printf("Reset Card Ok!\r\n");
				}
				else
				{
					printf("Reset Card Err, Picc_Reset= %d\r\n", iRet);
					break;
				}

				//void   SetTimer(uchar TimerNo, ushort Cnt100ms);
				//ushort CheckTimer(uchar TimerNo);
				SetTimer(0, 6000);
				for(n=0;n<1;n++)
				{

					apdusend.Command[0]=0x00;
					apdusend.Command[1]=0xa4;
					apdusend.Command[2]=0x00;
					apdusend.Command[3]=0x00;
					apdusend.Lc=2;
					apdusend.Le=256;
					memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

					printf("Command picc(%d)...\r\n",n);

					iRet = Picc_Command(&apdusend, &apduresp);
					printf("time(%d)\r\n",CheckTimer(0));
					if(!iRet)
					{
						printf("Select File:\r\n");
						for(i=0;i<apduresp.LenOut;i++) 
							printf("%02x ", apduresp.DataOut[i]);
						printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
					}
					else
					{
						printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
						break;
					} 
				}
				Lib_PiccHalt();
			}


			/*
			for(n=0;n<100;n++)
			{

			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xb0;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=0;
			apdusend.Le=256;
			//memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			printf("Command picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			if(!iRet)
			{
			printf("read File:\r\n");
			for(i=0;i<apduresp.LenOut;i++) 
			printf("%02x ", apduresp.DataOut[i]);
			printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
			printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
			break;
			} 
			} 
			*/


			break;
		case '7':
			iRet=Picc_Close();
			iRet=Picc_Open();
			printf("\r\nDetect Card(B)...\r\n");

			iRet = Picc_Check('B', argRspCardType, snBuf);

			g_Add++;
			if(g_Add>0x3f) g_Add=0;

			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);

			}
			else break;
			//break;
			SetTimer(0, 6000);

			for(n=0;n<5;n++)
			{

				apdusend.Command[0]=0x00;
				apdusend.Command[1]=0x84;
				apdusend.Command[2]=0x00;
				apdusend.Command[3]=0x00;
				apdusend.Lc=0;
				apdusend.Le=8;			        

				printf("Command get rnd picc(%d)...\r\n",n);

				iRet = Picc_Command(&apdusend, &apduresp);
				printf("time(%d)\r\n",CheckTimer(0));
				if(!iRet)
				{
					printf("Select File:\r\n");
					for(i=0;i<apduresp.LenOut;i++) 
						printf("%02x ", apduresp.DataOut[i]);
					printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
				}
				else
				{
					printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
					break;
				} 
			}
			//Lib_PiccHalt();
			/*
			for(n=0;n<100;n++)
			{

			apdusend.Command[0]=0xa0;
			apdusend.Command[1]=0xa4;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=2;
			apdusend.Le=256;
			memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			printf("Command picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			printf("time(%d)\r\n",CheckTimer(0));
			if(!iRet)
			{
			printf("Select File:\r\n");
			for(i=0;i<apduresp.LenOut;i++) 
			printf("%02x ", apduresp.DataOut[i]);
			printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
			printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
			break;
			} 
			}
			*/
			//Lib_PiccHalt();

			/*

			for(n=0;n<100;n++)
			{

			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0xb0;
			apdusend.Command[2]=0x01;
			apdusend.Command[3]=0x00;
			apdusend.Lc=0;
			apdusend.Le=256;
			//memcpy(apdusend.DataIn,"\x3f\x00",apdusend.Lc);

			printf("Command picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			printf("time(%d)\r\n",CheckTimer(0));
			if(!iRet)
			{
			printf("read File:\r\n");
			for(i=0;i<apduresp.LenOut;i++) 
			printf("%02x ", apduresp.DataOut[i]);
			printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
			printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
			break;
			} 
			}
			*/


			break;
		case '8':
			//iRet = Lib_PiccRemove();
			//printf("Lib_PiccRemove=%d \r\n", iRet);

			iRet=Picc_Close();
			iRet=Picc_Open();
			printf("\r\nDetect Card(A)...\r\n");

			iRet = Picc_Check('B', argRspCardType, snBuf);
			g_Add++;
			if(g_Add>0x3f) g_Add=0;
			if (!iRet)
			{
				printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			}
			else
			{
				printf("Picc_Check=%d \r\n", iRet);
			}


			break;
		case '9':
			/*
			iRet=Picc_Close();
			iRet=Picc_Open();
			printf("Picc_Open=%d\r\n",iRet);
			printf("Detect Card...\r\n");

			//寻卡
			iRet = Picc_Check('M', argRspCardType, snBuf);
			if (!iRet)
			{
			printf("Card Type is: [%c,%c]\r\n", argRspCardType[0], argRspCardType[1]);
			if (argRspCardType[1]=='M' || argRspCardType[1]=='m')
			{//如果是M1卡
			printf("It's Mifare1 Card!\r\n");

			//M1卡验证扇区密码
			//iRet = Lib_PiccM1Authority('A', 7, (uchar *)"\x00\x00\x00\x00\x00\x01", &snBuf[1]);	//块7
			iRet = Lib_PiccM1Authority('A', 4, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);	//块7
			if (!iRet)
			{//密码正确
			printf("\r\nAuthority blkNo4 is Card OK\r\n");
			iRet = Lib_PiccM1ReadBlock(4, blkBuf);
			if (!iRet)
			{
			printf("\r\nM1 Card Read Block OK, blkNo4:\r\n");
			for (i=0; i<16; i++)
			{
			printf("%02x ", blkBuf[i]);
			}
			printf("\r\nblkNo4 End!\r\n");
			}
			else{
			printf("\r\nRead blkNo4 err!\r\n");
			}

			iRet = Lib_PiccM1WriteBlock(4, (uchar *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01");
			if (!iRet)
			{			
			Lib_PiccM1Authority('A', 4, (uchar *)"\xff\xff\xff\xff\xff\xff", &snBuf[1]);	//块7
			iRet = Lib_PiccM1ReadBlock(4, blkBuf);
			if (!iRet)
			{
			printf("\r\nM1 Card Read Block OK, blkNo4:\r\n");
			for (i=0; i<16; i++)
			{
			printf("%02x ", blkBuf[i]);
			}
			printf("\r\nblkNo4 End!\r\n");
			}
			else{
			printf("\r\nRead blkNo4 err!\r\n");
			}
			}
			else
			{
			printf("\r\nErr! Lib_PiccM1WriteBlock=%d \r\n", iRet);
			}
			}
			else
			{
			printf("\r\nErr! Lib_PiccM1Authority=%d \r\n", iRet);
			}
			}
			}
			else
			{
			printf("\r\nCheck Card Err! Picc_Check=%d \r\n", iRet);
			}
			*/
			apdusend.Command[0]=0x00;
			apdusend.Command[1]=0x84;
			apdusend.Command[2]=0x00;
			apdusend.Command[3]=0x00;
			apdusend.Lc=0;
			apdusend.Le=8;			        

			printf("Command get rnd picc(%d)...\r\n",n);

			iRet = Picc_Command(&apdusend, &apduresp);
			printf("time(%d)\r\n",CheckTimer(0));
			if(!iRet)
			{
				printf("Select File:\r\n");
				for(i=0;i<apduresp.LenOut;i++) 
					printf("%02x ", apduresp.DataOut[i]);
				printf("\r\nCommand picc Ok! SWA=%02x SWB=%02x\r\n",apduresp.SWA,apduresp.SWB);                   
			}
			else
			{
				printf("Command picc Err! Picc_Command()=%d \r\n", iRet);
				break;
			} 
			break;
		case 'q'://
		case 'Q':
			return 0;
		default:
			break;
		}
	}

	return 0;
}

#if 0
#define PICC_WAIT_TIME   15  //单位秒

int Test_Picc(void) 
{
	BYTE snbuf[100],tmps[300],buf[300],ch, add;
	int iret,i;
	int okCnt,failCnt, cnt;

	APDU_SEND ApduSend;
	APDU_RESP ApduResp; 


	uchar  blkNo, odd;

	uchar addBuf[4] = {0x01,0x00,0x00,0x00};
	uchar ucTmp[10], pack[16]; //钱包



	blkNo = 3;
	pack[0] = 0x44;
	pack[1] = 0x00;
	pack[2] = 0x00;
	pack[3] = 0x00;

	pack[4] = 0xbb;
	pack[5] = 0xff;
	pack[6] = 0xff;
	pack[7] = 0xff;

	pack[8] = 0x44;
	pack[9] = 0x00;
	pack[10] = 0x00;
	pack[11] = 0x00;

	pack[12] = blkNo;
	pack[13] = ~blkNo;
	pack[14] = blkNo;
	pack[15] = ~blkNo;


LOOP:
	while(1)
	{
		memset(snbuf,0,sizeof(snbuf));
		memset(tmps,0,sizeof(tmps));

		Lcd_Cls();
		Lcd_Printxy(0,0,0x81,"   PICC TEST    ");
		Lcd_Printxy(0,2,0x01,"   1-TEST A     ");
		Lcd_Printxy(0,4,0x01,"   2-TEST M1    ");
		Lcd_Printxy(0,6,0x01,"   3-TEST B     ");
		Lcd_DrawBox(0,0,63, 127);

		Kb_Flush();
		ch=Kb_GetKey();
		if(ch==KEYCANCEL) 
		{
			Picc_Close();
			return 1;
		}

		iret = Picc_Open();
		if (iret)
		{
			Lcd_ClrLine(2,8);
			Lcd_Printxy(0,3,0x01," Open PED Failer! "); 
			Kb_Flush();
			Kb_GetKey();
			goto LOOP;
		}

		switch(ch)
		{
		case '3':
			Lcd_ClrLine(2,8);
			SetTimer(2, PICC_WAIT_TIME*10);
			while(1)
			{
				if (!Kb_Hit())
				{
					if (Kb_GetKey()==KEYCANCEL)
					{
						goto LOOP;
					}
				}
				iret=Picc_Check('B',0x00, snbuf); 
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01," Waiting Card... "); 
				}
				else
					break;

				if (!CheckTimer(2))
				{
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,4,0x01,"    NO CARD     "); 
					Beef(3, 1000);	 
					ch=Kb_GetKey();
					if(ch==KEYENTER) continue;
					goto LOOP;
				}
			}


			ApduSend.Command[0]=0x00;
			ApduSend.Command[1]=0xa4;
			ApduSend.Command[2]=0x00;
			ApduSend.Command[3]=0x00;
			ApduSend.Lc=2;
			ApduSend.Le=256;
			memcpy(ApduSend.DataIn,"\x3f\x00",ApduSend.Lc);

			okCnt = cnt = failCnt = 0;
			for (;;)
			{
				iret=Picc_Command(&ApduSend, &ApduResp);	  
				if (iret==0)
				{
					okCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"%d---Success--%d-", failCnt, okCnt);
					Lcd_Gotoxy(0,3);
					Lcd_SetFont(0);
					for(i=0;i<ApduResp.LenOut;i++)
						Lcd_printf("%02x ",ApduResp.DataOut[i]);
					Lcd_printf("%02x ",ApduResp.SWA);
					Lcd_printf("%02x ",ApduResp.SWB);
				}
				else 
				{
					failCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"%d---Failing----%d", failCnt, okCnt);
					Lcd_Printxy(0,3,0,"[ERROR] %d",iret);
				}

				if (!Kb_Hit())
				{
					if(Kb_GetKey()==KEYCANCEL)
						break;
				}
			}
			ch=Kb_GetKey();
			Picc_Close();
			break;

		case '1':
			Lcd_ClrLine(2,8);
			SetTimer(2, PICC_WAIT_TIME*10);
			while(1)
			{
				if (!Kb_Hit())
				{
					if (Kb_GetKey()==KEYCANCEL)
					{
						goto LOOP;
					}
				}

				iret=Picc_Check('A',0x00, snbuf); 
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01," Waiting Card... "); 
				}
				else
					break;

				if (!CheckTimer(2))
				{
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,4,0x01,"    NO CARD     "); 
					Beef(3, 1000);	 
					ch=Kb_GetKey();
					if(ch==KEYENTER) continue;
					goto LOOP;
				}
			}

			iret=Picc_Reset();
			if(iret)  
			{
				Lcd_ClrLine(2,8);
				Lcd_Printxy(0,4,0x01," RESET ERROR "); 
				Beef(3, 1000);   
				ch=Kb_GetKey();
				if(ch==KEYENTER) continue;
				return ;
			}

			ApduSend.Command[0]=0x00;  
			ApduSend.Command[1]=0xa4;  
			ApduSend.Command[2]=0x04;  
			ApduSend.Command[3]=0x00; 
			ApduSend.Lc=0x0e;          
			ApduSend.Le=256;           
			memcpy(ApduSend.DataIn,"1PAY.SYS.DDF02",14);

			okCnt = cnt = failCnt = 0;
			for (;;)
			{
				iret=Picc_Command(&ApduSend, &ApduResp);	  
				if (iret==0)
				{
					okCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"%d---Success--%d-", failCnt, okCnt);
					Lcd_Gotoxy(0,3);
					Lcd_SetFont(0);
					for(i=0;i<ApduResp.LenOut;i++)
						Lcd_printf("%02x ",ApduResp.DataOut[i]);
					Lcd_printf("%02x ",ApduResp.SWA);
					Lcd_printf("%02x ",ApduResp.SWB);
				}
				else 
				{
					failCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"%d---Failing----%d", failCnt, okCnt);
					Lcd_Printxy(0,3,0,"[ERROR] %d",iret);
				}


				if (!Kb_Hit())
				{
					if(Kb_GetKey()==KEYCANCEL)
						break;
				}
			}


			ch=Kb_GetKey();
			Picc_Close();

			break;

		case '2':
			Lcd_ClrLine(2,8);
			SetTimer(2, PICC_WAIT_TIME*10);
			while(1)
			{
				if (!Kb_Hit())
				{
					if (Kb_GetKey()==KEYCANCEL)
					{
						goto LOOP;
					}
				}

				iret=Picc_Check('M',0x00, snbuf); 
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01," Waiting Card... "); 
				}
				else
					break;

				if (!CheckTimer(2))
				{
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,4,0x01,"    NO CARD     "); 
					Beef(3, 1000);	 
					ch=Kb_GetKey();
					if(ch==KEYENTER) continue;
					goto LOOP;
				}
			}


			iret=Lib_PiccM1Authority('A',9,(BYTE *)"\xff\xff\xff\xff\xff\xff", &snbuf[1]);
			//iret=Lib_PiccM1Authority('A',9,(BYTE *)"\x00\x00\x00\x00\x00\x00", &snbuf[1]);
			if(iret)
			{
				Lcd_ClrLine(2,8);
				Lcd_Printxy(0,3,0x01,"   M1 Card  "); 
				Lcd_Printxy(0,5,0x01,"  AUTH M1 ERR "); 
				Beef(3, 1000);	 
				Kb_GetKey();
				goto LOOP;
			}

			okCnt = cnt = failCnt = 0;
			for (;;)
			{
				//Picc_Open();
				//iret=Picc_Check('M',0x00, snbuf); 

				//iret=Lib_PiccM1Authority('A',9,(BYTE *)"\xff\xff\xff\xff\xff\xff", &snbuf[1]);

				cnt++;
				iret=Lib_PiccM1ReadBlock(9, tmps);
				if(iret==0)
				{
					okCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"--%d Read M1 S %d--", failCnt, okCnt);
					Lcd_Gotoxy(0,3);
					Lcd_SetFont(0);
					for(i=0;i<16;i++)
						Lcd_printf("%02x ",tmps[i]);	
					//Kb_GetKey();
					//Picc_Close();
					//goto LOOP;			
				}
				else 
				{
					failCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"--%d Read M1 F  %d--", failCnt, okCnt);
					Lcd_Printxy(0,3,0,"[ERROR] %d",iret);
					Lcd_DrawBox(0,0,63,127);
					//Kb_GetKey();
					//continue;
				}

				if (!Kb_Hit())
				{
					if(Kb_GetKey()==KEYCANCEL)
						break;
				}
				//Picc_Close();
			}

			break;


		case KEY4:
			Lcd_ClrLine(2,8);
			SetTimer(2, PICC_WAIT_TIME*10);
			while(1)
			{
				if (!Kb_Hit())
				{
					if (Kb_GetKey()==KEYCANCEL)
					{
						goto LOOP;
					}
				}

				iret=Picc_Check('M',0x00, snbuf); 
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01," Waiting Card... "); 
				}
				else
					break;

				if (!CheckTimer(2))
				{
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,4,0x01,"    NO CARD     "); 
					Beef(3, 1000);	 
					ch=Kb_GetKey();
					if(ch==KEYENTER) continue;
					goto LOOP;
				}
			}


			iret=Lib_PiccM1Authority('A',9,(BYTE *)"\xff\xff\xff\xff\xff\xff", &snbuf[1]);
			//iret=Lib_PiccM1Authority('A',9,(BYTE *)"\x00\x00\x00\x00\x00\x00", &snbuf[1]);
			if(iret)
			{
				Lcd_ClrLine(2,8);
				Lcd_Printxy(0,3,0x01,"   M1 Card  "); 
				Lcd_Printxy(0,5,0x01,"  AUTH M1 ERR "); 
				Beef(3, 1000);	 
				Kb_GetKey();
				goto LOOP;
			}

			okCnt = cnt = failCnt = 0;
			for (;;)
			{
				Picc_Open();

				iret=Picc_Check('M',0x00, snbuf); 
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01,"Picc_Check=%d ", iret); 
					Kb_GetKey();
				}

				iret=Lib_PiccM1Authority('A',9,(BYTE *)"\xff\xff\xff\xff\xff\xff", &snbuf[1]);
				if(iret)  
				{
					Lcd_Printxy(0,3,0x01,"Lib_PiccM1Authority=%d ", iret); 
					Kb_GetKey();
				}


				add++;
				memset(buf, add, sizeof(buf));
				iret=Lib_PiccM1WriteBlock(9, buf);
				if(iret)
				{
					Lcd_Printxy(0,3,0x01,"Lib_PiccM1WriteBlock=%d ", iret); 
					Kb_GetKey();
				}


				memset(tmps, 0, sizeof(tmps));

				cnt++;
				iret=Lib_PiccM1ReadBlock(9, tmps);
				if(iret==0)
				{
					okCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"--%d Read M1 S %d--", failCnt, okCnt);
					Lcd_Gotoxy(0,3);
					Lcd_SetFont(0);
					for(i=0;i<16;i++)
						Lcd_printf("%02x ",tmps[i]);

					if (memcmp(buf, tmps, 16))
					{
						Lcd_Printxy(0,3,0x01,"ERR!!!!!!"); 
						Kb_GetKey();
					}
					//Kb_GetKey();
					//Picc_Close();
					//goto LOOP;			
				}
				else 
				{
					failCnt++;
					Lcd_ClrLine(2,8);
					Lcd_Printxy(0,2,0,"--%d Read M1 F  %d--", failCnt, okCnt);
					Lcd_Printxy(0,3,0,"[ERROR] %d",iret);
					Lcd_DrawBox(0,0,63,127);
					Kb_GetKey();
					//continue;
				}

				Picc_Close();
			}

			break;

		default : break;
		}
	}

	return 0;
}
#endif
#endif
