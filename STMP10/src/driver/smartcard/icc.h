#ifndef  _ICCARD_H
#define  _ICCARD_H
 
#ifndef   uchar
#define   uchar   unsigned char
#endif
#ifndef   uint
#define   uint    unsigned int
#endif
#ifndef   ushort  
#define   ushort  unsigned short
#endif
#ifndef   ulong
#define   ulong   unsigned long
#endif

#define   IN               1
#define   OUT              0
#define   LOW              0
#define   HIGH             1
#define   ON               1
#define   OFF              0
#define   Enable           1
#define   DisEnable        0
#define   CLEAR            0
/*******************************************
             IC卡处理状态
 *******************************************/
#define SUCCESS                    0
#define ICC_VCCMODEERR             (-2500)   //   电压模式错误 
#define ICC_INPUTSLOTERR           (-2501)   //   选择通道口错误 
#define ICC_VCCOPENERR             (-2502)   //
#define ICC_ICCMESERR              (-2503)   //   卡通讯失败 

#define ICC_T0_TIMEOUT             (-2200)   //   等待卡片响应超时   
#define ICC_T0_MORESENDERR         (-2201)   //   重发错误 
#define ICC_T0_MORERECEERR         (-2202)   //   重收错误 
#define ICC_T0_PARERR              (-2203)   //   字符奇偶错误 
#define ICC_T0_INVALIDSW           (-2204)   //   状态字节无效 

#define ICC_DATA_LENTHERR          (-2400)   //   数据长度错误   
#define ICC_PARERR                 (-2401)   //   奇偶错误       
#define ICC_PARAMETERERR           (-2402)   //   参数值为空 
#define ICC_SLOTERR                (-2403)   //   卡通道错误    
#define ICC_PROTOCALERR            (-2404)   //   协议错误      
#define ICC_CARD_OUT               (-2405)   //   卡拨出        
#define ICC_NO_INITERR             (-2406)   //   没有初始化    
#define ICC_ICCMESSOVERTIME        (-2407)   //   卡通讯超时 

#define ICC_ATR_TSERR              (-2100)   //   正反向约定错误,TS错误 
#define ICC_ATR_TCKERR             (-2101)   //   复位校验(T=1,TCK错误)错误     
#define ICC_ATR_TIMEOUT            (-2102)   //   复位等待超时     
#define ICC_TS_TIMEOUT             (-2115)   //   字符间隔超时     
#define ICC_ATR_TA1ERR             (-2103)   //   TA1错误          
#define ICC_ATR_TA2ERR             (-2104)   //   TA2错误          
#define ICC_ATR_TA3ERR             (-2105)   //   TA3错误          
#define ICC_ATR_TB1ERR             (-2106)   //   TB1错误          
#define ICC_ATR_TB2ERR             (-2107)   //   TB2错误          
#define ICC_ATR_TB3ERR             (-2108)   //   TB3错误          
#define ICC_ATR_TC1ERR             (-2109)   //   TC1错误          
#define ICC_ATR_TC2ERR             (-2110)   //   TC2错误          
#define ICC_ATR_TC3ERR             (-2111)   //   TC3错误          
#define ICC_ATR_TD1ERR             (-2112)   //   TD1错误          
#define ICC_ATR_TD2ERR             (-2113)   //   TD2错误          
#define ICC_ATR_LENGTHERR          (-2114)   //   ATR数据长度错误   

#define ICC_T1_BWTERR              (-2300)   //   T=1字组等待时间错误   
#define ICC_T1_CWTERR              (-2301)   //   T=1字符等待时间错误  
#define ICC_T1_ABORTERR            (-2302)   //   异常(ABORT)通信错误  
#define ICC_T1_EDCERR              (-2303)   //   字组校验码(EDC)错误  
#define ICC_T1_SYNCHERR            (-2304)   //   同步通信错误 
#define ICC_T1_EGTERR              (-2305)   //   字符保护时间错误   
#define ICC_T1_BGTERR              (-2306)   //   字组保护时间错误    
#define ICC_T1_NADERR              (-2307)   //   字组中NAD错误        
#define ICC_T1_PCBERR              (-2308)   //   字组PCB错误          
#define ICC_T1_LENGTHERR           (-2309)   //   字组LEN错误          
#define ICC_T1_IFSCERR             (-2310)   //   IFSC错误             
#define ICC_T1_IFSDERR             (-2311)   //   IFSD错误             
#define ICC_T1_MOREERR             (-2312)   //   多次传送错误并放弃   
#define ICC_T1_PARITYERR           (-2313)   //   字符奇偶错误 
#define ICC_T1_INVALIDBLOCK        (-2314)   //   无效的字组 

