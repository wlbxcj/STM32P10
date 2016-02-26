#ifndef _MYFRE_H_
#define	_MYFRE_H_


#define RET_RF_OK                  0x00
#define RET_RF_ERR_PARAM           0x01   
#define RET_RF_ERR_NO_OPEN         0x02


#define RET_RF_ERR_NOT_ACT         0x13
#define RET_RF_ERR_MULTI_CARD      0x14
#define RET_RF_ERR_TIMEOUT         0x15
#define RET_RF_ERR_PROTOCOL        0x16

#define RET_RF_ERR_TRANSMIT        0x17
#define RET_RF_ERR_AUTH            0x18
#define RET_RF_ERR_NO_AUTH         0x19
#define RET_RF_ERR_VAL             0x1A
#define RET_RF_ERR_CARD_EXIST      0x1B
#define RET_RF_ERR_STATUS          0x1C

#define RET_RF_ERR_OVERFLOW        0x1E
#define RET_RF_ERR_FAILED          0x1F

#define RET_RF_ERR_COLLERR         0x20
#define RET_RF_ERR_FIFO            0x21
#define RET_RF_ERR_CRC             0x22
#define RET_RF_ERR_FRAMING         0x23
#define RET_RF_ERR_PARITY          0x24

#define RET_RF_ERR_DES_VAL         0x25
#define RET_RF_ERR_NOT_ALLOWED     0x26
//end

enum DETECT_ERROR{E_INVALID_PARA=1,E_NO_POWER,E_NOT_DETECTED,E_MULTI_CARDS,E_ACT_FAILED,E_PROTOCOL};

enum ISOCMD_ERROR{EE_INVALID_PARA=1,EE_NO_POWER,EE_NOT_DETECTED,EE_TX,EE_PROTOCOL};

//API define
uchar PiccRemove_EMV(uchar cid, ushort ul100Ms);
uchar PiccRemove_PayPass(uchar cid, ushort ul100Ms);
uchar PiccCmdExchange(uint uiSendLen, uchar* paucInData, uint* puiRecLen, uchar* paucOutData);

extern uchar PiccOpen(void);
extern uchar M1Authority(uchar Type,uchar BlkNo,uchar *Pwd,uchar *SerialNo);
extern uchar M1Operate(uchar Type, uchar BlkNo, uchar *Value, uchar UpdateBlkNo);
extern uchar M1ReadBlock(uchar BlkNo,uchar *BlkValue);
extern uchar M1WriteBlock(uchar BlkNo,uchar *BlkValue);
extern void PiccClose(void);
extern uchar PiccCmdExchange(uint uiSendLen, uchar* paucInData, uint* puiRecLen, uchar* paucOutData);
extern uchar PiccDetect(uchar ucMode,uchar *CardType,uchar *SerialInfo,uchar *CID,uchar *Other);
extern uchar PiccIsoCommand(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv);
extern uchar PiccOpen(void);
extern uchar PiccRemove(uchar mode, uchar cid);
extern uchar PiccSetup(uchar ucMode, PICC_PARA *picc_para);
#endif




