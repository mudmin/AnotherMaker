;;;
;;; replacement ROM routines for 4P
;;;


rom0033:
	push	af
	ld	a,(ROM_MODEL)
	cp	ROM_MODEL_4P
	jr	z,+
	pop	af
	jp	0033h
+	pop	af
	jp	disp_byte

rom0049:	
	push	af
	ld	a,(ROM_MODEL)
	cp	ROM_MODEL_4P
	jr	z,+
	pop	af
	jp	0049h
+	pop	af
-	call	kiscan
	jr	nz,-
	ret
	


kb0	equ	3801h

kflag:	db	0
timer:	db	0
shift:	db	0

kidata:	db	0			; last key entered
	db	0			; repeat time check
rptinit	equ	$-kidata
	db	22			; 22 * 33.3ms = .733secs
rptrate	equ	$-kidata
	db	2			; 2 x RTC rate
kbrow0	equ	$-kidata
	db	-1,-1,-1,-1		; image rows 0-3
kbrow4	equ	$-kidata
	db	-1,-1			; image rows 4-5
kbrow6	equ	$-kidata
	db	-1,-1			; image rows 6-7

CR	equ	13
LF	equ	10

	;; Conversion table for keyboard row 7/8
kbtbl	db	CR,1DH,1FH,1FH		; <ENTER> <CLEAR>
	db	80H,0,0BH,1BH		; <BREAK> <UPARW>
	db	LF,1AH,8,18H		; <DNARW> <LTARW>
	db	9,19H,20H,20H		; <RTARW> <SPACE>
	db	81H,91H,82H,92H 	; <F1> <F2>
	db	83H,93H			; <F3>
	;; Table to generate 5B-5F, 7B-7F
spcltb	db	",/.;",CR
	
kiscan:
	ld	ix,kidata		; point to data area
	ld	hl,kidata+kbrow0	; load kbd image start
	ld	bc,kb0			; load start of keyboard
	ld	d,0
ki_1:	ld	a,(bc)			; load 1st char from kbd
	ld	e,a
	xor	(hl)			; xor with old value
	jr	nz,ki_2			; go if different
	inc	d			; bump key counter
	inc	hl			; bump image pointer
	rlc	c			; go to next row
	jp	p,ki_1			; loop until end of rows
	ld	a,(bc)			; get row 7
	and	78h			; strip shift, ctl
	ld	e,a
	xor	(hl)
	jr	nz,ki_2
	ld	a,(ix+0)		; key down ? it's same as
	or	a			;   the last if so
	jr	z,nochar		; ret if no key
	ld	a,(timer)		; do we repeat the 
	sub	(ix+1)			;   same key ?
	sub	(ix+rptinit)		; beyond 0.75 seconds?
	ld	a,(bc)
	ld	e,a
	jr	c,ki_10			; go if yes
nochar	or	1			; else don't repeat
	ld	a,0			; show NZ with A=0
	ret

	;; found key change in matrix
ki_2:	ld	(hl),e			; stuff kb image with new
	and	e			;   kb row value
	jp	z,nokey			; go if new is none

	;; convert the depressed key
	ld	e,a			; save the active bit
	ld	a,d			; calculate 8*row
	rlca
	rlca
	rlca
	ld	d,a			; save 8*row
	ld	c,1			; add 8*row + column
ki_3:	ld	a,c
	and	e			; check if bits match
	jr	nz,ki_6			; go if match
	inc	d			;   else bump value
	rlc	c			; shift compare bit
	jr	ki_3			; loop to test next

	;; key pressed was not an alpha
ki_4:	sub	90h			; adjust for no-alpha
	jr	nc,ki_9			; go if special key
	add	a,40h			; convert to numeric/symbol
	cp	3ch			; manipulate to get
	jr	c,ki_5			;   proper code
	xor	10h
ki_5:	bit	0,e			; check shift
	jr	z,ki_11			; go if unshift
	xor	10h			;   else adjust for shift
	jr	ki_11

	;; found a key - setup the function codes
