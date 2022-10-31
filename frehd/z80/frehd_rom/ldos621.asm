	LISTING ON
	NEWPAGE


;*=*=*
;	LDOS 6.3.1  FreHD patch
;
; This patch is applied automatically when FreHD loads LDOS 6.3.1.
; The patching occurs in 3 phases :
; 1- frehd.rom loads the boot sector at adress 4300h. This area is patched
;    first. The boot sector code will load 16 sectors at address 200h, and
;    this area also needs to be patched, so a call to the 'patch' routine
;    is installed.
; 2- The 2nd patch is applied. It intercepts the call to the entry address
;    of SYS0/SYS which is loaded by the code at 200h. The final patch is
;    installed
;*=*=*



;*=*=*
;	LDOS 6.3.1  BOOT/SYS patch
;*=*=*
ldos621:

	START_PATCH 219, 6A00h
read621_sector_DE
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
	ld	a,d
	sub	(iy+9)
	jr	nz,+
	ld	a,06h
	or	a
	ret
+	xor	a
	ret
;*=*=*
;	DCT handler, only read from HD
;*=*=*
dct621_read_hd
	ld	a,09h			; command READ SECTOR ?
	sub	b
	jr	z,read_sector_DE	; yes, do it !
	xor	a			; no, return ok.
	ret

	END_PATCH 219



	
;; 4300  00           nop
	START_PATCH 201, 4302h
	db	65h
;; 4301  fe 65        cp	65h		;# directory track location
	END_PATCH 201
;; 4303  f3           di
;; 4304  3e 86        ld	a,86h		; 80 col, mode 2
;; 4306  d3 84        out	(84h),a
;; 4308  32 78 00     ld	(0078h),a	; remember this mode
;; 430b  21 00 f8     ld	hl,f800h	; clear video ram
;; 430e  11 01 f8     ld	de,f801h
;; 4311  01 7f 07     ld	bc,077fh
;; 4314  36 20        ld	(hl),20h
;; 4316  ed b0        ldir
;; 4318  21 cd 43     ld	hl,43cdh	; set NMI vector
;; 431b  22 67 00     ld	(0067h),hl
;; 431e  3e c3        ld	a,c3h
;; 4320  32 66 00     ld	(0066h),a
;; 4323  3e c9        ld	a,c9h		; stuff return for ints
;; 4325  32 38 00     ld	(0038h),a
;; 
;; 	;; read the first 16 sectors of track 0
;; 
;; 4328  21 00 02     ld	hl,0200h	; ptr to page 2
;; 432b  55           ld	d,l		; init to track 0, sec 0
;; 432c  5d           ld	e,l
;; 432d  cd 77 43     call	4377h		; read a sector
;; 4330  24           inc	h		; bump to next page
;; 4331  1c           inc	e		; bump to next sector
;; 4332  3e 10        ld	a,10h
;; 4334  bb           cp	e		; loop if more
;; 4335  20 f6        jr	nz,432dh
;; 4337  cd dd 02     call	02ddh		; init the CRTC
;; 
;; 	;; set up to load SYSRES
;; 
;; 433a  3a 02 43     ld	a,(4302h)	; pick up DIR cylinder
;; 433d  32 79 04     ld	(0479h),a	; update dct to show dir
;; 4340  57           ld	d,a		; set starting track
	START_PATCH 202, 4341h
	jr	ap2_2
ap2_1	dw	dct621_read_hd				; our DCT handler
	db	0ch, 90h, 00h, 98h, 1fh, 0e3h	; DCT data
	nop
	nop
	nop
	nop
ap2_2	nop
;; 4341  1e 00        ld	e,00h
;; 4343  cd 74 43     call	4374h
;; 4346  3a cd 12     ld	a,(12cdh)
;; 4349  e6 20        and	20h
;; 434b  21 74 04     ld	hl,0474h
;; 434e  b6           or	(hl)
;; 434f  77           ld	(hl),a
	END_PATCH 202
;; 4350  1e 04        ld	e,04h
;; 4352  cd 74 43     call	4374h
;; 4355  3a 00 12     ld	a,(1200h)
;; 4358  e6 10        and	10h
;; 435a  28 2d        jr	z,4389h
;; 435c  21 1d 12     ld	hl,121dh
;; 435f  11 f6 43     ld	de,43f6h
;; 4362  01 08 00     ld	bc,0008h
;; 4365  ed b8        lddr
;; 4367  d5           push	de
;; 4368  dd e1        pop	ix
;; 436a  d9           exx
;; 436b  21 ff 12     ld	hl,12ffh
;; 436e  d9           exx
	START_PATCH 203, 436fh
	jr	ap5_1
	nop
