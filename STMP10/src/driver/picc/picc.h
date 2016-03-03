#ifndef _PICC_H
#define _PICC_H

#ifndef   uchar
#define   uchar   unsigned char
#endif
#ifndef   BYTE  
#define   BYTE    unsigned char
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


//#define USIP_MCU_M
#undef USIP_MCU_M
#define STM32F_MCU_M

#define PICC_REG_COMMON_ID    0 
#define PICC_REG_CARRIER_ID   1
//#define FAKE_FOR_TEST  
///////////////////////////////////////////////////////////////////// 
//MF522寄存器定义 
///////////////////////////////////////////////////////////////////// 
// PAGE 0 
#define     rRFU00                  0x00     //Reserved for future use
#define     rCommandReg             0x01     //Starts and stops command execution
#define     rComIEnReg              0x02     //Controls bits to enable and disable the passing of interrupt Requests
#define     rDivIEnReg              0x03     //Controls bits to enable and disable the passing of interrupt Requests
#define     rComIrqReg              0x04     //Contains interrupt Request bits
#define     rDivIrqReg              0x05     //Contains interrupt Request bits
#define     rErrorReg               0x06     //Error Bits showing the error status of the last command executed
#define     rStatus1Reg             0x07     //contains status bits for communication
#define     rStatus2Reg             0x08     //contains status bits of the receiver and transmitter
#define     rFIFODataReg            0x09     //in-and output of 64 byte FIFO buffer
#define     rFIFOLevelReg           0x0A     //Indicates the number of bytes stored in the FIFO
#define     rWaterLevelReg          0x0B     //Defines the level for FIFO under and overflow warning
#define     rControlReg             0x0C     //Contains miscellaneous control registers
#define     rBitFramingReg          0x0D     //adjustments for bit oriented frames
#define     rCollReg                0x0E     //bit position of the first bit collision detected on the rf-interface
#define     rRFU0F                  0x0F     //reserved for future use
// PAGE 1      
#define     rRFU10                  0x10     //reserved for future use
#define     rModeReg                0x11     //defines general modes for transmitting and receiving
#define     rTxModeReg              0x12     //defines the transmission data rate and framing
#define     rRxModeReg              0x13     //defines the receive data rate and framing
#define     rTxControlReg           0x14     //controls the logical behavior of the antenna driver pins TX1 and TX2
#define     rTxASKReg               0x15     //controls the setting of the TX modulation
#define     rTxSelReg               0x16     //selects the internal sources for the antenna driver
#define     rRxSelReg               0x17     //selects interrnal receiver steeings
#define     rRxThresholdReg         0x18     //selects thresholds for the bit decoder
#define     rDemodReg               0x19     //defines demodulator settings
#define     rRFU1A                  0x1A     //reserved for future use
#define     rRFU1B                  0x1B     //reserved for future use
#define     rMfTxReg                0x1C     //controls some MIFARE communication transmit parameters
#define     rMfRxReg                0x1D     //controls some MIFARE communication receive parameters
#define     rTypeBReg               0x1E     //Configure the ISO/IEC 14443 B functionality
#define     rSerialSpeedReg         0x1F     //selects the speed of the serial UART interface
// PAGE 2     
#define     rRFU20                  0x20     //reserved for future use
#define     rCRCResultRegM          0x21     //shows the actual MSB values of the CRC calculation
#define     rCRCResultRegL          0x22     //shows the actual LSB values of the CRC calculation
#define     rRFU23                  0x23     //reserved for future use
#define     rModWidthReg            0x24     //controls the setting of the ModWidth
#define     rRFU25                  0x25     //reserved for future use
#define     rRFCfgReg               0x26     //configures the receiver gain
#define     rGsNReg                 0x27     //selects the conductance of the antenna driver pins TX1
#define     rCWGsPReg               0x28     //and TX2 for modulation
#define     rModGsPReg              0x29 
#define     rTModeReg               0x2A     //defines setting for the interrnal timer
#define     rTPrescalerReg          0x2B 
#define     rTReloadRegH            0x2C     //describes the 16 bit timer reload value
#define     rTReloadRegL            0x2D 
#define     rTCounterValRegH        0x2E     //shows the 16 bit actual timer value
#define     rTCounterValRegL        0x2F 
// PAGE 3       
#define     rRFU30                  0x30     //reserved for future use
#define     rTestSel1Reg            0x31     //general test signal configuration
#define     rTestSel2Reg            0x32     //general test signal configuration the PRBS control
#define     rTestPinEnReg           0x33     //enables pin output driver on D1-D7
#define     rTestPinValueReg        0x34     //defines the values for D1-D7 when it is used as I/O bus
#define     rTestBusReg             0x35     //shows the status of the internal testbus
#define     rAutoTestReg            0x36     //controls the digital selftest
#define     rVersionReg             0x37     //shows the version
#define     rAnalogTestReg          0x38     //controls the pins AUX1 and AUX2
#define     rTestDAC1Reg            0x39     //defines the test value for the testDAC1
#define     rTestDAC2Reg            0x3A     //defines the test value for the TestDAC2
#define     rTestADCReg             0x3B     //shows the actual value of ADC I and Q
#define     rRFU3C                  0x3C    
#define     rRFU3D                  0x3D    
#define     rRFU3E                  0x3E    
#define     rRFU3F		            0x3F 
 

