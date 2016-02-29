/*
 *****************************************************************************
 * Copyright @ 2009 by austriamicrosystems AG                                *
 * All rights are reserved.                                                  *
 *                                                                           *
 * Reproduction in whole or in part is prohibited without the written consent*
 * of the copyright owner. Austriamicrosystems reserves the right to make    *
 * changes without notice at any time. The software is provided as is and    *
 * Austriamicrosystems makes no warranty, expressed, implied or statutory,   *
 * including but not limited to any implied warranty of merchantability or   *
 * fitness for any particular purpose, or that the use will not infringe any *
 * third party patent, copyright or trademark. Austriamicrosystems should    *
 * not be liable for any loss or damage arising from its use.                *
 *****************************************************************************
 */

/*
 * PROJECT: AS3911 firmware
 * $Revision: $
 * LANGUAGE: ANSI C
 */

/*! \file emv_layer4.c
 *
 * \author Oliver Regenfelder
 *
 * \brief EMV compliant ISO14443-4 data transmission.
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "emv_main.h"
#include "emv_layer4.h"
#include "emv_hal.h"
#include "emv_standard.h"
#include "emv_display.h"
#include "emv_response_buffer.h"
#include "emv_poll.h"


#include "..\AS3911\as3911_def.h"
#include "..\AS3911\as3911_gain_adjustment.h"
#include "..\AS3911\as3911_io.h"
#include "..\AS3911\as3911_irq.h"

#include "picc.h"

//#define PWR_CON 1
/*
******************************************************************************
* DEFINES
******************************************************************************
*/

/*! Maximum number of retransmission for the layer 4 protocol. */
#define EMV_MAX_RETRANSMISSIONS     2
/*! Maximum number of S(WTX) requests allowed for the PICC during a single layer-4 block transfer. */
#define EMV_MAX_NUM_CONSECUTIVE_SWTX_REQUESTS_AFTER_RETRANSMISSION_REQUESTS 2
/*! ISO14443-4 I-block PCB with the chaining bit set. */
#define EMV_PCB_IBLOCK_CHAINED      0x12
/*! ISO14443-4 I-block PCB with the chaining bit cleared. */
#define EMV_PCB_IBLOCK_UNCHAINED    0x02
/*! ISO14443-4 acknowledge R-block PCB. This constant needs to be xored with the current block number. */
#define EMV_PCB_RACK                0xA2
/*! ISO14443-4 not acknowledge R-block PCB. This constant needs to be xored with the current block number. */
#define EMV_PCB_RNAK                0xB2
/*! ISO14443-4 WTX request or response S-block PCB. This must not be combined with the current block number. */
#define EMV_PCB_SWTX                0xF2
/*! Bitmask for the chaining bit of an ISO14443-4 I-block PCB. */
#define EMV_PCB_CHAINING_BIT_MASK   0x10
/*! Bitmask for the block number of an ISO14443-4 R-block or I-block PCB. */
#define EMV_PCB_BLOCK_NUMBER_MASK   0x01
/*! Bitmask for the WTXM bits of the inf byte of an WTX request or response. */
#define EMV_WTXM_MASK               0x3F
/*!
 *****************************************************************************
 * Maximum allowed value for the WTXM of an WTX request. If a value above
 * EMV_MAX_WTXM is requested, then EMV_MAX_WTXM will be used instead of the
 * requested value.
 *****************************************************************************
 */
#define EMV_MAX_WTXM                59
#define s_UartPrint trace_debug_printf //sxl

/*
******************************************************************************
* MACROS
******************************************************************************
*/
s16 emvPiccReset (); //13/11/04
/*
******************************************************************************
* LOCAL DATA TYPES
******************************************************************************
*/

/*!
 *****************************************************************************
 * Retransmission request type to use when data from the PICC is not received
 * correctly (e.g. timeut or transmission error).
 *****************************************************************************
 */
typedef enum EMVRetransmissionRequestType
{
    /*! Retransmissions are requested with an R(ACK) block. */
    EMV_RETRANSMISSION_REQUEST_RACK,
    /*! Retransmissions are requested with an R(NAK) block. */
    EMV_RETRANSMISSION_REQUEST_RNAK
} EMVRetransmissionRequestType_t;

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*! EMV PICC data of the card used for ISO14443-4 communication */
static EmvPicc_t  _emvPicc;
static EmvPicc_t  *emvPicc;

/*! ISO14443-4 block number */
static u8 emvBlockNumber;

/*!
 *****************************************************************************
 * Buffer for low level data blocks (R,S and I blocks). This buffer is used
 * for transmission AND reception.
 *****************************************************************************
 */
static u8 emvBlockBuffer[EMV_FSD_MIN_PCD];

bool OPENED_FLAG = FALSE;

