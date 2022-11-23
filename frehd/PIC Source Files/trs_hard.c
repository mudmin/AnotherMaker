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

/* Copyright (c) 2000, Timothy Mann */
/* $Id: trs_hard.c,v 1.8 2009/06/15 23:40:47 mann Exp $ */

/* This software may be copied, modified, and used for any purpose
 * without fee, provided that (1) the above copyright notice is
 * retained, and (2) modified versions are clearly marked as having
 * been modified, with the modifier's name and the date included.  */

#include <string.h>
#include "HardwareProfile.h"
#include "action.h"
#include "reed.h"
#include "trs_hard.h"
#include "trs_extra.h"
#include "ds1307.h"
#include "led.h"
#include "version.h"

extern void handle_int2(void);

/* update status (using the SPI port) */
void update_status(UCHAR new_status) {
	SSPBUF = new_status;
	state_status = new_status;
	while (!SSPSTATbits.BF);
	STAT_CS = 1;
	foo = SSPBUF;
	STAT_CS = 0;
}


static void update_present(void)
{
	UCHAR i, p;
	
	p = 0;
	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		if (state_d[i].avail) {
			p = 1;
		}
	}
	state_present = p;
}


FRESULT write_header(FIL *file)
{
	ReedHardHeader *rhh;
	FRESULT res;
	UINT nbytes;

	rhh = (ReedHardHeader *)&sector_buffer[0];
	memset(rhh, 0, sizeof (ReedHardHeader));
	
	rhh->id1 = 0x56;
	rhh->id2 = 0xcb;
	rhh->ver = 0x10;
	rhh->blks = 1;
	rhh->mb4 = 1;
	rhh->crtr = 0x43;			// created by
	INTCONbits.GIEL = 0;
	rhh->mm = time[DS1307_MONTH];
	rhh->dd = time[DS1307_DAY];
	rhh->yy = 100 + time[DS1307_YEAR];
	INTCONbits.GIEL = 1;

	/*
     * Default configuration is ST251 equivilent 
     * 840 cylinders / 6 heads / 192 (6 x 32) sectors per cylinder
     */
	rhh->dparm = 3;			// Cylinders MSB
	rhh->cyl = 72;			// Cylinders LSB
	rhh->sec = 192;
	rhh->gran = 8;
	rhh->dcyl = 1;
	rhh->heads = 6;
	strcpypgm2ram(rhh->label, (const rom char far *)"etrshard");
	
	res = f_write(file, (const void *)sector_buffer, 256, &nbytes);
	if (res == FR_OK) {
		res = f_sync(file);
		if (res == FR_OK) {
			res = f_lseek(file, 0);
		}
	}		

	return FR_OK;	
}	

	
FRESULT open_drive(UCHAR drive_num, UCHAR options)
{
	CHAR *filename;
	FRESULT res;
	UINT nbytes;
	USHORT secs;
	BYTE mode;
	ReedHardHeader *rhh;
	Drive *d;
	
	d = &state_d[drive_num];
	if (d->avail) {
		// drive already open
		return (FR_OK);
	}

	filename = state_d[drive_num].filename;
	if (filename[0] == '\0') {
		// use default name
		strcpypgm2ram(filename, (const rom char far *)"/hard4-");
		filename[7] = '0' + drive_num;
		filename[8] = 0;
	}
	
	/* try to open the file */
	mode = FA_OPEN_EXISTING | FA_READ | FA_WRITE;
	if (options & TRS_EXTRA_MOUNT_CREATE) {
		mode = FA_CREATE_NEW | FA_READ | FA_WRITE;
	} else if (options & TRS_EXTRA_MOUNT_RO) {
		mode = FA_OPEN_EXISTING | FA_READ;
	}	
	res = f_open(&d->file, filename, mode);
	if (res != FR_OK) return res;

	/* initialize fast seek */
#if _USE_FASTSEEK
	if ((options & (TRS_EXTRA_MOUNT_SLOW | TRS_EXTRA_MOUNT_CREATE)) == 0) {
		d->file.cltbl = d->stbl;
		d->stbl[0] = FAST_SEEK_LEN;
		res = f_lseek(&d->file, CREATE_LINKMAP);
		if (res == FR_OK) {
			d->avail |= 0x2;
		}
	}
#endif
	/* if the file has been created, write a header */
	if (options & TRS_EXTRA_MOUNT_CREATE) {
		res = write_header(&d->file);
		if (res != FR_OK) {
			goto fail;
		}	
	}	

	/* read the header */
	res = f_read(&d->file, &sector_buffer[0], 256, &nbytes);
	if (res != FR_OK) {
		goto fail;
	}
	if (nbytes != 256) {
		res = FR_INVALID_OBJECT;
		goto fail;
	}
	rhh = (ReedHardHeader *)&sector_buffer[0];

	/* check magic numbers */
	/* rrh.ver = 0x10 = XTRS/Reed VHD1, 0x11 = Keil */
	if (rhh->id1 != 0x56 || rhh->id2 != 0xcb || !((rhh->ver == 0x10) || (rhh->ver == 0x11))) {
		res = FR_INVALID_OBJECT;
		goto fail;
	}

	/* write-protect bit */
    if (options & TRS_EXTRA_MOUNT_RO) {
        d->writeprot = 1;
    } else {
        d->writeprot = (rhh->flag1 & 0x80) ? 1 : 0;
    }

	/* use the number of cylinders specified in the header */
	d->cyls = ((USHORT)(rhh->dparm & 0x3)) << 8;
	d->cyls += rhh->cyl;

	secs = rhh->sec ? rhh->sec : 256;
	if (rhh->heads == 0)
	{
		/* header gives only secs/cyl. Compute number of heads from this and
		   the assumed number of secs/track */
		
		/* use the number of secs/track that RSHARD requires */
		d->secs = TRS_HARD_SEC_PER_TRK;
		d->heads = secs / d->secs;
	} else {
		/* header includes head count */
		
		d->heads = rhh->heads;
		d->secs = secs / d->heads;
	}
	if ((rhh->sec % d->secs) != 0 || d->heads > TRS_HARD_MAXHEADS) {
		res = FR_INVALID_OBJECT;
		goto fail;
	}

	d->dirty = 0;
	d->avail |= 1;

	return FR_OK;

fail:
	f_close(&d->file);
	return res;
}


