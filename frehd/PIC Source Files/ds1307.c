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

#include "sw_i2c.h"
#include "HardwareProfile.h"
#include "action.h"
#include "ds1307.h"

/* globals */
BYTE time[10];
BYTE tbuf[10];


/*
 * un-bcd : 0x00 - 0x99 --> 0 - 99
 */
char unbcd(unsigned char x)
{
	char res = 0;
	char i;

	i = (x & 0xF0) >> 4;
	while (i > 0) {
		res += 10;
		i--;
	}
	res += (x & 0x0F);

	return (res);
}


/*
 * bcd : 0 - 99 -->  0x00 - 0x99
 */
unsigned char bcd(char x)
{
	unsigned char res = 0;

	while (x >= 10) {
		x -= 10;
		res++;
	}
	res <<= 4;
	res |= x;

	return (res);
}


#if 0
static int ds1307_read(unsigned char address)
{
	unsigned int value;

	i2c_start();
	if (i2c_putc(DS1307_SLA)) {
		return (-1);
	}
	if (i2c_putc(address)) {
		return (-1);
	}
	i2c_restart();
	if (i2c_putc(DS1307_SLA | 0x1)) {
		return (-1);
	}
	value = i2c_getc();
	i2c_stop();

	return (value);
}
#endif

static char ds1307_write(unsigned char address, unsigned char data)
{
	i2c_start();
	if (i2c_putc(DS1307_SLA)) {
		return (-1);
	}
	if (i2c_putc(address)) {
		return (-1);
	}
	if (i2c_putc(data)) {
		return (-1);
	}
	i2c_stop();

	return (0);
}


char ds1307_read_time(void)
{
	char i;

	i = 1;
	i2c_start();
	if (i2c_putc(DS1307_SLA) == 0) {
		if (i2c_putc(DS1307_SEC) == 0) {
			i2c_restart();
			if (i2c_putc(DS1307_SLA | 0x1) == 0) {
				if (i2c_gets(tbuf, 10) == 0) {
					// all ok
					i = 0;
				}
			}
		}
	}
	i2c_stop();

	if (i == 0) {
		INTCONbits.GIEL = 0;
		for (i = 0; i <= DS1307_YEAR; i++) {
			time[i] = unbcd(tbuf[i]);
		}
		for (; i < 10; i++) {
			time[i] = tbuf[i];
		}
		INTCONbits.GIEL = 1;
	}

	return 0;
}


char ds1307_write_time(BYTE *time_bin)
{
	char i;
	char res;

	for (i = 0; i <= DS1307_YEAR; i++) {
		tbuf[i] = bcd(time_bin[i]);
	}
	tbuf[DS1307_CONTROL] = DS1307_CONFIG;
	tbuf[DS1307_MAGIC0] = DS1307_MAGIC0_VAL;
	tbuf[DS1307_MAGIC1] = DS1307_MAGIC1_VAL;

	res = 1;
	i2c_start();
	if (i2c_putc(DS1307_SLA) == 0) {
		if (i2c_putc(DS1307_SEC) == 0) {
			if (i2c_puts(tbuf, 10) == 0) {
				res = 0;
			}
		}
	}
	i2c_stop();

	return res;
}



void ds1307_init(void)
{
	ds1307_write(DS1307_CONTROL, DS1307_CONFIG);
	ds1307_read_time();
	if (time[DS1307_MAGIC0] != DS1307_MAGIC0_VAL || 
      time[DS1307_MAGIC1] != DS1307_MAGIC1_VAL) {
		// bad magic. Initialize time
		tbuf[DS1307_SEC] = 0;
		tbuf[DS1307_MIN] = 0;
		tbuf[DS1307_HOUR] = 0;
		tbuf[DS1307_WEEKDAY] = 1;	// ignored
		tbuf[DS1307_DAY] = 1;
		tbuf[DS1307_MONTH] = 4;
		tbuf[DS1307_YEAR] = 12;
		ds1307_write_time(tbuf);
		ds1307_read_time();
	}
}


// DS1307 interrupt : 1Hz
void ds1307_int(void)
{
	if (++time[DS1307_SEC] >= 60) {
		time[DS1307_SEC] = 0;
		if (++time[DS1307_MIN] >= 60) {
			time[DS1307_MIN] = 0;
			if (++time[DS1307_HOUR] >= 24) {
				time[DS1307_HOUR] = 0;
				/* A day has elapsed. Reload the date/time from
				   the DS1307. This takes care of incrementing the
				   date properly. */
				action_flags |= ACTION_DS1307_RELOAD;
			}
		}
	}

	// LED1 = LED1 ^ 1;
}


/*
 * Provide time to FatFS
 *
 * bit31:25		Year from 1980 (0..127)
 * bit24:21		Month (1..12)
 * bit20:16		Day in month(1..31)
 * bit15:11		Hour (0..23)
 * bit10:5		Minute (0..59)
 * bit4:0		Second / 2 (0..29)
 */
DWORD get_fattime(void)
{
	static DWORD tmr;

	INTCONbits.GIEL = 0;

	tmr = (((DWORD)time[DS1307_YEAR] + 20) << 25)
		| ((DWORD)time[DS1307_MONTH] << 21)
		| ((DWORD)time[DS1307_DAY] << 16)
		| ((WORD)time[DS1307_HOUR] << 11)
		| ((WORD)time[DS1307_MIN] << 5)
		| ((WORD)time[DS1307_SEC] >> 1);

	INTCONbits.GIEL = 1;

	return tmr;
}
