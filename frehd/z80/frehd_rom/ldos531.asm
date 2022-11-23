	LISTING ON
	NEWPAGE
;*=*=*
;	LDOS 5.3.1  BOOT/SYS PATCH
;*=*=*

ldos531:

	START_PATCH 7F00h
;*=*=*
;	routine to read a sector from HD
;	D = cylinder  E = sector
;*=*=*
read_sector_DE
	push	hl
	push	de
	ld	c,e
	ld	e,d
	ld	d,00h
	ld	b,0ch			; not needed (4P like)
	ld	a,02h			; not needed (4P like)
	call	hd_read_sector
	pop	de
	pop	hl
	ld	a,(4709h)
	sub	d
	ld	a,06h
	jr	z,+
	xor	a
+	and	a
	ret
	
;*=*=*
;	DCT handler, only read from HD
;*=*=*
dct_read_hd
	ld	a,09h			; command READ SECTOR ?
	sub	b
	jr	z,read_sector_DE	; yes, do it !
	xor	a			; no, return ok.
	ret

	END_PATCH


	START_PATCH 4709h
	db	0			; force directory to 0 for now
	END_PATCH
	

;; 4300  00           nop
;; 4301  fe 65        cp	65h
;; 4303  f3           di
;; 4304  3a 02 43     ld	a,(4302h)	; directory cylinder
;; 4307  57           ld	d,a
;; 4308  1e 04        ld	e,04h		; sector 4
	START_PATCH 430ah
	call	pp1
;; 430a  cd 74 43     call	4374h		; read it !
	END_PATCH
;; 430d  20 71        jr	nz,4380h
;; 430f  3a 00 51     ld	a,(5100h)	; test if system disk
;; 4312  e6 10        and	10h
;; 4314  28 6e        jr	z,4384h		; "no system" if z
;; 
;; 4316  d9           exx
;; 4317  2a 16 51     ld	hl,(5116h)
;; 431a  55           ld	d,l
;; 431b  7c           ld	a,h
;; 431c  07           rlca
;; 431d  07           rlca
;; 431e  07           rlca
;; 431f  e6 07        and	07h
;; 4321  67           ld	h,a
;; 4322  07           rlca
;; 4323  84           add	a,h
;; 4324  07           rlca
;; 4325  5f           ld	e,a
;; 
;; 	;;
;; 	;; Prepare to load SYS0/SYS
;; 	;; 
;; 4326  21 ff 51     ld	hl,51ffh
;; 4329  d9           exx
;; 
;; 	;;
;; 	;; LOAD
;; 	;; 
;; 432a  cd 5d 43     call	435dh		; get type code
;; 432d  3d           dec	a
;; 432e  20 0c        jr	nz,433ch	; bypass if not type 1
;; 4330  cd 4e 43     call	434eh		; get address
;; 4333  cd 5d 43     call	435dh
;; 4336  77           ld	(hl),a
;; 4337  23           inc	hl
;; 4338  10 f9        djnz	4333h
;; 433a  18 ee        jr	432ah
;; 
;; 433c  3d           dec	a		; test if type 2 (TRAADR)
;; 433d  28 0b        jr	z,434ah		; ah, go if transfer addr
;; 433f  cd 5d 43     call	435dh		; assume comment
;; 4342  47           ld	b,a		;   get comment length
;; 4343  cd 5d 43     call	435dh		;   and ignore it
;; 4346  10 fb        djnz	4343h
;; 4348  18 e0        jr	432ah		; continue to read
;;
	START_PATCH 434ah
	jr	pp2
	nop
;; 434a  cd 4e 43     call	434eh		; get addr
	END_PATCH
;; 434d  e9           jp	(hl)		; and jump there
;; 
;; 	;; getadr
;; 434e  cd 5d 43     call	435dh		; get block length
;; 4351  47           ld	b,a
;; 4352  cd 5d 43     call	435dh		; get low-order load addr
;; 4355  6f           ld	l,a
;; 4356  05           dec	b		; adj length for this byte
;; 4357  cd 5d 43     call	435dh		; get high-order load addr
;; 435a  67           ld	h,a
;; 435b  05           dec	b		; adj length for this byte
;; 435c  c9           ret
;; 
;; 	;; routine to read a byte
;; 435d  d9           exx			; switch memory/buff pointers
;; 435e  2c           inc	l		; bump up buf pointer
;; 435f  20 0f        jr	nz,4370h	; bypass disk i/o if more
;; 4361  e5           push	hl
;; 4362  cd 74 43     call	4374h		; read another sector
;; 4365  20 19        jr	nz,4380h	; jump if error
;; 4367  e1           pop	hl
;; 4368  1c           inc	e		; bump sector counter
;; 4369  7b           ld	a,e
;; 436a  d6 12        sub	12h		; last sector       [!!! 12h = sec/track]
;; 436c  20 02        jr	nz,4370h	;   on this cylinder ?
;; 436e  5f           ld	e,a		; yes, restart at 0
;; 436f  14           inc	d		;     and increment cylinder
;; 4370  7e           ld	a,(hl)		; get a byte
;; 4371  d9           exx			; exchange pointers back
;; 4372  c9           ret
;; 	
;; 4373  c7           rst	0
;; 	
;; 4374  06 05        ld	b,05h		; retry 5 times
;; 4376  c5           push	bc
;; 4377  cd 8e 43     call	438eh		; read sector
;; 437a  c1           pop	bc
;; 437b  e6 1c        and	1ch
;; 437d  c8           ret	z		; return if no error
;; 437e  10 f6        djnz	4376h		; try again
;; 4380  21 e6 43     ld	hl,43e6h	; disk error
;; 4383  dd
;; 4384  21 f4 43 	   ld	hl,43f4h	; no system
;; 4387  cd 1b 02     call	021bh		; display text
;; 438a  cd 40 00     call	0040h		; input text from keyboard
;; 438d  c7           rst	0		; reboot
;;
	START_PATCH 438eh
	jp	read_sector_DE
	
