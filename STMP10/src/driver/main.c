
#include "stm32f10x_lib.h"

#include "usb_lib.h"
#include "hw_config.h"
#include "usb_core.h"

//#define SKX_FOR_LIB//生成库文件

#ifndef SKX_FOR_LIB

#include "KF701DH.h"
#include "KF701DV.h"
#include "Toolkit.h"

#include "var.h"
#include "kb.h"

#include "ASCII8X8.h"

#define VOICE_PLS_INPUT_PWD					1		//请输入密码


//#define FLASH_4M
//#define FLASH_8M
#define FLASH_16M
//#define FLASH_32M

#ifdef FLASH_4M
  #define SECTOR 63
  #define RANGE  0X400000
#endif

#ifdef FLASH_8M
  #define SECTOR 127
  #define RANGE  0X800000
#endif

#ifdef FLASH_16M
  #define SECTOR 64
  #define RANGE  0X1000000
#endif

#ifdef FLASH_32M
  #define SECTOR 511
  #define RANGE  0X2000000
#endif

#if 0
unsigned char g_byCurAppNum = 0xff;
#endif

extern void flsh_wr_operate(void);
extern unsigned char DisplayType;
extern unsigned char LedSegCode[];
extern void LedSegTest(void);
extern void test_all(char cmd);
extern void RF_TypeAB_test(unsigned char which_api);

void FileTest();

extern int  g_iVosFirstRunFlag;  // 1:first run, 0:not first run

//#include "rc531/Mifre_TmDef.h"
#include "pn512/Mifre_TmDef.h"

//#if 0
/**
  * @brief Sets System clock frequency to 72MHz and configure HCLK, PCLK2 
  *        and PCLK1 prescalers. 
  * @param None.
  * @arg None.
  * @note : This function should be used only after reset.
  * @retval value: None.
  */
static void SetSysClockTo72(void)
{
#define  uint8_t unsigned          char
#define  uint16_t unsigned          short
#define  uint32_t unsigned int
  
#define  RCC_CR_HSEON                        ((unsigned int)0x00010000)        /*!< External High Speed clock enable */
#define  RCC_CR_HSERDY                       ((unsigned int)0x00020000)        /*!< External High Speed clock ready flag */
#define HSEStartUp_TimeOut   ((uint16_t)0x0500) /*!< Time out for HSE start up */
#define  FLASH_ACR_PRFTBE                    ((uint8_t)0x10)               /*!<Prefetch Buffer Enable */
#define  FLASH_ACR_LATENCY                   ((uint8_t)0x03)               /*!<LATENCY[2:0] bits (Latency) */
#define  FLASH_ACR_LATENCY_0                 ((uint8_t)0x00)               /*!<Bit 0 */
#define  FLASH_ACR_LATENCY_1                 ((uint8_t)0x01)               /*!<Bit 0 */
#define  FLASH_ACR_LATENCY_2                 ((uint8_t)0x02)               /*!<Bit 1 */
#define  RCC_CFGR_HPRE_DIV1                  ((unsigned int)0x00000000)        /*!< SYSCLK not divided */
#define  RCC_CFGR_PPRE2_DIV1                 ((unsigned int)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE1_DIV2                 ((unsigned int)0x00000400)        /*!< HCLK divided by 2 */
#define  RCC_CFGR_PLLSRC                     ((unsigned int)0x00010000)        /*!< PLL entry clock source */
#define  RCC_CFGR_PLLXTPRE                   ((unsigned int)0x00020000)        /*!< HSE divider for PLL entry */
#define  RCC_CFGR_PLLMULL                    ((uint32_t)0x003C0000)        /*!< PLLMUL[3:0] bits (PLL multiplication factor) */
#define  RCC_CFGR_PLLMULL9                   ((uint32_t)0x001C0000)        /*!< PLL input clock*9 */
#define  RCC_CR_PLLON                        ((uint32_t)0x01000000)        /*!< PLL enable */
#define  RCC_CR_PLLRDY                       ((uint32_t)0x02000000)        /*!< PLL clock ready flag */
#define  RCC_CFGR_SW                         ((uint32_t)0x00000003)        /*!< SW[1:0] bits (System clock Switch) */
#define  RCC_CFGR_SW_PLL                     ((uint32_t)0x00000002)        /*!< PLL selected as system clock */

#define  RCC_CFGR_SWS                        ((uint32_t)0x0000000C)        /*!< SWS[1:0] bits (System Clock Switch Status) */
  
  volatile unsigned int  StartUpCounter = 0, HSEStatus = 0;
  
  /*!< SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/    
  /*!< Enable HSE */    
  RCC->CR |= ((unsigned int)RCC_CR_HSEON);
 
  /*!< Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSEStartUp_TimeOut));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (unsigned int)0x01;
  }
  else
  {
    HSEStatus = (unsigned int)0x00;
  }  

  if (HSEStatus == (unsigned int)0x01)
  {
    /*!< Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /*!< Flash 2 wait state */
    FLASH->ACR &= (unsigned int)((unsigned int)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (unsigned int)FLASH_ACR_LATENCY_2;    
 
    /*!< HCLK = SYSCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_HPRE_DIV1;
      
    /*!< PCLK2 = HCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE2_DIV1;
    
    /*!< PCLK1 = HCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE1_DIV2;
    
    /*!< PLLCLK = 8MHz * 9 = 72 MHz */
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (unsigned int)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

    /*!< Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /*!< Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    /*!< Select PLL as system clock source */
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_SW));
    RCC->CFGR |= (unsigned int)RCC_CFGR_SW_PLL;    

    /*!< Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (unsigned int)RCC_CFGR_SWS) != (unsigned int)0x08)
    {
    }
  }
  else
  { /*!< If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */    

    /*!< Go to infinite loop */
    while (1)
    {
    }
  }
}