;; 436f  c3 38 02     jp	0238h
	END_PATCH 203
;; 4372  00           nop
;; 4373  00           nop
;; 4374  21 00 12     ld	hl,1200h
;;
;; 	;; routine to read a sector
;;	
;; 4374  21 00 12     ld	hl,1200h	; set buffer
;; 4377  06 05        ld	b,05h		; init retry counter
;; 4379  c5           push	bc		; save counter
;; 437a  e5           push	hl		; save for retries
	START_PATCH 204, 437bh
	call	ap5_read
	pop	hl
	pop	bc
	nop
	nop
;; 437b  cd 96 43     call	4396h
;; 437e  e1           pop	hl
;; 437f  c1           pop	bc
;; 4380  e6 1c        and	1ch
	END_PATCH 204
;; 4382  c8           ret	z
;; 4383  10 f4        djnz	4379h		; loop for retry
;;	
;; 4385  21 e0 43     ld	hl,43e0h	; "disk error"
;; 4388  dd				; [trick to hide next instruction]
;; 4389  21 ea 43     ld	hl,43eah	; "no system"
;; 438c  01 0a 00     ld	bc,000ah
;; 438f  11 93 fb     ld	de,fb93h	; middle of screen
;; 4392  ed b0        ldir
;; 4394  18 fe        jr	4394h		; wait for RESET
	START_PATCH 205, 4396h
ap5_1	ld	hl,ap2_1			; copy DCT data to DCT drive 0
	ld	de,0471h
	ld	bc,0008h
	ldir
	nop
	ld	hl,aloader		; patch SYS0/SYS which just got loaded
	call	patch
	jp	0238h			; jump into SYS0
ap5_2
	;di
	nop
	call	ap5_read
	;ei
	nop
	ld	a,05h			; error 5 if HD read failed
	ret	nz
	ld	a,(0479h)		; DIR location ?
	sub	d
	ld	a,06h
	jr	z,+			; yes, return A = 6
	xor	a			; no, return A = 0
+	and	a			; update Z flag
	ret
	nop
	nop
	nop
	nop
	nop
ap5_read
	push	hl
	push	de
	nop
	nop
	nop
	nop
	nop
	nop
	ld	c,e			; C = sector
	ld	e,d
	ld	d,00h			; DE = cyl nymber
	ld	b,0ch			; B = 0C (read)
	ld	a,02h			; A = 2 (hard disk)
	call	hd_read_sector		; read
	pop	de
	pop	hl
	ret
;; 4396  01 f4 81     ld	bc,81f4h	; Set DDEN,DS1,d.s.port
;; 4399  ed 41        out	(c),b		; Select it
;; 439b  0d           dec	c		; Point C to data register
;; 439c  3e 18        ld	a,18h		; Seek command (6 msec)
;; 439e  ed 51        out	(c),d		; Set desired track
;; 43a0  cd d9 43     call	43d9h		; Pass command and delay
;; 43a3  db f0        in	a,(f0h)		; Get status
;; 43a5  cb 47        bit	0,a		; Busy ?
;; 43a7  20 fa        jr	nz,43a3h
;; 43a9  7b           ld	a,e		; Set sector register
;; 43aa  d3 f2        out	(f2h),a
;; 43ac  3e 81        ld	a,81h		; Set DDEN and DS1
;; 43ae  d3 f4        out	(f4h),a
;; 43b0  d5           push	de
;; 43b1  11 02 c1     ld	de,c102h	; D=DS1 + DDEN + W.S.Gen
;; 43b4  3e 80        ld	a,80h		; FDC READ command
;; 43b6  cd d9 43     call	43d9h		; Pass to ctrlr & set B=0
;; 43b9  3e c0        ld	a,c0h		; Enable INTRQ & timeout
;; 43bb  d3 e4        out	(e4h),a
;; 43bd  db f0        in	a,(f0h)		; Grab status
;; 43bf  a3           and	e		; test bit 1
;; 43c0  28 fb        jr	z,43bdh
;; 43c2  ed a2        ini
;; 43c4  7a           ld	a,d		; Set DDEN & DS1 & WSGEN
;; 43c5  d3 f4        out	(f4h),a		; Continue to select
;; 43c7  ed a2        ini			;   while inputting
;; 43c9  20 fa        jr	nz,43c5h
;; 43cb  18 fe        jr	43cbh		; Wait for NMI
;; 43cd  d1           pop	de		; Pop interrupt ret
;; 43ce  d1           pop	de		; restore DE
;; 43cf  af           xor	a		; Disable INTRQ and timeout
;; 43d0  d3 e4        out	(e4h),a
;; 43d2  3e 81        ld	a,81h		; Reselect drive
;; 43d4  d3 f4        out	(f4h),a
;; 43d6  db f0        in	a,(f0h)		; Get status
;; 43d8  c9           ret
;; 43d9  d3 f0        out	(f0h),a		; Give command to controller
;; 43db  06 18        ld	b,18h		; Time delay
;; 43dd  10 fe        djnz	43ddh
;; 43df  c9           ret
	END_PATCH 205
