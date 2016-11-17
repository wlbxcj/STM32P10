/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for Custom HID Demo
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
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
/* �豸������������PC�� ����--Ӳ��ID  �鿴 */
const uint8_t CustomHID_DeviceDescriptor[CUSTOMHID_SIZ_DEVICE_DESC] =
{
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType   ���������� */
    0x00,                       /*bcdUSB            USBЭ��汾 */
    0x02,
    0x00,                       /*bDeviceClass      ����� ��� ����Ϊ0������һ
                                    ������Ҳ��Ϊ0   0x02 Ϊͨ���豸��*/
    0x00,                       /*bDeviceSubClass   ������� */
    0x00,                       /*bDeviceProtocol   �豸��ʹ�õ�Э�� */
    0x40,                       /*bMaxPacketSize40  �˵�0 �������� */
    0x88,//0x7E,//0x83,                       /*idVendor (0x0483) ����ID */
    0x88,//0x82,//0x04,
    0x05,//0x01,//0x53,//0x50,                       /*idProduct = 0x5750    ��Ʒ ID */
    0x00,//0x4F,//0x57,
    0x00,                       /*bcdDevice rel. 2.00   �豸�汾�� */
    0x02,
    1,                          /*Index of string descriptor describing
                                              manufacturer  �������̵��ַ��������� */
    2,                          /*Index of string descriptor describing
                                             product        ������Ʒ���ַ��������� */
    3,                          /*Index of string descriptor describing the
                                             device serial number */
    0x01                        /*bNumConfigurations        ���ܵ������� */
}; /* CustomHID_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
/*������������ ��*/
const uint8_t CustomHID_ConfigDescriptor[CUSTOMHID_SIZ_CONFIG_DESC] =
{
    /*  ����������  */
    0x09,         /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,  /* bDescriptorType: Configuration */
    CUSTOMHID_SIZ_CONFIG_DESC,          /* wTotalLength: Bytes returned */
    
    0x00,
    0x01,         /* bNumInterfaces: 1 interface    ��������֧�ֵĽӿ��� */
    0x01,         /* bConfigurationValue: Configuration value �����õ�ֵ */
    0x00,         /* iConfiguration: Index of string descriptor describing
                                 the configuration  ���������õ��ַ���������ֵ*/
    0xC0,         /* bmAttributes: Bus powered */
    0x96,//0x32,         /* MaxPower 300 mA: this current is used for detecting Vbus ��λΪ 2mA*/

    //0x03,0x09,0x03,

    /*  �ӿ������� ���踽��������������һ������     */
    /************** Descriptor of Custom HID interface ****************/
    /* 09 */
    0x09,         /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType: Interface descriptor type */
    0x00,         /* bInterfaceNumber: Number of Interface  �ӿڱ��    */
    0x00,         /* bAlternateSetting: Alternate setting   ���ýӿڱ��    */
    0x02,         /* bNumEndpoints  �˵���  */
    0x03,         /* bInterfaceClass: HID   �ýӿ�ʹ�õ���  0x03ΪHID   
                    1����Ƶ�࣬2��CDC�����࣬3���˻��ӿ��ࣨHID����5�������࣬
                    6��ͼ���࣬7����ӡ���࣬8�������ݴ洢�࣬9���������࣬
                    10��CDC�����࣬11�����ܿ��࣬13����ȫ�࣬220������豸�࣬
                    224�����߿����࣬254���ض�Ӧ���࣬255���̶�����豸*/
    0x00,         /* bInterfaceSubClass : 1=BOOT, 0=no boot ����    */
    0x00,         /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse �ӿ�Э��  */
    0x00,         /* iInterface: Index of string descriptor     �ַ�������ֵ    */

    /*  HID ������  */
    /******************** Descriptor of Custom HID HID ********************/
    /* 18 */
    0x09,         /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID */
    0x10,         /* bcdHID: HID Class Spec release number  HIDЭ��汾 2�ֽ�BCD 1.1�汾*/
    0x01,
    0x00,         /* bCountryCode: Hardware target country  ���Ҵ���    */
    0x01,         /* bNumDescriptors: Number of HID class descriptors to follow �¼�������������*/
    0x22,         /* bDescriptorType    �¼������������� 0x22 ���뱨�������� 0x23Ϊ����������*/
    (CUSTOMHID_SIZ_REPORT_DESC & 0xff),/* wItemLength: Total length of Report descriptor �¼��������ĳ���*/
    ((CUSTOMHID_SIZ_REPORT_DESC >> 8) & 0xff),


    /*  �˵�������  �����������˵�  */
    /******************** Descriptor of Custom HID endpoints ******************/
    /* 27 */
    0x07,           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */

    0x81,           /* bEndpointAddress: Endpoint Address (IN) �˵��ַ  */
    0x03,           /* bmAttributes: Interrupt endpoint  �˵����ԣ�����λ--0 ���ƴ��䣬
                                            1 ��ʱ���� 2 �������� 3 �жϴ���   */
    (MAX_PACK_SIZE & 0xFF),  //0x40,           /* wMaxPacketSize: 64 Bytes max       �˵�֧�ֵ���������*/
    ((MAX_PACK_SIZE >> 8) & 0xFF),
    0x01,           /* bInterval: Polling Interval (10 ms)   �˵��ѯʱ��    */


    /* 34 */
    0x07,	        /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
			        /*	Endpoint descriptor type */
    0x01,	        /* bEndpointAddress:  (OUT) */
    0x03,	        /* bmAttributes: Interrupt endpoint */
    (MAX_PACK_SIZE & 0xFF),  //0x40,           /* wMaxPacketSize: 64 Bytes max       �˵�֧�ֵ���������*/
    ((MAX_PACK_SIZE>> 8) & 0xFF),
    0x01,	        /* bInterval: Polling Interval (10 ms) */
    /* 41 */
}; /* CustomHID_ConfigDescriptor */


