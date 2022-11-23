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

#ifndef _LED_H
#define _LED_H


#define LED_OFF			0x0
#define LED_ON			0x1
#define LED_FAST_BLINK	0x2
#define LED_SLOW_BLINK	0x3
#define LED_VAL_MASK	0x03

#define LED_FLASH		0x80

typedef struct {
	unsigned char val;
	unsigned char flash;
	unsigned char rled;			// true for red led
} led_t;


extern led_t rled;
extern led_t gled;
extern unsigned char led_count;

void led_init(void);
void led_update(led_t *led);


#endif
