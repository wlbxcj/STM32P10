#ifndef _MYFRECMD_H_
#define	_MYFRECMD_H_

////////////////////////////////////////////////////////////////////////////////////

//                 PN512 √¸¡Ó¬Î∂®“Â                                 

///////////////////////////////////////////////////////////////////////////////////
#define PN512_IDLE_CMD          0x00   // No action; cancels current command execution
#define PN512_CONFIG_CMD        0x01   // Configures the PN512 for FeliCa, Mifare and NFCIP-1 communication
#define PN512_GRAMID_CMD        0x02   // Generates a 10 byte random ID number
#define PN512_CALCRC_CMD        0x03   // Activates the CRC co-processor or performs a selftest
#define PN512_TRANSMIT_CMD      0x04   // Transmits data from the FIFO buffer
#define PN512_NOCMDCHANGE_CMD   0x07   // No command change.this command can be used to modify different bits
                                       // in the command register without touching the command .E.G. Power down bit
#define PN512_RECEIVE_CMD       0x08   // Activates the receiver circuitry
#define PN512_TRANSCEIVE_CMD    0x0C   // Transmits data from FIFO buffer and automatically activates the receiver after transmission is finished
#define PN512_AUTOCOLL_CMD      0x0D   // Handles FeliCa polling(Card Operation mode only) and Mifare anticollision(Card Operation mode only)
#define PN512_MFAuthent_CMD     0x0E   // Performs the Mifare standard authentication in Mifare Reader/Writer mode only
#define PN512_SOFTRESET_CMD     0x0F   // Resets the PN512


/**********************************************************************

                         PICC√¸¡ÓºØ∫œ

***********************************************************************/
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
// T Y P E  B  -  C O M M A N D S
// commands which are handled by the tag
// Each tag command is written to the reader IC and transfered via RF
#define PICC_APF            0x05        // Prefix Byte
#define PICC_WUPB           0x08        // request wakeup
#define PICC_ATTRIB         0x1D        // Attrib√¸¡Ó¬Î

#define PICC_STATUS_WAKEN   0x01
#define PICC_STATUS_ACTIV   0x02
#define PICC_STATUS_REMOVE  0x03


void PN512_s_RF_vInitWorkInfo(void);
void PN512_s_vRFSelectType(uchar ucRFType);
uchar PN512_s_RF_ucCardTypeCheck(uchar *pCardType);
uchar PN512_s_RF_ucWUPA(uchar *pucResp);
uchar PN512_s_RF_ucAnticoll(uchar ucSEL, uchar* pucUID);
uchar PN512_s_RF_ucSelect(uchar ucSEL, uchar* pucUID, uchar* pucSAK);
uchar PN512_s_RF_ucGetUID(void);
uchar PN512_s_RF_ucActPro(uchar* pucOutLen, uchar* pucATSData);
uchar PN512_s_RF_ucRATS(uchar* pucOutLen, uchar* pucATSData);
void PN512_s_RF_vHALTA(void);
uchar  PN512_s_RF_ucWUPB(uchar* pucOutLen, uchar* pucOut);
uchar  PN512_s_RF_ucREQB(uchar* pucOutLen, uchar* pucOut);
uchar PN512_s_RF_ucAttrib(uchar* pucResp);
uchar PN512_s_RF_ucHALTB(uchar* pucResp);
uchar PN512_s_RF_ucActTypeB(uchar* pucOutLen, uchar* pucOut);
uchar PN512_s_RF_ucPOLL(uchar* pucMifType);
uchar  PN512_s_RF_ucIfCardExit(void);
uchar PN512_s_RF_ucProcess(uint uiSendLen, uint* puiRecLen);
uchar PN512_s_RF_ucDeselect(void);
uchar  PN512_s_RF_ucMifRBlock(uint* puiRecLen);
uchar PN512_s_RF_ucMifSBlock( uint* puiRecLen);
uchar PN512_s_RF_ucExchange(uint uiSendLen, uchar* pucInData, uint* puiRecLen, uchar* pucOutData);
void PN512_s_RF_vGetDetectAInfo(uchar *SerialInfo, uchar *Other);
void PN512_s_RFDelay10ETUs(ulong nETU);
void PN512_s_RFSetTimer(ulong nETU);
void PN512_s_RFOpenCarrier(void);
void PN512_s_RFCloseCarrier(void);
void PN512_s_RFResetCarrier(void);

#ifdef FOR_LINUX_SYS
	irqreturn_t PN512_s_RFIsr(int irq, void *dev_id, struct pt_regs *regs);
#else
	void PN512_s_RFIsr(void);
#endif

uchar PN512_s_RFExchangeCmd(uchar ucCmd);

void PN512_s_vRFSetBitMask (uchar ucRegAddr, uchar ucBitMask);
void PN512_s_vRFClrBitMask(uchar ucRegAddr, uchar ucBitMask);
void PN512_s_vRFFlushFIFO(void);

uchar PN512_felica_poll(uchar rate,uchar polarity,uchar *cmd,uint slen,uchar *Resp,ulong *rlen);
uchar PN512_s_RF_ucExchange_Felica(uint uiCmdLen, uchar* paucCmd, uint* puiRspLen, uchar* paucResp);
uchar PN512_s_RF_ucInit_Felica(uchar ucRate, uchar ucPol);
#endif

