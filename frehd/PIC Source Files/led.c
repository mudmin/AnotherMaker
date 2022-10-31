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
#include "led.h"


void led_init(void)
{
	rled.rled = 1;
	rled.flash = 0;
	rled.val = LED_OFF;
	gled.rled = 0;
	gled.flash = 0;
	gled.val = LED_OFF;
}	 
		
 
void led_update(led_t *led)
{
	static unsigned char val;

	switch (led->val & LED_VAL_MASK) {
	case LED_OFF:
		val = 0;
		break;
	case LED_ON:		
		val = 1;
		break;
	case LED_FAST_BLINK:
		val = led_count & 0x40;
		break;
	case LED_SLOW_BLINK:
		val = led_count & 0x80;
		break;
	}
	
	// flash LED for disk activity
	if (led->val & LED_FLASH) {
		led->val &= ~LED_FLASH;
		led->flash = 7+1;			// 20ms
	}
	if (led->flash) {
		led->flash--;
		val ^= 1;
	}
	
	// update hardware
	if (led->rled) {
		RLED = val;
	} else {
		GLED = val;
	}
}

