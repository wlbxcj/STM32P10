/// @file   Data.h
/// This is the Head file for module @ref fiscal Data
/// @author Travis Tu
/// @date   2006-Sep-1st

#ifndef _DATA_H_
#define _DATA_H_


/**
 * @defgroup TaxBox_PARAMETER
 * @ingroup TaxBox
 * @{
 */
///all on chip Data
#define TAX_INDEX_NB                        6       ///<the tax items number
#define TAX_INVOICE_SCROLL_DISTRIBUTED_NB   20 ///<To be contained in one response
#define CELL_OCUPIED                        0xEE
#define MAX_ITEM_NB_IN_ONE_INVOICE 10  ///<considerate it and promiss 4+sizeof(UARTIssInvParamType) + sizeof(InvoicePrintInfoType) < UART_CMD_PARAMETER_LENGTH
#define TAXER_PAYER_NAME_LENGH              40
#pragma pack()

///enumeration for three issue invoice types
typedef enum
{
    ISSUE_INVOICE   = 0x01,
    RETURN_INVOICE  = 0x02,
    DISCARD_INVOICE = 0x03
}IssueInvCmdModeType;

///enumeration for four check types
typedef enum
{
    DETAIL_CHECK = 0x00,
    DAILY_TRANSACTION_DATA_CHECK = 0x01,
    APPLY_DATA_CHECK = 0x02,
    MODIFY_CLOCK = 0x03
}CheckModeType;

#pragma pack(1)
///Structure for element of machine initialize data
typedef struct
{
    u8 MachineNB[8];                    ///<机器编号
    u8 ProduceDate[4];                  ///<生产日期        Big Endian
    u8 HardwareVersion[18];             ///<硬件版本号
    u8 SoftwareVersion[18];             ///<软件版本号
}POSInitDataType ;

///Structure for element of the user data
typedef struct
{
    u8 FiscalICCNB[8];                      ///<税控卡编号
    u8 FiscalICCPassword[8];                ///<税控卡使用口令
    u8 AppStartDate[4];                     ///<应用启始日期            Big Endian
    u8 AppStopDate[4];                      ///<应用有效日期            Big Endian
    u8 TaxerCompanyName[TAXER_PAYER_NAME_LENGH];                ///<纳税人名称or纳税人公司名称
    u8 TaxerNB[8];                          ///<纳税人编号
    u8 TaxerID[20];                         ///<税号or纳税人识别号
    u8 AdminDepNB[4];                       ///<主管分局代码
    u8 ApplayWay;                           ///<申报方式
    u8 IssuerInvoiceDeadLine[4];            ///<开票截止日期            Big Endian
    u32 SingleInvoiceUpLimitation;          ///<单张发票开票金额限额    Big Endian
    u32 IssueInvoiceTotalUpLimitation;      ///<开票累计金额限额        Big Endian
    u32 ReturnedInvoiceTotalUpLimitation;   ///<退票累计金额限额        Big Endian
}POSUserInfoType;

///Structure for element of the tax items info
typedef struct
{
    u8 TaxIndex;                        ///<税种税目索引号
    u8 TaxCode[4];                      ///<税种项目代码
    u8 TaxRate[2];                      ///<税率
    u8 TaxTypeCNName[20];               ///<经营项目中文名称
    u8 TaxTypeENName[20];               ///<经营项目英文名称
}TaxSpeciesTaxDetailsType;

///Structure for element of the invoice roll usage info
typedef struct{
	u8	InvoiceCode[10];                ///<发票代码
	u32	InvoiceBeginNB;                 ///<发票起始号 Big Endian
	u32	InvoiceEndNB;                   ///<发票终止号 Big Endian
	u8	NomInvoiceNB;                   ///<正常发票份数
	u8	RtnInvoiceNB;                   ///<退票份数
	u8	DscInvoiceNB;                   ///<废票份数
	u32 NomInvoiceTotalAmt;             ///<正常发票总金额    Big Endian
	u32	RtnInvoiceTotalAmt;             ///<退票总金额        Big Endian
	u8	IssueInvoiceBeginTime[4];       ///<开票起始时间      Big Endian
	u8	IssueInvoiceEndTime[4];         ///<开票截止时间      Big Endian
}SingleInvoiceScrollUsageDataType;

