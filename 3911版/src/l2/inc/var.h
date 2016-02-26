/*******************************************************************************
* File Name			 : var.h
* Author             : guohonglv
* Date First Issued  : 2010年11月12日9:55:48
* Description        : 直接引用QPBOCTEST项目的文件，为标准C语言改变了一下风格，并增加了头文件的引用
********************************************************************************/

#define uchar unsigned char

#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "emvcore.h"
#include "contactless_common.h"
#include "entrycore.h"
#include "qpboccore.h"
//#include "keymap.h"
#include "fcntl.h"
#include "emvif.h"
#include "struct.h"
//#include "logic_error.h"
//#include "errno.h"
#include "stat.h"
//#include "kf322_picc.h"
//#include "SystemFunction.h"
#include "scc-support.h"

#include "UartProtocol.h"

#pragma pack(1) 
typedef struct _POS_PARAM_STRC 
{
	uchar ucCommWaitTime; 	/*通讯等待时间*/
	uchar szUnitNum[16];
	uchar szPosId[9];
	uchar szUnitChnName[41];
	int  iMaxTransTotal;  	/*流水保存的最大交易笔数，可设*/
	uchar ucEmvSupport;
	uchar ucSelectCardType;	// PARAM_OPEN: oper select card type, PARAM_CLOSE: auto-select
	long lNowTraceNo; 		/*当前终端交易流水号*/
	long lNowBatchNum;		/*当前批次号 */
	uchar ucOprtLimitTime;  /* 操作限时时间 30S-60S*/
		

}T_POS_PARAM_STRC;

typedef struct T_CTRL_PARAM 
{

	uchar bPosStatus; /*'0' 正常 '1'测试状态 '2'批上送状态*/
	uchar bBatchUpStatus; 	/*当前批上送状态：RMBLOG 上送人民币卡 FRNLOG上送外卡 ALLLOG都上送状态 */
	uchar bLoadTask;  /*是否有下载任务标志 1 - 需要 0 - 不需要*/
	uchar sLastTransDate[9]; /*最后一笔交易的日期*/
	int   nTransNum;  	/*当前交易总笔数*/
	int   nBatchNum;  	/*当前Batch总笔数*/
	uchar ucClearLog;	/*结算后是否未清日志    TURE:未清*/
	uchar bEmvCapkLoaded;	// CAPK是否下载成功
	uchar bEmvAppLoaded;	// EMV AID参数是否下载成功

}T_CTRL_PARAM;

typedef struct _STRUCT_PACK
{
    unsigned char  TransType;        /* 9cTransaction type */
	unsigned char  TranDate[3];      //9a
    unsigned char  PanLen;
    unsigned char  Pan[10];          /* 5aPrimary account No. */
	unsigned char  PANSN;            //5f34
//    unsigned char  AmountLen;  //20090523 disable
    unsigned char  sAmount[6];        /* 9f02Trancaction amount */
//	unsigned char  AACLen;
	unsigned char  AAC[8];           //9f26 80Application Cryptogram (ARQC)
	unsigned char  CryData;          //9f27 81Cryptogram Information Data
	unsigned char  IssuerAppLen;
	unsigned char  IssuerAppData[32];//9f10 61Issuer Application Data (IAD)
	unsigned char  UnpredictNum[4];  //9f37 90Unpredictable Number
	unsigned char  ATC[2];           //9f36 89Application transaction counter (ATC)
	unsigned char  TVR[5];           //95 40Terminal Verification Result
	unsigned char  TranCurrency[2];  //5f2a 9Transaction Currency Code
	unsigned char  AIP[2];           //82 23Application Interchange Profile
	unsigned char  TerminalCountry[2];//9f1a 70Terminal Country Code
//	unsigned char  AmountOtherLen;
	unsigned char  AmountOtherNum[6];//9f03 50Amount Other
	unsigned char  POSEntry;         //9f39 92POS Entry Mode 9f39
	unsigned char  TranSeq[4];       //9f41 Transaction Sequence Counter
	unsigned char  MsgType[2];       //Advice - \x01\x20
	unsigned char  EC_Trans;
	unsigned char  Ec_Balance[6];
}T_EMVTRANSREC;

