#ifndef _LCD_H
#define _LCD_H


#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#define  CLOSEICON  0    // 关闭图标[针对所有图标]
#define  OPENICON   1    // 显示图标[针对打印机、IC卡、锁、电池、向上、向下]

#define LCD_WIDTH   (132)
#define LCD_HIGH    (48)

#define LCD_WIDTH_OLD   (122)
#define LCD_HIGH_OLD   (32)

#define ST7567_M
//#undef ST7567_M

int  s_LcdInit(void);
void Lib_LcdCls(void);
void Lib_LcdClrLine(BYTE startline, BYTE endline);
void Lib_LcdSetBackLight(BYTE mode);
void Lib_LcdSetGray(BYTE level);
void Lib_LcdGotoxy(BYTE x, BYTE y);
int  Lib_LcdSetFont(BYTE AsciiFontHeight, BYTE ExtendFontHeight, BYTE Zoom);
BYTE Lib_LcdSetAttr(BYTE attr);
void Lib_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color);
void Lib_LcdDrawLogo(BYTE *pDataBuffer);
void Lib_LcdDrawBox(BYTE y1, BYTE x1, BYTE y2, BYTE x2);
int  Lib_LcdRestore(BYTE mode);
void Lib_LcdSetIcon(BYTE byIconNo, BYTE byMode);
int  Lib_Lcdprintf(char *fmt,...);
//int  printf(const char *fmt,...);
void Lib_LcdPrintxy(BYTE col, BYTE row, BYTE mode, char *str,...);
void Lib_LcdGetSize(BYTE *x, BYTE *y);
void Lib_LcdDrawLine(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE byColor);


BYTE Lib_LcdGetSelectItem(BYTE *pbyItemString, BYTE byItemNumber, BYTE byExitMode);
BYTE Lib_LcdGetSelectItemCE(BYTE *pbyChnItemString, BYTE *pbyEngItemString,
                     BYTE byItemNumber, BYTE byExitMode);
void Lib_LcdPrintfCE(char *pCHN , char *pEN);
void Lib_LcdPrintxyCE(BYTE col, BYTE row, BYTE mode, char *pCHN , char *pEN);
void Lib_LcdPrintxyExtCE(BYTE col, BYTE row, BYTE mode, char * pCHN, char * pEN, int iPara);
BYTE Lib_LcdShowItem(BYTE *pbyItemString, BYTE byItemNumber, BYTE byShowPage);
#endif
