#include "port.h"
#include "flashoperate.h"

#define MAX_FILE_M   7

#define TERMAPPFILE_ID_M    0
#define ENTRYAPPLIST_ID_M   1
#define PREPROCINFO_ID_M    2
#define QPBOCREVOCLIST_ID_M 3  
#define ENCKEY_ID_M         4
#define QPBOCEXCEPTION_M    5
#define EMVTERMCAPK_M	    6

#define QPBOCCAPK_OFFSET_M  0x630

/*
 name           size
termAppFile    (39+1)*50=2000    page 0
entryapplist  (19+1)*30=600      page 1           
preprocinfo   (47+1)*30=1440     page 1
qpbocrevoclist (9+1)*30=300      page 2
Key                          page 2-4
 
 
 page0: 00-03   "RUN"               
        04-11   record count(termAppFile)
		12-4BB  1200 record(1-30)
		4BC-4C3 record count(qbocexception)
		4C4-62B 360 record(1-30)
		62C-7BB 400 提示信息(20x20) 
        
 page1: 00-03  record count (entryapplist)
        04-25B 600 record(1-30)
        25C-25F record count(preprocinfo)
        260-7FF 1440 record(1-30)        
 page2: 00-03 record count(qpbocrevoclist)
        04-12f  300  record(1-30)

        130-15d 46 record count(key)
        15e-15f 2  RFU
        160-7ff 1696 record(key1-106)


page3:
      0-c5  46(198) 系统参数(2*20+6),the last is baud(00-10)
      c6-c7 record count(emvtermcapk)
      c8-7FF 1848 record(1-7)
page4:0-62F 1584 record(8-13)
      630-737 264 qpboc CAPK
      738-7FF  200 RFU 
         738-791  82+8 POSPARAM
         792-7FF  110 ReaderParam
// 未用
page5:
      0-737   792 record(14-16) emvtermcapk
      738-7FF 200 RFU 
      
// page3:00-7ff record(key107-234)


 page3:  
 	   3F0-421 50  系统参数(2*20+6),the last is baud(00-10)

        00-01   (emvtermcapk)
        02-     record(1-7)
//page4:00-7ff record(key236-363)

 page4:00-07 record count(qbocexception)
       08-25F 600 record(1-50)
 	   260-3EF 400 提示信息(20x20)
 	   3F0-421 50  系统参数(2*20+6),the last is baud(00-10)
        
        

*/
#define MAX_PAGE_NUM   5   //5->7 gplian  11/05/30

//#define StartAddr  ((u32)0x0807D800)// 从start->end 容量为10K
#define StartAddr  ((u32)0x00)// 

#define EndAddr    ((u32)0x08080000)//512 K top
//#define EndAddr    ((u32)0x08081000)//512 K top   gplian 11/05/30
 
//#define WriteProtection_Disable
#define FLASH_PAGE_SIZE   ((u16)0x800)//2K/a block

#pragma pack(1) 
typedef struct
{
  unsigned char sName[20];
  unsigned short nOffset;
  unsigned char bOffsetLen;
  unsigned short nRecordLen;
  unsigned char *psNumAttrib;
  unsigned char nRecordNumMax;
  unsigned char bPage;
}T_FILESTRU;
#pragma pack() 

static unsigned char gsNumAttrib0[8];
static unsigned char gsNumAttrib1[4];
static unsigned char gsNumAttrib2[4];
static unsigned char gsNumAttrib3[4];
static unsigned char gsNumAttrib4[48];
static unsigned char gsNumAttrib5[8];
static unsigned char gsNumAttrib6[2];


static T_FILESTRU gtFileStru[]=
{
  {"termAppFile",4,8,40,gsNumAttrib0,30,0},
  {"entryapplist",0,4,20,gsNumAttrib1,30,1},
  {"preprocinfo",0x25c,4,48,gsNumAttrib2,30,1},
  {"qpbocrevoclist",0,4,10,gsNumAttrib3,30,2},
  {"key",0x130,48,16,gsNumAttrib4,106,2}, //主密钥 传输密钥
  {"qpbocexception",0x4BC,8,12,gsNumAttrib5,30,0},
  {"emvtermcapk",0xc6,2,264,gsNumAttrib6,13,3},
  
};  

//static unsigned short gnRecordNumMax;

