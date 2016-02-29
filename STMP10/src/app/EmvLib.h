#ifndef _VEMV_H_
#define _VEMV_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*emv_lib.h*/
#define ENGLISH				0
#define CHINESE				1

#define TYPE_CASH			0x01	//交易类型(现金)
#define TYPE_GOODS			0x02	//交易类型(购物)
#define TYPE_SERVICE		0x04	//交易类型(服务)
#define TYPE_CASHBACK		0x08	//交易类型(反现)
#define TYPE_INQUIRY		0x10	//交易类型(查询)
#define TYPE_PAYMENT		0x20	//交易类型(支付)
#define TYPE_ADMINISTRATIVE	0x40	//交易类型(管理)
#define TYPE_TRANSFER		0x80	//交易类型(转账)

#define MAX_APP_NUM			32		//应用列表最多可存储的应用数
#define MAX_CAPK_NUM		64		//认证公钥表最多可存储的公钥数
#define MAX_CAPKREVOKE_NUM	96		//认证公钥撤回列表的最大个数32*3

#define PART_MATCH			0x00	//ASI(部分匹配)
#define FULL_MATCH			0x01	//ASI(完全匹配)

#define EMV_GET_POSENTRYMODE			0 
#define EMV_GET_BATCHCAPTUREINFO		1 
#define EMV_GET_ADVICESUPPORTINFO		2 

//交易处理函数返回码定义
#define EMV_OK              0      //成功  
#define ERR_EMVRSP         (-1)      //返回码错误
#define ERR_APPBLOCK       (-2)      //应用已锁
#define ERR_NOAPP          (-3)      //卡片里没有EMV应用
#define ERR_USERCANCEL     (-4)      //用户取消当前操作或交易
#define ERR_TIMEOUT        (-5)      //用户操作超时
#define ERR_EMVDATA        (-6)      //卡片数据错误
#define ERR_NOTACCEPT      (-7)      //交易不接受
#define ERR_EMVDENIAL      (-8)      //交易被拒绝
#define ERR_KEYEXP         (-9)      //密钥过期
#define ERR_NOPINPAD       (-10)     //没有密码键盘或键盘不可用
#define ERR_NOPIN          (-11)     //没有密码或用户忽略了密码输入
#define ERR_CAPKCHECKSUM   (-12)     //认证中心密钥校验和错误
#define ERR_NOTFOUND       (-13)     //没有找到指定的数据或元素
#define ERR_NODATA         (-14)     //指定的数据元素没有数据
#define ERR_OVERFLOW       (-15)     //内存溢出
#define ERR_NOTRANSLOG     (-16)     //无交易日志
#define ERR_NORECORD       (-17)     //无记录
#define ERR_NOLOGITEM      (-18)     //目志项目错误
#define ERR_ICCRESET       (-19)     //IC卡复位失败
#define ERR_ICCCMD         (-20)     //IC命令失败
#define ERR_ICCBLOCK       (-21)     //IC卡锁卡 
#define ERR_ICCNORECORD    (-22)     //IC卡无记录
#define ERR_GENAC1_6985    (-23)     //GEN AC命令返回6985
#define ERR_USECONTACT     (-24)     //非接失败，改用接触界面
#define ERR_APPEXP         (-25)     //qPBOC卡应用过期
#define ERR_BLACKLIST      (-26)     //qPBOC黑名单卡
#define ERR_GPORSP         (-27)     //err from GPO
//#define ERR_TRANSEXCEEDED  (-28)     //非接交易超限
#define ERR_TRANSEXCEEDED  (-29)     //非接交易超限
#define ERR_QPBOCFDDAFAIL  (-30)     //非接qPBOC fDDA失败

