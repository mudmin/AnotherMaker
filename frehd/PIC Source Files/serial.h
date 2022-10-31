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

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "stdint.h"


void usart_init(void);
uint8_t usart_ok_to_send(void);
void usart_send(uint8_t data);
uint8_t usart_get(uint8_t *data);
void usart_flush(void);

/* debug utils */
void usart_puts(const rom char *str);
void usart_puts2(const rom char *str);
void usart_puts_r(const char *str);
void usart_puts2_r(const char *str);
void usart_put_hex(char c);
void usart_put_short(short c);
void usart_put_long(long c);

/* interrupt handlers */
void serial_rx_int(void);
void serial_tx_int(void);


#endif /* _SERIAL_H_ */
