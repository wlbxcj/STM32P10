#include "FlashOperate.h"
#include <string.h>
#include "comm.h"

#define PCI_BACK_ADDR     (4*2048)

//#define WriteProtection_Disable
#define FLASH_PAGE_SIZE   ((u16)0x800)//2K/a block

#define FLASH_OP_OK         (0x00)
#define PAGE_NUM_ERR        (0x11)
#define WR_LEN_ERR          (0x12)
#define WR_ADDRESS_ERR      (0x13)
#define ADDRESS_LEN_ERR     (0x14)

//unsigned char flash_erase_page(unsigned char page_num)
unsigned char flash_erase_page(u32 page_num)
{
  volatile FLASH_Status FLASHStatus;
  FLASHStatus = FLASH_COMPLETE;
#if 0
  if(page_num>=MAX_PAGE_NUM) return PAGE_NUM_ERR;
#endif  
  //Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
#if 0  
  FLASHStatus = FLASH_ErasePage(StartAddr + (FLASH_PAGE_SIZE * page_num));
#endif  
  FLASHStatus = FLASH_ErasePage(  (FLASH_PAGE_SIZE * page_num));
  
  FLASH_Lock();
  
  if(FLASHStatus==FLASH_COMPLETE) return FLASH_OP_OK;
  else
    return FLASHStatus;
}


unsigned char RM_flash_erase_page(unsigned char page_num)
{
  volatile FLASH_Status FLASHStatus;
  FLASHStatus = FLASH_COMPLETE;
#if 0
  if(page_num>=MAX_PAGE_NUM) return PAGE_NUM_ERR;
#endif  
  //Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
  
  FLASHStatus = FLASH_ErasePage( (0x08000000+APPLICATIONADDRESS+FLASH_PAGE_SIZE * page_num));
  
  FLASH_Lock();
  if(FLASHStatus==FLASH_COMPLETE) return FLASH_OP_OK;
  else
    return FLASHStatus;
}

unsigned char user_flash_erase_page(unsigned char page_num)
{
  volatile FLASH_Status FLASHStatus;
  FLASHStatus = FLASH_COMPLETE;

  if(page_num>=USER_MAX_PAGE_NUM) return PAGE_NUM_ERR;

  //Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
  
  FLASHStatus = FLASH_ErasePage(UserStartAddr + (FLASH_PAGE_SIZE * page_num));
  
  FLASH_Lock();
  if(FLASHStatus==FLASH_COMPLETE) return FLASH_OP_OK;
  else
    return FLASHStatus;
  
}

unsigned char sys_flash_erase_page(unsigned char page_num)
{
  volatile FLASH_Status FLASHStatus;
  FLASHStatus = FLASH_COMPLETE;

  if(page_num>=SYS_MAX_PAGE_NUM) return PAGE_NUM_ERR;

  //Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
  
  FLASHStatus = FLASH_ErasePage(SysStartAddr + (FLASH_PAGE_SIZE * page_num));
  
  FLASH_Lock();
  if(FLASHStatus==FLASH_COMPLETE) return FLASH_OP_OK;
  else
    return FLASHStatus;
  
}

unsigned char flash_write_operate(u32 Address,u16 * buff,u32 len)
{
  volatile FLASH_Status FLASHStatus;
  u32 Temp_Addr;
  FLASHStatus = FLASH_COMPLETE;
  
  if(len%2) return WR_LEN_ERR;
  if(Address%2) return WR_ADDRESS_ERR;
#if 0  
  if( Address<StartAddr || Address+len>EndAddr ) return ADDRESS_LEN_ERR;
#endif  
  //Unlock the Flash Program Erase controller 

  FLASH_Unlock();
  
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
  Temp_Addr=Address+len;
  
  while((Address < Temp_Addr) && (FLASHStatus == FLASH_COMPLETE))
  {
    FLASHStatus = FLASH_ProgramHalfWord(Address, *buff);
    Address = Address + 2;
    buff++;
  }
  
  FLASH_Lock();
  
  if( FLASHStatus==FLASH_COMPLETE ) return FLASH_OP_OK;
  else
    return FLASHStatus;
}

unsigned char user_flash_write_operate(u32 Address,u16 * buff,u32 len)
{
  //if( Address<UserStartAddr || Address+len>UserEndAddr ) 
 
  if(  Address+UserStartAddr+len>UserEndAddr ) 
    return ADDRESS_LEN_ERR;
  
  return flash_write_operate(Address+UserStartAddr,buff,len);
}


