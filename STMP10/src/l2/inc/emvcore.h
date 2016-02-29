
#ifndef _EMV_CORE_H
#define _EMV_CORE_H

#define STRUCT_PACK __attribute__((packed))




#define PART_MATCH  0
#define FULL_MATCH  1

//交易处理函数返回码定义
#define EMV_OK             0         //成功

#define ICC_RESET_ERR     -1         //IC卡复位失败
#define ICC_CMD_ERR       -2         //IC命令失败
#define ICC_BLOCK         -3         //IC卡锁卡    
   
#define EMV_RSP_ERR       -4         //IC返回码错误
#define EMV_APP_BLOCK     -5         //应用已锁
#define EMV_NO_APP        -6         //卡片里没有EMV应用
#define EMV_USER_CANCEL   -7         //用户取消当前操作或交易
#define EMV_TIME_OUT      -8         //用户操作超时
#define EMV_DATA_ERR      -9         //卡片数据错误
#define EMV_NOT_ACCEPT    -10        //交易不接受
#define EMV_DENIAL        -11        //交易被拒绝
#define EMV_KEY_EXP       -12        //密钥过期


#define EMV_NO_PINPAD     -13        //没有密码键盘或键盘不可用
#define EMV_NO_PASSWORD   -14        //没有密码或用户忽略了密码输入
#define EMV_SUM_ERR       -15        //认证中心密钥校验和错误
#define EMV_NOT_FOUND     -16        //没有找到指定的数据或元素
#define EMV_NO_DATA       -17        //指定的数据元素没有数据
#define EMV_OVERFLOW      -18        //内存溢出

//回调函数或其他函数返回码定义
#define REFER_APPROVE     0x01       //参考返回码(选择批准)
#define REFER_DENIAL      0x02       //参考返回码(选择拒绝)

#define ONLINE_APPROVE    0x00       //联机返回码(联机批准)		
#define ONLINE_FAILED     0x01       //联机返回码(联机失败)	
#define ONLINE_REFER      0x02       //联机返回码(联机参考)
#define ONLINE_DENIAL     0x03       //联机返回码(联机拒绝)
#define ONLINE_ABORT      0x04       //兼容PBOC(交易终止)

//读交易日志
#define NO_TRANS_LOG      -19
#define RECORD_NOTEXIST   -20
#define LOGITEM_NOTEXIST  -21
#define ICC_RSP_6985      -22        // GAC中卡片回送6985, 由应用决定是否fallback
#define EMV_FILE_ERR      -23

//other
#define EMV_COMM_ERR      -50      

#define EMV_ONLINE         0xc8
//typedef unsigned char       uchar;
#define uchar unsigned char

#define DE55_MUST_SET	1	// 必须存在
#define DE55_OPT_SET	2	// 可选择存在
#define DE55_COND_SET	3	// 根据条件存在

#pragma pack(1) 

typedef struct T_EMV_APP_PARAMETER
{
	uchar LocalAppLabel[33];   //本地名称,以字符串存放
	uchar len;
	uchar aid[16];		       //应用标示
	uchar bSelectIndicator;    //选择标志(0xff:partial_MATCH 部分匹配,full_Match)	
	uchar Priority;            //优先级标志
	uchar TargetPercentage;    
	uchar MaxTargetPercentage;
	uchar sTermFloor[4]; 
    uchar sOptional;
/*
 bit 8           7           6             5        4                 3    2   1
    FloorLimit RandTransSel VelocityCheck OnlinePin Selectable Kernel
    check
*/    	
	unsigned int  ThresholdValue;
	uchar TACDefault[5]; //b  5
	uchar TACDenial[5];  //b  5
	uchar TACOnline[5];  //b  5
	uchar AcquirerID[6];  //收单行
	unsigned int DdolLen;
	uchar Ddol[252];             //b,move
	unsigned int TdolLen;
	uchar Tdol[252];             //b,move
	uchar TermAVM[2];  //application version number 
	uchar sTmp[20];  //risk data(8data) (len(1byte)+data) AmountX(9-12 Byte) AmountY(13-16Byte)
}T_EMV_APP_PARAMETER;

