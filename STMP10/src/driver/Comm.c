/// @file   Comm.c
/// This is for module @ref COM
/// @author Jie Yu
/// @date   2006-Sep-1st


#include "TaxBox.h"
#include <string.h>
#include "usb_desc.h"
#include "Toolkit.h"
bool Comm_Mode;                     // FALSE: UART;  TRUE: USB
bool Comm_Flag;                     //

/// @param pRecvBuf \b Point to received data from communication port
/// @param pPacketLen \b Point to the length of received data
/// @return None
/*
u8 Comm_RecvPacket(u8 *pRecvBuf, u16 *pPacketLen)
{
    bool bFlag;
    unsigned long count1,count2;
      	   count1=0xffff;
           count2=0x9f;
           USBRecvedLen = 0;
           USBRecvLen = *pPacketLen;
	    pUSBRecvData=pRecvBuf;
        if (!Comm_Flag)
        {
            UARTRecvedLen = 0;
            UARTRecvLen = *pPacketLen;

			pUARTRecvData=pRecvBuf;

        }

        do{
            if (AppFlag.bPowerLost)
            {
                    WaitNuS(50000);
                    if (!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10))
                    {
                    LED_Show(ORANGE_LED,0);
                    WaitNuS(50000);
                    Protect_Switch(0);
                    WaitNuS(50000);
                    Buzzer_Control(FALSE);//YYW 20081219
                    NVIC_GenerateSystemReset();   //YYW 20081219
                    }
            }
			
                 	    count1--;
//            if(count1==0)
//              return 1;
	    if((count1==0)&&(count2!=0))
	    	{
                count2--;
		  count1=0x2fff;
		}
          else
          	{
		if((count1==0)&&(count2==0))
          	return 1;
		}
			
        }while (!Comm_Flag);

        LED_Show(ORANGE_LED, 0);

        if (Comm_Mode)
            bFlag = USBCom_RecvPacket(pPacketLen);
        else
            bFlag = UARTCOM_RecvPacket(pRecvBuf, pPacketLen);

        LED_Show(ORANGE_LED, 1);
        Comm_Flag = FALSE;

  //      if ((memcmp(pRecvBuf, "\x4E\x50\x00\x05\x01\x42\xCE", 7) == 0)
  //              || (memcmp(pRecvBuf, "\x4E\x50\x00\x05\x02\x42\xC4", 7) == 0))
  //          break;
       if(bFlag)
        return 0;
    else
        return 1;
	
}

/// @param pSendBuf \b Point to send data buffer
/// @param PacketLen \b Point to the length of send data
/// @return None
void Comm_SendPacket(u8 *pSendBuf, u16 PacketLen)
{
    LED_Show(ORANGE_LED, 0);

    if (Comm_Mode)
   {
        if( (PacketLen%(VIRTUAL_COM_PORT_DATA_SIZE - 1))==0 )
        {
            USBCom_SendPacket(pSendBuf, 30);
            USBCom_SendPacket(pSendBuf+30, PacketLen-30);
        }
        else
            USBCom_SendPacket(pSendBuf, PacketLen);
    }
      //  USBCom_SendPacket(pSendBuf, PacketLen);
    else
        UARTCOM_SendPacket(pSendBuf, PacketLen);

    LED_Show(ORANGE_LED, 1);
    Comm_Flag = FALSE;
}
*/