// P C D - C O M M A N D S
#define PCD_IDLE            0x00    // No action: cancel current command execution. 
#define PCD_MEM             0x01    // Stores 25 byts into the internal buffer 
#define PCD_GENRNDID        0x02    // Generates a 10 byte random ID number
#define PCD_CALCCRC         0x03    // Activate the CRC-Coprocessor or performs a selftest. 
#define PCD_TRANSMIT        0x04    // Transmit data from FIFO to the card  
#define PCD_NOCMDCHANGE     0x07    // No command change.This command can be used to modify different bits
                                    // in the command register without touching the command E.g. Power-down                              
#define PCD_RECEIVE         0x08    // Activate Receiver Circuitry. Before the 
                                    // receiver actually starts, the state machine 
                                    // waits until the time 
                                    // configured in the register RcvWait has 
                                    // passed.
                                    // Remark: It is possible to read data from the 
                                    // FIFO although Receive is active. Thus it is 
                                    // possible to receive any number of bytes by 
                                    // reading them from the FIFO in time.   
#define PCD_TRANSCEIVE      0x0C    // This Command has two modes:\n 
                                    // Transmits data from FIFO to the card and after  
                                    // that automatically activates  
                                    // the receiver. Before the receiver actually  
                                    // starts,the state machine waits until the  
                                    // time configured in the register RxWait has  
                                    // passed. \n  
                                    // Remark: This command is the combination of  
                                    // Transmit and Receive.  
#define PCD_AUTHENT         0x0E    // Perform the card 
                                    // authentication using the Crypto1 algorithm. 
#define PCD_SOFTRESET       0x0F    // Runs the Reset- and Initialisation Phase
                                    // Remark: This command can not be activated by 
                                    // software, but only by a Power-On or 
                                    // Hard Reset 
                                
                                
//////////////////////////////////////////////////////////////////////////////////// 
//Bit Definitions 
//////////////////////////////////////////////////////////////////////////////////
//name RC523 Bit definitions of Page 0 
///ingroup reg 
//Below there are useful bit definition of the RC522 register set of Page 0. 
////////////////////////////////////////////////////////////////////////////////// 
// Command Register		 (01)  
#define     BIT_RCVOFF             0x20   //Switches the receiver on/off. 
#define     BIT_POWERDOWN          0x10   //Switches RC522 to Power Down mode 
#define     BIT_COMMAND            0x0F   //command content
 
// CommIEn Register	      (02)  
#define     BIT_IRQINV             0x80   //1: the signal on pin IRQ is inverted with respect to bit IRq IN the register Status1Reg;
                                          //0: THE signal on pin IRQ is equal to bit IRq.In combination with bit IRqPushPull 
                                          //   in register DIVIENReg,the default value of logic 1 ensures, that the output level on pin 
                                          //   IRQ is tristate.
#define     BIT_TXIEN              0x40   //Allows the transmitter interrupt request(indicated by bit TXIRQ)
                                          //to be propagated to pin IRQ.
#define     BIT_RXIEN              0x20   //Allows the receiver interrupt request(indicated by bit RXIRq) to be 
                                          //propagated to pin IRQ.
#define     BIT_IDLEIEN            0x10   //Allows the idle interrupt request(indicated by bit IdleRq) to be 
                                          //propagated to pin IRQ.
