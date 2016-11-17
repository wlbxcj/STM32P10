/// @file   Taxbox.c
/// This is for module TaxBox and UART_Command
/// @author Travis Tu
/// @date   2006-Sep-1st


/* Includes ------------------------------------------------------------------*/
//platform depended type defination

#include "TaxBox.h"

//h file for lib
#include "KF701DH.h"
#include "Display.h"
#include "string.h"
#include "kb.h"
#include "misc.h"
#include "hw_config.h"
#include "tsc2046.h"
#include "lcd.h"
#include "dll.h"
//#include "usb_init.h"
//AT skx
#include "SysTimer.h"
AppFlags_t AppFlag;
#define USB_MOD

#define  s_RF_Init	PN512_s_RF_Init

extern USART_TypeDef * SC_UART;

extern void Protect_Switch_Init(void);
extern int InitNotUseGpio(void);
extern void SPI3_Init(void);
extern void USB_Init(void);
extern int s_KbInit(void);
extern void HID_FifoInt(void);

#define BKP_DR_NUMBER              42

/**
  * @brief  Writes data Backup DRx registers.
  * @param  FirstBackupData: data to be written to Backup data registers.
  * @retval None
  */
void WriteToBackupReg(u16 index ,u16 FirstBackupData)
{
  //u32 index = 0;
u16 BKPDataReg[BKP_DR_NUMBER] =
  {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
  };    
  //write index = 0
  
  //for (index = 0; index < BKP_DR_NUMBER; index++)
  {
    //BKP_WriteBackupRegister(BKPDataReg[index], FirstBackupData + (index * 0x5A));
    BKP_WriteBackupRegister(BKPDataReg[index], FirstBackupData );
  }  
}
void Tamper_Init()
{/*
u16 BKPDataReg[BKP_DR_NUMBER] =
  {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
  };*/    
  /* Disable Tamper pin */
  BKP_TamperPinCmd(DISABLE);
  
  /* Disable Tamper interrupt */
  BKP_ITConfig(DISABLE);

  /* Tamper pin active on low level */
  BKP_TamperPinLevelConfig(BKP_TamperPinLevel_Low);

  /* Clear Tamper pin Event(TE) pending flag */
  BKP_ClearFlag();

  /* Enable Tamper interrupt */
  BKP_ITConfig(ENABLE);

  /* Enable Tamper pin */
  BKP_TamperPinCmd(ENABLE);  
  
 /* Write data to Backup DRx registers */
 // WriteToBackupReg(0,0xA53C);  
  
}

/**
  * @brief  Checks if the Backup DRx registers values are correct or not.
  * @param  FirstBackupData: data to be compared with Backup data registers.
  * @retval 
  *         - 0: All Backup DRx registers values are correct
  *         - Value different from 0: Number of the first Backup register
  *           which value is not correct
  */
u32 CheckBackupReg(u16 index,u16 FirstBackupData)
{
  //u32 index = 0;
  //u16 nVal;
u16 BKPDataReg[BKP_DR_NUMBER] =
  {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
  };
  
  //for (index = 0; index < BKP_DR_NUMBER; index++)
//test

    //nVal = BKP_ReadBackupRegister(BKPDataReg[index]);
    BKP_ReadBackupRegister(BKPDataReg[index]);

  /*
  //test
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 1*8, 0x00, "BKP[%04x]",nVal);
        Lib_DelayMs(2000);
  */
  {
    //if (BKP_ReadBackupRegister(BKPDataReg[index]) != (FirstBackupData + (index * 0x5A)))
    if (BKP_ReadBackupRegister(BKPDataReg[index]) != (FirstBackupData ))
    {
      /*
      //test 13/04/20
	Lib_LcdCls();
	Lib_LcdPrintxy(0, 1*8, 0x00, "BKP[%04x]",nVal);
        Lib_DelayMs(2000);
      */
      return (index + 1);
    }
  }

  return 0;  
}