/*
******************************************************************************
* LOCAL TABLES
******************************************************************************
*/
static AS3911ModulationLevelAutomaticAdjustmentData_t mainModulationAutomaticAdjustmentData;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
extern int emvTypeA;
extern int emvTypeB;
/*!
 *****************************************************************************
 * \brief Transceive a single low level block of the ISO1443-4 protocol and
 * employ error handling.
 *
 * Transceive a single low level block of the ISO14443-4 protocol. Error
 * handling is performed according to requirement 10.13. The parameter
 * \a retransmissionRequestType is used to distinguish between error
 * handling after a non chaining block has been received and error handling
 * after a chaining block has beeen received.
 *
 * \param[in] pcb The PCB of the block.
 * \param[in] inf Pointer to the INF field of the block.
 * \param[in] infLength Length of the INF field in bytes.
 * \param[out] response Buffer for the piccs response.
 * \param[in] maxResponseLength Size of the response buffer in bytes.
 * \param[out] responseLength Length of the received response in bytes.
 * \param[in] retransmissionRequestType Defines the PCB Block to use for
 * retransmission requests.
 *
 * \return EMV_ERR_OK: No error, response APDU received.
 * \return EMV_ERR_PROTOCOL: Protocl error during reception of the response
 * APDU.
 * \return EMV_ERR_TRANSMISSION: Transmission error during reception of the
 * response APDU.
 * \return EMV_ERR_TIMEOUT: No response APDU receied, or a timeout occured during
 * reception of the response APDU.
 * \return EMV_ERR_INTERNAL: Internal buffer overflow during reception of the
 * response APDU.
 *****************************************************************************
 */
static s16 emvTransceiveBlock(u8 pcb, const u8 *inf, u32 infLength,
    u8 *response, u32 maxResponseLength, u32 *responseLength,
    enum EMVRetransmissionRequestType retransmissionRequestType);
/*
******************************************************************************
* GLOBAL VARIABLE DEFINITIONS
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

s16 emvInitLayer4(EmvPicc_t *picc)
{
	if(picc->fwi<=4)
		picc->fwi=14;
	else if(picc->fwi<=7)
		picc->fwi*=2;
	
	memcpy(&_emvPicc, picc, sizeof(EmvPicc_t));
	emvPicc = &_emvPicc;
	emvBlockNumber = 0;

    return EMV_ERR_OK;
}
 




static u16 emvConvertCarrierCyclesToMilliseconds(u32 num_cycles)
{
    return (num_cycles / 13560) + 1;
}

s16 emvTransceiveApdu(const u8 *apdu, u32 apduLength, u8 *response, u32 maxResponseLength, u32 *responseLength)
{
    u32 numApduBytesTransmitted = 0;
    u32 numResponseBytesReceived = 0;
    u32 index = 0;
    u32 piccResponseLength = 0;
    u16 fsc = 0;
    u8 pcb = 0;
    u8 numIBlockRetransmissions = 0;
    s16 error = EMV_ERR_OK;
    bool_t awaitingAnotherIBlock = FALSE;

    switch (emvPicc->fsci)
    {
    case 0: fsc = 16; break;
    case 1: fsc = 24; break;
    case 2: fsc = 32; break;
    case 3: fsc = 40; break;
    case 4: fsc = 48; break;
    case 5: fsc = 64; break;
    case 6: fsc = 96; break;
    case 7: fsc = 128; break;
    case 8: fsc = 256; break;
    default: fsc = 256; break;
    }

    numIBlockRetransmissions = 0;
    while ((apduLength - numApduBytesTransmitted) > (fsc - 3))
    {
        /* I-block with chaining. */
        pcb = EMV_PCB_IBLOCK_CHAINED | emvBlockNumber;

        error = emvTransceiveBlock(pcb, &apdu[numApduBytesTransmitted], fsc - 3,
                emvBlockBuffer, sizeof(emvBlockBuffer), &piccResponseLength, EMV_RETRANSMISSION_REQUEST_RNAK);
        //s_UartPrint("emvTransceiveBlock=%d\r\n",error);//sxl
        if (EMV_ERR_OK != error)
        {
            /* Pass on errors to higher layer. */
            return error;
        }

        /* PICC response must have a length of 1 (R(ACK)). */
        if (piccResponseLength != 1)
        {
            return EMV_ERR_PROTOCOL;
        }

        /* Response must be a R(ACK) */
        if ((emvBlockBuffer[0] & ~EMV_PCB_BLOCK_NUMBER_MASK) != 0xA2)
        {
            /* The received block is not a proper R(ACK). */
            return EMV_ERR_PROTOCOL;
        }

        /* Check block number of the received R(ACK) */
        if ((emvBlockBuffer[0] & EMV_PCB_BLOCK_NUMBER_MASK) != emvBlockNumber)
        {
            /* Non matching block number. Retransmit last I-block. */
            if (numIBlockRetransmissions >= EMV_MAX_RETRANSMISSIONS)
            {
                /* Too many retransmissions already, report protocol error. */
                return EMV_ERR_PROTOCOL;
            }
            else
            {
                numIBlockRetransmissions++;
                /* Do not toggle the block number and also do not increase the
                 * number of transmitted apdu bytes. This will cause the next I-block
                 * to be equal to the last one.
                 */
            }
        }
        else
        {
            /* Received proper R(ACK) =>  Toggle block number. */
            emvBlockNumber ^= 0x01;
            numApduBytesTransmitted += fsc - 3;
            numIBlockRetransmissions = 0;
        }
    }

    /* Send an I-block without chaining. Either as the last I-block of a chained sequence.
     * or as a single I-block if the APDU is small enough.
     */
    numIBlockRetransmissions = 0;
    while (1)
    {
        pcb = EMV_PCB_IBLOCK_UNCHAINED | emvBlockNumber;

        error = emvTransceiveBlock(pcb, &apdu[numApduBytesTransmitted]
                                , apduLength - numApduBytesTransmitted, emvBlockBuffer, sizeof(emvBlockBuffer)
                                , &piccResponseLength, EMV_RETRANSMISSION_REQUEST_RNAK);
        //s_UartPrint("emvTransceiveBlock1=%d\r\n",error); //sxl
        if (EMV_ERR_OK != error)
        {
            /* Pass on errors to higher layer. */
            return error;
        }

        /* If the response is an R(ACK) block, then we must check whether a retransmission
         * has to be performed.
         */
        if ((emvBlockBuffer[0] & ~EMV_PCB_BLOCK_NUMBER_MASK) != EMV_PCB_RACK)
        {
            /* The received block is not an R(ACK).
             * Break the while loop and continue with the handling of the response I-block.
             */
            break;
        }

        /* Check block number of the received R(ACK) */
        if ((emvBlockBuffer[0] & EMV_PCB_BLOCK_NUMBER_MASK) != emvBlockNumber)
        {
            /* Non matching block number. Retransmit last I-block. */
            if (numIBlockRetransmissions >= EMV_MAX_RETRANSMISSIONS)
            {
                /* Too many retransmissions already, report protocol error. */
                return EMV_ERR_PROTOCOL;
            }
            else
            {
                numIBlockRetransmissions++;
                /* Do not toggle the block number and also do not increase the
                 * number of transmitted apdu bytes. This will cause the next I-block
                 * to be equal to the last one.
                 */
            }
        }
        else
        {
            /* Received an R(ACK) with correct block number as response to an unchained
             * I-block. This is a protocol error.
             */
            return EMV_ERR_PROTOCOL;
        }
    }

    /* Handle the answer of the card to the APDU. */
    do
    {
        /* Pass on errors to the higher layer. */
        if (EMV_ERR_OK != error)
            return error;

        /* An I-block must have a size > 0. */
        if (0 == piccResponseLength)
            return EMV_ERR_PROTOCOL;

        /* The received block must be a proper I-block. */
        if (  ((emvBlockBuffer[0] & ~EMV_PCB_BLOCK_NUMBER_MASK) != EMV_PCB_IBLOCK_UNCHAINED)
           && ((emvBlockBuffer[0] & ~EMV_PCB_BLOCK_NUMBER_MASK) != EMV_PCB_IBLOCK_CHAINED))
            return EMV_ERR_PROTOCOL;

        /* The received block must have a correct block number. */
        if ((emvBlockBuffer[0] & EMV_PCB_BLOCK_NUMBER_MASK) != emvBlockNumber)
            return EMV_ERR_PROTOCOL;

        /* Received proper I-block => toggle block number. */
        emvBlockNumber ^= 0x01;

        /* Check for response buffer overflow */
        if (numResponseBytesReceived + piccResponseLength > maxResponseLength) {
            /* Copy as much bytes as possible into the response buffer. */
            for (index = 0; index < maxResponseLength - numResponseBytesReceived; index++)
                response[numResponseBytesReceived + index] = emvBlockBuffer[1 + index];

            /* Signal buffer overflow to caller. */
            return EMV_ERR_INTERNAL;
        }

        for (index = 0; index < piccResponseLength - 1; index++)
            response[numResponseBytesReceived + index] = emvBlockBuffer[1 + index];
        numResponseBytesReceived += index;

        if (emvBlockBuffer[0] & EMV_PCB_CHAINING_BIT_MASK)
        {
            /* Chaining bit set. Send R(ACK) and receive next I-block. */
            emvBlockBuffer[0] = EMV_PCB_RACK | emvBlockNumber;

            error = emvTransceiveBlock(emvBlockBuffer[0], NULL, 0, emvBlockBuffer, sizeof(emvBlockBuffer),
                        &piccResponseLength, EMV_RETRANSMISSION_REQUEST_RACK);
            awaitingAnotherIBlock = TRUE;
        }
        else
            awaitingAnotherIBlock = FALSE;

    } while (awaitingAnotherIBlock); /* Continue while the chained receive is in progress. */

    *responseLength = numResponseBytesReceived;
    return EMV_ERR_OK;
}