FRESULT open_drives(void)
{
	UCHAR i;

	state_wp = 0;
	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		if (open_drive(i, 0) == FR_OK) {
			state_present = 1;
			if (state_d[i].writeprot) {
				state_wp = TRS_HARD_WPBIT(i) | TRS_HARD_WPSOME;
			}
		}
	}
	update_present();

	return state_present ? FR_OK : FR_DISK_ERR;
}


void close_drive(UCHAR drive_num)
{
	UCHAR i, p;
	
	// close drive
	if (state_d[drive_num].avail) {
		f_close(&state_d[drive_num].file);
		state_d[drive_num].avail = 0;
		state_d[drive_num].filename[0] = '\0';
	}
	update_present();		
}	


void close_drives(void)
{
	UCHAR i;

	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		close_drive(i);
	}
}


static FRESULT find_sector(void)
{
	Drive *d;
	DWORD offset;
	FRESULT res;

	d = &state_d[state_drive];
	if (state_head >= d->heads || state_secnum > d->secs) {
		return FR_INVALID_PARAMETER;
	}
	offset = (DWORD)state_cyl * d->heads * d->secs;
	offset += (DWORD)state_head * d->secs;
	offset += (DWORD)state_secnum % d->secs;
	offset *= (DWORD)state_secsize16;
	offset += (DWORD)sizeof(ReedHardHeader);
	res = f_lseek(&d->file, offset);
	if (res != FR_OK) {
		return res;
	}

	return (FR_OK);
}


static FRESULT read_sector(void)
{
	Drive *d;
	UINT nbytes;
	FRESULT res;

	d = &state_d[state_drive];
	res = f_read(&d->file, sector_buffer, state_secsize16, &nbytes);
	if (res == FR_OK && d->dirty) {
		// reset sync delay
		d->dirty = SYNC_DELAY;
	}	

	return res;
}


static FRESULT write_sector(void)
{
	Drive *d;
	UINT nbytes;
	FRESULT res;

	d = &state_d[state_drive];
	res = f_write(&d->file, (const void *)sector_buffer, state_secsize16, &nbytes);
	if (res == FR_OK) {
		d->dirty = SYNC_DELAY;
		rled.val = LED_ON;
	}

	return res;
}


void trs_sync(void)
{
	UCHAR i, clean, found;
	Drive *d;

	clean = 1;
	found = 0;
	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		d = &state_d[i];
		if (d->avail) {
			found = 1;
			if (d->dirty) {
				d->dirty--;
				if (d->dirty == 0) {
					INTCONbits.GIEH = 0;
					f_sync(&d->file);
					if (GAL_INT == 0) {
						_asm
						call handle_int2,1
						_endasm
					}	
					INTCONbits.GIEH = 1;
				} else {
					clean = 0;
				}
			}
		}
	}
	if (found && clean) rled.val = LED_OFF;
}


void trs_hard_init(void)
{
	UCHAR i;

	for (i = 0; i < TRS_HARD_MAXDRIVES; i++) {
		memset(&state_d[i], 0, sizeof (Drive));
	}
	update_status(TRS_HARD_READY);
}


/*
 * Actions are executed from the main loop.
 */
void trs_action(void)
{
	switch (action_type) {

	case ACTION_HARD_SEEK:
		if (find_sector() != FR_OK) {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR;
    		state_error = TRS_HARD_NFERR;
		} else {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE;
		}
		break;

	case ACTION_HARD_READ:
		action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_DRQ;
		if (find_sector() != FR_OK) {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR;
    		state_error = TRS_HARD_NFERR;
		} else if (read_sector() != FR_OK) {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR;
    		state_error = TRS_HARD_DATAERR;
		}
		break;

	case ACTION_HARD_WRITE:
		action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE;
		if (find_sector() != FR_OK) {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR;
    		state_error = TRS_HARD_NFERR;
		} else if (write_sector() != FR_OK) {
			action_status = TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR;
    		state_error = TRS_HARD_DATAERR;
		}
		break;

	default:
		if (action_type & ACTION_EXTRA2) {
			action_status = (*trs_extra[action_type & 0xF])(1);
		} else if (action_type & ACTION_EXTRA) {
			action_status = (*trs_extra[action_type & 0xF])(0);
		}
	}

	// update status
	update_status(action_status);
}
