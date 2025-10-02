/******************************************************************************
    Filename: cc1120_radio.c

    Description: cc1120 radio functions

*******************************************************************************/


/******************************************************************************
* INCLUDES
 */

#include <msp430.h>
#include "driverlib.h"
#include "stdlib.h"

#include "hal.h"
#include "cc1120_radio.h"
#include "cc1120_spi.h"
#include "cc1120_reg_config.h"

/******************************************************************************
 * EXTERN VARIABLES
 */
extern volatile uint8_t bRFDataSentWaitConf_event;


/******************************************************************************
 * LOCAL VARIABLES
 */
static uint8 packetSemaphore;
static uint32 packetCounter;

//Initialize packet buffer of size PKTLEN + 1, array init
uint8 txBuffer[PKTLEN + 1] = { 0 };


/******************************************************************************
 * @fn          CC1120powerupReset
 *
 * @brief
 *
 * @param       none
 *
 * @return      none
 */

void CC1120powerupReset(void)
{
	//msp430 gpio & interrupt init in initGPIO() function

	// Calibrate radio according to cc1120 errata
	CC1120manualCalibration();

}

/******************************************************************************
 * @fn          CC1120sendPacket
 *
 * @brief
 *
* @param       *txBuffer
 *          		Pointer to the txbuffer containing the data to be transmitted
 *      		size
 *          		variable containing the size of the txbuffer
 *
 * @return      none
 */

void CC1120sendPacket(uint8 *txBuffer, uint8 size)
{

	//__disable_interrupt();

	// write packet to tx fifo
	cc112xSpiWriteTxFifo(txBuffer, size);

	// strobe TX to send packet
	trxSpiCmdStrobe(CC112X_STX);

	bRFDataSentWaitConf_event = TRUE;

	// PKT_SYNC_RXTX
	// wait for CC1120 GPIO0 to go low-high > SYNC word transmitted
	//while (!GPIO_getInputPinValue( CC1120_GPIO0_PORT, CC1120_GPIO0_PIN ));

	// wait for CC1120 GPIO2 to go high-low > end of packet transmitted
	//while (GPIO_getInputPinValue( CC1120_GPIO0_PORT, CC1120_GPIO0_PIN ));

	//__enable_interrupt();


}


/******************************************************************************
 * @fn          CC1120receivePacket
 *
 * @brief
 *
 * @param       *rxBuffer
 *          		Pointer to the rxbuffer where the incoming data should be stored
 *      		*size
 *          		Pointer to a variable where the size of the rxbuffer is saved
 *
 * @return      none
 */

void CC1120receivePacket(uint8 *rxBuffer, uint8 *size)
{
	uint8_t cc1120rxBytes, cc1120marcStatus;

	// Read number of bytes in rx fifo
	cc112xSpiReadReg(CC112X_NUM_RXBYTES, &cc1120rxBytes, 1);

	// Check that we have bytes in fifo
	if(cc1120rxBytes != 0){

	   // Read marcstate to check for RX FIFO error
	   cc112xSpiReadReg(CC112X_MARCSTATE, &cc1120marcStatus, 1);

	   // Mask out marcstate bits and check if we have a RX FIFO error
	   if((cc1120marcStatus & 0x1F) == RX_FIFO_ERROR){

		   // Flush RX Fifo
		   trxSpiCmdStrobe(CC112X_SFRX);
	   }
	   else {

		   // Read n bytes from rx fifo
		   cc112xSpiReadRxFifo((uint8_t*)rxBuffer, cc1120rxBytes);

		   // Save number of bytes read
		   *size = cc1120rxBytes;

		   // Read appended status bytes
		   // cc112xSpiReadRxFifo function returns status byte
	   }
	}

}

