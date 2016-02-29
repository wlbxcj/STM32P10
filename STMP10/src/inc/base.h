#ifndef __BASE_H__
#define __BASE_H__

/*
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
*/
#ifndef NULL
#define NULL 0
#endif 

//#define V10X
//#undef V10X           // 以后版本不再区分10X 10P

#define  VPOS335S_VOS_DEBUG
//#undef   VPOS335S_VOS_DEBUG

#ifdef  VPOS335S_VOS_DEBUG
#define UARTPRINF_DEBUG     //定义此宏，打开串口调试信息
#define SYSTEMALLTEST_DEBUG
#else
#undef  UARTPRINF_DEBUG   //取消此宏，禁止串口调试信息
#undef  SYSTEMALLTEST_DEBUG
#endif
 

#ifdef __cplusplus
extern "C" {
#endif

/*
====================================================================
                common data type definition
====================================================================
*/
#ifndef   int8_t
#define   int8_t    char
#endif
#ifndef   uint8_t
#define   uint8_t   unsigned char
#endif
#ifndef   int16_t
#define   int16_t   short
#endif
#ifndef   uint16_t
#define   uint16_t  unsigned short
#endif
#ifndef   int32_t
#define   int32_t   long
#endif
#ifndef   uint32_t
#define   uint32_t  unsigned long
#endif
#ifndef   uchar
#define   uchar     unsigned char
#endif
#ifndef   uint
#define   uint      unsigned int
#endif
#ifndef   ulong
#define   ulong     unsigned long
#endif
#ifndef   ushort
#define   ushort    unsigned short
#endif
#ifndef   U8
#define   U8        unsigned char
#endif
#ifndef   U16
#define   U16       unsigned short
#endif
#ifndef   U32
#define   U32       unsigned long
#endif
#ifndef   S8
#define   S8        char
#endif
#ifndef   S16
#define   S16       short
#endif
#ifndef   S32
#define   S32       long
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif
  
/*
====================================================================
              DEBUG AND UART  LIB function definition
====================================================================
*/

#define VCOM1       1
#define VCOM2       2 
#define VCOM_DEBUG  0
#define COMM_MAX    3

#define RS232A     VCOM1
#define RS232B     VCOM2
#define COM1       VCOM1
#define COM2       VCOM2 

//#define COM_DEBUG   hal_uart_dev0
#define COM_DEBUG   VCOM_DEBUG

void s_UartInit(int port,unsigned int baudrate);
void s_UartPrint(int port,char *fmt,...);
unsigned char s_getkey(int port); 
void s_InitAllInt(void);
unsigned int s_ReadMipsCpuCount(void); 
 
  

#endif