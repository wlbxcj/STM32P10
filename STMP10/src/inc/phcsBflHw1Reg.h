/* /////////////////////////////////////////////////////////////////////////////////////////////////
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
 * \file phcsBflHw1Reg.h
 *
 * Project: Object Oriented Library Framework Joiner register definitions.
 *
 *  Source: phcsBflHw1Reg.h
 * $Author: frq09147 $
 * $Revision: 1.1 $
 * $Date: Thu Sep 27 09:38:45 2007 $
 *
 * Comment:
 *  Joiner register and bit definitions.
 *
 * History:
 *  MHa: Generated 07. May 2003
 *  MHa: Migrated to MoReUse September 2005
 * \endif
 *
 */

#ifndef PHCSBFLHW1REG_H
#define PHCSBFLHW1REG_H


/*! \name Register definitions of Page 0
 *  Following all register defintions of the Joiner Page 0.
 */
/*@{*/
/*! \ingroup reg */
#define     PHCS_BFL_JREG_PAGE0           0x00      /*!< \brief Page register in page 0                                 */
#define     PHCS_BFL_JREG_COMMAND         0x01      /*!< \brief Contains Command bits, PowerDown bit and bit to 
                                                switch receiver off.                                    */
#define     PHCS_BFL_JREG_COMMIEN         0x02      /*!< \brief Contains Communication interrupt enable bits and
                                                bit for Interrupt inversion.                            */
#define     PHCS_BFL_JREG_DIVIEN          0x03      /*!< \brief Contains RfOn, RfOff, CRC and Mode Interrupt enable 
                                                and bit to switch Interrupt pin to PushPull mode.       */
#define     PHCS_BFL_JREG_COMMIRQ         0x04      /*!< \brief Contains Communication interrupt request bits.          */
#define     PHCS_BFL_JREG_DIVIRQ          0x05      /*!< \brief Contains RfOn, RfOff, CRC and Mode Interrupt request.  */
#define     PHCS_BFL_JREG_ERROR           0x06      /*!< \brief Contains Protocol, Parity, CRC, Collision, Buffer 
                                                overflow, Temperature and RF error flags.               */
#define     PHCS_BFL_JREG_STATUS1         0x07      /*!< \brief Contains status information about Lo- and HiAlert, 
                                                RF-field on, Timer, Interrupt request and CRC status.   */
#define     PHCS_BFL_JREG_STATUS2         0x08      /*!< \brief Contains information about internal states (Modemstate),
                                                Mifare states and possibility to switch Temperature 
                                                sensor off.                                             */
#define     PHCS_BFL_JREG_FIFODATA        0x09      /*!< \brief Gives access to FIFO. Writing to register increments the
                                                FIFO level (register 0x0A), reading decrements it.      */
#define     PHCS_BFL_JREG_FIFOLEVEL       0x0A      /*!< \brief Contains the actual level of the FIFO.                  */
#define     PHCS_BFL_JREG_WATERLEVEL      0x0B      /*!< \brief Contains the Waterlevel value for the FIFO              */
#define     PHCS_BFL_JREG_CONTROL         0x0C      /*!< \brief Contains information about last received bits, Initiator
                                                mode bit, bit to copy NFCID to FIFO and to Start and stop
                                                the Timer unit.                                         */
#define     PHCS_BFL_JREG_BITFRAMING      0x0D      /*!< \brief Contains information of last bits to send, to align 
                                                received bits in FIFO and activate sending in Transceive*/
#define     PHCS_BFL_JREG_COLL            0x0E      /*!< \brief Contains all necessary bits for Collission handling     */
#define     PHCS_BFL_JREG_RFU0F           0x0F      /*!< \brief Currently not used.                                     */
/*@}*/

/*! \name Register definitions of Page 1
 *  Following all register defintions of the Joiner Page 1.
 */
/*@{*/
/*! \ingroup reg */
#define     PHCS_BFL_JREG_PAGE1           0x10      /*!< \brief Page register in page 1                                 */
#define     PHCS_BFL_JREG_MODE            0x11      /*!< \brief Contains bits for auto wait on Rf, to detect SYNC byte in
                                                NFC mode and MSB first for CRC calculation              */ 
#define     PHCS_BFL_JREG_TXMODE          0x12      /*!< \brief Contains Transmit Framing, Speed, CRC enable, bit for 
                                                inverse mode and TXMix bit.                             */ 