typedef struct POS_COM_STRUCT 
{    
	long  lBalance;			/*  卡帐户余额          N(9)    */
///	uchar  szOldTrnDate[5];	/*  原交易日期                  */
	uchar  szRespCode[2+1];       /*  交易返回码              */
	uchar  szUnitNum[15+1];		/*  特约单位号          N(8)   */
	uchar  sPIN[8];				/*  个人密码                    */
	uchar  szNetInfoCode[3+1];
	uchar  szTrack2[TRACK2_LEN+1];
	uchar  szTrack3[TRACK3_LEN+1];
	
	//disable 2010/12/29
	//uchar  sField48[512];
	//int	   iField48Len;
	//uchar  sField62[512];
	//uchar  sField61[512];
	
	int    iField61Len;
	uchar  ucPrtflag;			//交易结束后是否打印凭条 
	uchar  ucWriteLog;			//交易结束后是否记日志
//	LOG_STRC stTrans;			///交易要素信息，需要记录到日志中的数据  09/06/29 
    uchar szCardNo[21];       //move from stTrans;
    T_EMVTRANSREC tEmvTrans; // 09/06/30 add
//	POS_8583PACKET_STRC stTrnTbl;  ///交易打包，是否打印等的信息   09/06/29
	uchar  ucAmtSign;	///金额的正负号 '+' '-'
	uchar  ucFallBack;	// TRUE , FALSE
	uchar  ucSwipedFlag; // 刷卡的标志
	uchar  ucPinEntry;	// TRUE: PIN enter, FALSE: No PIN enter
	//uchar  sReversalDE55[128];	// 冲正和脚本通知的BIT 55数据

	//disable 2010/12/30
//	uchar  sReversalDE55[219];	//reverse(test)最大长度为218,add type 冲正和脚本通知的BIT 55数据
//	int    iReversalDE55Len;
//	uchar  sScriptDE55[128];
//	int    iScriptDE55Len;
	
	uchar  bOnlineTxn;	// ICC online falg
	uchar  szCertData[2+20+1];	// type(2)+ID no(20)
	uchar  bIsFirstGAC;
//	STISO8583	stOrgPack;
	uchar	ucIsWriteRevLog;		// 是否已经记录冲正流水的标志(用于记录冲正流水文件的标志，防止重复记冲正流水)
	uchar  bSkipEnterAmt;	// for process TIP fallback when 1st GAC return 6985 [7/26/2006 Tommy]

}POS_COM;





/* Terminal supports exception file*/
typedef struct
{
	uchar bLen;
	uchar PAN[10];
	uchar PANSeqNum;
	
}T_TERMEXCEPTIONFILE;


typedef struct temp_global_var_strc
{
	int	 iTransNo;			//交易标号,
	int  iLogFilePosi;		/*被撤销的交易在文件中的位置*/
	uchar ucPINTimes;	//密码输入次数,终端控制不要超过3次
	uchar ucRmbSettRsp;		/*人民币卡结算结果*/
	uchar ucFrnSettRsp;		/*外币卡结算结果*/
	uchar szDate[9];
	uchar szTime[7];
	uchar sPacketHead[7];
	uchar szSendId; /*原交易是否已经上送 1：是 0：否*/
	uchar szDispBuf[33];
	uchar ucPrnVer; /* 打印机型号 'S' 针打 'T' 热敏打印*/
}T_TEMP_VAR;

typedef struct 
{
	uchar bId;
	uchar sAbbr[3];    //abbreviation
	uchar sName[20];
	
}T_LANGUAGE;

typedef struct 
{
	char sAmount[10];
	char sAmountOK[20];
	char sApproved[20];
	char sCallYourBank[30];
	char sCancelOrEnter[30];
	char sCardError[10];
	char sDeclined[10];
	char sEnterAmount[20];
	char sEnterPIN[20];
	char sIncorrectPIN[20];
	char sInsertCard[12];
	char sNotAccepted[20];
	char sPINOK[10];
	char sPleaseWait[20];
	char sProcessingError[20];
	char sRemoveCard[20];
	char sUseChipReader[20];
	char sUseMagStripe[20];
	char sTryAgain[10];
	
}T_STANDMESS;


typedef struct 
{
	uchar Buff[1024];
	uchar Len;
	uchar RecNum;
	uchar SFI;
}GLSTACK;

