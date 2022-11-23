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

#include "HardwareProfile.h"
#include <stdio.h>
#include <string.h>
#include <timers.h>
#include "FatFS/ff.h"
#include "FatFS/diskio.h"
#include "trs_hard.h"
#include "trs_extra.h"
#include "trs_uart.h"
#include "sw_i2c.h"
#include "ds1307.h"
#include "action.h"
#include "serial.h"
#include "led.h"
#include "bootloader.inc"

/* Hardware configuration */
#pragma config OSC = HSPLL
#pragma config PWRT = ON
#pragma config WDT = OFF
#pragma config MCLRE = ON
#pragma config PBADEN = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTC
#pragma config XINST = OFF
#pragma config DEBUG = OFF
// protect bootloader 0000-07FF
#pragma config WRTB = ON


/* Global variables */
#pragma udata fs_buffer
FATFS fs;
Drive state_d[TRS_HARD_MAXDRIVES];
FIL state_file2;
#if EXTRA_IM_SUPPORT
#if _USE_FASTSEEK
DWORD im_stbl[FAST_SEEK_LEN];
#endif
FIL im_file;
BYTE im_buf[0x80];
#endif
BYTE sector_buffer[MAX_SECTOR_SIZE];
#if EXTRA_IM_SUPPORT
image_t im[8];
#endif
#pragma udata extra_buffer
BYTE extra_buffer[EXTRA_SIZE];
#pragma udata
DIR state_dir;
FILINFO state_fno;
UCHAR state_dir_open;
UCHAR state_file2_open;
UCHAR fs_mounted;
UCHAR cur_unit;
UCHAR led_count;
led_t rled;
led_t gled;

#pragma udata access trs_access
near BYTE action_flags;
near BYTE action_type;
near BYTE action_status;
near UCHAR state_busy;
near UCHAR state_status;
near UCHAR state_present;
near UCHAR state_control;
near UCHAR state_error;
near UCHAR state_seccnt;
near UCHAR state_secnum;
near USHORT state_cyl;
near UCHAR state_drive;
near UCHAR state_head;
near UCHAR state_wp;
near UCHAR state_command;
near USHORT state_bytesdone;
near UCHAR state_secsize;
near USHORT state_secsize16;
near UCHAR state_command2;
near UCHAR state_error2;
near USHORT state_size2;
near UCHAR state_bytesdone2;
near UCHAR state_romdone;
near UCHAR state_rom;
near UCHAR val_1F;
near UCHAR crc7;
near USHORT crc16;
near UCHAR foo;

/* local prototypes */
void handle_interrupt_low(void);
extern void _startup(void);


/*
 * Interrupt handlers
 *
 * hi-priority is fixed at address APP_HI_INT
 * lo-priority is used in the bootloader, but in non-bootloader mode, it
 *             jumps to APP_LO_INT
 * startup code is fixed at address APP_STARTUP
 */
#pragma code startup_app_code = APP_STARTUP
void app_startup(void)
{
	_asm
		goto _startup
	_endasm
}

#pragma code interrupt_code_low = APP_LO_INT
void interrupt_low(void)
{
	_asm
		goto handle_interrupt_low
	_endasm
}

#pragma code
#pragma interruptlow handle_interrupt_low
void handle_interrupt_low(void)
{
	if (PIR1bits.TMR2IF) {
		PIR1bits.TMR2IF = 0;
		disk_timerproc();
		action_flags |= ACTION_TICK;
	}
	if (RTC_INT_IF) {
		RTC_INT_IF = 0;
		ds1307_int();
	}
	if (PIR1bits.TXIF) {
		PIR1bits.TXIF = 0;
		serial_tx_int();
	}
	if (PIR1bits.RCIF) {
		PIR1bits.RCIF = 0;
		serial_rx_int();
	}
}