static char gbCurFile=-1;
//static unsigned char gbPage;
static unsigned int  gnOffset[MAX_FILE_M];
static int openfilesize();



unsigned char FlashFirstInit(void)
{
	unsigned char i,sBuf[100];
	//flash_read_operate(StartAddr,sBuf,3);
	user_flash_read_operate(StartAddr,sBuf,3);
#if 0        
        //test
        for(i=0;i<3;i++)
        {
           Lib_Beep();
           Lib_DelayMs(100);
        }
#endif        
	if(memcmp(sBuf,"RUN",3)) //first
	{
		//for(i=0;i<5;i++)
		for(i=0;i<MAX_PAGE_NUM;i++)
			//flash_erase_page(i);
			user_flash_erase_page(i);
		memcpy(sBuf,"RUN0",4);
		//flash_write_operate( StartAddr, (unsigned short*)sBuf, 4);
		user_flash_write_operate( StartAddr, (unsigned short*)sBuf, 4);
		return 1;
		
	}
	return 0;
}



int  read(int fd,void * buf ,int count)  
{
  int nSize,nBegin;
  unsigned char bPage;
  
  if( (fd<0) )
    return -1;

  gbCurFile = fd;
  bPage = gtFileStru[gbCurFile].bPage;

  nSize = openfilesize();
  nBegin = StartAddr+gtFileStru[gbCurFile].nOffset+gtFileStru[gbCurFile].bOffsetLen+bPage*2048;
  
  if( (gnOffset[gbCurFile]+count)>(nBegin+nSize) )
    return -1;
  //flash_read_operate(gnOffset[gbCurFile],buf,count);
  user_flash_read_operate(gnOffset[gbCurFile],buf,count);
  
  gnOffset[gbCurFile] +=count;
  return count;
}

static unsigned char CheckZeroBit(unsigned char bFlag)
{
	unsigned char i,bZeroNum=0;

	for(i=0;i<8;i++)
	{
		if( ((bFlag>>i) &0x01)==0 )
			bZeroNum++;
	}
	return bZeroNum;
}

static int openfilesize()
{
  int i,nSize;
  uchar sNumAttrib[100];

  memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
  
  nSize = 0;
  
  for(i=0;i<gtFileStru[gbCurFile].bOffsetLen;i++)
  {
      nSize+=CheckZeroBit(sNumAttrib[i]);
    
  }
  return nSize*gtFileStru[gbCurFile].nRecordLen;  
}

