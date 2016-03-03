#ifndef _COMM_H
#define _COMM_H

#define  COM_INVALID_PARA     (-6501)
#define  COM_NOT_OPEN         (-6502)
#define  COM_INVALID_PORTNO   (-6503)
#define  COM_RX_TIMEOUT       (-6504)
#define  COM_TX_TIMEOUT       (-6505)
#define  COM_TX_MEMOVER       (-6506)
#define  COM_NO_EMPTYPORT     (-6507) 

#define UART2	         1
#define UART1	         0

#define MAX_PORT_NUM	 UART2

void s_ComInit(void);

int  Lib_ComOpen(unsigned char channel, char *Attr);
int  Lib_ComClose(unsigned char channel);
int  Lib_ComSendByte(unsigned char channel, unsigned char ch);
int  Lib_ComRecvByte(unsigned char channel, unsigned char *ch, int ms);
int  Lib_ComReset(unsigned char channel);
extern void trace_debug_printf(char *str,...);
extern void debug_printf(u8 col,u8 row,u8 mode,char *str,...);
extern unsigned char getkey(void);


#endif
