/*
Program: Encoder Firmware
Author: Michael D. Stone (AKA Neoaikon)
Date: 3/8/2015
Copyright (C) 2015 Mid-Ohio Area Robotics
 *
Description: Allows an end user to program the
Muribot using a simple set of commands. These
commands are documented in the Muribot manual.
 *
GPL V3 License
-------------
This file is part of the Muribot Encoder Firmware.

	The Encoder Firmware is free software: you 
can redistribute it and/or modify it under the
terms of the GNU General Public License as
published by the Free Software Foundation,
either version 3 of the License, or (at your
option) any later version.

	The Encoder Firmware is distributed in the
hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License
for more details.

	You should have received a copy of the GNU
General Public License along with the Encoder
Firmware.  If not, see
http://www.gnu.org/licenses/
*/

// Defines
#define _XTAL_FREQ 32000000
#define DEVICE_ADDRESS 	0xEC
#define PACKET_SIZE 	0x0A
#define LeftA			RA5
#define LeftB			RA4
#define RightA			RA3
#define RightB			RA0

// Hi-Tech C Header
#include <htc.h>

// I2C Variables
unsigned char packet[PACKET_SIZE] @ 0x20;
unsigned char right_direction @0x20;
unsigned char left_direction @ 0x21;
signed long right_count @ 0x22;
signed long left_count @ 0x26;
unsigned char rx_byte = 0, data_idx;

// IOC Variables
unsigned char last_left_state, left_state;
unsigned char last_right_state, right_state;
unsigned char left_flag, right_flag;

// Interrupt vector
void interrupt ISR() {
	// I2C Interrupt Detected
	if(SSP1IF == 1) { 
		if(S == 1) { // Start bit has been detected
			if(R_nW == 1) { // Read flag has been set
				if(CKP == 0) { // We've got the clock
					if(D_nA == 0) { // This is the address byte
						rx_byte = SSP1BUF;
						rx_byte = packet[0x09];
						// Load the data and release the clock
						SSP1BUF = rx_byte;
						CKP = 1;					
					} else { // Otherwise it's the data byte
						rx_byte = packet[0x09-data_idx];
						// Load the data and release the clock
						SSP1BUF = rx_byte;
						CKP = 1;
					}
					// Increment the counter and clear the interrupt flag				
					data_idx++;
					if(data_idx >= PACKET_SIZE) { data_idx = 0x00; };
					SSP1IF = 0;
				}
			} else {
				// Write command resets the encoder
				left_count = 0;
				right_count = 0;
				left_direction = 0;
				right_direction = 0;
				rx_byte = SSP1BUF;
				CKP = 1;
			}
		}
	}
	// Interrupt-on-Change Detected
	if(IOCIF == 1) {
		// Store the states of the wheels from last time
		last_left_state = left_state;
		last_right_state = right_state;
		
		// Get the new states based on the encoder module outputs
		left_state = (LeftA<<1) + LeftB;
		right_state = (RightA<<1) + RightB;
		
		// Use state transitions to determine left wheel direction
		if(last_left_state == 0 && left_state == 1) left_direction = 1;
		else if(last_left_state == 1 && left_state == 3) left_direction = 1;
		else if(last_left_state == 2 && left_state == 0) left_direction = 1;
		else if(last_left_state == 3 && left_state == 2) left_direction = 1;
		else if(last_left_state == 0 && left_state == 2) left_direction = 0;
		else if(last_left_state == 2 && left_state == 3) left_direction = 0;
		else if(last_left_state == 3 && left_state == 1) left_direction = 0;
		else if(last_left_state == 1 && left_state == 0) left_direction = 0;
		
		// Use state transitions to determine right wheel direction
		if(last_right_state == 0 && right_state == 1) right_direction = 1;
		else if(last_right_state == 1 && right_state == 3) right_direction = 1;
		else if(last_right_state == 2 && right_state == 0) right_direction = 1;
		else if(last_right_state == 3 && right_state == 2) right_direction = 1;
		else if(last_right_state == 0 && right_state == 2) right_direction = 0;
		else if(last_right_state == 2 && right_state == 3) right_direction = 0;
		else if(last_right_state == 3 && right_state == 1) right_direction = 0;
		else if(last_right_state == 1 && right_state == 0) right_direction = 0;
		
		// Check which outputs caused the interrupts and clear the IOC flags		
		if(IOCAF0 == 1) {
		    IOCAF0 = 0;
		    right_flag = 1;
		} else if(IOCAF3 == 1) {
		    IOCAF3 = 0;
		    right_flag = 1;
		}
		
		if(IOCAF4 == 1) {
		    IOCAF4 = 0;
		    left_flag = 1;
		} else if(IOCAF5 == 1) {
		    IOCAF5 = 0;
		    left_flag = 1;
		}
		
		// Update the counts
		if(left_flag)
		{
		    if(left_direction == 1) left_count++;
		    else left_count--;
		}
		if(right_flag)
		{
		    if(right_direction == 1) right_count++;
		    else right_count--;
		}
		
		// Clear left/right flags
		left_flag = 0;
		right_flag = 0;
		// Make sure the IOC flags are cleared properly
		IOCAF = 0;
	}
}


void main() {
	// Configure 32Mhz Internal Oscillator
	OSCCON = 0b11110000;

	// All digital inputs
	ANSELA = 0;
	LATA = 0;
	TRISA = 0b00111111;

	// Enable weak pull-ups	
	nWPUEN = 0;
	WPUA = 0b00111001;

	// Configure MSSP module for 7-bit I2C Slave mode, no slew
	SSP1STAT = 0b10000000;
	SSP1CON1 = 0b00110110;
	SEN = 1; // Redundant?
	SSP1ADD = 0xEC;

	// Configure MSSP interrupts
	SSP1IE = 1;
	SSP1IF = 0;

	// Configure Interrupt-On-Change for positive and negative edges
	IOCAP = 0b00111001;
	IOCAN = 0b00111001;
	IOCIE = 1;
	IOCAF = 0;

	// Enable Interrupts
	PEIE = 1;
	GIE = 1;

	// Final initialization
	left_count = 0;
	left_direction = 0;	
	right_count = 0;
	right_direction = 0;

	while(1) {};
}
