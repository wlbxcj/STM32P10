
#include "port.h"
#include <time.h>
#include "misc.h"

unsigned int time(unsigned int *tm)
{
  *tm =  RTC_GetCounter();
  return *tm;
}

struct tm * localtime(const unsigned int *nSec)
{
    unsigned int nHour,nHourOth;
    struct tm  ptTm;
 //   struct tm  RetTm;    
    unsigned char sBuf[10];

    
   // *ptTm = RetTm;
    nHour = (*nSec)/3600;
    nHourOth = (*nSec)%3600;
    
    ptTm.tm_hour = (nHour);
    ptTm.tm_min =  (nHourOth/60);
    ptTm.tm_sec =  (nHourOth%60);
      
    DateGet(sBuf);//CCYY MM DD
    ptTm.tm_year = (sBuf[2]*10) + sBuf[3];
    ptTm.tm_mon = (sBuf[4]*10) + sBuf[5];
    ptTm.tm_mday = (sBuf[6]*10) + sBuf[7];
    return &ptTm; 
}

unsigned int mktime(struct tm * ptTm)
{
  unsigned char sBuf[100];
  sBuf[0] = 0x20;
  sBuf[1] = ((ptTm->tm_year/10)<<4) | (ptTm->tm_year%10);
  sBuf[2] = ((ptTm->tm_mon/10)<<4) | (ptTm->tm_mon%10);
  sBuf[3] = ((ptTm->tm_mday/10)<<4) | (ptTm->tm_mday%10);
  
  ChangeDate(sBuf);
  sBuf[0] = ((ptTm->tm_hour/10)<<4) | (ptTm->tm_hour%10);
  sBuf[1] = ((ptTm->tm_min/10)<<4) | (ptTm->tm_min%10);
  sBuf[2] = ((ptTm->tm_sec/10)<<4) | (ptTm->tm_sec%10);
  
  ChangeTime(sBuf);
  return 0;
}

void  gettime(struct tm *tDate)
{
    time_t t1;
    struct tm *t;
    
    t1=time(&t1); 
    t=localtime(&t1);  
    
     
    tDate->tm_sec=t->tm_sec;
    tDate->tm_min=t->tm_min;
    
    tDate->tm_hour=t->tm_hour;
    tDate->tm_mday=t->tm_mday;
    tDate->tm_mon=t->tm_mon+1;
#ifdef KF311_M //KF322 KF311 ok
    tDate->tm_mon=t->tm_mon;
    tDate->tm_year = t->tm_year;
#else
    tDate->tm_mon=t->tm_mon+1;
    tDate->tm_year=(t->tm_year+1900)%100; //tm_year从1900算起的年数
#endif    
    /*
    //->bcd !!!change
    tDate->tm_sec=((tDate->tm_sec/10)<<4)|(tDate->tm_sec%10);
    tDate->tm_min=((tDate->tm_min/10)<<4)|(tDate->tm_min%10);
    
    tDate->tm_hour=((tDate->tm_hour/10)<<4)|(tDate->tm_hour%10);
    tDate->tm_mday=((tDate->tm_mday/10)<<4)|(tDate->tm_mday%10);
    tDate->tm_mon=(((tDate->tm_mon+1)/10)<<4)|((tDate->tm_mon+1)%10);
    tDate->tm_year=((((tDate->tm_year+1900)%100)/10)<<4)|((tDate->tm_year+1900)%10);
    */
}

void settime (struct tm *tDate)
{
    struct tm t;
    time_t t1;

	/*
    t.tm_sec=(buf[5]>>4)*10+(buf[5]&0x0f);
    t.tm_min=(buf[4]>>4)*10+(buf[4]&0x0f);
    t.tm_hour=(buf[3]>>4)*10+(buf[3]&0x0f);
    t.tm_mday=(buf[2]>>4)*10+(buf[2]&0x0f);
    t.tm_mon=(buf[1]>>4)*10+(buf[1]&0x0f)-1;
    t.tm_year=(buf[0]>>4)*10+(buf[0]&0x0f)+2000-1900;
    */
    t.tm_sec=tDate->tm_sec;
    t.tm_min=tDate->tm_min;
    t.tm_hour=tDate->tm_hour;
    t.tm_mday=tDate->tm_mday;
#ifdef KF311_M   //KF311 KF322 OK
    t.tm_mon=tDate->tm_mon;
    t.tm_year=tDate->tm_year;
#else    
    t.tm_mon=tDate->tm_mon-1;
    t.tm_year=tDate->tm_year+2000-1900;
#endif    
    t1=mktime(&t);  
#ifndef KF311_M
    stime(&t1);
    system("hwclock -w");
#endif	
	
}