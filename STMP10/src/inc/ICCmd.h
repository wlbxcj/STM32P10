/// @file   ICCmd.h
/// This is for module @ref TAX_ICC
/// @author Travis Tu
/// @date   2006-Sep-1st


/**
 * @addtogroup USER_Device_Library
 * @{
 */
/**
 * @addtogroup TAX_ICC
 * @{
 */

#ifndef _ICCMD_H
#define _ICCMD_H

/**
 * @defgroup TAX_ICC_DEFINE
 * @ingroup TAX_ICC
 * @{
 */
/// enumeration for TAX Box Visible IC Card
typedef enum
{
    Fiscal_ICC  = 0x01,
    User_ICC    = 0x02,
    Fiscal_Management_ICC = 0x03
}ICCType;

/// Structure for APDU Command and response Buffer
typedef union
{
    struct
    {
        u8 CLS;
        u8 INS;
        u8 P1;
        u8 P2;
        u8 LC;
        u8 Data[256];
    }APDUCmd;
    struct
    {
        u16 SW;
        u8  Data[255];
    }APDURsp;
}APDUBufType;

/// Structure for register Number
typedef struct
{
    u8 Random[4];
    u8 ICCNB[8];
    u8 MAC1[4];
}Register_NBType;

extern APDUBufType APDUBuf;///<APDUBuffer Used for @ref TAX_ICC
/**@}*/


/**
 * @defgroup TAX_ICC_LOW_API
 * @ingroup TAX_ICC
 * @{
 */
///Get the SC Type ID from TAX Box Visible Type ID
inline SC_ID            ICCGetSizeID(ICCType ICC)
{
    if (ICC==Fiscal_ICC)
        return SMALL_SC;
    else if (ICC==User_ICC)
        return BIG_SC;
    else return BIG_SC;
}
///Send APDU at @ref USER_Device_Library level
inline u16              ICCSendAPDU(ICCType ICC, const u8* APDU, u16 length)
{
    SC_SendAPDU(ICCGetSizeID(ICC), APDU, length, &APDUBuf.APDURsp.SW, APDUBuf.APDURsp.Data);
    return APDUBuf.APDURsp.SW;
}
///Power off the SMART CARD
inline void             ICCPowerOff(ICCType ICC)    {SC_PowerOff(ICCGetSizeID(ICC));}
///Power ON the SMART CArd and get ART at Response
inline SCART_TypeDef*   ICCPowerOn(ICCType ICC)     {return SC_PowerOn(ICCGetSizeID(ICC));}
///Reset the smart Card and get ART at Response
SCART_TypeDef*          ICCReset(ICCType ICC);
/**@}*/

/**
 * @defgroup TAX_ICC_HIGH_API_COM_ICC
 * @ingroup TAX_ICC
 * @{
 */
bool ICCSelectApplication(ICCType ICC);         ///<IC Card Select Application
bool ICCCMD_Select(ICCType ICC, u8 SelectType,u8 *pSelectFile, u8 FileLen);///<IC Card Select
bool ICCReadBinary(ICCType ICC, u8 sfi, u16 offset, u8 readLength); ///<IC Card Read Binary
bool ICCUpdateBinary(ICCType ICC, u8 SFI, u16 Offset, u8 *pData, u8 Len, bool bSFI);
bool ICCReadRecord(ICCType ICC, u8 sfi, u8 recordNB, u8 recordLength); ///<IC Card Read Record
bool ICCUpdateRecord(ICCType ICC, u8 sfi, u8 recordNB, u8 recordLength, const u8* newRecord); ///<IC Card Update Record
bool ICCGetResponse(ICCType ICC, u8 length);    ///<IC Card Get Response
bool ICCGetChallenge(ICCType ICC);              ///<IC Card Get Challenge
bool ICCInternalAuthentication(ICCType ICC, u8 keyID, u8* rdm, u8* ciphRdm); ///<IC Card Internal Authentication
bool ICCExternalAuthentication(ICCType ICC, u8 keyID, u8* ciphRdm); ///<IC Card External Authentication
/**@}*/


/**
 * @defgroup TAX_ICC_HIGH_API_FISCAL_CARD
 * @ingroup TAX_ICC
 * @{
 */
bool ICCGetRegisterNB(void);        ///<IC Card Get Register Number
bool ICCTerminalRegister(u8* MAC2); ///<IC Card Terminal Register
bool ICCVerifyFiscalPin(void);      ///<IC Card Verify Fiscal Pin
bool ICCInputInvoiceNB(SingleInvoiceScrollType* InvSclNB);      ///<IC Card Input Invoice Number
bool ICCCmd_IssueInvoice(ICCIssInvCarryDataType *pCryData, u8* pFiscalCode); ///<IC Card Issue Invoice
//bool ICCIssueInvoice(INVOICEDETAILINFO * InpData,IssueInvCmdModeType mode,u8* taxContralNB);    ///<IC Card Issue Invoice
bool ICCDailyDataSign_Daily(DailyTransctionDataType* DayData);  ///<IC Card Daily Data Sign_Daily
bool ICCDailyDataSign(u8* data, u8 Length); ///<IC Card Daily DataSign
bool ICCDeclareDuty(void);                  ///<IC Card Declare Duty
bool ICCUpdateContral(u8* carryData);       ///<IC Card Update Contral
/**@}*/


/**
 * @defgroup TAX_ICC_HIGH_API_USER_CARD
 * @ingroup TAX_ICC
 * @{
 */
bool ICCRegisterSign(Register_NBType * register_NB);///<IC Card Register Sign
bool ICCDistributeInvoiceNB(void);                  ///<IC Card Distribute Invoice Number
bool ICCDataCollect(ICCDataCollectCarryDataType * DatCltCryData);///<IC Card Data Collect
/**@}*/

///Convert Smart Card Status Words to VART Error Code
bool TaxBoxSCStatusWords2UARTErrorCode(u16 sw, ICCType ICC, bool blRead);
/**@}*/
/**@}*/

#endif
