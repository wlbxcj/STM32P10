#ifndef _PICC_H
#define _PICC_H

#include "base.h"
#include "vosapi.h"
//13/08/07
//#include "..\drv\picc\EMV\emv_error_codes.h"
#include "..\driver\picc_as3911\EMV\emv_error_codes.h"

#undef FALSE
#undef TRUE

#include "stm32f10x_type.h"

#ifndef BOOL
#define BOOL	int
#endif

#ifndef  bool_t
#define  bool_t BOOL  //bool->BOOL
#endif


#define LOGGER_ON		1
#define LOGGER_OFF		0

//#define USE_LOGGER  LOGGER_ON
#define USE_LOGGER  LOGGER_OFF

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#if (USE_LOGGER == LOGGER_ON)
#define LOG s_UartPrint/*!< macro used for printing debug messages */
//#define LOGDUMP dbgHexDump /*!< macro used for dumping buffers */
/* debug assertion macro: 
 * If condition mustBeTrue is not fulfilled, an endless (idle) loop is entered
 */
#define DBG_ASSERT(mustBeTrue) {                                            \
    if (!(mustBeTrue))                                                      \
    {                                                                       \
        while (1);                                                          \
    }                                                                       \
} 
#else
#define LOG(...)  /*!< macro used for printing debug messages if USE_LOGGER is set */
//#define LOGDUMP(...) /*!< macro used for dumping buffers if USE_LOGGER is set */
#define DBG_ASSERT(mustBeTrue)  {}
#endif



#define U8_C(x)     (x) /*!< \ingroup datatypes
                         Define a constant of type u8 */
#define S8_C(x)     (x) /*!< \ingroup datatypes
                         Define a constant of type s8 */
#define U16_C(x)    (x) /*!< \ingroup datatypes
                         Define a constant of type u16 */
#define S16_C(x)    (x) /*!< \ingroup datatypes
                         Define a constant of type s16 */
#define U32_C(x)    (x##UL) /*!< \ingroup datatypes
                             Define a constant of type u32 */
#define S32_C(x)    (x##L) /*!< \ingroup datatypes
                            Define a constant of type s32 */
#define U64_C(x)    (x##ULL) /*!< \ingroup datatypes
                              Define a constant of type u64 */
#define S64_C(x)    (x##LL) /*!< \ingroup datatypes
                             Define a constant of type s64 */
#define UMWORD_C(x) (x) /*!< \ingroup datatypes
                         Define a constant of type umword */
#define MWORD_C(x)  (x) /*!< \ingroup datatypes
                         Define a constant of type mword */



/*! \defgroup errorcodes Basic application error codes
 * Error codes to be used within the application.
 * They are represented by an s8
 */
#define ERR_NONE   0 /*!< \ingroup errorcodes
               no error occured */
#define ERR_NOMEM -1 /*!< \ingroup errorcodes
               not enough memory to perform the requested
               operation */
#define ERR_BUSY  -2 /*!< \ingroup errorcodes
               device or resource busy */
#define ERR_IO    -3 /*!< \ingroup errorcodes
                generic IO error */
#define ERR_TIMEOUT -4 /*!< \ingroup errorcodes
                error due to timeout */
#define ERR_REQUEST -5 /*!< \ingroup errorcodes
                invalid request or requested
                function can't be executed at the moment */
#define ERR_NOMSG -6 /*!< \ingroup errorcodes
                       No message of desired type */
#define ERR_PARAM -7 /*!< \ingroup errorcodes
                       Parameter error */

#define ERR_LAST_ERROR -32



#define PICC_OK                (0)
#define PICC_ChipIDErr         (-3500)
#define PICC_OpenErr           (-3501)
#define PICC_NotOpen           (-3502)
#define PICC_ParameterErr      (-3503)  //参数错误
#define PICC_TxTimerOut        (-3504)
#define PICC_RxTimerOut        (-3505)
#define PICC_RxDataOver        (-3506)
#define PICC_TypeAColl         (-3507)
#define PICC_FifoOver          (-3508)
#define PICC_CRCErr            (-3509)
#define PICC_SOFErr            (-3510)
#define PICC_ParityErr         (-3511)
#define PICC_KeyFormatErr      (-3512)

#define PICC_RequestErr        (-3513)
#define PICC_AntiCollErr       (-3514)
#define PICC_UidCRCErr         (-3515)
#define PICC_SelectErr         (-3516)
#define PICC_RatsErr           (-3517)
#define PICC_AttribErr         (-3518)
#define PICC_HaltErr           (-3519)
#define PICC_OperateErr        (-3520)
#define PICC_WriteBlockErr     (-3521)
#define PICC_ReadBlockErr      (-3522)
#define PICC_AuthErr           (-3523)
#define PICC_ApduErr           (-3524)
#define PICC_HaveCard          (-3525)   //有卡
#define PICC_Collision         (-3526) //多卡
#define PICC_CardTyepErr       (-3527) //A 卡发送RATS
#define PICC_CardStateErr      (-3528)



s16 emvPiccReset2 ();
s16 emvPiccRemove2();
s16 emvPiccCheck2(uchar mode,uchar *CardType,uchar *SerialNo);
s16 emvPiccCommand2 (APDU_SEND *ApduSend, APDU_RESP *ApduResp);
s16 emvPiccClose2(void);
s16 emvPiccOpen2(void);
#ifndef AS3911_M //13/08/07
#define Lib_PiccOpen emvPiccOpen2
#define Lib_PiccClose emvPiccClose2 
#define Lib_PiccCommand emvPiccCommand2 
#define Lib_PiccCheck emvPiccCheck2 
#define Lib_PiccRemove emvPiccRemove2
#define Lib_PiccReset emvPiccReset2
#endif

//extern APDU_SEND ApduSend_;
//extern APDU_RESP ApduResp_;



inline void sleepMilliseconds(unsigned int milliseconds)
{
//    timerStart(milliseconds);
//    while(timerIsRunning())
//        ;
	DelayMs(milliseconds);
}
 
#endif

