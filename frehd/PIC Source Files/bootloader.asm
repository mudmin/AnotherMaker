;;;
;;; Copyright (C) 2013 Frederic Vecoven
;;;
;;; This file is part of trs_hard
;;;
;;; trs_hard is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or
;;; (at your option) any later version.
;;;
;;; trs_hard is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program. If not, see <http://www.gnu.org/licenses/>.
;;;

	
	list	p=18f4620
	include p18f4620.inc
	include bootloader.inc
	include crc.inc

;
; TRS HARD DRIVE emulator bootloader
;
; - bootloader lives in 0000-07FF
; - Enter bootloader by erasing the last two locations of the EEPROM (0x3FE-3FF),
;   and reset the PIC.
; - In bootloader mode, the TRS80 can only write data to the interface.
;   The data is sent in intel-hex format. TRS I/O address 0xC0
;   Once a line is sent, write any data to address 0xC1. This will trigger the
;   processing of the line. During processing, the TRS80 can poll the status
;   register, and wait to the READY bit to be set. The lower nibble of status 
;   indicates the result of the processing (0 = no error)
; - The hex file must contain the CRC16 of the image (0x800 -> 0xFFFD) at
;   location FFFE-FFFF. If the CRC is invalid, the code will remain in
;   bootloader.
; - The last 2 bytes of the EEPROM (3FE-3FF) should also contain the CRC. If
;   the CRC is not found, the PIC will enter bootloader, but a timer will 
;   start. After ~30 seconds, the bootloader will rewrite the CRC at the end
;   of the EEPROM (if the flash is valid, obviously) and exit bootloader.
; - An additional type (06) has been added in the intel-hex format. It is used
;   to erase flash (use address FF00) or eeprom (use address FF01)
;


;
; 18F4620
;
#define BLOCKSIZE		0x40
#define ERASE_BLOCKSIZE		0x40

;
; local defines
;
#define TRS_ADDR		PORTA
#define TRS_A0			PORTA,3
#define TRS_A1			PORTA,2
#define TRS_A2			PORTA,1
#define TRS_A3			PORTA,0
#define TRS_DATA_IN		PORTD
#define TRS_DATA_OUT		LATD
#define TRS_DATA_TRIS		TRISD
#define TRS_RD_N		PORTA,4
#define TRS_WAIT		LATB,3
#define TRS_WAIT_TRIS		TRISB,3
#define GAL_INT_IE		INTCON3,INT1IE
#define GAL_INT_IP		INTCON3,INT1IP
#define GAL_INT_IF		INTCON3,INT1IF
#define GAL_INT_EDGE		INTCON2,INTEDG1
#define GAL_INT			PORTB,1
#define STAT_CS			LATC,0
#define STAT_CS_TRIS		TRISC,0
#define SPI_SCK_TRIS		TRISC,3
#define SPI_MOSI_TRIS		TRISC,5

#define GLED			LATB,7
#define GLED_TRIS		TRISB,7
#define RLED			LATB,6
#define RLED_TRIS		TRISB,6

; hard drive emulator status register. Lower nibble = errno
#define TRS_BOOT_BUSY     	0x80
#define TRS_BOOT_READY    	0x40

#define TRS_BOOT_OK		0x00
#define TRS_BOOT_RANGE_ERR	0x01
#define TRS_BOOT_IGNORED  	0x02
#define TRS_BOOT_CHECKSUM_ERR  	0x03
#define TRS_BOOT_INVALID 	0x04
#define TRS_BOOT_SEQ_ERR	0x05
#define TRS_BOOT_PROTECTED	0x06

; flags
#define BF_PROCESS		1	; true if a line should be processed
#define BF_IGNORE		2	; true if everything should be ignored
#define BF_DIRTY		3	; true if holding regs are dirty
#define BF_FLASH_INVALID	6	; true if flash has incorrect CRC16
#define BF_EEPROM_INVALID	7	; true if eeprom has incorrect CRC16

	
;
; DATA
;
var_g	UDATA_ACS 0x7B
timeout		res	.1
crc		res	.2
flags		res	.1
boot_mode	res	.1

