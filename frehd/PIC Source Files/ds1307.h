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

#ifndef _DS1307_H
#define _DS1307_H

#define DS1307_SLA		    0b11010000
#define	DS1307_CONFIG	    0x10	// clock output enable, 1Hz
#define	DS1307_SEC          0
#define	DS1307_MIN		    1
#define	DS1307_HOUR		    2
#define	DS1307_WEEKDAY	    3
#define	DS1307_DAY		    4
#define	DS1307_MONTH	    5
#define	DS1307_YEAR		    6
#define	DS1307_CONTROL		7
#define DS1307_MAGIC0		8
#define DS1307_MAGIC1		9
#define DS1307_MAGIC0_VAL	0x15
#define DS1307_MAGIC1_VAL	0x04

extern BYTE time[10];
extern BYTE tbuf[10];

void ds1307_init(void);
void ds1307_int(void);
char ds1307_read_time(void);
char ds1307_write_time(BYTE *);

#endif