/*  ������������һ��һ����Ŀ��ɵ�  */
#if 0//MY_DES
const u8 CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
{
#if 0
        /* USB Vendor define2 */
    
        0x05, 0x01, // USAGE_PAGE (Generic Desktop device)��;ҳΪͨ�������豸
    
        0x09, 0x00, // USAGE (0)
    
        0xa1, 0x01, // COLLECTION (Application)//application collection  Ӧ�ü���
        
    
        0x85, 0x01,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x08, // USAGE_MAXIMUM (8) 
    
        0x95, 0x08, // REPORT_COUNT (8)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x01,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x08, // USAGE_MAXIMUM (8) 
    
        0x95, 0x08, // REPORT_COUNT (8)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x02,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x10, // USAGE_MAXIMUM (16) 
    
        0x95, 0x10, // REPORT_COUNT (16)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x02,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x10, // USAGE_MAXIMUM (16) 
    
        0x95, 0x10, // REPORT_COUNT (16)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        
    
        0x85, 0x03,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x20, // USAGE_MAXIMUM (32) 
    
        0x95, 0x20, // REPORT_COUNT (32)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x03,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x20, // USAGE_MAXIMUM (32) 
    
        0x95, 0x20, // REPORT_COUNT (32)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x04,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x40, // USAGE_MAXIMUM (64) 
    
        0x95, 0x40, // REPORT_COUNT (64)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x04,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x40, // USAGE_MAXIMUM (64) 
    
        0x95, 0x40, // REPORT_COUNT (64)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x05,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x80, // USAGE_MAXIMUM (128) 
    
        0x95, 0x80, // REPORT_COUNT (128)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x05,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x29, 0x80, // USAGE_MAXIMUM (128) 
    
        0x95, 0x80, // REPORT_COUNT (128)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x06,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x01, // USAGE_MAXIMUM (256) 
    
        0x96, 0x00, 0x01, // REPORT_COUNT (256)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x06,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x01, // USAGE_MAXIMUM (256) 
    
        0x96, 0x00, 0x01, // REPORT_COUNT (256)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x07,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x02, // USAGE_MAXIMUM (512) 
    
        0x96, 0x00, 0x02, // REPORT_COUNT (512)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x07,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x02, // USAGE_MAXIMUM (512) 
    
        0x96, 0x00, 0x02, // REPORT_COUNT (512)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        
    
        0x85, 0x08,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x04, // USAGE_MAXIMUM (1024) 
    
        0x96, 0x00, 0x04, // REPORT_COUNT (1024)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x08,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x04, // USAGE_MAXIMUM (1024) 
    
        0x96, 0x00, 0x04, // REPORT_COUNT (1024)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x09,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x08, // USAGE_MAXIMUM (2K) 
    
        0x96, 0x00, 0x08, // REPORT_COUNT (2K)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x09,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x08, // USAGE_MAXIMUM (2K) 
    
        0x96, 0x00, 0x08, // REPORT_COUNT (2K)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
    
    
        0x85, 0x0A,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x10, // USAGE_MAXIMUM (4K) 
    
        0x96, 0x00, 0x10, // REPORT_COUNT (4K)
    
        0x75, 0x08, // REPORT_SIZE (8)
    
        0x81, 0x02, // INPUT (Data,Var,Abs)
    
        0x85, 0x0A,
    
        0x15, 0x00, // LOGICAL_MINIMUM (0)
    
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
    
        0x19, 0x01, // USAGE_MINIMUM (1)
    
        0x2A, 0x00, 0x10, // USAGE_MAXIMUM (4K) 
    
        0x96, 0x00, 0x10, // REPORT_COUNT (4K)
    
        0x75, 0x08, // REPORT_SIZE (8)    
    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        
        // download file
        0x85, 0x0B,
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xff, // LOGICAL_MAXIMUM (255)
        0x19, 0x01, // USAGE_MINIMUM (1)
        0x2A, 0x4D, 0x10, // USAGE_MAXIMUM (4K) 
        0x96, 0x4D, 0x10, // REPORT_COUNT (4K)
        0x75, 0x08, // REPORT_SIZE (8)
        0x81, 0x02, // INPUT (Data,Var,Abs)
        0x85, 0x0B,
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xff, // LOGICAL_MAXIMUM (255)    
        0x19, 0x01, // USAGE_MINIMUM (1)
        0x2A, 0x4D, 0x10, // USAGE_MAXIMUM (4K) 
        0x96, 0x4D, 0x10, // REPORT_COUNT (4K)
        0x75, 0x08, // REPORT_SIZE (8)    
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        0xc0        // END_COLLECTION
    
#else
        0x05, 0x01, // USAGE_PAGE(User define)
        0x09, 0x00, // USAGE(User define)
        0xa1, 0x01, // COLLECTION (Application)

        0x05, 0x01, // USAGE_PAGE(1)
        0x19, 0x00, // USAGE_MINIMUM(0)
        0x29, 0xFF, // USAGE_MAXIMUM(255)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xFF, // LOGICAL_MAXIMUM (255)
        0x75, 0x08, // REPORT_SIZE (8)
        0x95, 0X40, // REPORT_COUNT (64)
        0x81, 0x02, // INPUT (Data,Var,Abs)

        0x05, 0x02, // USAGE_PAGE(2)
        0x19, 0x00, // USAGE_MINIMUM (0)
        0x29, 0xFF, // USAGE_MAXIMUM (255)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xFF, // LOGICAL_MAXIMUM (255)
        0x95, 0x40, // REPORT_COUNT (8)
        0x75, 0x08, // REPORT_SIZE (64)
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        0xc0 // END_COLLECTION
#endif
}; /* CustomHID_ReportDescriptor */