//unsigned char sys_flash_write_operate(u32 Address,u16 * buff,u32 len)
unsigned char sys_flash_write_operate(u32 Address,u8 * buff,u32 len)
{
  u32  bBeginPage,bEndPage;
  
  unsigned char sPageBuf[2048];
  u32 nCurAddress,nCurLen,nSize,i,nOffset;
  //test
  unsigned char bRet,j;
  
  //if( Address<SysStartAddr || Address+len>SysEndAddr ) 
  if(  (Address+len+SysStartAddr)>SysEndAddr ) 
    return ADDRESS_LEN_ERR;
  bBeginPage = (Address+SysStartAddr)/FLASH_PAGE_SIZE;
  //if( (Address+SysStartAddr)%FLASH_PAGE_SIZE)
  //  bBeginPage++;
  bEndPage = (Address+SysStartAddr+len)/FLASH_PAGE_SIZE;
  if( (Address+SysStartAddr+len)%FLASH_PAGE_SIZE)
    bEndPage++;
  nCurAddress = Address+SysStartAddr;
  nCurLen = len;
  nOffset  = 0;
  bRet = 0;
  
  //13/06/24
  FLASH_Unlock();
  
  for(i=bBeginPage;i<bEndPage;i++)
  {
    bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    
    bRet = CrcPageCmp(i-SysStartAddr/FLASH_PAGE_SIZE,sPageBuf);
    //test
    trace_debug_printf("CrcPageCmp[%d]\n",bRet);
    
    bRet = flash_erase_page(i);
    //13/07/09
    if( (i<(SysStartAddr/FLASH_PAGE_SIZE +4)) && //save key page
       memcmp(buff,"\x0\x0\x0\x0\x0\x0\x0\x0",8)!=0) //key data
    bRet = flash_erase_page(i+PCI_BACK_ADDR/FLASH_PAGE_SIZE);
    //liantest 13/05/31
    
    if(bRet)
      trace_debug_printf("flash_erase_page[%d]", bRet);
    
    //nSize = nCurLen%FLASH_PAGE_SIZE;
    nSize =FLASH_PAGE_SIZE- ((nCurAddress-SysStartAddr)%FLASH_PAGE_SIZE);
    if(nSize>nCurLen)
      nSize =nCurLen;
    memcpy(&sPageBuf[nCurAddress%FLASH_PAGE_SIZE],(unsigned char *)&buff[nOffset],nSize);
    j = 0;
    
    bRet = CrcPageCalc(i-SysStartAddr/FLASH_PAGE_SIZE,sPageBuf);
    //test
    trace_debug_printf("CrcPageCalc[%d]\n",bRet);
    
  WRITE:    
    bRet = flash_write_operate(i*FLASH_PAGE_SIZE,(unsigned short *)sPageBuf,FLASH_PAGE_SIZE);
    //13/07/09 
    if( (i<(SysStartAddr/FLASH_PAGE_SIZE +4)) &&//save key page
       memcmp(buff,"\x0\x0\x0\x0\x0\x0\x0\x0",8)!=0) //key data
    bRet = flash_write_operate(i*FLASH_PAGE_SIZE+PCI_BACK_ADDR,(unsigned short *)sPageBuf,FLASH_PAGE_SIZE);
    
    //liantest 13/05/31
    if(bRet)
    {
      trace_debug_printf("number[%d] flash_write_operate[%d]", j,bRet);
      
      delay_ms(50);
      if(j++<4)
        goto WRITE;
    }
    
    //13/07/01
    bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    bRet = CrcPageCmp(i-SysStartAddr/FLASH_PAGE_SIZE,sPageBuf);
    //test
    trace_debug_printf("CrcPageCmp[%d]\n",bRet);
    
    //test
    //bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    
    nCurLen -= nSize;
    nCurAddress += nSize;
    nOffset +=nSize;
  }
  
  //13/06/24
  FLASH_Lock();
    
  return bRet;//13/05/31
}

unsigned char flash_write_operate_nocheckcomplete(u32 Address,u16 * buff,u32 len)
{
  volatile FLASH_Status FLASHStatus;
  u32 Temp_Addr;
  FLASHStatus = FLASH_COMPLETE;
  if(len%2) return WR_LEN_ERR;
  if(Address%2) return WR_ADDRESS_ERR;
  if( Address<StartAddr || Address+len>EndAddr ) return ADDRESS_LEN_ERR;
  
  //Unlock the Flash Program Erase controller 
  FLASH_Unlock();
  //Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
  Temp_Addr=Address+len;
  
  while((Address < Temp_Addr) )
  {
    FLASHStatus = FLASH_ProgramHalfWord(Address, *buff);
    Address = Address + 2;
    buff++;
  }
  
  FLASH_Lock();
  
  if( FLASHStatus==FLASH_COMPLETE ) return FLASH_OP_OK;
  else
    return FLASHStatus;
}


