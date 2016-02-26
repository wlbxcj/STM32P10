/*******************************************************************************
* File Name			 : contaceless_common.h
* Author             : guohonglv
* Date First Issued  : 2010/11/12
* Description        : 完全引用qpboctest项目的头文件，仅改动了结构体的语法
********************************************************************************/

#ifndef _CONTACTLESS_COMMON_H
#define _CONTACTLESS_COMMON_H

#define KERNTYPE_DEF   0
#define KERNTYPE_JCB   1	
#define KERNTYPE_MC    2
#define KERNTYPE_VIS   3
#define KERNTYPE_PBOC  4	
#define KERNTYPE_RFU 	 5


#define AC_AAC       0x00
#define AC_TC        0x01
#define AC_ARQC      0x02


//交易路径
#define CLSS_PATH_NORMAL      0
#define CLSS_VISA_MSD         1   // scheme_visa_msd_20
#define CLSS_VISA_QVSDC       2   // scheme_visa_wave3
#define CLSS_VISA_VSDC        3   // scheme_visa_full_vsdc
#define CLSS_VISA_CONTACT     4 
// 5~6: MCHIP, MAGSTRIPE      
#define CLSS_MC_MAG           5
#define CLSS_MC_MCHIP         6
#define CLSS_VISA_WAVE2       7   //SCHEME_VISA_WAVE_2 // 7   // SCHEME_VISA_WAVE_2 


//持卡人认证方式
#define RD_CVM_NO 						0x00
#define RD_CVM_SIG 						0x10
#define RD_CVM_ONLINE_PIN 		0x11
#define RD_CVM_OFFLINE_PIN 		0x12

#define CLSS_MAX_KEY_NUM       	7          //EMV库认证中心公钥表最多可存储的公钥数
#define CLSS_MAX_AIDLIST_NUM  	32

#define PART_MATCH  0
#define FULL_MATCH  1

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


#define CLSS_USE_CONTACT  			-24            // 必须使用其他界面进行交易
//#define CLSS_READDATA_ERR -25                // 读记录中出错,应终止交易并重新询卡
#define CLSS_TERMINATE    			-26            // 应终止交易       -25 
#define CLSS_FAILED       			-27            // 交易失败 20081217 
#define CONTACTLESS_PARAM_ERR    			-30            // -26 // 因EMV 内核中的参数错误定义为-30
#define CLSS_WAVE2_OVERSEA 			-31            // 20090418 for visa wave2 trans
#define CLSS_WAVE2_TERMINATED CLSS_TERMINATE //-32 // 20090421 for wave2 DDA response TLV format error
#define CLSS_WAVE2_US_CARD 			-33            // 20090418 for visa wave2 trans
#define CLSS_WAVE3_INS_CARD 		-34            // 20090427 FOR VISA L3
#define CLSS_RESELECT_APP   		-35
#define CLSS_CARD_EXPIRED       -36 // liuxl 20091104 for qPBOC spec updated

#pragma pack(1) 

// 每个AID对应的参数,用于交易预处理
typedef struct CLSS_PREPROC_INTER_FLAG_INFO
{
	uchar aucAID[17];       
	uchar ucAidLen; 
		
	// payWave
	uchar   ucZeroAmtFlg;       		// 0-交易金额!=0; 1-交易金额=0
	uchar   ucStatusCheckFlg;   		// 读卡器是否支持状态检查
    uchar   aucReaderTTQ[5];      // 终端交易性能，用于VISA/PBOC中，tag =9F66
	uchar   ucCLAppNotAllowed; 			// 1-该AID不能进行非接触交易
	
	// common
	uchar ucTermFLmtExceed; 
	uchar ucRdCLTxnLmtExceed; 
	uchar ucRdCVMLmtExceed;  
	uchar ucRdCLFLmtExceed;  

	uchar ucTermFLmtFlg;
	uchar aucTermFLmt[4];
	
	uchar aucRFU[2];
}T_Clss_PreProcInterInfo;

//	CLSS_TRANSPARAM 交易相关参数结构
typedef struct CLSS_TRANSPARAM
{
    unsigned long  ulAmntAuth;     // 授权金额(ulong)
    unsigned long  ulAmntOther;    // 其他金额(ulong) 
    unsigned long  ulTransNo;      // 交易序列计数器(4 BYTE)
	uchar  ucTransType;      // 交易类型 not  '9C' !!! (GOODS CASH)
	uchar  aucTransDate[4];  // 交易日期 YYMMDD
	uchar  aucTransTime[4];  // 交易时间 HHMMSS
}T_Clss_TransParam;

typedef struct  
{
	unsigned long   ulReferCurrCon;      	  // 参考货币代码和交易代码的转换系数(交易货币对参考货币的汇率*1000) //*
	unsigned int    usMchLocLen;          	// 商户名称及位置数据域的长度 
	uchar   aucMchNameLoc[50];   	// 商户名称及位置(1-256 字节) 257->50 2010/12/30
	uchar   aucMerchCatCode[2];   	// 商户分类码'9F15'(2字节)  
	uchar   aucMerchantID[15];    	// 商户标识(15字节)9F16  	
	uchar AcquirerId[6];       			//收单行标志 9F01  //* download app
  uchar   aucTmID[8];           	// 终端标识(终端号)9F1C  
	uchar   ucTmType;             	// 终端类型9F35 
	uchar   ucTransType;            // Goods,Services, Cash,CashBack  10/10/18
	uchar   aucTmCap[3];          	// 终端性能9F33 
  uchar   aucTmCapAd[5];        	// 终端附加性能 
  uchar   aucTmCntrCode [2];      // 终端国家代码 9F1A  
	uchar   aucTmTransCur[2];       // 终端交易货币代码'5F2A'(2字节)  
	uchar   ucTmTransCurExp;        // 终端交易货币指数'5F36'(1字节)  
	uchar   aucTmRefCurCode[2];     // 终端交易参考货币代码'9F3C'(2字节) 
	uchar   ucTmRefCurExp;	        // 终端交易参考货币指数'9F3D'(1字节) 
	uchar   aucRFU[3];
}Contactless_ReaderParam;

#pragma pack() 
#endif

