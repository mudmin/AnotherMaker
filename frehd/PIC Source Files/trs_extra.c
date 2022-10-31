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

#include <string.h>
#include "HardwareProfile.h"
#include "action.h"
#include "trs_hard.h"
#include "trs_extra.h"
#include "ds1307.h"
#include "eeprom.h"
#include "bootloader.inc"
#include "version.h"

#define DEFAULT_STATUS (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_DRQ)

rom UCHAR (*rom trs_extra[])(UCHAR) = {
	trs_extra_version,		// 0
	trs_extra_gettime,		// 1
	trs_extra_settime,		// 2
	trs_extra_openfile,		// 3
	trs_extra_readfile,		// 4
	trs_extra_writefile,	// 5
	trs_extra_closefile,	// 6
	trs_extra_bootloader,	// 7
	trs_extra_opendir,		// 8
	trs_extra_readdir,		// 9
	trs_extra_mountdrive,	// A
	trs_extra_seekfile,		// B
	trs_extra_infodrive,	// C
	trs_extra_image,		// D
	trs_extra_read_header,	// E
	trs_extra_ignored,		// F
};	
	

UCHAR trs_extra_ignored(UCHAR step)
{
	return DEFAULT_STATUS;
}	


/*
 * Get version
 * 1. TRS writes COMMAND2 + wait
 * 2. TRS reads DATA2 (up to 6 bytes)
 */
UCHAR trs_extra_version(UCHAR step)
{
	state_size2 = 6;
	extra_buffer[0] = VERSION_MAJOR;
	extra_buffer[1] = VERSION_MINOR;
	extra_buffer[2] = *((UCHAR rom *) 0x6);
	extra_buffer[3] = *((UCHAR rom *) 0x7);
	extra_buffer[4] = *((UCHAR rom *) FLASH_CRC_ADDR);
	extra_buffer[5] = *((UCHAR rom *) FLASH_CRC_ADDR+1);	
	
	return DEFAULT_STATUS;
}


/*
 * Get time
 * 1. TRS writes COMMAND2 + wait
 * 2. TRS reads DATA2 (6 bytes)
 */
UCHAR trs_extra_gettime(UCHAR step)
{
	state_size2 = 6;
	INTCONbits.GIEL = 0;
    extra_buffer[TRS80_SEC] = time[DS1307_SEC];
    extra_buffer[TRS80_MIN] = time[DS1307_MIN];
    extra_buffer[TRS80_HOUR] = time[DS1307_HOUR];
    extra_buffer[TRS80_YEAR] = time[DS1307_YEAR];
    extra_buffer[TRS80_DAY] = time[DS1307_DAY];
    extra_buffer[TRS80_MONTH] = time[DS1307_MONTH];
	INTCONbits.GIEL = 1;
		
	return DEFAULT_STATUS;	
}


/*
 * Set time
 * 1. TRS writes COMMAND2 + wait
 * 2. TRS writes DATA2 (6 bytes) + wait
 */
UCHAR trs_extra_settime(UCHAR step)
{
	if (step == 0) {
		state_size2 = 6;
	} else {
        tbuf[DS1307_SEC] = extra_buffer[TRS80_SEC];
        tbuf[DS1307_MIN] = extra_buffer[TRS80_MIN];
        tbuf[DS1307_HOUR] = extra_buffer[TRS80_HOUR];
        tbuf[DS1307_YEAR] = extra_buffer[TRS80_YEAR];
        tbuf[DS1307_DAY] = extra_buffer[TRS80_DAY];
        tbuf[DS1307_MONTH] = extra_buffer[TRS80_MONTH];
        tbuf[DS1307_WEEKDAY] = 1; // don't care
		ds1307_write_time(tbuf);
		action_flags |= ACTION_DS1307_RELOAD;
	}
	
	return DEFAULT_STATUS;
}


/*
 * Open file
 * 1. TRS writes SIZE2 (1 + strlen(filename))
 * 2. TRS wrties COMMAND2 + wait
 * 3. TRS writes DATA2 
 *       byte 0 : open options
 *       bytes 1..n : filename
 * 4. TRS waits
 * 5. TRS checks status for error (in ERROR2 if any)
 */
UCHAR trs_extra_openfile(UCHAR step)
{
	if (step == 1) {	
		if (state_file2_open) {
			f_close(&state_file2);
			state_file2_open = 0;
		}
		state_error2 = f_open(&state_file2, (const TCHAR *)&extra_buffer[1],
          extra_buffer[0]);
		if (state_error2 != FR_OK) {
			state_size2 = 0;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		} else {
			state_file2_open = 1;
			// provide file size to the TRS80
			state_size2 = sizeof (DWORD);
			memcpy(extra_buffer, (const void *)&state_file2.fsize,
              sizeof (DWORD));
		}
	}
	
	return DEFAULT_STATUS;
}		