unsigned char flash_read_operate(u32 Address,u8 * buff,u32 len)
{
  volatile FLASH_Status FLASHStatus;
  u32 Temp_Addr;
  FLASHStatus = FLASH_COMPLETE;
#if 0  
  if( Address<StartAddr || Address+len>EndAddr ) return ADDRESS_LEN_ERR;
#endif
  
  Temp_Addr=Address+len;
  while( Address < Temp_Addr )
  {
    *buff=*(vu8*)Address;
    Address += 1;
    buff++;
  }
  return FLASH_OP_OK;
}

unsigned char user_flash_read_operate(u32 Address,u8 * buff,u32 len)
{
  //if( Address<UserStartAddr || Address+len>UserEndAddr ) 
  if(  Address+UserStartAddr+len>UserEndAddr ) 
    return ADDRESS_LEN_ERR;
  return flash_read_operate(Address+UserStartAddr,buff,len);
}

unsigned char sys_flash_read_operate(u32 Address,u8 * buff,u32 len)
{
  //if( Address<SysStartAddr || Address+len>SysEndAddr ) 
  if(  Address+SysStartAddr+len>SysEndAddr ) 
    return ADDRESS_LEN_ERR;
  
  return flash_read_operate(Address+SysStartAddr,buff,len);
}    


unsigned char flash_op_test(void)
{
  u8 buff[1024*2+11],ret=0;
  u32 len=1024*2+2,i;
  
        ret=flash_read_operate(StartAddr,buff,len);
  
  memset(buff,0xa5,sizeof(buff));
  ret=flash_write_operate(StartAddr,(u16 *)buff,len);
  if(ret)
  {
    debug_printf(0,0,0,"flash_write_operate err1,ret:%d",ret);
    ret=flash_erase_page(0);
    if(ret)
    {
      debug_printf(0,0,0,"flash_erase_page err2,ret:%d",ret);
    }
    else
    {
      debug_printf(0,0,0,"flash_erase_page ok1");
      flash_erase_page(1);
      ret=flash_write_operate(StartAddr,(u16 *)buff,len);
      if(ret)
      {
        debug_printf(0,0,0,"flash_write_operate err3,ret:%d",ret);
       
      }
      else
      {
        debug_printf(0,0,0,"flash_write_operate ok2");
        memset(buff,0x00,sizeof(buff));
        ret=flash_read_operate(StartAddr+1,buff,len);
        if(ret)
        {
          debug_printf(0,0,0,"flash_read_operate err4,ret:%02x",ret);
        }
        else
        {
          debug_printf(0,0,0,"flash_read_operate ok3");
          for(i=0;i<len;i++)
          {
            debug_printf(0,0,0," %02x ",buff[i]);
          }
        }
      }
    }
  }
  else
  {
    debug_printf(0,0,0,"flash_write_operate okx1");
    memset(buff,0x00,sizeof(buff));
    ret=flash_read_operate(StartAddr+1,buff,len);
    if(ret)
    {
      debug_printf(0,0,0,"flash_read_operate errx1,ret:%02x",ret);
    }
    else
    {
      debug_printf(0,0,0,"flash_read_operate okx2");
      for(i=0;i<len;i++)
      {
        debug_printf(0,0,0," %02x ",buff[i]);
      }
    }
  }
  return 0;
}

int Lib_AppShareRead(unsigned long Address,unsigned char * buff,unsigned long len)
{
  return  user_flash_read_operate(Address,buff,len);
  
}


