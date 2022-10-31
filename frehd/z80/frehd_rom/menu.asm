;;;
;;; 
;;; BOOT MENU
;;;
;;; 

	
MENU_FIRST_ROW	equ	 	4
MENU_FIRST_COL	equ 	4
MENU_ROWS		equ 	16-MENU_FIRST_ROW
MENU_MAX		equ 	3*MENU_ROWS
getver     		equ     0
data2       	equ     0c2h
command2    	equ     0c4h
status      	equ     0cfh
svn_rev_		equ		SVN_REV	

im_cur		db	0			; currently select boot file
im_max		db	0			; number of boot files found
m4prom		db	1			; true if "modela.iii" is present


msg_not_found:	db "No bootable image!"
msg_not_found_len equ $-msg_not_found

	
;*=*=*
;	4P : is the rom file present on the SD card ?
;*=*=*
rom4p_file:	db	FA_OPEN_EXISTING|FA_READ,MODEL_III_FILE,0
rom4p_filelen:	equ	($ - rom4p_file)
	
init_4p:
	ld	a,(ROM_MODEL)		; skip if not 4P
	cp	ROM_MODEL_4P
	ret	nz
	ld	hl,rom4p_file
	ld	a,rom4p_filelen
	out	(SIZE2),a
	ld	a,OPEN_FILE
	out	(COMMAND2),a
	call	wait
	ld	bc,rom4p_filelen << 8 | DATA2
	otir
	call	wait
	jr	z,+
	xor	a
	ld	(m4prom),a
+	ld	a,CLOSE_FILE
	out	(COMMAND2),a
	call	wait
	ret


;*=*=*
;	banner
;*=*=*
banner:
		defb	1ch			;tof 
		defb	1fh			;cls 
		defb	0a8h,9ch,8ch,8ch,8ch,20h,20h,20h,20h,20h,20h,20h,20h
		defb	20h,20h,20h,0bch,20h,20h,0a8h,94h,0bch,8ch,8ch,0ach,90h

		defb    "             Firmware "
firm_	defb 	"0.00"
		defb	" Loader "
load_	defb	"0.00"	
		defb	0aah,97h,83h,81h,0aah,9dh,86h,83h,83h,8ch,0a8h,0b6h,0b3h
		defb	0b3h,0b9h,94h,0bfh,83h,83h,0abh,95h,0bfh,20h,20h,0aah,95h

		defb    "               FREHD.ROM  Ver. "
svn_	defb    "00000"
		defb	0dh;
		defb	8ah,85h,20h,20h,8ah,85h,20h,20h,20h,20h,82h,8dh,8ch,8ch
		defb	8ch,20h,8fh,20h,20h,8ah,85h,8fh,8ch,8ch,8eh,81h

		defb    "             TRS-80 Hard Disk Emulator"


		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	83h,83h,83h,83h,83h,83h,83h,83h,83h
		defb	03h			;eom

sptr	defw	3C00h


show_banner:
;
; build msg for firmware/loader versions
;
	ld      a,getver
	out     (command2),a
	ex      (sp),hl
	ex      (sp),hl         ; small delay to settle the controller
-	in      a,(status)
	cp      0ffh            ; no interface on the bus
	jr      z,+        		; keep going..
	rlca
	jr      c,-

+	ld		hl,firm_
	call    get_version  	; get firmware version data
	ld		hl,load_
	call    get_version    	; get loader version data
	ld		de,svn_rev_		; convert svn revision (hex) to ascii 
	ld		hl,svn_
	call	addr2ascii
;
;	now display the banner
; 
	ld	hl,banner
-	ld	a,(hl)
	inc	hl
	cp	03h
	ret	z
	call	rom0033
	jr	-
	
addr2ascii:
	ld		a,d				;msb
	call	+				;output
	ld		a,e				;lsb
	call	+				;output
	ld		a,'H'			;hex address
	ld		(hl),a	
	inc hl
	ret

+	push	af				;save byte
	rra		
	rra		
	rra		
	rra						;msb to lsb
	call	+				;output digit
	pop		af				;get byte
+	and		0fh				;lsb
	add		a,90h
	daa
	adc		a,40h			;convert trick
	daa		
	ld		(hl),a	
	inc hl
	ret

get_version:
	in		a,(data2)		; major version number
	call	+
	ld		a,'.'
	ld		(hl),a	
	inc hl
	in		a,(data2)		; minor version number
	call	++
	ret

+	call	bin2bcd			; enter here to mask the leading 0
	push	af
	and		0f0h			; mask out the lower digit
	jr		z,++
	jr		+

