
#include "var.h"
#include "stdio.h"

//typedef unsigned short  ushort;
#define ushort unsigned short
extern void lcdshow( int line_num, int place, int clear_line_flag, unsigned char *str );

#if 0
//从文件中读取保存的冲正记录到全局变量,
uchar ReadReversalData(void)
{
	int		fd;

	fd = open(REVERSAL_FILE, O_RDWR);
	if( fd<0 )
	{
		return E_MEM_ERR;
	}

	if( read(fd, (uchar *)&gtPosCom, COM_SIZE)!=COM_SIZE )
	{
		close(fd);
		return E_MEM_ERR;
	}
	close(fd);

	return OK_MIS;
}

//保存冲正记录需要的全局变量到文件中
uchar SaveReversalData(void)
{
	int fd;
	int len;

	fd = open(REVERSAL_FILE,  O_RDWR);
	if( fd<0 )
	{
		//creat(REVERSAL_FILE,0777);
		fd = open(REVERSAL_FILE, O_RDWR | O_CREAT | O_SYNC);
		if(fd<0)
			return (E_MEM_ERR);
	}

	len = write(fd, (uchar *) &gtPosCom, COM_SIZE);
	if( len!=COM_SIZE )
	{
		close(fd);
		return (E_MEM_ERR);
	}
	close(fd);

	return (OK_MIS);
}

// called only if EMVProcTrans() failed,kernel的冲正记录tag与实际上APP(如BankUnion, JCB)不一样,所以APP中自己生成
uchar UpdateEmvDupFile(void)
{
	uchar	ucRet;
	int		iLength;
	uchar	sReversalDE55[219];
	extern T_EMVTAG gtReversalTagList[];
	
	// 保存脚本处理结果
	SaveScriptResult();
	
	// Issuer approved, but ICC declined!
	if( fexist(REVERSAL_FILE)<0 )
	{
		return OK_MIS;
	}

	memset(sReversalDE55, 0, sizeof(sReversalDE55));
	sReversalDE55[0] = 6;
	ucRet = SetOnlineData(gtReversalTagList, sReversalDE55, &iLength);
	if( ucRet!=OK_MIS )
	{
		return E_TRANS_FAIL;
	}

	ReadReversalData();
	memcpy(gtPosCom.sReversalDE55, sReversalDE55, iLength);
	gtPosCom.iReversalDE55Len = iLength;
	if( gtPosCom.szRespCode[0]==0 || memcmp(gtPosCom.szRespCode, "00", 2)==0 )
	{
		sprintf((char *)gtPosCom.szRespCode, "06"); 
	}
	SaveReversalData();

	return OK_MIS;
}

#endif

// 只处理基本数据元素Tag,不包括结构/模板类的Tag
void BuildTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData)
{
	uchar	*psTemp;

	// 忽略长度为0的TAG
	if( iLength<=0 )
	{
		return;
	}

	// 设置TAG
	psTemp = *ppsOutData;
	if( uiEmvTag & 0xFF00 )
	{
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	*psTemp++ = (uchar)uiEmvTag;
	
	// 设置Length
	if( iLength<=127 )	// 目前数据长度均小余127字节,但仍按标准进行处理
	{
		*psTemp++ = (uchar)iLength;
	}
	else
	{	// EMV规定最多255字节的数据
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}
	
	// 设置Value
	memcpy(psTemp, psData, iLength);
	psTemp += iLength;
	
	*ppsOutData = psTemp;
}

//port



/* YYYYMMDD HHMMSS*/
void settimebybuf (unsigned char *sDate)
{
	#pragma pack(1) 
	typedef struct
	{
		uchar sYear[2];
		uchar sMon[2];
		uchar sDay[2];
		uchar sHour[2];
		uchar sMin[2];
		uchar sSec[2];

	}T_PART;
	#pragma pack() 
	//T_PART tPart;
	//struct tm sTime;

	//memcpy(&tPart,&sDate[2],
	//sTime.tm_year = 
}


unsigned char DispDateTime()
{
	struct tm  timestru;
	short offset;
	char sDisp[100]; 

	gettime(&timestru);
	//gotoxy(0,5);
	sprintf(sDisp,"%02d:%02d:%02d",timestru.tm_hour,timestru.tm_min,timestru.tm_sec);
	offset = strlen(sDisp)<MAX_COL_M?(MAX_COL_M-strlen(sDisp))/2:0;
	
	//gotoxy(offset,gety());
	//printf("%s\n",sDisp);
	
	lcdshow(1,MIDDLE,1, (unsigned char *)sDisp);
	
	memset(sDisp,0,50);
//	if(timestru.tm_year>49)
//		sprintf(sDisp,"19%02d %02d %02d",timestru.tm_year,timestru.tm_mon,timestru.tm_mday);
//	else
		sprintf(sDisp,"20%02d %02d %02d",timestru.tm_year,timestru.tm_mon,timestru.tm_mday);
	offset = strlen(sDisp)<MAX_COL_M?(MAX_COL_M-strlen(sDisp))/2:0;
	
	//gotoxy(offset,gety());
	//printf(sDisp);

	lcdshow(2,MIDDLE,1, (unsigned char *)sDisp);
    (void)offset;
    return 0;
}