typedef struct{
    u8  TaxerCompanyName[TAXER_PAYER_NAME_LENGH];           ///<纳税人名称or纳税人公司名称
    u8  TaxerID[20];                    ///<税号or纳税人识别号
    u8  MachineNB[8];                   ///<机器编号
	u8	InvoiceCode[10];                ///<发票代码
	u32	InvoiceBeginNB;                 ///<发票起始号 Big Endian
	u32	InvoiceEndNB;                   ///<发票终止号 Big Endian
	u8	NomInvoiceNB;                   ///<正常发票份数
	u8	RtnInvoiceNB;                   ///<退票份数
	u8	DscInvoiceNB;                   ///<废票份数
	u32 NomInvoiceTotalAmt;             ///<正常发票总金额    Big Endian
	u32	RtnInvoiceTotalAmt;             ///<退票总金额        Big Endian
	u8	IssueInvoiceBeginTime[4];       ///<开票起始时间      Big Endian
	u8	IssueInvoiceEndTime[4];         ///<开票截止时间      Big Endian
}InvoiceScrollUsagePrinterInfo;

typedef struct{
    u8  TaxerCompanyName[TAXER_PAYER_NAME_LENGH];           ///<纳税人名称or纳税人公司名称
    u8  TaxerID[20];                    ///<税号or纳税人识别号
    u8  MachineNB[8];                   ///<机器编号
	u8	IssueInvoiceBeginTime[4];       ///<开票起始时间      Big Endian
	u8	IssueInvoiceEndTime[4];         ///<开票截止时间      Big Endian
	u8	NomInvoiceNB;                   ///<正常发票份数
	u32 NomInvoiceTotalAmt;             ///<正常发票总金额    Big Endian
	u8	DscInvoiceNB;                   ///<废票份数
	u8	RtnInvoiceNB;                   ///<退票份数
	u32	RtnInvoiceTotalAmt;             ///<退票总金额        Big Endian
}InvoiceUsagePrinterInfo;

///Structure for element of the invoice info
typedef struct{
    u8  IssueInvoiceDate[4];            ///<开票日期      Big Endian
    u32 InvoiceNB;                      ///<发票号        Big Endian
    u8  IssueInvoiceType;               ///<开票类型
    u32 InvoiceAmount;                  ///<开票总金额    Big Endian
    u8  TaxContralNB[8];                ///<税控码
    u32 OriginalInvoiceNB;              ///<原发票号      Big Endian
    u8  RFU[2];                         ///<保留
}InvoicePrintInfoType;

///Structure for element of the DISTRIBUTE_INVOICE_NB command APUD carry data format
typedef struct{
    u8  InvoiceCode[10];                ///<发票代码
    u32 SingleScrollBeginNB;            ///<单卷发票起始号    Big Endian
    u32 SingleScrollEndNB;              ///<单卷发票终止号    Big Endian
    u8  MAC[4];                         ///<MAC
    u8  CellOccupied;                   ///<if this cell is in use , then set this byte to 0xEE CELL_OCUPIED
}SingleInvoiceScrollType;

typedef struct
{
    u8  ItemName[20];               ///<商品名称
    union{
        u8  DecFractionLength;      ///<数量的右起小书点位数
        u32 ItemQuantity;           ///<数量            Big Endian
    }Quantity;
    u32 UnitPrice;		            ///<单价	        Big Endian
    u32 ItemCost;                   ///<单项总金额      Big Endian
    u8  TaxIndex;                   ///<税种税目索引号
}INVOICEITEM;

