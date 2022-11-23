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

#ifndef _TRS_EXTRA
#define _TRS_EXTRA

#define TRS_EXTRA_VERSION		0x00
#define TRS_EXTRA_GETTIME		0x01
#define TRS_EXTRA_SETTIME		0x02
#define TRS_EXTRA_OPENFILE		0x03
#define TRS_EXTRA_READFILE		0x04
#define TRS_EXTRA_WRITEFILE		0x05
#define TRS_EXTRA_CLOSEFILE		0x06
#define TRS_EXTRA_BOOTLOADER	0x07
#define TRS_EXTRA_OPENDIR		0x08
#define TRS_EXTRA_READDIR		0x09
#define TRS_EXTRA_MOUNTDRIVE	0x0A
#define TRS_EXTRA_SEEKFILE		0x0B
#define TRS_EXTRA_INFODRIVE		0x0C
#define TRS_EXTRA_IMAGE			0x0D
#define TRS_EXTRA_READ_HEADER	0x0E

// LSDOS time : 6 bytes
#define TRS80_SEC       0
#define TRS80_MIN       1
#define TRS80_HOUR      2
#define TRS80_YEAR      3
#define TRS80_DAY       4
#define TRS80_MONTH     5

// TRS_EXTRA_MOUNTDRIVE options
#define TRS_EXTRA_MOUNT_SLOW	0x01	// no fast seek
#define TRS_EXTRA_MOUNT_CREATE	0x02	// create if needed
#define TRS_EXTRA_MOUNT_RO		0x04	// read-only

typedef enum {
    IM_NONE,
    IM_DMK,
    IM_JV3,
    IM_JV1
} im_type_t;

typedef struct {
    USHORT tlen;
    UCHAR cur_track;
    UCHAR cur_side;
    UCHAR sside;        // 1 if single side in header flag
    UCHAR sdensity;     // 1 if single density in header flag
    UCHAR nsectors;
    long offset;
} dmk_t;

typedef struct {
    UCHAR nsectors;
    UCHAR dam;
    DWORD offset;
} jv3_t;

typedef struct {
    DWORD offset;
} jv1_t;

typedef struct {
    im_type_t type;
    char filename[13];
    union {
        dmk_t dmk;
        jv3_t jv3;
        jv1_t jv1;
    } u;
} image_t;


extern rom UCHAR (*rom trs_extra[])(UCHAR);

UCHAR trs_extra_ignored(UCHAR);
UCHAR trs_extra_version(UCHAR);
UCHAR trs_extra_gettime(UCHAR);
UCHAR trs_extra_settime(UCHAR);
UCHAR trs_extra_openfile(UCHAR);
UCHAR trs_extra_readfile(UCHAR);
UCHAR trs_extra_writefile(UCHAR);
UCHAR trs_extra_closefile(UCHAR);
UCHAR trs_extra_bootloader(UCHAR);
UCHAR trs_extra_opendir(UCHAR);
UCHAR trs_extra_readdir(UCHAR);
UCHAR trs_extra_mountdrive(UCHAR);
UCHAR trs_extra_infodrive(UCHAR);
UCHAR trs_extra_seekfile(UCHAR);
UCHAR trs_extra_image(UCHAR);
UCHAR trs_extra_read_header(UCHAR);

// dsk.c
void process_image_cmd(void);

#endif