typedef struct T_EMV_TERM_CAPK
{
	uchar RID[5];
	uchar Index;
	uchar HashIndicator;
	uchar CAPKIndicator;
	uchar Len;
	uchar Modulus[248];
	uchar ExponentLen;
	uchar Exponent[ 3];
	uchar ExpDate[3]; //(yymmdd)
	uchar CheckSum[20];
	
}T_EMV_TERM_CAPK;

/*
at DICTCONTENT have
    Terminal Type:   
    Terminal Country Code
    Terminal Capabilities
    Additional Terminal Capabilities
    Point-of-Service (POS) Entry Mode
*/
typedef struct T_EMV_PARAMETER
{
	unsigned int MerchantNameLen;
	uchar MerchantNameLocation[256]; //?	
	uchar MerchantCode[2];
	uchar MerchantID[15];
	uchar TermID[8];
	uchar TermType;						//9f35 终端类型
	uchar TermCapa[3];                  
	uchar AddTermCapa[5];
	uchar TranRefCurrExp;				//9f3d 参考货币指数
	uchar TranRefCurr[2];				//9f3c 参考货币代码
	uchar TranCurrExp;					//5F36 交易货币代码指数
	uchar TranCurrCode[2];				//5F2A 交易货币代码
	unsigned int AmtReferRate;			//-     参考货币代码和交易代码的转换系数(交易货币对参考货币的汇率*1000)
	uchar TerminalCountry[2];
	uchar TransType; //
	uchar sOptions[10];
/*
 BIT 8          7             6                5              4              3                2                   1
BYTE PSE        Cardholder   Preferred        Partial AID    Multi          Common            RFU                RFU          
 1   Selection  Confirmation  Order           (Application)  Language       Character Set              
                                              Selection              
                                        
 BYTE revocation of  terminal PINByPass       GetDataPINTC   FAILCVM        Are amounts       RFU                RFU
  2   issuer public  contains                                               known before
     key certificate a default DDOL                                         CVM processing
      
 BYTE Floor      RandomTrans  Velocity        TransLog       Exception      TerminalRisk      RFU                RFU
  3   LimitCheck Select       Check                          file           Management
                                                                            BasedOnAip

 BYTE support process default skip/process    Detect CDA     request/never  request/never     RFU                RFU
  4     TAC   AC prior/after  TAC/IAC default Failure Before in 1st         in 2nd Generate
              1st GenAC    unable to go online TAAnalysis    GenerateAC(ARQC)  AC(TC)         
              
 BYTE ForceOnline Force      Advice         IssuerReferral           OnlineCapture/BatchCapture DefaultTDOL        RFU      
  5             Acceptance  
  
 BYTE equipped amount and PIN ICC/magstripe  magstripe      AccountType  issuer script      internate Date       RFU
  6   with     enter at the   reader         reader         Select       >128               management
      PINPAD   same keypad    combined       first
      
 BYTE FallBack   FallBack    FallBack
  7   not Select Terminate   1st GAC
  
 BYTE
  8
 
 BYTE
  9    PBOC    ETEC(Europay  VSDC(VISA ICC   VISANET      JCB(JCB EMV      AMEX(AMEX
               Terminal      Terminal                     specify)         Terminal
               Require)      specify)                                      Require)
 BYTE
  10
        
Terminal Capabilities include                                   
         Card Data Input Capabilities        
         CVM Capabilities
         Security Capabilities
Additional Terminal Capabilities include
		 Transaction type Capabilities           
		 Transaction type Capabilities           
		 Terminal Data input Capabilities
		 Terminal Data output Capabilities
		 Terminal Data output Capabilities
		 
*/
	
}T_EMV_PARAMETER;


typedef struct T_EMV_REVOCATIONLIST
{
	uchar RID[5];	
	uchar Index;
	uchar CertiSerial[3];
	uchar sRFU[1];
}T_EMV_REVOCATIONLIST; 

#pragma pack() 

