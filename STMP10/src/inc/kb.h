#ifndef _KEYBOARD_H
#define _KEYBOARD_H


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

extern void TimerScanBegin();

#endif