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

#include "stdint.h"
#include "HardwareProfile.h"


#define SERIAL_BUF_OUT_SIZE		128
#define SERIAL_BUF_IN_SIZE		128

#pragma udata uart_buffer
static uint8_t serial_out[SERIAL_BUF_OUT_SIZE];
static uint8_t serial_in[SERIAL_BUF_IN_SIZE];
#pragma udata
static uint8_t tx_count, tx_in, tx_out;
static uint8_t rx_count, rx_in, rx_out;
static uint8_t discard;


void usart_init(void)
{
	TX_TRIS = 0;
	RX_TRIS = 1;
	RTS_TRIS = 0;
	CTS_TRIS = 1;
	TXSTA = 0;
	TXSTAbits.TXEN = 1;
	TXSTAbits.BRGH = 1;
	RCSTA = 0;
	RCSTAbits.SPEN = 1;
	RCSTAbits.CREN = 1;
	IPR1bits.RCIP = 0;
	IPR1bits.TXIP = 0;
	
	// See if we can use the high baud rate setting
	#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock() + 2 * BAUD_RATE) / BAUD_RATE / 4 - 1;
		TXSTAbits.BRGH = 1;
	#else   // Use the low baud rate setting
		SPBRG = (GetPeripheralClock() + 8 * BAUD_RATE) / BAUD_RATE / 16 - 1;
	#endif

	BAUDCON = 0;
	PIE1bits.RCIE = 1;
	
	tx_count = 0;
	tx_in = 0;
	tx_out = 0;
	rx_count = 0;
	rx_in = 0;
	rx_out = 0;
}	


void serial_tx_int(void)
{
	if (tx_count == 0) {
		// nothing to send. disable interrupt
		PIE1bits.TXIE = 0;
		return;
	}
	TXREG = serial_out[tx_out];
	if (++tx_out == SERIAL_BUF_OUT_SIZE) {
		tx_out = 0;
	}
	tx_count--;
}	


uint8_t usart_ok_to_send(void)
{
	return (tx_count < SERIAL_BUF_OUT_SIZE);
}	

void usart_send(uint8_t data)
{
	while (tx_count >= SERIAL_BUF_OUT_SIZE);
	INTCONbits.GIEL = 0;
	serial_out[tx_in] = data;
	if (++tx_in == SERIAL_BUF_OUT_SIZE) {
		tx_in = 0;
	}
	tx_count++;
	PIE1bits.TXIE = 1;
	INTCONbits.GIEL = 1;	
}

void usart_flush(void)
{
	while (tx_count != 0);
}	

void serial_rx_int(void)
{
	if (RCSTAbits.OERR) {
		// reset receive bit after overrun error
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
	if (RCSTAbits.FERR == 0) {
		if (rx_count < SERIAL_BUF_IN_SIZE) {
			serial_in[rx_in] = RCREG;
			if (++rx_in == SERIAL_BUF_IN_SIZE) {
				rx_in = 0;
			}
			rx_count++;
			
			return;
		}	
	}	
	// discard byte
	discard = RCREG;
	
	return;
}


uint8_t usart_get(uint8_t *data)
{
	if (rx_count == 0) return 0;
	INTCONbits.GIEL = 0;
	*data = serial_in[rx_out];
	if (++rx_out == SERIAL_BUF_IN_SIZE) {
		rx_out = 0;
	}
	rx_count--;
	INTCONbits.GIEL = 1;
	
	return 1;
}		


void usart_puts(const rom char *str)
{
	while (*str) {
		usart_send(*(str++));
	}		
}

void usart_puts2(const rom char *str)
{
	usart_puts(str);
	usart_send('\r');
	usart_send('\n');	
}


void usart_puts_r(const char *str)
{
	while (*str) {
		usart_send(*(str++));
	}		
}

void usart_puts2_r(const char *str)
{
	usart_puts_r(str);
	usart_send('\r');
	usart_send('\n');	
}
	
void usart_put_hex(char c)
{
	char x;
	
	x = (c & 0xF0) >> 4;
	if (x > 9) 
		x += 'A' - 10;
	else
		x += '0';
	usart_send(x);
	x = c & 0x0F;
	if (x > 9)
		x += 'A' - 10;
	else
		x += '0';
	usart_send(x);
}	


void usart_put_short(short x)
{
	usart_put_hex((x >> 8) & 0xff);
	usart_put_hex(x & 0xff);	
}	

void usart_put_long(long x)
{
	usart_put_hex((x >> 24) & 0xff);
	usart_put_hex((x >> 16) & 0xff);
	usart_put_hex((x >> 8) & 0xff);
	usart_put_hex(x & 0xff);
}