#define ICC_ER_DAIN                (-2600)   //   ERror: Driver Already INitialized 
#define ICC_ER_DNIN                (-2601)   //   ERror: Driver Not INitialized 
#define ICC_ER_NOCD                (-2602)   //   ERror: NO sim CarD is present  
#define ICC_ER_SYSF                (-2603)   //   ERror: SYStem Failure  
#define ICC_ER_TMOT                (-2604)   //   ERror: TiMeOuT  
#define ICC_ER_AFTM                (-2605)   //   ERror: Atr arrived at False TiMe  
#define ICC_ER_INVA                (-2606)   //   ERror: INValid Atr  
#define ICC_ER_PAER                (-2607)   //   ERror: PArity ERror 奇偶效验错误 
#define ICC_ER_FRAM                (-2608)   //   ERror: FRAMing error 
#define ICC_ER_EDCO                (-2609)   //   ERror: Error Detection Code error  
#define ICC_ER_INFR                (-2610)   //   ERror: INvalid FRequency         
#define ICC_ER_INFN                (-2611)   //   ERror: INvalid FN                
#define ICC_ER_INDN                (-2612)   //   ERror: INvalid DN                
#define ICC_ER_INPA                (-2613)   //   ERror: INvalid PArameter         
#define ICC_ER_TOPS                (-2614)   //   ERror: Timeout for PpS response  
#define ICC_ER_INPS                (-2615)   //   ERror: INvalid PpS response      
#define ICC_ER_DOVR                (-2616)   //   ERror: Data OVeRflow on fifo reception buffer 接收字符溢出 
#define ICC_ER_NSFN                (-2617)   //   ERror: Not Supported FN          
#define ICC_ER_NSDN                (-2618)   //   ERror: Not Supported DN          
#define ICC_ER_NSPR                (-2619)   //   ERror: Not Supported PRotocol    
#define ICC_ER_MEMF                (-2620)   //   ERror: MEMory Failure            

#define  SEND_OK            0          //SEND data OK  
#define  RECE_OK            0          // RECE data OK  

#define   IC_MAXSLOT       3   //4->3
#define   USER_CARD        0
#define   SAM1_CARD        1
#define   SAM2_CARD        2
#define   SAM3_CARD        3

#define   VCC_OPEN         1
#define   VCC_CLOSE        0

#define   CRD_VCC_0V       0
#define   CRD_VCC_5V       1
#define   CRD_VCC_3V       2
#define   CRD_VCC_1_8V     3

#define TWT_READY  			       0
#define TWT_START  			       1
#define TWT_STOP   			       2

#define SC_UART_WAIT_TS          ( 1 << 0 )    //
#define SC_UART_RECV_EV          ( 1 << 1 )    //
#define SC_UART_SEND_EV          ( 1 << 2 )    //

#define SC_RECEIVE_TIMEOUT 0x4000  /* Direction to reader */

/* Define the STM32F10x hardware depending on the used evaluation board */

 #define CARD_V2

  //hardware
  #define SC_USART                 USART3

  #define SC_SML_UART              USART3

  #define SC_USART_GPIO            GPIOB
  #define SC_USART_CLK             RCC_APB1Periph_USART3
  #define SC_USART_GPIO_CLK        RCC_APB2Periph_GPIOB
  #define SC_USART_PIN_TX          GPIO_Pin_10
  #define SC_USART_PIN_CK          GPIO_Pin_12
  //#define SC_USART_IRQn            USART3_IRQn
  #define SC_USART_IRQn            USART3_IRQChannel

  #define SC_USART_IRQHandler      USART3_IRQHandler
  /* Smartcard Inteface GPIO pins */
  #define SC_PIN_3_5V_SAM1               GPIO_Pin_0
  #define SC_PIN_3_5V_SAM2               GPIO_Pin_1

  #define SC_PIN_3_5V_GPIO          GPIOB
  #define SC_PIN_3_5V_GPIO_CLK      RCC_APB2Periph_GPIOB
  #define SC_PIN_RESET_SAM1              GPIO_Pin_3