typedef struct TERMAPPLIST
{
	uchar Aid[17];
	uchar Aidlen;
	uchar onlinepin;
	uchar ucSelflag;
	long ulECTTLVal;
	uchar ucRdClssTxnLmtFlg;			// 是否存在读卡器非接触交易限额
	unsigned long ulRdClssTxnLmt;	//读卡器非接触交易限额 （n12）
	uchar ucRdCVMLmtFlg;				// 是否存在读卡器非接触CVM限额
	unsigned long ulRdCVMLmt;		// 读卡器非接触CVM限额（n12）
	uchar ucRdClssFLmtFlg;		// 是否存在读卡器非接触限额
	unsigned long ulRdClssFLmt;		//读卡器非接触限额 （n12） 
	unsigned char sRFU[1];   
}TERMAPPLIST;

#pragma pack() 

enum 
{
	LOCALE,  //0
	SUPPORT, //1
};
#define COM_SIZE  sizeof(struct POS_COM_STRUCT)

extern POS_COM gtPosCom;

extern T_EMV_PARAMETER gtEmvParameter;
extern T_CTRL_PARAM    gtCtrlParameter;
extern T_POS_PARAM_STRC gtPosParameter;

extern POS_COM gtPosCom;
extern T_TEMP_VAR gtTemp;

//contactless
extern  Contactless_ReaderParam ClssParam;
//extern	Clss_TransParam_stru ClssEcParam;

//extern	PICC_PARA PiccPara; 
//	EMV_REVOCLIST EmvRevoclist;
//extern	Clss_PreProcInfo_Stru qvClssParam;
//extern	Clss_PreProcInterInfo ClssProcInterInfo;
//	Clss_VisaAidParam ClssVisaAidParam;
//	Clss_PbocAidParam ClssPbocAidParam;
//	Clss_SchemeID_Info ClssSchemeIDInfo;
//	uchar Mob_Pboc_Support,qpboc_stau;

//extern TERMAPPLIST Term_Applist[AppListStoredNum];



extern T_STANDMESS glTStandMess[2];
extern T_LANGUAGE gltLanguage[2];
extern int glbLanguage;
extern char sDispEnterPINAgain[2][25];
extern char sDispCancelThisTran[2][30];
extern char sDispForcedOnline[2][50];
extern char sDispPAN[2][10];
extern char sDispSelect[2][10];
extern char sDispReferralCard[2][90];
extern char sDispReferralIssuer[2][50];
extern char sDispPINTryLimitExceeded[2][30];
extern char sDispFinishRemoveCard[2][30];
extern char sDispPleaseSelectLanguage[2][30];
extern char sDispEnter1Or2[2][15];
extern char sDispEnterAmountAuthorised[2][30];
extern char sDispEnterAmountOther[2][30];
extern char sDispPressAnyKeyContinue[2][30];
extern char sDispEnterPurchaseAmount[2][25];
extern char sDispEnterCashBackAmount[2][30];
extern char sDispEnterPINAgain[2][25];
extern char sDispIccInitErr[2][15];
extern char sDispCardholderConfirm[2][30];

typedef struct
{
    long tv_sec;
    long tv_usec;
}struct_timeval;

extern struct_timeval      g_key_start_tv; //记录最后一次按键的时间
extern struct_timeval      g_key_cur_tv;   //当前时间，用于和最后一次按键进行时差比较

extern unsigned char g_nCol,g_nRow;

extern int gnfd_rng;
extern uchar _gnRespCode[2];
extern uchar gbTimeout;
extern uchar gbPrintSignature;

//inter call
extern void RemoveTailChars(uchar *pszString, uchar ucRemove);
extern uchar MatchTrack2AndPan(uchar *pszTrack2, uchar *pszPan);
extern uchar GetCardFromTrack(uchar *szCardNo,uchar *track2,uchar *track3);
extern void  gettime(struct tm *tDate);





//extern void DebugTraceDisp(char *sDisp,uchar* sHex,int nHexlen);
extern void debug_printf(u8 col,u8 row,u8 mode,char *str,...);
extern void trace_debug_printf(char *str,...);
#ifdef DEBUG
	//#define TraceDisp(sMsg,sHex,nHexlen)		DebugTraceDisp(sMsg,sHex,nHexlen)
	#define TraceDisp		trace_debug_printf

#else
	#define TraceDisp		trace_debug_printf
#endif

