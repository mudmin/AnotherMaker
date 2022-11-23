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
#include <stdio.h>
#include <string.h>
#include <timers.h>
#include "FatFS/ff.h"
#include "FatFS/diskio.h"
#include "trs_hard.h"
#include "trs_extra.h"

// subcommands
#define IM_OPEN		   0
#define IM_READSEC	   1

// DMK
#define DMK_HEADER_LEN	16

// JV3
#define JV3_DENSITY		0x80	// 1=dden 0=sden
#define JV3_DAM			0x60
#define JV3_DAMSDFB		0x00
#define JV3_DAMSDFA		0x20
#define JV3_DAMSDF9		0x40
#define JV3_DAMSDF8		0x60
#define JV3_DAMDDFB		0x00
#define JV3_DAMDDF8		0x20
#define JV3_SIDE		0x10  /* 0=side 0, 1=side 1 */
#define JV3_ERROR		0x08  /* 0=ok, 1=CRC error */
#define JV3_SIZE		0x03  /* in used sectors: 0=256,1=128,2=1024,3=512
								 in free sectors: 0=512,1=1024,2=128,3=256 */
#define JV3_SECSTART   (34*256) /* start of sectors within file */
#define JV3_SECSPERBLK ((USHORT)(JV3_SECSTART/3))
#define JV3_FREE		0xff  /* in track/sector fields */

// JV1
#define JV1_SECSIZE		256
#define JV1_SECPERTRK	10

// LDOS error message
#define LDOS_OK				0
#define LDOS_SEEK			2
#define LDOS_DATA_NOT_FOUND 5
#define LDOS_SYSTEM_REC		6
#define LDOS_NOT_AVAIL		8
#define LDOS_NOT_FOUND		24


#if EXTRA_IM_SUPPORT


static void dmk_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity);
static void dmk_readsec(UCHAR unit, UCHAR track, UCHAR sector);
static void jv3_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity);
static void jv3_readsec(UCHAR unit, UCHAR track, UCHAR sector);
static void jv1_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity);
static void jv1_readsec(UCHAR unit, UCHAR track, UCHAR sector);


/*
 * Guess the image type. Code borrowed from xtrs/trs_disk.c
 * (Must enter with file pointer at 0)
 */
static im_type_t
set_disk_emutype(UCHAR unit)
{
	FRESULT res;
	USHORT count;
	UINT nbytes;

	res = f_read(&im_file, extra_buffer, 16, &nbytes);
	if (res != FR_OK || nbytes < 16) {
		return IM_NONE;
	}
	if (extra_buffer[0] == 0 || extra_buffer[0] == 0xff) {
		if (extra_buffer[0xc] == 0 && extra_buffer[0xd] == 0 && 
			extra_buffer[0xe] == 0 && extra_buffer[0xf] == 0) {
			count = (USHORT)extra_buffer[2];
			count |= (USHORT)extra_buffer[3] << 8;
			if (count >= 16 && count <= 0x4000) {
				return IM_DMK;
			}
		}
		if (extra_buffer[0xc] == 0x78 && extra_buffer[0xd] == 0x56 &&
			extra_buffer[0xe] == 0x34 && extra_buffer[0xf] == 0x12) {
			return IM_NONE;
		}
	}
	if (extra_buffer[0] == 0 && extra_buffer[1] == 0xfe) {
		return IM_JV1;
	}
	res = f_lseek(&im_file, JV3_SECSPERBLK * 3);
	if (res == FR_OK) {
		res = f_read(&im_file, extra_buffer, 8, &nbytes);
	}	
	if (res != FR_OK || nbytes < 8) {
		return IM_NONE;
	}
	if (extra_buffer[0] == 0 || extra_buffer[0] == 0xff) {
		return IM_JV3;
	}

	return IM_JV1;
}



