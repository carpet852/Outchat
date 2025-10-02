/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*
 * ======== hal.c ========
 *
 */

/*****************************************************************************
* INCLUDES
*/

#include "msp430.h"
#include "driverlib.h"
#include "hal.h"

#include "USB_API/USB_Common/device.h"
#include "USB_config/descriptors.h"


/*****************************************************************************
* GLOBAL VARIABLES
*/

uint32_t myACLK  = 0;
uint32_t mySMCLK = 0;
uint32_t myMCLK  = 0;

uint8_t  returnValue = 0;
bool     bReturn = STATUS_FAIL;


/******************************************************************************
 * CONSTANTS
 */

//#define __MSP430_HAS_PORT1_R__
//#define __MSP430_HAS_PORT2_R__
//#define __MSP430_HAS_PORT3_R__
//#define __MSP430_HAS_PORT4_R__
//#define __MSP430_HAS_PORT5_R__
//#define __MSP430_HAS_PORT6_R__
//#define __MSP430_HAS_PORT7_R__
//#define __MSP430_HAS_PORT8_R__

/*
* This function drives all the I/O's as output-low, to avoid floating inputs
* (which cause extra power to be consumed).  This setting is compatible with  
 * TI FET target boards, the F5529 Launchpad, and F5529 Experimenters Board;  
 * but may not be compatible with custom hardware, which may have components  
 * attached to the I/Os that could be affected by these settings.  So if using
* other boards, this function may need to be modified.
*/

void initPorts(void)
{
#ifdef __MSP430_HAS_PORT1_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT2_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT3_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT4_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT5_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT6_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT7_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT8_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORT9_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_P9, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_P9, GPIO_ALL);
#endif

#ifdef __MSP430_HAS_PORTJ_R__
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_ALL);
    GPIO_setAsOutputPin(GPIO_PORT_PJ, GPIO_ALL);
#endif
}




//*****************************************************************************
// Initialize clock system
//*****************************************************************************