+ 	call	bin2bcd			; enter here to keep the leading 0
	push	af
	and		0f0h 			
	srl		a
	srl		a
	srl		a
	srl		a
	add		a,30h			; to ascii

	ld		(hl),a
	inc hl

+	pop		af
	and		00fh			; mask out the upper digit
	add		a,30h			; to ascii
	ld		(hl),a
	inc hl			
	ret
			
bin2bcd:
	ld		bc,0
-	sub		10				; count number of 10's deducted into b
	jp		m,+
	inc		b
	jr		-
+	add		a,10			; add back the last 10
	sla		b				; move the 10's into the top 4 bits
	sla		b
	sla		b
	sla		b
	or		b				; merge in the 10's
	ret	
	
show_nothing:
	ld	de,3C00h+MENU_FIRST_ROW*40h+MENU_FIRST_COL
	ld	hl,msg_not_found
	ld	bc,msg_not_found_len
	ldir
-	call	read_key
	ld	a,b
	cp	5h			; 5 => re-read
	jr	z,build_menu
	jr	-

select_menu:
	call	init_4p
	call	show_banner		; show banner at top
build_menu:
	ld	c,0
	call	get_position		; get position of first entry
	ex	hl,de
	call	clear_screen2		; and clear the screen from there
	call	read_info		; get filename of current image
	call	read_directory		; read the root directory 
	ld	a,(im_max)
	or	a			; if 0, no image found
	jr	z,show_nothing
	call	display_bootfiles
	call	draw_arrow
menu_loop:
	call	read_key
	ld	a,b
	ld	hl,im_cur
	or	a
	jr	z,do_select		; 0 => select
	dec	a
	jr	z,do_prev		; 1 => previous
	dec	a
	jr	z,do_next		; 2 => next
	dec	a
	jr	z,do_left		; 3 => left
	dec	a
	jr	z,do_right		; 4 => right
	dec	a
	jr	z,build_menu		; 5 => re-read
	jr	menu_loop
do_prev:
	ld	a,(hl)
	or	a
	jr	z,menu_loop
	call	erase_arrow
	dec	(hl)
	call	draw_arrow
	jr	menu_loop
do_next:
	ld	a,(im_max)
	dec	a
	sub	(hl)
	jr	z,menu_loop
	call	erase_arrow
	inc	(hl)
	call	draw_arrow
	jr	menu_loop
do_left:
	ld	a,(hl)
	sub	MENU_ROWS
	jr	c,menu_loop
	call	erase_arrow
	ld	a,(hl)
	sub	MENU_ROWS
	ld	(hl),a
	call	draw_arrow
	jr	menu_loop
do_right:
	ld	a,(im_max)
	ld	b,a
	ld	a,(hl)
	add	a,MENU_ROWS
	sub	b
	jr	nc,menu_loop
	call	erase_arrow
	ld	a,(hl)
	add	a,MENU_ROWS
	ld	(hl),a
	call	draw_arrow	
	jr	menu_loop

do_select:
	call	keep_one
	ld	hl,BOOTFILES		; make HL points to filename
	ld	a,(im_cur)
	or	a
	jr	z,+
	ld	b,a
	ld	de,000Dh
-	add	hl,de
	djnz	-
+	push	hl			; save HL
	ld	b,3
-	ld	a,(hl)			; compute b = strlen(hl) + 2 + 1
	or	a
	jr	z,+
	inc	hl
	inc	b
	jr	-
+	pop	hl			; restore HL
	ld	a,b
	out	(SIZE2),a
	ld	a,MOUNT_DRIVE
	out	(COMMAND2),a
	call	wait
	ld	c,DATA2
	xor	a
	out	(c),a			; drive number 0
	out	(c),a			; options : default
	dec	b
	dec	b
	otir
	call	wait
	call	clear_screen		; clear screen before booting
	ret


;;;
;;; erase all names except the selected one
;;; 
keep_one:
	ld	b,0			; i = 0
-	ld	a,(im_cur)
	sub	b
	jr	z,+			; if i == im_cur, skip
	ld	c,b
	call	get_position
	ex	hl,de
	ld	b,FIL_FNAME_LEN+MENU_FIRST_COL
-	ld	(hl),' '
	inc	hl
	djnz	-
	ld	b,c
+	inc	b			; i++
	ld	a,(im_max)
	sub	b
	jr	nz,--			; loop if i != im_max
	ret
	
	
	
clear_screen:
	ld	hl,3c00h		; clear screen
clear_screen2:	
	ld	de,4000h
-	ld	(hl),' '
	inc	hl
	ld	a,h
	sub	d
	jr	nz,-
	ld	a,l
	sub	e
	jr	nz,-
	ret