void pic_init(void)
{
	UCHAR i;

	/* SD Card */
	SD_CS_TRIS = OUTPUT_PIN;
	SD_WP_TRIS = INPUT_PIN;
	SD_CD_TRIS = INPUT_PIN;
	SPI_SCK_TRIS = OUTPUT_PIN;
	SPI_MISO_TRIS = INPUT_PIN;
	SPI_MOSI_TRIS = OUTPUT_PIN;
	SD_CS = 1;

	/* Status 74HC595 */
	STAT_CS_TRIS = OUTPUT_PIN;
	STAT_CS = 0;

	/* TRS80 and GAL */
	ADCON1 = 0x0f;
	TRS_ADDR_TRIS |= 0x0F;
	TRS_WAIT_TRIS = OUTPUT_PIN;
	TRS_WAIT = 0;
	TRS_RD_N_TRIS = INPUT_PIN;
	GAL_INT_TRIS = INPUT_PIN;
	GAL_INT_IE = 1;
	GAL_INT_IP = 1;
	GAL_INT_EDGE = 0;	// interrupt on falling edge

	/* DS1307 interrupt */
	RTC_INT_TRIS = INPUT_PIN;
	RTC_INT_IE = 1;
	RTC_INT_IP = 0;
	RTC_INT_EDGE = 0;

	/* Debug LED */
	GLED_TRIS = OUTPUT_PIN;
	RLED_TRIS = OUTPUT_PIN;
	GLED = 0;
	RLED = 0;

	/* Timer 2 : ~200Hz */
	OpenTimer2(TIMER_INT_ON & T2_PS_1_16 & T2_POST_1_16);
	PR2 = 195;
	IPR1bits.TMR2IP = 0;

	/* Enable SPI */
	SSPSTAT = 0x40;
	SSPCON1 = 0x21;

	/* Interrupt priorities */
	RCONbits.IPEN = 1;

	/* globals init */
	val_1F = 0x1F;
	action_flags = 0;
	fs_mounted = FS_NOT_MOUNTED;
	state_file2_open = 0;
	state_dir_open = 0;
	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		state_d[i].filename[0] = '\0';
	}
}


static void handle_card_insertion(void)
{
	FRESULT rc;
	BYTE retry;

	gled.val |= LED_FLASH;
	fs_mounted = FS_MOUNTED_ERROR;
#if UART_DEBUG
	usart_puts2("Card inserted!");
#endif
	retry = 3;
	while ((rc = disk_initialize(0)) != RES_OK && retry-- > 0);
	if (rc == RES_OK) {
#if UART_DEBUG
		usart_puts2("Card initialized!");
#endif
		rc = f_mount(0, &fs);
		if (rc == FR_OK) {
			fs_mounted = FS_MOUNTED_OK;
#if UART_DEBUG
			usart_puts2("FAT mounted");
#endif
			rc = open_drives();
#if UART_DEBUG
			if (rc == FR_OK) {
				usart_puts2("Drives mounted");
			}
#endif
		}
	}
	if (fs_mounted == FS_MOUNTED_ERROR) {
#if UART_DEBUG
		usart_puts2("Card init failed.");
#endif
		rled.val = LED_FAST_BLINK;
	}

	// capture result
	state_error2 = rc;
}


static void handle_card_removal(void)
{
#if UART_DEBUG
	usart_puts2("Card removed!");
#endif
	close_drives();
	f_mount(0, NULL);
	fs_mounted = 0;
	rled.val = LED_OFF;
}


void main(void)
{
	pic_init();
	led_init();
	i2c_init();
	ds1307_init();
	trs_hard_init();
	usart_init();
	trs_uart_init();

	/* Set READY status */
	state_busy = 0;
	update_status(TRS_HARD_READY);

	/* Enable interrupts */
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;

	while (1) {

		/* DS1307 : reload time from chip */
		if (action_flags & ACTION_DS1307_RELOAD) {
			ds1307_read_time();
			action_flags &= ~ACTION_DS1307_RELOAD;
		}

		/* tick 200Hz */
		if (action_flags & ACTION_TICK) {
			action_flags &= ~ACTION_TICK;

			/* handle LEDs */
			led_update(&rled);
			led_update(&gled);

			/* sync hard drive files */
			trs_sync();

			/* card insertion */
			if (card_present() && fs_mounted == FS_NOT_MOUNTED) {
				handle_card_insertion();
			}

			/* card removal */
			if (!card_present() && fs_mounted != FS_NOT_MOUNTED) {
				handle_card_removal();
			}
		}

		/* hard drive and extra functions actions */
		if (action_flags & ACTION_TRS) {
			action_flags &= ~ACTION_TRS;
			trs_action();
		}

#if 0
		/* UART */
		trs_uart();
#endif
	}
}