#define     PHCS_BFL_JREG_RXMODE          0x13      /*!< \brief Contains Transmit Framing, Speed, CRC enable, bit for 
                                                multiple receive and to filter errors.                  */ 
#define     PHCS_BFL_JREG_TXCONTROL       0x14      /*!< \brief Contains bits to activate and configure Tx1 and Tx2 and
                                                bit to activate 100% modulation.                        */ 
#define     PHCS_BFL_JREG_TXAUTO          0x15      /*!< \brief Contains bits to automatically switch on/off the Rf and 
                                                to do the collission avoidance and the initial rf-on.   */
#define     PHCS_BFL_JREG_TXSEL           0x16      /*!< \brief Contains SigoutSel, DriverSel and LoadModSel bits.      */
#define     PHCS_BFL_JREG_RXSEL           0x17      /*!< \brief Contains UartSel and RxWait bits.                       */
#define     PHCS_BFL_JREG_RXTRESHOLD      0x18      /*!< \brief Contains MinLevel and CollLevel for detection.          */
#define     PHCS_BFL_JREG_DEMOD           0x19      /*!< \brief Contains bits for time constants, hysteresis and 
                                                IQ demodulator settings.                                */
#define     PHCS_BFL_JREG_FELICANFC       0x1A      /*!< \brief Contains bits for minimum FeliCa length received and for
                                                FeliCa syncronisation length.                           */
#define     PHCS_BFL_JREG_FELICANFC2      0x1B      /*!< \brief Contains bits for maximum FeliCa length received.       */
#define     PHCS_BFL_JREG_MIFARE          0x1C      /*!< \brief Contains Miller settings, TxWait settings and MIFARE 
                                                halted mode bit.                                        */
#define     PHCS_BFL_JREG_MANUALRCV       0x1D      /*!< \brief Currently not used.                                     */
#define     PHCS_BFL_JREG_RFU1E           0x1E      /*!< \brief Currently not used.                                     */
#define     PHCS_BFL_JREG_SERIALSPEED     0x1F      /*!< \brief Contains speed settings for serila interface.           */
/*@}*/

/*! \name Register definitions of Page 2
 *  Following all register defintions of the Joiner Page 2.
 */
/*@{*/
/*! \ingroup reg */
#define     PHCS_BFL_JREG_PAGE2           0x20      /*!< \brief Page register in page 2 */
#define     PHCS_BFL_JREG_CRCRESULT1      0x21	   /*!< \brief Contains MSByte of CRC Result.                          */
#define     PHCS_BFL_JREG_CRCRESULT2      0x22	   /*!< \brief Contains LSByte of CRC Result.                          */
#define     PHCS_BFL_JREG_GSNLOADMOD      0x23      /*!< \brief Contains the conductance and the modulation settings for 
                                                the N-MOS transistor only for load modulation (See 
                                                difference to PHCS_BFL_JREG_GSN!).                               */
#define     PHCS_BFL_JREG_MODWIDTH        0x24      /*!< \brief Contains modulation width setting.                      */
#define     PHCS_BFL_JREG_TXBITPHASE      0x25      /*!< \brief Contains TxBitphase settings and receive clock change.  */
#define     PHCS_BFL_JREG_RFCFG           0x26      /*!< \brief Contains sensitivity of Rf Level detector, the receiver
                                                gain factor and the RfLevelAmp.                         */
#define     PHCS_BFL_JREG_GSN             0x27      /*!< \brief Contains the conductance and the modulation settings for 
                                                the N-MOS transistor during active modulation (no load
                                                modulation setting!).                                   */
#define     PHCS_BFL_JREG_CWGSP           0x28      /*!< \brief Contains the conductance for the P-Mos transistor.      */
#define     PHCS_BFL_JREG_MODGSP          0x29      /*!< \brief Contains the modulation index for the PMos transistor.  */
#define     PHCS_BFL_JREG_TMODE           0x2A      /*!< \brief Contains all settings for the timer and the highest 4
                                                bits of the prescaler.                                  */
#define     PHCS_BFL_JREG_TPRESCALER      0x2B      /*!< \brief Contais the lowest byte of the prescaler.               */
#define     PHCS_BFL_JREG_TRELOADHI       0x2C      /*!< \brief Contains the high byte of the reload value.             */
#define     PHCS_BFL_JREG_TRELOADLO       0x2D      /*!< \brief Contains the low byte of the reload value.              */
#define     PHCS_BFL_JREG_TCOUNTERVALHI   0x2E      /*!< \brief Contains the high byte of the counter value.            */
#define     PHCS_BFL_JREG_TCOUNTERVALLO   0x2F      /*!< \brief Contains the low byte of the counter value.             */
/*@}*/

