#ifndef KF323_EMV_HARD_H
#define KF323_EMV_HARD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 提供给卡片的工作时钟频率
 */
#define FCLK_CARD     ( 3600000UL )

/**
 * 卡片通道编号定义
 */
#define USC           ( 0 )/*channel 0*/
#define SAM1          ( 1 )/*channel 1*/
#define SAM2          ( 2 )/*channel 2*/
#define SAM3          ( 3 )/*channel 3*/
#define SAM4          ( 4 )/*channel 4*/

#define SC_BIG_UART   USART2
#define SC_SML_UART   USART1

/* 
 * 用户卡硬件PIN脚定义
 */
#define		ICC_IO_PIN			GPIO_Pin_2		/* IC卡数据 */
#define		ICC_CLK_PIN			GPIO_Pin_4		/* IC卡时钟 */
#define		ICC_DET_PIN			GPIO_Pin_15		/* IC卡插入检测, 	H:无卡 L:有卡插入 */
#define		ICC_RST_PIN			GPIO_Pin_2		/* IC卡复位,		L:复位 */
#define		ICC_VCC_PIN			GPIO_Pin_6		/* IC卡上下电控制,	H:下电 L:上电 */
                                                
#define		ICC_GPIO_PORT_1		GPIOA			/*	IC卡- IO/CLK/DET -PORT */
#define		ICC_GPIO_PORT_2		GPIOE			/*	IC卡- RST/VCC 	-PORT */

/**	
 * SAM通道切换关系
 * ---------------------------------------------------------------------------
 *	SAM Card 	CSH	:	CSL
 * ---------------------------------------------------------------------------
 *	SAM1		L		L
 *	SAM2		L		H
 *	SAM3		H		L
 *	SAM4		H		H
 * ---------------------------------------------------------------------------
 */
#define		SAM_RST_PIN			GPIO_Pin_0		/*	SAM卡复位,		片选供给 */
#define		SAM_VCC_PIN			GPIO_Pin_1		/*	SAM卡上电控制	统一供给    L：上电 H：下电 */
#define		SAM_CLK_PIN			GPIO_Pin_8		/*	SAM卡时钟,		统一供给 */
#define		SAM_IO_PIN			GPIO_Pin_9		/*	SAM卡数据,		片选供给 */
#define		SAM_CSL_PIN			GPIO_Pin_10		/*	SAM卡选择低位,	H:片选 (*)旧版本的LED数码管驱动使用了这根线 */
#define		SAM_CSH_PIN			GPIO_Pin_11		/*	SAM卡选择高位,	H:片选 (*)旧版本的LED数码管驱动使用了这根线 */
                                                
#define		SAM_GPIO_PORT_1		GPIOA			/*	RST/VCC/CLK/IO Port */
#define		SAM_GPIO_PORT_2		GPIOF			/*	SL/SH Port */


#ifdef __cplusplus
}
#endif

#endif
