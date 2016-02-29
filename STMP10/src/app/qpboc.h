#ifndef  _QPBOC_H
#define  _QPBOC_H

#define PEDICCARD	0x01			//0x01:内置IC卡
#define EXICCARD	0x02			//外置IC卡
#define PEDPICCCARD	0x03			//内置PICC
#define EXPICCCARD	0x04			//外置PICC

// EMV AID参数列表
typedef  struct _tagTermAidList 
{
	u8	ucAidLen;				// AID的长度
	u8	sAID[17];				// 应用标志
	u8	bOnlinePin;				// CUP要求每个AID需要这个参数 CVM 加密PIN联机验证
	unsigned long EC_TermLimit; //EC 终端交易限额  gplian 11/06/28
}TERMAIDLIST;


extern char g_EmvFullSimple;			//是走EMV完整流程还是简易流程  0:完整流程   1: 简易流程 
extern EMV_PARAM	stEmvParam;
extern EMV_CAPK		gltCurCapk;			// 当前操作的CAPK元素
extern EMV_APPLIST  gltCurApp;			// 当前操作的APP参数
extern TERMAIDLIST  gltCurTermAid;		// 当前操作的终端AID参数(为了支持online pin)

/*************************************************************************************
作    者: 于祖苗
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: QPBOC快速支付
输入参数: 无
输出参数: 无
返 回 值: 0:成功  ESC:取消
备    注: 60秒没有操作自动超时退出	
*************************************************************************************/
int QPBOCSaleEx_Api(unsigned char *pAmt);

/*************************************************************************************
作    者: 于祖苗
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: QPBOC查余额
输入参数: 无
输出参数: 无
返 回 值: 0:成功  ESC:取消
备    注: 60秒没有操作自动超时退出	
*************************************************************************************/
int QpbocBalanceEx_Api(void);

/*************************************************************************************
作    者: 于祖苗
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: QPBOC查消费明细
输入参数: 无
输出参数: 无
返 回 值: 0:成功  ESC:取消
备    注: 60秒没有操作自动超时退出	
*************************************************************************************/
int QpbocDetailEx_Api(u8 *pBackData);


#endif

