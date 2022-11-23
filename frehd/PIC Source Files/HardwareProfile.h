/*
 * Copyright (C) 2013 Frederic Vecoven
 *
 * This file is part of trs_hard
 *
 * trs_hard is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * trs_hard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H
	
#include <p18f4620.h>
	
/** CLOCK FREQUENCY ********************************************/
#define GetSystemClock()		(40000000ul)      // Hz
#define GetInstructionClock()	(GetSystemClock()/4)
#define GetPeripheralClock()	GetInstructionClock()
   
/** I/O pin definitions ****************************************/
#define INPUT_PIN 		1
#define OUTPUT_PIN 		0    

/* SD CARD */
#define SD_CS			LATAbits.LATA5		// SD chip select
#define SD_CS_TRIS		TRISAbits.TRISA5	// SD chip select TRIS
#define SPI_SCK_TRIS	TRISCbits.TRISC3	// SPI Clock TRIS
#define SPI_MISO_TRIS	TRISCbits.TRISC4	// SPI Master input/Slave output TRIS
#define SPI_MOSI_TRIS	TRISCbits.TRISC5	// SPI Master output/Slave input TRIS
/*
 * Card write-protect and card detect must be on the same port.
 * If not, adapt diskio.c
 */
#define SD_WP			PORTBbits.RB5		// Card Write Protect
#define SD_WP_TRIS		TRISBbits.TRISB5	// Card Write Protect TRIS
#define SD_WP_PIN		5
#define SD_CD			PORTBbits.RB4		// Card Detect
#define SD_CD_TRIS		TRISBbits.TRISB4	// Card Detect TRIS
#define SD_CD_PIN		4
#define SD_WP_CD_PORT	PORTB

/* STATUS REGISTER */
#define STAT_CS			LATCbits.LATC0		// STATUS LOAD
#define STAT_CS_TRIS	TRISCbits.TRISC0
   
/* TRS80 signals */
#define TRS_ADDR		PORTA				// low nibble is A[3..0] reversed!  RW is bit 4
#define TRS_ADDR_TRIS	TRISA
#define TRS_DATA_IN		PORTD
#define TRS_DATA_OUT	LATD
#define TRS_DATA_TRIS	TRISD
#define TRS_RD_N		PORTAbits.RA4		// 0=read 1=write
#define TRS_RD_N_TRIS	TRISAbits.TRISA4
#define TRS_WAIT		LATBbits.LATB3
#define TRS_WAIT_TRIS	TRISBbits.TRISB3
#define GAL_INT_IE		INTCON3bits.INT1IE
#define GAL_INT_IP		INTCON3bits.INT1IP
#define GAL_INT_IF		INTCON3bits.INT1IF
#define GAL_INT_EDGE	INTCON2bits.INTEDG1
#define GAL_INT_TRIS	TRISBbits.TRISB1
#define GAL_INT			PORTBbits.RB1

/* DS1307 RTC */
#define SDA_LAT			LATCbits.LATC1
#define SDA_PIN			PORTCbits.RC1
#define SDA_TRIS		TRISCbits.TRISC1
#define SCL_LAT			LATCbits.LATC2
#define SCL_PIN			PORTCbits.RC2
#define SCL_TRIS		TRISCbits.TRISC2
#define RTC_INT_IE		INTCON3bits.INT2IE
#define RTC_INT_IP		INTCON3bits.INT2IP
#define RTC_INT_IF		INTCON3bits.INT2IF
#define RTC_INT_EDGE	INTCON2bits.INTEDG2
#define RTC_INT_TRIS	TRISBbits.TRISB2
#define RTC_INT			PORTBbits.RB2

/* Serial Port */
#define TX_TRIS			TRISCbits.TRISC6
#define RX_TRIS			TRISCbits.TRISC7
#define RTS_TRIS		TRISEbits.TRISE0
#define RTS_LAT			LATEbits.LATE0
#define RTS_PIN			PORTEbits.RE0
#define CTS_TRIS		TRISEbits.TRISE1
#define CTS_LAT			LATEbits.LATE1
#define CTS_PIN			PORTEbits.RE1

/* LEDs */
#define GLED			LATBbits.LATB7		// green LED 
#define GLED_TRIS		TRISBbits.TRISB7	// green LED TRIS
#define RLED			LATBbits.LATB6		// red LED
#define RLED_TRIS		TRISBbits.TRISB6	// red LED TRIS

typedef enum {
	LED_OFF,
	LED_ON,
	LED_FAST_BLINK,
	LED_SLOW_BLINK
} led_state_t;
	

/* Spare pins */
#define SPARE1_TRIS		TRISBbits.TRISB0
#define SPARE2_TRIS		TRISEbits.TRISE2

/** USART SETTINGS *********************************************/
#define BAUD_RATE 57600

#endif