void SystemInit (void)
{
  /*!< RCC system reset(for debug purpose) */
  /*!< Set HSION bit */
  RCC->CR |= (unsigned int)0x00000001;
  /*!< Reset SW[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], ADCPRE[1:0] and MCO[2:0] bits */
  RCC->CFGR &= (unsigned int)0xF8FF0000;  
  /*!< Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (unsigned int)0xFEF6FFFF;
  /*!< Reset HSEBYP bit */
  RCC->CR &= (unsigned int)0xFFFBFFFF;
  /*!< Reset PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE bits */
  RCC->CFGR &= (unsigned int)0xFF80FFFF;
  /*!< Disable all interrupts */
  RCC->CIR = 0x00000000;
    
  /*!< Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
  /*!< Configure the Flash Latency cycles and enable prefetch buffer */
  SetSysClockTo72();

}
//#endif

#if 0
/************************************************************
 针对第一次运行VOS时，须初始化
 ************************************************************/
int s_InitVos(void)
{
    int iRet;
	unsigned char buf[8];
	unsigned long temp[2];
	unsigned long dwCount;

	memset(buf,0xff,sizeof(buf));
	ByteToDword(&buf[0],4,&temp[0]);
	ByteToDword(&buf[4],4,&temp[1]);
     //now set run first  12/10/23 
    //g_iVosFirstRunFlag = CheckVosIsFirstRun();
    g_iVosFirstRunFlag = 1;
    if (0 != g_iVosFirstRunFlag)
    {//first time run
		s_PciWritePinCount(0,temp[0]);
		s_PciWritePinCount(1,temp[1]);
        //(void)s_RecreateSysKeyFile();
		s_DelAllSecrecyData();
        //tmp set      12/10/23   
        //AM_InitAll();
    }

    iRet = s_PciMMKCheck();
    if (0!=iRet)
    {
        //(void)SectorErase(0);
        //(void)s_RecreateSysKeyFile();
		s_DelAllSecrecyData();
                
        //AM_InitAll();  //12/10/23 disable
                
        s_PciInitMMK();
        g_iVosFirstRunFlag = 1;
    }
    return 0;
}
#endif

unsigned char CheckFirst()
{
    unsigned char sBuf[2];
    sys_flash_read_operate(0, sBuf,2);
    if(memcmp(sBuf,"\xa5\xa5",2)==0)
      return 0;
    return 1;
  
}

void testreset()
{
     int iRet,i,ret;
     uchar kb;
     int ExitPinPedProc =1;
     uchar sBuf[100];
	APDU_SEND ApduSend;
	APDU_RESP ApduResp;
#if 0
     Lib_IccClose(1);
     iRet = Lib_IccOpen(1,1,sBuf);
	  trace_debug_printf("\nLib_IccOpen[%d]",iRet);
	  if(iRet==0)
	  {
                for(i=0;i<sBuf[0];i++)
                  trace_debug_printf("%02x ",sBuf[i+1]);
	  }
#endif
		memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4);
		memcpy(ApduSend.DataIn, "1PAY.SYS.DDF01", 14);
		ApduSend.Le=256;
		ApduSend.Lc=14;

		iRet = Lib_IccCommand(1, &ApduSend, &ApduResp);
		if (iRet)
		{
			Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:CMD ERROR", i+1);
			Lib_Beef(3, 500);
			
		}
		else
		{
		    trace_debug_printf("\ncommand:");
		    for(i=0;i<ApduResp.LenOut;i++)
			  trace_debug_printf("%02x ",ApduResp.DataOut[i]);
		}
#if 0
     Lib_IccClose(2);
		         iRet = Lib_IccOpen(2,1,sBuf);
	  trace_debug_printf("\nLib_IccOpen[%d]",iRet);
	  if(iRet==0)
	  {
                for(i=0;i<sBuf[0];i++)
                  trace_debug_printf("%02x ",sBuf[i+1]);
	  }
#endif          
		iRet = Lib_IccCommand(2, &ApduSend, &ApduResp);
		if (iRet)
		{
			Lib_LcdPrintxy(0, (i+1)*8, 0x00, "SLOT%d:CMD ERROR", i+1);
			Lib_Beef(3, 500);
			
		}
		else
		{
		    trace_debug_printf("\ncommand:");
		    for(i=0;i<ApduResp.LenOut;i++)
			  trace_debug_printf("%02x ",ApduResp.DataOut[i]);
		}
                Lib_KbGetCh();
}

__IO uint8_t PrevXferComplete = 1;


void Usb_SendTest(unsigned char *pucData, unsigned char ucSendLen)
{
    unsigned char Send_Buffer[100] = {0, 0};

    //if (PrevXferComplete)
    {
      Send_Buffer[0] = 'A';
      
      //if (STM_EVAL_PBGetState(Button_TAMPER) == Bit_RESET)
      {
        memcpy(&Send_Buffer[1], pucData, ucSendLen);
        //Send_Buffer[1] = ucData;
      }

      
      /* Write the descriptor through the endpoint */    
      USB_SIL_Write(EP1_IN, (uint8_t*) Send_Buffer, ucSendLen + 1);  
      
      SetEPTxValid(ENDP1);
      
      //PrevXferComplete = 0;
    }
}