//static u8 updateFWACK = 0x88;
/***************************************
函数功能：上电后完成初始化系统,具体有如下操作
        1. initialize the system clock
        2. initialize the usb clock line
        3. wait for a while which can help to charge the cap
        4. init the i2c rtc
        5. init the bpsi flash communication channel
        6. init the UART communication challen
        7. init the USB slave
        8. init all the external interrupt
        9. setup the smart card device and prepare for the insertiong of any smart card
        10.init the IO for LED and power off it
        11.init the buz part
返回  0: 初始化成功  !=0 初始化失败
***************************************/
uchar initial_system(void)
{
    u8 DisplayType;
    extern u8 _glbPiccSetPara;
  
    //init the system clock
    SYSCLOCKInit();

    //init the interupters
    NVIC_Configuration();
    //init the SysTick for wait API
    SysTick_Config();
    /*wait for 0.1 seconds before capacity has been charged, promiss the capacity has been charged before the procedure of ram restore*/
    WaitNuS(100000);
    
    s_ComInit();
    /*Configure the UART 3 for incomming commands*/ 
    //AT skx
//#if 0    
    //PortOpen(0,115200,8,0,1);//应防止在启动时上位机连续发串口数据导致频繁中断
    Lib_ComOpen(0,"115200,8,N,1");
//#endif    


    /* init the USB */
//	#ifdef KF322_M
    //DisplayType = Display_Init();
    DisplayType = s_LcdInit();

    Lib_LcdSetBackLight(1);  
    
    
//	#endif
    /*Init Timer */
    //Tim2Init(); //for led display  
    
    //keyboard init
    s_KbInit();
    
    //AT skx
//#if 0    
    Tim3Init(); //for system timer
//#endif 
   
 
     
   
    /* Initialize the Calendar */
    //AT skx
   // #ifdef KF322_M   //12/10/25 enable
    Calendar_Init();
    //#endif
	
    
    LED_Init();
    //Buzzer_Init();//使用有源蜂鸣器  
      
    //FOR SPI PROTOCOL
    //SPI2_Init();
    extern int s_RF_Init(void);
    //13/06/27
#if 0
    (void)s_RF_Init();//初始化非接驱动
    //s_PiccInit(); //gplian
#endif    
    //SPI2_Init();
    //13/06/27
#if 0    
    s_VoiceInit(); //12/10/30 
#endif    
    //SC_Setup_SMALL();
    //Tamper 12/10/25
    Tamper_Init();
    //13/06/27
    InitNotUseGpio();

    //13/07/06
    _glbPiccSetPara= 0;
    
#ifdef AS3911_M
   // s_PiccInit(); //14/10/24
#else
    (void)s_RF_Init();//初始化非接驱动
#endif
    
    //TSC2046_init();   // 移到触屏OPEN函数里
    //13/11/27 for sm
    SPI3_Init();
    
        //13/07/08
    Lib_PiccGetPara(&_glbPiccSetPara);

    if (LCDValue >= 4000 || LCDValue <= 300)    // 带USB的设备
    {
        USB_Port_Set(0);    //USB先断开
        delay_ms(300);
        USB_Port_Set(1);    //USB再次连接
        /* USB 初始化 */
        USB_Config();
        HID_FifoInt();
        USB_Init();
    }
    return(DisplayType);

}
/**************************************************************************
函数功能：打开后备电源
 入口参数：无
 出口参数：无
**************************************************************************/
//extern u8 bDeviceState ;

void System_SoftReset(void)
{
   usb_power_off() ;
   NVIC_GenerateSystemReset();
}

/**************************************************************************
函数功能：打开后备电源
 入口参数：无
 出口参数：无
**************************************************************************/
void bat_on(void)
{
}

/**************************************************************************
函数功能：关闭后备电源
 入口参数：无
 出口参数：无
**************************************************************************/
void bat_off(void)
{
}

/*  ---------------------  sram  ------------------------*/
/***************************************************************************/
/* 函数功能：将SRAM中leng长度的数据读到缓存区中                            */
/* 入口参数：addr为读数据的起始地址,leng为数据长度，*buf为存放数据的指针   */
/* 出口参数：返回值0 正确                                                  */
/*                 1 出错                                                  */
/***************************************************************************/
uchar read_sramb(uint addr,uint leng,void *buf)
{
    return 0;
}
/***************************************************************************/
/* 函数功能：将leng长度的数据写入SRAM中                                    */
/* 入口参数：addr为写入数据的起始地址,leng为数据长度，*buf为存放数据的指针 */
/* 出口参数：返回值0 正确                                                  */
/*                 1 出错                                                  */
/***************************************************************************/
uchar write_sramb(uint addr,uint leng,void *buf)
{
    return 0;
}

/*  ---------------------  flash (S29GL064)------------------------*/
/***************************************
函数功能：分扇区擦除flash
函数入口：4M字节的flash共分64个扇区,每个扇区64K，sector为0～63
          8M字节的flash共分128个扇区,每个扇区64K，sector为0～127
          32M字节的flash共分512个扇区,每个扇区64K，sector为0～511
函数出口：0：正确      1：错误
***************************************/
uchar flash_sector_erase(uint sector)
{
    BSPIFlashSE(SPIFLASH_SECTOR_SIZE*sector);
    return 0;
}