/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/

static s16 emvTransceiveBlock(u8 pcb, const u8 *inf, u32 infLength,
    u8 *response, u32 maxResponseLength, u32 *responseLength,
    enum EMVRetransmissionRequestType retransmissionRequestType)
{
    u32 index = 0;
    s16 error = EMV_HAL_ERR_OK;
    u8 numRetransmissions = 0;
    u8 numConsecutiveSwtxRequestsAfterRnakReceived = 0;
    bool_t lastBlockWasRetransmissionRequest = FALSE;
    u32 fwtInCarrierCycles = 0;

    
    if (infLength + 3 > sizeof(emvBlockBuffer))
    {
        /* The given block is bigger than the internal buffer.
         * This should actually never happen.
         */
        return EMV_ERR_INTERNAL;
    }
    
    /* Construct layer 4 low level block */
    emvBlockBuffer[0] = pcb;
    for (index = 0; index < infLength; index++)
    {
        emvBlockBuffer[1 + index] = inf[index];
    }
    
    /* Calculate frame wait time */
    fwtInCarrierCycles = (4096L + 384) << emvPicc->fwi;

    error = emvHalTransceive(emvBlockBuffer, infLength + 1, response,
        maxResponseLength , responseLength, fwtInCarrierCycles,
        EMV_HAL_TRANSCEIVE_WITH_CRC);
    