static void SetLastBit()
{
    
  int i,nSize;
  uchar sNumAttrib[100];

  memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
  
  nSize = 0;
  for(i=0;i<gtFileStru[gbCurFile].bOffsetLen;i++)
  {
      nSize+=CheckZeroBit(sNumAttrib[i]);
    
  }
  i =  (nSize-1)/8;
  
  sNumAttrib[i] |= 0x01 << ( (nSize-1)%8);

  memcpy(gtFileStru[gbCurFile].psNumAttrib,sNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
}

static void SetLastWriteBit()
{
    
  int i,nSize;
  uchar sNumAttrib[100];

  memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
  
  nSize = 0;
  for(i=0;i<gtFileStru[gbCurFile].bOffsetLen;i++)
  {
      nSize+=CheckZeroBit(sNumAttrib[i]);
    
  }
  i =  (nSize+1)/8;
  
  sNumAttrib[i] &= ~(0x01 << ( (nSize+1)%8));
  memcpy(gtFileStru[gbCurFile].psNumAttrib,sNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
  
}

static unsigned char CheckWriteBit(unsigned char bRecNum)
{
	unsigned char i,j;
	uchar sNumAttrib[100];

  	memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
	
	i = bRecNum /8;
	j = bRecNum %8;
	if(sNumAttrib[i] & (0x01 << ( j)))
		return 0;   //可写
	return 1;
}

static unsigned char SetWriteBit(unsigned char bRecNum)
{
	unsigned char i,j;
	uchar sNumAttrib[100];

  	memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
	
	i = bRecNum /8;
	j = bRecNum %8;
	sNumAttrib[i] &= ~(0x01 << ( j));

	memcpy(gtFileStru[gbCurFile].psNumAttrib,sNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
	
	return 0;
}

int stat(const char * file_name,struct stat *buf)
{
    //int nBegin; //never be used //guohonglv 2011/02/17
    //假定以前已open!
    if(gbCurFile < 0)
        return -1;

    buf->st_size = openfilesize();

    return 0;
}

int  write (int fd,const void * buf,int count) //在linux下，原函数是：ssize_t write (int fd,const void * buf,size_t count)
 {
   
   int nSize,nBegin;
   unsigned char sPageBuf[2048];
   unsigned short nRecordNumMax,nRecNum;
   unsigned char bPage;

   
   if( (fd<0)  )
    return -1;

   gbCurFile = fd;
   bPage = gtFileStru[gbCurFile].bPage;
   
   nRecordNumMax = gtFileStru[gbCurFile].nRecordNumMax;
   
   nSize = gtFileStru[gbCurFile].nRecordLen*nRecordNumMax;
   nBegin = StartAddr+gtFileStru[gbCurFile].nOffset+gtFileStru[gbCurFile].bOffsetLen+bPage*2048;
   if( (gnOffset[gbCurFile]-nBegin+count)>nSize)
     return -1;
   //check erase page
   nRecNum = (gnOffset[gbCurFile]-nBegin)/gtFileStru[gbCurFile].nRecordLen;
   //if(CheckWriteBit(nRecNum))  //不可直接写，要erase
   //全部直接写
   {
	    SetWriteBit(nRecNum);
        //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        
        //flash_erase_page(bPage);
        user_flash_erase_page(bPage);
		nBegin = gnOffset[gbCurFile]-StartAddr-(bPage*FLASH_PAGE_SIZE);
		
		memcpy(&sPageBuf[gtFileStru[gbCurFile].nOffset],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
		if( (nBegin+1)>FLASH_PAGE_SIZE) //ex key save page 2 and 3
		{
			//flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);
			user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);

			bPage += (nBegin+1)/FLASH_PAGE_SIZE;
    	    //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    	    user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
	        //flash_erase_page(bPage);
	        user_flash_erase_page(bPage);
			
			memcpy(&sPageBuf[nBegin%FLASH_PAGE_SIZE],buf,count);			
			//flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);
			user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);
			
		}
		else
		{
			memcpy(&sPageBuf[nBegin],buf,count);
			//flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);
			user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE, (unsigned short*)sPageBuf, FLASH_PAGE_SIZE);
		}
   }
   /*
   else
   {
	   SetWriteBit(nRecNum);
   	   nBegin = StartAddr+bPage*FLASH_PAGE_SIZE+gtFileStru[gbCurFile].nOffset;
	   flash_write_operate_nocheckcomplete(nBegin,(unsigned short*)gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
	   flash_write_operate(gnOffset[gbCurFile],(unsigned short*)buf,count);
	   
   }
   */
   gnOffset[gbCurFile] +=count;

   return count;
   
 }

int creat(const char * pathname, int mode_tmode) //在disp.c里面有引用 函数原型是：int creat(const char * pathname, mode_tmode) 
{
  return 1;
}

int close(int fd) //在disp.c 里面有引用
{
  if(fd<0)
    return 0;
  gbCurFile = -1;
  
  return 0;
}

int open( const char * pathname, int flags) ////在disp.c 里面有引用
{
  unsigned char i;
  char *pPath;
  char sName[200];
  for(i=0;i<MAX_FILE_M;i++)
  {
#ifdef FOR_LINUX_SYS    
    strcpy(sName,"/");
    strcat(sName,pathname);
    pPath = strrchr( sName,'/');
#else
    strcpy(sName,"\\"); 
    strcat(sName,pathname);
    //pPath = sName;
    pPath =strrchr( sName,'\\');
#endif
    
    if(strcmp(&pPath[1], (char *)gtFileStru[i].sName)==0)
    {
      gbCurFile = i;
      switch(gbCurFile)
      {
        case 0: 
          //gbPage = 0;
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          break;
        case 1:
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          //gbPage = 1;
          break;
        case 2:
          //gbPage = 1;
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          break;
        case 3:
          //gbPage = 2;
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          break;
        case 4:
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //gbPage = 2;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          //gnRecordNumMax = 234;//362->234
          break;
		case 5: //except file
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //gbPage = 4;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          //gnRecordNumMax = 50;
		  break;
		case 6: //term capk
          gnOffset[gbCurFile] = StartAddr+gtFileStru[i].nOffset+gtFileStru[i].bPage*2048;
          //gbPage = 3;
          //flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          user_flash_read_operate(gnOffset[gbCurFile],gtFileStru[gbCurFile].psNumAttrib,gtFileStru[i].bOffsetLen);
          gnOffset[gbCurFile] +=gtFileStru[i].bOffsetLen;
          //gnRecordNumMax = 50;
		  break;
		
		
      }
      break;
    }
  }
                     
  if(i==MAX_FILE_M)
    return -1;
  return gbCurFile;
}

//int open2( const char * pathname,int flags, int mode_tmode)  //在disp.c 里面有引用, linux的原型是：int open( const char * pathname,int flags, mode_t mode);
//{
//  return 1; //IAR不支持多态？？
//}

int lseek(int fildes,int offset ,int whence)//disp.c里面有应用，函数的原型是：off_t lseek(int fildes,off_t offset ,int whence); 
{
  int nSize,nBegin;
  unsigned char bPage;
  if(fildes<0)
    return fildes;
 // if(gbCurFile<0)
  //  return gbCurFile;
  gbCurFile = fildes;  
  bPage = gtFileStru[gbCurFile].bPage;

  nSize = gtFileStru[gbCurFile].nRecordLen*gtFileStru[gbCurFile].nRecordNumMax;
  nBegin = StartAddr+gtFileStru[gbCurFile].nOffset+gtFileStru[gbCurFile].bOffsetLen+bPage*2048;
  if(whence==SEEK_CUR)
  {
    if( (gnOffset[gbCurFile]+offset)> (nBegin+nSize) )
      return -1;
    gnOffset[gbCurFile] += offset;
  }
  else if(whence==SEEK_SET)
  {
    if(   offset> nSize)
      return -1;
    gnOffset[gbCurFile] = nBegin+offset;
  }
  else if(whence==SEEK_END)
  {
    
    if( gnOffset[gbCurFile]>(nBegin+nSize) )
      return -1;
    gnOffset[gbCurFile] = nBegin + openfilesize()-offset;
	
  }
  return 0;
}


int ftruncate (int __fd, unsigned int __length)
{
  unsigned char sPageBuf[2048];
  unsigned int nSize;
  unsigned char bPage;
  unsigned char sNumAttrib[100];
  
  if(__fd<0 )
  	return -1;
  gbCurFile = __fd;
  bPage = gtFileStru[gbCurFile].bPage;

   memcpy(sNumAttrib,gtFileStru[gbCurFile].psNumAttrib,gtFileStru[gbCurFile].bOffsetLen);

   if(__length==0)
   {
     switch(gbCurFile)
     {
	 	/*
       case  ENTRYAPPLIST_ID_M:
       case  QPBOCREVOCLIST_ID_M:
         flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         flash_erase_page(bPage);
         flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE+gtFileStru[gbCurFile].nOffset,(unsigned short*)&sPageBuf[gtFileStru[gbCurFile].nOffset],FLASH_PAGE_SIZE-gtFileStru[gbCurFile].nOffset);
         break;
       case PREPROCINFO_ID_M:
         flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         flash_erase_page(bPage);
         nSize = gtFileStru[gbCurFile].nRecordLen*gtFileStru[gbCurFile].nRecordNumMax + gtFileStru[gbCurFile].bOffsetLen;
         flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(unsigned short*)&sPageBuf[0],FLASH_PAGE_SIZE-nSize);
         break;
		*/ 
       case PREPROCINFO_ID_M:
       case  ENTRYAPPLIST_ID_M:
       case  QPBOCREVOCLIST_ID_M:
	   case QPBOCEXCEPTION_M: //save leave 
         //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         //flash_erase_page(bPage);
         user_flash_erase_page(bPage);

		 //11/06/03
         nSize = gtFileStru[gbCurFile].nRecordLen*gtFileStru[gbCurFile].nRecordNumMax + gtFileStru[gbCurFile].bOffsetLen;
		 memset(&sPageBuf[gtFileStru[gbCurFile].nOffset],0xff,nSize);
		 
		 //flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
		 user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
   
	     break;
	   case EMVTERMCAPK_M: //at page3 and page4
         //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         //flash_erase_page(bPage);
         user_flash_erase_page(bPage);
         //nSize = gtFileStru[gbCurFile].bRecordLen*gtFileStru[gbCurFile].nRecordNumMax + gtFileStru[gbCurFile].bOffsetLen;
         //flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(unsigned short*)&sPageBuf[0],gtFileStru[gbCurFile].nOffset);
         user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(unsigned short*)&sPageBuf[0],gtFileStru[gbCurFile].nOffset);

		 bPage++;
         //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
         //flash_erase_page(bPage);
         user_flash_erase_page(bPage);
         //nSize = gtFileStru[gbCurFile].bRecordLen*gtFileStru[gbCurFile].nRecordNumMax + gtFileStru[gbCurFile].bOffsetLen;
         //flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE+QPBOCCAPK_OFFSET_M,(unsigned short*)&sPageBuf[QPBOCCAPK_OFFSET_M],FLASH_PAGE_SIZE-QPBOCCAPK_OFFSET_M);
         user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE+QPBOCCAPK_OFFSET_M,(unsigned short*)&sPageBuf[QPBOCCAPK_OFFSET_M],FLASH_PAGE_SIZE-QPBOCCAPK_OFFSET_M);
	   	
	   	break;
       default:
         break;      
     }
   }
   else
   {
     //flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
     user_flash_read_operate(StartAddr+bPage*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
     SetLastBit();
     //flash_erase_page(bPage);
     user_flash_erase_page(bPage);
     memcpy(&sPageBuf[gtFileStru[gbCurFile].nOffset],sNumAttrib,gtFileStru[gbCurFile].bOffsetLen);
     //rm the last record,disable,now only rm size
     // nSize = openfilesize()+gtFileStru[gbCurFile].nOffset+gtFileStru[gbCurFile].bOffsetLen;
     //memset(&sPageBuf[nSize],0xff,gtFileStru[gbCurFile].bRecordLen);
	 
     //flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(u16*)sPageBuf,FLASH_PAGE_SIZE); //guohonglv 进行了强制类型转换
     user_flash_write_operate(StartAddr+bPage*FLASH_PAGE_SIZE,(u16*)sPageBuf,FLASH_PAGE_SIZE); //guohonglv 进行了强制类型转换
   }
   return 0;
}
                     