var_b0	UDATA_OVR 0x80
checksum	res	.1
count		res	.1
addr		res	.4
type		res	.1
tmp		res	.1


; buffers
#define BUF1	0x100


boot_reset CODE 0x0
	goto	_bootloader
	
boot_version CODE_PACK 0x6
	db	BOOTLOADER_MAJOR
	db	BOOTLOADER_MINOR

boot_int_hi_code CODE 0x8
	goto	APP_HI_INT
	
bootloader_code	CODE 0x18
	; start with interrupt routine to avoid one extra goto

;	
; low priority interrupt
;
; - if not in bootloader mode, go to app lo interrupt routine
; - ignore TRS reads
; - receive data in *FSR2
; - if A0 is set, set the process flag and BUSY status
;
boot_int_lo:
	tstfsz	boot_mode		; bootloader mode ?
	goto	APP_LO_INT		; no... goto app interrupt
					; yes..
	btfss	GAL_INT_IF		; GAL interrupt?
	bra	bi_timer0
	
	bcf	GAL_INT_IF		; ack interrupt
	btfss	TRS_RD_N
	bra	bi_read
	movff	TRS_DATA_IN,POSTINC2	; put one byte in buffer
	btfss	TRS_A0
	bra	bi_done
	bsf	flags,BF_PROCESS	; set flag for main loop
	movlw	TRS_BOOT_BUSY		; update status with BUSY bit
	rcall	update_status
bi_done:
	bsf	TRS_WAIT		; tell GAL to release TRS_WAIT
	btfss	GAL_INT
	bra	$-2
	setf	TRS_DATA_TRIS
	bcf	TRS_WAIT		; be ready for next transfer
	retfie	FAST
bi_read
	movf	TRS_ADDR,w
	andlw	0x0f
	bnz	bi_read1
bi_read0
	movlw	BOOTLOADER_MAJOR	; C0 -> major version
	movwf	TRS_DATA_OUT
	clrf	TRS_DATA_TRIS
	bra	bi_done
bi_read1
	xorlw	0x8
	bnz	bi_read2
	movlw	BOOTLOADER_MINOR	; C1 -> minor version
	movwf	TRS_DATA_OUT
	clrf	TRS_DATA_TRIS
	bra	bi_done	
bi_read2
	xorlw	0x8 ^ 0x4
	bnz	bi_read3
	movff	crc+1,TRS_DATA_OUT	; C2 -> CRC high
	clrf	TRS_DATA_TRIS
	bra	bi_done	
bi_read3
	xorlw	0x4 ^ 0xC
	bnz	bi_read4
	movff	crc+0,TRS_DATA_OUT	; C3 -> CRC low
	clrf	TRS_DATA_TRIS
	bra	bi_done
bi_read4
	movlw	0x4F			; others -> magic 4F
	movwf	TRS_DATA_OUT
	clrf	TRS_DATA_TRIS
	bra	bi_done
	
bi_timer0
	btfss	INTCON,TMR0IF
	bra	bi_ret
	
	bcf	INTCON,TMR0IF
	tstfsz	timeout
	decf	timeout,f
	
bi_ret
	retfie	FAST
	
;
; update status (with SPI)
;
update_status:
	movwf	SSPBUF
	btfss	SSPSTAT,BF
	bra	$-2
	bsf	STAT_CS
	movf	SSPBUF,w
	bcf	STAT_CS
	return
	
;
; read byte : take 2 ascii chars (pointed by FSR2) and convert
;
read_byte:
	swapf	INDF2,w			; swap hi nibble into result
	btfsc	POSTINC2,6		; check if range 'A'-'F'
	addlw	0x8F			; add correction
	addwf	INDF2,w			; add lo nibble
	btfsc	POSTINC2,6		; check if range 'A'-'F'
	addlw	0xF9			; add correction '9'-'A'+1
	addlw	0xCD			; adjust final result -0x33
	return
	
		