    numRetransmissions = 0;
    numConsecutiveSwtxRequestsAfterRnakReceived = 0;
    lastBlockWasRetransmissionRequest = FALSE;
    //s_UartPrint("emvHalTransceive1=%d\r\n",error);//sxl
    while (numRetransmissions <EMV_MAX_RETRANSMISSIONS)
    {
        /* Mesages larger than the reported reader buffer size (FSD) must be
         * treated as a protocol error.
         * Note: There have been diverting statements from FIME regarding
         * treatment of recieved block exceeding FSD. One statement was to
         * tread it as a protocol error (which implies that no further
         * retransmissions are requested). Another statement was to treat this
         * as a transmission error which might require a retransmission request
         * to be send.
         *
         * This implementation treats too long messages as protocol error.
         */
      // s_UartPrint("numRetransmissions1 < EMV_MAX_RETRANSMISSIONS\r\n");//sxl
       
        if (*responseLength > EMV_FSD_MIN_PCD)
            return EMV_ERR_PROTOCOL;
        else if (EMV_HAL_ERR_OVERFLOW == error)
            return EMV_ERR_PROTOCOL;
        else if (EMV_HAL_ERR_OK == error)
        {
          
            if (response[0] == 0xF2)
            {
                /* Handle frame wait extension requests. */
                u8 requestedWtxm = 0;
                u8 usedWtxm = 0;
                u8 swtxResponse[2];
                u32 fwtInCarrierCycles = 0;
                //s_UartPrint("numRetransmissions < EMV_MAX_RETRANSMISSIONS\r\n");//sxl
                /* The EMV standard limits the number of consecutive sequences of
                 * 'retransmission request followed by S(WTX) request' but there is
                 * no limit to the number of S(WTX) requests received after any
                 * other block.
                 */
                if (lastBlockWasRetransmissionRequest)
                    numConsecutiveSwtxRequestsAfterRnakReceived++;
                else
                    numConsecutiveSwtxRequestsAfterRnakReceived = 0;

                /* According to FIME reception of an S(WTX) request counts as reception of a correct frame.
                 * Thus the counter for consecutive transmission errors must be reseted.
                 */
                numRetransmissions = 0;

                /* Check for proper S(WTX) request size (2 byte + 2 bytes CRC). */
                if (*responseLength != 2 + 2)
                    return EMV_ERR_PROTOCOL;

                /* An excess of S(WTX) requests after retransmission requests must
                 * be treated as timeout error.
                 * See 10.3.5.5. The explanation from this clause also applies to
                 * 10.3.5.8 although consecutive S(WTX) requests are not mentioned there.
                 */
                if (numConsecutiveSwtxRequestsAfterRnakReceived > EMV_MAX_NUM_CONSECUTIVE_SWTX_REQUESTS_AFTER_RETRANSMISSION_REQUESTS)
                 
                    return EMV_ERR_TIMEOUT;

                requestedWtxm = response[1] & EMV_WTXM_MASK;

                /* Check validity of wtxm value and adjust range. */
                if (requestedWtxm == 0)
                    return EMV_ERR_PROTOCOL;
                else if (requestedWtxm > EMV_MAX_WTXM)
                    usedWtxm = EMV_MAX_WTXM;
                else
                    usedWtxm = requestedWtxm;

                /* Calculate requested frame wait time in carrier cycles. */
                fwtInCarrierCycles = ((4096L + 384) << emvPicc->fwi) * usedWtxm;

                /* In the EMV standard there is no clear definition on what to do
                 * if the requested FWT_TEMP exceeds FWT_MAX.
                 * But, according to FIME the PCD shall limit FWT_TEMP to FWT_MAX if that
                 * happens.
                 */
                if (fwtInCarrierCycles > ((4096L + 384) << EMV_FWI_MAX_PCD))
                    fwtInCarrierCycles = ((4096L + 384) << EMV_FWI_MAX_PCD);

                swtxResponse[0] = 0xF2;
                swtxResponse[1] = requestedWtxm;

s_UartPrint("swtxResponse[1]=[%02x]fwtInCarrierCycles[%d]\r\n", requestedWtxm,fwtInCarrierCycles);

                lastBlockWasRetransmissionRequest = FALSE;
                error = emvHalTransceive(&swtxResponse[0], 2 , response, maxResponseLength
                    , responseLength, fwtInCarrierCycles, EMV_HAL_TRANSCEIVE_WITH_CRC);
              // s_UartPrint("emvHalTransceive2=%d\r\n",error);//sxl
                /* ToDo: Check whether S(WTX) pairs are counted as retransmissions. */
            }
            else
            {
                /* The CRC bytes are not reported in the response length. */
                *responseLength -= 2;
                return EMV_ERR_OK;
            }
        }
        else
        {
            /* Check for a timeout after consecutive R(NAK), S(WTX), S(WTX) response chains.
             * See PCD 10.3.5.5 for details.
             *
             * Note: The >= is there for a reason. Please think _twice_ before changing it into >.
             */
            if ((EMV_HAL_ERR_TIMEOUT == error)
               && (numConsecutiveSwtxRequestsAfterRnakReceived >= EMV_MAX_NUM_CONSECUTIVE_SWTX_REQUESTS_AFTER_RETRANSMISSION_REQUESTS))
              
                return EMV_ERR_TIMEOUT;

            /* All (other) error conditions lead to a retransmission request. */
            if (EMV_RETRANSMISSION_REQUEST_RACK == retransmissionRequestType)
                emvBlockBuffer[0] = EMV_PCB_RACK | emvBlockNumber;
            else if(EMV_RETRANSMISSION_REQUEST_RNAK == retransmissionRequestType)
                emvBlockBuffer[0] = EMV_PCB_RNAK | emvBlockNumber;
            else
                return EMV_ERR_INTERNAL;

            lastBlockWasRetransmissionRequest = TRUE;
            error = emvHalTransceive(emvBlockBuffer, 1, response, maxResponseLength
                , responseLength, fwtInCarrierCycles, EMV_HAL_TRANSCEIVE_WITH_CRC);
            numRetransmissions++;

            
        }
       
    }
      //s_UartPrint("emvHalTransceive3=%d\r\n",error);//sxl
    /* Blocks greater than the FSD must be considered as a protocol error. */
    if (*responseLength > EMV_FSD_MIN_PCD)
        return EMV_ERR_PROTOCOL;
    else if (EMV_HAL_ERR_OVERFLOW == error)
        return EMV_ERR_PROTOCOL;
  