unsigned char user_flash_write_erase_operate(u32 Address,u8 * buff,u32 len)
{
  u32  bBeginPage,bEndPage;
  
  unsigned char sPageBuf[2048];
  u32 nCurAddress,nCurLen,nSize,i,nOffset;
  //test
  unsigned char bRet;

  //trace_debug_printf("UserEndAddr = %x,address = %x\n\r",UserEndAddr,(Address + UserStartAddr));
  //if( Address<SysStartAddr || Address+len>SysEndAddr ) 
  if((Address + UserStartAddr) > UserEndAddr)
    return WR_ADDRESS_ERR;
  if(  (Address+len+UserStartAddr)>UserEndAddr ) 
    return ADDRESS_LEN_ERR;
  bBeginPage = (Address+UserStartAddr)/FLASH_PAGE_SIZE;
  //if( (Address+SysStartAddr)%FLASH_PAGE_SIZE)
  //  bBeginPage++;
  bEndPage = (Address+UserStartAddr+len)/FLASH_PAGE_SIZE;
  if( (Address+UserStartAddr+len)%FLASH_PAGE_SIZE)
    bEndPage++;
  nCurAddress = Address+UserStartAddr;
  nCurLen = len;
  nOffset  = 0;
  for(i=bBeginPage;i<bEndPage;i++)
  {
    bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    bRet = flash_erase_page(i);
    //nSize = nCurLen%FLASH_PAGE_SIZE;
    nSize =FLASH_PAGE_SIZE- ((nCurAddress-UserStartAddr)%FLASH_PAGE_SIZE);
    if(nSize>nCurLen)
      nSize =nCurLen;
    memcpy(&sPageBuf[nCurAddress%FLASH_PAGE_SIZE],(unsigned char *)&buff[nOffset],nSize);
    bRet = flash_write_operate(i*FLASH_PAGE_SIZE,(unsigned short *)sPageBuf,FLASH_PAGE_SIZE);
    //test
    //bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    
    nCurLen -= nSize;
    nCurAddress += nSize;
    nOffset +=nSize;
  }
  
  return 0;
}

int Lib_AppShareWrite(unsigned long Address,unsigned char * buff,unsigned long len)
{
  //return user_flash_write_operate(Address,(u16*)buff,len);
  return user_flash_write_erase_operate(Address,buff,len);
}

unsigned char tsc_flash_write_erase_operate(u32 Address,u8 * buff,u32 len)
{
  u32  bBeginPage,bEndPage;
  
  unsigned char sPageBuf[2048];
  u32 nCurAddress,nCurLen,nSize,i,nOffset;
  //test
  unsigned char bRet;

  //trace_debug_printf("TscEndAddr = %x,address = %x\n\r",TscEndAddr,(Address + TscStartAddr));
  //if( Address<SysStartAddr || Address+len>SysEndAddr ) 
  if((Address + TscStartAddr) > TscEndAddr)
    return WR_ADDRESS_ERR;
  if(  (Address+len+TscStartAddr)>TscEndAddr ) 
    return ADDRESS_LEN_ERR;
  
  bBeginPage = (Address+TscStartAddr)/FLASH_PAGE_SIZE;
  //if( (Address+SysStartAddr)%FLASH_PAGE_SIZE)
  //  bBeginPage++;
  bEndPage = (Address+TscStartAddr+len)/FLASH_PAGE_SIZE;
  if( (Address+TscStartAddr+len)%FLASH_PAGE_SIZE)
    bEndPage++;
  nCurAddress = Address+TscStartAddr;
  nCurLen = len;
  nOffset  = 0;
  for(i=bBeginPage;i<bEndPage;i++)
  {
    bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    bRet = flash_erase_page(i);
    //nSize = nCurLen%FLASH_PAGE_SIZE;
    nSize =FLASH_PAGE_SIZE- ((nCurAddress-TscStartAddr)%FLASH_PAGE_SIZE);
    if(nSize>nCurLen)
      nSize =nCurLen;
    memcpy(&sPageBuf[nCurAddress%FLASH_PAGE_SIZE],(unsigned char *)&buff[nOffset],nSize);
    bRet = flash_write_operate(i*FLASH_PAGE_SIZE,(unsigned short *)sPageBuf,FLASH_PAGE_SIZE);
    //test
    //bRet = flash_read_operate(i*FLASH_PAGE_SIZE,sPageBuf,FLASH_PAGE_SIZE);
    
    nCurLen -= nSize;
    nCurAddress += nSize;
    nOffset +=nSize;
  }
  
  return 0;
}

int Lib_TscWrite(unsigned long Address,unsigned char * buff,unsigned long len)
{
  //return user_flash_write_operate(Address,(u16*)buff,len);
  return tsc_flash_write_erase_operate(Address,buff,len);
}
unsigned char tsc_flash_read_operate(u32 Address,u8 * buff,u32 len)
{
  //if( Address<UserStartAddr || Address+len>UserEndAddr ) 
  if(  Address+TscStartAddr+len>TscEndAddr ) 
    return ADDRESS_LEN_ERR;
  return flash_read_operate(Address+TscStartAddr,buff,len);
}

int Lib_TscRead(unsigned long Address,unsigned char * buff,unsigned long len)
{
  return  tsc_flash_read_operate(Address,buff,len);
  
}