/***************************************
函数功能：整片擦除flash
返回值 ： 0：擦除正确 1：擦除失败
***************************************/
uchar flash_chip_erase(void)
{
    BSPIFlashBE(SPIFLASH_1ST_BULK_ADDR);
    return 0;
}

/***************************************
函数功能：读wr_buf缓冲区中wr_length长度的数据写入flash指定单元
函数入口：wr_addr:  写操作起始地址(0---0X3FFFFF)  4M空间
                    写操作起始地址(0---0X7FFFFF)  8M空间
                    写操作起始地址(0---0X1FFFFFF)  32M空间
		 wr_length:写入字节数
		 wr_buf:   写操作数据缓冲区
返回值 ： 0：写入正确 1：写入失败
***************************************/
uchar write_flash(ulong wr_addr,uint wr_length,  void *wr_buf)
{
    if( wr_addr>=(SPIFLASH_2ND_BULK_ADDR+SPIFLASH_BULK_SIZE) )
        return 1;
    if( (wr_addr+wr_length)>(SPIFLASH_2ND_BULK_ADDR+SPIFLASH_BULK_SIZE) )
        return 1;
    if( (wr_addr<SPIFLASH_BULK_SIZE) && ((wr_addr+wr_length)>SPIFLASH_BULK_SIZE) )
    {
        ProgramSPIFlash((const u8*)wr_buf,
                        (const u8*)wr_addr,
                        (u16)(SPIFLASH_2ND_BULK_ADDR-wr_addr));
        ProgramSPIFlash((const u8*)((u8*)wr_buf+SPIFLASH_2ND_BULK_ADDR-wr_addr),
                        (const u8*)SPIFLASH_2ND_BULK_ADDR,
                        (u16)(wr_length-(SPIFLASH_2ND_BULK_ADDR-wr_addr)));
    }
    else
    {
        ProgramSPIFlash((const u8*)wr_buf, (const u8*) wr_addr, (u16)wr_length);
    }
    return 0;
}

/***************************************
函数功能：将FLASH中rd_length长度的数据读到rd_buf缓冲区中
函数入口：rd_addr:  读操作起始地址 (0---0X3FFFFF)  4M空间
                    读操作起始地址 (0---0X7FFFFF)  8M空间
                    读操作起始地址 (0---0X1FFFFFF)  32M空间
		 rd_length:读出字节数
		 rd_buf:   读操作数据缓冲区
返回值 ： 0：读操作正确 1：读操作失败,地址溢出
***************************************/
uchar read_flash(ulong rd_addr,uint rd_leng, void *rd_buf)
{
    if( rd_addr>=(SPIFLASH_2ND_BULK_ADDR+SPIFLASH_BULK_SIZE) )
        return 1;
    if( (rd_addr+rd_leng)>(SPIFLASH_2ND_BULK_ADDR+SPIFLASH_BULK_SIZE) )
        return 1;
    if( (rd_addr<SPIFLASH_BULK_SIZE) && ((rd_addr+rd_leng)>SPIFLASH_BULK_SIZE) )
    {
        ReadSPIFlash((const u8*)rd_addr,
                     (u8*) rd_buf,
                     (u16)(SPIFLASH_2ND_BULK_ADDR-rd_addr));
        ReadSPIFlash((const u8*) SPIFLASH_2ND_BULK_ADDR,
                     (u8*)((u8*)rd_buf+SPIFLASH_2ND_BULK_ADDR-rd_addr),
                     (u16)(rd_leng-(SPIFLASH_2ND_BULK_ADDR-rd_addr)));
    }
    else
        ReadSPIFlash((const u8*)rd_addr, (u8*) rd_buf, (u16)(rd_leng));
    return 0;
}

/*  ---------------------      eeprom      ------------------------*/
//1个24c64,共8k字节存储容量
/***************************************
函数功能：读eeprom
函数入口：rd_addr:  读操作起始地址(0--0x1FFF);
		 rd_length:读出字节数
		 rd_buf:   读操作数据缓冲区
返回值 ： 0：读操作正确 1：读操作失败
***************************************/
uchar read_eeprom(uint rd_addr,uint rd_length,void *rd_buf)
{
	  //USB_Interrupts_Disable();
	
    return I2CEEPROMGetbytes((u16)rd_addr,(u8*)rd_buf,rd_length);
	//USB_Interrupts_Config();
}

/***************************************
函数功能：向eeprom某地址开始写wr_length长的数据
函数入口：wr_addr:  写操作起始地址(0--0x1FFF);
		  wr_length:写入字节数
		  wr_buf:   写操作数据缓冲区
返回值 ： 0：写入正确   1：写入失败
***************************************/
uchar write_eeprom(uint wr_addr,uint wr_length,void *wr_buf)
{
    return I2CEEPROMUpdateBytes((u16)wr_addr,(u8*)wr_buf,wr_length);
}