#else
/*
Short Items
    BYTE[0] :
        bit0-bit1: 0 = 0 bytes  1 = 1 byte 2 = 2 bytes 3 = 4 bytes
        bit2-bit3: 0 = Main     1 = Global 2 = Local   3 = Reserved
        bit4-bit7: Numeric expression specifying the function of the item
    BYTE[1..4]: DATA

Long items
    BYTE[0] : 0xFE
    BYTE[1] : bDataSize
    BYRE[2] : bLongItemTag
    BYTE[3..n] : DATA
*/
const uint8_t CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
 {
#if 1
    //#ifdef 0 
    0x05, 0x8c,         /* USAGE_PAGE (ST Page) */ 
    0x09, 0x01,         /* USAGE (Demo Kit) */ 
    0xa1, 0x01,         /* COLLECTION (Application) */ 
    /* 6 */ 
    
    // The Input report 
    0x09,0x03,          // USAGE ID - Vendor defined 
    0x15,0x00,          // LOGICAL_MINIMUM (0) 
    0x26,0x00, 0xFF,    // LOGICAL_MAXIMUM (255) 
    0x75,0x08,          // REPORT_SIZE (8 bit) 
    //0x95,0x40,        // REPORT_COUNT (64)
    0x95,MAX_PACK_SIZE,
    0x81,0x02,          // INPUT (Data,Var,Abs) 
    //19
    // The Output report 
    0x09,0x04,          // USAGE ID - Vendor defined 
    0x15,0x00,          // LOGICAL_MINIMUM (0) 
    0x26,0x00,0xFF,     // LOGICAL_MAXIMUM (255) 
    0x75,0x08,          // REPORT_SIZE (8 bit) 
    //0x95,0x40,        // REPORT_COUNT (64)
    0x95,MAX_PACK_SIZE,
    0x91,0x02,          // OUTPUT (Data,Var,Abs) 
    //32
    0xc0 /* END_COLLECTION */ 
    //#endif 
#else
    0x06, 0xFF, 0x00,      /* USAGE_PAGE (Vendor Page: 0xFF00) */                       
    0x09, 0x01,            /* USAGE (Demo Kit)               */    
    0xa1, 0x01,            /* COLLECTION (Application)       */            
    /* 6 */
    
    /* Led 1 */        
    0x85, 0x01,            /*     REPORT_ID (1)		     */
    0x09, 0x01,            /*     USAGE (LED 1)	             */
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */          
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */           
    0x75, 0x08,            /*     REPORT_SIZE (8)            */        
    0x95, 0x01,            /*     REPORT_COUNT (1)           */       
    0xB1, 0x82,             /*    FEATURE (Data,Var,Abs,Vol) */     

    0x85, 0x01,            /*     REPORT_ID (1)              */
    0x09, 0x01,            /*     USAGE (LED 1)              */
    0x91, 0x82,            /*     OUTPUT (Data,Var,Abs,Vol)  */
    /* 26 */
    
    /* Led 2 */
    0x85, 0x02,            /*     REPORT_ID 2		     */
    0x09, 0x02,            /*     USAGE (LED 2)	             */
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */          
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */           
    0x75, 0x08,            /*     REPORT_SIZE (8)            */        
    0x95, 0x01,            /*     REPORT_COUNT (1)           */       
    0xB1, 0x82,             /*    FEATURE (Data,Var,Abs,Vol) */     

    0x85, 0x02,            /*     REPORT_ID (2)              */
    0x09, 0x02,            /*     USAGE (LED 2)              */
    0x91, 0x82,            /*     OUTPUT (Data,Var,Abs,Vol)  */
    /* 46 */
    
    /* Led 3 */        
    0x85, 0x03,            /*     REPORT_ID (3)		     */
    0x09, 0x03,            /*     USAGE (LED 3)	             */
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */          
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */           
    0x75, 0x08,            /*     REPORT_SIZE (8)            */        
    0x95, 0x01,            /*     REPORT_COUNT (1)           */       
    0xB1, 0x82,             /*    FEATURE (Data,Var,Abs,Vol) */     

    0x85, 0x03,            /*     REPORT_ID (3)              */
    0x09, 0x03,            /*     USAGE (LED 3)              */
    0x91, 0x82,            /*     OUTPUT (Data,Var,Abs,Vol)  */
    /* 66 */
    
    /* Led 4 */
    0x85, 0x04,            /*     REPORT_ID 4)		     */
    0x09, 0x04,            /*     USAGE (LED 4)	             */
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */          
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */           
    0x75, 0x08,            /*     REPORT_SIZE (8)            */        
    0x95, 0x01,            /*     REPORT_COUNT (1)           */       
    0xB1, 0x82,            /*     FEATURE (Data,Var,Abs,Vol) */     

    0x85, 0x04,            /*     REPORT_ID (4)              */
    0x09, 0x04,            /*     USAGE (LED 4)              */
    0x91, 0x82,            /*     OUTPUT (Data,Var,Abs,Vol)  */
    /* 86 */
    
    /* key Push Button */  
    0x85, 0x05,            /*     REPORT_ID (5)              */
    0x09, 0x05,            /*     USAGE (Push Button)        */      
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */      
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */      
    0x75, 0x01,            /*     REPORT_SIZE (1)            */  
    0x81, 0x82,            /*     INPUT (Data,Var,Abs,Vol)   */   
    
    0x09, 0x05,            /*     USAGE (Push Button)        */               
    0x75, 0x01,            /*     REPORT_SIZE (1)            */           
    0xb1, 0x82,            /*     FEATURE (Data,Var,Abs,Vol) */  
         
    0x75, 0x07,            /*     REPORT_SIZE (7)            */           
    0x81, 0x83,            /*     INPUT (Cnst,Var,Abs,Vol)   */                    
    0x85, 0x05,            /*     REPORT_ID (2)              */         
                    
    0x75, 0x07,            /*     REPORT_SIZE (7)            */           
    0xb1, 0x83,            /*     FEATURE (Cnst,Var,Abs,Vol) */                      
    /* 114 */

    /* Tamper Push Button */  
    0x85, 0x06,            /*     REPORT_ID (6)              */
    0x09, 0x06,            /*     USAGE (Tamper Push Button) */      
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */      
    0x25, 0x01,            /*     LOGICAL_MAXIMUM (1)        */      
    0x75, 0x01,            /*     REPORT_SIZE (1)            */  
    0x81, 0x82,            /*     INPUT (Data,Var,Abs,Vol)   */   
    
    0x09, 0x06,            /*     USAGE (Tamper Push Button) */               
    0x75, 0x01,            /*     REPORT_SIZE (1)            */           
    0xb1, 0x82,            /*     FEATURE (Data,Var,Abs,Vol) */  
         
    0x75, 0x07,            /*     REPORT_SIZE (7)            */           
    0x81, 0x83,            /*     INPUT (Cnst,Var,Abs,Vol)   */                    
    0x85, 0x06,            /*     REPORT_ID (6)              */         
                    
    0x75, 0x07,            /*     REPORT_SIZE (7)            */           
    0xb1, 0x83,            /*     FEATURE (Cnst,Var,Abs,Vol) */  
    /* 142 */
    
    /* ADC IN */
    0x85, 0x07,            /*     REPORT_ID (7)              */         
    0x09, 0x07,            /*     USAGE (ADC IN)             */          
    0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */               
    0x26, 0xff, 0x00,      /*     LOGICAL_MAXIMUM (255)      */                 
    0x75, 0x08,            /*     REPORT_SIZE (8)            */           
    0x81, 0x82,            /*     INPUT (Data,Var,Abs,Vol)   */                    
    0x85, 0x07,            /*     REPORT_ID (7)              */                 
    0x09, 0x07,            /*     USAGE (ADC in)             */                     
    0xb1, 0x82,            /*     FEATURE (Data,Var,Abs,Vol) */                                 
    /* 161 */

    0xc0 	          /*     END_COLLECTION	             */
#endif
}; /* CustomHID_ReportDescriptor */
#endif