/*! \name Register definitions of Page 3
 *  Following all register defintions of the Joiner Page 3.
 */
/*@{*/
/*! \ingroup reg */
#define     PHCS_BFL_JREG_PAGE3           0x30      /*!< \brief Page register in page 3 */
#define     PHCS_BFL_JREG_TESTSEL1        0x31      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTSEL2        0x32      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTPINEN       0x33      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTPINVALUE    0x34      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTBUS         0x35      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_AUTOTEST        0x36      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_VERSION         0x37      /*!< \brief Contains the product number and the version.            */
#define     PHCS_BFL_JREG_ANALOGTEST      0x38      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTDAC1        0x39      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTDAC2        0x3A      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_TESTADC         0x3B      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_ANALOGUETEST1   0x3C      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_ANALOGUETEST0   0x3D      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_ANALOGUETPD_A   0x3E      /*!< \brief Test register                                           */
#define     PHCS_BFL_JREG_ANALOGUETPD_B   0x3F      /*!< \brief Test register                                           */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Possible commands
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name Joiner Command definitions
 *  Following all commands of the Joiner.
 */
/*@{*/
/*! \ingroup reg */
#define     PHCS_BFL_JCMD_IDLE          0x00 /*!< \brief No action: cancel current command 
                                     or home state. \n */
#define     PHCS_BFL_JCMD_CONFIG        0x01 /*!< \brief Copy configuration data from FIFO to 
                                     internal Buffer or if FIFO empty vice versa */
#define     PHCS_BFL_JCMD_RANDOMIDS     0x02 /*!< \brief replaces stored IDs with random ones */
#define     PHCS_BFL_JCMD_CALCCRC       0x03 /*!< \brief Activate the CRC-Coprocessor \n<em><strong>
                                     Remark: </strong>The result of the CRC calculation can
                                     be read from the register CRCResultXXX </em>*/
#define     PHCS_BFL_JCMD_TRANSMIT      0x04 /*!< \brief Transmit data from FIFO to the card \n<em>
                                     <strong>Remark: </strong>If data is already in 
                                     the FIFO when the command is activated, this data is 
                                     transmitted immediately. It is possible to 
                                     write data to the FIFO while the Transmit 
                                     command is active. Thus it is possible to 
                                     transmit an unlimited number of bytes in one
                                     stream by writting them to the FIFO in time.</em>*/
#define     PHCS_BFL_JCMD_NOCMDCHANGE   0x07 /*!< \brief This command does not change the actual commant of
                                     the PN51x and can only be written. \n<em><strong>
                                     Remark: </strong>This command is used for WakeUp procedure
                                     of PN51x to not change the current state. </em>*/
#define     PHCS_BFL_JCMD_RECEIVE       0x08 /*!< \brief Activate Receiver Circuitry. Before the 
                                     receiver actually starts, the state machine 
                                     waits until the time configured in the 
                                     register RxWait has passed. \n<em><strong>
                                     Remark: </strong>It is possible to read any received 
                                     data from the FIFO while the Receive command
                                     is active. Thus it is possible to receive an 
                                     unlimited number of bytes by reading them 
                                     from the FIFO in time. </em>*/
#define     PHCS_BFL_JCMD_TRANSCEIVE    0x0C /*!< \brief This Command has two modes:\n
                                     If Initiator is 1: Transmits data from FIFO to 
                                     the card and after that automatically activates 
                                     the receiver. Before the receiver actually 
                                     starts,the state machine waits until the 
                                     time configured in the register RxWait has 
                                     passed. \n <em><strong>
                                     Remark: </strong>This command is the combination of 
                                     Transmit and Receive.</em> \n
                                     If Initiator is 0: Wait until data received,
                                     writes them into the FIFO abd switches afterwards 
                                     to Transmit Mode. Data are only send if Bit
                                     StartSend is set. \n <em><strong>
                                     Remark: </strong>This command is the combination of 
                                     Transmit and Receive. </em>*/