;; 43e0               db 'Disk error'
;; 43ea               db 'No system '
;; 43f4  00           nop
;; 43f5  00           nop
;; 43f6  00           nop
;; 43f7  00           nop
;; 43f8  00           nop
	START_PATCH 206, 43f9h
ap6	ld	a,09h
	sub	b
	jr	z,ap5_2
	xor	a
	ret
;; 43f9  00           nop
;; 43fa  00           nop
;; 43fb  00           nop
;; 43fc  00           nop
;; 43fd  00           nop
;; 43fe  00           nop
;; 43ff  00           nop
	END_PATCH 206

	LAST_PATCH


;*=*=*
;	Loader patch
;*=*=*

;;                00139 ;       Set up for a fragmented file
;;                00140 ;
;; 0258  D9       00141         EXX
;; 0259  0E06     00142         LD      C,6             ;Sectors/gran
;; 025B  CDB102   00143         CALL    GETEXT          ;Pick up extent 1
;; 025E  D9       00144         EXX
;;                00145 ;
;; 025F  CD6802   00146         CALL    LOAD            ;Read in SYSRES
;; 0262  3EFB     00147         LD      A,0FBH          ;EI instruction
;; 0264  32950F   00148         LD      (DISKEI),A      ;  stuffed into FDCDVR
;; 0267  E9       00149         JP      (HL)            ;Continue system init
;;                00150 ;

aloader:
	START_PATCH 2020, 264h
	jp	5F00h
	END_PATCH 2020

	START_PATCH 2021, 5F00h
	ld	(0f95h),a
	push	hl
	ld	hl,asys0sys
	call	patch
	pop	hl
	jp	(hl)
	END_PATCH 2021

	LAST_PATCH
	
	
;*=*=*
;	LDOS 6.3.1 SYS0/SYS patch
;*=*=*
asys0sys:

;; 1e18  af           xor	a
;; 1e19  21 81 03     ld	hl,0381h
;; 1e1c  2d           dec	l
;; 1e1d  77           ld	(hl),a
;; 1e1e  20 fc        jr	nz,1e1ch
;; 1e20  ed 56        im	1
;; 1e22  31 80 03     ld	sp,0380h
;; 1e25  af           xor	a
	START_PATCH 2010, 1e22h
	nop
	nop
	nop
	END_PATCH 2010


;; 1e73  3a 9d 43     ld	a,(439dh)
;; 1e76  e6 03        and	03h
;; 1e78  47           ld	b,a
;; 1e79  21 73 04     ld	hl,0473h
;; 1e7c  7e           ld	a,(hl)
;; 1e7d  e6 fc        and	fch
;; 1e7f  b0           or	b
;; 1e80  77           ld	(hl),a
;; 1e81  db f1        in	a,(f1h)
;; 1e83  32 75 04     ld	(0475h),a
;; 1e86  11 08 02     ld	de,0208h
	START_PATCH 2011, 1e73h
	jp	1e86h
	END_PATCH 2011
	

;; 1fde  cd d7 1f     call	1fd7h
;; 1fe1  cb 67        bit	4,a
;; 1fe3  e5           push	hl
;; 1fe4  21 08 1b     ld	hl,1b08h
;; 1fe7  e3           ex	(sp),hl
;; 1fe8  c2 a0 19     jp	nz,19a0h
;; 1feb  d1           pop	de
;; 1fec  2f           cpl
;; 1fed  e6 01        and	01h
	START_PATCH 2012, 1fe8h
	nop
	nop
	nop
	END_PATCH 2012


;; 2024  01 50 00     ld	bc,0050h
;; 2027  d9           exx
;; 2028  e6 82        and	82h
;; 202a  c0           ret	nz
;; 202b  21 00 15     ld	hl,1500h
;; 202e  06 03        ld	b,03h
	START_PATCH 2013, 202ah
	nop
	END_PATCH 2013



;; 2056  21 7c 00     ld	hl,007ch
;; 2059  cb a6        res	4,(hl)
;; 205b  20 0c        jr	nz,2069h
;; 205d  21 76 00     ld	hl,0076h
;; 2060  7e           ld	a,(hl)
	START_PATCH 2014, 205bh
	jr	205bh
	END_PATCH 2014


	LAST_PATCH