///Structure for element of the data from the UART port
typedef struct{
    u16 SubAccNB;                       ///<分户编号          Big Endian
    u8 TaxerName[TAXER_PAYER_NAME_LENGH];                   ///<付款单位名称
    u8 ItemsNum;                        ///<经营项目个数
    struct{
        u8  ItemName[20];               ///<商品名称
        union{
            u8  DecFractionLength;      ///<数量的右起小书点位数
            u32 ItemQuantity;           ///<数量                Big Endian
        }Quantity;
        u8  TaxIndex;                   ///<税种税目索引号
        u32 UnitPrice;                  ///<单价	Big Endian
    }Items[MAX_ITEM_NB_IN_ONE_INVOICE]; ///<多个item结构
}UARTIssInvParamType;

///Structure for element of the invoice detail store info
typedef struct{
    u8 TaxerName[TAXER_PAYER_NAME_LENGH];                   ///<付款单位名称
    u8 OperatorName[10];                ///<收款员
    u8 ItemsNum;                        ///<项目数
    struct{
        u8  ItemName[20];               ///<商品名称
        union{
            u8  DecFractionLength;      ///<数量的右起小书点位数
            u32 ItemQuantity;           ///<数量            Big Endian
        }Quantity;
        u32 UnitPrice;		            ///<单价	        Big Endian
        u32 ItemCost;                   ///<单项总金额      Big Endian
        u8  TaxIndex;                   ///<税种税目索引号
    }Items[MAX_ITEM_NB_IN_ONE_INVOICE]; ///<多个item结构
}INVOICEDETAILINFO;

///Structure for element of the ISSUE_INVOICE command APUD carry data format
typedef struct{
    u8 Date[4]; 		                ///<开票时间            Big Endian
    u8 IssType;                         ///<开票类型 01 正常发票 02 退票 03 废票
    u32 InvNB;                          ///<发票号              Big Endian
    struct{                             ///<发票分类金额, 按税种税目计算,分成6类
        u8 TaxIndex;                    ///<税种税目索引号
        u32 Cost;                       ///<该税种消费总额      Big Endian
    }ClassifiedCost[TAX_INDEX_NB];
    u32 TotalCost;                      ///<开票总金额          Big Endian
    u8 CS;                              ///<命令报文数据异或和校验
}ICCIssInvCarryDataType;

///Structure for element of the declare data
typedef struct{
    u8  StartDate[4];                   ///<申报数据起始日期	4 CCYYMMDD  Big Endian
    u8  StopDate[4];                    ///<申报数据截止日期	4 CCYYMMDD  Big Endian
    u32 NomInvNB;                       ///<正常发票份数	    4           Big Endian
    u16 RtnInvNB;                       ///<退票份数            2           Big Endian
    u16 DscInvNB;                       ///<废票份数            2           Big Endian
    u8  TaxIndexs[TAX_INDEX_NB];        ///<税种税目索引号	    6
    u32 NomInvAmt[TAX_INDEX_NB];        ///<正常发票分类累计金额            Big Endian
    u32 RtnInvAmt[TAX_INDEX_NB];        ///<退票分类累计金额                Big Endian
    u32 NomInvTotalAmt;                 ///<税控卡正常发票总累计金额        Big Endian
    u32 RtnInvTotalAmt;                 ///<税控卡退票总累计金额            Big Endian
}DeclareDataType;

///Structure for element of the DATA_COLLECT command APUD carry data format
typedef struct{
    u8  FiscalCardNB[8];                ///<税控卡编号	        8
    u8  StartDate[4];                   ///<申报数据起始日期	4 CCYYMMDD  Big Endian
    u8  StopDate[4];                    ///<申报数据截止日期	4 CCYYMMDD  Big Endian
    u32 NomInvNB;                       ///<正常发票份数	    4           Big Endian
    u16 RtnInvNB;                       ///<退票份数            2           Big Endian
    u16 DscInvNB;                       ///<废票份数            2           Big Endian
    u8  TaxIndexs[TAX_INDEX_NB];        ///<税种税目索引号	    6
    u32 NomInvAmt[TAX_INDEX_NB];        ///<正常发票分类累计金额            Big Endian
    u32 RtnInvAmt[TAX_INDEX_NB];        ///<退票分类累计金额                Big Endian
    u32 NomInvTotalAmt;                 ///<税控卡正常发票总累计金额        Big Endian
    u32 RtnInvTotalAmt;                 ///<税控卡退票总累计金额            Big Endian
    u8  StatusByte;                     ///<状态字              1
    u8  MAC1[4];                        ///<申报数据MAC1        4
    u8  Signature[128];                 ///<电子签名            128
}ICCDataCollectCarryDataType;