read_info:
	xor	a			; drive0 = ""
	ld	(DRIVE0),a
	ld	a,INFO_DRIVE		; get info for mounted drive 0
	out	(COMMAND2),a
	call	wait
	ret	nz			; return if error
	ld	hl,BUF
	ld	bc,20h<<8|DATA2
	inir
	ld	a,(BUF)			; drive 0 available ?
	and	1h
	ret	z			; return if not
	ld	hl,BUF+6
	ld	de,DRIVE0
	ld	bc,FIL_FNAME_LEN+1
	ldir
	ret

	
read_directory:
	ld	de,BOOTFILES		; DE = boot files buffer
	xor	a
	ld	(im_max),a		; nothing in there
	ld	(im_cur),a

	ld	a,2			; strlen("/")
	out	(SIZE2),a
	ld	a,OPEN_DIR		; read directory command
	out	(COMMAND2),a
	call	wait
	ld	a,'/'			; send '/'
	out	(DATA2),a
	xor	a			; send null terminator
	out	(DATA2),a
	call	wait
	jr	nz,dir_error
read_dir_loop:
	ld	a,(im_max)		; do we have space for another entry?
	cp	MENU_MAX
	ret	nc
	ld	a,READ_DIR		; request one entry
	out	(COMMAND2),a
	call	wait
	jr	nz,dir_error
	in	a,(STATUS)		; check DRQ to see if we got an entry
	and	STATUS_DRQ
	ret	z			; no, we are done
	ld	hl,BUF1
	in	a,(SIZE2)
	ld	b,a
	ld	c,DATA2
	inir
	ld	a,(BUF1+FIL_ATTRIB_OFFSET) ; is it a directory ?
	and	FIL_ATTRIB_DIR
	jr	nz,read_dir_loop	; yes, skip this entry
	ld	hl,BUF1
	ld	a,(hl)
	inc	hl
	or	(hl)
	inc	hl
	or	(hl)
	jr	z,read_dir_loop		; too small, skip this entry
	call	file_candidate
	jr	read_dir_loop
dir_error:
	xor	a
	ld	(im_cur),a
	ld	(im_max),a
	ret

	
	
;;;
;;; We have a file, bigger than 255 bytes. Check if it's a bootable image
;;; 
file_candidate:
	ld	b,2			; compute strlen(fname)
	ld	hl,BUF1+FIL_FNAME_OFFSET
-	ld	a,(hl)
	or	a
	jr	z,+
	inc	hl
	inc	b
	jr	-
+	ld	a,b
	out	(SIZE2),a
	ld	a,OPEN_FILE
	out	(COMMAND2),a
	call	wait
	ld	hl,BUF1+FIL_FNAME_OFFSET
	ld	c,DATA2
	ld	a,FA_OPEN_EXISTING|FA_READ
	out	(c),a			; send option
	otir				; send filename
	xor	a			; send null-terminator
	out	(c),a
	call	wait
	ret	nz			; ignore this file if error
	xor	a
	out	(SIZE2),a		; request 256 bytes
	ld	a,READ_FILE
	out	(COMMAND2),a
	call	wait
	ret	nz
	ld	hl,BUF
	ld	bc,DATA2
	inir
	call	check_reed
	ld	a,CLOSE_FILE
	out	(COMMAND2),a
	call	wait
	ret


;;;
;;; Verify if BUF points to a reed buffer, with auto-boot and recognized OS.
;;; For model 3 OS (ldos 5.3.1, newdos/80), also check that the rom is present
;;; (only for 4P, file MODEL_III_FILE must be present on sd card)
;;;
check_reed:
	ld	a,(BUF)			; verify first 2 bytes
	cp	REED_MAGIC0
	ret	nz
	ld	a,(BUF+1)
	cp	REED_MAGIC1
	ret	nz
	ld	a,(BUF+REED_FLAG)	; check auto-boot
	and	01h
	ret	z
	ld	a,(BUF+REED_OS)		; do we support this OS ?
	cp	MAX_OS
	ret	nc
	cp	OS_LDOS531		; if 4P and LDOS531
	jr	z,+
	cp	OS_NEWDOS25		;       or NEWDOS
	jr	nz,++
+	ld	a,(m4prom)		; then we need the rom file !
	or	a
	ret	z
					; we got one !
+	ld	hl,BUF1+FIL_FNAME_OFFSET
	ld	bc,000dh		; 8.3 + null-terminator
	ldir

	ld	hl,BUF1+FIL_FNAME_OFFSET
	call	is_drive0		; is this current drive 0 ?
	jr	nz,+			; no
	ld	a,(im_max)		; yes.. im_cur = im_max
	ld	(im_cur),a
	
