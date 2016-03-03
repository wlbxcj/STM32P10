

#ifndef _BUZZER_H_
#define _BUZZER_H_

#include "stm32f10x_lib.h"


void Buzzer_Init(u16 nFre);
void Buzzer_Control(bool Control);
void Buzzer_Ring(u16 Fre);
extern void Buzzer_Off(void);

#endif
