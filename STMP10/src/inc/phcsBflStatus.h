/* 
/////////////////////////////////////////////////////////////////////////////////////////////////
//                     Copyright (c) NXP Semiconductors
//
//                       (C)NXP Electronics N.V.2004
//         All rights are reserved. Reproduction in whole or in part is
//        prohibited without the written consent of the copyright owner.
//    NXP reserves the right to make changes without notice at any time.
//   NXP makes no warranty, expressed, implied or statutory, including but
//   not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party patent,
//   copyright or trademark. NXP must not be liable for any loss or damage
//                            arising from its use.
///////////////////////////////////////////////////////////////////////////////////////////////// */

/*! 
 * \if cond_generate_filedocu 
 * \file phcsBflStatus.h
 *
 * Project: Object Oriented Library Framework status defintions.
 *
 * Workfile: phcsBflStatus.h
 * $Author: frq09147 $
 * $Revision: 1.1 $
 * $Date: Thu Sep 27 09:38:45 2007 $
 *
 * Comment:
 *  None.
 *  
 *
 * History:
 *  GK:  Generated 4. Sept. 2002
 *  MHa: Migrated to MoReUse September 2005
 * \endif
 * 
*/

#ifndef phcsBflSTATUS_H
#define phcsBflSTATUS_H

//#include <phcsTypeDefs.h>

/*! \ingroup error 
 * phcsBfl_Status_t is a signed short value, using the positive range. \n
 * High byte: Category (group) Identifier. \n
 * Low byte : Error Specifier.
 */
//typedef UINT16 phcsBfl_Status_t;
  
  
/* S U C C E S S                                                                                            */
/*! \name Success */
/*@{*/
/*! \ingroup error */
#define PH_ERR_BFL_SUCCESS                 (0x0000)            /*!< \brief Returned in case of no error when there 
                                                                    isn't any more appropriate code.        */
/*@}*/


/* C O M M U N I C A T I O N                                                                                */
/*! 
 *  \if sec_reg_communication
 *  \name Communication Errors/Status Values 
 *  \endif
 */ 
/*@{*/
/*! \if sec_reg_communication
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_IO_TIMEOUT              (0x0001)  /*!< \brief No reply received, e.g. PICC removal.    */
#define PH_ERR_BFL_CRC_ERROR               (0x0002)  /*!< \brief Wrong CRC detected by RC or library.     */
#define PH_ERR_BFL_PARITY_ERROR            (0x0003)  /*!< \brief Parity error detected by RC or library.  */
#define PH_ERR_BFL_BITCOUNT_ERROR          (0x0004)  /*!< \brief Typically, the RC reports such an error. */
#define PH_ERR_BFL_FRAMING_ERROR           (0x0005)  /*!< \brief Invalid frame format.                    */
#define PH_ERR_BFL_COLLISION_ERROR         (0x0006)  /*!< \brief Typically, the RC repors such an error.  */
#define PH_ERR_BFL_BUF_2_SMALL             (0x0007)  /*!< \brief Communication buffer size insufficient.  */
#define PH_ERR_BFL_ACCESS_DENIED           (0x0008)  /*!< \brief Access has not been granted (readonly?). */
#define PH_ERR_BFL_BUFFER_OVERFLOW         (0x0009)  /*!< \brief Attempt to write beyond the end of a 
                                                          buffer.                                  */
#define PH_ERR_BFL_RF_ERROR                (0x000A)  /*!< \brief RF is not switched on in time in active
                                                          communication mode.                      */
#define PH_ERR_BFL_PROTOCOL_ERROR          (0x000B)  /*!< \brief Mifare start bit wrong, buffer length
                                                          error.                                   */
#define PH_ERR_BFL_ERROR_NY_IMPLEMENTED    (0x000C)  /*!< \brief Feature not yet implemented.             */
#define PH_ERR_BFL_FIFO_WRITE_ERROR        (0x000D)  /*!< \brief Error caused because of interface conflict
                                                          during write access to FIFO.             */
#define PH_ERR_BFL_USERBUFFER_FULL         (0x000E)  /*!< \brief The user buffer is full, the calling 
                                                          application/routine gets the chance to
                                                          save user buffer data and start over.    */
/*@}*/
                                                               

/* I N T E R F A C E (Device as well as function parameters) errors:                               */
/*! 
 *  \if sec_reg_interface
 *  \name Interface Errors/Status Values 
 *  \endif
 */
/*@{*/
/*! \if sec_reg_interface
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_INVALID_PARAMETER       (0x0010)  /*!< \brief Parameter is invalid (range, format).    */
#define PH_ERR_BFL_UNSUPPORTED_PARAMETER   (0x0011)  /*!< \brief Parameter value/format is correct but not
                                                          supported in the current configuration.  */