/*  ---------------------     时钟函数     ------------------------*/
/***********************
函数功能：设置时钟
函数入口：rtc_buf:设置时钟数据缓冲区
			rtc_buf中的数据定义如下：

			rtc_buf[0]=年低位
			rtc_buf[1]=月
			rtc_buf[2]=日
			rtc_buf[3]=时
			rtc_buf[4]=分
			rtc_buf[5]=秒
			数据采用压缩BCD码格式
返回值：0:设置成功  1：设置失败,时钟数据格式错误
***********************/
uchar set_rtc(uchar *rtc_buf)
{
    RTCTimeType time;
    time.Century = 0x20;
    time.Year    = rtc_buf[0];
    time.Month   = rtc_buf[1];
    time.Day     = rtc_buf[2];
    time.Hour    = rtc_buf[3];
    time.Minute  = rtc_buf[4];
    time.Second  = rtc_buf[5];
    I2CRTCSetTime(&time);
    return 0;
}

/***********************
函数功能：读日期时间
函数入口：rtc_buf:读操作数据缓冲区
			rtc_buf中的数据存放如下：

			rtc_buf[0]=年低位
			rtc_buf[1]=月
			rtc_buf[2]=日
			rtc_buf[3]=时
			rtc_buf[4]=分
			rtc_buf[5]=秒
			数据采用压缩BCD码格式
***********************/
void read_rtc(uchar *rtc_buf)
{
    RTCTimeType time;
    I2CRTCGetTime(&time);
    rtc_buf[0]=time.Year;
    rtc_buf[1]=time.Month;
    rtc_buf[2]=time.Day;
    rtc_buf[3]=time.Hour;
    rtc_buf[4]=time.Minute;
    rtc_buf[5]=time.Second;
}

/********************************************
函数功能:验证rtc_buf所指向的日历/时钟数据的合法性
入口:   rtc_buf:存放日期时间的首地址
            rtc_buf中的数据定义如下：

        	rtc_buf[0]=年
        	rtc_buf[1]=月
        	rtc_buf[2]=日
        	rtc_buf[3]=时
        	rtc_buf[4]=分
        	rtc_buf[5]=秒

            数据采用压缩BCD码格式，
出口：无
返回值: 数据合法返回0，否则返回1
*********************************************/
//润年每月天数
uchar DaysTab1[]={0,31,29,31,30,31,30,31,31,30,31,30,31};
//非润年每月天数
uchar DaysTab2[]={0,31,28,31,30,31,30,31,31,30,31,30,31};

uchar check_rtc(uchar *rtc_buf)      //yyw 20090106
{
	uchar temp,year,month,day;

	//  Year
	temp=((rtc_buf[0]>>4)*10)+(rtc_buf[0]&0x0f);
	year=temp;
	if(temp>99) return(1);

	//  Month
	temp=((rtc_buf[1]>>4)*10)+(rtc_buf[1]&0x0f);
	month=temp;
	if((temp==0)||(temp>12)) return(1);

	//  Day
	temp=((rtc_buf[2]>>4)*10)+(rtc_buf[2]&0x0f);
	day=temp;
	if(temp==0) return(1);
	if((year%4)==0)
	{
		if(day>DaysTab1[month])	
			return(1);
	}
	else
	{
		if(day>DaysTab2[month])	
			return(1);
	}

	//  Hour
	temp=((rtc_buf[3]>>4)*10)+(rtc_buf[3]&0x0f);
    if(temp>23) return(1);

	//  Minute
	temp=((rtc_buf[4]>>4)*10)+(rtc_buf[4]&0x0f);
    if(temp>59) return(1);

	//  Second
	temp=((rtc_buf[5]>>4)*10)+(rtc_buf[5]&0x0f);
    if(temp>59) return(1);

	return(0);
}


/***************************
函数功能:以查询方式,接收一个字节(8位数据,1个起始一个停止,无校验)
入口:无
出口:buf 收到的数据首地址
返回值:
        == 0 正确 收到数据,
        != 0 超时(TIMEO)
****************************/		
uchar rece(uint length,uchar *buffer)
{
    //u16 len=length;
    //return Comm_RecvPacket(buffer, &len);
  return 0;
}

/********************************
函数功能:以查询方式,发送一个字节(8位数据,1个起始一个停止,无校验),
入口:    c 发送的字符
出口:
返回值: 无
*********************************/		
void send_str(uint length,uchar *buffer)
{
    //Comm_SendPacket(buffer, (u16)length);
}

