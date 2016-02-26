#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "stm32f10x_lib.h"

u8 Display_Init(void);
void Ledarray_Init(void);
void Stnlcd_Init(void);
void Ledarray_On(u8 *NewLedData);
void Ledarray_Off(void);

void LedBit1_Control(bool Control);
void LedBit2_Control(bool Control);
void LedBit3_Control(bool Control);
void LedBit4_Control(bool Control);
void LedBit5_Control(bool Control);
void LedBit6_Control(bool Control);
void LedBit7_Control(bool Control);
void LedBit8_Control(bool Control);
void LedBits_Control(bool Control);
void Tim2Init(void);

#endif