void main()
{
    // uchar CmdBuf[256];
    uchar RF1356Infor[4][16];
    uchar CmdDataOldType = 0xFF;  
    uchar CmdDataType = 0xFF;
    uchar KeepTime = 0xFF;
    uint  BuzzerFre;
    uint  BuzzerTime;
    uchar LineX,ByteY,AsciiLength,ByteLength;
    uchar i,j,Status;
    uchar buzzerTab[5]={3200,5200,7200,9200,11200};

    int nRet,nLen;
    uchar sBuf[100];
    //#define SysStartAddr  ((u32)0X08033800)//

    //lian
    extern int ContactlessCardProc(uchar *psAmountBcd,uchar *psRet);
    struct tm tTime;

    //DisplayType = initial_system();//115200
    initial_system();//115200

    trace_debug_printf("\r\nready to int usb\r\n");
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
    trace_debug_printf("usb int over\r\n");
    j = 0;
    while (1)
    {
        if (0 == Lib_KbCheck())
        {
            for (i = 0; i < 64; i++)
                sBuf[i] = i + j;
            CmdDataType = Lib_KbGetCh();
            Usb_SendTest(sBuf, 63);
            trace_debug_printf("KB = 0x%02X\r\n", nRet);
            j++;
        }
    }
   //Lib_AppInit();
#if 0   
 //触发
   if(CheckFirst())
   {
      sys_flash_erase_page(0);
      memcpy(sBuf,"\xa5\xa5",2);
      flash_write_operate(0+SysStartAddr,(unsigned short*)sBuf,2);
      WriteToBackupReg(5,0xA5A5);
      
   }
   else
   {  
      if(CheckBackupReg(5,0xA5A5)) //触发
      {
	//Lib_LcdCls();
	Lib_LcdPrintxy(0, 2*8, 0x00, "TAMPER!!!");
        sys_flash_erase_page(0);//下次为第一次
        
        RM_flash_erase_page(0);
        RM_flash_erase_page(1);
        RM_flash_erase_page(2);
        RM_flash_erase_page(3);
        RM_flash_erase_page(4);
        RM_flash_erase_page(5);
        RM_flash_erase_page(6);
        memset(sBuf,0,sizeof(sBuf));
        flash_write_operate(0,(unsigned short*)sBuf,2048);
    
	//Lib_LcdCls();
	//Lib_LcdPrintxy(0, 2*8, 0x00, "TAMPER!!!");
	Lib_KbGetCh();
        for(;;);
      }
   }
#endif

   //RF_TypeAB_test(0);
   //smartcardtest();
   //testreset();
   //Test_PsamFunc(0);
   //for(;;);
#if 0  
   //手写板通讯
   nRet = Lib_ComOpen(1,"57600,8,N,1");
   nRet = Sign_panel_handshake(NULL);
   i = 0;
   for(;;)
   {

     nRet = Lib_ComRecvByte(1,sBuf,5);
     //nRet = Lib_ComRecv(1,sBuf,50,&nLen,200);
     if(nRet==0)
       trace_debug_printf("rece[%02x]",sBuf[0]);
     
   }
#endif
   
   //flash_op_test();
   //s_PciInit();
   //s_InitVos();
//#if 0   
 //Buzzer_Ring(3200);  
  //delay_ms(500); 
  //Buzzer_Off();
//#endif  
  //buz_on();
  //for(;;);
  
   //TestLcd();
  //Test_ClkFunc();
   //Test_BeepLoop();
   //pcimain();
   //for(;;);
   
   maintest();
   //TestKbGetch();
   s_KbInit();
   for(;;)
   {
#if 0     
     if(Keyboard_Change_Flag==1)
     {
        trace_debug_printf("[%d]",Keyboard_Val);
        Keyboard_Change_Flag = 0 ;   //清除键盘改变标志位

     }
#endif
     
   }
#if 0
   for(;;)
   {
     
     i = Get_KeyValue();
     if(i!=0)
     {
       
     trace_debug_printf("[%c]",i);
     }
   }
#endif   
  //test
  gettime(&tTime);
  tTime.tm_sec = 50;
  settime(&tTime);
  gettime(&tTime);

  
  ledall_on();
 
  
  Buzzer_Ring(3200);  
  delay_ms(50); 
  Buzzer_Off();     
  ledall_off(); 
  
  //NVIC_GenerateCoreReset();//software reset core
  //Show  Test
#if 0 
  if(DisplayType)  
    LedSegTest();            //LED
  else      
#endif
    
    LcdDotTest();            //LCD
  

    
 // Entry_CoreInit();  //gplian
 // QPboc_CoreInit();  //gplian

  //LcdDotTest();
  
  //led test
  /*
  ledall_off();
  while(1)
  {
    ledgreen_on();
    delay_ms(50);
    ledgreen_off();
    delay_ms(100);
    ledred_on();
    delay_ms(50);
    ledred_off();
    delay_ms(100);
    ledorange_on();
    delay_ms(50);
    ledorange_off();
    delay_ms(100); 
  }
  */
  //uart test
  //uart_test(i);
  
  //uart driver test
  //uart_driver_test(i);
  
  //delay test
  
  /*
  while(1)
  {
    
    WaitNuS(50); 
    //GPIOC, GPIO_Pin_13
    GPIOC->BSRR = GPIO_Pin_13;
    WaitNuS(50); 
    GPIOC->BRR = GPIO_Pin_13;
  }
  */
  
  //sys timer test
  //systimer_test(1);

  //test protocol
  //Uart_Protocol();//协议处理
  
  //test flash
  //flash_op_test();
  
  //Rf Test
  
  extern void RfTestAll(void);

  //s_VoiceInit();
 
  
  //for(;;)
  {
  /*  
  SetVoice(VOICE_PLS_INPUT_PWD);
  delay_ms(100);
  */  
  
  SetVoice(2);
  delay_ms(100);
 /* 
  SetVoice(3);
  delay_ms(100);
  SetVoice(4);
  delay_ms(100);
 */ 



  
  }
  
  RF_TypeAB_test(0);
  //MifreQpbocTest();
  //lian disable
  //RfTestAll();

//test
 // FileTest();
 
  //for factory test
  //test_all(i);
  
  //lian
  //debug_printf(0,0,0,"%i\n",-6);
  
  //ContactlessCardProc("\x00\x00\0x10\x00\x00",value);

//  Uart_Protocol();//协议处理
  
  delay_ms(5000);   
  Lcdarray_clear(0x0F);
  LcdBL_Control(0);
  
  while(1);
    	
}