+	ld	a,(im_max)		; im_max++
	inc	a
	ld	(im_max),a
	ret


;;;
;;; String compare HL with drive0. Return Z if same.
;;;
is_drive0:
	push	de
	ld	de,DRIVE0
-	ld	a,(de)
	cp	(hl)			; (HL) == (DE) ?
	jr	nz,++			; no.. return NZ
	or	a			; (HL) == (DE) == 0 ?
	jr	z,++			; yes.. return Z
	ld	a,(de)
	or	a			; at the end of (DE) ?
	jr	z,+			; yes..  return NZ
	ld	a,(hl)
	or	a			; at the end of (HL) ?
	jr	z,+			; yes.. return NZ
	inc	de
	inc	hl
	jr	-
+	ld	a,1
	or	a
+	pop	de
	ret


;;;
;;; Display boot files
;;;
;;; When we are here, we have the logo at the top of the empty screen. 
;;;
display_bootfiles:
	ld	hl,BOOTFILES
	ld	c,0
	
-	ld	a,(im_max)		; are we done ?
	cp	c
	ret	z			; yes

	call	get_position
	ld	a,e
	add	a,MENU_FIRST_COL
	ld	e,a
	
	ld	b,0dh			; copy 13 bytes
-	ld	a,(hl)
	or	a
	jr	z,+			; we reach null-terminator
	ld	(de),a
	inc	hl
	inc	de
	dec	b
	jr	-
+	ld	a,' '			; copy spaces after null-terminator
-	ld	(de),a
	inc	de
	inc	hl
	djnz	-

	inc	c			; next entry
	jr	---
	
	
;;;
;;; Input:  C  = bootfile number
;;; Output: DE = screen position
;;;
get_position:
	push	hl
	push	bc
	;; compute D = number of columns
	ld	d,0			; default 1 column
	ld	a,(im_max)
	cp	MENU_ROWS
	jr	c,+
	inc	d			; at least 2 columns
	cp	2*MENU_ROWS
	jr	z,+
	jr	c,+
	inc	d			; 3 columns
	;; compute column for entry in C
+	ld	hl,3C00h		; top of screen
	ld	a,c
	cp	2*MENU_ROWS		; on 3rd column ?
	jr	c,+			; no
	sub	2*MENU_ROWS
	ld	l,42
	jr	++
+	cp	MENU_ROWS		; on 2nd column ?
	jr	c,+			; no
	sub	MENU_ROWS
	ld	l,21
	dec	d			; if 2 columns only,
	jr	nz,+			;   then start 2nd column
	ld	l,32			;   at 32
	;; compute location on screen
+	ld	de,0040h
	add	a,MENU_FIRST_ROW
	ld	b,a
-	add	hl,de
	djnz	-
	ex	hl,de
	pop	bc
	pop	hl
	ret
	
;;;
;;; Draw arrow at current selected bootfile
;;;
draw_arrow:
	ld	a,(im_cur)
	ld	c,a
	call	get_position
	ld	a,'='
	ld	(de),a
	inc	de
	ld	(de),a
	inc	de
	ld	a,'>'
	ld	(de),a
	ret


;;;
;;; Erase arrow at current selected bootfile
;;;
erase_arrow:
	ld	a,(im_cur)
	ld	c,a
	call	get_position
	ld	a,' '
	ld	(de),a
	inc	de
	ld	(de),a
	inc	de
	ld	(de),a
	ret



;;;
;;; read key : read keyboard
;;;
;;; Returns B = 0 : <enter>
;;;             1 : <up>
;;;             2 : <down>
;;;             3 : <left>
;;;             4 : <right>
;;;		5 : <R>
;;; 
read_key:
	call	rom0049
	ld	c,a			; save key
	ld	hl,key_table
-	ld	a,(hl)
	cp	0FFh			; end of table
	jr	z,read_key		; yes, ignore this key, loop
	cp	c			; is this our key ?
	jr	z,+			; yes !
	inc	hl			; no, loop
	inc	hl
	jr	-
+	inc	hl
	ld	a,(hl)			; read converted value
	ld	b,a
	ret
	


;;; convert table
key_table:
	db	0dh, 0			; <enter>
	
	db	'A', 1			; UP
	db	'a', 1
	db	05Bh, 1
	db	0Bh, 1

	db	'Z', 2			; DOWN
	db	'z', 2
	db	0Ah, 2

	db	'O', 3			; LEFT
	db	'o', 3
	db	08h, 3

	db	'P', 4			; RIGHT
	db	'p', 4
	db	09h, 4

	db	'R', 5			; RELOAD
	db	'r', 5

	db	0FFh
