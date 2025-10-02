/******************************************************************************
    Filename: cc1120_radio.h

    Description: header file for cc1120 radio functions

*******************************************************************************/
#ifndef CC1120_RADIO_H
#define CC1120_RADIO_H


/******************************************************************************
* INCLUDES
 */

#include "hal_types.h"
#include "hal_defs.h"


/******************************************************************************
 * CONSTANTS
 */

#define ISR_ACTION_REQUIRED 1
#define ISR_IDLE            0
#define PKTLEN              30

#define RX_FIFO_ERROR       0x11


/******************************************************************************
 * FUNCTIONS
 */

void CC1120powerupReset(void);
void CC1120sendPacket(uint8 *txBuffer, uint8 size);
void CC1120receivePacket(uint8 *rxBuffer, uint8 *size);

// copied from project cc1120_easy_link_vchip_boosterpack_for_MSP430_launchpad/cc1120_vchip_easy_link_trx.c
static void runTX(uint8 txBuffer[]);
static void runRX(void);
void CC1120registerConfig(void);
void CC1120createPacket(uint8 txBuffer[]);
void CC1120manualCalibration(void);



#endif
