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

#ifndef _TRS_HARD_H
#define _TRS_HARD_H

#include "FatFS/ff.h"
#include "trs_hard_defs.h"
#include "trs_extra.h"

/*
 * NOTE: many #define are commented below because they were moved
 *       to trs_hard_defs.h. trs_hard_defs.h is used in the .asm
 *       file and .c files.
 */

#define TRS_HARD_MAXDRIVES  	4
#define FAST_SEEK_LEN			32
#define MAX_SECTOR_SIZE			512

/* IMAGE SUPPORT is optional */
#define EXTRA_IM_SUPPORT		1



/* supported sector sizes */
typedef enum {
	SECTOR_SIZE_256 = 0,
	SECTOR_SIZE_512,
	SECTOR_SIZE_1024,
	SECTOR_SIZE_128
} sector_size_t;

/* drive structure */
typedef struct {
	FIL file;
	CHAR filename[13];
	UCHAR avail;		// 1 if drive open and available
	USHORT dirty;		// countdown before calling fsync() (in ms)

	/* values decoded from rhh */
	UCHAR writeprot;
	USHORT cyls;		// cyls per drive
	UCHAR heads;		// tracks per cyl
	UCHAR secs;			// secs per track

	/* fast seek */
#if _USE_FASTSEEK
	DWORD stbl[FAST_SEEK_LEN];
#endif
} Drive;

/* SD-Card and its FAT status */
#define FS_NOT_MOUNTED		0
#define FS_MOUNTED_OK		1
#define FS_MOUNTED_ERROR	2

/* Extra buffer size */
#define EXTRA_SIZE				256

/* RSHARD assumes 32 sectors/track */
/* Other sizes currently not emulated */
#define TRS_HARD_SEC_PER_TRK 	32

/* Calls to f_sync() are delayed by this amount of 200Hz ticks */
#define SYNC_DELAY		600		// 3 secs

/*
 * Tandy-specific registers
 */

/* Upper 4 bits where the hard drive controller is mapped.
   This is normally 0xC0, but in this controller, the GAL decodes
   the upper bits, and we are only interrupted if they match.
   Furthermore, we don't even see A[7..4].  */
#define TRS_HARD_MAP			0x00

/* Write protect switch register (read only):
 *  abcd--pi
 *  a = drive 0 write protected
 *  b = drive 1 write protected
 *  c = drive 2 write protected
 *  d = drive 3 write protected
 *  p = at least one drive write protected
 *  i = interrupt request
 */
#define TRS_HARD_WP				(TRS_HARD_MAP | 0x0)
#define TRS_HARD_WPBIT(d)		(0x80 >> (d))
#define TRS_HARD_WPSOME			0x02
#define TRS_HARD_INTRQ			0x01 /* not emulated */

/* Control register (read(?)/write):
 *  ---sdw--
 *  s = software reset
 *  d = device enable
 *  w = wait enable
 */
#define TRS_HARD_CONTROL		(TRS_HARD_MAP | 0x1)
#define TRS_HARD_SOFTWARE_RESET 0x10
#define TRS_HARD_DEVICE_ENABLE  0x08
#define TRS_HARD_WAIT_ENABLE    0x04


/*
 * WD1010 registers
 */

/* Data register (read/write) */
#define TRS_HARD_DATA 			(TRS_HARD_MAP | 0x8)

/* Error register (read only):
 *  bdin-atm
 *  b = block marked bad
 *  e = uncorrectable error in data
 *  i = uncorrectable error in id (unused?)
 *  n = id not found
 *  - = unused
 *  a = command aborted
 *  t = track 0 not found
 *  m = bad address mark
 */
#define TRS_HARD_ERROR 			(TRS_HARD_DATA+1)
//#define	TRS_HARD_BBDERR   		0x80
//#define TRS_HARD_DATAERR  		0x40
//#define TRS_HARD_IDERR    		0x20 /* unused? */
//#define TRS_HARD_NFERR    		0x10
//#define TRS_HARD_MCRERR   		0x08 /* unused */
//#define TRS_HARD_ABRTERR  		0x04
//#define TRS_HARD_TRK0ERR  		0x02
//#define TRS_HARD_MARKERR  		0x01

/* Write precompensation register (write only) */
/* Value *4 is the starting cylinder for write precomp */
#define TRS_HARD_PRECOMP 		(TRS_HARD_DATA+1)

/* Sector count register (read/write) */
/* Used only for multiple sector accesses; otherwise ignored. */
/* Autodecrements when used. */
#define TRS_HARD_SECCNT 		(TRS_HARD_DATA+2)

/* Sector number register (read/write) */
#define TRS_HARD_SECNUM 		(TRS_HARD_DATA+3)

/* Cylinder low byte register (read/write) */
#define TRS_HARD_CYLLO 			(TRS_HARD_DATA+4)