    else if (EMV_HAL_ERR_OK == error)
    {
        /* The last retransmitted block is not allowed to be an S(WTX) request. */

        if (response[0] == EMV_PCB_SWTX)
        {

       
          return EMV_ERR_TIMEOUT; 
        }


        /* The CRC bytes are not reported as part of the repsonse length. */
        *responseLength -= 2;
        return EMV_ERR_OK;
    }
    else if (EMV_HAL_ERR_TIMEOUT == error)
      
        return EMV_ERR_TIMEOUT;
    else
        return EMV_ERR_TRANSMISSION;
}
#ifdef PWR_CON
void emvPiccGoToSleepMode ()
{

	as3911ClearInterrupts(AS3911_IRQ_MASK_WCAP);
	as3911EnableInterrupts(AS3911_IRQ_MASK_WCAP);
//	as3911ExecuteCommand(AS3911_CMD_START_WUP_TIMER);
	as3911WriteRegister(AS3911_REG_OP_CONTROL, 0x04);
}






void emvPiccGoToExchange ()
{
	as3911DisableInterrupts(AS3911_IRQ_MASK_WCAP);
	/* MCU_CLK and LF MCU_CLK off, 27MHz XTAL */
	as3911WriteRegister(AS3911_REG_IO_CONF1, 0x0F);
	/* Enable Oscillator, Transmitter and receiver. */
	as3911WriteRegister(AS3911_REG_OP_CONTROL, 0xC8);
	sleepMilliseconds(5);

	
	/* Enable AM/PM receiver signal output on CSI/CSO. */
	//as3911WriteTestRegister(AS3911_REG_IO_CONF2, 0x03);
	
	/* Enable AS3911 IRQ handling. */
	AS3911_IRQ_ON();
	/* Voltage Regulator setup. */
	as3911ModifyRegister(AS3911_REG_IO_CONF2, 0x80, 0x00);
	
	/* Disable the voltage regulator. */
	//as3911ModifyRegister(AS3911_REG_IO_CONF2, 0x40, 0x40);
	/* Enable the voltage regulator. */
	//as3911ModifyRegister(AS3911_REG_IO_CONF2, 0x40, 0x00);
	as3911ModifyRegister(AS3911_REG_VSS_REGULATOR_CONF, 0xF8, 0xD0);
	/* Antenna trim setup. */
	as3911ModifyRegister(AS3911_REG_ANT_CAL_CONF, 0xF8, 0x88);
	//as3911ExecuteCommand(AS3911_CMD_CALIBRATE_ANTENNA); 
	/* Receive channel setup. */
	as3911ModifyRegister(AS3911_REG_OP_CONTROL, 0x30, 0x00);
	as3911ModifyRegister(AS3911_REG_RX_CONF1, 0x80, 0x00);
	/* First stage gain reduction. */
	as3911ModifyRegister(AS3911_REG_RX_CONF3, 0xFC, 0xc0 | 0x18);
	/* Second/Third stage gain reduction. */
	as3911WriteRegister(AS3911_REG_RX_CONF4, 0x11);
	as3911ExecuteCommand(AS3911_CMD_CLEAR_SQUELCH);

	/* Automatic gain control and squelch. */
	as3911ModifyRegister(AS3911_REG_RX_CONF2, 0x1F, 0x12);
			
	/* Gain adjustment based on lookup table. */
	/* Readout gain lookup table. */
	as3911SetGainMode(AS3911_GAIN_FIXED, NULL);
      /* Read ISO14443B modulation depth mode byte. */	
	as3911WriteRegister(AS3911_REG_AM_MOD_DEPTH_CONF, 0x80);
	as3911WriteRegister(AS3911_REG_RFO_AM_ON_LEVEL, 0x88); 
	emvHalSetAs3911TypeBModulationMode(AS3911_MODULATION_LEVEL_FIXED, NULL);
	
	LOG("EMV: settings applied\n");
	displayRegisterValue(AS3911_REG_IO_CONF2);
	displayRegisterValue(AS3911_REG_VSS_REGULATOR_CONF);
	displayRegisterValue(AS3911_REG_ANT_CAL_CONF);
	displayRegisterValue(AS3911_REG_OP_CONTROL);
	displayRegisterValue(AS3911_REG_RX_CONF1);
	displayRegisterValue(AS3911_REG_RX_CONF2);
	displayRegisterValue(AS3911_REG_RX_CONF3);
	displayRegisterValue(AS3911_REG_RX_CONF4);
	displayRegisterValue(AS3911_REG_AM_MOD_DEPTH_CONF);
	displayRegisterValue(AS3911_REG_RFO_AM_ON_LEVEL);
	//displayTestRegisterValue(AS3911_REG_ANALOG_TEST);
}
#endif


/*!
 *****************************************************************************
 * \brief Display the value of an AS3911 register on the debug output.
 *
 * \param[in] address Address of the register whose value shall be display.
 *****************************************************************************
 */