/********************************
函数功能:清除串口缓冲区
*********************************/
void clear_rece_buffer(void)
{
   // UARTCOM_Reset();
}

/*------------------------ 卡操作函数 ---------------------------------*/
/***************************************
函数功能：检测是否有卡插入
返回值  ：0：有卡   1：无卡
***************************************/
uchar inserted_card(void)
{

    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7))
        return 1;//high stand for card not inserted
    else
        return 0;//low stand for card inserted
}

/***************************************
函数功能：小卡座上电
***************************************/
u8 sam_atr[34];
void sam_pwr_on(void)/////////yyw2008 08 12
{

    SC_Setup_SMALL();
    SC_UART=SC_SML_UART;
    SC_Powerconnect();
    SCART_TypeDef* atr = SC_Cold_Reset(SMALL_SC);
}

/***************************************
函数功能：大卡座上电
***************************************/
u8 id_atr[34];
void id_pwr_on(void)
{
    SC_Setup_BIG();
    SC_UART=SC_BIG_UART;
    SC_Powerconnect();
}

/***************************************
函数功能：大卡座下电
***************************************/
void id_pwr_off(void)
{
  SC_UART=SC_BIG_UART;
  SC_PowerCut();
  SC_Turnoff_BIG();
}

/***************************************
函数功能：小卡座下电
***************************************/
void sam_pwr_off(void)/////////yyw2008 08 12
{
  SC_UART=SC_SML_UART;
  SC_PowerCut();
  SC_Turnoff_SMALL();
}


/***************************************
函数功能：大卡座上的cpu卡复位应答(含有上电)
函数入口：无
函数出口：返回0：复位正常  否则：复位异常
		 buf：存放复位信息的首地址，数据格式为：ATR长度、ATR1信息、ATR2信息......		
***************************************/
uchar atr_id(uchar *buf)
{
    SCART_TypeDef* atr = SC_Cold_Reset(BIG_SC);
	
    if ((atr->buf[0] != 0x3B) && (atr->buf[0] != 0x3F))
    {
        id_atr[0] = 0;
    }
    else if (atr->bTCKNOShow)
    {
    	id_atr[0] = 1;
	}
    else if (atr->bTCKError)
    {
    	id_atr[0] = 2;
	}
    else
    {
        id_atr[0]=atr->length;
        memcpy(&id_atr[1], atr->buf, atr->length);
    }
	
    //If only the length is right
    if ((id_atr[0] <= 33) && (id_atr[0] >= 3) )
    {
        memcpy(buf, id_atr, id_atr[0] + 1);
        return 0;
    }
    else
    {
        id_pwr_off();
        return 1;
    }
}

/***************************************
功能描述：与大卡座上电CPU卡进行通讯
入口参数:
	ptr:输入数据缓冲区。格式为：60H,LEN,ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n
	60H --命令头
	LEN	--ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n的总字节数
	ISO_MODE--0DAH,ISO_IN操作，写卡/校密码
			--0DBH,ISO_OUT操作，读卡
			--其它内容为非法
	ISO_IN :60H,LEN,ISO_MODE,CLS,INS,P1,P2,Lc,data_1,...,data_n
	ISO_OUT:60H,LEN,ISO_MODE,CLS,INS,P1,P2,Le
	        le最大为240
出口参数:  返回0：操作成功  否则操作失败	
	ptr：输出数据缓冲区，其格式为：60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	LEN	--data_1,...,data_n的字节数
	ISO_IN:	60H,LEN,ISO_MODE,SW1,SW2
	ISO_OUT:60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	无论ISO_IN还是ISO_OUT,ptr缓冲区都会被改写,
***************************************/
uchar id_iso(uchar *ptr)
{
    delay_ms(50);
    return SC_SendAPDU_KaiFa(ptr,BIG_SC);
}

/***************************************
函数功能：SAM卡座上的CPU卡复位
函数入口：无
函数出口：返回0：复位正常   否则：复位异常
		 buf：存放复位信息的首地址，数据格式为：ATR长度、ATR1信息、ATR2信息......
***************************************/
uchar atr_sam(uchar *buf)
{
    SCART_TypeDef* atr = SC_Cold_Reset(SMALL_SC);
    if( (atr->buf[0]!=0x3B) && (atr->buf[0]!=0x3F) )
    {
        sam_atr[0]=0;
    }
    else if(atr->bTCKNOShow)
    {
    	sam_atr[0]=1;
	}
    else if(atr->bTCKError)
    {
    	sam_atr[0]=2;
	}
    else
    {
        sam_atr[0]=atr->length;
        memcpy(&sam_atr[1], atr->buf, atr->length);
    }
    //If only the length is right
   if( (sam_atr[0]<=33) && (sam_atr[0]>=3) )
    {
        memcpy(buf, sam_atr, sam_atr[0]+1);
        return 0;
    }
    else
    {
        sam_pwr_off();
        return 1;
    }
}