#define REFER_APPROVE		0x01	//参考返回码(选择批准)
#define REFER_DENIAL		0x02	//参考返回码(选择拒绝)
#define ONLINE_APPROVE		0x00	//联机返回码(联机批准)     
#define ONLINE_FAILED		0x01	//联机返回码(联机失败) 
#define ONLINE_REFER		0x02	//联机返回码(联机参考)
#define ONLINE_DENIAL		0x03	//联机返回码(联机拒绝)
#define ONLINE_ABORT		0x04	//兼容PBOC(交易终止)
#define ONLINE_REFERANDFAIL 0x05	//联机拒绝和联机参考


#define PATH_PBOC			0x00	//应用路径：标准PBOC
#define PATH_QPBOC			0x01	//应用路径：qPBOC
#define PATH_MSD			0x02	//应用路径：MSD
#define PATH_ECash			0x03	//应用路径：电子现金

//STRUCTOR DEF
typedef struct _EMV_APPLIST_{
    unsigned char AppName[33];       //本地应用名，以'\x00'结尾的字符串
    unsigned char AID[17];           //应用标志
    unsigned char AidLen;            //AID的长度
    unsigned char SelFlag;           //选择标志( 部分匹配/全匹配)      
    unsigned char Priority;          //优先级标志
    unsigned char TargetPer;         //目标百分比数
    unsigned char MaxTargetPer;      //最大目标百分比数
    unsigned char FloorLimitCheck;   //是否检查最低限额
    unsigned char RandTransSel;      //是否进行随机交易选择
    unsigned char VelocityCheck;     //是否进行频度检测
    unsigned long FloorLimit;        //最低限额
    unsigned long Threshold;         //阀值
    unsigned char TACDenial[6];      //终端行为代码(拒绝)
    unsigned char TACOnline[6];      //终端行为代码(联机)
    unsigned char TACDefault[6];     //终端行为代码(缺省)
    unsigned char AcquierId[7];      //收单行标志
    unsigned char dDOL[256];         //终端缺省DDOL
    unsigned char tDOL[256];         //终端缺省TDOL
    unsigned char Version[3];        //应用版本
    unsigned char RiskManData[10];   //风险管理数据
	unsigned char EC_bTermLimitCheck;      //是否支持终端交易限额
	unsigned long EC_TermLimit;            //终端交易限额，
	unsigned char CL_bStatusCheck;         //是否支持qPBOC状态检查
	unsigned long CL_FloorLimit;        //非接触终端最低限额
	unsigned long CL_TransLimit;        //非接触终端交易限额
	unsigned long CL_CVMLimit;          //非接触终端CVM限
	unsigned char TermQuali_byte2;      //交易金额与每个AID限额的判断结果，在刷卡前处理，通过此变量缓存判断结果
}EMV_APPLIST;

typedef struct {
    unsigned char RID[5];            //应用注册服务商ID
    unsigned char KeyID;             //密钥索引
    unsigned char HashInd;           //HASH算法标志
    unsigned char ArithInd;          //RSA算法标志
    unsigned char ModulLen;          //模长度
    unsigned char Modul[248];        //模
    unsigned char ExponentLen;       //指数长度
    unsigned char Exponent[3];       //指数
    unsigned char ExpDate[3];        //有效期(YYMMDD)
    unsigned char CheckSum[20];      //密钥校验和
}EMV_CAPK;

typedef struct{
    unsigned char MerchName[256];    //商户名
    unsigned char MerchCateCode[2];  //商户类别码(没要求可不设置)
    unsigned char MerchId[15];       //商户标志(商户号)
    unsigned char TermId[8];         //终端标志(POS号)
    unsigned char TerminalType;      //终端类型
    unsigned char Capability[3];     //终端性能
    unsigned char ExCapability[5];   //终端扩展性能
    unsigned char TransCurrExp;      //交易货币代码指数
    unsigned char ReferCurrExp;      //参考货币指数
    unsigned char ReferCurrCode[2];  //参考货币代码
    unsigned char CountryCode[2];    //终端国家代码
    unsigned char TransCurrCode[2];  //交易货币代码
    unsigned long ReferCurrCon;      //参考货币代码和交易代码的转换系数(交易货币对参考货币的汇率*1000)
    unsigned char TransType;         //当前交易类型 
    unsigned char ForceOnline;       //商户强制联机(1 表示总是联机交易)
    unsigned char GetDataPIN;        //密码检测前是否读重试次数
    unsigned char SupportPSESel;     //是否支持PSE选择方式
	unsigned char TermTransQuali[4]; //终端交易限制
	unsigned char TermIsSupEct;       //终端是否支持电子现金功能
	unsigned char ECTSI;             //电子现金终端支持指示器
	unsigned char EC_bTermLimitCheck;//是否支持终端交易限额
	unsigned long EC_TermLimit;		//终端交易限额，
	unsigned char CL_bStatusCheck;	//是否支持qPBOC状态检查
	unsigned long CL_FloorLimit;	//非接触终端最低限额
	unsigned long CL_TransLimit;	//非接触终端交易限额
	unsigned long CL_CVMLimit;		//非接触终端CVM限额
}EMV_PARAM;

