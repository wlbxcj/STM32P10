/// @file   USBCOM.h
/// This is for module @ref USB_COM
/// @author Jie Yu
/// @date   2006-Sep-1st

#ifndef _USBCOM_H_
#define _USBCOM_H_

/**
 * @addtogroup Platform_Device_Library
 * @{
 */

/**
 * @addtogroup USB_COM
 * @{
 */
/**
 * @defgroup USB_COM_HIGH_INTERFACE
 * @ingroup USB_COM
 * @{
 */
extern u8 *pUSBRecvData;    ///<Pointer for USB Receive Buffer
extern u8 *pUSBSendData;    ///<Pointer for USB Send BUffer
extern u16 USBRecvLen;      ///<Length of the Data Buffer to be received on USB
extern u16 USBRecvedLen;    ///<Length of the Data Buffer Just been acturely recieved on UART
extern u16 USBSendLen;      ///<Length of the Data Buffer to be sent on USB
extern u16 USBSentLen;      ///<Length of the Data Buffer Just been sent on USB

///USB INITIALIZATION
void USBCom_Init(void);
///Receive Data on USB Port
bool USBCom_RecvPacket(u16 *pDataLength);
///Send Data On USB Port
void USBCom_SendPacket(u8 *Data, u16 DataLength);
u8 usb_power_off(void);
/**@}*/

/**@}*/

/**@}*/

#endif
