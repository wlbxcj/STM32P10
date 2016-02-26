#include "KF701DH.h"
#include "stm32f10x_lib.h"
#include "..\..\inc\FunctionList.h"

#include <string.h>
#include "vosapi.h"
#include "base.h"
#include "..\..\inc\FunctionList.h"

#include "..\pci\pci.h"

#include "FlashOperate.h"

#define MY_MAX_FILE_M   5

typedef struct //__attribute__ ((__packed__))
{
     char file_name[16];
     WORD StartBlock;
     long size;
}MY_FILE_ITEM;

static MY_FILE_ITEM k_Flist[5]=
{
  {SYSKEY_FILE,SYSKEY_FILE_ADDR,SYSKEY_FILE_SIZE},
  {AUTHKEY_FILE,AUTHKEY_FILE_ADDR,AUTHKEY_FILE_SIZE},
  {DUKPTKEY_FILE,DUKPTKEY_FILE_ADDR,DUKPTKEY_FILE_SIZE},
  {MDK_FILE,MDK_FILE_ADDR,MDK_FILE_SIZE},
  {"",0,0},
};

static unsigned int  gnOffset[5];

int  my_open(char *filename, BYTE mode, BYTE *attr)
{
  BYTE i;
  
#if 0  
  char sDisp[100];
  //test

  int nTmp;
  
  strcpy(k_Flist[0].file_name,SYSKEY_FILE);
  k_Flist[0].StartBlock = SYSKEY_FILE_ADDR;
  k_Flist[0].size = SYSKEY_FILE_SIZE;
#endif
  
  for(i=0;i<MY_MAX_FILE_M;i++)
  {
#if 0    
    strcpy(sDisp,k_Flist[i].file_name);
    
    nTmp = k_Flist[i].StartBlock;
    nTmp = k_Flist[i].size;
#endif    
    if(strcmp(filename,k_Flist[i].file_name)==0)
    //if(strcmp(filename,sDisp)==0)
    {
      break;
      
    }
  }
  if(i==MY_MAX_FILE_M)
    return -1;
  gnOffset[i] = k_Flist[i].StartBlock;
  return i;
}

int  my_FileClose(int fd)
{
  if(fd<0)
    return fd;
  return 0;
}

int  my_FileSeek(int fd, long offset, BYTE origin)
{
  int nSize,nBegin;
  
  if(fd<0)
    return fd;
  nBegin = k_Flist[fd].StartBlock;
  nSize = k_Flist[fd].size;
  
  if(origin==FILE_SEEK_CUR)
  {
    if( (gnOffset[fd]+offset)> (nBegin+nSize) )
      return -1;
    gnOffset[fd] += offset;
  }
  else if(origin==FILE_SEEK_SET)
  {
    if(   offset> nSize)
      return -1;
    gnOffset[fd] = nBegin+offset;
  }
  else if(origin==FILE_SEEK_END)
  {
    
    if( gnOffset[fd]>(nBegin+nSize) )
      return -1;
    //当前大小没保存，会有问题，现没用FILE_SEEK_END！！！
    //gnOffset[fd] = nBegin + openfilesize()-offset;
    gnOffset[fd] = nBegin + nSize-offset;
	
  }
  return 0;  
  
}

//13/07/09
int _CopyFromBack(unsigned int nAddr)
{
  u32 bBeginPage;
  u8  bRet,sPageBuf[2048];
#define FLASH_PAGE_SIZE   ((u16)0x800)//2K/a block
  //test
  trace_debug_printf("CopyFromBack[%x]H\n",nAddr);
  
  bBeginPage = (nAddr+SysStartAddr)/FLASH_PAGE_SIZE;
  //read from back
  bRet = flash_read_operate(bBeginPage*FLASH_PAGE_SIZE+PCI_BACK_ADDR,sPageBuf,FLASH_PAGE_SIZE);
  flash_erase_page(bBeginPage);
  if(bBeginPage==(SysStartAddr/FLASH_PAGE_SIZE))
    flash_read_operate(bBeginPage*FLASH_PAGE_SIZE,sPageBuf,PCI_PIN_MKEY_ADDR);
  bRet = CrcPageCalc(bBeginPage-SysStartAddr/FLASH_PAGE_SIZE,sPageBuf);
  //write local
  bRet = flash_write_operate(bBeginPage*FLASH_PAGE_SIZE,(unsigned short *)sPageBuf,FLASH_PAGE_SIZE);
  return bRet;   
}



int my_FileRead(int fd, BYTE *dat, int len)
{
  int nRet;
  if(fd<0)
    return fd;
  if((gnOffset[fd]+len- k_Flist[fd].StartBlock)>k_Flist[fd].size)
    return -1;
  sys_flash_read_operate(gnOffset[fd],dat,len);
  
  //13/07/09
  if( (gnOffset[fd] >=PCI_PIN_MKEY_ADDR_PAGE0) && (gnOffset[fd]<PCI_BACK_ADDR) &&
     (len==32) )
  {
    
    if( (nRet=_s_CheckPinKey(dat))!=0)
    //if(memcmp(dat,"\x0\x0\x0\x0\x0\x0\x0\x0",8)==0)
    {
      //test
      trace_debug_printf("find %i key  fail[%d]\n",dat[1],nRet);
      
      sys_flash_read_operate(gnOffset[fd]+PCI_BACK_ADDR,dat,len);
      
      //if(memcmp(dat,"\x0\x0\x0\x0\x0\x0\x0\x0",8)!=0)
      if(_s_CheckPinKey(dat)==0)
        _CopyFromBack(gnOffset[fd]); 
    }
  }
  gnOffset[fd]+=len;
  return len;
 
}

int  my_FileWrite(int fd, BYTE *dat, int len)
{
  int nRet;
  //liantest
  int i;
  unsigned char sTemp[200];
  
  if(fd<0)
    return fd;
  if((gnOffset[fd]+len- k_Flist[fd].StartBlock)>k_Flist[fd].size)
    return -1;
  nRet = sys_flash_write_operate(gnOffset[fd],(unsigned short *)dat,len);

  //13/05/27
  if(len<200)
  {
      sys_flash_read_operate(gnOffset[fd],sTemp,len);
      
      if(memcmp(dat,sTemp,len))
      {
        //liantest
        trace_debug_printf("sys_flash_write_operate[%d]\n",nRet);
        trace_debug_printf("gnOffset[%d]\n",gnOffset[fd]);
        trace_debug_printf("in  data:");
        for(i=0;i<len;i++)
          trace_debug_printf("%02x ",dat[i]);
        trace_debug_printf("\n");
        
        trace_debug_printf("out data:");
        for(i=0;i<len;i++)
          trace_debug_printf("%02x ",sTemp[i]);
        trace_debug_printf("\n");
        
        return -2;
      }       
  }
  
#if 0
  //liantest
  if( (gnOffset[fd]>(PCI_PIN_MKEY_ADDR+55*32) )  )
  {
    if(len<200)
    {
      trace_debug_printf("gnOffset[%d]\n",gnOffset[fd]);
      sys_flash_read_operate(gnOffset[fd],sTemp,len);
      trace_debug_printf("FileWrite:");
      for(i=0;i<len;i++)
        trace_debug_printf("%02x ",sTemp[i]);
      trace_debug_printf("\n");
      
    }
  }
#endif  
  
  gnOffset[fd]+=len;
  
  
  return len;
  
  
}