static void
open_dsk_image(UCHAR unit, const char *filename)
{
	UCHAR ntrack, sside, sdensity;
	FRESULT res;

	// assume ok
	state_error2 = LDOS_OK;
	
	// already open ?	
	if (filename == NULL && cur_unit == unit) {
		return;
	}
	// close previous
	if (cur_unit != -1) {
		f_close(&im_file);
		cur_unit = -1;
	}

	// open new image
	if (filename && filename[0] != '\0') {
		strcpy(im[unit].filename, filename);
	}
	res = f_open(&im_file, im[unit].filename, FA_OPEN_EXISTING | FA_READ);
	if (res != FR_OK) {
		state_error2 = LDOS_NOT_FOUND;
		return;
	}
	cur_unit = unit;

	// if we are re-opening an old file, we are done
	if (filename == NULL) {
		return;
	}

	// find image type
	im[unit].type = set_disk_emutype(unit);
	f_lseek(&im_file, 0);
	
	switch (im[unit].type) {
	case IM_NONE:
		state_error2 = LDOS_NOT_AVAIL;
		return;
	case IM_DMK:
		dmk_open(unit, &ntrack, &sside, &sdensity);
		break;
	case IM_JV3:
		jv3_open(unit, &ntrack, &sside, &sdensity);
		break;
	case IM_JV1:
		jv1_open(unit, &ntrack, &sside, &sdensity);
		break;
	}

	// return image info to TRS80
	extra_buffer[0] = ntrack;
	extra_buffer[1] = sside;
	extra_buffer[2] = sdensity;
	extra_buffer[3] = 1;			// write-protected, we don't support writes
	extra_buffer[4] = im[unit].type;
	state_size2 = 5;
}


/*
 * Called after TRS80 has written data and bytescount2 matches size2
 */
void process_image_cmd(void)
{
	UCHAR unit;

	unit = extra_buffer[0];
	if (unit > 7) {
		state_error2 = LDOS_DATA_NOT_FOUND;
		return;
	}
	
	state_bytesdone2 = 0;

	switch (extra_buffer[1]) {
	case IM_OPEN:
		open_dsk_image(unit, (char *)&extra_buffer[2]);
		break;

	case IM_READSEC:
		if (cur_unit != unit) {
			// re-open our file
			open_dsk_image(unit, NULL);
			if (im[unit].type == IM_DMK) {
				im[unit].u.dmk.cur_track = -1;
			}
			if (state_error2 != LDOS_OK) {
				return;
			}	
		}
		switch (im[unit].type) {
		case IM_NONE:
			state_error2 = LDOS_DATA_NOT_FOUND;
			break;
		case IM_DMK:
			dmk_readsec(unit, extra_buffer[2], extra_buffer[3]);
			break;
		case IM_JV3:
			jv3_readsec(unit, extra_buffer[2], extra_buffer[3]);
			break;
		case IM_JV1:
			jv1_readsec(unit, extra_buffer[2], extra_buffer[3]);
			break;
		}
		break;
	}
}


/*****************************************************************************
 *																			 *
 * DMK																		 *
 *																			 *
 *****************************************************************************/

static int
dmk_get_track_header(UCHAR unit, UCHAR track, UCHAR side)
{
	dmk_t *dmk = &im[unit].u.dmk;
	USHORT u16;
	FRESULT res;
	UINT nbytes;

	// load track header
	u16 = track;
	if (!dmk->sside) u16 <<= 1;
	u16 += side;
	dmk->offset = (DWORD)u16;
	dmk->offset *= dmk->tlen;
	dmk->offset += DMK_HEADER_LEN;

	res = f_lseek(&im_file, dmk->offset);
	if (res == FR_OK) {
		res = f_read(&im_file, im_buf, 0x80, &nbytes);
	}
	if (res != FR_OK || nbytes < 0x80) {
		state_error2 = LDOS_DATA_NOT_FOUND;
	} else {
		dmk->cur_track = track;
		dmk->cur_side = side;
		state_error2 = LDOS_OK;
	}

	return (state_error2);
}