; ********************************************************************
; bootloader entry
; ********************************************************************
_bootloader:
	GLOBAL	_bootloader

	clrf	boot_mode		; assume bootloader mode for now
	clrf	flags
		
	call	do_crc16		; compute CRC on flash[0800:FFFD]

	movlw	low(FLASH_CRC_ADDR)	; verify if flash is valid
	movwf	TBLPTRL
	movlw	high(FLASH_CRC_ADDR)
	movwf	TBLPTRH
	clrf	TBLPTRU
	tblrd*+
	movf	TABLAT,w
	cpfseq	crc+1			; upper crc match ?
	bsf	flags,BF_FLASH_INVALID	; no. set invalid flag
	tblrd*+
	movf	TABLAT,w
	cpfseq	crc+0			; lower crc match ?
	bsf	flags,BF_FLASH_INVALID	; no. set invalid flag
	btfsc	flags,BF_FLASH_INVALID	; invalid flash
	bra	enter_bootloader	; yes => bootloader mode

	movlw	high(EEPROM_CRC_ADDR)	; verify if eeprom has valid checksum
	movwf	EEADRH
	movlw	low(EEPROM_CRC_ADDR)
	movwf	EEADR
	bcf	EECON1,EEPGD
	bcf	EECON1,CFGS
	bsf	EECON1,RD		; read eeprom
	movf	EEDATA,w
	cpfseq	crc+1			; upper crc match ?
	bsf	flags,BF_EEPROM_INVALID	; no. set invalid flag
	incf	EEADR,f
	bcf	EECON1,EEPGD
	bcf	EECON1,CFGS
	bsf	EECON1,RD		; read eeprom
	movf	EEDATA,w
	cpfseq	crc+0			; lower crc match ?
	bsf	flags,BF_EEPROM_INVALID	; no. set invalid flag
	
	btfsc	flags,BF_EEPROM_INVALID	; invalid eeprom flag
	bra	enter_bootloader
	
enter_app_mode
	setf	boot_mode
	goto 	APP_STARTUP		; not magic, jump to C
	
exit_bootloader
	movlw	high(EEPROM_CRC_ADDR)	; restore eeprom CRC
	movwf	EEADRH
	movlw	low(EEPROM_CRC_ADDR)
	movwf	EEADR
	movff	crc+1,EEDATA
	movlw	0x04			; eeprom write
	movwf	EECON1
	rcall	do_write
	incf	EEADR,f
	movff	crc+0,EEDATA
	movlw	0x04
	movwf	EECON1
	rcall	do_write	
	reset

enter_bootloader	
	bcf	STAT_CS_TRIS		; configure pins (minimal config)
	movlw	0x0F
	movwf	ADCON1
	bcf	TRS_WAIT
	bcf	TRS_WAIT_TRIS
	bsf	GAL_INT_IE
	bcf	GAL_INT_IP		; low priority in bootloader
	bcf	GAL_INT_EDGE
	bcf	GLED
	bcf	GLED_TRIS
	bcf	RLED
	bcf	RLED_TRIS
	bcf	SPI_SCK_TRIS
	bcf	SPI_MOSI_TRIS
	movlw	0x40
	movwf	SSPSTAT
	movlw	0x21
	movwf	SSPCON1
	movlw	0x87			; timer0, 1:256 prescale
	movwf	T0CON
	bcf	INTCON2,TMR0IP
	bsf	INTCON,TMR0IE
	movlw	.18			; about 30 seconds
	movwf	timeout			; (30 * 10e6 / 256 / 65536)
	bsf	RCON,IPEN
	
	bsf	GLED			; turn both LEDs on
	bsf	RLED
	
	clrf	addr+3			; init our variables
	clrf	addr+2
	lfsr	FSR2,BUF1
	
	bsf	INTCON,GIEL		; enable interrupts
	bsf	INTCON,GIEH

	movlw	TRS_BOOT_READY		; we are ready !
	rcall	update_status
	
