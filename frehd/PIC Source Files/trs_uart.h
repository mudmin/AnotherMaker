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

#ifndef _TRS_UART_H
#define _TRS_UART_H

/* Status bits */
#define TRS_UART_TX_OF		(1 << 7)
#define TRS_UART_TX_EMPTY	(1 << 6)
#define TRS_UART_RX_UF		(1 << 5)
#define TRS_UART_RX_AVAIL	(1 << 4)

/* prototypes */
void trs_uart_init(void);
void trs_uart(void);

/* trs I/O related to UART */
void trs_write_uart(void);
void trs_read_uart(void);
void trs_read_uart_status(void);
void trs_write_uart_ctrl(void);


#endif
