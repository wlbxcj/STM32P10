#ifndef _MYFRE_H_
#define	_MYFRE_H_

/**********************************************************************

                         参数定义
                          
***********************************************************************/
// ISO14443 
#define RF_TYPEA				0x00
#define RF_TYPEB				0x08
#define RF_TYPEF        		0x0F

#define RF_TYPE_S50          0
#define RF_TYPE_S70          1
#define RF_TYPE_PRO          2
#define RF_TYPE_PRO_S50      3
#define RF_TYPE_PRO_S70      4
#define RF_TYPE_B            5

// 函数返回码定义
#define RET_RF_OK                  0x00    // 操作成功
#define RET_RF_ERR_PARAM           0x01    // 0x09 参数错误
#define RET_RF_ERR_NO_OPEN         0x02    // 0x08 射频卡模块未开启

#define RET_RF_ERR_NOT_ACT         0x13    // 0x0B 卡片未激活 // added by liuxl 20070925
#define RET_RF_ERR_MULTI_CARD      0x14    // 多卡冲突
#define RET_RF_ERR_TIMEOUT         0x15    // 超时无响应
#define RET_RF_ERR_PROTOCOL        0x16    // 协议错误

#define RET_RF_ERR_TRANSMIT        0x17    // 通信传输错误
#define RET_RF_ERR_AUTH            0x18    // 验证失败
#define RET_RF_ERR_NO_AUTH         0x19    // 扇区未认证
#define RET_RF_ERR_VAL             0x1A    // 数值块数据格式有误
#define RET_RF_ERR_CARD_EXIST      0x1B    // 卡片仍在感应区内
#define RET_RF_ERR_STATUS          0x1C    // 卡片未进入协议态时调用APDU命令,或已进入协议态时调用M1命令// added by liuxl 20070925
//#define RET_RF_ERR_NO_MODULE       0x1D    // 无RF模块
//end
//end
// p80 中的定义
enum DETECT_ERROR{E_INVALID_PARA=1,E_NO_POWER,E_NOT_DETECTED,E_MULTI_CARDS,E_ACT_FAILED,E_PROTOCOL};

enum ISOCMD_ERROR{EE_INVALID_PARA=1,EE_NO_POWER,EE_NOT_DETECTED,EE_TX,EE_PROTOCOL};


// 定时常量定义
#define  RF_FWTMIN       9           // 用于WUPA、ANTICOLLISION和SELECT命令的等待响应时间
                                     // 实际上应该是9.2ETU
#define  RF_FWTRATS      560         // 用于等待RATS帧
#define  RF_FWTDESELECT  560         // 用于等待DESELECT响应帧
#define  RF_FWTWUPB      60          // 用于等待WUPB响应帧
#define  RF_MAXFWT       8960        // FWT最大时间为32 * 2^8 + 3 * 2^8 = 8960ETU
#define  RF_DEFAULT      560         // 缺省帧等待时间为560ETU
#define  RF_TPDEALY     (uint)500    // 切换调制方式时延时500个ETU
#define  RF_TRESET      (uint)1000   // 复位PICC时需延时1000ETU
#define  RF_FDTMIN      (uint)70     // 用于保证PCD发送下一条命令帧前的500us延时

// 内部常量定义
#define RECEIVE_SBLOCK       0x31
#define RECEIVE_IBLOCK       0x32
#define SEND_NEXTBLOCK       0x33
#define RESEND_IBLOCK        0x34
#define RESEND_ACK           0x35

#define IBLOCK_UNCHAINING    0x00
#define IBLOCK_CHAINING      0x01
#define IBLOCK_SEND          0x00
#define IBLOCK_RECEIVE       0x01

#define RF_BUFFER_LEN        272  // liuxl 接收缓冲区最大长度

#define RF_RATE_212_FELICA 0 //default = 0;
#define RF_RATE_424_FELICA 1
#define RF_POL_0_FELICA    0 // default = 0;
#define RF_POL_1_FELICA    1


extern uchar PiccOpen(void);
extern uchar PiccDetect(uchar Mode,uchar *CardType,uchar *SerialInfo,uchar *CID,uchar *Other);
extern uchar PiccIsoCommand(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv);
extern void PiccClose(void);

#endif