b_main:
	btfsc	flags,BF_PROCESS	; work to do ?
	rcall	b_process		; yes, do it
	btfsc	flags,BF_FLASH_INVALID	; flash valid ?
	bra	b_main			; no.. stay in bootloader
	tstfsz	timeout			; yes. timeout ?
	bra	b_main			;      no
	bra	exit_bootloader		;      yes, exit bootloader

;
; process one line of the intel file
;	
b_process:
	bcf	flags,BF_PROCESS
	lfsr	FSR2,BUF1
					;
					; step 1 : confirm line starts with ':'
					;
	movf	POSTINC2,w		; get first char
	xorlw	':'			; must be ':'
	bnz	b_invalid_err
					;
					; step 2 : verify checksum
					;
	rcall	read_byte		; number of data bytes
	movwf	checksum		; init checksum
	addlw	0x4			; addr hi/lo, type, checksum
	movwf	count
bp_0:	rcall	read_byte		; read line and update checksum
	addwf	checksum,f
	decfsz	count
	bra	bp_0
	tstfsz	checksum		; checksum must be 0
	bra	b_checksum_err
					;
					; step 3 : evaluate line
					;
	lfsr	FSR2,BUF1+1
	rcall	read_byte		; number of data bytes
	movwf	count
	rcall	read_byte		; addr hi
	movwf	addr+1
	rcall	read_byte		; addr lo
	movwf	addr+0
	rcall	read_byte		; type
	movwf	type
	andlw	0x7
	call	jump_table
	goto	b_data			; 00 = regular data
	goto	b_end_file		; 01 = end of file
	goto	b_ignore		; 02 = ignore
	goto	b_ignore		; 03 = ignore
	goto	b_set_hi_addr		; 04 = set address upper 16 bits
	goto	b_ignore		; 05 = ignore
	goto	b_erase			; 06 = erase
	goto	b_ignore		; 07 = ignore
b_done:
	iorlw	TRS_BOOT_READY
	rcall	update_status		; update status
	lfsr	FSR2,BUF1		; get ready for next line
	return
	
b_ok:
	movlw	TRS_BOOT_OK
	bra	b_done

b_ignore:
	movlw	TRS_BOOT_IGNORED
	bra	b_done

b_checksum_err:
	movlw	TRS_BOOT_CHECKSUM_ERR
	bra	b_done

b_seq_err:
	bsf	flags,BF_IGNORE		; ignore everything
	movlw	TRS_BOOT_SEQ_ERR
	bra	b_done

b_range_err:
	movlw	TRS_BOOT_RANGE_ERR
	bra	b_done
	
b_invalid_err:
	movlw	TRS_BOOT_INVALID
	bra	b_done



;
; TYPE 00 : data
;
b_data:
	btfsc	flags,BF_IGNORE
	bra	b_ignore
	movf	addr+3,w		; check addr+3 : must be 0
	bnz	b_ignore
	movf	addr+2,w		; dispatch write based on addr+2
	bz	b_data_flash		; 00 => main flash
	rcall	b_last			; not 00 => make sure last block is done
	movf	addr+2,w
	xorlw	0xF0
	bz	b_data_eeprom		; F0 => eeprom
	bra	b_ignore		; everything else => ignored

; write some data in flash
b_data_flash:
	movf	addr+0,w		; compute TBLPTR - addr
	subwf	TBLPTRL,w
	movwf	tmp
	movf	addr+1,w
	subwfb	TBLPTRH,w
#if 0					; not needed for <= 64K PIC
	iorwf	tmp,f
	movf	addr+2,w
	subwfb	TBLPTRU,w
#endif
	bnc	b_fill_ff		; TBLPTR < load => fill with FF
	iorwf	tmp,f
	bnz	b_seq_err		; TBLPTR > load => sequence error

	rcall	read_byte		; get data byte
	decf	count,f
	movwf	TABLAT			; put in holding regs
	bsf	flags,BF_DIRTY
	rcall	inc_addr		; addr++
	bra	b_fill
b_fill_ff:
	setf	TABLAT
