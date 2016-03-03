#ifndef _FLASHOPERATE_H_
#define _FLASHOPERATE_H_

#include "stm32f10x_lib.h"
#if 0
unsigned char flash_erase_page(unsigned char page_num);//page_num:0-4
#endif
unsigned char flash_erase_page(u32 page_num);//page_num:0-4
unsigned char flash_write_operate(u32 Address,u16 * buff,u32 len);
unsigned char flash_write_operate_nocheckcomplete(u32 Address,u16 * buff,u32 len);

unsigned char flash_read_operate(u32 Address,u8 * buff,u32 len);
unsigned char sys_flash_write_operate(u32 Address,u8 * buff,u32 len);
unsigned char sys_flash_read_operate(u32 Address,u8 * buff,u32 len);

/*
Address:0x0807D800+?

*/


/*0x0807D800-0x08080000: 10K Byte
key,parameter
// 2k byte/page
*/

/*
0x08000000-0X0802AFFF   :172k program & data: (boot 26K)+ 146
0x0802B000-0X0803B7FF     34K USER 34k->66K
                          0x0802B000-0X080337FF ->0x0802B000-0X0803B7FF
0X0803B800-0X0803FFFF     18K  SYS(PCI:49848byte+syspara:200byte)=>(PCI:16320+200) 50k->18k
                          0X08033800-0X0803FFFF->0X0803B800-0X0803FFFF
*/
#define MAX_PAGE_NUM   5 
#define StartAddr  ((u32)0x0807D800)//502k ´Óstart->end ÈÝÁ¿Îª10K
#define EndAddr    ((u32)0x08080000)//512 K top

#define APP_MAX_PAGE_NUM   84 

#define AppStartAddr   ((u32)0x08001000)
#define AppEndAddr     ((u32)0x0802AFFF)

#define USER_MAX_PAGE_NUM   33          //17=>33
#define UserStartAddr  ((u32)0x0802B000)//
#define UserEndAddr    ((u32)(0X0803B7FF-0x65))//0X080337FF=>0X0803B7FF+1

#define TscStartAddr  ((u32)0X0803B7FF-0x64)//
#define TscEndAddr    ((u32)(0X0803B7FF-1))//0X080337FF=>0X0803B7FF+1

#define SYS_MAX_PAGE_NUM   9           //25=>9
#define SysStartAddr  ((u32)0X0803B800)//0X08033800=>0X0803B800
#define SysEndAddr    ((u32)0X0803FFFF)//

#define APPLICATIONADDRESS 0x6800       //0x9000->0x6800 13/01/24  

#endif