void FileTest()
{
#pragma pack(1)

typedef struct 
{
	uchar bAidLen;
	uchar sAid[16];
	uchar bSelectIndicator; //选择标志(0xff:partial_MATCH 部分匹配,full_Match)
	uchar bKernType;      //应用内核类型
						  //	KERNTYPE_DEF：根据RID，内核进行匹配确认;
	  						  //KERNTYPE_JCB：JCB
	 						  //KERNTYPE_MC:MASTERCARD
	 						  //KERNTYPE_VIS：VISA
	 						  //KERNTYPE_PBOC：PBOC
	 						  //其它：返回参数错误

	
	uchar sRFU[1];        //凑为20byte
}T_ENTRY_APP_LISTT; //guohonglv

typedef struct 
{
	unsigned long ulTermFLmt;     //Terminal Floor Limit (Tag ‘9F1B
	unsigned long ulRdClssTxnLmt; //Terminal Contactless Transaction Limit
	unsigned long ulRdCVMLmt;     //Terminal CVM Required Limit
	unsigned long ulRdClssFLmt;   //Terminal Contactless Floor Limit

	unsigned char aucAID[16];       
	unsigned char ucAidLen; 
	
	unsigned char ucKernType; // 定义后续该AID 所遵循的内核类型

	// payWave
	unsigned char  ucCrypto17Flg;
	unsigned char   ucZeroAmtNoAllowed;       // 0-若授权金额=0,则TTQ要求联机密文, 1-若授权金额=0, 则内部qVSDC不支持标志置位
	unsigned char   ucStatusCheckFlg;    // 读卡器是否支持状态检查
    unsigned char   aucReaderTermTransQualifiers[4];      // 终端交易性能，用于VISA/PBOC中，tag =9F66
	
	// common
	unsigned char ucTermFLmtFlg; 
	unsigned char ucRdClssTxnLmtFlg; 
	unsigned char ucRdCVMLmtFlg;   
	unsigned char ucRdClssFLmtFlg; 	 
	
	unsigned char aucRFU[3];
}Clss_PreProcInfoo; //guohonglv
#pragma pack()

	int fid,i;
	unsigned char sBuf[300],sPageBuf[2048];
	TERMAPPLIST tTermAppList;
	T_ENTRY_APP_LISTT tEntryAppList;
	Clss_PreProcInfoo tPreProcInfo;
/*	
	fid=open(FILE_TERMAPP_M, O_RDWR | O_CREAT | O_SYNC);
	for(i=0;i<50;i++)
	{
		memset(&tTermAppList,i,sizeof(TERMAPPLIST));
		write(fid,&tTermAppList,sizeof(TERMAPPLIST));
	}
	lseek(fid,0,SEEK_SET);
	for(i=0;i<50;i++)
	{
		read(fid,&tTermAppList,sizeof(TERMAPPLIST));
		memset(sBuf,i,sizeof(TERMAPPLIST));
		if(memcmp(sBuf,&tTermAppList,sizeof(TERMAPPLIST)))
			break;
	}
	//flash_read_operate(0x0807D800,sPageBuf,2048);
*/
	
	flash_read_operate(0x0807D800,sBuf,300);
        return ;
	flash_read_operate(0x0807D800+2048,sBuf,300);

	flash_read_operate(0x0807D800+2048+0x25c,sBuf,300);
        
        fid=open(".\\data\\emv\\entryapplist", O_RDWR | O_CREAT | O_SYNC);
	for(i=0;i<30;i++)
	{
		memset(&tEntryAppList,i,sizeof(T_ENTRY_APP_LISTT));
		write(fid,&tEntryAppList,sizeof(T_ENTRY_APP_LISTT));
        	flash_read_operate(0x0807D800+2048+i*20,sBuf,24);
                
	}
	lseek(fid,0,SEEK_SET);
	for(i=0;i<30;i++)
	{
		read(fid,&tEntryAppList,sizeof(T_ENTRY_APP_LISTT));
		memset(sBuf,i,sizeof(T_ENTRY_APP_LISTT));
		if(memcmp(sBuf,&tEntryAppList,sizeof(T_ENTRY_APP_LISTT)))
			break;
	}
	
	fid=open(".\\data\\emv\\preprocinfo", O_RDWR | O_CREAT | O_SYNC);
	for(i=0;i<30;i++)
	{
		memset(&tPreProcInfo,i,sizeof(Clss_PreProcInfoo));
		write(fid,&tPreProcInfo,sizeof(Clss_PreProcInfoo));
	}
	lseek(fid,0,SEEK_SET);
	for(i=0;i<30;i++)
	{
		read(fid,&tPreProcInfo,sizeof(Clss_PreProcInfoo));
		memset(sBuf,i,sizeof(Clss_PreProcInfoo));
		if(memcmp(sBuf,&tPreProcInfo,sizeof(Clss_PreProcInfoo)))
			break;
	}

	return;
	
}