/*
 * Read file
 * 1. file must have been opened by openfile()
 * 2. TRS sends COMMAND2 + wait
 * 3. TRS checks status. If DRQ set, data was read
 *      DRQ : TRS reads DATA2 (SIZE2 bytes)
 *     !DRQ : done
 *      ERR : error in ERROR2
 */
UCHAR trs_extra_readfile(UCHAR step)
{
	UINT n;
	
	if (state_file2_open) {
		state_error2 = f_read(&state_file2, extra_buffer, state_size2, &n);
		if (state_error2 != FR_OK) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
		state_size2 = n;
		if (n == 0) {
			// got no data, don't set DRQ
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE);
		}
	} else {
		state_error2 = FR_NO_FILE;
		return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
	}	
	
	return DEFAULT_STATUS;
}		
	

/*
 * Write file
 * 1. file must have been opened by openfile()
 * 2. TRS sends COMMAND2 + wait
 * 3. TRS sends SIZE2 (if not, default is 256 bytes)
 * 4. TRS waits (after sending size2 bytes)
 * 5. TRS checks status
 */
UCHAR trs_extra_writefile(UCHAR step)
{
	UINT n;
	
	if (step == 0) {
		if (state_file2_open) {
			state_size2 = 256;
		} else {
			state_error2 = FR_NO_FILE;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}	
	} else {	
		state_error2 = f_write(&state_file2, (const void *)extra_buffer,
          state_size2, &n);
		if (state_error2 != FR_OK) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
		if (n < state_size2) {
			// disk full
			state_error2 = FR_DISK_FULL;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
	}	
	
	return DEFAULT_STATUS;	
}	


/*
 * Seek file
 * 1. file must have been opened by openfile()
 * 2. TRS sends COMMAND2 + wait
 * 3. TRS writes DATA2 (position (DWORD)) + wait
 * 4. TRS checks status.
 *
 * Tell position
 * 1. file must have been opened by openfile()
 * 2. TRS sends COMMAND2 + wait
 * 3. TRS reads DATA2 (DWORD). It contains the current position (ftell)
 */
 UCHAR trs_extra_seekfile(UCHAR step)
 {
	DWORD offset;
	
	if (step == 0) {
		if (state_file2_open) {
			state_size2 = sizeof (DWORD);
			offset = f_tell(&state_file2);
			memcpy(extra_buffer, (const void *)&offset, sizeof (DWORD));
		} else {
			state_error2 = FR_NO_FILE;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
	} else {
		memcpy(&offset, (const void *)extra_buffer, sizeof (DWORD));
		// XXX: should we check offset here and prevent too big offsets ?
		state_error2 = f_lseek(&state_file2, offset);
		if (state_error2 != FR_OK) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);	
		}	
	}
	
	return DEFAULT_STATUS;
 }


/*
 * Close file
 * 1. file must have been opened by openfile()
 * 2. TRS sends COMMAND2 + wait
 * 3. TRS checks status for ERR.
 */
UCHAR trs_extra_closefile(UCHAR step)
{
	if (state_file2_open) {
		state_error2 = f_close(&state_file2);
		if (state_error2 != FR_OK) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
		state_file2_open = 0;
	}
	
	return (TRS_HARD_READY | TRS_HARD_SEEKDONE);
}	


/*
 * Enter bootloader
 * 1. TRS issues COMMAND2 + wait
 * 2. TRS writes DATA2 0x15 0x04 + wait
 * 3. emulator should be in bootloader mode
 *    (can be checked by reading 4F from emulator, see bootloader.asm)
 */
UCHAR trs_extra_bootloader(UCHAR step)
{
	if (step == 0) {
		state_size2 = 2;	
	} else {
		if (extra_buffer[0] == 0x15 && extra_buffer[1] == 0x04) {
			ee_write8(EEPROM_CRC_ADDR, 0xff);
			ee_write8(EEPROM_CRC_ADDR+1, 0xff);
			_asm
				RESET
			_endasm
			// not reached
		}
	}	
	
	return DEFAULT_STATUS;
}	


/*
 * Open a directory
 * 1. TRS issues SIZE2 = strlen(path), and COMMAND2 + wait
 * 2. TRS sends DATA2 path
 * 3. TRS waits
 * 4. TRS reads status. If TRS_HARD_ERR set, check ERROR2.
 */
UCHAR trs_extra_opendir(UCHAR step)
{
	if (step == 1) {
		state_error2 = f_opendir(&state_dir, (const TCHAR *)&extra_buffer[0]);
		if (state_error2 != FR_OK) {
			state_size2 = 0;
			state_dir_open = 0;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		} else {
			state_dir_open = 1;
		}		
	}
		
	return DEFAULT_STATUS;
}