b_fill:	
	tblwt	*+
	movf	TBLPTRL,w
	andlw	(BLOCKSIZE-1)
	btfsc	STATUS,Z		; cross next block ?
	rcall	b_write_block
	tstfsz	count			; done with this line ?
	bra	b_data_flash		; no... loop
	bra	b_ok			; yes.. done


; write some data in eeprom
b_data_eeprom:	
	btfsc	EECON1,WR		; wait for previous write to complete
	bra	$-2
	movf	addr+2,w
	sublw	0xF0			; EEPROM is at 0x00F0.0000, 1024 bytes
	movwf	tmp			; temp save
	movf	addr+1,w
	andlw	0xFC			; upper 6 bits of addr+1 must be 0
	iorwf	tmp,w
	iorwf	addr+3,w		; addr+3 must be 0
	bnz	b_range_err
	
	movf	addr+1,w
	movwf	EEADRH
	movf	addr+0,w
	movwf	EEADR			; address to write
	rcall	read_byte		; get data byte
	decf	count,f
	movwf	EEDATA
	movlw	0x04			; eeprom write
	movwf	EECON1
	rcall	do_write
	rcall	inc_addr
	tstfsz	count
	bra	b_data_eeprom
	bra	b_ok
	

	
; commit eventual last block
b_last:
	btfss	flags,BF_DIRTY		; something to write ?
	return				; no.. return
	setf	TABLAT			; pad FF
bl0:	tblwt	*+
	movf	TBLPTRL,w
	andlw	(BLOCKSIZE-1)
	bnz	bl0
	rcall	b_write_block
	return
	

;
; write block subroutine
;
b_write_block:
	btfss	flags,BF_DIRTY		; any data to write ?
	bra	b_write_no_data
	bcf	flags,BF_DIRTY
	tblrd	*-			; point back in data block
	
	movlw	low (BOOTLOADER_END)	; check if its inside the bootloader
	subwf	TBLPTRL,w
	movlw	high (BOOTLOADER_END)
	subwfb	TBLPTRH,w
#if 0
	movlw	upper (BOOTLOADER_END+1)
	subwfb	TBLPTRU,w
#endif
	bnc	b_write_addr_bad	; protect bootloader (ignore this write)
b_write_addr_ok:
	bsf	flags,BF_FLASH_INVALID
	movlw	0x84			; flash write
	movwf	EECON1
	rcall	do_write
b_write_addr_bad:
	tblrd	*+			; restore pointer
b_write_no_data:
	clrf	EECON1			; inhibit writes
	return

;
; write magic sequence
;
do_write:
	bcf	INTCON,GIEH
	bcf	INTCON,GIEL
	movlw	0x55			; magic unlock sequence
	movwf	EECON2
	movlw	0xAA
	movwf	EECON2
	bsf	EECON1,WR
	bsf	INTCON,GIEL
	bsf	INTCON,GIEH
	nop				; write happens here
	btfsc	EECON1,WR		; wait for previous write to complete
	bra	$-2
	bcf	EECON1,WREN
	return


;
; inc address (24 bits)
;
inc_addr:
	clrf	WREG
	incf	addr+0,f		; addr++
	addwfc	addr+1,f
	addwfc	addr+2,f
	return


; 
; TYPE 04 : set upper 16 bits of address
;   address high :
;   0020 : ID location
;   0030 : CONFIG location
;   00F0 : EEPROM location
;
b_set_hi_addr:
	decf	count,w
	decfsz	WREG,w			; make sure block length is 2
	bra	b_invalid_err
	rcall	read_byte		; upper address hi
	movwf	addr+3
	rcall	read_byte		; upper address lo
	movwf	addr+2
	movlw	TRS_BOOT_OK
	bra	b_done
	
;
; Type 01 : end file
;
b_end_file:
	rcall	b_last		 	; commit last block, if needed
	reset				; reset!



;
; Type 06 - erase. Address must be FF0x (x=0 for flash, x=1 for eeprom)
;
b_erase:
	incf	addr+1,w		; addr+1 == FF ?
	bnz	b_invalid_err		; no.. invalid
	movf	addr,w
	bz	erase_flash
	bra	erase_eeprom

	
