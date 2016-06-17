/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


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

void HID_FifoPut(unsigned char *pucData, unsigned int ulLen)
{
    unsigned int    ulRecLen = 0;
    unsigned int    ulNextNo = 0;
    
    if (pucData == NULL || ulLen == 0)
        return;

    ulRecLen = ulLen > MAX_PACK_SIZE ? MAX_PACK_SIZE : ulLen;

    ulNextNo = s_Hid_Fifo.ulInNum;
    memcpy(&Receive_Buffer[ulNextNo % HID_REC_BUFF_SIZE], pucData, ulRecLen);

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
    unsigned int  ulRecLen = 0;
    unsigned char aucBuf[MAX_PACK_SIZE] = {0};
     
    ulRecLen = USB_SIL_Read(EP1_OUT, aucBuf);
    HID_FifoPut(aucBuf, ulRecLen);
    SetEPRxStatus(ENDP1, EP_RX_VALID);

#if 0//MY_DES
    /* 既收也发 */
    USB_SIL_Write(EP1_IN, (uint8_t*) Receive_Buffer, ulRecLen);  
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
//    PrevXferComplete = 1;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