/*
 * Read a directory
 * 1. opendir (see above)
 * 2. TRS issues COMMAND2 + wait
 * 3. TRS reads status. If HARD_ERR, check ERROR2
 *    If ok, check DRQ. 
 *           If set: SIZE2 contains directory info size.
 *                   DATA2 contains directory info data.
 *           If clear: no more entries in this directory.
 */
UCHAR trs_extra_readdir(UCHAR step)
{
	if (state_dir_open == 0) {
		// user has not done opendir() before.
		state_error2 = FR_NO_PATH;
		return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
	}
	state_error2 = f_readdir(&state_dir, &state_fno);
	if (state_error2 != FR_OK) {
		return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
	}
	if (state_fno.fname[0] == 0) {
		// reach end of dir : do not set DRQ
		return (TRS_HARD_READY | TRS_HARD_SEEKDONE);
	}
	state_size2 = sizeof (state_fno);
	memcpy(extra_buffer, (const void *)&state_fno, state_size2);	

	return DEFAULT_STATUS;
}
	

/*
 * Mount a drive
 * 1. TRS issues SIZE2 = 2 + strlen(filename), and COMMAND2 + wait
 * 2. TRS sends DATA2
 *    byte 0         : drive number (0..TRS_HARD_MAXDRIVES) 
 *    byte 1         : option bit 0 set = no fast seek)
 *                            bit 1 set = create if file doesn't exist
 *                            bit 2 set = mount read-only 
 *    bytes 2..size2 : filename ('\0' means default filename)
 * 3. TRS waits
 * 4. TRS checks status
 */
UCHAR trs_extra_mountdrive(UCHAR step)
{
	UCHAR drive_num;
	
	if (step == 1) {
		if (state_size2 < 3 || 
          (drive_num = extra_buffer[0]) >= TRS_HARD_MAXDRIVES) {
			state_error2 = FR_INVALID_PARAMETER;
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
		close_drive(drive_num);	
		strcpy(state_d[drive_num].filename, (const char *)&extra_buffer[2]);
		state_error2 = open_drive(extra_buffer[0], extra_buffer[1]);
		if (state_error2 != FR_OK) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}	
	}
	
	return DEFAULT_STATUS;	
}	


/*
 * Info about mounted drives
 * 1. TRS sends command and wait
 * 2. TRS reads size2 and DATA2
 *
 * Data returned, per drive:
 * offset 0 : drive available (bit0 = avail, bit1 = use fast seek)
 *        1 : write protect flag
 *      2-3 : number of cylinders
 *        4 : number of heads
 *        5 : number of sectors per track
 *     6-18 : filename (8.3 format)
 */
UCHAR trs_extra_infodrive(UCHAR step)
{
	UCHAR drive_num;
	Drive *d;
	
	if (step == 0) {
		state_size2 = 0;
		for (drive_num = 0; drive_num < TRS_HARD_MAXDRIVES; drive_num++) {
			d = &state_d[drive_num];
			extra_buffer[state_size2++] = d->avail;			// 0
			extra_buffer[state_size2++] = d->writeprot;		// 1
			extra_buffer[state_size2++] = d->cyls >> 8;		// 2
			extra_buffer[state_size2++] = d->cyls & 0xff;	// 3
			extra_buffer[state_size2++] = d->heads;			// 4
			extra_buffer[state_size2++] = d->secs;			// 5
			memcpy(&extra_buffer[state_size2], (const char *)d->filename, 13);
			state_size2 += 13;
		}	
	}	
	
	return DEFAULT_STATUS;
}
	

/*
 * JV1, JV3 and DMK image support (read-only)
 * 1. TRS sends command and waits
 * 2. TRS reads size2 and DATA2 (DATA2 contains subcommand)
 * 3. TRS waits
 * 4. TRS gets error status in ERROR2, and acts accordingly
 * 5. (optional if no error)  TRS gets DATA2
 */
UCHAR trs_extra_image(UCHAR step)
{
#if EXTRA_IM_SUPPORT
	if (step == 1) {
		process_image_cmd();
	}
	
	return DEFAULT_STATUS;		
#else
	state_error2 = FR_NOT_ENABLED;
	return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
#endif
}	


/*
 * Read hard drive image header of the currently select drive
 * (use SDH to select a drive)
 * 1. TRS sends command and wait
 * 2. TRS reads size2 and DATA2 (256 bytes)
 */
UCHAR trs_extra_read_header(UCHAR step)
{
	Drive *d;
	UINT n;
	
	if (step == 0) {
		d = &state_d[state_drive];
		if (d->avail == 0) {
			return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
		}
		state_error2 = f_lseek(&d->file, 0);
		if (state_error2 == FR_OK) {
			state_error2 = f_read(&d->file, extra_buffer, 256, &n);
			if (state_error2 != FR_OK) {
				return (TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR);
			}
			state_size2 = n;
		}		
	}
	
	return DEFAULT_STATUS;
}	
