
#define USBERR_MISS_DIR			    0xB3	/* 指定路径的某个子目录没有找到,可能是目录名称错误 */
#define USBERR_FILE_CLOSE			0xB4	/* 文件已经关闭,如果需要使用,应该重新打开文件 */
#define USBERR_OPEN_DIR			    0x41	/* 指定路径的目录被打开 */
#define USBERR_MISS_FILE			0x42	/* 指定路径的文件没有找到,可能是文件名称错误 */

///waiting the insertion of U-Disk pend on sem
//unsigned char USB_Wait_InsertDisk(unsigned int uiTimeOut);
//unsigned char USB_Wait_InsertDisk(void);    //YYW 20080826
//USB_Wait_InsertDisk();

///Read a file from given offset
unsigned char USB_ReadFile(unsigned int uiOffset,unsigned char *pucData,unsigned int uiLen);
//USB_ReadFile("\\NEWFILE.TXT",5,temp,10);


///write a file from given offset
unsigned char USB_WriteFile(unsigned int uiOffset, unsigned char *pucData, unsigned int uiLen, unsigned char ucWriteMode);
//USB_WriteFile("\\NEWFILE.TXT",1,"ab",2);


///append a file from the end of current file
unsigned char USB_AppendFile(unsigned char *pucFileName,unsigned char *pucData,unsigned int uiLen);
//USB_AppendFile("\\NEWFILE.TXT","zzzzzzzzzz",10);


///inquire the file size of given file name (temp file handler)
unsigned char USB_FileSize(unsigned char *pFilename,unsigned int *piSize);



///erase the file with given file name (temp file handler)
unsigned char USB_EraseFile(unsigned char *pucFileName);
     
unsigned char USB_CreateFile(unsigned char *pucFileName);
unsigned char USB_OpenFile(unsigned char *pucFileName);
unsigned char USB_CloseFile(void);

//usage:
//    //case 1
//    USB_Wait_InsertDisk();
//    int m,i,j,k;
//    for(m=0;m<10;m++)
//    {
//        u16 buf[128];
//        u16 i;
//        for(i=0;i<128;i++) buf[i]=i;
//
//        u8 path[]="\\tst1abc.txt";
//        path[6] = m/100+'0';
//        path[7] = m/10+'0';
//        path[8] = m%10+'0';
//
//        for(i=1;i<=2;i++)
//            for(j=1;j<=2;j++)
//                for(k=1;k<=2;k++)
//                {
//                    USB_AppendFile(path,buf,128*2);
//                }
//
//        u16 buf2[128*8];
//        memset(buf2,0,128*8*2);
//        USB_ReadFile(path,0,buf2,128*8*2);
//
//        printf("file: %d OK \n",m);
//    }
//
//
//    for(m=0;m<10;m++)
//    {
//        u16 buf[128];
//        u16 i;
//        for(i=0;i<128;i++) buf[i]=i;
//
//        u8 path[]="\\tst2abc.txt";
//        path[6] = m/100+'0';
//        path[7] = m/10+'0';
//        path[8] = m%10+'0';
//
//        for(i=1;i<=2;i++)
//            for(j=1;j<=2;j++)
//                for(k=1;k<=2;k++)
//                {
//                    USB_WriteFile(path,((i-1)*4+(j-1)*2+(k-1))*128*2,buf,128*2);
//                }
//
//        u16 buf2[128*8];
//        memset(buf2,0,128*8*2);
//        USB_ReadFile(path,0,buf2,128*8*2);
//
//        printf("file: %d OK \n",m);
//    }
             
unsigned char	mFlushBufferToDisk( unsigned char force );
unsigned char USB_OpenFile_Sector(unsigned char *pucFileName) ;
unsigned char USB_CloseFile_Sector(void);