#define PH_ERR_BFL_UNSUPPORTED_COMMAND     (0x0012)  /*!< \brief The device does not support the command. */
#define PH_ERR_BFL_INTERFACE_ERROR         (0x0013)  /*!< \brief Host-peripheral interface error.         */
#define PH_ERR_BFL_INVALID_FORMAT          (0x0014)  /*!< \brief The data format does not match the spec. */
#define PH_ERR_BFL_INTERFACE_NOT_ENABLED   (0x0015)  /*!< \brief This interface is currently(!) not
                                                          supported (e.g. function ptr. to NULL).  */
#define PH_ERR_BFL_UNKNOWN_HARDWARE_TYPE   (0x0016)  /*!< \brief The chosen hardware for configurations is
                                                          not known by the initialisation function. 
                                                          The default hardware (HW_1) is used instead. */
/*@}*/



/* M F  errors:                                                                                             */
/*! 
 *  \if sec_reg_mifare
 *  \name Mifare Errors/Status Values 
 *  \endif
 */
/*@{*/
/*! \if sec_reg_mifare
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_AUTHENT_ERROR           (0x0019)  /*!< \brief Authentication failure (e.g. key
                                                          mismatch).                                         */
#define PH_ERR_BFL_ACK_SUPPOSED            (0x001A)  /*!< \brief Single byte or nibble received, CRC error 
                                                          detected, possibly MF (N)ACK response.            */
#define PH_ERR_BFL_NACK_RECEIVED		   (0x001B)  /*!< \brief NACK detected */
/*@}*/


/* I S O 1 4 4 4 3 . 4 Level specific errors:                                                               */
/*! 
 *  \if sec_reg_iso_4
 *  \name ISO 14443-4 Errors/Status Values 
 *  \endif
 */
/*@{*/
/*! \if sec_reg_iso_4
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_BLOCKNR_NOT_EQUAL       (0x0020)  /*!< \brief Frame OK, but Blocknumber mismatch.               */
/*@}*/


/* N F C  Errors and Stati:                                                                                 */
/*! 
 *  \if sec_reg_nfc
 *  \name NFC Errors
 *  \endif
 */
/*@{*/
/*! \if sec_reg_nfc
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_TARGET_DEADLOCKED       (0x0029)  /*!< \brief Target has not sent any data, but RF 
                                                          (generated by the Target) is still switched on.   */
#define PH_ERR_BFL_TARGET_SET_TOX          (0x002A)  /*!< \brief Target has sent Timeout Extension Request.        */
#define PH_ERR_BFL_TARGET_RESET_TOX        (0x002B)  /*!< \brief Reset timeout-value after Timeout Extension.      */
#define PH_ERR_BFL_TARGET_DESELECTED	   (0x002C)  /*!< \brief Target has been deselected (used in the dispatcher) */
#define PH_ERR_BFL_TARGET_RELEASED  	   (0x002D)  /*!< \brief Target has been released (used in the dispatcher) */
#define PH_ERR_BFL_TARGET_NFCID_MISMATCH   (0x002E)  /*!< \brief The NFCID2 does not match withthe NFCID3 during 
                                                          activation in passive communication mode above 106 
                                                          kbps. (used in the dispatcher)                    */
/*@}*/


/* I S O 1 4 4 4 3 . 3 Level specific errors:                                                     */
/*! 
 *  \if sec_reg_iso_3
 *  \name ISO 14443-3 Errors/Status Values 
 *  \endif
 */
/*@{*/
/*! \if sec_reg_iso_3
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_WRONG_UID_CHECKBYTE     (0x0030)  /*!< \brief UID check byte is wrong.                 */
#define PH_ERR_BFL_WRONG_HALT_FORMAT       (0x0031)  /*!< \brief HALT Format error.                       */
/*@}*/


/*  I D  M A N A G E R  specific errors:                                                                    */
/*! 
 *  \if sec_reg_idmanager
 *  \name ID-Manager Errors/Status Values 
 *  \endif
 */
/*@{*/
/*! \if sec_reg_idmanager
 *  \ingroup error 
 *  \endif  */
#define PH_ERR_BFL_ID_ALREADY_IN_USE       (0x0039)  /*!< \brief ID cannot be assigned because it is already used. */
#define PH_ERR_BFL_INSTANCE_ALREADY_IN_USE (0x003A)  /*!< \brief INSTANCE cannot be assigned because it is already used. */
#define PH_ERR_BFL_ID_NOT_IN_USE           (0x003B)  /*!< \brief Specified ID is not in use.                       */
#define PH_ERR_BFL_NO_ID_AVAILABLE         (0x003C)  /*!< \brief No ID is available, all are occupied.             */
/*@}*/