// functions not used
/*
******************************************************************************
 * @fn          runTX
 *
 * @brief       sends one packet. Updates packet counter and
 *              display for each packet sent.
 *
 * @param       pointer to txBuffer
 *
 * @return      none

static void runTX(uint8 txBuffer[])
{
	// Initialize packet buffer of size PKTLEN + 1, array init
	//uint8 txBuffer[PKTLEN + 1] = { 0 };

	// configure P2 bit6 as GPIO not peripheral P2SEL bit to zero
	//P2SEL &= ~0x40;

	// connect ISR function to GPIO0, interrupt on falling edge
	//trxIsrConnect(GPIO_0, FALLING_EDGE, &radioRxTxISR);

	// enable interrupt from GPIO_0
	//trxEnableInt(GPIO_0);

	// Set P2.0 as input with PUP for connection to CC1120 GPIO0
    //  configure interrupt on high-low transition
	//  and then clear flag and enable the interrupt
	GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN0 );
	GPIO_interruptEdgeSelect ( GPIO_PORT_P2, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION );
	GPIO_clearInterruptFlag ( GPIO_PORT_P2, GPIO_PIN0 );
	GPIO_enableInterrupt ( GPIO_PORT_P2, GPIO_PIN0 );

	// Calibrate radio according to cc1120 errata
	CC1120manualCalibration();

	// update packet counter
	packetCounter++;

	// write packet to tx fifo
	cc112xSpiWriteTxFifo(txBuffer, sizeof(txBuffer));

	// strobe TX to send packet
	trxSpiCmdStrobe(CC112X_STX);

	// wait for interrupt that packet has been sent.
	// (Assumes the GPIO connected to the radioRxTxISR function is set
	// to GPIOx_CFG = 0x06)
	while (!packetSemaphore)
		;

	// clear semaphore flag
	packetSemaphore = ISR_IDLE;


	// infinite loop

	while (TRUE) {
		// wait for button push
		if (halButtonPushed()) {
			//continiously sent packets until button is pressed
			do {
				// update packet counter
				packetCounter++;

				// create a random packet with PKTLEN + 2 byte packet counter + n x random bytes
				createPacket(txBuffer);

				// write packet to tx fifo
				cc112xSpiWriteTxFifo(txBuffer, sizeof(txBuffer));

				// strobe TX to send packet
				trxSpiCmdStrobe(CC112X_STX);

				// wait for interrupt that packet has been sent.
				// (Assumes the GPIO connected to the radioRxTxISR function is set
				// to GPIOx_CFG = 0x06)
				while (!packetSemaphore)
					;

				// clear semaphore flag
				packetSemaphore = ISR_IDLE;

				halLedToggle(LED1);
				__delay_cycles(250000);
				halLedToggle(LED1);

			} while (!halButtonPushed());
		}
	}


}

******************************************************************************
 * @fn          runRX
 *
 * @brief       puts radio in RX and waits for packets. Update packet counter
 *              and display for each packet received.
 *
 * @param       none
 *
 * @return      none

static void runRX(void)
{
	uint8 rxBuffer[128] = {0};
	uint8 rxBytes, marcStatus;

	//P2SEL &= ~0x40; // P2SEL bit 6 (GDO0) set to one as default. Set to zero (I/O)
	// connect ISR function to GPIO0, interrupt on falling edge
	//trxIsrConnect(GPIO_0, FALLING_EDGE, &radioRxTxISR);

	// enable interrupt from GPIO_0
	//trxEnableInt(GPIO_0);

	// Set P2.0 as input with PUP for connection to CC1120 GPIO0
	//  configure interrupt on high-low transition
	//  and then clear flag and enable the interrupt
	GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN0 );
	GPIO_interruptEdgeSelect ( GPIO_PORT_P2, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION );
	GPIO_clearInterruptFlag ( GPIO_PORT_P2, GPIO_PIN0 );
	GPIO_enableInterrupt ( GPIO_PORT_P2, GPIO_PIN0 );

	// Calibrate radio according to errata
	CC1120manualCalibration();

	// set radio in RX
	trxSpiCmdStrobe(CC112X_SRX);

	// reset packet counter
	packetCounter = 0;

	// Infinite loop
	while(TRUE){

		// Wait for packet received interrupt
		if(packetSemaphore == ISR_ACTION_REQUIRED){

		  // Read number of bytes in rx fifo
		  cc112xSpiReadReg(CC112X_NUM_RXBYTES, &rxBytes, 1);

		  // Check that we have bytes in fifo
		  if(rxBytes != 0){

			// Read marcstate to check for RX FIFO error
			cc112xSpiReadReg(CC112X_MARCSTATE, &marcStatus, 1);

			// Mask out marcstate bits and check if we have a RX FIFO error
			if((marcStatus & 0x1F) == RX_FIFO_ERROR){

			  // Flush RX Fifo
			  trxSpiCmdStrobe(CC112X_SFRX);
			}
			else{

			  // Read n bytes from rx fifo
			  cc112xSpiReadRxFifo(rxBuffer, rxBytes);

			  // Check CRC ok (CRC_OK: bit7 in second status byte)
			  // This assumes status bytes are appended in RX_FIFO
			  // (PKT_CFG1.APPEND_STATUS = 1.)
			  // If CRC is disabled the CRC_OK field will read 1
			  if(rxBuffer[rxBytes-1] & 0x80){

				// Update packet counter
				packetCounter++;

			  }
			}
		  }
		  // Reset packet semaphore
		  packetSemaphore = ISR_IDLE;

		  // Set radio back in RX
		  trxSpiCmdStrobe(CC112X_SRX);

		}
	}


}


*******************************************************************************
 * @fn          radioRxTxISR
 *
 * @brief       ISR for packet handling in RX. Sets packet semaphore, puts radio
 *              in idle state and clears isr flag.
 *
 * @param       none
 *
 * @return      none
 *

static void radioRxTxISR(void)
{

	// set packet semaphore
	packetSemaphore = ISR_ACTION_REQUIRED;
	// clear isr flag
	//TBD
}



*******************************************************************************
 * @fn          trxIsrConnect
 *
 * @brief       Connects an ISR function to PORT1 interrupt vector and
 *              configures the interrupt to be a high-low transition.
 *
 * input parameters
 *
 * @param       pF  - function pointer to ISR
 *
 * output parameters
 *
 * @return      void
 *

void trxIsrConnect(uint8 gpio, uint8 edge, ISR_FUNC_PTR pF)
{
  digio io;

  switch(gpio)
  {
   case GPIO_3:
    io = gpio3;
    break;
   case GPIO_2:
    io = gpio2;
    break;
   case GPIO_0:
    io = gpio0;
    break;
   default:
    io = gpio0;
    break;
  }

  // Assigning ISR function
  halDigio2IntConnect(io, pF);
  // Setting rising or falling edge trigger
  halDigio2IntSetEdge(io, edge);
  return;
}

*******************************************************************************
 * @fn          trxEnableInt
 *
 * @brief       Enables sync interrupt
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * @return      void
 *
void trxEnableInt(uint8 gpio)
{
  digio io;

  switch(gpio)
  {
   case GPIO_3:
    io = gpio3;
    break;
   case GPIO_2:
    io = gpio2;
    break;
   case GPIO_0:
    io = gpio0;
    break;
   default:
    io = gpio0;
    break;
  }

  halDigio2IntEnable(io);
  return;
}

 */