ki_6:	ld	a,(shift)		; p/u the shift key
	ld	e,a			; merge lh & rh shift keys
	and	2			; only merge bit 1
	rrca				; bit 1 to bit 0
	or	e			; merge bits 0&1
	ld	e,a			; value of rh or lh shift
	ld	a,d			; load semi-converted
	add	a,60h			; if alpha, convert to
	cp	80h			;   correct value
	ld	hl,kflag
	jr	nc,ki_4			; go if not alpha

	;;  alpha @-Z - if caps lock or <shift>, convert to caps unless CLEAR
	bit	2,e			; ctrl key down ?
	jr	nz,ctla2z		; ctrl sets <00-1A>
	cp	60h			; invert @ and '
	jr	nz,ki_7
	xor	20h			; invert and bypass test
	jr	ki_8			;   for caps lock
ki_7:	bit	1,(ix+kbrow6)		; if clear, don't test
	jr	nz,ki_8			;   for caps lock
	bit	5,(hl)			; caps lock ?
	jr	nz,tglcase
ki_8:	bit	0,e			; shift key down ?
	jr	z,ki_11			; bypass if not shifted
	jr	tglcase			; convert to upper case
ctla2z:	sub	60h			; convert ctrl a-z
	jr	nz,ki_11		; go on a-z
	bit	0,e			; shifted ?
	scf				; set c flag for ctrl-@
	ret	z			;   & return if unshifted
	ld	a,1ch			;   else set eof error
	ret
ki_10:	ld	a,(timer)		; advance time check
	add	a,(ix+rptrate)
	jr	ki_12			; go output the key

	;; special keys - rows 6 & 7
ki_9:	cp	11			; compress F1-F3 keys
	jr	z,capskey		;   while checking for CAP
	jr	c,ki_91			;   F1-F3 to 8-10
	sub	4
ki_91:	ld	hl,kbtbl		; pt to special char table
	rlca				; index into table
	bit	0,e			;   shift code is + 1
	jr	z,ki_92
	inc	a
ki_92:	ld	c,a			; index the table
	ld	b,0
	add	hl,bc
	ld	a,(hl)			; load char from table
	jr	ki_11			; bypass restore of char
tglcase:
	xor	20h			; toggle the case
ki_11:	cp	80h			; break key ?
	jr	nz,ki_11a		; ck on <break> disable
	ld	hl,kflag
	set	0,(hl)
	jr	ki_11a
ki_11b:	rla
ki_11a:	bit	1,(ix+kbrow6)		; clear key pressed ?
	jr	z,notalph		; go if not down
	ld	d,a			; save code
	res	5,a			; set to upper case for
	sub	'A'			;   test A-Z
	cp	'Z'-'A'+1
	ld	a,d			; get back actual char
	jr	nc,ki_11c		; go if not A-Z
	xor	20h			; shift keyboard case
ki_11c:	or	80h			; set bit 7 for clear key
notalph:bit	0,e			; shift key down ?
	jr	z,fixclr		; go if not
gotshft:cp	9Fh			; shift-clear ?
	jr	z,fixscl		; go if so
tstspa:	cp	20h			; shift-0 or shift-space?
	jr	nz,keyok		; go if not
	bit	0,(ix+kbrow4)		; ck zero key
	jr	z,keyok			; go if not down
	
	;; toggle the capslock in kflag
capskey:
	ld	a,20h			; caps wasn't 20h
cashk:	ld	hl,kflag		; reverse case by
	xor	(hl)			;   flipping bit 5
	ld	(hl),a
	jr	nokey
fixscl	xor	80h			; reset bit 7
fixclr	cp	9fh			; clear key ?
	jr	nz,keyok		; go if not
nokey	xor	a
keyok	ld	(ix+0),a
	ld	bc,0184h		; delay
typhk	call	pause
	ld	a,(timer)		; set initialization
delay2	add	a,(ix+rptinit)		;   repeat key delay
ki_12	ld	(ix+1),a		; save new repeat time
	ld	a,(ix+0)		; check if any key
	or	a			;   code was saved
	jp	z,nochar		; ret if none
	bit	2,e			; shift key down ?
	scf				; init carry
	jr	nz,specl		; ret if ctrl
	ccf