/* O T H E R   E R R O R S :                                                                       */
/*! \name Other Errors/Status Values */
/*@{*/
/*! \ingroup error */
#define PH_ERR_BFL_OTHER_ERROR             (0x0070)  /*!< \brief Unspecified, error, non-categorised.     */
#define PH_ERR_BFL_INSUFFICIENT_RESOURCES  (0x0071)  /*!< \brief The system runs low of resources!        */
#define PH_ERR_BFL_INVALID_DEVICE_STATE    (0x0072)  /*!< \brief The (sub-)system is in a state which 
                                                          does not allow the operation.            */
#define PH_ERR_BFL_JOINER_TEMP_ERROR       (0x0073)  /*!< \brief Temperature error indicated by Joiner HW.*/
/*@}*/


/* U S E R   D E F I N E D   E R R O R S :  
                                                                     */
/*! \name User defined Error Values */
/*@{*/
/*! \ingroup error */
/*! User defined errors shall be in the range from 0x0080 to 0x00FF where 0x0000 is success! */
/*@}*/



/* C O M P O N E N T   C O D E S */
/*! \name Components identifiers
    \brief This group defines the component identifiers for the complete BFL. */
/*@{*/
/*! \ingroup error */
#define PH_ERR_BFL_COMP_MASK               (0xFF00)  /*!< \brief Mask to check only error code and not layer identifier */
#define PH_ERR_BFL_BAL                     (0x0100)  /*!< \brief Identifier for the Bal component. */
#define PH_ERR_BFL_REGCTL                  (0x0200)  /*!< \brief Identifier for the Register Control component. */
#define PH_ERR_BFL_IO                      (0x0300)  /*!< \brief Identifier for the IO component. */
#define PH_ERR_BFL_OPCLT                   (0x0400)  /*!< \brief Identifier for the Operation Control component. */
#define PH_ERR_BFL_POLACT                  (0x0500)  /*!< \brief Identifier for the Passive Polling Activation component. */
#define PH_ERR_BFL_I3P3A                   (0x0600)  /*!< \brief Identifier for the ISO14443-3A component. */
#define PH_ERR_BFL_AUX                     (0x0700)  /*!< \brief Identifier for the Auxiliary component. */
#define PH_ERR_BFL_NFC                     (0x0800)  /*!< \brief Identifier for the NFC component. */
#define PH_ERR_BFL_MFRD                    (0x0900)  /*!< \brief Identifier for the Mifare Reader component. */
#define PH_ERR_BFL_I3P4AACT                (0x0A00)  /*!< \brief Identifier for the ISO14443-4A Activation component. */
#define PH_ERR_BFL_I3P4                    (0x0B00)  /*!< \brief Identifier for the ISO14443-4 component. */
#define PH_ERR_BFL_IDMAN                   (0x0C00)  /*!< \brief Identifier for the ID Manager component. */
/*@}*/
 
/* Definitions for Request command. */
#define REQUEST_BITS    0x07
#define REQUEST_CMD_LENGTH    0x01
#define ATQA_LENGTH     0x02

/* Command byte definitions for Anticollision/Select functions. */
#define SELECT_CASCADE_LEVEL_1  0x93
#define SELECT_CASCADE_LEVEL_2  0x95
#define SELECT_CASCADE_LEVEL_3  0x97
#define MAX_CASCADE_LEVELS      0x03
#define SINGLE_UID_LENGTH       0x20
#define CASCADE_BIT             0x04

/* Definitions for lower Anticollision / Select functions. */
#define BITS_ONE_BYTE       0x08
#define UPPER_NIBBLE_SHIFT  0x04
#define COMPLETE_UID_BITS   0x28
#define NVB_MIN_PARAMETER   0x20
#define NVB_MAX_PARAMETER   0x70

/* Definitions for Select functions. */
#define SAK_LENGTH  0x01

#define SAK_NFC_COMPARATOR      0x40
#define SAK_TCL_COMPARATOR      0x20
#define SAK_MIFARE_COMPARATOR   0x08

/* Command and Parameter byte definitions for HaltA function. */
#define HALTA_CMD           0x50
#define HALTA_PARAM         0x00
#define HALTA_CMD_LENGTH    0x02

// The maximum number of tags that are polled for
#define NB_TAG_MAX                      5
#define MULTICELL				2
#endif /* phcsBflSTATUS_H */

