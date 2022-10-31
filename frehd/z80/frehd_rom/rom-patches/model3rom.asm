
	CPU z80

	include ../frehd.inc
	
;;;
;;; Fred's MODEL 3 ROM C :
;;;
;;;   SCM91692P
;;;  Tandy (c) 80
;;;    8040316B
;;;    (mot) QQ8115          (motorolla sign)
;;;

	BINCLUDE model3.rom
	

;; ROM:3489		 call	 clear_screen		   ; clear screen
;; ROM:348C		 call	 check_break		   ; check <break>
;; ROM:348F		 jp	 nz, non_disk		   ; non-disk if pressed

	ORG 348Fh
	call	hd_boot1

	
	

	
;; ROM:338E keyboard_in:					   ; CODE XREF:	cas_off+18j
;; ROM:338E		 call	 check_computer_version	   ; lower case	installed ?
;; ROM:3391		 jr	 nz, loc_33A3		   ; skip if not
;; ROM:3393		 ld	 bc, 3880h		   ; BC	=> shift keys
;; ROM:3396		 ld	 hl, 4018h		   ; HL	=> flag	byte
;; ROM:3399		 ld	 a, (bc)		   ; get image of keys
;; ROM:339A		 and	 2			   ; mask bit 1	(right shift)
;; ROM:339C		 ld	 e, a			   ; save masked image
;; ROM:339D		 xor	 (hl)			   ; toggle against old	image
;; ROM:339E		 ld	 (hl), e		   ; save new image
;; ROM:339F		 and	 e			   ; mask bit 1
;; ROM:33A0		 jp	 nz, loc_30BD		   ; jump if right shift pressed
	
	ORG 338Eh
keyboard_in:
	jr	+
			;; 19 bytes free here !
hd_boot2:
	ld	a,ROM_MODEL_3
	out	(WRITEROM),a
	in	a,(READROM)
	cp	0FEh
	ret	nz
	ld	hl,ROM_LOAD
	ld	bc,READROM
	jp	hd_boot3
	nop
+
	IF $ <> 33A3h
	ERROR "Must be at 33A3h"
	ENDIF


;; ROM:3411
;; ROM:3411 loc_3411:					   ; CODE XREF:	cas_off+409j
;; ROM:3411		 rrca				   ; D = 8 * row# + key#
;; ROM:3412		 jr	 c, loc_3417
;; ROM:3414		 inc	 d
;; ROM:3415		 jr	 loc_3411
;; ROM:3417 ; ---------------------------------------------------------------------------
;; ROM:3417
;; ROM:3417 loc_3417:					   ; CODE XREF:	cas_off+406j
;; ROM:3417		 call	 check_computer_version	   ; dual shifts ?
;; ROM:341A		 ld	 a, (3880h)		   ; get shift(s)
;; ROM:341D		 jr	 nz, loc_3421		   ; skip if dual shifts
;; ROM:341F		 and	 1			   ; mask for shifts
;; ROM:3421
;; ROM:3421 loc_3421:					   ; CODE XREF:	cas_off+411j
;; ROM:3421		 and	 3			   ; mask for shifts
;; ROM:3423		 jr	 z, loc_3427		   ; skip if no	shifts
;; ROM:3425		 set	 6, d			   ; offset D for shifts
;; ROM:3427

	ORG 3411h

loc_3411:	
	rrca
	jr	c,+
	inc	d
	jr	loc_3411
				;; 7 bytes free here !
hd_boot3:
	inir
	jp	ROM_LOAD
	nop
	nop

+	ld	a,(3880h)

	IF $ <> 3421h
	ERROR "Must be at 3421h"
	ENDIF



;; ROM:342F loc_342F:					   ; CODE XREF:	cas_off+41Fj
;; ROM:342F		 ld	 hl, 3045h		   ; HL	=> keyboard tables
;; ROM:3432		 ld	 e, d			   ; DE	=> offset
;; ROM:3433		 ld	 d, 0
;; ROM:3435		 add	 hl, de			   ; HL	= char position
;; ROM:3436		 ld	 a, (hl)		   ; get character
;; ROM:3437		 cp	 1Ah			   ; shift down	arrow ?
;; ROM:3439		 jp	 z, loc_30A1		   ; NULL exit if so
;; ROM:343C		 ld	 b, a			   ; save char in B
;; ROM:343D		 call	 check_computer_version
;; ROM:3440		 ld	 a, b			   ; restore char
;; ROM:3441		 jr	 z, loc_3447
;; ROM:3443		 or	 a			   ; NULL char ?
;; ROM:3444		 jp	 z, loc_30BD		   ; toggle caps lock if so
;; ROM:3447
;; ROM:3447 loc_3447:					   ; CODE XREF:	cas_off+435j
;; ROM:3447		 ld	 hl, 4224h		   ; HL	=> "control" flag
;; ROM:344A		 cp	 '*'                       ; key = '*' ?
;; ROM:344C		 jr	 nz, loc_3452		   ; skip if not
;; ROM:344E		 ld	 a, 1Fh			   ; control flag set
;; ROM:3450		 cp	 (hl)
;; ROM:3451		 ld	 a, b			   ; restore char
;; ROM:3452 loc_3452:					   ; CODE XREF:	cas_off+440j
;; ROM:3452		 jp	 loc_30FD

	ORG 343Ch

	ld	b,a
	or	a
	jp	z,30BDh
	ld	hl,4224h
	cp	'*'
	jr	nz,+
	ld	a, 1Fh
	cp	(hl)
	ld	a,b
+	jp	30FDh

hd_boot1:
	jp	nz,37AFh		; non-disk
	jp	hd_boot2
	
	IF $ <> 3455h
	ERROR "Must be at 3455h"
	ENDIF
	

	;; Make EXT I/O enabled
	ORG 3724h

	db	38h