/***************************************
功能描述：与小卡座上电CPU卡进行通讯
入口参数:
	ptr:输入数据缓冲区。格式为：60H,LEN,ISO_MODE,CLS,INS,P1,P2,lc,data_1,...,data_n le
	60H --命令头
	LEN	--ISO_MODE,CLS,INS,P1,P2,P3,data_1,...,data_n的总字节数
	ISO_MODE--0DAH,ISO_IN操作，写卡/校密码
			--0DBH,ISO_OUT操作，读卡
			--其它内容为非法
	ISO_IN :60H,LEN,ISO_MODE,CLS,INS,P1,P2,Lc,data_1,...,data_n
	ISO_OUT:60H,LEN,ISO_MODE,CLS,INS,P1,P2,Le
	        le最大为240
出口参数:  返回0：操作成功  否则操作失败	
	ptr：输出数据缓冲区，其格式为：60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
	LEN	--data_1,...,data_n的字节数
	ISO_IN:	60H,LEN,ISO_MODE,SW1,SW2
	ISO_OUT:60H,LEN,ISO_MODE,data_1,...,data_n,SW1,SW2
***************************************/
uchar sam_iso(uchar *ptr)
{
    delay_ms(50);
    return SC_SendAPDU_KaiFa(ptr,SMALL_SC);
}

//   flash card
/**********************************************
函数功能:AT45D041 RESET
***********************************************/
void at45d041_reset(void)
{
   DF_Reset();
}

/*********************************
函数功能：写flash卡
入口参数：  ADDR:开始的地址 (0---0x83fff)
            LENGTH:长度     (1---264)
            BUF:数据的起始地址
返回值：  写入成功0，否则1: 入口参数错误(length>264或者length==0)
*********************************/
uchar f45d041_writecard(ulong addr,uint length,uchar *buf)
{
    return DF_MemProgram(addr, length, buf);
}

/************************************
函数功能:从卡中读length bytes到缓冲区
输入参数:addr(F45D041卡:0..0x82fff):卡地址;
		lenght(0..263):长度;
		buf:缓冲区;

适用机型：
修改时间:2002.4.12
****************************/
uchar f45d041_readcard(ulong addr,uint length,uchar *buf)
{
   return DF_MemRead(addr, length, buf);
}

/*-----------------------------------其它函数---------------------------------*/
/***************************************
函数功能：打开蜂鸣器
***************************************/
void buz_on(void)
{
    Buzzer_Control(TRUE);
}

/***************************************
函数功能：关闭蜂鸣器
***************************************/
void buz_off(void)
{
   Buzzer_Control(FALSE);
}

/***************************************
函数功能：绿指示灯亮
***************************************/
void ledgreen_on(void)
{
    LED_Show(BLUE_LED, 1);
}

/***************************************
函数功能：绿指示灯灭
***************************************/
void ledgreen_off(void)
{
    LED_Show(BLUE_LED, 0);
}


/***************************************
函数功能：黄指示灯亮
***************************************/
void ledorange_on(void)
{
    LED_Show(ORANGE_LED, 1);
}

/***************************************
函数功能：黄指示灯灭
***************************************/
void ledorange_off(void)
{
    LED_Show(ORANGE_LED, 0);
}

/***************************************
函数功能：红指示灯亮
***************************************/
void ledred_on(void)
{
    LED_Show(RED_LED, 1);
}

/***************************************
函数功能：红指示灯灭
***************************************/
void ledred_off(void)
{
    LED_Show(RED_LED, 0);
}


/***************************************
函数功能：绿指示灯亮
***************************************/
void ledbule_on(void)
{
    LED_Show(GREEN_LED, 1);
}

/***************************************
函数功能：绿指示灯灭
***************************************/
void ledbule_off(void)
{
    LED_Show(GREEN_LED, 0);
}

void ledall_on(void)
{
    LED_Show(BLUE_LED, 1);  
    LED_Show(ORANGE_LED, 1);    
    LED_Show(RED_LED, 1);  
    LED_Show(GREEN_LED, 1);
}

void ledall_off(void)
{
    LED_Show(BLUE_LED, 0);  
    LED_Show(ORANGE_LED, 0);    
    LED_Show(RED_LED, 0);  
    LED_Show(GREEN_LED, 0);
}




