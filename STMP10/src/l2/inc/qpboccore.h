
#ifndef _QPBOCCORE_H
#define _QPBOCCORE_H

//#define STRUCT_PACK __attribute__((packed))

#define ushort unsigned short

int QPboc_CoreInit(void);

int QPboc_SetFinalSelectData(uchar *paucDtIn, int nDtLen);

int QPboc_SetTransData(T_Clss_TransParam *ptTransParam, T_Clss_PreProcInterInfo *ptInfo);

int QPboc_Proctrans(uchar *pucTransPath, uchar *pucACType);

int QPboc_CardAuth(uchar *pucACType, uchar *pucDDAFailFlg);

int QPboc_GetCvmType(uchar *pucType);

int QPboc_SetReaderParam(Contactless_ReaderParam *ptParam);
int QPboc_GetReaderParam(Contactless_ReaderParam *ptParam);


int  QPboc_GetTLVData(ushort Tag, uchar *DtOut, int *nDtLen);
int  QPboc_SetTLVData(ushort usTag, uchar *pucDtIn, int nDtLen);

int  QPboc_AddCAPK(T_EMV_TERM_CAPK  *capk );
//int  QPboc_GetCAPK(int Index, T_EMV_TERM_CAPK  *capk );
int  QPboc_GetCAPK(unsigned char KeyID, unsigned char *RID, T_EMV_TERM_CAPK  *capk );

int  QPboc_DelCAPK(unsigned char KeyID, unsigned char *RID);
void QPboc_DelAllCAPK(void);

int  QPboc_AddRevocList(T_EMV_REVOCATIONLIST *pRevocList);
int  QPboc_DelRevocList(unsigned char ucIndex, unsigned char *pucRID,unsigned char *CertiSerial);
void QPboc_DelAllRevocList(void);
int  QPboc_FindRevocList(unsigned char ucIndex, unsigned char *pucRID,unsigned char *CertiSerial);




int QPboc_nGetTrack2MapData(uchar *pTrackData, int *pLen);



#endif
