/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module  (C)ChaN, 2007             */
/*-----------------------------------------------------------------------*/
/* Only rcvr_spi(), xmit_spi(), disk_timerproc() and some macros         */
/* are platform dependent.                                               */
/*-----------------------------------------------------------------------*/

#include <p18cxxx.h>
#include "bootloader.inc"
#include "diskio.h"
#include "HardwareProfile.h"
#include "ffconf.h"
#include "trs_hard.h"
#include "led.h"
#if UART_DEBUG || PROTEUS_SIMULATOR
#include "serial.h"
#endif

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */
#define CMD59	(0x40+59)	/* CRC_ON_OFF */


/* Port Controls  (Platform dependent) */
#define SELECT()	SD_CS = 0	/* MMC CS = L */
#define DESELECT()	SD_CS = 1	/* MMC CS = H */

#define SOCKPORT	SD_WP_CD_PORT	/* Socket contact port */
#define SOCKWP		(1<<SD_WP_PIN)		/* Write protect switch */
#define SOCKINS		(1<<SD_CD_PIN)		/* Card detect switch */

#define	FCLK_SLOW()	(SSPCON1 = 0x22)	/* Set slow clock (100k-400k) */
#define	FCLK_FAST()	(SSPCON1 = 0x20)	/* Set fast clock (depends on the CSD) */

#define MAX_RETRIES		32

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
UCHAR Timer1, Timer2;		/* 200Hz decrement timer */

static
UINT CardType;

static
BYTE Retry;

extern BYTE crc7;
extern USHORT crc16;
extern BYTE crc_enabled;


/*-----------------------------------------------------------------------*/
/* Exchange a byte between PIC and MMC via SPI  (Platform dependent)     */
/*-----------------------------------------------------------------------*/

#define xmit_spi(dat) 	xchg_spi(dat)
#define rcvr_spi()		xchg_spi(0xFF)
#define rcvr_spi_m(p)	SSPBUF = 0xFF; while (!SSPSTATbits.BF); *(p) = (BYTE)SSPBUF;