#ifndef CARD_V2
  #define SC_PIN_RESET_SAM2              GPIO_Pin_4
#endif
  #define SC_PIN_RESET_GPIO         GPIOC
  #define SC_PIN_RESET_GPIO_CLK     RCC_APB2Periph_GPIOC
  #define SC_PIN_CMDVCC             GPIO_Pin_2
  #define SC_PIN_CMDVCC_GPIO        GPIOC
  #define SC_PIN_CMDVCC_GPIO_CLK    RCC_APB2Periph_GPIOC

//智能卡检测
  #define SC_PIN_OFF                GPIO_Pin_7 
  #define SC_PIN_OFF_GPIO           GPIOC
  #define SC_PIN_OFF_GPIO_CLK       RCC_APB2Periph_GPIOC


  #define SC_DETECT_EXTI            EXTI_Line7
  #define SC_DETECT_PIN             GPIO_PortSourceGPIOC
  #define SC_DETECT_GPIO            GPIO_PinSource7
  #define SC_DETECT_IRQ             EXTI9_5_IRQn
  #define SC_DETECT_IRQHandler      EXTI9_5_IRQHandler


typedef struct //__attribute__ ((__packed__)) 
{
     uchar open;         //   0 关闭 1 打开   

     uchar existent;     //   0 无卡 1 有卡  
     uchar resetstatus;  //   0-没有复位；1-已冷复位；2-已热复位  
     uchar autoresp;     //   自动应答设置：0=自动应答  1=非自动应答  
     
     uchar D;            //   修改异步通信波特率变量因子    
     uchar T;            //   卡片协议类型  值为0或1  
     uchar TS;           //   卡正、反向约定  3B=正向 3F=反向  

     uchar ATR_len;      //   卡片复位应答长度  
     uchar ATR[40];      //   卡片复位应答内容  
     
     uchar TA;           //   低4位有效，对应于TA1、TA2、TA3,TA4的存在   
     uchar TA1;          //   卡片复位接口字符    
     uchar TA2;          //   卡片复位接口字符   
     uchar TA3;          //   卡片复位接口字符   
     uchar TA4;          //   卡片复位接口字符    
     
     uchar TB;           //   低4位有效，对应于TB1、TB2、TB3,TB4的存在   
     uchar TB1;          //   卡片复位接口字符    
     uchar TB2;          //   卡片复位接口字符    
     uchar TB3;          //   卡片复位接口字符   
     uchar TB4;          //   卡片复位接口字符   
     
     uchar TC;           //   低4位有效，对应于TC1、TC2、TC3,TC4的存在   
     uchar TC1;          //   卡片复位接口字符    
     uchar TC2;          //   卡片复位接口字符    
     uchar TC3;          //   卡片复位接口字符    
     uchar TC4;          //   卡片复位接口字符   
     
     uchar TD;           //   低4位有效，对应于TD1、TD2、TD3,TD4的存在     
     uchar TD1;          //   卡片复位接口字符    
     uchar TD2;          //   卡片复位接口字符    
     uchar TD3;          //   卡片复位接口字符    
          
     uchar IFSC;         //   卡片允许最大字组信息长度    
     uchar IFSD;         //   终端允许最大字组信息长度   
     
     uchar term_pcb;     //   T=1协议下终端发送下个字段格式与序列号   
     uchar card_pcb;     //   T=1协议下卡发送下个字段格式与序列号    
     
     uchar sw1;          //   状态字节1   
     uchar sw2;          //   状态字节2   
}ASYNCARD_INFO;

typedef struct
{
    uchar qbuf[300];
    uchar ip;
    uchar op;
    uchar sf;
    uchar pf;
}emv_queue;

#if 0
struct emv_core:

terminal_ch  //slot
terminal_ptype //protocol
queue
terminal_di
terminal_implict_di
terminal_fi
terminal_implict_fi
terminal_conv  //方向
terminal_cgt //? guard
terminal_bwt
terminal_cwt
terminal_wwt
terminal_vcc
terminal_state //EMV_COLD_RESET
terminal_spec //emv 7816
#endif
//hard
#define FCLK_CARD     ( 3600000UL )


#endif