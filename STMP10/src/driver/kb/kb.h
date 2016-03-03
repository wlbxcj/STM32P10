#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#define BIG_STRING_NUM    20
#define SMALL_STRING_NUM  26




/*
============================================================================

============================================================================
*/
//Defined for input key values
#define KEYF1            0x01
#define KEYF2            0x02
#define KEYF3            0x03
#define KEYF4            0x04
#define KEYF5            0x09
#define KEYF6            0x0a

#define KEY1             0x31
#define KEY2             0x32
#define KEY3             0x33
#define KEY4             0x34
#define KEY5             0x35
#define KEY6             0x36
#define KEY7             0x37
#define KEY8             0x38
#define KEY9             0x39
#define KEY0             0x30
#define KEYSTAR          0x3a

#define KEYCLEAR         0x08
#define KEYALPHA         0x07
#define KEYUP            KEYF1//0x05
#define KEYDOWN          KEYF2//0x06
#define KEYFN            0x15
#define KEYMENU          0x14
#define KEYENTER         0x0d
#define KEYCANCEL        0x1b
#define KEYNUM           0x1c
#define KEYPOWER         0x1d
#define NOKEY            0xff

#define CAN_ENTER_EXIT     0x80
#define IS_FONT_WIDTH_8    0x40
#define CAN_INPUT_NUMBER   0x20
#define CAN_INPUT_CHAR     0x10
#define CAN_INPUT_PASSWORD 0x08
#define IS_FLUSH_LEFT      0x04
#define CAN_INPUT_DECIMAL  0x02
#define DISPLAY_NOT_REVER  0x01

//(0)     输入成功
//(-1010) 输入参数错误
//(-1011) 输入超时
//(-1012) 用户取消
//(-1013) 用户确认退出
//（-1000）    无按键

#define KB_NoKey_Err        (-1000)
#define KB_InputPara_Err    (-1010)
#define KB_InputTimeOut_Err (-1011)
#define KB_UserCancel_Err   (-1012)
#define KB_UserEnter_Exit   (-1013)

int   s_KbInit(void);
BYTE  Lib_KbGetCh(void);
BYTE  Lib_KbUnGetCh(void);
int   Lib_KbCheck(void);
void  Lib_KbFlush(void);
int   Lib_KbSound(BYTE mode, WORD DlyTimeMs);
int   Lib_KbMute(BYTE mode);
int   Lib_KbGetStr(BYTE *str, BYTE minlen, BYTE maxlen, BYTE mode, WORD timeoutsec);

#define DELAY_COUNT    0x0FFFF

#define Keyboard_EXTI_Row1   EXTI_Line5
#define Keyboard_EXTI_Row2   EXTI_Line6
#define Keyboard_EXTI_Row3   EXTI_Line7
#define Keyboard_EXTI_Row4   EXTI_Line8

#define Keyboard_EXTI_Line    (Keyboard_EXTI_Row1 | Keyboard_EXTI_Row2 | Keyboard_EXTI_Row3 | Keyboard_EXTI_Row4)

#define Keyboard_Line   (GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 )
#define KB_IN_MASK      (GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 )

#define Keyboard_LineBase GPIO_Pin_9
#define Keyboard_Row_1 GPIO_Pin_5
#define Keyboard_Row_2 GPIO_Pin_6
#define Keyboard_Row_3 GPIO_Pin_7
#define Keyboard_Row_4 GPIO_Pin_8
#define Keyboard_IRQ_Channel  EXTI9_5_IRQChannel

extern unsigned int Keyboard_Val ;
extern unsigned char Keyboard_Change_Flag ;

#endif