//////////////////////////////////////////////////////////////////////////////////
//callback programe
int  cEmvLib_WaitAppSel(int TryCnt, EMV_APPLIST *List, int AppNum);  
int  cEmvLib_InputAmt(unsigned long *AuthAmt, unsigned long *CashBackAmt);
int  cEmvLib_GetHolderPwd(int TryFlag, int RemainCnt, unsigned char *pin);
int  cEmvLib_ReferProc(void);
int  cEmvLib_GetUnknowTLV(unsigned short Tag, unsigned char *dat, int len);
void cEmvLib_AdviceProc(void);
void cEmvLib_VerifyPINOK(void);
int  cEmvLib_VerifyCert(void);
int  cEmvLib_GetDateTime(unsigned char *datetime);
void cEmvLib_IoCtrl(unsigned char ioname,unsigned char *iovalue);
unsigned long  cEmvLib_GetAllAmt(unsigned char *PANData,int PANDataLen, unsigned long AuthAmt);
//////////////////////////////////////////////////////////////////////////////////
//EXPORT FUNCRION
void  Rsa_ItWell(unsigned char* Modul,unsigned int ModulLen,unsigned char* Exp,unsigned int ExpLen,unsigned char* DataIn,unsigned char* DataOut);
void  Hash(unsigned char* DataIn, unsigned long DataInLen, unsigned char* DataOut);

//int EmvLib_AppSelForLog(int Slot);
//int EmvLib_ReadLogRecord(int RecordNo);
//int EmvLib_GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen);

/*************************************************************************************
作    者: 刘福标
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: 设置对IC卡操作的卡类型
入口参数: Mode 0x01:内置IC卡  0x02:外置IC卡  0x3:内置PICC 0x04:外置PICC 	
返 回 值: 0x00 成功  0x01:Mode参数错误
其    它: 
备	  注:  
*************************************************************************************/
int EmvLib_SetIcCardType(int Mode);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：取emv库版本号
输入参数：
输出参数：无
返    回：int型版本号
备    注：
*************************************************************************************/
int  EmvLib_GetVer(void);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：读取EMV参数
输入参数：
		tParam:参数结构指针
输出参数：输入指针
返    回：无
备    注：
*************************************************************************************/
void EmvLib_GetParam(EMV_PARAM *tParam);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：设置EMV参数
输入参数：
		tParam:参数结构指针
输出参数：无
返    回：无
备    注：
*************************************************************************************/
void EmvLib_SetParam(EMV_PARAM *tParam);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：根据tag取得数据
输入参数：
		Tag:类别
		DataOut:数据指针
		OutLen:长度指针
输出参数：输入指针
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_GetTLV(unsigned short Tag, unsigned char *DataOut, int *OutLen);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：根据tag设置数据
输入参数：
		Tag:类别
		DataIn:数据指针
		DataLen:长度
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_SetTLV(unsigned short Tag, unsigned char *DataIn, int DataLen);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：取得脚本执行结果
输入参数：
		Result:执行结果指针
		RetLen:长度指针
