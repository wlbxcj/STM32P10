#ifndef _PICC_STM32F_H
#define _PICC_STM32F_H

#include "stm32f10x_lib.h"
#include "stm32f10x_spi.h"

#define SPI1_NSS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define SPI1_NSS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4)

#undef Picc_Reset

// ¼Ä´æÆ÷Î»¶¨Òå
#ifndef BIT0
 #define BIT0    ((uint)1<<0)
#endif
#ifndef BIT1
 #define BIT1    ((uint)1<<1)
#endif

#define	RFINT	BIT1
#define RFRESET BIT0

#define ERR_NO 0


#endif