#define     BIT_HIALERTIEN         0x08   //Allows the high alert interrupt request.
#define     BIT_LOALERTIEN         0x04   //Allows the low alert interrupt request.
#define     BIT_ERRIEN             0x02   //Allows the error interrupt request.
#define     BIT_TIMERIEN           0x01   //Allows the timer interrupt request.                                                                                                                              
 
// DivIEn Register	 (03)  
#define     BIT_IRQPUSHPULL        0x80   //Sets the IRQ pin to Push Pull mode.  
#define     BIT_MFINACTIEN         0x10   //Allows the MFIN active interrupt request to be propagated 
#define     BIT_CRCIEN             0x04   //Allows the CRC interrupt request.
 
//  CommIrq Register    ( 04)  
#define     BIT_SET1               0x80   
#define     BIT_TXIRQ              0x40   //Bit position for Transmit Interrupt Enable/Request.   
#define     BIT_RXIRQ              0x20   //Bit position for Receive Interrupt Enable/Request.   
#define     BIT_IDLEIRQ            0x10   //Bit position for Idle Interrupt Enable/Request. 
#define     BIT_HIALERTIRQ         0x08   //Bit position for HiAlert Interrupt Enable/Request.   
#define     BIT_LOALERTIRQ         0x04   //Bit position for LoAlert Interrupt Enable/Request.   
#define     BIT_ERRIRQ             0x02   //Bit position for Error Interrupt Enable/Request.   
#define     BIT_TIMERIRQ           0x01   //Bit position for Timer Interrupt Enable/Request.  
 
// DivIrq Register          (05) 
#define     BIT_SET2               0x80   //set2 defines;
#define     BIT_MFINACTIRQ         0x10   //Bit position for SiginAct Interrupt Enable/Request 
#define     BIT_CRCIRQ             0x04   //Bit position for CRC Interrupt Enable/Request. 
 
//Error Register 				 (06)  
#define     BIT_WRERR              0x80   //Bit position for Write Access Error.  
#define     BIT_TEMPERR            0x40   //Bit position for Temerature Error. 
#define     BIT_BUFFEROVFL         0x10   //Bit position for Buffer Overflow Error.  
#define     BIT_COLLERR            0x08   //Bit position for Collision Error.  
#define     BIT_CRCERR             0x04   //Bit position for CRC Error.  
#define     BIT_PARITYERR          0x02   //Bit position for Parity Error.   
#define     BIT_PROTOCOLERR        0x01   //Bit position for Protocol Error. 
 
//Status 1 Register 		  (07)  
#define     BIT_CRCOK              0x40   //Bit position for status CRC OK.  
#define     BIT_CRCREADY           0x20   //Bit position for status CRC Ready.  
#define     BIT_IRQ                0x10   //Bit position for status IRQ is active. 
#define     BIT_TRUNNUNG           0x08   //Bit position for status Timer is running.  
#define     BIT_HIALERT            0x02   //Bit position for status HiAlert.   
#define     BIT_LOALERT            0x01   //Bit position for status LoAlert.   
 
//Status 2 Register				 (08)  
#define     BIT_TEMPSENSCLEAR      0x80   //Bit position to switch Temperture sensors on/off. 
#define     BIT_I2CFORCEHS         0x40   //Bit position to forece High speed mode for I2C Interface.  
#define     BIT_MFCRYPTO1ON        0x08   //Bit position for reader status Crypto is on. 
#define     BIT_MODEMSTATE         0x07   //Shows the state of the transmitter and receiver state machines.
 
 
//FIFOLevel Register			 	(0A)  
#define     BIT_FLUSHBUFFER        0x80   //Clears FIFO buffer if set to 1 
 
//Control Register				    (0C) 
#define     BIT_TSTOPNOW           0x80   //Stops timer if set to 1.  
#define     BIT_TSTARTNOW          0x40   //Starts timer if set to 1.  
 
//BitFraming Register			   (0D)  
#define     BIT_STARTSEND          0x80   //Starts transmission in transceive command if set to 1.  
 
//BitFraming Register			   (0E)  
#define     BIT_VALUESAFTERCOLL    0x80   //Activates mode to keep data after collision.  
 
//name RC522 Bit definitions of Page 1 
// \ingroup reg 
// Below there are useful bit definition of the RC522 register set of Page 1. 
  
 
//Mode Register		 (11) 
#define     BIT_TXWAITRF           0x20   //Tx waits until Rf is enabled until transmit is startet, else  
                                           //transmit is started immideately.  
