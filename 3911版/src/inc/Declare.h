
#ifndef _USBDECLARE_H_
#define _USBDECLARE_H_


#define DECLARETYPE_DAILYTRANS              1
#define DECLARETYPE_INVOICEDETAIL           2

/**
 * @defgroup Declare
 * @ingroup TaxBox_COMMAND_HANDLER
 * @{
 */

///Adjust the declare data
void DECLARE_DeclareInfo(void);
///Write the invoice roll data to the user card
bool DECLARE_WriteInvSclInfo(u8 *pStartEndDate);
///Write the daily transaction data for during the declare
bool DECLARE_WriteDeclareDaily(void);
///Write the invoice print data for during the declare
bool DECLARE_WriteDeclareList(void);
///Write the daily or invoice data for during the declare
bool DECLARE_WriteDeclareData(void);

u8 DECLARE_GetFileName(u8 FileType, u32 *pFileOffset, u8 *pIndexFileName, u8 *pRecordFileName);

#endif