void LedSegTest(void)
{
  uchar i,j;
  uchar LedSegVal[8];
  uchar LedSegNum[8] = {0x01,	0x2,	0x3,	0x4,	0x5,	0x6,	0x7,	0x8};

  for(i=0;i<8;i++) LedSegVal[i] = LedSegCode[LedSegNum[i]];
  Ledarray_On(LedSegVal);
  //while(1);
  delay_ms(100);    
  for(j=1;j<8;j++)  
  {
    LedSegNum[j-1] = 0x12;
    LedSegNum[j] = j+1;
    LedSegVal[j-1] = LedSegCode[LedSegNum[j-1]];
    LedSegVal[j] = LedSegCode[LedSegNum[j]];
    Ledarray_On(LedSegVal);    
    delay_ms(100);    
  }    
   
  LedSegVal[7] = LedSegCode[0x12];
  Ledarray_Off();
}


void Display16Regs(uchar PageX,uchar *pRegValue)
{
  uchar i,Reg4Bit;
  uchar LcdLine[16];
  
  LcdLine[0] ='P';
  LcdLine[1] ='g';  
  LcdLine[2] = PageX + '0';
  LcdLine[3] = ':';
  LcdLine[4] = 0x00; 
  Lcdstring_On(0,0,LcdLine);  
  //Line0
  for(i=0;i<4;i++)
  {
    Reg4Bit = *(pRegValue+i) >> 4;
    if(Reg4Bit < 0x0A)  LcdLine[i*3] = Reg4Bit + '0';
    else                LcdLine[i*3] = Reg4Bit + '7';
    Reg4Bit = *(pRegValue+i) & 0x0F;
    if(Reg4Bit < 0x0A)  LcdLine[i*3+1] = Reg4Bit + '0';    
    else                LcdLine[i*3+1] = Reg4Bit + '7';  
    LcdLine[i*3+2] = ' ';
  }
  LcdLine[12] = 0x00; 
  Lcdstring_On(0,32,LcdLine);   
  
  //Line1
  pRegValue += 4;
  for(i=0;i<4;i++)
  {
    Reg4Bit = *(pRegValue+i) >> 4;
    if(Reg4Bit < 0x0A)  LcdLine[i*3] = Reg4Bit + '0';
    else                LcdLine[i*3] = Reg4Bit + '7';
    Reg4Bit = *(pRegValue+i) & 0x0F;
    if(Reg4Bit < 0x0A)  LcdLine[i*3+1] = Reg4Bit + '0';    
    else                LcdLine[i*3+1] = Reg4Bit + '7';  
    LcdLine[i*3+2] = ' ';
  }
  LcdLine[12] = 0x00; 
  Lcdstring_On(1,32,LcdLine);     
  
  //Line2
  pRegValue += 4;
  for(i=0;i<4;i++)
  {
    Reg4Bit = *(pRegValue+i) >> 4;
    if(Reg4Bit < 0x0A)  LcdLine[i*3] = Reg4Bit + '0';
    else                LcdLine[i*3] = Reg4Bit + '7';
    Reg4Bit = *(pRegValue+i) & 0x0F;
    if(Reg4Bit < 0x0A)  LcdLine[i*3+1] = Reg4Bit + '0';    
    else                LcdLine[i*3+1] = Reg4Bit + '7';  
    LcdLine[i*3+2] = ' ';
  }
  LcdLine[12] = 0x00; 
  Lcdstring_On(2,32,LcdLine);   

  //Line3
  pRegValue += 4;
  for(i=0;i<4;i++)
  {
    Reg4Bit = *(pRegValue+i) >> 4;
    if(Reg4Bit < 0x0A)  LcdLine[i*3] = Reg4Bit + '0';
    else                LcdLine[i*3] = Reg4Bit + '7';
    Reg4Bit = *(pRegValue+i) & 0x0F;
    if(Reg4Bit < 0x0A)  LcdLine[i*3+1] = Reg4Bit + '0';    
    else                LcdLine[i*3+1] = Reg4Bit + '7';  
    LcdLine[i*3+2] = ' ';
  }
  LcdLine[12] = 0x00; 
  Lcdstring_On(3,32,LcdLine);   
}


//add aging test pro

void test_all(char cmd)
{
  int i,j,k;
  while(1)
  {
    //if(DisplayType)  LedSegTest();            //LED
    //else             
      LcdDotTest();            //LCD
  
    RF_TypeAB_test(i);
  }
  
   
}



int ContactlessCardProc(uchar *psAmountBcd,uchar *psRet)   

//uchar ContactlessCardProc()   

