// OUTCHAT project
// FW version: 0.00
// Author: SCa
// CCS version: 6.0.1
// driverlib version: 2.10.00.09
// USB dev package version: 5.00.00

#include <string.h>

#include "driverlib.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

#include "cc1120_spi.h"
#include "cc1120_radio.h"

/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */
#include "hal.h"

// Global flags set by events
volatile uint8_t bCDCDataReceived_event = FALSE;  // Flag set by event handler to
                                               // indicate data has been
                                               // received into USB buffer
volatile uint8_t bUSBSendNACK_event = FALSE;
volatile uint8_t bRFDataSent_event = FALSE;
volatile uint8_t bRFDataSentWaitConf_event = FALSE;
volatile uint8_t bRFAckSentWaitConf_event = FALSE;
volatile uint8_t bRFDataReceived_event = FALSE;
volatile uint8_t bGetRSSI_event = FALSE;
volatile uint8_t bGetFWver_event = FALSE;
volatile uint8_t hour = 4, min = 30, sec = 00;  // Real-time clock (RTC) values.  4:30:00

#define FW_VER			000
#define SELF_ADDR		0x01
#define REMOTE_ADDR		0x02
#define BUFFER_SIZE1 	16
#define BUFFER_SIZE2 	64
#define BUFFER_OFFSET 	2

char USBinputBuffer[BUFFER_SIZE1] = "";
char DataInBuffer[BUFFER_SIZE2] = "";
char DataOutBuffer[BUFFER_SIZE2] = "";
char Buffer[12];
char ack[3] = "A\r";
//char ack[3] = "\x06\r";
char nack[3] = "N\r";
//char nack[3] = "\x15\r";
char lf = '\n';	// char is 'A', string is "A"
char cr = '\r';	// char is 'A', string is "A"
uint16_t count, count2, size;
uint16_t x,y;
uint8_t length;
int i;
int rssi_val;

// Function to remove the terminating null character of a string
// Attention overflow BUG: destination array must be large enough or stange things happen
void strtoarr(char *arr, const char *str)
{
	int len = strlen(str);	//size of string minus null character
	strncpy(arr, str, len);	// strncpy() does not always null terminate strings
}

// Function to check if two arrays are equal
// bool type not supported
char comp_array(char *arr1, char *arr2, int nbchar)
{
    int x = 0;
    char equal;
    while(arr1[x] == arr2[x] && x < nbchar) {
        x++;
    }
    if(x == nbchar) {
        equal = TRUE;
    } else {
        equal = FALSE;
    }
    return equal;
}

// Function to copy one array to another
void copy_array(char *dst, char *src, int n)
{
    while (n-- > 0)        // Loop that counts down from n to zero
        *dst++ = *src++;   // Copies element *(src) to *(dst),
                           //  then increments both pointers
}

// Function to convert a number 'bin' of value 0-99 into its ASCII equivalent.  Assumes
// str is a two-byte array.
void convertTwoDigBinToASCII(uint8_t bin, uint8_t* str)
{
    str[0] = '0';
    if (bin >= 10)
    {
        str[0] = (bin / 10) + 48;
    }
    str[1] = (bin % 10) + 48;
}

