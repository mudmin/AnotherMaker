;;;
;;; TIME
;;;

NEWDOS_VALID_BYTE  equ 0a5h
;; Model 1
NEWDOS_VALID_ADDR  equ 43abh
NEWDOS_MONTH       equ 43b1h
NEWDOS_DAY         equ 43b0h
NEWDOS_YEAR        equ 43afh
NEWDOS_HOUR        equ 43aeh
NEWDOS_MIN         equ 43adh
NEWDOS_SEC         equ 43ach
;; Model 3
NEWDOS3_VALID_ADDR equ 42cbh
NEWDOS3_MONTH      equ 42d1h
NEWDOS3_DAY        equ 42d0h
NEWDOS3_YEAR       equ 42cfh
NEWDOS3_HOUR       equ 42ceh
NEWDOS3_MIN        equ 42cdh
NEWDOS3_SEC        equ 42cch
;; LDOS (doesn't store time)
LDOS_MONTH         equ 4306h
LDOS_DAY           equ 4307h
LDOS_YEAR          equ 4466h
LDOS3_MONTH        equ 442fh
LDOS3_DAY          equ 4457h
LDOS3_YEAR         equ 4413h
LDOS4_MONTH        equ 0035h
LDOS4_DAY          equ 0034h
LDOS4_YEAR         equ 0033h


hack_time:
	call	get_datetime
	ld	a,(ROM_MODEL)
	cp	ROM_MODEL_1
	jr	nz,+
	call	time_newdos_m1		; model 1
	call	time_ldos5_m1
	ret
+	call	time_newdos_m3		; model 3, 4, 4P
	call	time_ldos5_m3
	ld	a,(ROM_MODEL)
	cp	ROM_MODEL_4
	ret	c
	call	time_ldos6		; model 4, 4P
	ret
	
get_datetime:
	ld	a,GET_TIME
	out	(COMMAND2),a
	call	wait
	ld	hl,BUF1
	ld	bc,6<<8|DATA2
	inir
	ret
	
time_newdos_m1:
	ld	hl,NEWDOS_VALID_ADDR
	ld	de,NEWDOS_SEC
	jr	+

time_newdos_m3:	
	ld	hl,NEWDOS3_VALID_ADDR
	ld	de,NEWDOS3_SEC
+	ld	(hl),NEWDOS_VALID_BYTE
	ld	hl,BUF1
	ld	bc,6
	ldir
	ret

time_ldos5_m1:
	ld	hl,BUF1+FREHD_YEAR	; year
	ld	de,LDOS_YEAR
	ld	a,(hl)
	sub	80
	ld	(de),a
	inc	hl			; day
	ld	de,LDOS_DAY
	ld	a,(hl)
	ld	(de),a
	inc	hl			; month
	ld	de,LDOS_MONTH
	ld	a,(hl)
	xor	50h
	ld	(de),a
	ret

time_ldos5_m3:	
	ld	hl,BUF1+FREHD_YEAR	; year
	ld	de,LDOS3_YEAR
	ld	a,(hl)
	sub	80
	ld	(de),a
	inc	hl			; day
	ld	de,LDOS3_DAY
	ld	a,(hl)
	ld	(de),a
	inc	hl			; month
	ld	de,LDOS3_MONTH
	ld	a,(hl)
	xor	50h
	ld	(de),a
	ret

time_ldos6:
	ld	a,2			; map RAM
	out	(84h),a
	ld	hl,BUF1+FREHD_YEAR
	ld	de,LDOS4_YEAR
	ld	bc,3	
	ldir
	xor	a
	out	(84h),a			; unmap ROM
	ret