typedef struct{
    u8  MachineNB[8];                   ///<机器编号	        8
    u8  StartDate[4];                   ///<申报数据起始日期	4 CCYYMMDD  Big Endian
    u8  StopDate[4];                    ///<申报数据截止日期	4 CCYYMMDD  Big Endian
    u8  RecordFileName[8];              ///<日交易数据文件名
    u8  XOR;                            ///<数据异或和校验
}DeclareDailyIndexFileType;

typedef struct{
    u8  MachineNB[8];                   ///<机器编号	        8
    u8  StartDate[4];                   ///<申报数据起始日期	4 CCYYMMDD  Big Endian
    u8  StopDate[4];                    ///<申报数据截止日期	4 CCYYMMDD  Big Endian
    u16 Recordtotal;                    ///<发票明细总纪录数        Big Endian
    u8  RecordFileName[8];              ///<发票明细数据文件名
    u16 RecordStartNo;                  ///<发票明细开始记录编号    Big Endian
    u16 RecordEndNo;                    ///<发票明细结束纪录编号    Big Endian
    u8  XOR;                            ///<数据异或校验
}DeclareDetailIndexFileType;

///Structure for element of the declare store data
typedef struct{
    DeclareDataType dclData;            ///<申报数据
    u8  StatusByte;                     ///<状态字              1
    u8  MAC1[4];                        ///<申报数据MAC1        4
    u8  Signature[128];                 ///<电子签名            128
}DeclareDataLogType;

///Structure for element of the invoice print store data
typedef struct{
    InvoicePrintInfoType PrintInfo;
    u8 InvoiceScrollCode[10];		    ///<发票代码
    INVOICEDETAILINFO    InvoiceDetailInfo;
    u8                      TaxerCompanyName[TAXER_PAYER_NAME_LENGH];           ///<纳税人名称or纳税人公司名称
    u8                      Time[2];                        ///<开票时间
}InvoiceDetailInfoType;


typedef struct{
    InvoicePrintInfoType    PrintInfo;
    u8                      InvoiceScrollCode[10];		    ///<发票代码
    INVOICEDETAILINFO       InvoiceDetailInfo;
    u8                      TaxerCompanyName[TAXER_PAYER_NAME_LENGH];           ///<纳税人名称or纳税人公司名称
//    BankingInfoType         BankingData[2];
    u8                      Time[2];                        ///<开票时间
}InvoiceCounterfoilType;

typedef struct{
    InvoicePrintInfoType    PrintInfo;
    u8                      InvoiceScrollCode[10];		    ///<发票代码
    INVOICEDETAILINFO       InvoiceDetailInfo;
    u8                      TaxerCompanyName[TAXER_PAYER_NAME_LENGH];           ///<纳税人名称or纳税人公司名称
//    BankingInfoType         BankingData[2];
    u8                      TaxerID[20];                    ///<税号or纳税人识别号
    u8                      MachineNB[8];                   ///<机器编号
}InvoicePrintInfo;
///Structure for element of the daily transaction data
typedef struct{
    u8  CurDate[4];                     ///<当前日期                Big Endian
    u16 NomInvNB;                       ///<当天正常发票份数        Big Endian
    u16 RtnInvNB;                       ///<当天退票份数            Big Endian
    u16 DscInvNB;                       ///<当天废票份数            Big Endian
                                        ///<当天发票分类统计金额
    u8  TaxIndexs[TAX_INDEX_NB];        ///<税种税目索引号6B
    u32 NomInvAmt[TAX_INDEX_NB];        ///<正常发票分类累计金额    Big Endian
    u32 RtnInvAmt[TAX_INDEX_NB];        ///<退票分类累计金额        Big Endian
    u8  Sign[128];                      ///<电子签名
}DailyTransctionDataType;