void initClocks(uint32_t mclkFreq)
{
	// Assign the REFO as the FLL reference clock
	UCS_initClockSignal(
	   UCS_FLLREF,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

	// Assign the REFO as the source for ACLK
	UCS_initClockSignal(
	   UCS_ACLK,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

	// Configure the FLL frequency and set MCLK & SMCLK to use the FLL as their source
	// The UCS initFLL functions configure both MCLK and SMCLK. A common mistake is to configure SMCLK before calling the FLL init function
	UCS_initFLLSettle(
        mclkFreq/1000,
        mclkFreq/32768);

	// The XT2 frequency is board specific and needs to be specified by the
	// application. However this information needs to be set in the Descriptor
	// Tool (or in descriptors.h). The USB API will start and stop XT2 based on
	// USB connection state.

    //use REFO for FLL and ACLK
    //UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
    //UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

}


/*
void initClocks(void) {

    **************************************************************************
     Configure Oscillators
    **************************************************************************

    // Initialize the XT1 and XT2 crystal frequencies being used
    //  so driverlib knows how fast they are
    UCS_setExternalClockSource(
            LF_CRYSTAL_FREQUENCY_IN_HZ,
            HF_CRYSTAL_FREQUENCY_IN_HZ
    );

    // Verify if the default clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();

    // Initialize XT1. Returns STATUS_SUCCESS if initializes successfully.
    bReturn = UCS_LFXT1StartWithTimeout(
            UCS_XT1_DRIVE0,
            UCS_XCAP_3,
            XT_TIMEOUT
            );

    if (bReturn == STATUS_FAIL )
    {
        while(1); // modify and choose an alternative clock source
    }

    // Initializes the XT2 crystal oscillator with no timeout.
    // In case of failure, code hangs here.
    // For time-out instead of code hang use UCS_LFXT2StartWithTimeout().
    UCS_XT2Start( UCS_XT2DRIVE_4MHZ_8MHZ );

//        bReturn = UCS_XT2StartWithTimeout(
//                                           UCS_XT2DRIVE_4MHZ_8MHZ,
//                                           XT2_TIMEOUT
//                  );
//
//        if (bReturn == STATUS_FAIL )
//        {
//          while(1);
//        }

    **************************************************************************
     Configure Clocks
    **************************************************************************

    // Select XT1 as ACLK source
    UCS_clockSignalInit(
            UCS_ACLK,
            UCS_XT1CLK_SELECT,
            UCS_CLOCK_DIVIDER_1
            );

    // Select XT2 as SMCLK source
    UCS_clockSignalInit(
            UCS_SMCLK,
            UCS_XT2CLK_SELECT,
            UCS_CLOCK_DIVIDER_1
            );

    // Set the FLL's reference clock source
    UCS_clockSignalInit(
            UCS_FLLREF,                                  // Clock you're configuring
            UCS_REFOCLK_SELECT,                          // Clock source
            UCS_CLOCK_DIVIDER_1                          // Divide down clock source by this much
    );

    // Configure the FLL's frequency and set MCLK & SMCLK to use the FLL as their source
    // The UCS initFLL functions configure both MCLK and SMCLK. A common mistake is to configure SMCLK before calling the FLL init function
    UCS_initFLLSettle(
            MCLK_DESIRED_FREQUENCY_IN_KHZ,               // MCLK frequency
            MCLK_FLLREF_RATIO                            // Ratio between MCLK and FLL's reference clock source
    );

    // Verify that the modified clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();

    // Select XT2 as SMCLK source
    // Re-doing this call ... if you use UCS_initFLLSettle to
    //   setup MCLK, it also configures SMCLK; therefore, you
    //   should call this function afterwards
    UCS_clockSignalInit(
            UCS_SMCLK,
            UCS_XT2CLK_SELECT,
            UCS_CLOCK_DIVIDER_1
            );

    // Verify that the modified clock settings are as expected
    myACLK  = UCS_getACLK();
    mySMCLK = UCS_getSMCLK();
    myMCLK  = UCS_getMCLK();
}
*/

//*****************************************************************************
// Initialize GPIO
//*****************************************************************************
void initGPIO(void) {

    // Set P1.0,P4.7 to output direction and turn LED off
    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN0 );                   // Red LED1
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN0 );
    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN7 );                   // Green LED2
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN7 );

    // Set P2.1 as input with PUP
    // configure P2.1 interrupt on low-to-high transition
    // and then clear flag and enable the interrupt
    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN1 );	// button S1
    GPIO_selectInterruptEdge( GPIO_PORT_P2, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION );
    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN1 );
    GPIO_enableInterrupt( GPIO_PORT_P2, GPIO_PIN1 );

    // Set P1.1 as input with PUP
	// configure P1.1 interrupt on low-to-high transition
	// and then clear flag and enable the interrupt
	GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P1, GPIO_PIN1 );	// button S2
	GPIO_selectInterruptEdge( GPIO_PORT_P1, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION );
	GPIO_clearInterrupt( GPIO_PORT_P1, GPIO_PIN1 );
	GPIO_enableInterrupt( GPIO_PORT_P1, GPIO_PIN1 );

    // Set P2.0 as input with PUP for connection to CC1120 GPIO0
    //  configure interrupt on high-low transition
    //  and then clear flag and enable the interrupt
    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN0 );	// cc1120 GPIO0
    GPIO_selectInterruptEdge( GPIO_PORT_P2, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION );
    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN0 );
    GPIO_enableInterrupt( GPIO_PORT_P2, GPIO_PIN0 );

	// Set P6.5 as input with PDW for connection to CC1120 GPIO2
	// Interrupt signals available for P1/P2 only
	GPIO_setAsInputPinWithPullDownResistor( GPIO_PORT_P6, GPIO_PIN5 );	// cc1120 GPIO2

	// Set P2.6 to output high: CC1120 RESET_N
	GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN6 );			// CC1120 RESET_N
	GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN6 );

    // Connect pins to clock crystals
	/*
	GPIO_setAsPeripheralModuleFunctionInputPin(
			GPIO_PORT_P5,
			GPIO_PIN5 +                          // XOUT on P5.5
			GPIO_PIN4 +                          // XIN  on P5.4
			GPIO_PIN3 +                          // XT2OUT on P5.3
			GPIO_PIN2                            // XT2IN  on P5.2
			);
	*/
}


