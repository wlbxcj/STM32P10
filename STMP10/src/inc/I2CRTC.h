/// @file   I2CRTC.h
/// This is for module @ref I2C_RTC
/// @author Travis Tu
/// @date   2006-Sep-1st


#ifndef _I2CRT_H_
#define _I2CRT_H_

/**
 * @addtogroup Platform_Device_Library
 * @{
 */

/**
 * @addtogroup I2C_RTC
 * @{
 */

/**
 * @defgroup I2C_RTC_DEFINE
 * @ingroup I2C_RTC
 * @{
 */
#define I2C_RTC_SCL 0x0004      ///<PIN for RTC SCL
#define I2C_RTC_SDA 0x0008      ///<PIN for RTC SDA
#define I2C_RTC_ADDRESS 0xD0    ///<RTC Chip Phisical ADDRESS
/**@}*/


/// structur for Date And Time
typedef struct
{
    u8 Century; ///<century
    u8 Year;    ///<Year
    u8 Month;   ///<Month
    u8 Day;     ///<day
    u8 Hour;    ///<Hour
    u8 Minute;  ///<Minute
    u8 Second;  ///<Second
}RTCTimeType;


/**
 * @defgroup I2C_RTC_LOW_INTERFACE
 * @ingroup I2C_RTC
 * @{
 */
///initial the RTC communication
void I2CRTCInit(void);

///update a byte string in RTC register
u8 I2CRTCUpdateBytes(u8 address, u8* byte, u8 length);
///get a byte string from RTC Register
u8 I2CRTCGetbytes(u8 address, u8* bytes, u8 length);

///Update a single byte in RTC register
inline void I2CRTCUpdateByte(u8 address,u8 byte) {I2CRTCUpdateBytes(address,&byte,1);}
///get a byte from RTC Register
inline u8   I2CRTCGetbyte(u8 address) {u8 byte; I2CRTCGetbytes(address,&byte,1); return byte;}

///lock the RTC register from modification from internal or external
inline void I2CRTCLock  (void) {I2CRTCUpdateByte(0x0C,(u8)(0x40|I2CRTCGetbyte(0x0C)));}
///unlock the RTC register make it changable
inline void I2CRTCUnlock(void) {I2CRTCUpdateByte(0x0C,(u8)(0xBF&I2CRTCGetbyte(0x0C)));}

///stop the RTC
inline void I2CRTCStop  (void) {I2CRTCUnlock();I2CRTCUpdateByte(0x01,(u8)(0x80|I2CRTCGetbyte(0x01)));}
///wait the I2C Device to be ready
void I2CWaitStandbyState(void);
/**@}*/


/**
 * @defgroup I2C_RTC_HIGH_INTERFACE
 * @ingroup I2C_RTC
 * @{
 */
///set RTC Time
void I2CRTCSetTime(RTCTimeType* showTime);
///get RTC Time
void I2CRTCGetTime(RTCTimeType* showTime);
/**@}*/


/**@}*/

/**@}*/
#endif /* _I2CRT_H_ */