///Structure for element of the tax box management data
typedef struct
{
    u8 TaxIndexsNb;					
    u8 TaxIndexs[TAX_INDEX_NB];         ///<税种索引序列

    ICCDataCollectCarryDataType DclData;                ///<当前申报数据

    SingleInvoiceScrollUsageDataType CurInvScl;         ///<当前发票卷详细信息
    u32                              CurInvSclCurInvNB; ///<当前发票卷之当前还未使用的第一张发票号 Big endian

    DailyTransctionDataType          DayTscData;        ///<日交易数据

    SingleInvoiceScrollType          InvScl[TAX_INVOICE_SCROLL_DISTRIBUTED_NB]; ///<当前被分发但未被录入的发票卷信息
    SingleInvoiceScrollType          DispenseAbortData;
    u8  RandomPassword[8];                              ///<税控卡的随机密码
    u32 CurrNomInvTotalAmt;             ///<税控卡正常发票总累计金额        Little Endian
    u32 CurrRtnInvTotalAmt;             ///<税控卡退票总累计金额            Little Endian

    u32 CurPrtingInvNB;                 ///<当前正在打印的发票号            Big Endian
    u8 DeclareType;                     ///<申报类型
    u8 DistributeIndex;                 ///<最后一次分发的发票卷位置
}POSInterInfoType;

typedef struct
{
    u8 FiscalICCNB[8];                  ///<税控卡编号
    u8 EncryptSymbol;                   ///<数据加密密钥标识符
    u8 Cryptograph[32];                 ///<税控数据密文
    u8 MAC[4];                          ///<MAC2
}USERCARDINFOSFI01;

typedef struct
{
    u8 CardType;                        ///<卡类型标志
    u8 MachineNb;                       ///<税控收款机数量
    u8 TaxerNB[8];                      ///<纳税人编号
    u8 AppStartDate[4];                 ///<应用启始日期            Big Endian
    u8 AppStopDate[4];                  ///<应用有效日期 }USERCARDINFOSFI02;
    u8 AppEdition;                      ///<应用版本
    u8 FCI;                             ///<发卡方自定义FCI数据
    u8 TaxerCompanyName[TAXER_PAYER_NAME_LENGH];            ///<纳税人名称or纳税人公司名称
}USERCARDINFOSFI02;

typedef struct
{
    u8 MachineNB[8];                    ///<机器编号
    u8 FiscalICCNB[8];                  ///<税控卡编号
    u8 SFI[2];                          ///<日交易数据文件标识符
}USERCARDINFOSFI09;

typedef struct
{
    u8  MachineNB[8];                   ///<机器编号
    u8  StartDate[4];                   ///<申报数据起始日期	4 CCYYMMDD  Big Endian
    u8  StopDate[4];                    ///<申报数据截止日期	4 CCYYMMDD  Big Endian
    u16 Recordtotal;                    ///<发票明细总纪录数        Big Endian
    u8  SFI[2];                         ///<发票明细数据文件标示符
    u16 RecordStartNo;                  ///<发票明细开始记录编号    Big Endian
    u16 RecordEndNo;                    ///<发票明细结束纪录编号    Big Endian
    u8  XOR;                            ///<数据异或校验
}USERCARDINFOSFI10;

typedef struct
{
    u8  SFI[2];                         ///<发票明细数据文件标示符
    u16 FileLen;                        ///<发票明细数据文件长度
}USERCARDINFOSFI11;

typedef struct
{
    u8  Flag;                                   ///<发票明细数据标志
    u16 Len;                                    ///<发票明细数据长度

    u8  IssueInvoiceDate[4];                    ///<开票日期      Big Endian
    u8  IssueInvoiceType;                       ///<开票类型
    u32 InvoiceNB;                              ///<发票号        Big Endian
    u32 InvoiceAmount;                          ///<开票总金额    Big Endian
    u8  TaxContralNB[8];                        ///<税控码
    u32 OriginalInvoiceNB;                      ///<原发票号      Big Endian
    
    u8 InvoiceScrollCode[10];		    ///<发票代码
    INVOICEDETAILINFO    InvoiceDetailInfo;
    
    u8  XOR;                                    ///<发票明细数据异或校验
}USERCARDINFOINVOICEDETAIL;