static
BYTE xchg_spi (BYTE dat)
{
	SSPBUF = dat;
	while (!SSPSTATbits.BF);
	return (BYTE)SSPBUF;
}


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (void)
{
	BYTE res;

	Timer2 = 100;	/* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do
		res = rcvr_spi();
	while ((res != 0xFF) && Timer2);

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	DESELECT();
	rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
#if 0
	SSPSTAT = 0x40;
	SSPCON1 = 0x02;

	SSPCON1bits.SSPEN = 1;
#endif
}

static
void power_off (void)
{
	SELECT();				/* Wait for card ready */
	wait_ready();
	release_spi();

	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL rcvr_datablock (
	BYTE *buff,							/* Data buffer to store received data */
	UINT btr							/* Byte count (must be multiple of 4) */
)
{
	static BYTE token;
	static BYTE crcH, crcL;

	gled.val |= LED_FLASH;

	Timer1 = 40;
	do {								/* Wait for data packet in timeout of 200ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);

	if (token != 0xFE) return FALSE;		/* If not valid data token, retutn with error */

	crc16 = 0;
	do {								/* Receive the data block into buffer */
		*buff = rcvr_spi();
		mmc_crc16(*buff);
		buff++;
	} while( --btr );
	crcH = rcvr_spi();					/* CRC */
	crcL = rcvr_spi();
	if (crcH != (BYTE)(crc16 >> 8) || crcL != (BYTE)crc16) {
#if UART_DEBUG || PROTEUS_SIMULATOR
		usart_put_hex(crcH);
		usart_put_hex(crcL);
		usart_send('-');
		usart_put_short(crc16);
#endif
		return FALSE;					/* BAD CRC */
	}

#if UART_DEBUG || PROTEUS_SIMULATOR
	usart_send('C');
#endif

	return TRUE;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0
static
BOOL xmit_datablock (
	const BYTE *buff,				/* 512 byte data block to be transmitted */
	BYTE token						/* Data/Stop token */
)
{
	static BYTE resp;
	static UINT bc;
	static BYTE *ptr;

	if (wait_ready() != 0xFF) return FALSE;

	xmit_spi(token);				/* Xmit data token */
	if (token != 0xFD) {			/* Is data token */

		rled.val |= LED_FLASH;

		bc = 512;
		ptr = buff;
		crc16 = 0;
		do {
			mmc_crc16(*ptr++);
		} while (--bc);

		bc = 512;
		do {						/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
		} while (--bc);
		xmit_spi((BYTE)(crc16 >> 8));		/* CRC */
		xmit_spi((BYTE)crc16);
		resp = rcvr_spi();			/* Receive data response */
#if UART_DEBUG || PROTEUS_SIMULATOR
		usart_put_hex(resp);
#endif
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif	/* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	static BYTE n, res;

	if (cmd & 0x80) {					/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	crc7 = 0;
	xmit_spi(cmd);						/* Start + Command index */
	mmc_crc7(cmd);
	n = (BYTE)(arg >> 24);
	xmit_spi(n);
	mmc_crc7(n);
	n = (BYTE)(arg >> 16);
	xmit_spi(n);
	mmc_crc7(n);
	n = (BYTE)(arg >> 8);
	xmit_spi(n);
	mmc_crc7(n);
	n = (BYTE)arg;
	xmit_spi(n);
	mmc_crc7(n);
	crc7 <<= 1;
	crc7 |= 0x01;
	xmit_spi(crc7);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;							/* Return with the response value */
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	BYTE n, cmd, ty, ocr[4];


	if (drv) return STA_NOINIT;				/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;		/* No card in the socket */

	power_on();								/* Force socket power on */
	FCLK_SLOW();
	for (n = 10; n; n--) rcvr_spi();		/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 200;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDSC */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}

	CardType = ty;
	release_spi();

	if (ty) {					/* Initialization succeded */
		FCLK_FAST();
		n = send_cmd(CMD59, 0x1);
		if (n == 0) {
			Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		} else {
			ty = 0;
		}
	}
	if (!ty) {
		power_off();			/* Initialization failed */
	}

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
#if UART_DEBUG || PROTEUS_SIMULATOR
	usart_puts("R ");
	usart_put_long(sector);
#endif
	if (drv || count != 1) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector <<= 9; 	/* Convert to byte address if needed */

	Retry = MAX_RETRIES;
	do {
#if UART_DEBUG || PROTEUS_SIMULATOR
		usart_puts(".");
#endif
		/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)		/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512)) {
			count = 0;
		}
		release_spi();
	} while (count && Retry-- > 0);

#if UART_DEBUG || PROTEUS_SIMULATOR
	usart_puts2("");
#endif

	return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
#if UART_DEBUG || PROTEUS_SIMULATOR
	usart_puts("W ");
	usart_put_long(sector);
#endif
	if (drv || count != 1) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector <<= 9;	/* Convert to byte address if needed */

	Retry = MAX_RETRIES;
	do {
#if UART_DEBUG || PROTEUS_SIMULATOR
		usart_puts(".");
#endif
		/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE)) {
			count = 0;
		}
		release_spi();
	} while (count && Retry-- > 0);

#if UART_DEBUG || PROTEUS_SIMULATOR
	usart_puts2("");
#endif

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD csize;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	/* Flush dirty buffer if present */
			SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* MMC or SDC ver 1.XX */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
			if (CardType & CT_SD2) {	/* SDC ver 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC ver 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {		/* SDC ver 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 0; n < 4; n++)
					*((BYTE*)buff+n) = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if ((CardType & CT_SD2) && send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(buff, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
	}

	release_spi();

	return res;
}

/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 1ms                         */


void disk_timerproc (void)
{
	static WORD pv;
	WORD p;
	BYTE s;

	/* 200Hz decrement timer */
	if (Timer1) Timer1--;
	if (Timer2) Timer2--;
	led_count++;

	p = pv;
	pv = SOCKPORT & (SOCKWP | SOCKINS);	/* Sample socket switch */

	if (p == pv) {					/* Have contacts stabled? */
		s = Stat;

		if (p & SOCKWP)				/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (p & SOCKINS)			/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}


/*
 * Card presence helper
 */
BYTE card_present(void)
{
	return (!(Stat & STA_NODISK));
}