/*********************************
函数功能:延时时间 t ms
*********************************/
//AT skx
void delay_ms(ulong t)
{
  ulong i;
  for(i=0;i<t;i++)
    WaitNuS(1000);
  //WaitNuS(1000*t);
}
/*
void delay_ms(ulong t)
{
    t*=1000; // transform t from ms to us
    while(t)
    {
        if(t<200000)
        {
            WaitNuS(t);
            break;
        }
        else
        {
            WaitNuS(200000);
            t-=200000;
        }
    }
}
*/



//================================================
// 函 数 名: calc_next_day
// 功能描述: 判断输入日期的合法性并返回下一日的日期
// 全局变量: 无
// 输入参数: date 要计算的日期(CCYYMMDD)
// 输出参数: 无
// 返 回 值: 所指字符串的头4字节均为0xFF 输入日期错误;
//           ==其他，下一日的日期(CCYYMMDD)
// 作    者: 周剑冬
// 创建日期: 2003-06-XX
// 修 改 者: 周剑冬
// 修改日期: 2003-06-XX
//================================================

/**********************************************
功能:   指令测试程序
返回值：测试成功0，否则!0
*************************************************/
extern uchar test_ins(void);

extern void dtr_on(void);

extern void dtr_off(void);





/******************************************************************
crc参数表(8005,crc-16)
******************************************************************/
uint Table_CRC[256]=
{
0x0000,0x8005,0x800f,0x000a,0x801b,0x001e,0x0014,0x8011,
0x8033,0x0036,0x003c,0x8039,0x0028,0x802d,0x8027,0x0022,
0x8063,0x0066,0x006c,0x8069,0x0078,0x807d,0x8077,0x0072,
0x0050,0x8055,0x805f,0x005a,0x804b,0x004e,0x0044,0x8041,
0x80c3,0x00c6,0x00cc,0x80c9,0x00d8,0x80dd,0x80d7,0x00d2,
0x00f0,0x80f5,0x80ff,0x00fa,0x80eb,0x00ee,0x00e4,0x80e1,
0x00a0,0x80a5,0x80af,0x00aa,0x80bb,0x00be,0x00b4,0x80b1,
0x8093,0x0096,0x009c,0x8099,0x0088,0x808d,0x8087,0x0082,
0x8183,0x0186,0x018c,0x8189,0x0198,0x819d,0x8197,0x0192,
0x01b0,0x81b5,0x81bf,0x01ba,0x81ab,0x01ae,0x01a4,0x81a1,
0x01e0,0x81e5,0x81ef,0x01ea,0x81fb,0x01fe,0x01f4,0x81f1,
0x81d3,0x01d6,0x01dc,0x81d9,0x01c8,0x81cd,0x81c7,0x01c2,
0x0140,0x8145,0x814f,0x014a,0x815b,0x015e,0x0154,0x8151,
0x8173,0x0176,0x017c,0x8179,0x0168,0x816d,0x8167,0x0162,
0x8123,0x0126,0x012c,0x8129,0x0138,0x813d,0x8137,0x0132,
0x0110,0x8115,0x811f,0x011a,0x810b,0x010e,0x0104,0x8101,
0x8303,0x0306,0x030c,0x8309,0x0318,0x831d,0x8317,0x0312,
0x0330,0x8335,0x833f,0x033a,0x832b,0x032e,0x0324,0x8321,
0x0360,0x8365,0x836f,0x036a,0x837b,0x037e,0x0374,0x8371,
0x8353,0x0356,0x035c,0x8359,0x0348,0x834d,0x8347,0x0342,
0x03c0,0x83c5,0x83cf,0x03ca,0x83db,0x03de,0x03d4,0x83d1,
0x83f3,0x03f6,0x03fc,0x83f9,0x03e8,0x83ed,0x83e7,0x03e2,
0x83a3,0x03a6,0x03ac,0x83a9,0x03b8,0x83bd,0x83b7,0x03b2,
0x0390,0x8395,0x839f,0x039a,0x838b,0x038e,0x0384,0x8381,
0x0280,0x8285,0x828f,0x028a,0x829b,0x029e,0x0294,0x8291,
0x82b3,0x02b6,0x02bc,0x82b9,0x02a8,0x82ad,0x82a7,0x02a2,
0x82e3,0x02e6,0x02ec,0x82e9,0x02f8,0x82fd,0x82f7,0x02f2,
0x02d0,0x82d5,0x82df,0x02da,0x82cb,0x02ce,0x02c4,0x82c1,
0x8243,0x0246,0x024c,0x8249,0x0258,0x825d,0x8257,0x0252,
0x0270,0x8275,0x827f,0x027a,0x826b,0x026e,0x0264,0x8261,
0x0220,0x8225,0x822f,0x022a,0x823b,0x023e,0x0234,0x8231,
0x8213,0x0216,0x021c,0x8219,0x0208,0x820d,0x8207,0x0202
	};