#define     BIT_POLSIGIN           0x08   //Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures 
                                           //when Sigin line is 0.  
 
//TxMode Register			 	(12)  
#define     BIT_INVMOD             0x08   //Activates inverted transmission mode. 
 
//RxMode Register			 		(13) 
#define     BIT_RXNOERR            0x08   //If 1, receiver does not receive less than 4 bits.  
 
//Definitions for Tx and Rx		    (12, 13)  
#define     BIT_106KBPS            0x00   //Activates speed of 106kbps.  
#define     BIT_212KBPS            0x10   //Activates speed of 212kbps.   
#define     BIT_424KBPS            0x20   //Activates speed of 424kbps.   
#define     BIT_CRCEN              0x80   //Activates transmit or receive CRC. 
 
//TxControl Register						(14) 
#define     BIT_INVTX2ON           0x80   //Inverts the Tx2 output if drivers are switched on.  
#define     BIT_INVTX1ON           0x40   //Inverts the Tx1 output if drivers are switched on.  
#define     BIT_INVTX2OFF          0x20   //Inverts the Tx2 output if drivers are switched off. 
#define     BIT_INVTX1OFF          0x10   //Inverts the Tx1 output if drivers are switched off.   
#define     BIT_TX2CW              0x08   //Does not modulate the Tx2 output, only constant wave.  
#define     BIT_TX2RFEN            0x02   //Switches the driver for Tx2 pin on.  
#define     BIT_TX1RFEN            0x01   //Switches the driver for Tx1 pin on. 
 
//Demod Register 							(19)  
#define     BIT_FIXIQ              0x20   //If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel. 
                                           //If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. 
 
//\name RC522 Bit definitions of Page 2 
//\ingroup reg 
//Below there are useful bit definition of the RC522 register set. 
 
//TMode Register 			 (2A)  
#define     BIT_TAUTO              0x80   //Sets the Timer start/stop conditions to Auto mode.   
#define     BIT_TAUTORESTART       0x10   //Restarts the timer automatically after finished 
                                           // counting down to 0.  
 
/////////////////////////////////////////////////////////////////////////////// 
// Bitmask Definitions 
///////////////////////////////////////////////////////////////////////////////
//name RC522 Bitmask definitions 
//\ingroup reg 
//Below there are some useful mask defintions for the RC522. All specified  
//bits are set to 1.  
//Command register                 (0x01) 
#define     MASK_COMMAND           0x0F   //Bitmask for Command bits in Register JREG_COMMAND.   
 
//Waterlevel register              (0x0B) 
#define     MASK_WATERLEVEL        0x3F   //Bitmask for Waterlevel bits in register JREG_WATERLEVEL.  
 
//Control register                 (0x0C) 
#define     MASK_RXBITS            0x07   //Bitmask for RxLast bits in register JREG_CONTROL.  
 
//Mode register                    (0x11) 
#define     MASK_CRCPRESET         0x03   //Bitmask for CRCPreset bits in register JREG_MODE.  
 
//TxMode register                  (0x12, 0x13) 
#define     MASK_SPEED             0x70   //Bitmask for Tx/RxSpeed bits in register JREG_TXMODE and JREG_RXMODE.  
 
//TxSel register                   (0x16)  
#define     MASK_DRIVERSEL         0x30   //Bitmask for DriverSel bits in register JREG_TXSEL.  
#define     MASK_SIGOUTSEL         0x0F   //Bitmask for SigoutSel bits in register JREG_TXSEL.   
 
//RxSel register                   (0x17) 
#define     MASK_UARTSEL           0xC0   //Bitmask for UartSel bits in register JREG_RXSEL.  
#define     MASK_RXWAIT            0x3F   //Bitmask for RxWait bits in register JREG_RXSEL.  
 
//RxThreshold register             (0x18) 
#define     MASK_MINLEVEL          0xF0   //Bitmask for MinLevel bits in register JREG_RXTHRESHOLD.  
#define     MASK_COLLEVEL          0x07   //Bitmask for CollLevel bits in register JREG_RXTHRESHOLD.  
 