/* Cylinder high byte register (read/write) */
#define TRS_HARD_CYLHI 			(TRS_HARD_DATA+5)

/* Size/drive/head register (read/write):
 *  essddhhh
 *  e = 0 if CRCs used; 1 if ECC used
 *  ss = sector size; 00=256, 01=512, 10=1024, 11=128
 *  dd = drive
 *  hhh = head
 */
#define TRS_HARD_SDH 			(TRS_HARD_DATA+6)
#define TRS_HARD_ECCMASK    	0x80
#define TRS_HARD_ECCSHIFT   	7
#define TRS_HARD_SIZEMASK   	0x60
#define TRS_HARD_SIZESHIFT  	5
#define TRS_HARD_DRIVEMASK  	0x18
#define TRS_HARD_DRIVESHIFT 	3
#define TRS_HARD_HEADMASK  		0x07
#define TRS_HARD_HEADSHIFT  	0
#define TRS_HARD_MAXHEADS   	8

/* Status register (read only):
 *  brwsdcie
 *  b = busy
 *  r = drive ready
 *  w = write error
 *  s = seek complete
 *  d = data request
 *  c = corrected ecc (reserved)
 *  i = command in progress (=software reset required)
 *  e = error
 */
#define TRS_HARD_STATUS 		(TRS_HARD_DATA+7)
//#define TRS_HARD_BUSY     		0x80
//#define TRS_HARD_READY    		0x40
//#define TRS_HARD_WRERR    		0x20
//#define TRS_HARD_SEEKDONE 		0x10
//#define TRS_HARD_DRQ	  		0x08
//#define TRS_HARD_ECC	  		0x04
//#define TRS_HARD_CIP	  		0x02
//#define TRS_HARD_ERR	  		0x01

/* Command register (write only) */
#define TRS_HARD_COMMAND 		(TRS_HARD_DATA+7)

/*
 * WD1010 commands
 */

//#define TRS_HARD_CMDMASK 		0xf0

/* Restore:
 *  0001rrrr
 *  rrrr = step rate; 0000=35us, else rrrr*0.5ms
 */
//#define TRS_HARD_RESTORE 		0x10

/* Read sector:
 *  0010dm00
 *  d = 0 for interrupt on DRQ, 1 for interrupt at end (DMA style)
 *      TRS-80 always uses programmed I/O, INTRQ not connected, I believe.
 *  m = multiple sector flag (not emulated!)
 */
//#define TRS_HARD_READ  			0x20
//#define TRS_HARD_DMA   			0x08
//#define TRS_HARD_MULTI 			0x04

/* Write sector:
 *  00110m00
 *  m = multiple sector flag (not emulated!)
 */
//#define TRS_HARD_WRITE 			0x30

/* Verify sector (or "Scan ID"):
 *  01000000
 */
//#define TRS_HARD_VERIFY 		0x40

/* Format track:
 *  01010000
 */
//#define TRS_HARD_FORMAT 		0x50

/* Init (??):
 *  01100000
 */
//#define TRS_HARD_INIT 			0x60

/* Seek to specified sector/head/cylinder:
 *  0111rrrr
 *  rrrr = step rate; 0000=35us, else rrrr*0.5ms
 */
//#define TRS_HARD_SEEK 			0x70


/* globals */
extern near UCHAR state_status;
extern near UCHAR state_busy;
extern near UCHAR state_present;
extern near UCHAR state_control;
extern near UCHAR state_error;
extern near UCHAR state_seccnt;
extern near UCHAR state_secnum;
extern near USHORT state_cyl;
extern near UCHAR state_drive;
extern near UCHAR state_head;
extern near UCHAR state_wp;
extern near UCHAR state_command;
extern near USHORT state_bytesdone;
extern near UCHAR state_secsize;
extern near USHORT state_secsize16;
extern Drive state_d[];
extern near UCHAR state_command2;
extern near UCHAR state_error2;
extern near USHORT state_size2;
extern near UCHAR state_bytesdone2;
extern UCHAR state_file2_open;
extern FIL state_file2;
extern BYTE sector_buffer[];
extern BYTE extra_buffer[];
extern near UCHAR val_1F;
extern near UCHAR foo;
extern DIR state_dir;
extern UCHAR state_dir_open;
extern FILINFO state_fno;
extern FIL im_file;
extern BYTE im_buf[];
extern UCHAR cur_unit;
extern image_t im[];
#if _USE_FASTSEEK
extern DWORD im_stbl[];
#endif

/* prototypes */
void update_status(UCHAR new_status);
void trs_action(void);
void trs_sync(void);
void trs_hard_init(void);
FRESULT open_drive(UCHAR drive_num, UCHAR options);
FRESULT open_drives(void);
void close_drive(UCHAR drive_num);
void close_drives(void);

#endif
