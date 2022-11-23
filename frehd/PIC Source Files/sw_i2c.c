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

#include <delays.h>
#include "HardwareProfile.h"

#define DELAY			10

/* macros */
#define SDA_LOW         SDA_TRIS = 0;
#define SDA_HI          SDA_TRIS = 1;
#define SCL_LOW         SCL_TRIS = 0;
#define SCL_HI          SCL_TRIS = 1;

/* globals */
unsigned int i2c_buffer;
char bit_counter;


void i2c_init(void)
{
	SDA_HI;
	SCL_HI;
}


void i2c_start(void)
{
	SDA_LAT = 0;
    SDA_LOW;
	Delay10TCYx(DELAY);
}


void i2c_stop(void)
{
	SCL_LAT = 0;
	SCL_LOW;
	SDA_LAT = 0;
	SDA_LOW;
	Delay10TCYx(DELAY);
	SCL_HI;
	Delay10TCYx(DELAY);
	SDA_HI;
	Nop();
	Nop();
}


void i2c_restart(void)
{
	SCL_LAT = 0;
	SCL_LOW;
	SDA_HI;
	Delay10TCYx(DELAY);
	SCL_HI;
	Delay10TCYx(DELAY);
	SDA_LAT = 0;
	SDA_LOW;
	Delay10TCYx(DELAY);
}


signed char i2c_ack(void)
{
	SCL_LAT = 0;
	SCL_LOW;
	SDA_HI;
	Delay10TCYx(DELAY);
	SCL_HI;
	Delay10TCYx(5 * DELAY);
	
	if (SDA_PIN) {
		return (-1);    // slave didn't ack
	} else {
		return (0);
	}
}

signed char i2c_clock_test(void)
{
	Delay10TCYx(3 * DELAY);	// wait period
	if (!SCL_PIN) {
		return (-2);	// clock still low after wait
	} else {
		return (0);
	}
}

unsigned int i2c_read(void)
{
	bit_counter = 8;
	SCL_LAT = 0;
	do {
		SCL_LOW;
		SDA_HI;
		Delay10TCYx(DELAY);
		SCL_HI;
		Delay10TCY();

		if (!SCL_PIN) {
			if (i2c_clock_test()) {
				return (-2);
			}
		}

		i2c_buffer <<= 1;
		i2c_buffer &= 0xFE;

		if (SDA_PIN) {
			i2c_buffer |= 0x01;
		}

	} while (--bit_counter);

	return ((unsigned int) i2c_buffer);
}

unsigned int i2c_getc(void)
{
	unsigned int x;

	x = i2c_read();
	if (x & 0xFF00) {
		return (-1);
	}
	if (!i2c_ack()) {
		return (-1);
	}
	return (x);
}

signed char i2c_gets(unsigned char *ptr, unsigned char length)
{
	unsigned int thold;

	while (length--) {
		thold = i2c_read();
		if (thold & 0xFF00) {
			return (-1);
		} else {
			*ptr++ = thold;
		}

		if (!length) {
			SCL_LOW;
			SDA_HI;
			Delay10TCYx(DELAY);
			SCL_HI;
			Delay10TCYx(DELAY);
		} else {
			SCL_LOW;
			SDA_LAT = 0;
			SDA_LOW;
			Delay10TCYx(DELAY);
			SCL_HI;
			Delay10TCYx(DELAY);
		}
	}

	return (0);
}


signed char i2c_write(unsigned char data)
{
	bit_counter = 8;
	i2c_buffer = data;
	SCL_LAT = 0;

	do {
		if (!SCL_PIN) {
			if (i2c_clock_test()) {
				return (-1);
			}
		} else {
			i2c_buffer &= 0xFF;
			_asm
			rlcf i2c_buffer,1,1
			_endasm

			if (STATUS & 0x01) {
				SCL_LOW;
				SDA_HI;
				Delay10TCYx(DELAY);
				SCL_HI;
				Delay10TCYx(DELAY);
			} else {
				SCL_LOW;
				SDA_LAT = 0;
				SDA_LOW;
				Delay10TCYx(DELAY);
				SCL_HI;
				Delay10TCYx(DELAY);
			}

			bit_counter--;
		}
	} while (bit_counter);

	return (0);
}

signed char i2c_putc(unsigned char data)
{
	if (i2c_write(data)) {
		return (-1);
	}
	if (i2c_ack()) {
		return (-1);
	}
	return (0);
}

signed char i2c_puts(unsigned char *ptr, unsigned char length)
{
	while (length) {
		if (i2c_write(*ptr++)) {
			return (-1);
		} else {
			if (i2c_ack()) {
				return (-1);
			}
		}
		length--;
	}

	return (0);
}