//Demod register                   (0x19) 
#define     MASK_ADDIQ             0xC0   //Bitmask for ADDIQ bits in register JREG_DEMOD.  
#define     MASK_TAURCV            0x0C   //Bitmask for TauRcv bits in register JREG_DEMOD.   
#define     MASK_TAUSYNC           0x03   //Bitmask for TauSync bits in register JREG_DEMOD.   
 
//RFCfg register                   (0x26) 
#define     MASK_RXGAIN            0x70   //Bitmask for RxGain bits in register JREG_RFCFG.  
 
//GsN register                     (0x27)  
#define     MASK_CWGSN             0xF0   //Bitmask for CWGsN bits in register JREG_GSN.  
#define     MASK_MODGSN            0x0F   //Bitmask for ModGsN bits in register JREG_GSN.   
 
//CWGsP register                   (0x28) 
#define     MASK_CWGSP             0x3F   //Bitmask for CWGsP bits in register JREG_CWGSP.   
 
//ModGsP register                  (0x29)  
#define     MASK_MODGSP            0x3F   //Bitmask for ModGsP bits in register JREG_MODGSP.   
 
//TMode register             (0x2A) 
#define     MASK_TGATED            0x60   //Bitmask for TGated bits in register JREG_TMODE.  
#define     MASK_TPRESCALER_HI     0x0F   //Bitmask for TPrescalerHi bits in register JREG_TMODE.  
//////////////////////////////////////////////////////////////////////////////////////////////////                     

#define T_FIFO_LEVEL             16    //the FIFO warning byte threshold
#define PICC_TXPOOL_SIZE          1024   //tx buffer size for frame exchange
#define PICC_RXPOOL_SIZE          1024   //rx buffer size for frame exchange

enum PICC_STATUS{S_IDLE=0,S_READY,S_ACTIVE,S_PROTOCOL,S_HALT};

typedef struct
{
    uchar  status;
    uchar  max_delay_index;
    uchar  cid_enable;
    uchar  nad_enable;
    ushort max_frame_size;
    uchar  max_buffer_index;//type B only
    uchar  blk_no;
	uchar  card_type;
	uchar  delay_multiple;
	uchar  serial_len;
	uchar  serial_no[10];
	
	uchar  FSC;
	uchar bitrate_up;
	uchar bitrate_down;
	uchar T0;
	uchar TA;
	uchar TB;
	uchar TC;
	
//A
	uchar ATQA[2];

	uchar ATS[1+4+15];


	
//B

	//ATQB
		uchar ATQB[13];
		uchar PUPI[4];
		uchar proto_info[4];


		uchar WTX;

		uchar SFGT_need;
	
}PICC_INFO;

typedef struct
{
     uchar  open;
}PCD_INFO;




// P I C C - C O M M A N D S
// commands which are handled by the tag
// Each tag command is written to the reader IC and transfered via RF
#define PICC_REQSTD        0x26              // request idle
#define PICC_REQALL        0x52              // request all
#define PICC_ANTICOLL1     0x93              // anticollision level 1
#define PICC_ANTICOLL2     0x95              // anticollision level 2
#define PICC_ANTICOLL3     0x97              // anticollision level 3
#define PICC_SELECT1       0x93              // select level 1
#define PICC_SELECT2       0x95              // select level 2
#define PICC_SELECT3       0x97              // select level 3
#define PICC_AUTHENT1A     0x60              // authentication step 1
#define PICC_AUTHENT1B     0x61              // authentication step 2
#define PICC_READ          0x30              // read block
#define PICC_WRITE         0xA0              // write block
#define PICC_DECREMENT     0xC0              // decrement value
#define PICC_INCREMENT     0xC1              // increment value
#define PICC_RESTORE       0xC2              // restore command code
#define PICC_TRANSFER      0xB0              // transfer command code
#define PICC_HALT          0x50              // halt

#define PICC_RATS          0xe0              //Request for Answer To Select
#define PICC_DESELECT      0xC2              // restore command code

// T Y P E  B  -  C O M M A N D S 
// commands which are handled by the tag
// Each tag command is written to the reader IC and transfered via RF
#define PICC_REQBSTD        0x00             // request normal
#define PICC_REQBWUP        0x08             // request wakeup

// ISO14443 
#define TYPEA               0x00
#define TYPEB               0x08 