输出参数：输入指针
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_GetScriptResult(unsigned char *Result, int *RetLen);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：增加支持的应用
输入参数：
		App:应用数据
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_AddApp(EMV_APPLIST *App);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：根据应用索引取得支持应用数据
输入参数：
		Index:应用索引
		App:应用数据
输出参数：输入指针
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_GetApp(int Index, EMV_APPLIST *App);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：删除支持应用
输入参数：
		AID:应用名称
		AidLen:
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_DelApp(unsigned char *AID, int AidLen);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：增加cap密钥
输入参数：
		capk:cap密钥数据结构指针
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_AddCapk(EMV_CAPK  *capk );

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：取得cap密钥
输入参数：
		capk:cap密钥数据结构指针
输出参数：输入指针
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_GetCapk(int Index, EMV_CAPK  *capk );

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：删除cap密钥
输入参数：
		KeyID:密钥序号
		RID:应用注册服务商ID
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_DelCapk(unsigned char KeyID, unsigned char *RID);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：检查cap 密钥是否有效
输入参数：
		KeyID:密钥序号
		RID:应用注册服务商ID
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_CheckCapk(unsigned char *KeyID, unsigned char *RID);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：应用选择
输入参数：
		Slot: 卡槽号
		TransNo:交易类型号
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_AppSel(int Slot,unsigned long TransNo);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：EMV卡片读取应用数据
输入参数：无
输出参数：库内全局结构
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_ReadAppData(void);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：EMV卡片认证
输入参数：无
输出参数：无
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_CardAuth(void);

/*************************************************************************************
作    者: 于祖苗
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: 交易完成后根据返回结果对卡进行完成操作
输入参数: 
	ucResult:如下取值
		REFER_APPROVE     0x01     //参考返回码(选择批准)
		REFER_DENIAL      0x02     //参考返回码(选择拒绝)
		ONLINE_APPROVE    0x00     //联机返回码(联机批准)
		ONLINE_FAILED     0x01     //联机返回码(联机失败)
		ONLINE_REFER      0x02     //联机返回码(联机参考)
		ONLINE_DENIAL     0x03     //联机返回码(联机拒绝)
		ONLINE_ABORT      0x04     //兼容PBOC(交易终止)
	RspCode: 后台返回错误码，=PosCom.stTrans.szRespCode
	AuthCode:后台返回授权码，=PosCom.stTrans.AuthCode
	AuthCodeLen:
	IAuthData:发卡方认证数据;
	IAuthDataLen:
	script:发卡方脚本
	ScriptLen:
输出参数：无 
返 回 值: 0=成功
备   注:  
*************************************************************************************/
int EmvLib_ProcTransComplete(unsigned char ucResult, unsigned char *RspCode, 
									 unsigned char *AuthCode, int AuthCodeLen, 
									 unsigned char *IAuthData, int IAuthDataLen, 
									 unsigned char *script, int ScriptLen);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：初始化EMV库
输入参数：无
输出参数：0
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_Init(void);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：EMV消费密码后处理
输入参数：无
		pStrMoney:金额串
		pPin:密码加密后串
		bIfGoOnline:是否联机。
输出参数：输入指针 bIfGoOnline
返    回：0=成功
备    注：
*************************************************************************************/
int EmvLib_ProcTrans(unsigned long nConsumeMoney, unsigned long nCashBackMoney, unsigned char *bIfGoOnline);

/*************************************************************************************
作    者：于祖苗
版    权：艾体威尔电子技术（北京）有限公司
功    能：取电子现金余额
输入参数：无
		BcdBalance:金额串
输出参数：输入指针 BcdBalance
返    回：0=成功 ，-1失败
备    注：
*************************************************************************************/
int  EmvLib_GetBalance(unsigned char* BcdBalance);


//
//qPBOC交易预处理
int EmvLib_qPBOCPreProcess(void);
//qPBOC交易处理
int EmvLib_ProcCLTrans(void);
//
int EmvLib_AppSelForLog(int Slot);


#ifdef __cplusplus
}
#endif



#endif

