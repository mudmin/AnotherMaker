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
#include "eeprom.h"

/*
 * read one byte from EEPROM
 */
uint8_t ee_read8(uint16_t address)
{
	EEADR = address & 0xff;
	EEADRH = address >> 8;
	EECON1bits.EEPGD = 0;           // data memory
	EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;

	return (EEDATA);
}


/*
 * write one byte into EEPROM
 */
void ee_write8(uint16_t address, uint8_t value)
{
	EEADR = address & 0xff;
	EEADRH = address >> 8;
	EEDATA = value;
	EECON1bits.CFGS = 0;
	EECON1bits.EEPGD = 0;           // data memory
	EECON1bits.WREN = 1;
	INTCONbits.GIE = 0;
	_asm
		MOVLW   0x55
		MOVWF   EECON2, 0
		MOVLW   0xAA
		MOVWF   EECON2, 0
	_endasm
	EECON1bits.WR = 1;
	
	INTCONbits.GIE = 1;
	
	while (EECON1bits.WR == 1);		// wait until write cycle is done
	
	EECON1bits.WREN = 0;
}

