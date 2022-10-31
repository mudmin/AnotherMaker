
	CPU z80

	INCLUDE ../frehd.inc
	BINCLUDE model4.rom

	
;;;
;;; Need to find some free space in the ROM. For now, remove the support
;;; for slow_tape_write.
;;;

	ORG 3000h
	jp	329bh			; replace slow_tape_write by fast

;; 343F call clear_screen
;; 3442 call check_break
;; 3445 jr   nz,non_disk
;; 3448 in   a,(F0h)

	ORG 3445h
	call	hd_boot1

	;; This was cas_write_8bits_slow
	ORG 3241h
hd_boot1:	
	jp	nz, 3105h		; jump if <break> pressed
	ld	a,ROM_MODEL_4		; rom parameter to FreHD. 4 for now.
	out	(WRITEROM),a		; 2
	in	a,(READROM)		; 2
	cp	0FEh			; FE is FreHD present and valid code!
	ret	nz			; return if no FreHD or old firmware
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