static void
dmk_analyze(UCHAR unit, UCHAR *sside, UCHAR *sdensity)
{
	dmk_t *dmk = &im[unit].u.dmk;
	UCHAR i, j, incr, ddensity;
	FRESULT res;
	DWORD offset;
	USHORT idamp;
	UINT nbytes;

	// consider single-side single-density for now
	*sside = 1;
	*sdensity = 1;

	// load track header of given track, side 1
	offset = (DWORD)dmk->tlen;
	offset += DMK_HEADER_LEN;
	res = f_lseek(&im_file, offset);
	if (res == FR_OK) {
		res = f_read(&im_file, im_buf, 0x80, &nbytes);
	}	
	if (res != FR_OK && nbytes < 0x80) return;

	dmk->nsectors = 0;
	// analyze this header
	for (i = 0; i < 0x40; i+=2) {
		idamp = (USHORT)im_buf[i] + ((USHORT)im_buf[i+1] << 8);
		if (idamp == 0) {
			break;
		}
		// bit 15 means double-density sector
		ddensity = (idamp & 0x8000) ? 1 : 0;
		idamp &= 0x3fff;
		dmk->nsectors++;
		if (ddensity) *sdensity = 0;
		incr = (dmk->sdensity || ddensity) ? 1 : 2;
		// read sector id
		res = f_lseek(&im_file, offset + (DWORD)idamp);
		if (res == FR_OK) {
			res = f_read(&im_file, extra_buffer, 14, &nbytes);
		}
		if (res != FR_OK || nbytes < 14) {
			break;	
		}	
		j = 0;
		// check IDAM marker
		if (extra_buffer[j] != 0xFE) continue;
		j += incr;
		// skip track
		j += incr;
		// check side
		if (extra_buffer[j] & 0x1) {
			*sside = 0;
		}
	}

	if (dmk->nsectors == 0) {
		// single-side... must still check double-density
		dmk_get_track_header(unit, 0, 0);
		for (i = 0; i < 0x40; i += 2) {
			idamp = (USHORT)im_buf[i] + ((USHORT)im_buf[i+1] << 8);
			if (idamp == 0) {
				break;
			}
			// bit 15 means double-density sector
			ddensity = (idamp & 0x8000) ? 1 : 0;
			idamp &= 0x3fff;
			dmk->nsectors++;
			if (ddensity) *sdensity = 0;
		}
	}

	// flags in header take precedence
	if (dmk->sdensity) *sdensity = 1;
	if (dmk->sside) *sside = 1;
}


static void
dmk_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity)
{
	dmk_t *dmk = &im[unit].u.dmk;
	FRESULT res;
	UINT nbytes;

	res = f_read(&im_file, im_buf, 16, &nbytes);
	if (res != FR_OK || nbytes < 16) {
		state_error2 = LDOS_DATA_NOT_FOUND;
		return;
	}
	// parse DMK header
	*ntrack = im_buf[1] - 1;
	dmk->sside = im_buf[4] & 0x10;
	dmk->sdensity = im_buf[4] & 0x40;
	dmk->tlen = (USHORT)im_buf[2] | ((USHORT)im_buf[3] << 8);
	dmk->cur_track = -1;
	dmk->cur_side = -1;
	// guess real density and number of sides of this image
	dmk_analyze(unit, sside, sdensity);

	state_error2 = LDOS_OK;
}