// Function to convert a signed int -32768 to 32768 into its ASCII equivalent.  Assumes result is a 6-byte array.
void convertSignedIntToASCII(signed int value, char* result)
{
	char* ptr = result;
	char* ptr1 = result;
	char tmp_char;
	signed int tmp_value;

	do {
		tmp_value = value;
		value /= 10;
		*ptr++ = "9876543210123456789" [9 + (tmp_value - value * 10)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
}

// Function to convert the binary globals hour/min/sec into a string, of format "hr:mn:sc"
// Assumes str is an nine-byte string.
void convertTimeBinToASCII(uint8_t *str)
{
uint8_t hourStr[2], minStr[2], secStr[2];
convertTwoDigBinToASCII(hour, hourStr);
convertTwoDigBinToASCII(min, minStr);
convertTwoDigBinToASCII(sec, secStr);
str[0] = hourStr[0];
str[1] = hourStr[1];
str[2] = ':';
str[3] = minStr[0];
str[4] = minStr[1];
str[5] = ':';
str[6] = secStr[0];
str[7] = secStr[1];
str[8] = '\n';
}


/*----------------------------------------------------------------------------+
 | Main Routine                                                                |
 +----------------------------------------------------------------------------*/
void main (void)
{
    //WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    // MSP430 USB requires a Vcore setting of at least 2.  2 is high enough for 8MHz MCLK.
    PMM_setVCore(PMM_CORE_LEVEL_2);

    initPorts();          	// Config GPIOS for low-power (output low)
    initGPIO();				// Initialize GPIO
    initClocks(8000000);   	// Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    initSPI();				// initialize USCI_B as SPI, clock source SMCLK=8MHz
    USB_setup(TRUE, TRUE); 	// Init USB & events; if a host is present, connect

    CC1120manualCalibration();
    CC1120registerConfig();
    trxSpiCmdStrobe(CC112X_SRX); // set CC1120 in RX mode
    // When a pkt is received, it will signal on GPIO0 and wake CPU up

    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN0 );	// clear GPIO2 IFG or an interrupt is trigged here
    __enable_interrupt();	// Enable interrupts globally
    //__bis_SR_register( GIE );         // Enable interrupts globally

    count2 = BUFFER_OFFSET;				// initialize USB DataOutBuffer index
    rssi_val = -128;

    while (1)
    {
        uint8_t USBReceiveError = 0, USBSendError = 0;

        // This switch() creates separate main loops, depending on whether USB
		// is enumerated and active on the host, or disconnected/suspended.  If
		// you prefer, you can eliminate the switch, and just call
		// USB_getConnectionState() prior to sending data (to ensure the state is
		// ST_ENUM_ACTIVE).
		switch (USB_getConnectionState())
		{
			// This case is executed while your device is connected to the USB
			// host, enumerated, and communication is active.  Never enter
			// LPM3/4/5 in this mode; the MCU must be active or LPM0 during USB
			// communication.
			case ST_ENUM_ACTIVE:

				/*
				// Sleep if there are no bytes to process.
				__disable_interrupt();
				if (!USBCDC_getBytesInUSBBuffer(CDC0_INTFNUM)) {

					// Enter LPM0 until awakened by an event handler
					__bis_SR_register(LPM0_bits + GIE);
				}

				__enable_interrupt();

				// Exit LPM because of a data-receive event, and
				// fetch the received data
				*/


				// data received on USB CDC interface
				if (bCDCDataReceived_event) {

                	// Clear flag early -- just in case execution breaks
                	// below because of an error
					bCDCDataReceived_event = FALSE;

					count = USBCDC_receiveDataInBuffer((uint8_t*)USBinputBuffer, BUFFER_SIZE1, CDC0_INTFNUM);

					// strlen: returns the length of the string not including the terminating "\0" character
					// sizeof: returns the array size in bytes
					for (i = 0; i < count; i++) {
						if (count2<BUFFER_SIZE2) {
							DataOutBuffer[count2] = USBinputBuffer[i];
							count2++;
						}
					}
					if (USBinputBuffer[count-1] == cr) {	// press ENTER to send data
						__disable_interrupt();
						DataOutBuffer[0] = count2-1;	// packet length byte = addr byte + data bytes
						DataOutBuffer[1] = REMOTE_ADDR;	// packet addr byte
						CC1120sendPacket((uint8*)DataOutBuffer, count2);
						__enable_interrupt();
					}
                }

				// data sent on RF interface
				else if (bRFDataSent_event)	{
					bRFDataSent_event = FALSE;
					trxSpiCmdStrobe(CC112X_SRX);	// return to RX mode
					if (!bRFAckSentWaitConf_event) {	// data sent by CC1120 is not a ACK
						// Echo back to the host.
						DataOutBuffer[0] = 'T';
						DataOutBuffer[1] = 0x3E;	// ascii >
						if (USBCDC_sendDataInBackground((uint8_t*)DataOutBuffer, count2, CDC0_INTFNUM, 1))	{
							USBSendError = 0x01;
							USBCDC_abortSend(&size,CDC0_INTFNUM);	//Operation probably still open; cancel it
							break;	// Exit the while loop if something went wrong.
						}
						StartTimerA0();				// start timer and wait for ACK message
						count2 = BUFFER_OFFSET;		// reinitialize USB DataOutBuffer index
					}
					else {							// data sent by CC1120 is a ACK
						bRFAckSentWaitConf_event = FALSE;
					}
					memset(DataOutBuffer, '\0', sizeof(DataOutBuffer)); // clear Buffer
					GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);	// Red LED1 OFF => radio Tx ended
				}

                // data received on RF interface
				else if (bRFDataReceived_event) {
					bRFDataReceived_event = FALSE;
					CC1120receivePacket((uint8*)DataInBuffer, &length);
					trxSpiCmdStrobe(CC112X_SRX);	// set radio back in RX mode
					DataInBuffer[0] = 'R';
					DataInBuffer[1] = 0x3E;	// ascii >
					// DataInBuffer[0] & DataInBuffer[1] can be erased by another interrupt before USBCDC_sendData has completed
					// use separate buffers for each sendDataInBackground operation, see Example C6_SendDataBackground
					// use USBCDC_sendDataAndWaitTillDone instead of USBCDC_sendDataInBackground to prevent buffer issue with alternating data/ack
					if (USBCDC_sendDataInBackground((uint8_t*)DataInBuffer, length-2, CDC0_INTFNUM, 1)){	// remove 2 status bytes appended after data in RXFIFO
						USBSendError = 0x01;
						USBCDC_abortSend(&size,CDC0_INTFNUM);	//Operation probably still open; cancel it
						break;	// Exit the while loop if something went wrong.
					}
					//if (!comp_array(&(DataInBuffer[BUFFER_OFFSET]),ack,2)) {
					if (!comp_array((DataInBuffer+BUFFER_OFFSET),ack,2)) {	// data received is not an ACK
						__disable_interrupt();
						Buffer[0] = 3;
						Buffer[1] = REMOTE_ADDR;
						Buffer[2] = ack[0];
						Buffer[3] = cr;
						CC1120sendPacket((uint8*)Buffer, 4);
						__enable_interrupt();
						bRFAckSentWaitConf_event = TRUE;
						memset(Buffer, '\0', sizeof(Buffer)); // clear Buffer
					}
					else {	// ACK received
						StopTimerA0();
						_NOP();
					}
					rssi_val = ((int)DataInBuffer[length-2])-102;		// RSSI(dB) = StatusByte1 - 102
					memset(DataInBuffer, '\0', sizeof(DataInBuffer)); 	// clear Buffer
					GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);	// Green LED2 OFF => radio Rx ended
                }

                // no ACK received on RF interface
				else if (bUSBSendNACK_event) {
					bUSBSendNACK_event = FALSE;
					StopTimerA0();
					// use separate buffers for each sendDataInBackground operation
					// see Example C6_SendDataBackground
					Buffer[0] = 'R';
					Buffer[1] = 0x3E;	// ascii >
					Buffer[2] = nack[0];
					Buffer[3] = cr;
					if (USBCDC_sendDataInBackground((uint8_t*)Buffer, 4, CDC0_INTFNUM, 1)){
						USBSendError = 0x01;
						USBCDC_abortSend(&size,CDC0_INTFNUM);	//Operation probably still open; cancel it
						break;	// Exit the while loop if something went wrong.
					}
					memset(Buffer, '\0', sizeof(Buffer)); // clear Buffer
                }

				else if (bGetRSSI_event) {
					bGetRSSI_event = FALSE;
					Buffer[0] = 'R';
				    Buffer[1] = 'S';
				    Buffer[2] = 'S';
				    Buffer[3] = 'I';
				    Buffer[4] = '>';
				    Buffer[11] = cr;
				    convertSignedIntToASCII(rssi_val, Buffer+5);
					if (USBCDC_sendDataInBackground((uint8_t*)Buffer, 12, CDC0_INTFNUM, 1)) {
						USBSendError = 0x01;
						USBCDC_abortSend(&size,CDC0_INTFNUM);	//Operation probably still open; cancel it
						break;	// Exit the while loop if something went wrong.
					}
					memset(Buffer, '\0', sizeof(Buffer)); // clear Buffer
				}

				else if (bGetFWver_event) {
					bGetFWver_event = FALSE;
					Buffer[0] = 'F';
					Buffer[1] = 'W';
					Buffer[2] = '>';
					Buffer[11] = cr;
					convertSignedIntToASCII((int)FW_VER, Buffer+3);
					if (USBCDC_sendDataInBackground((uint8_t*)Buffer, 12, CDC0_INTFNUM, 1)) {
						USBSendError = 0x01;
						USBCDC_abortSend(&size,CDC0_INTFNUM);	//Operation probably still open; cancel it
						break;	// Exit the while loop if something went wrong.
					}
					memset(Buffer, '\0', sizeof(Buffer)); // clear Buffer
				}

				break;

            // These cases are executed while your device is:
			case ST_USB_DISCONNECTED: // physically disconnected from the host
			case ST_ENUM_SUSPENDED:   // connected/enumerated, but suspended
			case ST_NOENUM_SUSPENDED: // connected, enum started, but the host is unresponsive

				// In this example, for all of these states we enter LPM3.  If
				// the host performs a "USB resume" from suspend, the CPU will
				// automatically wake.  Other events can also wake the
				// CPU, if their event handlers in eventHandlers.c are
				// configured to return TRUE.
				__bis_SR_register(LPM3_bits + GIE);
                _NOP();
                break;

            // The default is executed for the momentary state
            // ST_ENUM_IN_PROGRESS.  Usually, this state only last a few
            // seconds.  Be sure not to enter LPM3 in this state; USB
            // communication is taking place here, and therefore the mode must
            // be LPM0 or active-CPU.
            case ST_ENUM_IN_PROGRESS:
            default:;
        }

        if (USBReceiveError || USBSendError){
            // TO DO: User can place code here to handle error
        }

    }  							//while(1)
}                               // main()