typedef struct
{
    u8  InvoiceBlackFlag : 1;                   ///<黑标是否检测
    u8  PrintDirection : 1;                     ///<最后打印方向
    u32 PageMoveStep;                           ///<最后走纸步数
}PRINTERINFO;

#pragma pack()
/**@}*/

#define SF_ADDR_INVOICE_SCROLL_INFO     0x50000000
#define SF_ADDR_INVOICE_PRINT_INFO      (SF_ADDR_INVOICE_SCROLL_INFO+POS_INVOICE_SCROLL_INFO_NB*sizeof(SingleInvoiceScrollUsageDataType))
#define SF_ADDR_DAILY_TRANSACTION_DATA  (SF_ADDR_INVOICE_PRINT_INFO+POS_INVOICE_PRINT_INFO_NB*sizeof(InvoiceCounterfoilType))
#define SF_ADDR_DECLARE_DATA            (SF_ADDR_DAILY_TRANSACTION_DATA+POS_DAILY_TRANSACTION_DATA_NB*sizeof(DailyTransctionDataType))

/**
 * @addtogroup TaxBox_PARAMETER
 * @ingroup TaxBox
 * @{
 */
//all on SPI Flash Data
#define POS_INVOICE_SCROLL_INFO_NB      300    ///<存储单卷发票使用汇总数据的最大纪录数
#define POS_INVOICE_PRINT_INFO_NB       5718   ///<存储发票打印数据的最大纪录数
#define POS_DAILY_TRANSACTION_DATA_NB   (5 * 366)///<存储日交易信息的最大纪录数
#define POS_DECLARE_DATA_NB             300    ///<存储申报数据的最大纪录数


extern POSInitDataType POSInitData;             ///<the machine initialize info
extern POSUserInfoType POSUserInfo;             ///<the user info
extern TaxSpeciesTaxDetailsType POSTSTD[TAX_INDEX_NB];///<tax Species item info
extern POSInterInfoType POSInterInfo;           ///<the Tax Box management info

extern SingleInvoiceScrollUsageDataType * POSInvScl;    ///<max POSInvScl[POS_INVOICE_SCROLL_INFO_NB-1]
extern u32  POSInvSclNB;        ///<已经使用的 历史单卷发票使用汇总数据 的条数

extern InvoiceCounterfoilType *POSInvPrt;               ///<max POSInvPrt[POS_INVOICE_PRINT_INFO_NB-1]
extern u32  POSInvPrtNB;        ///<已经使用的 发票打印以及明细信息 的条数

extern DailyTransctionDataType * POSDayData;            ///<max POSDayData[POS_DAILY_TRANSACTION_DATA_NB-1]
extern u32  POSDayDataNB;       ///<已经使用的 日交易数据 的条数

extern DeclareDataLogType * POSDclData;                 ///<max POSDclData[POS_DECLARE_DATA_NB-1]
extern u32  POSDclDataNB;       ///<已经使用的 申报数据log 的条数

extern u32  PowerLostPrintLineNum;
extern PRINTERINFO PrintMemoryInfo;
/**@}*/

extern bool Comm_Flag;
extern InvoiceCounterfoilType prtInfo;


/*all data related API*/
///Store the daily transaction data
bool storeDailyData(RTCTimeType *pCurrentTime);

/**
 * @addtogroup TaxBox_COMMAND_HANDLER
 * @ingroup TaxBox
 * @{
 */

/**
 * @defgroup Fiscal_Initialize
 * The module is initialize the Tax Box according to GB18240.2
 * @ingroup TaxBox_COMMAND_HANDLER
 * @{
 */
///Authentication ID from Fiscal Card and Tax Box
bool RightCardsInstered_Fiscal(void);
///Authentication ID from User Card and Tax Box
bool RightCardsInstered_User(void);

/**@}*/

