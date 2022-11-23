
	CPU z80

;;;
;;; Fred's AZERTY ROM C :
;;;
;;;   SCM90899P
;;;    8049003
;;;  Tandy (c) 1983
;;;    (mot) 8335          (motorolla sign)
;;;

	BINCLUDE model4french.rom
	INCLUDE ../frehd.inc

	

;;;
;;; Need to find some free space in the ROM. For now, remove the support
;;; for slow_tape_write.
;;;

	ORG 3000h
	jp	329bh			; replace slow_tape_write by fast



;; 346F  call clear_screen
;; 3472  call check_break
;; 3475  jp   nz,non_disk
;; 3478  in   a,(F0h)
	
	ORG 3475h
	call	hd_boot1
	

	;; This was cas_write_8bits_slow
	ORG 3241h
hd_boot1:	
	jp	nz, 33F9h		; jump if <break> pressed
	ld	a,ROM_MODEL_4A		; rom parameter to FreHD. 0 for now.
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

	db	30h
	
