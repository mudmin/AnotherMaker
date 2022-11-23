
	CPU z80

	INCLUDE ../frehd.inc
	BINCLUDE model4.gen2.rom

;;; update 13-nov-13, db	
;;;
;;; Need to find some free space in the ROM. For now, remove the support
;;; for slow_tape_write.
;;;

	ORG 3000h
	jp	329bh			; replace slow_tape_write by fast

;;3464	call 01c9h		; clear_screem
;;3467	call 028dh		; check break
;;346a	jp   nz,33f9h	; non_disk
;;346d	in   a,(f0h)

	ORG 346ah
	call	hd_boot1

	;; This was cas_write_8bits_slow
	ORG 3241h 
hd_boot1:	
	jp	nz, 33f9h		; jump if <break> pressed
	ld	a,ROM_MODEL_4	; rom parameter to FreHD. 4 for now.
	out	(WRITEROM),a	; 2
	in	a,(READROM)		; 2
	cp	0FEh			; FE is FreHD present and valid code!
	ret	nz				; return if no FreHD or old firmware
	ld	hl,ROM_LOAD		; load more "rom" from FreHD
	jr	hd_boot2

	IF $ >= 3254h
	ERROR "Code too big"
	ENDIF
	

	;; This was slow_tape_write
	ORG 325Eh 
hd_boot2:
	ld	bc,READROM
	inir
	jp	ROM_LOAD

	IF $ >= 3274h
	ERROR "Code too big"
	ENDIF


	;; Make EXT I/O enabled
	ORG 3724h

	db	38h