;
; erase flash
;
erase_flash:
	bsf	flags,BF_FLASH_INVALID
	movlw	low (BOOTLOADER_END+1)
	movwf	TBLPTRL
	movlw	high (BOOTLOADER_END+1)
	movwf	TBLPTRH
	clrf	TBLPTRU
ef0:	movlw	0x94			; flash erase
	movwf	EECON1
	rcall	do_write
	movlw	ERASE_BLOCKSIZE
	addwf	TBLPTRL,f
	clrf	WREG
	addwfc	TBLPTRH,f
	bnz	ef0			; this PIC is 64K
	bra	b_ok
	
;
; erase eeprom 000-3FF
;
erase_eeprom:
	setf	EEDATA
	clrf	EEADRH
	clrf	EEADR
ee0:	movlw	0x04
	movwf	EECON1
	rcall	do_write
	btfsc	EECON1,WR
	bra	$-2
	incfsz	EEADR,f			; addr++
	bra	ee0
	incf	EEADRH,f		; addrH++
	movf	EEADRH,w		; (incfsz doesn't work on simulation)
	bnz	ee0
	bra	b_ok

;;;
;;; jump table : modify return address by adding W*4 to it.
;;;
jump_table
	GLOBAL	jump_table
	rlncf	WREG,w
	rlncf	WREG,w
	addwf	TOSL,f
	movlw	0x00
	addwfc	TOSH,f
	;addwfc	TOSU,f			; if more than 64kB
	return


;;;
;;; Compute CRC16 of 0800..FFFF
;;;
do_crc16
	movlw	low (BOOTLOADER_END+1)
	movwf	TBLPTRL
	movlw	high (BOOTLOADER_END+1)
	movwf	TBLPTRH
	clrf	TBLPTRU
	clrf	crc+0
	clrf	crc+1
doc0	tblrd*+
	movf	TABLAT,w
	rcall	crc16
	movf	TBLPTRH,w		; stop when TBLPTR = 00FFFE
	sublw	high(FLASH_CRC_ADDR)
	bnz	doc0
	movf	TBLPTRL,w
	sublw	low(FLASH_CRC_ADDR)
	bnz	doc0
	return
	


;;;
;;; CRC-16  (x^16+x^15+x^2+x^0)
;;;
crc16
	xorwf	crc+0,w		; W = input xor old_crc+0
	xorwf	crc+1,w		; swap old_crc+1 with w
	xorwf	crc+1,f
	xorwf	crc+1,w		; new crc+1 = input xor old_crc+0
	movwf	crc+0		; new crc+0 = old_crc+1
	
	movf	crc+1,w		; save crc+1 in W
	swapf	crc+1,f		; trade nibbles
	xorwf	crc+1,f		; XOR high half byte with low
	rrcf	crc+1,f		; initialize carry
	btfsc	crc+1,0
	btg	STATUS,C	; compliment carry
	btfsc	crc+1,1
	btg	STATUS,C	; compliment carry
	btfsc	crc+1,2
	btg	STATUS,C	; compliment carry
	movwf	crc+1		; restore crc+1 from W
	
	movlw	0x1
	btfsc	STATUS,C	; if carry
	xorwf	crc+0,f		; flip bit 0 of crc+0
	movlw	0x40
	rrcf	crc+1,f		; shift parity into crc+1
	btfsc	STATUS,C	; if shift out is one
	xorwf	crc+0,f		; flip bit 6 of crc+0
	rlcf	crc+1,w		; unshift crc+1 into W
	xorwf	crc+1,f		; combine them
	rrcf	crc+1,f		; shift parity back into crc+1
	movlw	0x80
	btfsc	STATUS,C	; if shift out is one
	xorwf	crc+0,f		; flip bit 7 of crc+0
	
	retlw	0



crc16_val CODE_PACK FLASH_CRC_ADDR
	db	high(THECRC), low(THECRC)

exit_boot CODE_PACK (0xF00000 | EEPROM_CRC_ADDR)
	db	high(THECRC), low(THECRC)
	
	END
	
