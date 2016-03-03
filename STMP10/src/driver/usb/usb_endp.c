/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : usb_endp.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Endpoint routines
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#ifdef STM32L1XX_MD
#include "stm32l1xx.h"
#else
#include "stm32f10x_lib.h"
#endif /* STM32L1XX_MD */

//#include "platform_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Receive_Buffer[HID_REC_BUFF_SIZE][MAX_PACK_SIZE + 1];  // 2 K 缓存
//extern __IO uint8_t PrevXferComplete;

struct HID_REC_FIFO
{
    unsigned int    ulInNum;
    unsigned int    ulOutNum;
    unsigned char   Reserve;
};

struct HID_REC_FIFO s_Hid_Fifo;

void HID_FifoInt(void)
{
    memset(Receive_Buffer, 0, sizeof(Receive_Buffer));
    memset(&s_Hid_Fifo, 0, sizeof(s_Hid_Fifo));
}

void HID_FifoPut(unsigned char *pucData, unsigned char ucLen)
{
    unsigned char   ucRecLen = 0;
    unsigned int    ulNextNo = 0;
    
    if (pucData == NULL || ucLen == 0)
        return;

    ucRecLen = ucLen > MAX_PACK_SIZE ? MAX_PACK_SIZE : ucLen;

    ulNextNo = s_Hid_Fifo.ulInNum;
    memcpy(&Receive_Buffer[ulNextNo % HID_REC_BUFF_SIZE], pucData, ucRecLen);

    ulNextNo = s_Hid_Fifo.ulInNum + 1;
    // 防止回绕成0后数据不连续
    if (0 == ulNextNo)
    {
        ulNextNo = s_Hid_Fifo.ulInNum % HID_REC_BUFF_SIZE + 1;
    }
        
    if ((ulNextNo % HID_REC_BUFF_SIZE) == (s_Hid_Fifo.ulOutNum % HID_REC_BUFF_SIZE))
    {
        s_Hid_Fifo.ulOutNum = ulNextNo % HID_REC_BUFF_SIZE + 1;   // 覆盖最老的一条
    }
    s_Hid_Fifo.ulInNum = ulNextNo;


    return;
}

int HID_IfHaveData(void)
{
    if ((s_Hid_Fifo.ulInNum % HID_REC_BUFF_SIZE) == (s_Hid_Fifo.ulOutNum  % HID_REC_BUFF_SIZE))
        return 0;
    else
        return 1;
}

int HID_FifoGet(unsigned char *pucData)
{
    unsigned int ulCurOutNo = 0;

    if (pucData == NULL)
        return -1;

    ulCurOutNo = s_Hid_Fifo.ulOutNum;

    if (HID_IfHaveData())
    {
        memcpy(pucData, &Receive_Buffer[ulCurOutNo % HID_REC_BUFF_SIZE], MAX_PACK_SIZE);
        ulCurOutNo++;
        if (0 == ulCurOutNo)
        {
            s_Hid_Fifo.ulOutNum = s_Hid_Fifo.ulOutNum % HID_REC_BUFF_SIZE + 1;
        }
        else
        {
            s_Hid_Fifo.ulOutNum++;
        }
        return MAX_PACK_SIZE;
    }

    return 0;
}
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
    unsigned char ucRecLen = 0;
    unsigned char aucBuf[MAX_PACK_SIZE] = {0};
     
    ucRecLen = USB_SIL_Read(EP1_OUT, aucBuf);
    SetEPRxStatus(ENDP1, EP_RX_VALID);

    HID_FifoPut(aucBuf, ucRecLen);

#if !MY_DES
    /* 既收也发 */
    USB_SIL_Write(EP1_IN, (uint8_t*) Receive_Buffer, ucRecLen);  
    SetEPTxValid(ENDP1);
#endif
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback.
* Description    : EP1 IN Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
  //PrevXferComplete = 1;
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

