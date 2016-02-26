/// @file   Taxbox.h
/// This is the Head file for module @ref TaxBox and @ref UART_Command
/// @author Travis Tu
/// @date   2006-Sep-1st


#ifndef _TAXBOX_H_
#define _TAXBOX_H_

#define BACKUPDATA _Pragma("location=\"BACKUP\"") __no_init

/**
 * @addtogroup TaxBox
 * @{
 */


/**
 * @defgroup TaxBox_CONFIGURATION
 * @ingroup TaxBox
 * @{
 */
#undef  NULL
#define NULL 0
//#define NOCSCheck             ///<Do not Check Sum on incoming Uart Command
//#define RUNINRAM              ///<All the application will be run in RAM
//#define USB_MOD                 ///<Use the USB communication
#define TaxICCardNOPPS          ///<No PPS during IC card ATR
#define WITHLASTMEM             ///<Restart the TAX Box With Last Memory
/**@}*/


#include "stm32f10x_type.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_map.h"
#include "stm32f10x_lib.h"
#include "I2CRTC.h"
#include "UARTSC.h"
#include "SPIFlash.h"
#include "Mem.h"
#include "comm.h"
#include "Toolkit.h"
#include "Data.h"
#include "ICCmd.h"
#include "Comm.h"
#include "USBCom.h"
#include "Buzzer.h"
#include "DataFlash.h"
#include "USBHost.h"
#include "I2C.h"

///Structure of flags bit field set
typedef struct
{
    bool bRspWanted;         ///<需要response
    bool bPowerLost;         ///<indicate the power lose
    bool bDclMngDataNoBack;  ///<申报管理数据未回传   TRUE:已申报未完税；FALSE:无申报数据
    bool bBoxIsInitialized;  ///<引进被初始化了
    bool bDispenseAbortFlag; ///<发票分发异常中断标志位
    bool bDeclareAbortFlag;  ///<申报异常中断标志位
    bool bPayTaxAbortFlag;   ///<完税异常中断标志位
}AppFlags_t;

///Structure for element of UART Incomming Commands Table
typedef struct
{
    u8 InstructByte;            ///<the instruction Byte for this Uart Command
    void (*Cmd)(u16, u8* );     ///<this instruction command Handler
}UARTCmd_t;
/**
 * @addtogroup TaxBox_PARAMETER
 * @ingroup TaxBox
 * @{
 */
extern AppFlags_t AppFlag;      ///<bit fields for application flags
extern u16 UARTRspParamLength;  ///<Parameter's length which is pedding on Response buffer
/**@}*/


/**
 * @defgroup TaxBox_ERROR_CODE
 * Define the Tax Box error code in response
 * @ingroup TaxBox
 * @{
 */