static void displayRegisterValue(u8 address)
{
	u8 value = 0;
	as3911ReadRegister(address, &value);
	LOG("REG: 0x%hhx: 0x%hhx\n", address, value);
}

bool emvPiccIsRuning(void)
{
	return as3911Runing ? TRUE : FALSE;
}

bool emvPiccIsOpening(void)
{
	return OPENED_FLAG ? TRUE : FALSE;
}

s16 emvPiccOpen(void)
{
	if (!emvPiccIsRuning())
	{
		LOG("AS3911 is not runing\r\n");
		return -1;
	}
	emvStopRequestReceived();
#ifdef PWR_CON
	as3911ExecuteCommand(AS3911_CMD_SET_DEFAULT);
	as3911WriteRegister(AS3911_REG_CAP_SENSOR_CONF, 0x06);
	as3911ExecuteCommand(AS3911_CMD_CALIBRATE_C_SENSOR);
	
	/* MCU_CLK and LF MCU_CLK off, 27MHz XTAL */
	as3911WriteRegister(AS3911_REG_IO_CONF1, 0x0F);
	/* Enable Oscillator, Transmitter and receiver. */
	as3911WriteRegister(AS3911_REG_OP_CONTROL, 0xC8);
	sleepMilliseconds(5);
	/* Enable AM/PM receiver signal output on CSI/CSO. */
	as3911WriteTestRegister(AS3911_REG_IO_CONF2, 0x03);
	
	as3911WriteRegister(AS3911_REG_WUP_TIMER_CONF, 0x01);//0x9
	as3911WriteRegister(AS3911_REG_CAPACITANCE_MEASURE_CONF, 0x30);//0x0B
	as3911WriteRegister(AS3911_REG_CAPACITANCE_MEASURE_REF, 0x7a);//0x0B
#else
	as3911ExecuteCommand(AS3911_CMD_SET_DEFAULT); 

	as3911WriteRegister(AS3911_REG_IO_CONF1, 0x0F);
	/* Enable Oscillator, Transmitter and receiver. */
	//as3911WriteRegister(AS3911_REG_OP_CONTROL, 0xC8);
	as3911WriteRegister(AS3911_REG_OP_CONTROL, 0xC0);  //C8 open tx
	sleepMilliseconds(5);

	/* Enable AM/PM receiver signal output on CSI/CSO. */
	// as3911WriteTestRegister(AS3911_REG_IO_CONF2, 0x03);
	
	/* Enable AS3911 IRQ handling. */
	AS3911_IRQ_ON();
	/* Voltage Regulator setup. */
	as3911ModifyRegister(AS3911_REG_IO_CONF2, 0xFF, 0x18);
	
	/* Disable the voltage regulator. */
	//as3911ModifyRegister(AS3911_REG_IO_CONF2, 0x40, 0x40);
	/* Enable the voltage regulator. */
	//as3911ModifyRegister(AS3911_REG_IO_CONF2, 0x40, 0x00);
	as3911ModifyRegister(AS3911_REG_VSS_REGULATOR_CONF, 0xF8, 0xF8);
	/* Antenna trim setup. */
	as3911ModifyRegister(AS3911_REG_ANT_CAL_CONF, 0xF8, 0x80);
	//as3911ExecuteCommand(AS3911_CMD_CALIBRATE_ANTENNA); 
	/* Receive channel setup. */
	as3911ModifyRegister(AS3911_REG_OP_CONTROL, 0x30, 0x00);
	//as3911ModifyRegister(AS3911_REG_RX_CONF1, 0x80, 0x00);
	as3911WriteRegister(AS3911_REG_RX_CONF1, 0x40);  //AM channel
	/* First stage gain reduction. */
	as3911ModifyRegister(AS3911_REG_RX_CONF3, 0xFC, 0x00);//d8 //d0 //00
	/* Second/Third stage gain reduction. */
	as3911WriteRegister(AS3911_REG_RX_CONF4, 0x00); //0x22 //00
	as3911ExecuteCommand(AS3911_CMD_CLEAR_SQUELCH);

	/* Automatic gain control and squelch. */
	as3911ModifyRegister(AS3911_REG_RX_CONF2, 0x1F, 0x12);
			
	/* Gain adjustment based on lookup table. */
	/* Readout gain lookup table. */
	as3911SetGainMode(AS3911_GAIN_FIXED, NULL);
   /* Read ISO14443B modulation depth mode byte. */	
        //13/10/15 sxl 
	as3911WriteRegister(AS3911_REG_AM_MOD_DEPTH_CONF, 0x80);
//sxl
#ifdef V10X
    as3911WriteRegister(AS3911_REG_RFO_AM_ON_LEVEL, 0xDA);//db  
#else
    as3911WriteRegister(AS3911_REG_RFO_AM_ON_LEVEL, 0xDF);//调制度//e2 
#endif
	emvHalSetAs3911TypeBModulationMode(AS3911_MODULATION_LEVEL_FIXED, NULL);
        as3911WriteRegister(AS3911_REG_RFO_AM_OFF_LEVEL, 0xAC);	 //场强
#if 0
	mainModulationAutomaticAdjustmentData.targetValue = 0x21;
	mainModulationAutomaticAdjustmentData.delay = 2;
	emvHalSetAs3911TypeBModulationMode(AS3911_MODULATION_LEVEL_AUTOMATIC, &mainModulationAutomaticAdjustmentData);
#endif
        OPENED_FLAG = TRUE;
	LOG("EMV: settings applied\n");
	displayRegisterValue(AS3911_REG_IO_CONF2);
	displayRegisterValue(AS3911_REG_VSS_REGULATOR_CONF);
	displayRegisterValue(AS3911_REG_ANT_CAL_CONF);
	displayRegisterValue(AS3911_REG_OP_CONTROL);
	displayRegisterValue(AS3911_REG_RX_CONF1);
	displayRegisterValue(AS3911_REG_RX_CONF2);
	displayRegisterValue(AS3911_REG_RX_CONF3);
	displayRegisterValue(AS3911_REG_RX_CONF4);
	displayRegisterValue(AS3911_REG_AM_MOD_DEPTH_CONF);
	displayRegisterValue(AS3911_REG_RFO_AM_ON_LEVEL);
	//displayTestRegisterValue(AS3911_REG_ANALOG_TEST);

#endif
    return EMV_ERR_OK;
}