#define PICC_OK                (0)
#define PICC_ChipIDErr         (-3500)
#define PICC_OpenErr           (-3501)
#define PICC_NotOpen           (-3502)
#define PICC_ParameterErr      (-3503)  //参数错误
#define PICC_TxTimerOut        (-3504)
#define PICC_RxTimerOut        (-3505)
#define PICC_RxDataOver        (-3506)
#define PICC_TypeAColl         (-3507)
#define PICC_FifoOver          (-3508)
#define PICC_CRCErr            (-3509)
#define PICC_SOFErr            (-3510)
#define PICC_ParityErr         (-3511)
#define PICC_KeyFormatErr      (-3512)

#define PICC_RequestErr        (-3513)
#define PICC_AntiCollErr       (-3514)
#define PICC_UidCRCErr         (-3515)
#define PICC_SelectErr         (-3516)
#define PICC_RatsErr           (-3517)
#define PICC_AttribErr         (-3518)
#define PICC_HaltErr           (-3519)
#define PICC_OperateErr        (-3520)
#define PICC_WriteBlockErr     (-3521)
#define PICC_ReadBlockErr      (-3522)
#define PICC_AuthErr           (-3523)
#define PICC_ApduErr           (-3524)
#define PICC_HaveCard          (-3525)   //有卡
#define PICC_Collision         (-3526) //多卡
#define PICC_CardTyepErr       (-3527) //A 卡发送RATS
#define PICC_CardStateErr      (-3528)

#define PICC_RxTimerOut2      (-3529)
#define PICC_RxErr           (-3530)
#define PICC_RxOverFlow           (-3531)


#define PICC_ProtocolErr          (-3532)

#define PICC_FastOut          (-3533)

#define PICC_Fsderror          (-3533)
#define PICC_CRCErr2            (-3534)

#define PICC_KeyCancel			(-3535)


#define PICC_ApduErr1           (-3540)
#define PICC_ApduErr2          (-3541)
#define PICC_ApduErr3          (-3542)
#define PICC_ApduErr4           (-3543)
#define PICC_ApduErr5           (-3544)
#define PICC_ApduErr6          (-3545)
#define PICC_ApduErr7           (-3546)
#define PICC_ApduErr8           (-3547)
#define PICC_ApduErr9           (-3548)
#define PICC_ApduErr10           (-3549)
#define PICC_ApduErr11           (-3550)
#define PICC_ApduErr12           (-3551)
#define PICC_ApduErr13           (-3552)
#define PICC_ApduErr14           (-3553)
#define PICC_ApduErr15           (-3554)
#define PICC_ApduErr16           (-3555)
#define PICC_ApduErr17           (-3556)
#define PICC_ApduErr18           (-3557)
#define PICC_ApduErr19           (-3558)
#define PICC_ApduErr20           (-3559)
#define PICC_ApduErr21           (-3560)
#define PICC_ApduErr22           (-3561)
#define PICC_ApduErr23           (-3562)
#define PICC_ApduErr24           (-3563)





#define PICC_NotImplement           (-3570)








int  s_PiccInit(void);


//#define __FUNCTION__ ""


#if 0

#define printf(args...) s_UartPrint(COM_DEBUG, ##args)

#else
//#define s_UartPrint printf

#define s_printf(...)  
//#undef s_UartPrint
//#define s_UartPrint(...)

#endif



#if 0

#define printf2(args...) s_UartPrint(COM_DEBUG, ##args)

#else
//#define s_UartPrint printf

#define printf2(...)
//#undef s_UartPrint
//#define s_UartPrint(...)

#endif

#if 0

#define printfXX(args...) s_UartPrint(COM_DEBUG, ##args)
#define printfx(args...) s_UartPrint(COM_DEBUG, ##args)

#else
//#define s_UartPrint printf
#define printfx  
#define printfXX 
//#undef s_UartPrint
//#define s_UartPrint(...)

#endif



#define DEF_FIFO_LEVEL   (0x1A)

#define     DEF_FIFO_LENGTH       64  // default FIFO length




#define FSGImax 8

#define FSDImin 8
#define FSCImin 2

//#define FSDImin 8
//#define FSCImax 8


#define FSDmin 256
#define FSCmin 32


#define FWTactivation	71680
#define FWTatqb	7680


#define Xfwt 384	//384*2^fwi

#define Xsfgt 384	//384*2^sfgi

#define WaterLevel 10


#define Trecovery 1280

#endif















