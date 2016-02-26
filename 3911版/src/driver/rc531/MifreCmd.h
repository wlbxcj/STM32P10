#ifndef _MYFRECMD_H_
#define	_MYFRECMD_H_


#include "mifre_tmdef.h"


/*cmd define*/
#define RF_CMD_IDLE           0x00 // No action: cancel current command 
                                // or home state, respectively
#define RF_CMD_WRITEE2        0x01 // Get data from FIFO and write it to the E2PROM
#define RF_CMD_READE2         0x03 // Read data from E2PROM and put it into the FIFO
#define RF_CMD_LOADCONFIG     0x07 // Read data from E2PROM and initialise the registers
#define RF_CMD_LOADKEYE2      0x0B 
#define RF_CMD_AUTHENT1       0x0C
#define RF_CMD_CALCCRC        0x12 
#define RF_CMD_AUTHENT2       0x14 
#define RF_CMD_RECEIVE        0x16 
#define RF_CMD_LOADKEY        0x19 
#define RF_CMD_TRANSMIT       0x1A
#define RF_CMD_TRANSCEIVE     0x1E 
#define RF_CMD_RESETPHASE     0x3F // Runs the Reset- and Initialisation Phase
                                // Remark: This command can not be activated by 
                                // software, but only by a Power-On or 
                                // Hard Reset 


/*picc cmd*/
#define PICC_REQA           0x26        // REQA
#define PICC_WUPA           0x52        // request all
#define PICC_ANTICOLL1      0x93        // anticollision level 1
#define PICC_ANTICOLL2      0x95        // anticollision level 2
#define PICC_ANTICOLL3      0x97        // anticollision level 3
#define PICC_AUTHENTA       0x60        // authentication KeyA
#define PICC_AUTHENTB       0x61        // authentication KeyB
#define PICC_READ           0x30        // read block
#define PICC_WRITE          0xA0        // write block
#define PICC_DECREMENT      0xC0        // decrement value
#define PICC_INCREMENT      0xC1        // increment value
#define PICC_RESTORE        0xC2        // restore command code
#define PICC_TRANSFER       0xB0        // transfer command code
#define PICC_HALT           0x50        // halt
#define PICC_DESELECT       0xCA        // DESELECT command code
// typeB proprietary Command
#define PICC_APF            0x05        // Prefix Byte
#define PICC_WUPB           0x08        // request wakeup
#define PICC_ATTRIB         0x1D        // AttribÃüÁîÂë
#define PICC_REQB           0x00        // request ALL

//PICC status
#define PICC_STATUS_WAKEN   0x01
#define PICC_STATUS_ACTIV   0x02
#define PICC_STATUS_REMOVE  0x03


#define RF_TYPEA				0x00
#define RF_TYPEB				0x08

//A type
#define RF_TYPE_S50          0
#define RF_TYPE_S70          1
#define RF_TYPE_PRO          2
#define RF_TYPE_PRO_S50      3
#define RF_TYPE_PRO_S70      4

//B type
#define RF_TYPE_B            5

#define  RF_FWTMIN       9           // WUPA,ANTICOLLISION,SELECT wait time
                                     // act is 9.2ETU
#define  RF_FWTRATS      560         // wait for RATS
#define  RF_FWTDESELECT  560         // wait for DESELECT
#define  RF_FWTWUPB      60          // wait for WUPB
#define  RF_MAXFWT       8960        // FWT Max is 32 * 2^8 + 3 * 2^8 = 8960ETU
#define  RF_DEFAULT      560         // default is 560ETU
#define  RF_TPDEALY     (uint)500    
#define  RF_TRESET      (uint)1000   
#define  RF_FDTMIN      (uint)70

//rec bff Max len
#define RF_BUFFER_LEN        272 

//EMV&PayPass Test define
#define REQA_CMD      (char*)"REQA-> "
#define WUPA_CMD      (char*)"WUPA-> "
#define ANTICOL_CMD   (char*)"ANTI-> "
#define SELECT_CMD    (char*)"SLT -> "
#define RATS_CMD      (char*)"RATS-> "
#define WUPB_CMD      (char*)"WUPB-> "
#define REQB_CMD      (char*)"REQB-> "
#define ATTRIB_CMD    (char*)"ATTR-> "

#define HALTA_CMD     (char*)"HLTA-> " 
#define PICC_RSP      (char*)"RSPD<- " 
#define CPDU_CMD      (char*)"CPDU-> "

#define CARRIER_OFF   (char*)"RFOF-> \n"   
#define CARRIER_ON    (char*)"RFON-> \n"

//function define 

void s_RF_vInitWorkInfo(void);
void s_vRFSelectType(uchar ucRFType);
uchar s_RF_ucCardTypeCheck(uchar *pCardType);
uchar s_RF_ucWUPA(uchar *pucResp);
uchar s_RF_ucAnticoll(uchar ucSEL, uchar* pucUID);
uchar s_RF_ucSelect(uchar ucSEL, uchar* pucUID, uchar* pucSAK);
uchar s_RF_ucGetUID(void);
uchar s_RF_ucActPro(uchar* pucOutLen, uchar* pucATSData);
uchar s_RF_ucRATS(uchar* pucOutLen, uchar* pucATSData);
void s_RF_vHALTA(void);
uchar  s_RF_ucWUPB(uchar* pucOutLen, uchar* pucOut);
uchar s_RF_ucAttrib(uchar* pucResp);
uchar s_RF_ucHALTB(uchar* pucResp);
uchar s_RF_ucActTypeB(uchar* pucOutLen, uchar* pucOut);
uchar s_RF_ucPOLL(uchar* pucMifType);
uchar  s_RF_ucIfCardExit(void);
uchar s_RF_ucProcess(uint uiSendLen, uint* puiRecLen);
uchar s_RF_ucDeselect(void);
uchar  s_RF_ucMifRBlock(uint* puiRecLen);
uchar s_RF_ucMifSBlock( uint* puiRecLen);
uchar s_RF_ucExchange(uint uiSendLen, uchar* pucInData, uint* puiRecLen, uchar* pucOutData);
void s_RF_vGetDetectAInfo(uchar *SerialInfo, uchar *Other);
void s_RFDelay10ETUs(ulong nETU);
void s_RFSetTimer(ulong nETU);
void s_RFOpenCarrier(void);
void s_RFCloseCarrier(void);
void s_RFResetCarrier(void);
void s_RFIsr(void);
uchar s_RFExchangeCmd(uchar ucCmd);

// 20080907
uchar s_RF_ucREQA(uchar *pucResp);
uchar s_RF_ucCardStatusCheck(uchar ucInProtocolNeed);

// for emv L1
uchar s_RF_ucPOLL_EMV(uchar* pucMifType);
uchar  s_RF_ucRemoval(void);

// 20081027 
uchar s_RF_ucPOLL_EMV_Step(uchar* pucType);
uchar s_RF_Activate(uchar ucType, uchar *pucMifType);

// 20081215 for paypass 1.1
uchar  s_RF_ucREQB(uchar* pucOutLen, uchar* pucOut);

void s_vRFSetBitMask (uchar ucRegAddr, uchar ucBitMask);
void s_vRFClrBitMask(uchar ucRegAddr, uchar ucBitMask);
void s_vRFFlushFIFO(void);

//mifare one driver
uchar s_M1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo);
uchar s_M1ReadBlock(uchar BlkNo,uchar *BlkValue);
uchar s_M1WriteBlock(uchar BlkNo,uchar *BlkValue);
uchar s_M1Operate(uchar Type, uchar BlkNo, uchar *Value, uchar UpdateBlkNo);
#endif