#define     PHCS_BFL_JCMD_AUTOCOLL      0x0D /*!< \brief Activates automatic anticollision in Target 
                                     mode. Data from Config command is used.
                                     \n <em><strong>Remark: </strong>Activate CRC before, 
                                     (Mifare does it's own settings for CRC) </em>*/
#define     PHCS_BFL_JCMD_AUTHENT       0x0E /*!< \brief Perform the card authentication using the 
                                     Crypto1 algorithm.
                                     \n <em><strong>Remark: </strong></em>*/
#define     PHCS_BFL_JCMD_SOFTRESET     0x0F /*!< \brief Runs the Reset- and Initialisation Phase
                                     \n <em><strong>Remark:</strong> This command can 
                                     be activated by software, but only by a Power-On 
                                     or Hard Reset </em>*/
/*! 
 *  \brief 
 *  Defintion for special transceive command, which uses only timeout to terminate!
 *  This is especially used for the FeliCa Polling command because there only a 
 *  timeout is valid to terminate for a slow interface!      */
#define     PHCS_BFL_CMD_TRANSCEIVE_TO       0x8C

/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bit Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name Joiner Bit definitions of Page 0
 *  Below there are useful bit definition of the Joiner register set of Page 0.
 */
/*@{*/
/*! \ingroup reg */
/* Command Register							(01) */
#define     PHCS_BFL_JBIT_RCVOFF             0x20   /*!< \brief Switches the receiver on/off. */
#define     PHCS_BFL_JBIT_POWERDOWN          0x10   /*!< \brief Switches Joiner to Power Down mode. */

/* CommIEn Register							(02) */
#define     PHCS_BFL_JBIT_IRQINV             0x80   /*!< \brief Inverts the output of IRQ Pin. */

/* DivIEn Register							(03) */
#define     PHCS_BFL_JBIT_IRQPUSHPULL        0x80   /*!< \brief Sets the IRQ pin to Push Pull mode. */

/* CommIEn and CommIrq Register         (02, 04) */
#define     PHCS_BFL_JBIT_TXI                0x40   /*!< \brief Bit position for Transmit Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_RXI                0x20   /*!< \brief Bit position for Receive Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_IDLEI              0x10   /*!< \brief Bit position for Idle Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_HIALERTI           0x08   /*!< \brief Bit position for HiAlert Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_LOALERTI           0x04   /*!< \brief Bit position for LoAlert Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_ERRI               0x02   /*!< \brief Bit position for Error Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_TIMERI             0x01   /*!< \brief Bit position for Timer Interrupt Enable/Request. */

/* DivIEn and DivIrq Register           (03, 05) */
#define     PHCS_BFL_JBIT_SIGINACT           0x10   /*!< \brief Bit position for SiginAct Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_MODEI              0x08   /*!< \brief Bit position for Mode Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_CRCI               0x04   /*!< \brief Bit position for CRC Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_RFONI              0x02   /*!< \brief Bit position for RF On Interrupt Enable/Request. */
#define     PHCS_BFL_JBIT_RFOFFI             0x01   /*!< \brief Bit position for RF Off Interrupt Enable/Request. */

/* CommIrq and DivIrq Register          (04, 05) */
#define     PHCS_BFL_JBIT_SET                0x80   /*!< \brief Bit position to set/clear dedicated IRQ bits. */

/* Error Register 							(06) */
#define     PHCS_BFL_JBIT_WRERR              0x40   /*!< \brief Bit position for Write Access Error. */
#define     PHCS_BFL_JBIT_TEMPERR            0x40   /*!< \brief Bit position for Temerature Error. */
#define     PHCS_BFL_JBIT_RFERR              0x20   /*!< \brief Bit position for RF Error. */
#define     PHCS_BFL_JBIT_BUFFEROVFL         0x10   /*!< \brief Bit position for Buffer Overflow Error. */
#define     PHCS_BFL_JBIT_COLLERR            0x08   /*!< \brief Bit position for Collision Error. */
#define     PHCS_BFL_JBIT_CRCERR             0x04   /*!< \brief Bit position for CRC Error. */
#define     PHCS_BFL_JBIT_PARITYERR          0x02   /*!< \brief Bit position for Parity Error. */
#define     PHCS_BFL_JBIT_PROTERR            0x01   /*!< \brief Bit position for Protocol Error. */

/* Status 1 Register 						(07) */
#define     PHCS_BFL_JBIT_CRCOK              0x40   /*!< \brief Bit position for status CRC OK. */
#define     PHCS_BFL_JBIT_CRCREADY           0x20   /*!< \brief Bit position for status CRC Ready. */
#define     PHCS_BFL_JBIT_IRQ                0x10   /*!< \brief Bit position for status IRQ is active. */
#define     PHCS_BFL_JBIT_TRUNNUNG           0x08   /*!< \brief Bit position for status Timer is running. */
#define     PHCS_BFL_JBIT_RFON               0x04   /*!< \brief Bit position for status RF is on/off. */
#define     PHCS_BFL_JBIT_HIALERT            0x02   /*!< \brief Bit position for status HiAlert. */
#define     PHCS_BFL_JBIT_LOALERT            0x01   /*!< \brief Bit position for status LoAlert. */

/* Status 2 Register				    		(08) */
#define     PHCS_BFL_JBIT_TEMPSENSOFF        0x80   /*!< \brief Bit position to switch Temperture sensors on/off. */
#define     PHCS_BFL_JBIT_I2CFORCEHS         0x40   /*!< \brief Bit position to forece High speed mode for I2C Interface. */
#define     PHCS_BFL_JBIT_MFSELECTED         0x10   /*!< \brief Bit position for card status Mifare selected. */
#define     PHCS_BFL_JBIT_CRYPTO1ON          0x08   /*!< \brief Bit position for reader status Crypto is on. */

/* FIFOLevel Register				    		(0A) */
#define     PHCS_BFL_JBIT_FLUSHBUFFER        0x80   /*!< \brief Clears FIFO buffer if set to 1 */

/* Control Register					    		(0C) */
#define     PHCS_BFL_JBIT_TSTOPNOW           0x80   /*!< \brief Stops timer if set to 1. */
#define     PHCS_BFL_JBIT_TSTARTNOW          0x40   /*!< \brief Starts timer if set to 1. */
#define     PHCS_BFL_JBIT_WRNFCIDTOFIFO      0x20   /*!< \brief Copies internal stored NFCID3 to actual position of FIFO. */
#define     PHCS_BFL_JBIT_INITIATOR          0x10   /*!< \brief Sets Initiator mode. */

/* BitFraming Register					    (0D) */
#define     PHCS_BFL_JBIT_STARTSEND          0x80   /*!< \brief Starts transmission in transceive command if set to 1. */

/* BitFraming Register					    (0E) */
#define     PHCS_BFL_JBIT_VALUESAFTERCOLL    0x80   /*!< \brief Activates mode to keep data after collision. */
/*@}*/

/*! \name Joiner Bit definitions of Page 1
 *  Below there are useful bit definition of the Joiner register set of Page 1.
 */
/*@{*/
/*! \ingroup reg */
/* Mode Register							(11) */
#define     PHCS_BFL_JBIT_MSBFIRST           0x80   /*!< \brief Sets CRC coprocessor with MSB first. */
#define     PHCS_BFL_JBIT_DETECTSYNC         0x40   /*!< \brief Activate automatic sync detection for NFC 106kbps. */
#define     PHCS_BFL_JBIT_TXWAITRF           0x20   /*!< \brief Tx waits until Rf is enabled until transmit is startet, else 
                                                transmit is started immideately. */
#define     PHCS_BFL_JBIT_RXWAITRF           0x10   /*!< \brief Rx waits until Rf is enabled until receive is startet, else 
                                                receive is started immideately. */
#define     PHCS_BFL_JBIT_POLSIGIN           0x08   /*!< \brief Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures
                                                when Sigin line is 0. */
#define     PHCS_BFL_JBIT_MODEDETOFF         0x04   /*!< \brief Deactivates the ModeDetector during AutoAnticoll command. The settings
                                                of the register are valid only. */

/* TxMode Register							(12) */
#define     PHCS_BFL_JBIT_INVMOD             0x08   /*!< \brief Activates inverted transmission mode. */
#define     PHCS_BFL_JBIT_TXMIX              0x04   /*!< \brief Activates TXMix functionality. */

/* RxMode Register							(13) */
#define     PHCS_BFL_JBIT_RXNOERR            0x08   /*!< \brief If 1, receiver does not receive less than 4 bits. */
#define     PHCS_BFL_JBIT_RXMULTIPLE         0x04   /*!< \brief Activates reception mode for multiple responses. */

/* Definitions for Tx and Rx		    (12, 13) */
#define     PHCS_BFL_JBIT_106KBPS            0x00   /*!< \brief Activates speed of 106kbps. */
#define     PHCS_BFL_JBIT_212KBPS            0x10   /*!< \brief Activates speed of 212kbps. */
#define     PHCS_BFL_JBIT_424KBPS            0x20   /*!< \brief Activates speed of 424kbps. */
#define     PHCS_BFL_JBIT_848KBPS            0x30   /*!< \brief Activates speed of 848kbps. */
#define     PHCS_BFL_JBIT_1_6MBPS            0x40   /*!< \brief Activates speed of 1.6Mbps. */
#define     PHCS_BFL_JBIT_3_2MBPS            0x50   /*!< \brief Activates speed of 3_3Mbps. */

#define     PHCS_BFL_JBIT_MIFARE             0x00   /*!< \brief Activates Mifare communication mode. */
#define     PHCS_BFL_JBIT_NFC                0x01   /*!< \brief Activates NFC/Active communication mode. */
#define     PHCS_BFL_JBIT_FELICA             0x02   /*!< \brief Activates FeliCa communication mode. */

#define     PHCS_BFL_JBIT_CRCEN              0x80   /*!< \brief Activates transmit or receive CRC. */

/* TxControl Register						(14) */
#define     PHCS_BFL_JBIT_INVTX2ON           0x80   /*!< \brief Inverts the Tx2 output if drivers are switched on. */
#define     PHCS_BFL_JBIT_INVTX1ON           0x40   /*!< \brief Inverts the Tx1 output if drivers are switched on. */
#define     PHCS_BFL_JBIT_INVTX2OFF          0x20   /*!< \brief Inverts the Tx2 output if drivers are switched off. */
#define     PHCS_BFL_JBIT_INVTX1OFF          0x10   /*!< \brief Inverts the Tx1 output if drivers are switched off. */
#define     PHCS_BFL_JBIT_TX2CW              0x08   /*!< \brief Does not modulate the Tx2 output, only constant wave. */
#define     PHCS_BFL_JBIT_CHECKRF            0x04   /*!< \brief Does not activate the driver if an external RF is detected.
                                                Only valid in combination with PHCS_BFL_JBIT_TX2RFEN and PHCS_BFL_JBIT_TX1RFEN. */
#define     PHCS_BFL_JBIT_TX2RFEN            0x02   /*!< \brief Switches the driver for Tx2 pin on. */
#define     PHCS_BFL_JBIT_TX1RFEN            0x01   /*!< \brief Switches the driver for Tx1 pin on. */

/* TxAuto Register							(15) */
#define     PHCS_BFL_JBIT_AUTORFOFF          0x80   /*!< \brief Switches the RF automatically off after transmission is finished. */
#define     PHCS_BFL_JBIT_FORCE100ASK        0x40   /*!< \brief Activates 100%ASK mode independent of driver settings. */
#define     PHCS_BFL_JBIT_AUTOWAKEUP         0x20   /*!< \brief Activates automatic wakeup of the PN51x if set to 1. */
#define     PHCS_BFL_JBIT_CAON               0x08   /*!< \brief Activates the automatic time jitter generation by switching 
                                                on the Rf field as defined in ECMA-340. */
#define     PHCS_BFL_JBIT_INITIALRFON        0x04   /*!< \brief Activate the initial RF on procedure as defined iun ECMA-340. */
#define     PHCS_BFL_JBIT_TX2RFAUTOEN        0x02   /*!< \brief Switches on the driver two automatically according to the 
                                                other settings. */
#define     PHCS_BFL_JBIT_TX1RFAUTOEN        0x01   /*!< \brief Switches on the driver one automatically according to the 
                                                other settings. */

/* Demod Register 							(19) */
#define     PHCS_BFL_JBIT_FIXIQ              0x20   /*!< \brief If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel.
                                                If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. */

/* Felica/NFC 2 Register 				    (1B) */
#define     PHCS_BFL_JBIT_WAITFORSELECTED    0x80   /*!< \brief If this bit is set to one, only passive communication modes are possible.
                                                In any other case the AutoColl Statemachine does not exit. */
#define     PHCS_BFL_JBIT_FASTTIMESLOT       0x40   /*!< \brief If this bit is set to one, the response time to the polling command is half as normal. */

/* Mifare Register 							(1C) */
#define     PHCS_BFL_JBIT_MFHALTED           0x04   /*!< \brief Configures the internal state machine only to answer to
                                                Wakeup commands according to ISO 14443-3. */

/* RFU 0x1D Register 					    (1D) */
#define     PHCS_BFL_JBIT_PARITYDISABLE      0x10   /*!< \brief Disables the parity generation and sending independent from the mode. */
#define     PHCS_BFL_JBIT_LARGEBWPLL         0x08   /* */
#define     PHCS_BFL_JBIT_MANUALHPCF         0x04   /* */
/*@}*/

/*! \name Joiner Bit definitions of Page 2
 *  Below there are useful bit definition of the Joiner register set.
 */
/*@{*/
/*! \ingroup reg */
/* TxBitPhase Register 					    (25) */
#define     PHCS_BFL_JBIT_RCVCLKCHANGE       0x80   /*!< \brief If 1 the receive clock may change between Rf and oscilator. */

/* RfCFG Register 							(26) */
#define     PHCS_BFL_JBIT_RFLEVELAMP         0x80   /*!< \brief Activates the RF Level detector amplifier. */

/* TMode Register 							(2A) */
#define     PHCS_BFL_JBIT_TAUTO              0x80   /*!< \brief Sets the Timer start/stop conditions to Auto mode. */
#define     PHCS_BFL_JBIT_TAUTORESTART       0x10   /*!< \brief Restarts the timer automatically after finished
                                                counting down to 0. */
/*@}*/

/*! \name Joiner Bit definitions of Page 3
 *  Below there are useful bit definition of the Joiner register set.
 */
/*@{*/
/*! \ingroup reg */
/* AutoTest Register 					    (36) */
#define     PHCS_BFL_JBIT_AMPRCV             0x40   /* */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bitmask Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name Joiner Bitmask definitions
 *  Below there are some useful mask defintions for the Joiner. All specified 
 *  bits are set to 1.
 */
/*@{*/
/*! \ingroup reg */

/* Command register                 (0x01)*/
#define     PHCS_BFL_JMASK_COMMAND           0x0F   /*!< \brief Bitmask for Command bits in Register PHCS_BFL_JREG_COMMAND. */

/* Waterlevel register              (0x0B)*/
#define     PHCS_BFL_JMASK_WATERLEVEL        0x3F   /*!< \brief Bitmask for Waterlevel bits in register PHCS_BFL_JREG_WATERLEVEL. */

/* Control register                 (0x0C)*/
#define     PHCS_BFL_JMASK_RXBITS            0x07   /*!< \brief Bitmask for RxLast bits in register PHCS_BFL_JREG_CONTROL. */

/* Mode register                    (0x11)*/
#define     PHCS_BFL_JMASK_CRCPRESET         0x03   /*!< \brief Bitmask for CRCPreset bits in register PHCS_BFL_JREG_MODE. */

/* TxMode register                  (0x12, 0x13)*/
#define     PHCS_BFL_JMASK_SPEED             0x70   /*!< \brief Bitmask for Tx/RxSpeed bits in register PHCS_BFL_JREG_TXMODE and PHCS_BFL_JREG_RXMODE. */
#define     PHCS_BFL_JMASK_FRAMING           0x03   /*!< \brief Bitmask for Tx/RxFraming bits in register PHCS_BFL_JREG_TXMODE and PHCS_BFL_JREG_RXMODE. */

/* TxSel register                   (0x16)*/
#define     PHCS_BFL_JMASK_LOADMODSEL        0xC0   /*!< \brief Bitmask for LoadModSel bits in register PHCS_BFL_JREG_TXSEL. */
#define     PHCS_BFL_JMASK_DRIVERSEL         0x30   /*!< \brief Bitmask for DriverSel bits in register PHCS_BFL_JREG_TXSEL. */
#define     PHCS_BFL_JMASK_SIGOUTSEL         0x0F   /*!< \brief Bitmask for SigoutSel bits in register PHCS_BFL_JREG_TXSEL. */

/* RxSel register                   (0x17)*/
#define     PHCS_BFL_JMASK_UARTSEL           0xC0   /*!< \brief Bitmask for UartSel bits in register PHCS_BFL_JREG_RXSEL. */
#define     PHCS_BFL_JMASK_RXWAIT            0x3F   /*!< \brief Bitmask for RxWait bits in register PHCS_BFL_JREG_RXSEL. */

/* RxThreshold register             (0x18)*/
#define     PHCS_BFL_JMASK_MINLEVEL          0xF0   /*!< \brief Bitmask for MinLevel bits in register PHCS_BFL_JREG_RXTHRESHOLD. */
#define     PHCS_BFL_JMASK_COLLEVEL          0x07   /*!< \brief Bitmask for CollLevel bits in register PHCS_BFL_JREG_RXTHRESHOLD. */

/* Demod register                   (0x19)*/
#define     PHCS_BFL_JMASK_ADDIQ             0xC0   /*!< \brief Bitmask for ADDIQ bits in register PHCS_BFL_JREG_DEMOD. */
#define     PHCS_BFL_JMASK_TAURCV            0x0C   /*!< \brief Bitmask for TauRcv bits in register PHCS_BFL_JREG_DEMOD. */
#define     PHCS_BFL_JMASK_TAUSYNC           0x03   /*!< \brief Bitmask for TauSync bits in register PHCS_BFL_JREG_DEMOD. */

/* FeliCa / FeliCa2 register        (0x1A, 0x1B)*/
#define     PHCS_BFL_JMASK_FELICASYNCLEN     0xC0   /*!< \brief Bitmask for FeliCaSyncLen bits in registers PHCS_BFL_JREG_FELICANFC. */
#define     PHCS_BFL_JMASK_FELICALEN         0x3F   /*!< \brief Bitmask for FeliCaLenMin and FeliCaLenMax in 
                                                registers PHCS_BFL_JREG_FELICANFC and PHCS_BFL_JREG_FELICANFC2. */
/* Mifare register                  (0x1C)*/
#define     PHCS_BFL_JMASK_SENSMILLER        0xE0   /*!< \brief Bitmask for SensMiller bits in register PHCS_BFL_JREG_MIFARE. */
#define     PHCS_BFL_JMASK_TAUMILLER         0x18   /*!< \brief Bitmask for TauMiller bits in register PHCS_BFL_JREG_MIFARE. */
#define     PHCS_BFL_JMASK_TXWAIT            0x03   /*!< \brief Bitmask for TxWait bits in register PHCS_BFL_JREG_MIFARE. */

/* Manual Rcv register				(0x1D)*/
#define     PHCS_BFL_JMASK_HPCF				0x03   /*!< \brief Bitmask for HPCF filter adjustments. */

/* TxBitPhase register              (0x25)*/
#define     PHCS_BFL_JMASK_TXBITPHASE        0x7F   /*!< \brief Bitmask for TxBitPhase bits in register PHCS_BFL_JREG_TXBITPHASE. */

/* RFCfg register                   (0x26)*/
#define     PHCS_BFL_JMASK_RXGAIN            0x70   /*!< \brief Bitmask for RxGain bits in register PHCS_BFL_JREG_RFCFG. */
#define     PHCS_BFL_JMASK_RFLEVEL           0x0F   /*!< \brief Bitmask for RfLevel bits in register PHCS_BFL_JREG_RFCFG. */

/* GsN register                     (0x27)*/
#define     PHCS_BFL_JMASK_CWGSN             0xF0   /*!< \brief Bitmask for CWGsN bits in register PHCS_BFL_JREG_GSN. */
#define     PHCS_BFL_JMASK_MODGSN            0x0F   /*!< \brief Bitmask for ModGsN bits in register PHCS_BFL_JREG_GSN. */

/* CWGsP register                   (0x28)*/
#define     PHCS_BFL_JMASK_CWGSP             0x3F   /*!< \brief Bitmask for CWGsP bits in register PHCS_BFL_JREG_CWGSP. */

/* ModGsP register                  (0x29)*/
#define     PHCS_BFL_JMASK_MODGSP            0x3F   /*!< \brief Bitmask for ModGsP bits in register PHCS_BFL_JREG_MODGSP. */

/* TMode register                   (0x2A)*/
#define     PHCS_BFL_JMASK_TGATED            0x60   /*!< \brief Bitmask for TGated bits in register PHCS_BFL_JREG_TMODE. */
#define     PHCS_BFL_JMASK_TPRESCALER_HI     0x0F   /*!< \brief Bitmask for TPrescalerHi bits in register PHCS_BFL_JREG_TMODE. */

/*@}*/
/*! \phJalJoinerAux.h */

#define PHJAL_NORFON                        0x00
#define PHJAL_EXTRFON                       0x01
#define PHJAL_INTRFON                       0x02

/*@}*/

#endif /* PHCSBFLHW1REG_H */

/* E.O.F. */