/*******************************************************************************
 * @fn          CC1120registerConfig
 *
 * @brief       Write register settings as given by SmartRF Studio
 *
 * @param       none
 *
 * @return      none
 */
void CC1120registerConfig(void) {
	uint8 writeByte;

	// reset radio
	trxSpiCmdStrobe(CC112X_SRES);
	// write registers to radio
	uint16 i;
	for (i = 0; i < (sizeof preferredSettings / sizeof(registerSetting_t));
			i++) {
		writeByte = preferredSettings[i].data;
		cc112xSpiWriteReg(preferredSettings[i].addr, &writeByte, 1);
	}
}

/******************************************************************************
 * @fn          CC1120createPacket
 *
 * @brief       This function is called before a packet is transmitted. It fills
 *              the txBuffer with a packet consisting of a length byte, two
 *              bytes packet counter and n random bytes.
 *
 *              The packet format is as follows:
 *              |--------------------------------------------------------------|
 *              |           |           |           |         |       |        |
 *              | pktLength | pktCount1 | pktCount0 | rndData |.......| rndData|
 *              |           |           |           |         |       |        |
 *              |--------------------------------------------------------------|
 *               txBuffer[0] txBuffer[1] txBuffer[2]  ......... txBuffer[PKTLEN]
 *
 * @param       pointer to start of txBuffer
 *
 * @return      none
 */
void CC1120createPacket(uint8 txBuffer[]) {

	txBuffer[0] = PKTLEN; // Length byte
	txBuffer[1] = (uint8) packetCounter >> 8; // MSB of packetCounter
	txBuffer[2] = (uint8) packetCounter; // LSB of packetCounter

	// fill rest of buffer with random bytes
	uint8 i;
	for (i = 3; i < (PKTLEN + 1); i++) {
		txBuffer[i] = (uint8) rand();
	}
}

/******************************************************************************
 * @fn          CC1120manualCalibration
 *
 * @brief       calibrates radio according to CC112x errata
 *
 * @param       none
 *
 * @return      none
 */
#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2

void CC1120manualCalibration(void) {

	uint8 original_fs_cal2;
	uint8 calResults_for_vcdac_start_high[3];
	uint8 calResults_for_vcdac_start_mid[3];
	uint8 marcstate;
	uint8 writeByte;

	// 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
	writeByte = 0x00;
	cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

	// 2) Start with high VCDAC (original VCDAC_START + 2):
	cc112xSpiReadReg(CC112X_FS_CAL2, &original_fs_cal2, 1);
	writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
	cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

	// 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
	trxSpiCmdStrobe(CC112X_SCAL);

	do {
		cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
	} while (marcstate != 0x41);

	// 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
	cc112xSpiReadReg(CC112X_FS_VCO2,
			&calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
	cc112xSpiReadReg(CC112X_FS_VCO4,
			&calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
	cc112xSpiReadReg(CC112X_FS_CHP,
			&calResults_for_vcdac_start_high[FS_CHP_INDEX], 1);

	// 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
	writeByte = 0x00;
	cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

	// 6) Continue with mid VCDAC (original VCDAC_START):
	writeByte = original_fs_cal2;
	cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

	// 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
	trxSpiCmdStrobe(CC112X_SCAL);

	do {
		cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
	} while (marcstate != 0x41);

	// 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
	cc112xSpiReadReg(CC112X_FS_VCO2,
			&calResults_for_vcdac_start_mid[FS_VCO2_INDEX], 1);
	cc112xSpiReadReg(CC112X_FS_VCO4,
			&calResults_for_vcdac_start_mid[FS_VCO4_INDEX], 1);
	cc112xSpiReadReg(CC112X_FS_CHP,
			&calResults_for_vcdac_start_mid[FS_CHP_INDEX], 1);

	// 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
	if (calResults_for_vcdac_start_high[FS_VCO2_INDEX]
			> calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) {
		writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
		cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
		writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
		cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
		writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
		cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
	} else {
		writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
		cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
		writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
		cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
		writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
		cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
	}
}


