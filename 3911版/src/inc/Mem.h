/// @file   Mem.h
/// This is for module @ref MEM_MGT
/// @author Travis Tu
/// @date   2006-Sep-1st

/**
 * @addtogroup USER_Device_Library
 * @{
 */

/**
 * @addtogroup MEM_MGT
 * @{
 */

#ifndef _MEM_H_
#define _MEM_H_

/**
 * @defgroup MEM_MGT_RAM
 * @ingroup MEM_MGT
 * @{
 */
bool MemIs0(const u8* d, u16 length);                   ///<Judge whether the mem area is ZERO
void MemSet0(u8* d, u16 length);                        ///<Set all given mem area to be Zero
void MemCpy(const u8* from, u8* to, u16 length);        ///<Copy string from ram to ram only
bool MemEql(const u8* d1, const u8* d2, u16 length);    ///<Judge the mem contents whether equal
bool MemGrt(const u8* d1, const u8* d2, u16 length);      ///<Mem compare (ASCII) d1 greater than d2 array
bool MemGrtEql(const u8* d1, const u8* d2, u16 length);   ///<Mem compare (ASCII) d1 is greater than or equal to d2 array
/**@}*/

/**
 * @defgroup MEM_MGT_DATAFLASH
 * @ingroup MEM_MGT
 * @{
 */
#define RAMBSTART                           0x20004400         ///<Ram back up part start address
#define RAMBEND                             0x20004FFF         ///<Ram back up part highest address
#define CHIP_BACKUP_MEMORY_ADDR             0x0801F000         ///<Start Address of ON Chip System Memroy
#define CHIP_FLASH_PAGE_SIZE                0x00000400         ///<page size of on chip flash
#define CHIP_FLASH_FLAG_BYTES               0x0801FC00         ///<Address OF User Option Bytes Area
/**@}*/

/**
 * @defgroup MEM_MGT_SPIFLASH
 * @ingroup MEM_MGT
 * @{
 */
/*SPI Flash Map*/
#define M25P32
#ifdef M25P32
    #define SPI_FLASH_SECTOR_NB     0x40                    ///<SPI FLASH SECTOR NUMBER
#endif
#ifdef M25P64
    #define SPI_FLASH_SECTOR_NB     0x80                    ///<SPI FLASH SECTOR NUMBER
#endif

#define SPI_FLASH_PAGE_SIZE     0x100                   ///<SPI FlASH PAGE SIZE
#define SPI_FLASH_SECTOR_SIZE   0x10000                 ///<SPI FLASH SECTOER SIZE IN BYTE
#define SPI_FLASH_LOW_ADDRESS   0x50000000                               ///<The lowest address of SPI Flash, 8M Bytes SPI external Flash (128 sectors), at most first 127 sectors can be used
#define SPI_FLASH_SIZE          SPI_FLASH_SECTOR_NB * SPI_FLASH_SECTOR_SIZE///<Use SPI flash within a continuous area from begining(0x50000000),
#define SPI_FLASH_HIGH_ADDRESS  SPI_FLASH_LOW_ADDRESS + SPI_FLASH_SIZE - 1   ///<The highest address of SPI Flash, the all the rear continuous sectors are intended to be cyclic buffer sectors

#define SF_S_MASK  0xFFFF0000   ///<Mask helps to get SPI Flash Sector start address
#define SF_P_MASK  0xFFFFFF00   ///<Mask helps to get SPI Flash Page start address
#define SF_SADS_MASK 0x0000FFFF ///<Mask helps to get SPI Flash Sector inner address
#define SF_PADS_MASK 0x000000FF ///<Mask helps to get SPI Flash Page inner address

#define SF_B_SECTORS_NB 5      ///<Number of sectors at the rear of SPI Flash for cyclic SPI Flash Updating Backup Buffer
#define SF_B_SECTORS_FIRST_ID (SPI_FLASH_SECTOR_NB - SF_B_SECTORS_NB) ///<First SECTOR ID of first cyclic SPI Flash updateing Backup Buffer
#define SF_B_SECTORS_ADDRESS  (SPI_FLASH_LOW_ADDRESS + SF_B_SECTORS_FIRST_ID * SPI_FLASH_SECTOR_SIZE) ///<Low address of first cyclic SPI Flash updating Backup Buffer

#define RD_RAM_BUF_SIZE 256 ///<Ram buffer size to be used in SPI flash Movement , which should be 2 power n , 256 has the fast situation

/*SPI Flash Management Table*/
extern u8 CurSectorID4Buff; ///<current Sector ID (118~127) for backup buffer usage

void ReadSPIFlash(const u8* from, u8* to, u16 length);                      ///<Read Data from SPI Flash
void ProgramSPIFlash(const u8* from, const u8* to, u16 length);             ///<Program on spi flash accross pages, assuming the area are all '1' already
void WriteSPIFlashSingleSector(const u8* from, const u8* to, u16 length);   ///<Copy mem from ram or chip flash to spi flash, assuming the area are be constrained in single sector
void WriteSPIFlash(const u8* from, const u8* to, u16 length);               ///<copy mem from ram or chip flash to spi flash freely, this can handle two sectors accrossing situation,advise to not design a sector accrossing situation ,currently the worst situation cost 8.5S,just do not take risc even it's very tiny
/**@}*/

/**
 * @defgroup MEM_MGT_RAM_BACKUP
 * @ingroup MEM_MGT
 * @{
 */
inline bool RamIsBackuped()     {return *((u32*)CHIP_FLASH_FLAG_BYTES) == 0x00000000?TRUE:FALSE;} ///<an judge on whether the ram is backuped
void        RamBackup(void);   ///<called when sudden power lose, back up ram(Global Variable Part) to chip data flash
void        RamRestore(void);  ///<called when POS start up, restore ram (Global Variable Part)from data flash
/**@}*/


#endif /*_MEM_H_*/
/**@}*/
/**@}*/