#define UART_COMMAND_SUCCESS                                    0x00
#define UART_COMMAND_FISCAL_ICC_POWERON_ERR                     0x01    ///<税控卡上电错误
#define UART_COMMAND_USER_ICC_POWERON_ERR                       0x02    ///<用户卡上电错误
#define UART_COMMAND_MANAGEMENT_ICC_POWERON_ERR                 0x03    ///<管理卡上电错误
#define UART_COMMAND_NO_USER_ICC                                0x04    ///<没有用户卡
#define UART_COMMAND_NO_FISCAL_ICC                              0x05    ///<没有税控卡
#define UART_COMMAND_NO_FISCAL_MANAGEMENT_ICC                   0x06    ///<没有管理卡
#define UART_COMMAND_READ_FISCAL_ICC_ERROR                      0x07    ///<读取税控卡错误
#define UART_COMMAND_READ_USER_ICC_ERROR                        0x08    ///<读取用户卡错误
#define UART_COMMAND_READ_FISCAL_MANAGEMENT_ICC_ERROR           0x09    ///<读取管理卡错误
#define UART_COMMAND_WRITE_FISCAL_ICC_ERROR                     0x0A    ///<更新税控卡错误
#define UART_COMMAND_WRITE_USER_ICC_ERROR                       0x0B    ///<更新用户卡错误
#define UART_COMMAND_WRITE_FISCAL_MANAGEMENT_ICC_ERROR          0x0C    ///<更新管理卡错误
#define UART_COMMAND_ILLEGAL_FISCAL_ICC                         0x0D    ///<非法税控卡
#define UART_COMMAND_ILLEGAL_USER_ICC                           0x0E    ///<非法用户卡
#define UART_COMMAND_ILLEGAL_FISCAL_MANAGEMENT_ICC              0x0F    ///<非法管理卡
#define UART_COMMAND_TAX_BOX_NOT_INITIALIZED                    0x10    ///<税控盒还未初始化
#define UART_COMMAND_ISSUER_INVOICE_DEADLINE_EXCEEDED           0x11    ///<开票截至日已过期
#define UART_COMMAND_SINGLE_INVOICE_UP_LIMIT_EXCEEDED           0x12    ///<已超过单张发票开票金额限额
#define UART_COMMAND_ISSUE_INVOICE_TOTAL_UP_LIMIT_EXCEEDED      0x13    ///<开票总额超上限
#define UART_COMMAND_RETURN_INVOICE_TOTAL_UP_LIMIT_EXCEEDED     0x14    ///<退票总额超上限
#define UART_COMMAND_CARD_MACHINE_DEFFER                        0x15    ///<税控卡的注册号与税控器的注册号不一致
#define UART_COMMAND_USER_CARD_MAC_ERR                          0x16    ///<用户卡MAC错
#define UART_COMMAND_FISCAL_CARD_MAC_ERR                        0x17    ///<税控卡MAC错
#define UART_COMMAND_PIN_ERROR                                  0x18    ///<检查PIN失效
#define UART_COMMAND_ILLEGAL_TAX_SPECIES_TAX_DETAILS            0x19    ///<非法税种税目
#define UART_COMMAND_DECLARE_DATE_EXCEEDED_CURRENT              0x1A    ///<申报截止日期大于当前日期
#define UART_COMMAND_DECLARE_DATE_EXCEEDED_LAST                 0x1B    ///<申报截止日期小于上次申报日期
#define UART_COMMAND_USER_CARD_LOCKED                           0x1C    ///<用户卡锁死
#define UART_COMMAND_FISCAL_CARD_LOCKED                         0x1D    ///<税控卡锁死
#define UART_COMMAND_ALREADE_INIT                               0x1E    ///<税控器已经初始化
#define UART_COMMAND_CHECK_SUM_ERROR                            0x1F    ///<校验位不正确
#define UART_COMMAND_INSTRUCTION_TYPE_ERROR                     0x20    ///<命令类型错误
#define UART_COMMAND_HEAD_ERROR                                 0x21    ///<命令头错误
#define UART_COMMAND_COMMAND_LENGTH_ERROR                       0x22    ///<命令长度错误
#define UART_COMMAND_INVOICE_OVER                               0x23    ///<发票已用完
#define UART_COMMAND_INVOICE_MEMORY_ERR                         0x24    ///<发票存储器异常
#define UART_COMMAND_DATA_STORAGE_EXCEPTION                     0x25    ///<税控存储器异常
#define UART_COMMAND_DATE_OVERFLOW                              0x26    ///<时间、日期、数值溢出
#define UART_COMMAND_FISCAL_ICC_ALREADY_REGISTER                0x27    ///<税控卡已注册
#define UART_COMMAND_CARD_FULL_CHANGE                           0x28    ///<卡已满换卡操作
#define UART_COMMAND_UNUSERD_INVOICE_EMPTY                      0x29    ///<没有已分发但未使用的发票号
#define UART_COMMAND_NoMoreInvToDistribute                      0x2A    ///<用户卡中无发票分发
#define UART_COMMAND_ILLEGAL_SUB_ACCOUNT_NB                     0x2B    ///<非法分户编号
#define UART_COMMAND_NO_INVOICE_PRINT_INFO_FOUND                0x2C    ///<此发票号无相关数据
#define UART_COMMAND_INVOICE_HAS_BEEN_RETURNED                  0x2D    ///<此发票已作退票处理
#define UART_COMMAND_INVOICE_HAS_BEEN_DISCARDED                 0x2E    ///<此发票已作废票处理
#define UART_COMMAND_ISSUE_INVOICE_FAIL                         0x40    ///<开票失败
#define UART_COMMAND_NO_TAX_INDEXS                              0x41    ///<无税种税目信息
#define UART_COMMAND_DECLARE_DUTY_ERROR                         0x42    ///<申报失败
#define UART_COMMAND_FISCAL_MANAGEMENT_CARD_AUTHENTICATION_FAIL 0x43    ///<税务管理卡认证失败
#define UART_COMMAND_SINGLE_INVOICE_SCROLL_INFO_NOT_FOUND       0x44    ///<无相关发票卷使用信息
#define UART_COMMAND_NO_NEW_DECLARE                             0x45    ///<监控管理数据已回传
#define UART_COMMAND_MANAGEMENT_DATA_TYPE_DIFFERE               0x46    ///<换入核查卡类型不一致
#define UART_COMMAND_NO_LAST_DECLARE_DATA_YET                   0x47    ///<无查询申报数据
#define UART_COMMAND_INPUT_DATE_ERR                             0x48    ///<输入日期时间错误
#define UART_COMMAND_NO_DAILY_TRANSACTION_INFO_FOUND            0x49    ///<无相关日交易数据
#define UART_COMMAND_INPUT_INVOICE_FULL                         0x4A    ///<税控卡内有发票，无法录入
#define UART_COMMAND_DISCARD_NUMBER_ERR                         0x4B    ///<废票号码错误
#define UART_COMMAND_FISCAL_CARD_NO_EXP                         0x4C    ///<税控卡不在有效期内
#define UART_COMMAND_USER_CARD_NO_EXP                           0x4D    ///<用户卡不在有效期内
#define UART_COMMAND_INVOICE_ITEMS_NUM_EXCEEDED                 0x4E    ///<开票项目数超过最大限制
#define UART_COMMAND_ALREADY_DECLARE_RECORD                     0x4F    ///<卡中有未完成的申报纪录
#define UART_COMMAND_INVOICE_FRACTION_LENGTH_ERR                0x50    ///<开票项目数量异常
#define UART_COMMAND_DATA_SIGN_ERROR                            0x51    ///<数据签名错误
#define UART_COMMAND_CHECK_TYPE_ERROR                           0x52    ///<核查类别错误
#define UART_COMMAND_FISCAL_MANAGEMENT_CARD_FULL                0x53    ///<税务管理卡已满
#define UART_COMMAND_USER_CARD_FULL                             0x54    ///<用户卡已满
#define UART_COMMAND_NO_DISTRIBUTED_INV_SCL                     0x55    ///<没有可录入的发票卷
#define UART_COMMAND_FISCAL_CARD_TAXER_NAME_DIFFER              0x56    ///<税控卡纳税人名称不一致
#define UART_COMMAND_USER_CARD_TAXER_NAME_DIFFER                0x57    ///<用户卡纳税人名称不一致
#define UART_COMMAND_DISTRIBUTE_STORAGE_FULL                    0x58    ///<分发发票存储区已满
#define UART_COMMAND_DAILY_MEMORY_FULL                          0x59    ///<日交易存储区已满
#define UART_COMMAND_USER_CARD_NO_FISCAL_CARD                   0x5A    ///<用户卡中无相应税控卡卡号
#define UART_COMMAND_OVER_ITEMS_NUMBER                          0x5B    ///<超出发票最大项目个数
#define UART_COMMAND_DECLARE_DATE_EXCEEDED_CURRENT_RECORD       0x5C    ///<申报截止日期大于当前日交易记录日期
#define UART_COMMAND_POWER_LOST                                 0x5D    ///<供电异常
#define UART_COMMAND_FISCALCODEERR                              0x5E    ///<税控卡编号错误
#define UART_COMMAND_CHANGECARDTIMEOUT                          0x5F    ///<换卡超时
#define UART_COMMAND_REQUESTINSERTUSB                           0x60    ///<请插入USB
#define UART_COMMAND_INITIALZEUSBSTORAGE                        0x61    ///<U盘异常
#define UART_COMMAND_SELECTDECLAREDEVICE                        0x62    ///<选择申报设备
#define UART_COMMAND_CHANGEINVOICEDETAILCARD                    0x63    ///<换明细卡
/**@}*/