static void
dmk_readsec(UCHAR unit, UCHAR track, UCHAR sector)
{
	dmk_t *dmk = &im[unit].u.dmk;
	UCHAR side;
	USHORT idamp;
	UCHAR ddensity;
	UCHAR i, j, k;
	UCHAR incr;
	UCHAR *p1, *p2;
	UCHAR damlimit;
	UCHAR dam;
	UINT nbytes;
	FRESULT res;

	side = (sector >= dmk->nsectors);
	if (side) sector -= dmk->nsectors;

	if (dmk->cur_track != track || dmk->cur_side != side) {
		res = dmk_get_track_header(unit, track, side);
		if (res != 0) {
			goto not_found;
		}
	}

	for (i = 0; i < 0x40; i+=2) {
		idamp = (USHORT)im_buf[i] + ((USHORT)im_buf[i+1] << 8);
		if (idamp == 0) {
			goto not_found;
		}
		// bit 15 means double-density sector
		ddensity = (idamp & 0x8000) ? 1 : 0;
		idamp &= 0x3fff;

		incr = (dmk->sdensity || ddensity) ? 1 : 2;

		// read sector id
		res = f_lseek(&im_file, dmk->offset + (DWORD)idamp);
		if (res == FR_OK) {
			res = f_read(&im_file, extra_buffer, 14, &nbytes);
		}
		if (res != FR_OK || nbytes < 14) {
			goto not_found;
		}	

		j = 0;
		// check IDAM marker
		if (extra_buffer[j] != 0xFE) continue;
		j += incr;

		// check track
		if (extra_buffer[j] != track) continue;
		j += incr;

		// check side
		if ((extra_buffer[j]  & 0x1) != side) continue;
		j += incr;

		// check sector
		if (extra_buffer[j] != sector) continue;
		j += incr;

		// size code
		j += incr;
		
		// CRC
		j += incr;
		j += incr;

		// if we are here, we found our sector !
		damlimit = ddensity ? 43 : 30;
		damlimit *= incr;
		res = f_lseek(&im_file, dmk->offset + (DWORD)(idamp + j));
		if (res == FR_OK) {
			res = f_read(&im_file, extra_buffer, damlimit, &nbytes);
		}	
		if (res != FR_OK || nbytes < damlimit) {
			goto not_found;
		}	
		dam = 0xff;
		for (k = 0; k < damlimit; k += incr) {
			if (0xf8 <= extra_buffer[k] && extra_buffer[k] <= 0xfb) {
				dam = extra_buffer[k];
				break;
			}
		}
		if (dam == 0xff) {
			// DAM not found
			goto not_found;
		}
		k += incr;

		// read the sector
		res = f_lseek(&im_file, dmk->offset + (DWORD)(idamp + j + k));
		if (res != FR_OK) {
			goto not_found;
		}	
		if (incr == 1) {
			res = f_read(&im_file, extra_buffer, 256, &nbytes);
		} else {
			p1 = extra_buffer;
			res = f_read(&im_file, sector_buffer, 256, &nbytes);
			if (res != FR_OK || nbytes < 256) {
				goto not_found;
			}	
			p2 = sector_buffer;
			for (k = 0; k < 128; k++) {
				*(p1++) = *(p2++);
				p2++;
			}
			res = f_read(&im_file, sector_buffer, 256, &nbytes);
			if (res != FR_OK || nbytes < 256) {
				goto not_found;
			}	
			p2 = sector_buffer;
			for (k = 0; k < 128; k++) {
				*(p1++) = *(p2++);
				p2++;
			}
		}
		state_error2 = (dam == 0xf8 || dam == 0xfa) ? LDOS_SYSTEM_REC : LDOS_OK;
		state_size2 = 256;
		return;
	}
not_found:
	state_error2 = LDOS_DATA_NOT_FOUND;
	return;
}


/*****************************************************************************
 *																			 *
 * JV3																		 *
 *																			 *
 *****************************************************************************/

static void
jv3_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity)
{
	UCHAR i;
	USHORT n;
	FRESULT res;
	UINT nbytes;
	jv3_t *jv3 = &im[unit].u.jv3;

	*ntrack = 0;
	*sside = 1;
	*sdensity = 1;
	jv3->nsectors = 0;

	n = 0;
	while (n < JV3_SECSPERBLK) {
		res = f_read(&im_file, extra_buffer, 255, &nbytes);
		if (res != FR_OK || nbytes < 255) {
			return;
		}	
		for (i = 0; i < 255 && n < JV3_SECSPERBLK; i+=3, n++) {
			if (extra_buffer[i] != 0xff) {
				if (extra_buffer[i] > *ntrack) {
					*ntrack = extra_buffer[i];
				}
				if (extra_buffer[i+1] > jv3->nsectors) {
					jv3->nsectors = extra_buffer[i+1];
				}
				if (extra_buffer[i+2] & JV3_DENSITY) *sdensity = 0;
				if (extra_buffer[i+2] & JV3_SIDE) *sside = 0;
			}
		}
	}
    jv3->nsectors++;
}


