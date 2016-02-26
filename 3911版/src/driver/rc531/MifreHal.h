
#ifndef _MYFREHAL_H_
#define	_MYFREHAL_H_

/***********************************
   SPI Defines
***********************************/

#define SPICS                    7//SSEL

#define SYSCLK            12000000L
#define RF_SPI_BAUDRATE   3000000L
#define SPIBAUDCKR        ((0.5*SYSCLK)/(RF_SPI_BAUDRATE)-1)//spi baud rate


// page 0:
#define     RF_REG_PAGE                  0x00    
#define     RF_REG_COMMAND               0x01    
#define     RF_REG_FIFODATA              0x02    
#define     RF_REG_PRIMARYSTATUS         0x03    
#define     RF_REG_FIFOLENGTH            0x04    
#define     RF_REG_SECONDARYSTATUS       0x05    
#define     RF_REG_INTERRUPTEN           0x06    
#define     RF_REG_INTERRUPTRQ           0x07    

// page 1:
#define     RF_REG_CONTROL               0x09    
#define     RF_REG_ERRORFLAG             0x0A
#define     RF_REG_COLLPOS               0x0B
#define     RF_REG_TIMERVALUE            0x0C
#define     RF_REG_CRCRESULTLSB          0x0D
#define     RF_REG_CRCRESULTMSB          0x0E
#define     RF_REG_BITFRAMING            0x0F

// page 2:
#define     RF_REG_TXCONTROL             0x11
#define     RF_REG_CWCONDUCTANCE         0x12
#define     RF_REG_MODCONDUCTANCE        0x13
#define     RF_REG_CODERCONTROL          0x14   
#define     RF_REG_MODWIDTH              0x15
#define     RF_REG_MODWIDTHSOF           0x16
#define     RF_REG_TYPEBFRAMING          0x17

// page 3:
#define     RF_REG_RXCONTROL1            0x19
#define     RF_REG_DECODERCONTROL        0x1A
#define     RF_REG_BITPHASE              0x1B
#define     RF_REG_RXTHRESHOLD           0x1C
#define     RF_REG_BPSKDEMCONTROL        0x1D
#define     RF_REG_RXCONTROL2            0x1E
#define     RF_REG_CLOCKQCONTROL         0x1F

// page 4:
#define     RF_REG_RXWAIT                0x21
#define     RF_REG_CHANNELREDUNDANCY     0x22
#define     RF_REG_CRCPRESETLSB          0x23
#define     RF_REG_CRCPRESETMSB          0x24
#define     RF_REG_TIMESLOTPERIOD        0x25
#define     RF_REG_MFOUTSELECT           0x26
#define     RF_REG_RFU27                 0x27

// page 5:
#define     RF_REG_FIFOLEVEL             0x29
#define     RF_REG_TIMERCLOCK            0x2A
#define     RF_REG_TIMERCONTROL          0x2B
#define     RF_REG_TIMERRELOAD           0x2C
#define     RF_REG_IRQPINCONFIG          0x2D
#define     RF_REG_RFU2E                 0x2E
#define     RF_REG_RFU2F                 0x2F

// page 6:RFU
#define     RF_REG_RFU31                 0x31
#define     RF_REG_RFU32                 0x32
#define     RF_REG_RFU33                 0x33
#define     RF_REG_RFU34                 0x34
#define     RF_REG_RFU35                 0x35
#define     RF_REG_RFU36                 0x36
#define     RF_REG_RFU37                 0x37

// page 7:
#define     RF_REG_RFU39                 0x39
#define     RF_REG_TESTANASELECT        0x3A   
#define     RF_REG_RFU3B                 0x3B
#define     RF_REG_RFU3C                 0x3C
#define     RF_REG_TESTCONFIGURATION    0x3C
#define     RF_REG_TESTDIGISELECT        0x3D   
#define     RF_REG_RFU3E                 0x3E
#define     RF_REG_TESTDIGIACCESS        0x3F






/*Write to on board spi device*/
//funtion for MifreCmd.c
extern signed int s_vRFReadReg(unsigned int ulLen, unsigned char ucRegAddr, unsigned char* pucData);
extern signed int s_vRFWriteReg(unsigned int ulLen, unsigned char ucRegAddr, unsigned char* pucData);
extern void sHal_EnCpuInt(void);
extern void sHal_MaskCpuInt(void);

//funtion for Mifre.c
extern int s_RF_Init(void);
extern uchar sHal_Module_Disable(void);

//function for internal
extern void s_InitRFGPIO(void);
extern void s_vRFReset(void);
extern void s_vRFSwitchNCS(unsigned char chHigh);
extern unsigned char sPcdInit(void);
extern void s_InitRFSPI(unsigned char spibaudckr);
extern void s_RF_SPIProc(void);

#endif
