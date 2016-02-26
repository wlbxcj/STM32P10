
#ifndef _ENTRYCORE_H
#define _ENTRYCORE_H

#define STRUCT_PACK __attribute__((packed))

#pragma pack(1) 
// 每个AID对应的参数,for Pre-Processing
typedef struct Clss_PreProcInfo_Stru
{
	unsigned long ulTermFLmt;     //Terminal Floor Limit (Tag ‘9F1B
	unsigned long ulRdClssTxnLmt; //Terminal Contactless Transaction Limit
	unsigned long ulRdCVMLmt;     //Terminal CVM Required Limit
	unsigned long ulRdClssFLmt;   //Terminal Contactless Floor Limit

	unsigned char aucAID[16];       
	unsigned char ucAidLen; 
	
	unsigned char ucKernType; // 定义后续该AID 所遵循的内核类型

	// payWave
	unsigned char  ucCrypto17Flg;
	unsigned char   ucZeroAmtNoAllowed;       // 0-若授权金额=0,则TTQ要求联机密文, 1-若授权金额=0, 则内部qVSDC不支持标志置位
	unsigned char   ucStatusCheckFlg;    // 读卡器是否支持状态检查
    unsigned char   aucReaderTermTransQualifiers[4];      // 终端交易性能，用于VISA/PBOC中，tag =9F66
	
	// common
	unsigned char ucTermFLmtFlg; 
	unsigned char ucRdClssTxnLmtFlg; 
	unsigned char ucRdCVMLmtFlg;   
	unsigned char ucRdClssFLmtFlg; 	 
	
	unsigned char aucRFU[3];
}T_Clss_PreProcInfo;

#pragma pack() 



int Entry_CoreInit(void);

int Entry_SetPreProcInfo(T_Clss_PreProcInfo *ptPreProcInfoIn);
int Entry_PreTransProc(T_Clss_TransParam *pstTransParam);
int Entry_AppSlt(int nSlot, int nReadLogFlag);

int Entry_FinalSelect(uchar *pucKernType, uchar *pucDtOut, int *pnDtLen);

int Entry_GetPreProcInterFlg(T_Clss_PreProcInterInfo *ptInfo);

int Entry_GetFinalSelectData(uchar *paucDtOut, int *pnDtLen);


void Entry_DelAllAidList(void);
int Entry_AddAidList(uchar *pucAID, uchar ucAidLen, uchar ucSelFlg, uchar ucKernType); 
int Entry_DelAidList(uchar  *pucAID, uchar  ucAidLen);
int Entry_GetAidList(uchar *pucAID, uchar ucAidLen,uchar *pucSelFlg, uchar *pucKernType);
int Entry_GetAidListByNum(unsigned short nNum,uchar *pucAID, uchar *pucAidLen,uchar *pucSelFlg, uchar *pucKernType);

int Entry_DelCurCandApp(void);

void Entry_DelAllPreProcInfo(void);
int Entry_DelPreProcInfo(uchar  *pucAID, uchar  ucAidLen);
int Entry_GetPreProcInfo(uchar ucAidLen,uchar *sAID,T_Clss_PreProcInfo *ptPreProcInfoIn);

#endif
