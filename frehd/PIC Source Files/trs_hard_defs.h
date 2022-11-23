

#ifndef _TRS_HARD_DEFS_H
#define _TRS_HARD_DEFS_H

#define	TRS_HARD_BBDERR   		0x80
#define TRS_HARD_DATAERR  		0x40
#define TRS_HARD_IDERR    		0x20
#define TRS_HARD_NFERR    		0x10
#define TRS_HARD_MCRERR   		0x08
#define TRS_HARD_ABRTERR  		0x04
#define TRS_HARD_TRK0ERR  		0x02
#define TRS_HARD_MARKERR  		0x01

#define TRS_HARD_BUSY     		0x80
#define TRS_HARD_READY    		0x40
#define TRS_HARD_WRERR    		0x20
#define TRS_HARD_SEEKDONE 		0x10
#define TRS_HARD_DRQ	  		0x08
#define TRS_HARD_ECC	  		0x04
#define TRS_HARD_CIP	  		0x02
#define TRS_HARD_ERR	  		0x01

#define TRS_HARD_CMDMASK 		0xf0
#define TRS_HARD_RESTORE		0x10
#define TRS_HARD_READ  			0x20
#define TRS_HARD_DMA   			0x08
#define TRS_HARD_MULTI 			0x04
#define TRS_HARD_WRITE 			0x30
#define TRS_HARD_VERIFY 		0x40
#define TRS_HARD_FORMAT 		0x50
#define TRS_HARD_INIT 			0x60
#define TRS_HARD_SEEK 			0x70


#define ACTION_DS1307_RELOAD	0x01
#define ACTION_DS1307_BIT		0
#define ACTION_TICK				0x02
#define ACTION_TICK_BIT			1
#define ACTION_TRS				0x04
#define ACTION_TRS_BIT			2

#define ACTION_HARD_SEEK		0x1
#define ACTION_HARD_READ		0x2
#define ACTION_HARD_WRITE		0x3

#define ACTION_EXTRA2			0x40
#define ACTION_EXTRA2_BIT		6
#define ACTION_EXTRA			0x80
#define ACTION_EXTRA_BIT		7

#endif