static UCHAR
jv3_find_sector(UCHAR unit, UCHAR track, UCHAR sector, UCHAR side)
{
	jv3_t *jv3 = &im[unit].u.jv3;
	USHORT n;
	UCHAR i, j;
	UCHAR fside;
	UINT nbytes;
	FRESULT res;

	res = f_lseek(&im_file, jv3->offset);
	if (res != FR_OK) {
		return 0;
	}	
	jv3->offset += 3*JV3_SECSPERBLK + 1;
	n = 0;
	while (n < JV3_SECSPERBLK) {
		res = f_read(&im_file, extra_buffer, 255, &nbytes);
		if (res != FR_OK || nbytes < 255) {
			return 0;
		}	
		for (i = 0; i < 255 && n < JV3_SECSPERBLK; i+=3, n++) {
			if (extra_buffer[i] != 0xff && extra_buffer[i] == track &&
				extra_buffer[i+1] == sector) {
				fside = (extra_buffer[i+2] & JV3_SIDE) ? 1 : 0;
				if (side == fside) {
					jv3->dam = extra_buffer[i+2] & (JV3_DENSITY | JV3_DAM);
					return 1;
				}
			}
			// increment offset
			j = (extra_buffer[i+2] & JV3_SIZE) ^ ((extra_buffer[i] == JV3_FREE) ? 2 : 1);
			jv3->offset += ((DWORD)128 << j);
		}
	}
	// not found
	return 0;
}


static void
jv3_readsec(UCHAR unit, UCHAR track, UCHAR sector)
{
	jv3_t *jv3 = &im[unit].u.jv3;
	FRESULT res;
	UINT nbytes;
	UCHAR side;

	side = (sector >= jv3->nsectors);
	if (side) sector -= jv3->nsectors;

	// find the track/sector/side offset
	jv3->offset = 0;
	if (!jv3_find_sector(unit, track, sector, side)) {
		// try 2nd header
		if (!jv3_find_sector(unit, track, sector, side)) {
			// failure
			state_error2 = LDOS_DATA_NOT_FOUND;
			return;
		}
	}

	res = f_lseek(&im_file, jv3->offset);
	if (res == FR_OK) {
		res = f_read(&im_file, extra_buffer, 256, &nbytes);
	}
	if (res != FR_OK || nbytes < 256) {
		state_error2 = LDOS_DATA_NOT_FOUND;
		return;
	}
	if (jv3->dam == (JV3_DENSITY | JV3_DAMDDF8) ||
	  jv3->dam == JV3_DAMSDF8 || jv3->dam == JV3_DAMSDFA) {
		state_error2 = LDOS_SYSTEM_REC;
	} else {
		state_error2 = LDOS_OK;
	}
	state_size2 = 256;
}



/*****************************************************************************
 *																			 *
 * JV1																		 *
 *																			 *
 *****************************************************************************/

static void
jv1_open(UCHAR unit, UCHAR *ntrack, UCHAR *sside, UCHAR *sdensity)
{
	*ntrack = 35;
	*sside = 1;
	*sdensity = 1;
}


static void
jv1_readsec(UCHAR unit, UCHAR track, UCHAR sector)
{
	jv1_t *jv1 = &im[unit].u.jv1;
	FRESULT res;
	UINT nbytes;

	if (sector >= JV1_SECPERTRK) {
		state_error2 = LDOS_DATA_NOT_FOUND;
		return;
	}
	jv1->offset = track;
	jv1->offset *= JV1_SECPERTRK;
	jv1->offset += sector;
	jv1->offset <<= 8;

	res = f_lseek(&im_file, jv1->offset);
	if (res == FR_OK) {
		res = f_read(&im_file, extra_buffer, 256, &nbytes);
	}
	if (res != FR_OK || nbytes < 256) {
		state_error2 = LDOS_NOT_FOUND;
		return;
	}		
	state_size2 = 256;
	state_error2 = (track == 17) ? LDOS_SYSTEM_REC : LDOS_OK;
}

#endif // EXTRA_IM_SUPPORT
