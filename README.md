# Outchat

## Objective
Outchat is a DIY project to build a communication system using RF ISM transceivers.  
There has been a debate about the pros & cons of spread-spectrum (i.e. LORA) VS narrow-band for long-range RF communications.  
I picked at the time a TI CC1120 narrow-band ISM transceiver after reading a [TI white paper](https://www.ti.com/lit/wp/swry006/swry006.pdf) on the subject.

## Harwdware
I assembled the hardware using a MSP430 USB development kit and a CC1120 433MHz demo board:
- TI MSP-EXP430F5529LP USB development kit
- Vchip CC1120 433MHz +15dBm booster pack

## Firmware
I wrote the MSP430 firmware using the IDE TI Code Composer Studio (CCS version: 6.0.1).  
The MCU firmware implements a USB CDC device that allows serial communication with a PC or smartphone.  
I configured the CC1120 registers using the TI tool [SmartRF Studio](https://www.ti.com/tool/SMARTRFTM-STUDIO).  

## App
The Android app was never finalized.

## Test
I debugged & tested the system on a PC using [Teraterm](https://teratermproject.github.io/index-en.html).
