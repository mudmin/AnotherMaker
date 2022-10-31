
	cpu z80

	include	frehd_macro.inc
	include frehd.inc

	;; Buffers
SECTOR1		equ 4300h
SECTOR1_M1	equ 4200h
REED		equ 5F00h
BUF		equ 5E00h
BUF1		equ REED+80h
DRIVE0		equ REED+0D0h
BOOTFILES	equ 5C00h
	

 IF (REED & 0FFh) <> 0
	ERROR "REED must be 256-bytes aligned!"
 ENDIF
	
	org	6000h

	ld	a,ALWAYS
	or	a
	jr	nz,+

	ld	a,(3801h)		; keyboard
	and	04h			; 'B' ?
	jr	z,++			; no, skip menu
+	call	select_menu		; yes, go to boot menu

+	xor	a			; select drive 0
	out	(SDH),a
	ld	a,READ_HEADER		; read drive header
	out	(COMMAND2),a
	call	wait
	ret	nz			; return if error
	ld	hl,REED
	ld	bc,DATA2		; read 256 bytes from DATA2
	inir

	ld	a,(REED+REED_FLAG)	; auto-boot this image ?
	and	01h
	ret	z			; no

	ld	a,(REED+REED_OS)	; can we support this OS ?
	sub	MAX_OS
	ret	nc			; no

	call	sector_addr
	ld	c, 01h		      	; C=01 (sector num)
	ld	de, 0			; cylinder to read
	call	hd_read_sector
	ret	nz			; if error, return

	ld	a,(REED+REED_OS)	; find the patch table entry
	ld	l,a			;   for this OS
	ld	h,0h
	add	hl,hl
	ld	de,patch_table
	add	hl,de
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	call	patch			; patch the OS

	ld	a,(ROM_MODEL)		; 4P is again special...
	cp	ROM_MODEL_4P
	jr	z,boot4p

-	call	hack_time		; all models except 4P
	call	sector_addr
	jp	(hl)			; jump in OS !

	
boot4p:
	ld	a,(REED+REED_OS)	; LDOS531
	cp	OS_LDOS531
	jr	z,+
	cp	OS_NEWDOS25		; NEWDOS/80
	jr	nz,-
+	call	model4p_rom3		; both need the model 3 rom
final4p:
	ld	hl, unpatch_rom4p	; unpatch 3 bytes
	ld	de, 30d9h
	ld	bc, 3
	ldir
	xor	a
	out	(84h),a			; write-protect ram
	
	;call	hack_time		; (doesn't work for now)
	jp	SECTOR1
	
	


;;;
;;; return first sector address in HL
;;;
sector_addr:
	ld	hl, SECTOR1		; address of buffer for sector
	ld	a,(ROM_MODEL)
	cp	ROM_MODEL_1		; model 1 uses a different address
	jr	nz,+
	ld	hl,SECTOR1_M1
+	ret



;;;
;;; 4P model 3 support
;;;
;;; - load the rom from FreHD
;;; - patch the rom (to return here after basic initialization)
;;; - map the rom read-only
;;; - set speed for model 3
;;; - initialize the rom code (jump in and return)
;;;
patch_rom4p:
	db	0c3h
	dw	final4p
unpatch_rom4p:
	db	0cdh
	dw	3518h
	
model4p_rom3:
	call	0fc9h			; load model.iii
	ret	nz			; failed

	ld	(4024h),hl		; stuff rom entry point

	ld	hl, patch_rom4p		; patch 3 bytes
	ld	de, 30d9h
	ld	bc, 3
	ldir
	xor	a
	out	(9ch),a			; switch rom out
	ld	a, 10h			; 2MHz, I/O enabled
	out	(0ECh),a
	jp	4020h			; continue boot (will go to rom)


;;;
;;; read sector
;;;
;;; - C  : sector number
;;; - DE : cylinder
;;; - HL : buffer address
;;;
hd_read_sector:
	push	de
	ld	a, c			; sector to read
	out	(SECNUM), a		; send sector number to FreHD
	xor	a			; drive 0, head 0, sector 256 bytes
	out	(SDH), a		; send size/drive/head to FreHD
	ld	a, d			; LSB cylinder to read
	out	(CYLHI), a
	ld	a, e			; MSB cylinder to read
	out	(CYLLO), a
	ld	a, 20h			; read command
	out	(COMMAND), a		; send command
	call	wait			; wait while command is processed
	jr	nz,+
	ld	bc, DATA		; get 256 bytes
	inir
	xor	a			; set Z indicates no error
+	pop	de
	ret


wait:
	ex (sp),hl
	ex (sp),hl			; small delay to settle the controller
-	in a, (STATUS)
	rlca				; busy bit -> C
	jr c, -
	in a, (STATUS)			; read status again
	and 01h				; nz = error
	ret


;;;
;;; patch boot sector
;;;
;;; Format:  aaaa xxxx [data...]
;;;
;;; aaaa = address
;;; xxxx = length of data
;;;        if bit 15 set, xxxx = 1<len 7bits> <offset>
;;;           where up to 128 bytes are copied from reed header starting at
;;;           <offset>
;;;
;;; entry: HL -> patch data
;;;
patch:
	push	af
	push	bc
	push	de
-	ld	e,(hl)			; get address
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	a,d			; is it 0000h ?
	or	e
	jr	nz,+
	pop	de
	pop	bc
	pop	af
	ret
+	ld	c,(hl)			; get length
	inc	hl
	ld	b,(hl)
	inc	hl
	bit	7,b			; bit 15 set ?
	jr	nz,+			; yes.. copy from reed header
	ldir				; no... patch this area
	jr	-			;       and loop
+	push	hl
	ld	h,REED>>8
	ld	l,c			; c = offset
	ld	a,b
	and	7Fh
	ld	c,a
	ld	b,0			; bc = len
	ldir				; copy reed -> patched area
	pop	hl
	jr	-
	
	include menu.asm
	include menu_util.asm
	include time.asm


;*=*=*
;	Patch table
;*=*=*
patch_table:
	dw	ldos631
	dw	ldos531
	dw	cpm
	dw	newdos25
	dw	m1ldos531
	dw	m1newdos25

;*=*=*
;	LDOS 6.3.1 patch
;*=*=*
	include ldos631.asm

;*=*=*
;	LDOS 5.3.1 patch
;*=*=*
	include ldos531.asm

;*=*=*
;	CPM doesn't need to be patched !
;*=*=*
cpm:	LAST_PATCH

;*=*=*
;	M3 NEWDOS 2.5 patch
;*=*=*
	include newdos25.asm

;*=*=*
;	M1 LDOS 5.3.1 patch
;*=*=*
	include m1ldos531.asm

;*=*=*
;	M1 NEWDOS 2.5 patch
;*=*=*
	include m1newdos25.asm