s16 emvPiccClose(void)
{
#ifdef PWR_CON	

	 //emvPiccGoToSleepMode();
	 return emvHalActivateField(FALSE);
#else
    if (!OPENED_FLAG)
        return ERR_NONE;
	 OPENED_FLAG = FALSE;
	 return emvHalActivateField(FALSE);
#endif
}
s16 emvTypeBCardReCollision()
{
    u8 wupb[3];
    u8 atqb[13+2];
    s8 error = EMV_HAL_ERR_OK;
    u32 responseLength = 0;
    emvHalSetStandard(EMV_HAL_TYPE_B);
    emvHalSetErrorHandling(EMV_HAL_PREACTIVATION_ERROR_HANDLING);
    // s_UartPrint("emvHalSetErrorHandling= %d\r\n", emvHalSetErrorHandling);	//sxl
    wupb[0] = 0x05;
    wupb[1] = 0x00;
    wupb[2] = 0x09;
    error = emvHalTransceive(wupb, sizeof(wupb), atqb, sizeof(atqb), &responseLength, EMV_FWT_ATQB_PCD, EMV_HAL_TRANSCEIVE_WITH_CRC);
   // s_UartPrint("emvHalTransceive= %d\r\n", error);	//sxl
    if (error)
        return 0;
    else
        return 1;
}


s16 emvPiccCheck(uchar mode,uchar *CardType,uchar *SerialNo)
{
	int error;
	int recollision_cnt =10;
	
	//u32 irqs = 0;
	EmvPicc_t picc;
	if (!emvPiccIsOpening())
	{
		LOG("AS3911 is not open\r\n");
		return EMV_ERR_RUNNING;
	}
	if ((CardType == NULL) || (SerialNo == NULL))
	{
		LOG("CardType or SerialNo is NULL \r\n");
		return EMV_ERR_INTERNAL;
	}
	/* Polling. */
	emvDisplayMessage(EMV_M_POLLING);
	error = emvPollSingleIteration();
        //s_UartPrint("emvPollSingleIteration = %d\r\n", error);	//sxl
	if(EMV_ERR_OK != error){
			LOG("emvPollSingleIteration error");
		goto error;
	}
	//LOG("emvPoll end");
	/* Anticollision. */

	sleepMilliseconds(EMV_T_P);
	error = emvCollisionDetection(&picc);
        //s_UartPrint("emvCollisionDetection = %d\r\n", error);	//sxl
	if (EMV_ERR_OK != error){
		LOG("emvCollisionDetection error");
		goto error;
	}

        
           /* CardType */
        if ( emvTypeA == 1 )
        {
        CardType[0] = 'A';
        }
        else if (emvTypeB == 1)
        {
        CardType[0] = 'B';
        }
        else 
        {
        goto error;
        }
        /* SerialNo */
        int i;
        SerialNo[0] = picc.uidLength;
        for (i = 0;i < picc.uidLength; i++)
        {
        SerialNo[i+1] = picc.uid[i];
        }
	/* Activation. */
	error = emvActivate(&picc);
        //s_UartPrint("emvActivate = %d\r\n", error);//sxl
	if (EMV_ERR_OK != error){
		LOG("emvActivate error");
		goto error;
	}
        
#if 1    //13/11/04
        if(picc.sfgi > 0)
	{
		LOG("picc.sfgi = %d", picc.sfgi);
		u32 sfgtCycles = (4096UL + 384) << picc.sfgi;
		u16 sfgtMilliseconds = emvConvertCarrierCyclesToMilliseconds(sfgtCycles);
		sleepMilliseconds(sfgtMilliseconds);
	}
	while(recollision_cnt--)
       {
		if(CardType[0] == 'A' && emvTypeACardPresent())
		{
			//s_UartPrint("card a collision\r\n");
			error = EMV_ERR_COLLISION;
			goto error;
		}

		if(CardType[0] == 'B' && emvTypeBCardReCollision())
		{			
			s_UartPrint("card b collision\r\n");
			error = EMV_ERR_COLLISION;
			goto error;
		}
	}
	//LOG("emvPiccActivate2 end");
	/* Wait for SFGT. */
#endif
#if 1   //13/11/04 sxl
        emvPiccReset ();
        error = emvPollSingleIteration();
        if(EMV_ERR_OK != error){
        s_UartPrint("emvPollSingleIteration error");
        goto error;
        }
        //LOG("emvPoll end");
        /* Anticollision. */
         
        sleepMilliseconds(EMV_T_P);
        error = emvCollisionDetection(&picc);
        if (EMV_ERR_OK != error){
        s_UartPrint("emvCollisionDetection error");
        goto error;
        }
         
        //LOG("emvCollisionDetection end");
        /* Activation. */
        error = emvActivate(&picc);
        if (EMV_ERR_OK != error){
        s_UartPrint("emvActivate error");
        goto error;
        }
#endif        

 	/* Initialize layer 4. */
	error = emvInitLayer4(&picc);
        //s_UartPrint("emvInitLayer4 = %d\r\n", error);//sxl
	if (EMV_ERR_OK != error)
		goto error;
	
	return EMV_ERR_OK;
        error:
        emvHalActivateField(FALSE); //sxl 13/10/08 失败会自动关闭场强
        
	return error;	
}

