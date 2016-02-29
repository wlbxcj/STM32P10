/// @file   I2CRTC.h
/// This is for module @ref I2C_RTC
/// @author Travis Tu
/// @date   2006-Sep-1st


#ifndef _I2CEEPROM_H_
#define _I2CEEPROM_H_

#define I2C_EEPROM_1_ADDRESS    0xA0    ///<RTC Chip Phisical ADDRESS
#define I2C_EEPROM_2_ADDRESS    0xA2    ///<RTC Chip Phisical ADDRESS

#define I2C_EEPROM_PAGE_SIZE    32      //important
#define I2C_EEPROM_PAGE_ADDR_MASK   (0x1F)
#define I2C_EEPROM_PAGE_ID_MASK     ((u32)(~I2C_EEPROM_PAGE_ADDR_MASK))

#define I2C_EEPROM_PAGE_ID(addr)    (I2C_EEPROM_PAGE_ID_MASK & addr)
#define I2C_EEPROM_PAGE_ADDR(addr)  (I2C_EEPROM_PAGE_ADDR_MASK & addr)


#define I2C_EEPROM_SIZE_SHIFT   13     //important

#define I2C_EEPROM_SIZE         (0x1<<I2C_EEPROM_SIZE_SHIFT  )

#define I2C_EEPROM_1_SIZE       I2C_EEPROM_SIZE  //same in size
#define I2C_EEPROM_2_SIZE       I2C_EEPROM_SIZE  //same in size

#define I2C_EEPROM_1_STARTADDR  0
#define I2C_EEPROM_2_STARTADDR  I2C_EEPROM_1_SIZE

#define I2C_EEPROM_CHIP_ID(addr)  (((addr)&I2C_EEPROM_SIZE)>>I2C_EEPROM_SIZE_SHIFT)


u8 I2CEEPROMUpdateBytes(u16 address, u8* byte, u32 length);

u8 I2CEEPROMGetbytes(u16 address, u8* bytes, u32 length);



#endif /* _I2CEEPROM_H_ */