/**
 * @addtogroup TaxBox_PARAMETER
 * @ingroup TaxBox
 * @{
 */
#define UART_CMD_PARAMETER_LENGTH 512                   ///<Uart Command Max parameter  Length
#define UART_RSP_PARAMETER_LENGTH 512                   ///<Uart Response Max Parameter Length
#define UART_RSP_PARAMETER_MAX_LENGTH_ONE_RSP (255-2-4) ///<Max Parameter length for each Response
/**@}*/


void TaxBoxInit();  ///<Tax Box Initializer
void TaxBoxMain(void);  ///<Tax Box Incomming UART or USB Commands Dispatcher

/**
 * @defgroup TaxBox_COMMAND_HANDLER
 * Define the function of Tax Box instruction
 * @ingroup TaxBox
 * @{
 */
///联机
void X00_TryConnection(u16 ParameterLength, u8* Parameter);
///税控器初始化
void X30_TaxBoxInitialization(u16 ParameterLength, u8* Parameter);
///分发发票号
void X31_DistributeInvoiceNB(u16 ParameterLength, u8* Parameter);
///已分发的发票号上传
void X32_SubmitDistributedInvoiceNB(u16 ParameterLength, u8* Parameter);
///当前发票号上传
void X33_SubmitCurrentInvoiceNB(u16 ParameterLength, u8* Parameter);
///开正常发票
void X34_IssueInvoice(u16 ParameterLength, u8* Parameter);
///退票
void X35_ReturnInvoice(u16 ParameterLength, u8* Parameter);
///废票
void X36_DiscardInvoice(u16 ParameterLength, u8* Parameter);
///机器出厂数据上传
void X37_SubmitTaxBoxOriginalInfo(u16 ParameterLength, u8* Parameter);
///用户信息上传
void X38_SubmitUserInfo(u16 ParameterLength, u8* Parameter);
///税种税目信息上传
void X39_SubmitTaxSpeciesTaxDetails(u16 ParameterLength, u8* Parameter);
///日期时间上传
void X3A_SubmitDateTime(u16 ParameterLength, u8* Parameter);
///设置时间
void X3B_SetTime(u16 ParameterLength, u8* Parameter);
///单卷发票使用汇总数据上传
void X3C_SubmitSingleScrollInvoiceUsageaAssembleInfo(u16 ParameterLength, u8* Parameter);
///申报数据和发票打印数据写卡
void X3D_WriteCardWithDeclareDataAndInvoicePrintingData(u16 ParameterLength, u8* Parameter);
///申报数据上传
void X3E_SubmitDeclareData(u16 ParameterLength, u8* Parameter);
///发票数据上传
void X3F_SubmitInvoiceData(u16 ParameterLength, u8* Parameter);
///监控管理数据回送
void X40_SubmitSuperviseData (u16 ParameterLength, u8* Parameter);
///税控器状态上传
void X41_SubmitTaxBoxStatus(u16 ParameterLength, u8* Parameter);
///按发票起止号上传发票打印数据
void X42_SubmitInvoicePrintingDataByInvoiceNBRange(u16 ParameterLength, u8* Parameter);
///按时间段上传发票打印数据
void X43_SubmitInvoicePrintingDataByInvoiceTimeRange(u16 ParameterLength, u8* Parameter);
///按时间段上传日交易数据
void X44_SubmitDailyTransactionDataByTimeRange(u16 ParameterLength, u8* Parameter);
///核查数据写卡
void X45_WriteCardWithCheckData(u16 ParameterLength, u8* Parameter);
///数据签名
void X46_DataSignature(u16 ParameterLength, u8* Parameter);
///更新分户开票编号文件
void X47_UpdateUsersIssueInvoiceNB(u16 ParameterLength, u8* Parameter);
///录入发票号
void X50_InputInvoiceNB(u16 ParameterLength, u8* Parameter);
///机器初始化
void X51_ManufactureInit(u16 ParameterLength, u8* Parameter);
///清空内存
void X52_ReVergin(u16 ParameterLength, u8* Parameter);
///发票打印信息处理
void X53_InfoPrtInProgress(u16 ParameterLength, u8* Parameter);
///发票打印完毕
void X54_InfoPrtFinished(u16 ParameterLength, u8* Parameter);
///
void X55_InquireUnfinishedInvData(u16 ParameterLength, u8* Parameter);
///海量测试-初始化
void X56_PressureTestInit(u16 ParameterLength, u8* Parameter);
///海量测试-分发发票
void X57_PressureTestDistribute(u16 ParameterLength, u8* Parameter);
///海量测试-开票
void X58_PressureTestInvoice(u16 ParameterLength, u8* Parameter);
///IC卡上电
void X59_SmartCardPowerOn(u16 ParameterLength, u8* Parameter);
///IC卡下电
void X5A_SmartCardPowerOff(u16 ParameterLength, u8* Parameter);
///IC卡命令
void X5B_SmartCardAPDUCommand(u16 ParameterLength, u8* Parameter);
///CPU自检
void X5C_CPUSelfCheck(u16 ParameterLength, u8* Parameter);
///ROM自检
void X5D_ROMSelfCheck(u16 ParameterLength, u8* Parameter);
///RAM自检
void X5E_RAMSelfCheck(u16 ParameterLength, u8* Parameter);
///MEM自检
void X5F_MEMSelfCheck(u16 ParameterLength, u8* Parameter);
///串口自检
void X60_SerialSelfCheck(u16 ParameterLength, u8* Parameter);
///循环自检
void X61_CycleSelfCheck(u16 ParameterLength, u8* Parameter);
///更新纳税人编码
void X62_UpdateTaxerName(u16 ParameterLength, u8* Parameter);
///修改日期时间
void X63_UpdateDateTime(u16 ParameterLength, u8* Parameter);
///更新税控器固件
void X64_UpdataFirmware(u16 ParameterLength, u8* Parameter);
/**@}*/
/**@}*/



/**
 * @addtogroup USER_Device_Library
 * @{
 */
/**
 * @addtogroup COM
 * @{
 */
/**
 * @defgroup TaxBox_COM_RESPONSE_HANDLER
 * @ingroup COM
 * @{
 */
void SendRsp(void);             ///<Response On UART incoming Command immediatly
void Response_OK(u16 length);   ///<Response OK on Uart Incoming Command but not immediatly
void Response_ER(u8 errorCode); ///<Response ER on Uart Incomming Command but not immediatly
void Response_NP(u8 SN);        ///<Response NP on Uart Incomming Command but not immediatly
void SendRsp_WT(void);          ///<Response WT on Uart Incomming Command but not immediatly
void SendRspCancel(void);       ///<Cancel last registed Response Information
bool SendRsp_OK_Manual(u16 paraLength, u8 SN); ///<Manually Send One OK Response
bool SendRsp_CC(void);          ///<Manually Send One change card Response
/**@}*/
/**@}*/
/**@}*/

#endif /*_TAXBOX_H_*/