/* USB String Descriptors (optional) */
const uint8_t CustomHID_StringLangID[CUSTOMHID_SIZ_STRING_LANGID] =
{
    CUSTOMHID_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
};/* LangID = 0x0409: U.S. English */

const uint8_t CustomHID_StringVendor[CUSTOMHID_SIZ_STRING_VENDOR] =
{
    CUSTOMHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'S', 0, 'h', 0, 'e', 0, 'n', 0, 'Z', 0, 'h', 0, 'e', 0, 'n', 0,
    ' ', 0, 'V', 0, 'a', 0, 'n', 0, 's', 0, 't', 0, 'o', 0, 'n', 0,
    'e', 0, ' ', 0
};

const uint8_t CustomHID_StringProduct[CUSTOMHID_SIZ_STRING_PRODUCT] =
{
    CUSTOMHID_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'V', 0, 'a', 0, 'n', 0, 's', 0, 't', 0, 'o', 0, 'n', 0,
    'e', 0, ' ', 0, 'c', 0, 'u', 0, 's', 0, 't', 0, 'm', 0,
    ' ', 0, 'H', 0, 'I', 0, 'D', 0,
};

uint8_t CustomHID_StringSerial[CUSTOMHID_SIZ_STRING_SERIAL] =
  {
    CUSTOMHID_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'V', 0, 'a', 0, 'n', 0, 's', 0, 't', 0, 'o', 0, 'n', 0,
    'e', 0, '1', 0, '0', 0,
  };

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