/******************************************************************************
 * @fn          initSPI
 *
 * @brief		initialize SPI bus
 *
 * @param       none
 *
 * @return      none
 */

void initSPI(void)
{
  /* Configuration USCI_B
   * -  8-bit
   * -  Master Mode
   * -  3-pin
   * -  synchronous mode
   * -  MSB first
   * -  Clock phase select = captured on first edge Low to High
   * -  Inactive state is low
   * -  SMCLK as clock source
   * -  Spi clk is adjusted corresponding to systemClock as the highest rate
   *    supported by the supported radios: this could be optimized and done
   *    after chip detect.
   */

  // set pins high, change P3OUT before P3DIR to prevent glitches on the output lines
  //SPI_PORT_OUT |= SPI_MISO_PIN;
  CS_N_PORT_OUT |= CS_N_PIN;

  // Set directions
  CS_N_PORT_DIR |= CS_N_PIN;	//output
  SPI_PORT_DIR |= SPI_MOSI_PIN + SPI_SCLK;	//output
  SPI_PORT_DIR &= ~SPI_MISO_PIN;	//input

  // Set USCI in reset state
  UCB0CTL1 |= UCSWRST;

  // Init USCI registers
  UCB0CTL0  =  UCSYNC + UCMST  + UCMSB + UCCKPH;	// USCI SPI mode, 3-pin (UC7BIT=0), 8 bit data (UCMODEx=0), Master, MSB, Clock in phase lo-hi transition
  UCB0CTL1 |=  UCSSEL_2;	// Clock source for BRCLK: SMCLK

  // data rate: bitclock prescaler
  // fBitClock = fBRCLK/(UCBRx+1), here BRCLK/2
  UCB0BR1   =  0x00; // high byte
  UCB0BR0   =  0x02; // low byte
  //UCB0MCTL = 0;      // modulation ctrl register, reserved, always write as 0

  // Configure ports
  SPI_PORT_SEL |= SPI_MISO_PIN + SPI_MOSI_PIN + SPI_SCLK;	  //P3SEL => select peripheral module
  CS_N_PORT_SEL &= ~CS_N_PIN;	  // manually set CS_N to P2.2

  // Clear UCSWRST. Initialize USCI state machine
  UCB0CTL1 &= ~UCSWRST;

  // Enable interrupts UCxRXIE and UCxTXIE
  // UCB0IE |= UCRXIE;	// not needed, msp430 send R/W commands to cc1120
}


/******************************************************************************
 * @fn          StartTimerA0
 *
 * @brief		Configure and start Timer0_A5
 *
 * @param       none
 *
 * @return      none
 */

void StartTimerA0(void)
 {
	TA0CCTL0 = CCIE; // enable counter/compare register 0 interrupt in the CCTL0
	TA0CCR0 = TA0CCR0_VAL; // set TACCR0 value. When the timer reaches the value in CCR0, int generated
	TA0CTL = TASSEL_1 + MC_2; // Timer_A control register: selects the ACLK + sets the operation for continuous mode
 }

/******************************************************************************
 * @fn          StopTimerA0
 *
 * @brief		stop Timer0_A5
 *
 * @param       none
 *
 * @return      none
 */

void StopTimerA0(void)
 {
	TA0CTL = MC_0 + TACLR; // Timer_A control register: stop timer + clear counter
 }










