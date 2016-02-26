#define PHCS_BFLOPCTL_VAL_MATRIX_END           	0xC3    											/*!< Defines the end of the current record to be read. */
#define PHCS_BFLOPCTL_VAL_BEG_PARAM_SET(param_set)          	u8   (param_set)[][3] = {
#define PHCS_BFLOPCTL_VAL_PARAM_ENTRY(param1, param2, param3) 	{(param1), (param2), (param3)},
#define PHCS_BFLOPCTL_VAL_END_PARAM_SET                       	{PHCS_BFLOPCTL_VAL_MATRIX_END, 0x00, 0x00}};

#define PHCS_BFLOPCTL_VAL_ADDRESS_POS   	0x00
#define PHCS_BFLOPCTL_VAL_AND_POS              	0x01
#define PHCS_BFLOPCTL_VAL_OR_POS               	0x02

PHCS_BFLOPCTL_VAL_BEG_PARAM_SET(PHCS_BFLOPCTL_VAL_HW4PASI_106)
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_TXAUTO, 	        0xFF,               PHCS_BFL_JBIT_FORCE100ASK)    /* TXAutoReg:       clear bit Force100ASK */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_TXBITPHASE,PHCS_BFL_JBIT_RCVCLKCHANGE,  0x0F)                /* TxBitphaseReg:   unchanged bit RcvClkChange, set other to 0x0F */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_RXTRESHOLD,	        0x00,               0x55)                /* RxTresholdReg:   set to 0x55 */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_MODWIDTH,     	0x00,               0x26)                /* ModWidthReg:     set to 0x26 */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_DEMOD,        	0x00,               0x4D)                /* DemodReg:        set to 0x4D */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_GSNLOADMOD,   	0x00,               0x6F)                /* GsNLoadModReg:   set to 0x6F */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_RFCFG,        	0x00,               0x59)                /* RFCfgReg:        set to 0x59 */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_GSN,          	0x00,               0xF4)                /* GsNReg:          set to 0xF4 */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_CWGSP,        	0x00,               0x3F)                /* CWGsPReg:        set to 0x3F */    
	PHCS_BFLOPCTL_VAL_PARAM_ENTRY(PHCS_BFL_JREG_MODGSP,       	0x00,               0x11)                /* ModGsPReg:       set to 0x11 */
PHCS_BFLOPCTL_VAL_END_PARAM_SET

/***********************************************************************************************/
#define REQA     	0x26 	/* Request code for all devices. */
#define WUPA     	0x52	/* Request code only for halt devices. */
#define SELECTA1  	0x93 	/* Anticollision/Select codes */
#define SELECTA2  	0x95 	/* Anticollision/Select codes */
#define SELECTA3  	0x97 	/* Anticollision/Select codes */
#define HALTA		0X50	/* definitions for HaltA function.*/
#define NULLCODE	0X00	




#define UINT8     unsigned char
#define uint8_t	  unsigned char 
#define uint16_t  unsigned short 

typedef struct ISO14443_3A_Request_t
{
	UINT8	Code;
	UINT8	AtqaValue[2];
	UINT8	UidLength;
	UINT8	BitCol;
} ISO144433ARequest;

typedef struct ISO14443_3A_Select_t
{    
	UINT8	Code;
	UINT8 	BitLength;
	UINT8	ColFlags;	
	UINT8	NvbValue;	
	UINT8	UidLength;
	UINT8	UidValue[12];
	UINT8	SakValue;
	UINT8	CrcValue;
} ISO144433ASelect;

typedef struct ISO14443_3A_Halt_t
{    
	UINT8	Code;
	UINT8	CrcValue;
} ISO144433AHalt;

typedef struct ISO14443_3A_Command_t
{     
	ISO144433ARequest  	Request;               
  	ISO144433ASelect  	Select;   
	ISO144433AHalt		Halt;
} TypeA3CmdSet;

	
TypeA3CmdSet 	ISO14443_3A_Command ;

typedef struct 
{  
	uint8_t     cmd;              		/*!< \brief [in] command code                                          */  
	uint8_t     initiator;        		/*!< \brief [in] Initiator mode configuration                          */  
	uint8_t     target_send;      	/*!< \brief [in] In target mode configuration differs between                                                  
								send and receive mode for transceive.                 */  
	uint8_t    *tx_buffer;        		/*!< \brief [in] References data to transmit via the UART.             */  
	uint16_t    tx_bytes;         		/*!< \brief [in] Number of bytes to transmit. if there are only                                                 
								bits to transmit, set length to full byte             */  
	uint8_t    *rx_buffer;        		/*!< \brief [in] Place where to put received bytes.\n [out] data.         */  
	uint16_t    rx_bytes;         		/*!< \brief [in] Maximum return buffer size[bytes]. \n                                          
								[out] Number of bytes returned.                             */  
	uint16_t    rx_bits;         		/*!< \brief [out] Number of bits returned (additional to bytes).        */
} phcsBflAux_CommandParam_t;

typedef struct CommandParam_t
{
  uint8_t   cmd;
  uint8_t   *tx_buffer;   
  uint8_t   tx_bytes;
  uint8_t   *rx_buffer;   
  uint8_t   rx_bytes;  
  uint8_t   rx_bits;  
} YXHCommandParam;

YXHCommandParam  TestCmd;

/***********************************************************************************************/