uchar FlashWriteParam(char *pIndexStr,uchar *pBuf,unsigned short nLen)
{
	#define POSPARAM_OFFSET_M     0x738
	#define READERPARAM_OFFSET_M  0x792
	unsigned char sPageBuf[2048];

	if(strcmp(pIndexStr,"posparam")==0)
	{
        //flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        user_flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        //flash_erase_page(4);
        user_flash_erase_page(4);
    	memcpy(&sPageBuf[POSPARAM_OFFSET_M],(unsigned char *)pBuf,nLen);
    	//return flash_write_operate(StartAddr+4*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
    	return user_flash_write_operate(StartAddr+4*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
		
	}
	else if(strcmp(pIndexStr,"readerparam")==0)
	{
        //flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        user_flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
        //flash_erase_page(4);
        user_flash_erase_page(4);
    	memcpy(&sPageBuf[READERPARAM_OFFSET_M],(unsigned char *)pBuf,nLen);
    	//return flash_write_operate(StartAddr+4*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
    	return user_flash_write_operate(StartAddr+4*FLASH_PAGE_SIZE,(unsigned short*)sPageBuf,FLASH_PAGE_SIZE);
	}

    return 0;
}

uchar FlashReadParam(char *pIndexStr,uchar *pBuf,unsigned short nLen)
{
	#define POSPARAM_OFFSET_M     0x738
	#define READERPARAM_OFFSET_M  0x792
	//unsigned char sPageBuf[2048];
	
	if(strcmp(pIndexStr,"posparam")==0)
	{
		//return flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE+POSPARAM_OFFSET_M,(unsigned char *)pBuf,nLen);
		return user_flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE+POSPARAM_OFFSET_M,(unsigned char *)pBuf,nLen);

	}
	else if(strcmp(pIndexStr,"readerparam")==0)
	{
		//return flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE+READERPARAM_OFFSET_M,(unsigned char *)pBuf,nLen);
		return user_flash_read_operate(StartAddr+4*FLASH_PAGE_SIZE+READERPARAM_OFFSET_M,(unsigned char *)pBuf,nLen);

	}

    return 0;
}

  
