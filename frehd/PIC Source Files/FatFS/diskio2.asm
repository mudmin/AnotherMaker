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
	include	p18f4620.inc
	include bootloader.inc

	;; extern udata
	extern	crc7,crc16,foo
	

diskio_asm_code	CODE

mmc_crc7
	GLOBAL	mmc_crc7
	
	movf	POSTDEC1,w		; FSR1--
	movlw	.8
	movwf	foo
mc0	bcf	STATUS,C
	rlcf	crc7
	btfss	INDF1,7
	bra	mc1
	movlw	0x09
	xorwf	crc7,f
mc1	btfss	crc7,7
	bra	mc2
	movlw	0x09
	xorwf	crc7
mc2	rlncf	INDF1
	decfsz	foo,f
	bra	mc0
	movf	POSTINC1,w		; FSR1++
	return


;;;
;;; CRC-16  (x^16+x^12+x^5+x^0)
;;;
mmc_crc16
	GLOBAL	mmc_crc16
	
	movlw	0xFF
	movf	PLUSW1,w		; W = 1st param
	xorwf	crc16+1,w
	movwf	foo
	andlw	0xf0
	swapf	foo,f
	xorwf	foo,f
	movf	foo,w
	andlw	0xf0
	xorwf	crc16+0,w
	movwf	crc16+1
	rlncf	foo,w
	xorwf	crc16+1,f
	andlw	0xe0
	xorwf	crc16+1,f
	swapf	foo,f
	xorwf	foo,w
	movwf	crc16+0
	
	retlw	0

	END
	