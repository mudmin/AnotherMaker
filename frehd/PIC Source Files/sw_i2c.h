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

#ifndef _SW_I2C_H
#define _SW_I2C_H

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_restart(void);
signed char i2c_ack(void);
signed char i2c_clock_test(void);
unsigned int i2c_read(void);
unsigned int i2c_getc(void);
signed char i2c_gets(unsigned char *ptr, unsigned char length);
signed char i2c_write(unsigned char);
signed char i2c_putc(unsigned char);
signed char i2c_puts(unsigned char *ptr, unsigned char length);

#endif
