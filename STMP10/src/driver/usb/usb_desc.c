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
/* 设备描述符，可在PC端 属性--硬件ID  查看 */
const uint8_t CustomHID_DeviceDescriptor[CUSTOMHID_SIZ_DEVICE_DESC] =
{
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType   描述符类型 */
    0x00,                       /*bcdUSB            USB协议版本 */
    0x02,
    0x00,                       /*bDeviceClass      类代码 如果 这里为0，下面一
                                    个参数也得为0   0x02 为通信设备类*/
    0x00,                       /*bDeviceSubClass   子类代码 */
    0x00,                       /*bDeviceProtocol   设备所使用的协议 */
    0x40,                       /*bMaxPacketSize40  端点0 最大包长度 */
    0x88,//0x7E,//0x83,                       /*idVendor (0x0483) 厂商ID */
    0x88,//0x82,//0x04,
    0x05,//0x01,//0x53,//0x50,                       /*idProduct = 0x5750    产品 ID */
    0x00,//0x4F,//0x57,
    0x00,                       /*bcdDevice rel. 2.00   设备版本号 */
    0x02,
    1,                          /*Index of string descriptor describing
                                              manufacturer  描述厂商的字符串的索引 */
    2,                          /*Index of string descriptor describing
                                             product        描述产品的字符串的索引 */
    3,                          /*Index of string descriptor describing the
                                             device serial number */
    0x01                        /*bNumConfigurations        可能的配置数 */
}; /* CustomHID_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
/*配置描述符集 合*/
const uint8_t CustomHID_ConfigDescriptor[CUSTOMHID_SIZ_CONFIG_DESC] =
{
    /*  配置描述符  */
    0x09, /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    CUSTOMHID_SIZ_CONFIG_DESC,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01,         /* bNumInterfaces: 1 interface    该配置所支持的接口数 */
    0x01,         /* bConfigurationValue: Configuration value 该配置的值 */
    0x00,         /* iConfiguration: Index of string descriptor describing
                                 the configuration  描述该配置的字符串的索引值*/
    0xC0,         /* bmAttributes: Bus powered */
    0x96,//0x32,         /* MaxPower 300 mA: this current is used for detecting Vbus 单位为 2mA*/

    //0x03,0x09,0x03,

    /*  接口描述符 必需附着在配置描述后一并返回     */
    /************** Descriptor of Custom HID interface ****************/
    /* 09 */
    0x09,         /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,/* bDescriptorType: Interface descriptor type */
    0x00,         /* bInterfaceNumber: Number of Interface  接口编号    */
    0x00,         /* bAlternateSetting: Alternate setting   备用接口编号    */
    0x02,         /* bNumEndpoints  端点数  */
    0x03,         /* bInterfaceClass: HID   该接口使用的类  0x03为HID   
                    1：音频类，2：CDC控制类，3：人机接口类（HID），5：物理类，
                    6：图像类，7：打印机类，8：大数据存储类，9：集线器类，
                    10：CDC数据类，11：智能卡类，13：安全类，220：诊断设备类，
                    224：无线控制类，254：特定应用类，255厂商定义的设备*/
    0x00,         /* bInterfaceSubClass : 1=BOOT, 0=no boot 子类    */
    0x00,         /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse 接口协议  */
    0x00,            /* iInterface: Index of string descriptor     字符串索引值    */

    /*  HID 描述符  */
    /******************** Descriptor of Custom HID HID ********************/
    /* 18 */
    0x09,         /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID */
    0x10,         /* bcdHID: HID Class Spec release number  HID协议版本 2字节BCD 1.1版本*/
    0x01,
    0x00,         /* bCountryCode: Hardware target country  国家代码    */
    0x01,         /* bNumDescriptors: Number of HID class descriptors to follow 下级描述符的数量*/
    0x22,         /* bDescriptorType    下级描述符的类型 0x22 代码报告描述符 0x23为物理描述符*/
    (CUSTOMHID_SIZ_REPORT_DESC & 0xff),/* wItemLength: Total length of Report descriptor 下级描述符的长度*/
    ((CUSTOMHID_SIZ_REPORT_DESC >> 8) & 0xff),


    /*  端点描述符  这里有两个端点  */
    /******************** Descriptor of Custom HID endpoints ******************/
    /* 27 */
    0x07,           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */

    0x81,           /* bEndpointAddress: Endpoint Address (IN) 端点地址  */
    0x03,           /* bmAttributes: Interrupt endpoint  端点属性，低两位--0 控制传输，
                                            1 等时传输 2 批量传输 3 中断传输   */
    (MAX_PACK_SIZE & 0xFF),  //0x40,           /* wMaxPacketSize: 64 Bytes max       端点支持的最大包长度*/
    ((MAX_PACK_SIZE >> 8) & 0xFF),
    0x01,           /* bInterval: Polling Interval (10 ms)   端点查询时间    */


    /* 34 */
    0x07,	        /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
			        /*	Endpoint descriptor type */
    0x01,	        /* bEndpointAddress: */
			        /*	Endpoint Address (OUT) */
    0x03,	        /* bmAttributes: Interrupt endpoint */
    (MAX_PACK_SIZE & 0xFF),  //0x40,           /* wMaxPacketSize: 64 Bytes max       端点支持的最大包长度*/
    ((MAX_PACK_SIZE>> 8) & 0xFF),
    0x01,	        /* bInterval: Polling Interval (10 ms) */
    /* 41 */
}; /* CustomHID_ConfigDescriptor */


/*  报告描述符是一个一个条目组成的  */
#if 0//MY_DES
const u8 CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
{
#if 0
        /* USB Vendor define2 */
    
        0x05, 0x01, // USAGE_PAGE (Generic Desktop device)用途页为通用桌面设备
    
        0x09, 0x00, // USAGE (0)
    
        0xa1, 0x01, // COLLECTION (Application)//application collection  应用集合
        
    
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
const uint8_t CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
 {
#if 1
    //#ifdef 0 
    0x05, 0x8c, /* USAGE_PAGE (ST Page) */ 
    0x09, 0x01, /* USAGE (Demo Kit) */ 
    0xa1, 0x01, /* COLLECTION (Application) */ 
    /* 6 */ 
    
    // The Input report 
    0x09,0x03, // USAGE ID - Vendor defined 
    0x15,0x00, // LOGICAL_MINIMUM (0) 
    0x26,0x00, 0xFF, // LOGICAL_MAXIMUM (255) 
    0x75,0x08, // REPORT_SIZE (8) 
    //0x95,0x40, // REPORT_COUNT (64)
    0x95,MAX_PACK_SIZE,
    0x81,0x02, // INPUT (Data,Var,Abs) 
    //19
    // The Output report 
    0x09,0x04, // USAGE ID - Vendor defined 
    0x15,0x00, // LOGICAL_MINIMUM (0) 
    0x26,0x00,0xFF, // LOGICAL_MAXIMUM (255) 
    0x75,0x08, // REPORT_SIZE (8) 
    //0x95,0x40, // REPORT_COUNT (64)
    0x95,MAX_PACK_SIZE,
    0x91,0x02, // OUTPUT (Data,Var,Abs) 
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