extern int 	 EMV_KernelInit (void);
extern void	 EMV_KernelVersion(char *sVer);
extern void  EMV_SetParameter(T_EMV_PARAMETER *Param);
extern void  EMV_GetParameter(T_EMV_PARAMETER *Param);
extern int   EMV_GetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen);
extern int   EMV_SetTLVData(unsigned short Tag, unsigned char *DataIn, int DataLen);
extern int   EMV_ClearALLTLVData();
extern int   EMV_GetScriptResult(unsigned char *Result, int *RetLen);
	
extern int   EMV_AddApp(T_EMV_APP_PARAMETER *App);
extern int   EMV_GetApp(int Index, T_EMV_APP_PARAMETER *App);
extern int   EMV_DelApp(unsigned char *AID, int AidLen);
extern int   EMV_AddCAPK(T_EMV_TERM_CAPK  *capk );
extern int   EMV_GetCAPK(int Index, T_EMV_TERM_CAPK  *capk );
extern int   EMV_DelCAPK(unsigned char KeyID, unsigned char *RID);

extern int   EMV_DelAllCAPK(); //2011/01/07 add

extern int   EMV_CheckCAPK(unsigned char *KeyID, unsigned char *RID);
extern int   EMV_AddRevocList(T_EMV_REVOCATIONLIST *pRevocList);
extern int   EMV_DelRevocList(unsigned char ucIndex, unsigned char *pucRID,unsigned char *CertiSerial);
extern void  EMV_DelAllRevocList(void);
//exception file(black list)在应用中来检查

extern int   EMV_AppSelect(int Slot,unsigned long TransNo);
extern int   EMV_ReadAppData(void);
extern int   EMV_OfflineDataAuth(void);
extern int   EMV_ProcessRestrictions(void);
extern int   EMV_CardholderVerification(void);
extern int   EMV_TerminalRiskManagement(void);
extern int   EMV_TerminalAndCardActionAnalysis(void);
extern int   EMV_AppSelectForLog(int Slot); //pboc
//extern int  EMV_ReadCardLogRecord(int RecordNo);   //pboc
extern int   EMV_ReadCardLogRecord(int RecordNo,uchar *sFormat,uchar *sRecord);   //pboc,RecordNo>=1
extern int   EMV_OnlineProcessing();

//extern int  EMV_GetItemFromLog(unsigned short Tag, unsigned char *TagData, int *TagLen); //pboc

//call back
extern int   EMVMMI_MenuAppSel(int TryCnt, char *List[], int AppNum);
extern int   EMVMMI_InputAmount(unsigned char *AuthAmt, unsigned char *CashBackAmt);
extern int   EMVMMI_InputPIN(unsigned char TryFlag, int RemainCnt, unsigned char *pin);

extern int   EMVIF_CertVerify(void);
extern int   EMVIF_ReferProc(void);  //only for disp,ex boolean EmvIF_GetReferralResults(EMVLanguages *pelsLangs,boolean blRemove);
extern int   EMVIF_OnlineProc(unsigned char *RspCode, unsigned char *AuthCode, unsigned int *AuthCodeLen, unsigned char *IAuthData,unsigned int *IAuthDataLen, unsigned char *script71,unsigned int *ScriptLen71,unsigned char *script72,unsigned int *ScriptLen72);
extern void  EMVIF_AdviceProc(void);
extern void  EMVMMI_VerifyPINOK(unsigned char bFlag);
extern int   EMVIF_UnknowTLVData(unsigned short Tag, unsigned char *dat, int *pnLen);
extern void  EMVIF_ClearDsp(void);
extern int  EMVIF_DisplayWait(int line_num, int place, int clear_line_flag, unsigned char *str,int iTimeInt);
extern uchar EMVIF_SearchExceptionList(unsigned char bLen,unsigned char *rdPAN,unsigned char  *rdPANSeq);
//log.save at App.
extern uchar EMVIF_GetsumLogByPAN(unsigned char bLen,unsigned char *sPAN,unsigned int *nAmount);
extern void  EMVIF_GetRandNumber(unsigned int *prdRand);
extern void EMVIF_PrintSignature();

extern int EMVIF_FileSize(char *sName); 
//Transaction Sequence Counter(9f41)

#endif

