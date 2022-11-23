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

/*
 * TRS UART interface
 *
 * 
 * Addr |  read    | write
 * -----+----------+----------
 *  C6  |  status  | control
 *  C7  |  rx data | tx data
 *
 * The serial routines are buffered, but we must use a double-buffering
 * scheme. We cannot afford to call the send() and get() routines from
 * high priority interrupt, which is why a double-buffer (1 byte) is used.
 */

#include <string.h>
#include "HardwareProfile.h"
#include "integer.h"
#include "serial.h"
#include "trs_uart.h"

UCHAR tx_char;
UCHAR rx_char;
UCHAR tx_full;
UCHAR trs_uart_status;



void trs_uart_init(void)
{
	tx_full = 0;
	trs_uart_status = TRS_UART_TX_EMPTY;
}

	
/* 
 * trs_uart implements the double-buffering between the trs I/O routines
 * and the serial routines.
 *
 * This function is called in the main loop.
 */
void trs_uart(void)
{
	// something to send ?
	if (tx_full) {
		usart_send(tx_char);
		tx_full = 0;	
	}
	// update "tx_empty" status
	if (usart_ok_to_send()) {
		trs_uart_status |= TRS_UART_TX_EMPTY;
	} else {
		trs_uart_status &= ~TRS_UART_TX_EMPTY;
	}
	
	// rx_avail == 0 && something arrived ?
	if (!(trs_uart_status & TRS_UART_RX_AVAIL)) {
		if (usart_get(&rx_char)) {
			trs_uart_status |= TRS_UART_RX_AVAIL;
		}
	}	
}	


/*
 * TRS sends a char
 */
void trs_write_uart(void)
{
	if (!tx_full) {
		tx_char = TRS_DATA_IN;
		tx_full = 1;
		trs_uart_status &= ~TRS_UART_TX_EMPTY;
	} else {
		trs_uart_status |= TRS_UART_TX_OF;
	}		
}


/*
 * TRS reads a char
 */
void trs_read_uart(void)
{
	if (trs_uart_status & TRS_UART_RX_AVAIL) {
		TRS_DATA_OUT = rx_char;
		trs_uart_status &= ~TRS_UART_RX_AVAIL;
	} else {	
		trs_uart_status |= TRS_UART_RX_UF;
		TRS_DATA_OUT = 0xff;
	}
	TRS_DATA_TRIS = 0;
}


/*
 * TRS reads UART status
 */
void trs_read_uart_status(void)
{
	TRS_DATA_OUT = trs_uart_status;
	TRS_DATA_TRIS = 0;
}

	
void trs_write_uart_ctrl(void)
{
	
}
	