dvrexit:
	bit	7,a			; Z flag set on non-clear
	ret	z
specl:	push	af			; save code
ki_13:	ld	hl,spcltb		; special char table
	res	7,a			; turn off clear
	ld	bc,055bh		; 5 chars, starting char
	jr	nc,spcllp		;   if not ctrl
	dec	b			;   else only 4
spcllp:	cp	(hl)			; is this it ?
	jr	z,hit			; go if so
	xor	10h			; flip shift state
	cp	(hl)			; is that it ?
	jr	z,hitws			; go if so
	xor	10h			; flip back
	inc	hl			; bump spcl table ptr
	inc	c			; bump "convert to" char
	djnz	spcllp			; loop through table
	pop	af			; not found in table
	jr	c,ckctl2		; ck ctrl for c-flag
ckctl1	cp	a			; set z-flag
	ret
hitws	set	5,c			; move to LC set
hit	pop	af			; restore orig char
	ld	a,c			; load converted one
ckctl	jr	nc,ckctl1		; go if ctrl key not down
	and	1fh			; force ctrl code
ckctl2	cp	a			; set z flag
	scf				; set c flag for ctrl
	ret
	

pause:	ld	a,a
	dec	bc
	ld	a,b
	or	c
	jr	nz,pause
	ret




;;
;; basic display driver
;;
;; (supported control char : 0D, 1C-1F)
;;
disp_byte:
	push	hl
	ld	hl,(sptr)		; restore HL = sptr

	cp	20h			; control char ?
	jr	c,disp_ctrl		; yes
	cp	0C0h			; tab/special char ?
	jr	nc,disp_done3		; yes => ignore

	ld	(hl),a			; display regular char
	inc	hl
	jr	disp_done

disp_done3:	
	pop	hl
	ret

disp_08:
	dec	hl
	ld	(hl),20h
	jr	disp_done

disp_ctrl:
	cp	08h			; backspace
	jr	z,disp_08
	cp	0ah			; line feed
	jr	z,disp_0d
	cp	0dh			; new line
	jr	z,disp_0d
	cp	0Fh			; cursor back
	jr	z,disp_18
	cp	010h			; cursor forward
	jr	z,disp_19
	cp	01ah			; cursor down
	jr	z,disp_1a
	cp	01bh			; cursor up
	jr	z,disp_1b
	cp	01ch			; top of screen
	jr	z,disp_1c
	cp	01dh			; begin of line
	jr	z,disp_1d
	cp	01eh			; clear end of line
	jr	z,disp_1e
	cp	01fh			; erase until end of screen
	jr	z,disp_1f
	jr	disp_done2		; all others => ignore

disp_18:				; cursor back
	ld	a,l
	and	3fh
	dec	hl
	jr	nz,disp_done
disp_1a:				; cursor down
	ld	de,0040h
	add	hl,de
	jr	disp_done
disp_19:				; cursor forward
	inc	hl
	ld	a,l
	and	3fh
	jr	nz,disp_done
disp_1b:				; cursor up
	ld	de,0ffc0h
	add	hl,de
	jr	disp_done	
disp_1c:				; cursor home
	ld	hl,3c00h
	jr	disp_done
disp_1d:				; cursor at begin of line
	ld	a,l
	and	0c0h
	ld	l,a
	jr	disp_done
disp_1e:				; erase until end of line
	ld	d,h
	ld	a,l
	or	3Fh
	ld	e,a
	inc	de
	jr	+
disp_1f:				; erase until end of screen
	ld	de,4000h
/	ld	(hl),20h		; erase char until HL == DE
	inc	hl
	ld	a,h
	sub	d
	jr	nz,-
	ld	a,l
	sub	e
	jr	nz,-
	jr	disp_done2
disp_0d:
	ld	a,l			; cursor to start of line
	and	0c0h
	ld	l,a
	ld	de,0040h		; next line
	add	hl,de
	jr	disp_done
disp_done:
	ld	(sptr),hl
disp_done2:	
	pop	hl
	ret
