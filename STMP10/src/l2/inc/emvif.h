/*******************************************************************************
* File Name			 : emvif.h
* Author             : guohonglv
* Date First Issued  : 2010年11月15日14:23:46
* Description        : 完全引用qpboctest项目的头文件 
* 注意：             ：第一句的include注释了
********************************************************************************/

//#include <sys/ioctl.h> //guohonglv

//#include "SystemFunction.h"

#include "misc.h"

#define uchar unsigned char

#define TAG_NULL_1				0x00	// null tag
#define TAG_NULL_2				0xFF	// null tag

extern uchar WaitKey(unsigned int iTimeInt);

#pragma pack(1) 
typedef struct
{
	unsigned int  nTag;
	unsigned char bOption;
}T_EMVTAG;
#pragma pack() 

/* Values for the WHENCE argument to lseek.  */

#ifndef	_STDIO_H		/* <stdio.h> has the same definitions.  */

# define SEEK_SET	0	/* Seek from beginning of file.  */

# define SEEK_CUR	1	/* Seek from current position.  */

# define SEEK_END	2	/* Seek from end of file.  */

#endif