/**
 * @defgroup Invoice_Rolls
 * The module provide integrated invoice rolls distribution and management function
 * @ingroup TaxBox_COMMAND_HANDLER
 * @{
 */

///get the index inside POSInterInfo.InvScl[] which is empty for usage
u8 getEmptyInvSclCell(void);
///whether exist appointed invoice scroll and return index
u8 existInvSclCell(u32 invoiceNB);
///load a new invoice scroll to the current invoice scroll uasge information
void loadNewInvScl(SingleInvoiceScrollType* InvScl);
///list the occupied invoice scroll on given buffer one SingleInvoiceScrollType by one SingleInvoiceScrollType (excluding u8 CellOccupied)
u16 listOccupiedInvSclCell(u8* buff);

/**@}*/

/**
 * @defgroup Issue_Invoice
 * @ingroup TaxBox_COMMAND_HANDLER
 * @{
 */
///Check Current invoice roll used out
bool curInvSclUsedOut(void);
///Adjust the invoice detail data
bool Adjust_Invoice_Detail(UARTIssInvParamType *pInputData, INVOICEDETAILINFO *pDetailData);
///Judge the tax item
bool checkTSTDLegal(INVOICEDETAILINFO* InpData);
///Search the Species tax item No
u8 getTSTD(u8 TaxIndex);
///Check exist of the account
bool checkSubAccountExist(UARTIssInvParamType* InpData);
///build input data of ICC command ISSUE_INVOICE
void buildICCIssInvCarryData(INVOICEDETAILINFO * InpData,
                             ICCIssInvCarryDataType* CryData, IssueInvCmdModeType mode);
///Save the invoice print data
void SaveInvoice(InvoiceCounterfoilType *pInputInfo);
///Update Current Invoice Roll data
void Data_UpdateCurInvScl(u32 amount, RTCTimeType *pRTCTime,
                          IssueInvCmdModeType mode);
///Update the daily transaction data
void updataDailyData_IssInv(INVOICEDETAILINFO* InpData,IssueInvCmdModeType mode);
/**@}*/


///Read the user information from user card, and update the relate data
bool UpdateUserInformation(u8 FromUserCardFlag);
/**@}*/


/**
 * @defgroup Tax_Management
 * @ingroup TaxBox_COMMAND_HANDLER
 * @{
 */
///Authentication safely by the fiscal management card
bool authFiscalManageCard(u8* CheckType, RTCTimeType *pCurrentTime, u8* pCheckLevel);
///Change the fiscal Management card
bool Change_Management(u8 CheckTypeOld);

u8  Data_CheckInvoiceContent(u8 *pContent, u32 *pInvoiceAmount);

/**@}*/
/**@}*/

///Search the single roll invoice usage data
bool getHistorySglInvSclInfo(u32 BeginNB,SingleInvoiceScrollUsageDataType* SclInfo);
///为了X3F_SubmitInvoiceData的比较函数
bool X3F_SubmitInvoiceData_COMPARE(u8* Parameter,InvoicePrintInfoType* prtData);
///为了X42_SubmitInvoicePrintingDataByInvoiceNBRange的比较函数
bool X42_SubmitInvoicePrintingDataByInvoiceNBRange_COMPARE(u8* Parameter,InvoicePrintInfoType* prtData);
///为了X43_SubmitInvoicePrintingDataByInvoiceTimeRange的比较函数
bool X43_SubmitInvoicePrintingDataByInvoiceTimeRange_COMPARE(u8* Parameter,InvoicePrintInfoType* prtData);
///为了X44_SubmitDailyTransactionDataByTimeRange的比较函数
bool X44_SubmitDailyTransactionDataByTimeRange_COMPARE(u8* Parameter,DailyTransctionDataType* dayData);
///按给定条件查找发票打印数据
void submitInvPrtDataOnCondition(u8* Parameter, bool (*cmp)(u8*,InvoicePrintInfoType*));
///按给定条件查找日交易数据
void submitDailyDataOnCondition(u8* Parameter, bool (*cmp)(u8*,DailyTransctionDataType*));
#endif