//crc中间暂存值
uint crc_val=0;

/*********************************************
函数功能： CRC校验算法,(8005,crc-16)
入口参数： length 参加运算数据长度
   clear==1 CRC初值清零开始运算,仅仅计算buffer内数据的crc校验值
   clear==0 在保留CRC初值基础上计算buffer内数据的crc校验值
   buffer 数据地址指针			
返回值： CRC校验码
********************************************/
uint crc_16(uint length,uchar clear,void *buffer)
{
        uint i;
	uchar *p;

	p=(uchar *)buffer;
	if(clear) crc_val=0;
	
    for ( i = 0; i < length; i++ )
        crc_val = ( crc_val << 8 ) ^ (uint)Table_CRC[( crc_val >> 8 ) ^ *p++];
    return crc_val;
}

/*-------------------------------------------------------------
	计算一个字节的CRC
	人口参数:
		onebyte == 计算字节
		crc_reg == 计算出的CRC值
-------------------------------------------------------------*/
void crc_byte(uint onebyte,uint *crc_reg)
{
    int i;
 	
    onebyte <<= 1;
    for(i=8;i>0;i--)
    {	
      	onebyte >>= 1;
       	if((onebyte^*crc_reg) & 0x0001 )
        	*crc_reg = (*crc_reg>>1)^0xa001;
		else
          	*crc_reg >>= 1;
  	}
}



uint crcxdata_old(uint length,uchar clear,void *buffer)
{
    uint i;
    static uint crc;
    uchar *p;

	p=(uchar *)buffer;
    if(clear)
       crc = 0;
    for(i=0;i<length;i++)
    {	
     	crc_byte(*p,&crc);
     	p++;
    }
    return crc;
}


/*-------------------------------------------------------------------------------
 Funtion: 将压缩Bcd转换为Bin
 Input  : TUlBinValue:BIN码值
	  TUcCompBcdLen:压缩bcd码字节数
	  TUcCompBcdBuf: 压缩的bcd码
 excemple:十进制数：12h,34h,56h 转换为0x1e240h
 Editor : NingNeng
 Date   : 2002-11-19
----------------------------------------------------------------------------*/
void bcd_to_bin(ulong *TUlBinValue,uchar TUcCompBcdLen,uchar *TUcCompBcdBuf)
{

  	#define MAX_COMP_BCD_LEN 10
  	uchar LUcBcdBuf[MAX_COMP_BCD_LEN*2];
  	uchar i;

  	*TUlBinValue=0;
  	for(i=0;i<TUcCompBcdLen;i++)
  	{
     	LUcBcdBuf[2*i]=TUcCompBcdBuf[i]>>4;
     	LUcBcdBuf[2*i+1]=TUcCompBcdBuf[i]&0xf;
     	*TUlBinValue=LUcBcdBuf[2*i]*10+LUcBcdBuf[2*i+1]+(*TUlBinValue)*100;
  	}
}



/*-------------------------------------------------------------------------------
 Funtion: 将BIN转换为压缩BCD
 Input  : TUlBinValue:BIN码值
	  TUcCompBcdLen:压缩后的bcd码字节数,
	  TUcCompBcdBuf: 压缩后的bcd码
 excemple:十进制数：123456 转换为12H，34H，56H的序列
----------------------------------------------------------------------------*/
void bin_to_bcd(ulong TUlBinValue,uchar TUcCompBcdLen,uchar *TUcCompBcdBuf)
{
	#define MAX_COMP_BCD_NUM 15
 	uchar LUcBcdBuf[MAX_COMP_BCD_NUM*2];
 	uchar i;

 	i=TUcCompBcdLen*2-1;
 	do	//由个位开始从高字节向低字节存放bcd码
   	{
    	LUcBcdBuf[i]=TUlBinValue%10;
		TUlBinValue /=10;
   	}while(i--);//}while(--i); Debug by HKD

 	i=0;
  	do	//将bcd码压缩到指针变量中
    {
     	*TUcCompBcdBuf=(LUcBcdBuf[i]<<4)|(LUcBcdBuf[i+1]&0x0f);
     	TUcCompBcdBuf++;
     	i +=2;
    }while(i<TUcCompBcdLen*2);
}


/***************************************
函数功能：开电源保护
***************************************/
void power_on(void)
{
    Protect_Switch(1);
}

/***************************************
函数功能：关电源保护
***************************************/
void power_off(void)
{
    Protect_Switch(0);
}