{

	uchar ret,buf[100],mode=0,tmp[100],swa[10],swb[10],time1[10],time2[10],Aid[32],key;
	int result,time,len,i,k,Aidlen;
	unsigned short tag;
	uchar amt[13];
        POS_COM gtPosCom; //guohonglv
        //struct ClssProcInter ClssProcInterInfo;//guohonglv
        //struct ClssEc ClssEcParam; //guohonglv
        //struct gtPosParamet gtPosParameter; //guohonglv
        
        T_Clss_TransParam tClssEcParam;
        T_Clss_PreProcInterInfo tClssProcInterInfo;
        PICC_PARA PiccPara;
        
	uchar inActype[256],outActype[256];
	uchar keyId;
	struct tm sTime;

	*psRet = 0;
	if( (ret=ClssTransInit()) != EMV_OK) 
	{
		TraceDisp("ClssTransInit[%i]\n",ret);
		*psRet = RC_FILE_ERR;
		return NO_DISP;
	}

	
 	gtPosCom.ucSwipedFlag = CONTACTLESS_ENTER; //guohonglv

//	ret = AppGetAmount(6, TRAN_AMOUNT,(char*)buf);
//	if(ret != OK) return ret;
	memcpy(gtPosCom.tEmvTrans.sAmount,psAmountBcd,6);

	memset(buf,0,sizeof(buf));
	dat_bcdtoasc( buf, gtPosCom.tEmvTrans.sAmount, 12 );

	tClssEcParam.ulAmntAuth = atol(buf);
	tClssEcParam.ulAmntOther = 0;
	//ClssEcParam.ucTransType = EMV_GOODS;
	tClssEcParam.ucTransType = 0;


	
	gettime(&sTime);
	
	tClssEcParam.aucTransDate[0] = ((sTime.tm_year/10)<<4)|(sTime.tm_year%10);
	tClssEcParam.aucTransDate[1] = ((sTime.tm_mon/10)<<4)|(sTime.tm_mon%10);
	tClssEcParam.aucTransDate[2] = ((sTime.tm_mday/10)<<4)|(sTime.tm_mday%10);

	tClssEcParam.aucTransTime[0] = ((sTime.tm_hour/10)<<4)|(sTime.tm_hour%10);
	tClssEcParam.aucTransTime[1] = ((sTime.tm_min/10)<<4)|(sTime.tm_min%10);
	tClssEcParam.aucTransTime[2] = ((sTime.tm_sec/10)<<4)|(sTime.tm_sec%10);
	
	tClssEcParam.ulTransNo = gtPosParameter.lNowTraceNo;


	
	result = Entry_PreTransProc((T_Clss_TransParam *)&tClssEcParam);  //guohonglv 不知道为什么，必须进行类型的强制转换

	TraceDisp("Entry_PreTransProc[%i]\n", result);
	
	if(result == CLSS_USE_CONTACT) 
	{

		*psRet = RC_USE_CONTACT;
		return (disp_clss_err(result));
		
	}

	if(result != EMV_OK)
	{
		*psRet = RC_FAILURE;
		return E_TRANS_FAIL;
	}

	ClrBelow(2);
	memset(buf,0,sizeof(buf));

	sprintf(tmp, "%012ld", tClssEcParam.ulAmntAuth);
	tableamount(tmp,12 );
	sprintf(buf,"金额: %s",tmp);
	lcdshow(1, MIDDLE, 1, buf);
	
	
	while(1){
	//if( (bRet=Picc_open()))
	//	printf("Picc open err [%02x]\n",bRet);
		
		//if(Picc_open() != 0){
		if(picc_open() != 0){
			lcdshow(2,MIDDLE,1,"射频模块故障");
			//Picc_close();
			picc_close();
			WaitKey(3);
			TraceDisp("picc_open err");
			*psRet = RC_READER_ERR;
			return NO_DISP;
		}
                
		PiccPara.wait_retry_limit_w = 1;
		PiccPara.wait_retry_limit_val = 0;
		PiccPara.card_buffer_w = 1;
		PiccPara.card_buffer_val = 256; 
		//Picc_setup('W',&PiccPara);
		picc_setup('W',&PiccPara);

	
		lcdshow(2,MIDDLE,1,"请出示射频卡");
		
		TimerSet(3,WAITTIMEFORDETECT);  //   10ms unit
		while(1)
		{
			//if(getkey() == KEY_ESC) 
			//	return E_TRANS_CANCEL;
			
			if( !TimerCheck(3) ) 
			{
				//*psRet = RC_TRAN_FAILURE;
				*psRet = RC_NO_CARD;
				return E_TRANS_FAIL;
			}
			//ret = Picc_detect(mode,NULL,NULL,NULL,NULL);
			ret = picc_detect(mode,NULL,NULL,NULL,NULL);
                        
			if(ret == 0) break;
			if(ret == 1 ||ret==2)
			{
				*psRet = RC_READER_ERR;
				return E_TRANS_FAIL;
			}
			if(ret == 3||ret==5||ret==6){
				delay_ms(100);
				continue;
			}
			if(ret == 4){
				lcdshow(2,MIDDLE,1,"感应区卡片过多");
				//Picc_close();
				picc_close();
				delay_ms(1000);
				break;
			}
		}
		if(ret == 4)
		{
			//continue;  //2011/03/10 disable,terminal should display
			*psRet = RC_MORE_CARDS;
			return E_TRANS_FAIL;
		}
		break;
	}

        
	ClrBelow(2);
	
	
	lcdshow(1, MIDDLE, 1, "请稍侯...");
//	memcpy(PosCom.stTrans.szEntryMode, "072", 3);
	buf[0] = 0x07;
	Contactless_SetTLVData(0x9f39,buf,1);
//	gtPosCom.ucSwipedFlag = CONTACTLESS_ENTER;
	Contactless_SetTLVData(0x9c,"\x00",1);
	
	
		
	result = Entry_AppSlt(0,0);

TraceDisp("Entry_AppSlt[%i]\n",result);
    
	if(   result == EMV_NO_APP ||result == ICC_BLOCK || result == EMV_APP_BLOCK) 
	{
		TraceDisp("Entry_AppSlt[%i]\n",result);
		if(result == EMV_NO_APP)
			*psRet = RC_EMV_NO_APP;
		else if(result == ICC_BLOCK)
			*psRet = RC_ICC_BLOCK;
		else
			*psRet = RC_EMV_APP_BLOCK;
			
		return(disp_clss_err(result));
	}
	if(result != EMV_OK)
	{
		if(result==ICC_CMD_ERR)
		{
			*psRet =RC_ICC_CMD_ERR;
			TraceDisp("交易终止_0");
			return E_TRANS_FAIL;
				
		}
		//*psRet = RC_TRAN_FAILURE;
		*psRet =RC_TERMINATE; // 11/06/04 交易终止
		TraceDisp("交易终止_1");
		return E_TRANS_FAIL;
	}

	while(1){
		memset(buf,0,sizeof(buf));

		result = Entry_FinalSelect(buf, Aid, &Aidlen);

	TraceDisp("Entry_FinalSelect[%i]\n",result);
	
		if(result == EMV_NO_APP || result == CLSS_USE_CONTACT ) 
		{
			if(result==EMV_NO_APP)
				*psRet = RC_EMV_NO_APP;
			else
				*psRet = RC_USE_CONTACT;
			return(disp_clss_err(result));
		}
		if(result != EMV_OK)
		{
			#ifdef PAYWAVE_M    //shall select the next application with highest priority when the SW1
								//SW2 response t the SELECT ADF command is not ‘9000’
			Entry_DelCurCandApp();
			continue;				
			#else
			if(result==CLSS_RESELECT_APP)
			{
				Entry_DelCurCandApp();
				continue;				
				
			}
			
			//*psRet = RC_TRAN_FAILURE;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return E_TRANS_FAIL;
			#endif
		}

//TraceDisp("kernel type[%i]\n",buf[0]);

		if( buf[0] != KERNTYPE_PBOC )
		{
			//*psRet = RC_TRAN_FAILURE;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return E_TRANS_FAIL;
		}
		/*pos_com.log.vis_qpboc_flag*/
                keyId = buf[0];

		QPboc_SetTLVData(0x9C,"\x00",1);
		result = Entry_GetPreProcInterFlg((T_Clss_PreProcInterInfo *)&tClssProcInterInfo);   // guohonglv 对参数进行了类型转换

	TraceDisp("Entry_GetPreProcInterFlg[%i]\n",result);
	
		if( result == CLSS_USE_CONTACT )
		{
			//*psRet = RC_USE_CONTACT;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return disp_clss_err(result);
		}
		else if( result == CLSS_TERMINATE)
		{
			*psRet = RC_TERMINATE;
			return disp_clss_err(result);

		}
			
		if( result != EMV_OK ) 
		{
			//*psRet = RC_TRAN_FAILURE;
			*psRet = RC_TERMINATE; //11/06/12
			return E_TRANS_FAIL;
		}
		
		result = Entry_GetFinalSelectData(inActype, &len);

	TraceDisp("Entry_GetFinalSelectData[%i]\n",result);
		if( result != EMV_OK )
		{
			//*psRet = RC_TRAN_FAILURE;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return E_TRANS_FAIL;
		}

		result = QPboc_SetFinalSelectData(inActype,len);
	TraceDisp("QPboc_SetFinalSelectData[%i]\n",result);
		
		if( result != EMV_OK )
		{
			//*psRet = RC_TRAN_FAILURE;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return E_TRANS_FAIL;
		}

		result = QPboc_SetTransData(&tClssEcParam, &tClssProcInterInfo);
	TraceDisp("QPboc_SetTransData[%i]\n",result);

		if( result != EMV_OK)
		{
			//*psRet = RC_TRAN_FAILURE;
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return E_TRANS_FAIL;
		}

		memset(tmp,0,sizeof(tmp));
		memset(outActype,0,sizeof(outActype));
		result = QPboc_Proctrans(tmp, outActype);

		
	TraceDisp("QPboc_Proctrans[%i]\n",result);
		if(result == CLSS_RESELECT_APP){ //
			Entry_DelCurCandApp();
			continue;
			//*psRet =RC_TERMINATE; // 11/06/08 交易终止
			//return E_TRANS_FAIL;
			
		}
		if(result == CLSS_USE_CONTACT ) //converto to the other card-> the other card(ex contact IC) not support -->terminate 11/06/08 
		{
			//*psRet = RC_USE_CONTACT; 
			*psRet =RC_TERMINATE; // 11/06/04 交易终止
			return(disp_clss_err(result));
		}
		if( (result == EMV_USER_CANCEL) || (result==EMV_DATA_ERR) 
			||(result == EMV_RSP_ERR) )   //2011/02/18
		{

			*psRet = RC_TERMINATE; 
			return(disp_clss_err(result));

		}

		
		if(result != EMV_OK) 
		{
			//CLSS_CARD_EXPIRED
			//PAYWAVE: ensure that if the card returns a TC, the reader shall continue the
			//transaction online if the Application Expiration Date is earlier than the
			//current terminal date and if the CTQ indicates “G Online If Application
			//Expired” is 1.
			if(result==CLSS_CARD_EXPIRED)
				//*psRet = RC_DECLINE;	
				*psRet = RC_CARD_EXPIRED;	//11/06/08
			else
				*psRet = RC_TRAN_FAILURE; 
			return E_TRANS_FAIL;
		}

		break;
	}

	if(tmp[0] == CLSS_VISA_MSD)
	{
		buf[0] = 0x91; //ENTRY MODE
		Contactless_SetTLVData(0x9f39,buf,1);

	}
	tag = 0x57;
	ret = Contactless_GetTLVData(tag,tmp,&len);
	if(ret != EMV_OK)
	{
		//*psRet = RC_TRAN_FAILURE; 
		*psRet =RC_TERMINATE; // 11/06/04 交易终止
		TraceDisp("交易终止_14\n");
		return E_TRANS_FAIL;
	}
	dat_bcdtoasc(gtPosCom.szTrack2,tmp,len*2);

	len = len*2;
	gtPosCom.szTrack2[len]=0;
	if((tmp[len/2-1] & 0x0f) == 0x0f){
		gtPosCom.szTrack2[len-1] = 0;
	}
	gtPosCom.szTrack2[37]=0;
	for(i=0;i<len;i++){
		if(gtPosCom.szTrack2[i] == 'D'){
			gtPosCom.szTrack2[i] = '=';
			break;
		}
	}
	ret = GetCardFromTrack(gtPosCom.szCardNo,gtPosCom.szTrack2,NULL);
	if(ret != EMV_OK)
	{
		//*psRet = RC_TRAN_FAILURE; 
		*psRet =RC_TERMINATE; // 11/06/04 交易终止
		TraceDisp("交易终止_15\n");
		return E_TRANS_FAIL;
	}


	gtPosCom.tEmvTrans.PANSN = 0xff;
	//PosCom.stTrans.bPanSeqNoOk = FALSE;
	ret = Contactless_GetTLVData(0x5f34,tmp,&len);
		
	if(ret == EMV_OK) 
	{
		//PosCom.stTrans.bPanSeqNoOk = TRUE;
		gtPosCom.tEmvTrans.PANSN = tmp[0];
	}
	
	if(outActype[0] == AC_AAC || 
		outActype[0]==0x11 )   //cryptogram type cannot be determined,the reader shall set the Decline
	{
		//return TRANS_REF;
		*psRet = RC_DECLINE;
		TraceDisp("交易拒绝_16\n");
		return 29;
	}
	if(outActype[0] == AC_ARQC){

		*psRet = RC_ONLINE;

	
		//使用宏
		//if(memcmp(Aid,"\xA0\x00\x00\x03\x33\x01\x01\x06",8) == 0){
			result = QPboc_GetTLVData(0x9f5d,tmp,&len);
			if(result == EMV_OK){
				/*disable tmp disable
				PubBcd2Asc(tmp,12,amt);
				if(memcmp(tmp,PosCom.stTrans.sAmount,6) < 0){
					ScrClrLine(2,7);
					PubDispString("卡余额不足",LINE4|DISP_LINE_CENTER);
					
					WaitKey(1);
					return NO_DISP;
				}
				*/
				/* disable 11/06/01		
				if( tClssProcInterInfo.ucRdCLFLmtExceed == 1) {

					ClrBelow(2);
					lcdshow(2, MIDDLE, 1, "纯电子钱包卡");
					lcdshow(3, MIDDLE, 1, "超过脱机交易限额");
					WaitKey(1);
					*psRet = RC_EXCEED_OFFLINEAMT;
					return NO_DISP;
				}
				*/
			}
		//}
		
		QPboc_nGetTrack2MapData(tmp, &len);//track2等价数据 发送到terminal 2011/02/21

		QPboc_GetCvmType(tmp);


		return 0;
		//return(Clss_transmit(1)); disable tmp 10/10/18
	}
	
//11/06/08
//	QPboc_DelAllRevocList(); 
//	QPboc_DelAllCAPK();

	
	QPboc_GetTLVData(0x82,buf,&len);
	if( (buf[0] & 0x20)==0x20 ) //support dda 11/06/08
	{
		len = 0;
	
		result = QPboc_GetTLVData(0x8f,buf,&len);
		if(result == EMV_OK)
		{
			if(AppSetClss_capk(buf[0],Aid))
			{

			*psRet = RC_DECLINE;
			TraceDisp("交易拒绝_18\n");
			return EMV_NOT_FOUND;

			}
		//		AppSetClss_RevocList(buf[0],tmp);
		}
	}	

	result = QPboc_CardAuth(buf, tmp);
	//result = Clss_GetTLVData(0x9f6c,tmp,&len);

	TraceDisp("QPboc_CardAuth[%i] buf[%02x]\n",result,buf[0]);

	if(result == CLSS_USE_CONTACT)
	{
		//*psRet = RC_USE_CONTACT;
		*psRet =RC_TERMINATE; // 11/06/04 交易终止
		return(disp_clss_err(result));
	}

	if(result != EMV_OK)
	{
		//*psRet = RC_TRAN_FAILURE;
		*psRet =RC_TERMINATE; // 11/06/04 交易终止
		return E_TRANS_FAIL;
	}
	if(buf[0] == AC_ARQC) 
	{
		QPboc_GetCvmType(tmp);

		*psRet = RC_ONLINE;
		return 0;
		//return(Clss_transmit(1)); disable tmp 10/10/18
	}
	else if(buf[0] == AC_TC)
	{

		//11/05/31
		QPboc_GetCvmType(tmp);
		if((int)tmp[0]==RD_CVM_ONLINE_PIN)
		{
			*psRet = RC_ONLINE;
			//sdk_ScrDispRow("交易失败_21在线", 0, 1);
        	 TraceDisp("该交易需联机");
		 	return 0;

		}

		 //11/06/02 
		/* move to app 
		result = QPboc_GetTLVData(0x5A,tmp,&len);
		if(result == EMV_OK)
		{
			
			result = QPboc_GetTLVData(0x5F34,buf,&i);
			if(QPboc_SearchExceptionList(len,tmp,buf)==0)
			{
				*psRet = RC_DECLINE;
				sdk_ScrDispRow("交易拒绝-24", 0, 1);
				DisInquire();
				//return 29;
				return RC_DECLINE;

			}
		}
		*/
		
		return EMV_OK;
	}
	else  //the other is AAC
	{

		*psRet = RC_DECLINE;
		//return 29;
		return RC_DECLINE;

	}

	/* disable tmp 10/10/18
	Beep();

	result = QPboc_GetTLVData(0x9f5d,tmp,&len);
	if(result == EMV_OK){

		PubBcd2Asc(tmp,6,PosCom.stTrans.Ec_Balance);
		PosCom.stTrans.Ec_Balance[12]=0;
		memset(buf,0,sizeof(buf));
		ConvBcdAmount(tmp,buf);
		ScrClrLine(2,7);
		PubDispString("卡余额:",LINE5|DISP_LINE_CENTER);
		PubDispString(buf,LINE7|DISP_LINE_CENTER);
		//需要修改
		PubWaitKey(5);
	
	}

	Clss_SetTLVData(0x9f27,"\x40",1);
	Clss_SetTLVData(0x8a,"Y1",2);
	// PrepareDe55(EMV_OFF_SEND);
	InceaseTraceNo();
	
	*/
	return EMV_OK;

}


#endif
