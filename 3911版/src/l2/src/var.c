#include "var.h"
#include "struct.h"
#include "emvcore.h"
#include "entrycore.h"

//T_EMV_PARAMETER gtEmvParameter;

//T_CTRL_PARAM    gtCtrlParameter;
T_POS_PARAM_STRC gtPosParameter;

//POS_COM gtPosCom;
//T_TEMP_VAR gtTemp;
//int glbLanguage;


//非接触

        Contactless_ReaderParam ClssParam;
//	Clss_TransParam ClssEcParam;
//        PICC_PARA PiccPara;  
        
//	EMV_REVOCLIST EmvRevoclist;

//	T_Clss_PreProcInfo qvClssParam;
//	Clss_PreProcInterInfo ClssProcInterInfo;
//	Clss_VisaAidParam ClssVisaAidParam;
//	Clss_PbocAidParam ClssPbocAidParam;
//	Clss_SchemeID_Info ClssSchemeIDInfo;
//	uchar Mob_Pboc_Support,qpboc_stau;



/*

TERMAPPLIST Term_Applist[AppListStoredNum]={
		{"\xA0\x00\x00\x03\x33\x01\x01\x02",8,1,1,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x03\x33\x01\x01\x01",8,1,1,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x03\x33\x01\x01\x03",8,1,1,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x03\x33\x01\x01\x06",8,1,1,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x00\x03\x10\x10",7,0,1,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x00\x99\x90\x90",7,0,1,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x00\x03\x20\x10",7,0,1,100,1,100,1,100,1,100},   //?
		{"\xA0\x00\x00\x00\x04\x10\x10",7,1,1,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x00\x04\x30\x60",7,1,1,100,1,100,1,100,1,100},   //?
		{"\xA0\x00\x00\x00\x04\x60\x00",7,1,1,100,1,100,1,100,1,100},   //?
		{"\xA0\x00\x00\x00\x10\x10\x30",7,1,1,100,1,100,1,100,1,100},   //?
		{"\xA0\x00\x00\x00\x65\x10\x10",7,0,1,100,1,100,1,100,1,100}
};
*/

/* //move to local
TERMAPPLIST Term_Applist[AppListStoredNum]={

		{"\xA0\x00\x00\x03\x33\x01\x01",7,1,FULL_MATCH,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x00\x03\x10\x10",7,1,FULL_MATCH,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x00\x03\x10\x10\x03",8,1,FULL_MATCH,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x00\x03\x10\x10\x04",8,1,FULL_MATCH,50000,1,50000,1,50000,1,50000},
		{"\xA0\x00\x00\x00\x03\x10\x10\x05",8,0,FULL_MATCH,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x00\x03\x10\x10\x06",8,0,FULL_MATCH,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x00\x03\x10\x10\x07",8,0,FULL_MATCH,100,1,100,1,100,1,100}, 
		{"\xA0\x00\x00\x00\x99\x90\x90",7,1,FULL_MATCH,100,1,100,1,100,1,100},
		{"\xA0\x00\x00\x99\x99\x01",6,1,FULL_MATCH,100,1,100,1,100,1,100},   
		{"\xA0\x00\x00\x00\x04\x10\x10",7,1,FULL_MATCH,100,1,100,1,100,1,100},   
		{"\xA0\x00\x00\x00\x65\x10\x10",7,1,FULL_MATCH,100,1,100,1,100,1,100},   
                
};

*/
  
/* disable 2010/12/30
T_LANGUAGE gltLanguage[2]=
{
	{LOCALE, "zh", "中文"},
	{SUPPORT,"en","English"}
};


T_STANDMESS glTStandMess[2]=
{
	{
		"金额", 	"金额,正确?",   "确认",    "请与银行联系",    "取消或确认",    "卡错",   "拒绝",
        "输入金额", "输入PIN",       "密码错",           "插卡",   "不接受",  "密码正确",     "请等待",
		"处理错",               "拔卡",         "用读卡器",    "用磁卡",        "重试"
	},
	{
		"(AMOUNT)",	"(AMOUNT) OK?",	"APPROVED",	"CALL YOUR BANK","CANCEL OR ENTER",	"CARD ERROR","DECLINED",
		"ENTER AMOUNT",	"ENTER PIN","INCORRECT PIN", "INSERT CARD","NOT ACCEPTED",	"PIN OK",	"PLEASE WAIT",
		"PROCESSING ERROR",	"REMOVE CARD",	"USE CHIP READER","USE MAG STRIPE",	"TRY AGAIN"
	},
};
    
char sDispCancelThisTran[2][30]=
{
	{" 终止这交易"},	              //local
	{" Terminate this Transaction"},    //support
};

char sDispForcedOnline[2][50]=
{
	{"强制在线 1. 否  2. 是 选择 1 "},
	{"forced online 1. No  2. Yes Enter 1"},
};

char sDispPAN[2][10]=
{
	{"帐号:"},	
	{"PAN:"},
};


char sDispSelect[2][10]=
{
	{"选择:"},	
	{"Select:"},
};

char sDispReferralCard[2][90]=
{
	{"1.在线2.确认3.拒绝4.接受\n请选择\n"},
	{"1.go online 2.Approved 3.Declined 4.Accept\nEnter number\n"},
};

char sDispReferralIssuer[2][50]=
{
	{"1.批准 2.拒绝"},
	{"1.Approved 2.Declined\nEnter (1 or 2)"},
};

char sDispPINTryLimitExceeded[2][30]=
{
	{"PIN输入超过尝试次数"},
	{"PIN Try Limit exceeded"},
};

char sDispFinishRemoveCard[2][30]=
{
	{"成功,请拔卡\n"},
	{"Finish,Please Remove Card\n"},
};

char sDispPleaseSelectLanguage[2][30]=
{
	{"请选择语言"},
	{"Please Select language"},       //local
};

char sDispEnter1Or2[2][15]=
{
	{"选 1 或 2"},
	{"Enter 1 or 2"},	
};

char sDispEnterAmountAuthorised[2][30]=
{
	{"输入金额,授权"},
	{"Enter Amount, Authorised"},
};

char sDispEnterAmountOther[2][30]=
{
	{"输入金额,其他"},
	{"Enter Amount, Other"},
};

char sDispPressAnyKeyContinue[2][30]=
{
	{"按任意键,继续"},     //locale
	{"Press any key,continue"},     //local
};

char sDispEnterPurchaseAmount[2][25]=
{
	{"请输入消费金额"},	
	{"Enter Purchase Amount"},
};

char sDispEnterCashBackAmount[2][30]=
{
	{"请输入找钱金额"},
	{"Enter Cash Back Amount"},
};

char sDispEnterPINAgain[2][25]=
{
	{"重输密码:"},
	{" ENTER PIN AGAIN:"},
};

//char sDispIccInitErr[15]="卡初始化错";//"Icc Init Err";  //local

char sDispIccInitErr[2][15]=
{
	{"卡初始化错"},
	{"Icc Init Err"},   //local
};

//char sDispCardholderConfirm[30]="持卡者确认";//"Cardholder Confirm";


char sDispCardholderConfirm[2][30]=
{
	{"持卡者确认"},
	{"Cardholder Confirm"},
};

*/
struct_timeval      g_key_start_tv; //记录最后一次按键的时间
struct_timeval      g_key_cur_tv;   //当前时间，用于和最后一次按键进行时差比较

//lcd 行,列
unsigned char g_nCol,g_nRow;

int gnfd_rng;

uchar _gnRespCode[2] ={0};
uchar gbTimeout=5;
