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
 * ======== hal.h ========
 *
 * Device and board specific pins need to be configured here
 *
 */


/******************************************************************************
 * INCLUDES
 */
#include <msp430.h>
#include "hal_types.h"
#include "hal_defs.h"
#include "stdint.h"


/******************************************************************************
 * CONSTANTS
 */

#define ONE_SECOND  8000000        // Well, it's about a second
#define HALF_SECOND 4000000

#define LF_CRYSTAL_FREQUENCY_IN_HZ     32768
#define HF_CRYSTAL_FREQUENCY_IN_HZ     4000000

#define MCLK_DESIRED_FREQUENCY_IN_KHZ  8 * 1000
#define MCLK_FLLREF_RATIO              MCLK_DESIRED_FREQUENCY_IN_KHZ / ( UCS_REFOCLK_FREQUENCY/1024 )   // ratio = 250


#define     SPI_PORT_SEL          P3SEL
#define     SPI_PORT_DIR          P3DIR
#define     SPI_PORT_OUT          P3OUT
#define     SPI_PORT_IN           P3IN

#define     SPI_MOSI_PIN          BIT0
#define     SPI_MISO_PIN          BIT1
#define     SPI_SCLK              BIT2

#define     CS_N_PORT_SEL         P2SEL
#define     CS_N_PORT_DIR         P2DIR
#define     CS_N_PORT_OUT         P2OUT

#define     CS_N_PIN              BIT2

#define GPIO_ALL	GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3| \
					GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7

//CC1120 GPIO1 shared with SPI MISO (active when CS_N high)
#define CC1120_GPIO0_PORT		GPIO_PORT_P2
#define CC1120_GPIO0_PIN		GPIO_PIN0
#define CC1120_GPIO1_PORT		GPIO_PORT_P3
#define CC1120_GPIO1_PIN		GPIO_PIN1
#define CC1120_GPIO2_PORT		GPIO_PORT_P6
#define CC1120_GPIO2_PIN		GPIO_PIN5
#define CC1120_GPIO3_PORT		GPIO_PORT_P8
#define CC1120_GPIO3_PIN		GPIO_PIN1

// When the timer reaches the value in CCR0, int generated.
// ACLK= 32768 Hz
// 8000 = 1s, 4000 = 500ms
#define TA0CCR0_VAL		50000


/******************************************************************************
 * FUNCTIONS
 */

void initGPIO(void);
void initPorts(void);
void initClocks(uint32_t mclkFreq);
void initSPI(void);
void StartTimerA0(void);
void StopTimerA0(void);