pp1	ld	hl,5100h
	call	4374h
	ret
	
pp2	call	434eh
	push	hl			; save SYS0 entry point
	ld	hl,pdct			; load DCT
	ld	de,4701h
	ld	bc,0008h
	ldir
	ld	hl,sys0_531		; patch !
	call	patch
	pop	hl			; restore entry point
	jp	(hl)			; and jump

pdct	dw	dct_read_hd
	db	0ch, 90h, 00h, 98h, 1fh, 0e3h


;; 438e  01 f4 81     ld	bc,81f4h	; Set DDEN,DS1,d.s.port
;; 4391  ed 41        out	(c),b		; Select it
;; 4393  0d           dec	c		; Point C to data register
;; 4394  3e 1b        ld	a,1bh		; 
;; 4396  ed 51        out	(c),d		; Set desired track
;; 4398  cd df 43     call	43dfh		; Pass command and delay
;; 439b  db f0        in	a,(f0h)		; Get status
;; 439d  cb 47        bit	0,a		; Busy ?
;; 439f  20 fa        jr	nz,439bh
;; 43a1  7b           ld	a,e		; Set sector register
;; 43a2  d3 f2        out	(f2h),a
;; 43a4  3e 81        ld	a,81h		; Set DDEN and DS1
	END_PATCH
;; 43a6  d3 f4        out	(f4h),a
;; 43a8  21 d3 43     ld	hl,43d3h	; install NMI vector
;; 43ab  22 4a 40     ld	(404ah),hl
;; 43ae  3e c3        ld	a,c3h
;; 43b0  32 49 40     ld	(4049h),a
;; 43b3  21 00 51     ld	hl,5100h
;; 43b6  d5           push	de
;; 43b7  11 02 c1     ld	de,c102h	; D=DS1 + DDEN + W.S.Gen
;; 43ba  3e 80        ld	a,80h		; FDC READ command
;; 43bc  cd df 43     call	43dfh		; Pass to ctrlr & set B=0
;; 43bf  3e c0        ld	a,c0h		; Enable INTRQ & timeout
;; 43c1  d3 e4        out	(e4h),a
;; 43c3  db f0        in	a,(f0h)		; Grab status
;; 43c5  a3           and	e		; Test bit 1
;; 43c6  28 fb        jr	z,43c3h
;; 43c8  ed a2        ini
;; 43ca  7a           ld	a,d		; Set DDEN & DS1 & WSGEN
;; 43cb  d3 f4        out	(f4h),a		; Continue to select
;; 43cd  ed a2        ini			;    while inputting
;; 43cf  20 fa        jr	nz,43cbh
;; 43d1  18 fe        jr	43d1h		; Wait for NMI
;; 43d3  e1           pop	hl		; Pop interrupt ret
;; 43d4  d1           pop	de		; restore DE
;; 43d5  af           xor	a		; Disable INTRQ and timetout
;; 43d6  d3 e4        out	(e4h),a
;; 43d8  3e 81        ld	a,81h		; Reselect drive
;; 43da  d3 f4        out	(f4h),a
;; 43dc  db f0        in	a,(f0h)		; Get status
;; 43de  c9           ret
;; 	
;; 43df  d3 f0        out	(f0h),a		; Give command to controller
;; 43e1  06 0c        ld	b,0ch		; Time delay
;; 43e3  10 fe        djnz	43e3h
;; 43e5  c9           ret
;; 
;; 43e6  db	17h,0e8h,'Disk error',1Fh,03h
;; 
;; 43f4  db	17h,0e8h, 'No system',03h

	LAST_PATCH


sys0_531:
	START_PATCH 4e68h
	ld	a,(4302h)
	ld	(4709h),a
	jr	4e90h
	END_PATCH

	START_PATCH 4049h
	db	0c3h
	END_PATCH

	LAST_PATCH
	