static u8 emvSendBuffer[EMV_RESPONSE_BUFFER_SIZE];
//static const u8 emvSelectppseApdu[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E
//    , '2', 'P', 'A', 'Y', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1', 0x00 };


s16 emvPiccCommand (APDU_SEND *ApduSend, APDU_RESP *ApduResp)
{
	u32 responselen = 0;	
	u32 datalen = 0;
	s16 error = EMV_ERR_OK;
        s16 i,j;
	if (!emvPiccIsOpening())
	{
		LOG("AS3911 is not open\r\n");
		return EMV_ERR_RUNNING;
	}
	
	#if 1
	if( (ApduSend == NULL) || (ApduResp == NULL))
		return EMV_ERR_INTERNAL;
	memset(emvSendBuffer, 0, EMV_RESPONSE_BUFFER_SIZE);
	memcpy(emvSendBuffer, ApduSend->Command, 4);
	datalen += 4;
        //13/10/22
        /* lc=le=0的情况 */
        if ((ApduSend->Lc == 0)&&(ApduSend->Le == 0))
        {
            emvSendBuffer[datalen++] = 0x00;
        }
        /* lc不为0的情况 */
        if (ApduSend->Lc)
        {
        if(ApduSend->Lc>250)    
        return EMV_ERR_INTERNAL;
        else {
        emvSendBuffer[datalen++] = (u8)ApduSend->Lc;
        memcpy(&emvSendBuffer[datalen], ApduSend->DataIn, ApduSend->Lc);
        datalen = datalen + ApduSend->Lc;
        }
        }
        /* le不为0的情况 */
        if(ApduSend->Le)
        {
        //emvSendBuffer[datalen++] = (u8)ApduSend->Le;
        //le>=256的情况
        if(ApduSend->Le>=256)    
                emvSendBuffer[datalen++]=0x00;
            else                     
        emvSendBuffer[datalen++]=(u8)(ApduSend->Le);
        }

        /*
	emvSendBuffer[datalen++] = (u8)ApduSend->Lc;
	memcpy(&emvSendBuffer[datalen], ApduSend->DataIn, ApduSend->Lc);
	datalen = datalen + ApduSend->Lc;
        */
	//emvSendBuffer[datalen++] = (u8)ApduSend->Le;
	#endif
	
	#if 1
	//LOG("******************************************sizeof(emvSelectppseApdu) = %d", sizeof(emvSelectppseApdu));
	//emvDisplayCAPDU(emvSelectppseApdu, sizeof(emvSelectppseApdu));
	
	emvDisplayCAPDU(emvSendBuffer, datalen);
	#endif
	//error = emvTransceiveApdu((const u8 *)emvSelectppseApdu, sizeof(emvSelectppseApdu), emvResponseBuffer, EMV_RESPONSE_BUFFER_SIZE, &responselen);
	error = emvTransceiveApdu((const u8 *)emvSendBuffer, datalen, emvResponseBuffer, EMV_RESPONSE_BUFFER_SIZE, &responselen);
        //14/2/11
        
#if 0
        
	Lib_LcdSetFont(8, 16, 0);
        Lib_Lcdprintf("emvSendBuffer:");
	for(i=0;i<datalen;i++)
        Lib_Lcdprintf("%02x ",emvSendBuffer[i]);
        Lib_KbGetCh();
        Lib_LcdCls();
        //s_UartPrint("\n");
        Lib_Lcdprintf("emvResponseBuffer:");
	for(j=0;j<responselen;j++)
        Lib_Lcdprintf("%02x ",emvResponseBuffer[j]);
        Lib_KbGetCh();
#endif       
        //s_UartPrint("\n");
        //s_UartPrint("emvTransceiveApdu=%d\r\n",error);//sxl
	if(error)
	{
		LOG("command exchange error");
		return error;
	}
	ApduResp->LenOut = responselen-2;
	memcpy (ApduResp->DataOut, &emvResponseBuffer[0], ApduResp->LenOut);
	ApduResp->SWA = emvResponseBuffer[responselen-2];
	ApduResp->SWB = emvResponseBuffer[responselen-1];
	
#if 1
	LOG("ApduResp->LenOut = %d", ApduResp->LenOut);
	LOG("AApduResp->DataOut:");
	emvDisplayRAPDU(emvResponseBuffer, responselen-2);
	LOG("ApduResp->SWA = 0x%x", ApduResp->SWA);
	LOG("ApduResp->SWB = 0x%x", ApduResp->SWB);
#endif
	return EMV_ERR_OK;

	
}

s16 emvPiccReset ()
{
	 return emvHalResetField();
}

s16 emvPiccActivate()
{
	return emvPicc->activate(emvPicc);
}


s16 emvPiccRemove()
{
	if (!emvPiccIsOpening())
	{
		LOG("AS3911 is not open\r\n");
		return EMV_ERR_RUNNING;
	}

	if((emvPicc == NULL)||(emvPicc->remove == NULL))
		return EMV_ERR_PROTOCOL;

    return emvPicc->remove(emvPicc);
}