//*****************************************************************************
// Interrupt Service Routines
//*****************************************************************************

// UMNI ISR: User Non Maskable Interrupts
// edge on the RST/NMI pin when configured in NMI mode, oscillator fault, access violation to the flash memory


#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCS_clearFaultFlag(UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_DCOFFG);
            SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.
            SYSBERRIV = 0; // clear bus error flag
            USB_disable(); // Disable
    }
}


// TimerA0 ISR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA0_ISR (void)
 {
	bUSBSendNACK_event = TRUE;
 }


// GPIO P2.0 ISR
#pragma vector=PORT2_VECTOR
__interrupt void port2_ISR (void)
{
	switch( __even_in_range( P2IV, 10 )) {		// Reading PxIV returns highest priority interrupt and clears it’s IFG bit in PxIFG
		   case 0x00: break;					// none
		   case 0x02: 							// Pin 0: CC1120 Tx/Rx interrupt
			   if (!bRFDataSentWaitConf_event)	{		// Rx interrupt
				   bRFDataReceived_event = TRUE;
				   GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);	// Green LED2 ON => radio Rx
			   }
			   else	{									// Tx interrupt
				   bRFDataSentWaitConf_event = FALSE;
				   bRFDataSent_event = TRUE;
				   GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);	// Red LED1 ON => radio Tx
			   }
			   //GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN0 );	// clear interrupt flag
			   //GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0 );	// Toggle the LED on/off
			   break;
		   case 0x04:							// Pin 1: test button S1 P2.1
			   bGetRSSI_event = TRUE;
			   break;
		   case 0x06: break;                    // Pin 2
		   case 0x08: break;                    // Pin 3
		   case 0x0A: break;                    // Pin 4
		   case 0x0C: break;                    // Pin 5
		   case 0x0E: break;                    // Pin 6
		   case 0x10: break;                    // Pin 7
		   default:   _never_executed();
		}
}


#pragma vector=PORT1_VECTOR
__interrupt void port1_ISR (void)
{
	switch( __even_in_range( P1IV, 10 )) {
		   case 0x00: break;					// none
		   case 0x02: break; 					// Pin 0
		   case 0x04:							// Pin 1: test button S2 P1.1
			   bGetFWver_event = TRUE;
			   break;
		   case 0x06: break;                    // Pin 2
		   case 0x08: break;                    // Pin 3
		   case 0x0A: break;                    // Pin 4
		   case 0x0C: break;                    // Pin 5
		   case 0x0E: break;                    // Pin 6
		   case 0x10: break;                    // Pin 7
		   default:   _never_executed();
		}
}

