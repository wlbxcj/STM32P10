

#define I2C_EEPROM_PAGE_SIZE    32      //important
#define I2C_EEPROM_PAGE_ADDR_MASK   (0x1F)
#define I2C_EEPROM_PAGE_ID_MASK     ((u32)(~I2C_EEPROM_PAGE_ADDR_MASK))

#define I2C_EEPROM_PAGE_ID(addr)    (I2C_EEPROM_PAGE_ID_MASK & addr)
#define I2C_EEPROM_PAGE_ADDR(addr)  (I2C_EEPROM_PAGE_ADDR_MASK & addr)
#define I2C_EEPROM_SIZE_SHIFT   14     //important

#define I2C_EEPROM_SIZE         (0x1<<I2C_EEPROM_SIZE_SHIFT  )
void I2C_t_Init();
u8 I2CEEPROMGetbytes(u16 rd_addr,u8* rd_buf,u16 rd_length);
u8 I2CEEPROMUpdateBytes(u16 wr_addr,u8* wr_buf,u16 wr_length);
u8 I2CRTCGetbytes(u8 address, u8* bytes, u8 length);
u8 I2CRTCUpdateBytes(u8 address, u8* byte, u8 length);

