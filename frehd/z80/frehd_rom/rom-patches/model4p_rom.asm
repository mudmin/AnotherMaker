
	CPU	z80

FREHD	equ	1
	
;
; Model 4P Boot ROM v1.16
; 
		ORG	0000H

		di		; Start	here when Reset	switch is pressed
;
		db  46h	; F	Four pointless LD instructions which...
		db  44h	; D	spell the initials of the ROM"s author.
		db  49h	; I	When using BOOT command in 5.x & 6.x...
		db  56h	; V	execution of ROM will start at 0005H.
; 
		jr	loc_3E	; Skip over the	RSTs, and continue below
; 
		db    0		; Next are RST vectors -- all jump to RAM

rst_8:		jp	4000h	; RST 8	(Disk I/O) -- will point to a copy of
loc_B:		jp	loc_8B5	; this instruction, so a RST 8 will go
		db    0		; from ROM to RAM and back to ROM.
		db    0

rst_10:		jp	4003h	; RST 10H (Display String) will	also point...
		jp	sub_249	; to a copy of the instruction on the
		db    0		; following line.
		db    0

rst_18:		jp	4006h	; RST 18H (Display Message)
		jp	loc_255	; Ditto
		db    0
		db    0

rst_20:		jp	4009h	; RST 20H (Get Byte)
		jp	loc_B74	; Initialized to get byte from MODEL%/III file
		db    0		; (Vector in RAM is altered for	RS-232 boot)
		db    0

rst_28:		jp	400Ch	; RST 28H (Load	Object Code)
		jp	loc_B48	; Ditto
		db    0
		db    0

rst_30:		jp	400Fh	; RST 30H (Scan	Keyboard)
		jp	loc_83E	; Ditto
		db    0
		db    0

rst_38:		jp	4012h	; RST 38H -- Mode 1 Interrupts
		ret		; If any interrupts, just ignore them
		db    0		; (These RSTs jump to the same addresses...
		db    0		; as the RSTs in the Model III ROM.)

		
loc_3E:		xor	a	; Initialization continues here
		out	(0E4h),	a	; NMI mask -- no NMIs, please
		ld	a, 50h ; "P"    ; Use 4 MHz clock, enable I/O bus
		out	(0ECh),	a
		ld	b, 7		; Number of times to loop
		ld	hl, 0Bh		; Point	to instruction to copy
		ld	de, 4000h	; Point	to RAM destination of RST 8 jump
loc_4D:		ld	c, 3		; Number of bytes to copy
loc_4F:		ld	a, (hl)		; Copy three bytes from	(HL) to	(DE)...
		ld	(de), a		; these	are the	instructions that...
		inc	de		; these	RSTs will jump to.
		inc	hl
		dec	c
		jr	nz, loc_4F	; Copy second and third	bytes of instruction
		ld	a, l		; Add 5	to HL, so it points to the next...
		add	a, 5		; instruction to copy into RAM.
		ld	l, a
		djnz	loc_4D		; Loop until all 7 instructions	are copied
		ld	sp, 40A0h	; Set up stack pointer
		ld	c, 88h 		; CRTC Address Register
		jr	loc_C1		; Skip over data, and continue from there
	
aIii:		db "III"           	; Extension for ROM image file
		jp	4015h
		jp	loc_A48		; This block is	copied to RAM, 4015H-4047H
aModel:		db "MODEL%  "		; Filename for ROM image
		xor	a		; Last part of boot -- ends up at 4020H
		out	(9Ch), a	; Switch boot ROM out...
		jp	4300h		; and start executing boot sector.
		db    	0
		db	66h		; f Ends up at 4027H - pointless LDs that
		db	64h		; d   are the author"s initials in lowercase.
		db  	69h		; i   Executed when fatal error occurs, and 
		db	76h		; v   this instruction ends the boot procedure.
		db	0

ROMDATE:	db	"1(16) 18-Oct-83" ; Boot ROM version and date...
		db	0 
		db	0
		db	0
		xor	a		; Ends up at 403EH after block copy
		out	(9Ch), a	; Switch boot ROM out...
		ld	hl, (loc_B)	; Get word from	from ROM image,	or...
		inc	a		; whatever"s there instead.
		out	(9Ch), a	; Boot ROM back	in, and	return
		ret			; This is the end of the copy to RAM.

BootROMVers:	db "Boot ROM Version Is "
		db    0			; Message terminated with zero byte
		db    0			; Data for CRTC	registers 1 through 15...
		db    0			; see description in text.
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0
		db    0

loc_C1:		im	1		; Z80 was set to IM 0 by RESET
		ld	a, 0D0h		; Terminate command w/o interrupt
		out	(0F0h),	a	; to FDC
		ld	b, 0Fh		; Number of CRTC registers to program
		ld	hl, 0BFh 	; Point to data for register 15

loc_CC:		ld	a, (hl)		; Get data for CRTC register
		out	(c), b		; Send regnum to CRTC addr register
		out	(89h), a	; Send data to CRTC data register
		dec	hl		; Point	to next	byte of	data
		djnz	loc_CC		; Repeat for next register, down to R1
		ld	a, (3820h)	; Keyboard column 5
		bit	6, a		; <.> pressed?
		jp	nz, loc_CF2	; If yes, then go to RAM test
		ld	a, 80h 		; Set second page of video memory...
		out	(84h), a
		call	rst_206		; and clear it.
		ld	a, 0		; Back to first	page of	video memory
		out	(84h), a
		call	rst_206		; Clear	that too
		ld	de, 4015h	; Point	to destination in RAM for block	copy
		ld	hl, 69h		;  Point to source, in ROM
		ld	bc, 33h		;  Length of block
		ldir			; Copy it into RAM

loc_F5:		ld	hl, 4055h	; Point	to boot	ROM"s data area in RAM...
		ld	(hl), 0		; and clear it.
		ld	d, h
		ld	e, l
		inc	de
		ld	bc, 15h		; Number of bytes to clear, minus one
		ldir
		ld	bc, 7FFFh	; Number of times to scan keyboard...
		rst	30h		; so scan the keyboard.
		push	af		; save the result
		ld	a, 1		; Floppy disk drive
		ld	b, 4		; Restore -- drive 0
		rst	8		; Do disk I/O
		jr	z, loc_115	; Jump if no errors...
		ld	(4067h), a	; else store error code.
		cp	6		; Was the error	"Floppy not available"?
		jr	z, loc_11A	; If yes, then skip next three lines

loc_115:	ld	b, 32h ; "2"    ; Restore drives 1,2,3 (if possible)
		ld	a, 1		; Floppy disk drive
		rst	8		; Do Disk I/O

loc_11A:	pop	af		; Retrieve result of keyboard scan
		jr	z, loc_163	; If no	keys pressed, then try everything
		ld	a, (4059h)	; Get value stored by kbd scan
		cp	86h 		; Was it either <F1> or <1>?
		jr	nz, loc_129	; If not, then jump...
		call	sub_1C0		; else try HD boot -- returns only if...
		jr	loc_130		; unsuccessful,	so jump	(error no. in A).

loc_129:	cp	87h		; Was either <F2> or <2> pressed?
		jr	nz, loc_135	; If not, skip next few	lines
		call	sub_1CA		; Try floppy boot (returns only	if error)

loc_130:	or	a		; Error	in HD or floppy	boot
		jr	z, loc_137	; If error 0 (Mod III disk), then jump...
		jr	FatalError	; else display fatal error and halt.

loc_135:	cp	88h 		; Was either <F3> or <3> pressed?

loc_137:	jr	z, loc_175	; If yes, then try Model III boot
		cp	80h 		; Was <V> (display ROM version) pressed?
		jr	nz, loc_14A	; If not, then skip next few lines
		ld	de, 0		; Display at top of screen
		ld	hl, 9Ch		; "Boot ROM Version Is "
		rst	10h		; Display it
		ld	hl, 80h		; Point to boot ROM version...
		rst	10h		; and display it...
		jr	loc_160		; and halt
; 

loc_14A:	cp	83h 		; Was <Right Shift> pressed?
		jp	z, RS232Boot	; If yes, then jump to RS-232 boot...
		jr	loc_F5		; else clear data, scan	kbd, repeat.
; 
; START	OF FUNCTION CHUNK FOR ArcnetError

FatalError:	add	a, a		; Fatal	Error: error number x2
		ld	hl, 273h	; Start	of list	for messages
		ld	d, 0		; Error	number times two into DE
		ld	e, a
		add	hl, de		; Point	to appropriate entry in	list
		ld	a, (hl)		; Get start of table for message into HL
		inc	hl
		ld	h, (hl)
		ld	l, a
		xor	a		; Set Z	because	we have	starting position for msg
		ld	e, a		; D is still 0 -- 0000H	means top of screen
		rst	18h		; Display message...

loc_160: 	jp	4027h		; and HALT.  End of an unsuccessful boot!

loc_163: 	ld	a, 55h ; "U"    ; No keys were pressed during keyboard scan:
		ld	b, a		; Take a "U"...
		out	(0B6h),	a	; send it out, and see if there"s an...
		in	a, (0B6h)	; Arcnet board	to echo	it.
		cp	b		; Is an	Arcnet board attached?
		call	z, ArcnetError	; If it	is, try	Arcnet boot
 IF FREHD
		call	frehd_boot
 ELSE
		call	sub_1C0		; Try hard drive boot (no return if success)
 ENDIF
		or	a		; Ignore error 0 (M3 disk or 512-byte sector)
		call	nz, sub_1CA	; If other error, try floppy boot

loc_175: 	ld	a, (4066h)	; Model	III boot:
		or	a		; Was <L> pressed?
		jr	nz, loc_180	; If it	was, skip next two lines
		call	sub_1EB		; Is ROM image already loaded?
		jr	z, loc_1A2	; If it	is, then skip next section

loc_180:
 IF FREHD
		call	frehd_iii
		nop
		jr	z,loc_19D
 ELSE
		ld	a, (4067h)	; Error	code from restore cmd on floppy	0
		or	a		; Was there an error?
		jr	nz, FatalError	; If so, display error message and halt
 ENDIF
loc_186:	ld	a, 1		; Write-enable 0000H-37FFH
		out	(84h), a	; Read boot ROM, but write to RAM
		ld	de, 0		; Display message at top of screen
		ld	hl, mLoading	; "Loading ROM Image..."
		rst	18h		; Display it
		call	sub_A72		; Load ROM image from floppy disk
		push	af		; Save registers...
		push	hl
		call	rst_206		; clear	screen (erase "Loading..." msg)...
		pop	hl
		pop	af		; restore registers.
		jr	nz, FatalError	; If error loading image, display msg &	halt
loc_19D:	ld	a, 1		; Indicate ROM image was just loaded
		ld	(4058h), a	; Store	it

loc_1A2: 	ld	(4024h), hl	; Put transfer addr for	ROM image...
		ld	a, (4065h)	; into last part of boot routine.
		or	a
		jr	z, loc_1B6	; If <P> was NOT pressed, skip next few	lines
		ld	hl, 443h	; "ROM image loaded -- press ENTER or BREAK"
		rst	18h		; Dispaly it

loc_1AF: 	ld	a, (3840h)	; Keyboard matrix -- column 6
		and	5		; Loop until <ENTER> or	<BREAK>...
		jr	z, loc_1AF	; the ROM image	will check for <BREAK>.

loc_1B6: 	xor	a		; Set 0000H-37FFH back to read-only...
		out	(84h), a	; still	using Model III	memory map.
		ld	a, 10h		; 2 MHz	clock, I/O bus enabled
		out	(0ECh),	a
		jp	4020h		; Jump to last portion of boot code

sub_1C0: 	ld	a, 2		; Hard Drive Boot:
		ld	b, 1		; Initialize & restore
		rst	08h		; Do disk I/O
		ret	nz		; Return if any	errors...
		ld	a, 2		; else indicate	HD...
		jr	loc_1D0		; and attempt to boot.

sub_1CA:	ld	a, (4067h)	; Error	code from floppy restore cmd
		or	a
		ret	nz		; If there had been any	errors,	then return...
		inc	a		; A=1 indicates	floppy I/O

loc_1D0:	ld	bc, 0C01h	; 0CH=Read, 01H=sector to read
		ld	(4055h), a	; Store	boot type (1 for floppy, 2 for HD)
		ld	de, 0		; Cylinder to read
		ld	hl, 4300h	; Address of buffer for	sector
		rst	08h		; Do disk I/O
		ret	nz		; Return if any	errors
		ld	a, e		; Length of sector that	was read...
		ld	(4056h), a	; 1 for	256 bytes, 2 for 512 --	store it.
		dec	a		; Was it a 256-byte sector?
		call	z, sub_215	; If so, see if	it needs ROM image
		jr	z, loc_1B6	; If ROM image NOT needed, then	jump & boot...
		ld	a, 0		; else "ROM Image Not Found"...
		ret			; and return (NZ set).

sub_1EB:	ld	hl, 3000h	; Check	if ROM image is	in memory:
		ld	b, 0Ah		; Start	at 3000H
		ld	a, 0C3h		; Opcode for JP instruction

loc_1F2:	cp	(hl)		; Do we	have a JP?
		ret	nz		; If not, no ROM image in memory
		inc	hl		; Point	to third byte following...
		inc	hl		; make sure that the ROM image"s...
		inc	hl		; jump table is	in place.
		djnz	loc_1F2		; Check	ten times in all
		call	403Eh		; Code had been	copied from 0092H to 403EH...
		xor	a		; it gets the word at 000BH from ROM image.
		ld	de, 0E9E1h	; Value	that ought to be there for ROM image
		sbc	hl, de		; Set Z	if ROM image present, else NZ
		ld	hl, 3015h	; Transfer address if image already loaded
		ret

rst_206:	ld	a, 20h ; " "    ; Clear Screen: Put space into A
		ld	hl, 3C00h	; Start	of video memory
		ld	bc, 3FFh	; Length of screen, minus one
		ld	(hl), a		; Put space at start of	screen
		ld	d, h		; Set DE to...
		ld	e, l
		inc	de		; HL plus one.
		ldir			; Voila!
		ret			; The screen has been cleared.

sub_215:	ld	a, (4064h)	; Address for recording	<N> key
		or	a		; Was <N> key pressed?
		jr	nz, loc_228	; If yes, don"t check if ROM image is needed
		ld	hl, 4300h	; Point	to start of the	boot sector we read
		ld	b, 0FEh		; Number of bytes to check

loc_220: 	ld	a, 0CDh		; Opcode for CALL instruction
		cp	(hl)		; Look in boot sector for a CALL
		jr	z, loc_23F	; Jump if we find one...

loc_225:	inc	hl		; else point to	next byte...
		djnz	loc_220		; and keep looking.

loc_228:	call	sub_1EB		; Is ROM image in memory?
		jr	nz, loc_23D	; Jump if not, because we won"t need to...
		ld	a, 1		; destroy the ROM image.
		out	(84h), a	; Write-enable that part of RAM
		ld	hl, 3000h	; Starting at 3000H...
		ld	d, h
		ld	e, l
		ld	(hl), l		; put zeros into...
		inc	de
		ld	bc, 1Eh		; 1EH consecutive bytes...
		ldir			; from 3000H to	301DH.

loc_23D:	xor	a		; Set Z	flag to	indicate that we won"t need...
		ret			; to load the ROM image, and RETurn.

loc_23F:	inc	hl		; Point	to next	byte in	boot sector...
		dec	b		; and decrement	byte counter.
		inc	hl		; Point	to second byte after "CALL"...
		dec	b		; which	is the MSB of the CALL"s destination.
		ld	a, (hl)
		or	a		; Is it	a call to page 0? (CALL	00xxH)
		jr	nz, loc_225	; If not, keep looking through boot sector...
		or	l		; else set NZ to indicate we"ll need the...
		ret			; ROM image, and RETurn.

sub_249:	ld	a, d		; RST 10H -- Display String
		or	3Ch ; "<"       ; Point DE to video memory
		ld	d, a

loc_24D:	ld	a, (hl)		; Get character	to display
		or	a		; Is it	zero?
		ret	z		; If zero, then	we"re done displaying it
		ld	(de), a		; Put character	into video memory
		inc	hl		; Next character to display
		inc	de		; Next byte in video memory
		jr	loc_24D		; Repeat for next character, until done

loc_255:	jr	z, loc_25B	; RST 18H -- Display Message: If Z, then...

loc_257:	ld	e, (hl)		; we already have display position in DE...
		inc	hl		; else HL points to it,	and we must...
		ld	d, (hl)		; read it into DE.
		inc	hl

loc_25B:	ld	b, h		; Now BC points	to the...
		ld	c, l		; table	of strings to display.

loc_25D:	ld	a, (bc)		; Get first value from table into HL
		inc	bc
		ld	l, a
		ld	a, (bc)
		inc	bc
		ld	h, a
		inc	a		; Does MSB = 0FFH?
		jr	nz, loc_270	; Jump if not (display string &	loop again)
		ld	a, l
		inc	a		; Does LSB = 0FFH?
		ret	z		; If it	does, we"re done
		inc	a		; Does LSB = 0FEH?
		jr	nz, loc_270	; Jump if not (display string &	loop again)
		push	bc		; Must be 0FFFEH...load	pointer	to...
		pop	hl		; table	of strings into	HL...
		jr	loc_257		; get display position for next	part of	msg.

loc_270:	rst	10h		; Display string that HL points	to...
		jr	loc_25D		; and repeat for next value in table.
;
	dw	m_error0
	dw	m_error1
	dw	m_error2
	dw	m_error3
	dw	m_error4
	dw	m_error5
	dw	m_error6
	dw	m_error7
	dw	m_error8
	dw	m_error9
	dw	m_error10
	dw	m_error11
	dw	m_error12

m_error0:	
		dw aThe		; Error	0 -- pointers to phrases: "The "
		dw aRomImage	; "ROM Image "
		dw aWasNot	; "Was Not "
		dw aFoundOnDrive; "Found On Drive "
		dw a0		; "0"
		dw 0FFFEh	; End of this part of message
	
		dw 0C0h		; Display next part of msg at row 3, col 0
		dw aRomImage	; "ROM Image "
		dw aNicht	; "Nicht "
		dw aGefundenAuf	; "Gefunden Auf "
		dw aLaufwerk	; "Laufwerk "
		dw a0		; "0"
		dw 0FFFEh	; End of this part of message
	
		dw 180h		; Display next part at row 6, col 0
		dw aImage	; "Image "
		dw aROM		; "ROM "
		dw aAbsent	; "Absent"
		dw aEDu		; "e Du "
		dw aDisque	; "Disque"
		dw aDansLUnit	; " Dans l`Unite 0"
		dw 0FFFFh	; 0FFFFH = End of message
	
m_error1:	
		dw 4EBh		; Error	1: "Arcnet "
		dw 4F3h		; "Boot"
		dw 4F9h		; "Is Not"
		dw 501h		; "Available"
		dw 0FFFEh
		dw 0C0h
		dw 4EBh		; "Arcnet"
		dw 4F3h		; "Boot"
		dw 5E6h		; "Ist Nicht "
		dw 6F0h		; "Im System"
		dw 0FFFEh
		dw 180h
		dw 4F3h		; "Arcnet"
		dw 4EBh		; "Boot"
		dw aAbsent	; "Absent"
		dw 708h		; " Du "
		dw 71Dh		; "Systeme"
		dw 0FFFFh
m_error2:	
		dw aRomImage	; Error	2: "ROM Image "
		dw 50Ch		; "Can`t Be Loaded - "
		dw 51Fh		; "Too Many Extents"
		dw 0FFFEh
		dw 0C0h
		dw aRomImage	; "ROM Image "
		dw 613h		; "Kann "
		dw aNicht	; "Nicht "
		dw 61Dh		; "Geladen "
		dw 653h		; "Werden "
		dw 51Ch		; "- "
		dw 65Bh		; "Zu Viele Bereiche"
		dw 0FFFEh
		dw 180h
		dw aImage	; "Image "
		dw 7DDh		; "ROM "
		dw 726h		; "Non Chargeable "
		dw 51Ch		; "- "
		dw 736h		; "Segments Trop Numbreux"
		dw 0FFFFh
m_error3:	
		dw aThe		; Error	3: "The "
		dw 538h		; "Hard Disk "
		dw 4E2h		; "Drive "
		dw 4F9h		; "Is Not "
		dw 550h		; "Ready "
		dw 0FFFEh
		dw 0C0h
		dw 643h		; "Die Festplatte "
		dw 5E6h		; "Ist Nicht "
		dw 66Dh		; "Bereit "
		dw 0FFFEh
		dw 180h
		dw 74Dh		; "Unite "
		dw aDisque	; "Disque"
		dw 781h		; "-Dur "
		dw 787h		; "Non Prete "
		dw 0FFFFh
m_error4:	
		dw aThe		; Error	4: "The "
		dw 530h		; "Floppy "
		dw 53Dh		; "Disk "
		dw 4E2h		; "Drive "
		dw 4F9h		; "Is Not "
		dw 550h		; "Ready "
		dw 0FFFEh
		dw 0C0h
		dw 5FFh		; "Disketten-Laufwerk "
		dw aNicht	; "Nicht "
		dw 66Dh		; "Bereit "
		dw 0FFFEh
		dw 180h
		dw 74Dh		; "Unite "
		dw aDisque	; "Disque"
		dw 778h		; "-Souple "
		dw 787h		; "Non Prete "
		dw 0FFFFh
m_error5:	
		dw aThe		; Error	5: "The "
		dw 538h		; "Hard Disk "
		dw 4E2h		; "Drive "
		dw 4F9h		; "Is Not "
		dw 501h		; "Available "
		dw 0FFFEh
		dw 0C0h
		dw 643h		; "Die Festplatte "
		dw 5E6h		; "Ist Nicht "
		dw 6F0h		; "Im System"
		dw 0FFFEh
		dw 180h
		dw 74Dh		; "Unite "
		dw aDisque	; "Disque"
		dw 781h		; "-Dur "
		dw aAbsent	; "Absent"
		dw 708h		; " Du "
		dw 71Dh		; "Systeme "
		dw 0FFFFh
m_error6:	
		dw aThe		; Error	6: "The "
		dw 530h		; "Floppy "
		dw 53Dh		; "Disk "
		dw 4E2h		; "Drive "
		dw 4F9h		; "Is Not "
		dw 501h		; "Available "
		dw 0FFFEh
		dw 0C0h
		dw 5FFh		; "Disketten-Laufwerk "
		dw 5E6h		; "Ist Nicht "
		dw 6F0h		; "Im System"
		dw 0FFFEh
		dw 180h
		dw 74Dh		; "Unite "
		dw aDisque	; "Disque"
		dw 778h		; "-Souple "
		dw aAbsent	; "Absent"
		dw 708h		; " Du "
		dw 71Dh		; "Systeme "
		dw 0FFFFh
m_error7:	
		dw 557h		; Error	7: "Close "
		dw aThe		; "The "
		dw 530h		; "Floppy "
		dw 4E2h		; "Drive "
		dw 55Eh		; "Door And Try Again "
		dw 0FFFEh
		dw 0C0h
		dw 5FFh		; "Disketten-Laufwerk "
		dw 675h		; "Schliessen Und Erneut Starten"
		dw 0FFFEh
		dw 180h
		dw 792h		; "Fermez Porte d`Unite Et Reessayez "
		dw 0FFFFh
m_error8:	
		dw 57Dh		; Error	8: "CRC "
		dw 587h		; "Error"
		dw 58Dh		; ", "
		dw 567h		; "Try Again "
		dw 590h		; "Or Use Another "
		dw 53Dh		; "Disk "
		dw 0FFFEh
		dw 0C0h
		dw 57Dh		; "CRC "
		dw 698h		; "Fehler, "
		dw 6A1h		; "Neu Starten..."
		dw 0FFFEh
		dw 180h
		dw 7BBh		; "Erreur "
		dw 7B5h		; "CRC, "
		dw 7AAh		; "Reessayez "
		dw 75Bh		; "Ou Utilisez Une Autre Disque"
		dw 0FFFFh
m_error9:	
		dw 582h		; Error	9: "Seek Error"
		dw 58Dh		; ", "
		dw 567h		; "Try Again "
		dw 590h		; "Or Use Another "
		dw 53Dh		; "Disk "
		dw 0FFFEh
		dw 0C0h
		dw 6FAh		; "Such "
		dw 698h		; "Fehler, "
		dw 6A1h		; "Neu Starten..."
		dw 0FFFEh
		dw 180h
		dw 7BBh		; "Erreur "
		dw 7C3h		; "De Chercher, "
		dw 7AAh		; "Reessayez "
		dw 75Bh		; "Ou Utilisez Une Autre Disque"
		dw 0FFFFh
m_error10:	
		dw aThe		; Error	10: "The "
		dw aRomImage	; "ROM Image "
		dw 50Ch		; "Can`t Be Loaded - "
		dw 530h		; "Floppy "
		dw 53Dh		; "Disk "
		dw 4E2h		; "Drive "
		dw 4F9h		; "Is Not "
		dw 550h		; "Ready "
		dw 0FFFEh
		dw 0C0h
		dw aRomImage	; "ROM Image "
		dw 613h		; "Kann "
		dw aNicht	; "Nicht "
		dw 61Dh		; "Geladen "
		dw 653h		; "Werden "
		dw 51Ch		; "- "
		dw 5FFh		; "Disketten-Laufwerk "
		dw 0FFFEh
		dw 100h		; (second line of German text)
		dw 5E6h		; "Ist Nicht "
		dw 66Dh		; "Bereit "
		dw 0FFFEh
		dw 180h
		dw aImage	; "Image "
		dw 7DDh		; "ROM "
		dw 726h		; "Non Chargeable "
		dw 51Ch		; "- "
		dw 74Dh		; "Unite "
		dw aDisque	; "Disque"
		dw 778h		; "-Souple "
		dw 787h		; "Non Prete "
		dw 0FFFFh
m_error11:	
		dw 572h		; Error	11: "Lost Data "
		dw 587h		; "Error"
		dw 0FFFFh
m_error12:	
		dw 5E2h		; Error	12: "ID "
		dw 587h		; "Error"
		dw 0FFFFh	; End of error messages
		dw 0		; Prompt message after loading ROM image...
		dw aThe		; start	at row 0, col 0: "The "
		dw aRomImage	; "ROM Image "
		dw 5A0h		; "Has Been"
		dw 514h		; " Loaded - "
		dw 5A9h		; "Switch Disks And "
		dw 0FFFEh
		dw 40h		; (second line of message)
		dw 5C7h		; "Press <Enter> "
		dw 5D6h		; "Or <Break> "
		dw 543h		; "When You Are Ready "
		dw 0FFFEh
		dw 0C0h
		dw aRomImage	; "ROM Image "
		dw 619h		; "Ist Geladen "
		dw 51Ch		; "- "
		dw 626h		; "Disketten Wechseln Und Dann "
		dw 5CDh		; "<Enter> "
		dw 0FFFEh
		dw 100h
		dw 6D7h		; "Oder "
		dw 5D9h		; "<Break> "
		dw 6CEh		; "Drucken "
		dw 0FFFEh
		dw 180h
		dw 7D1h		; "l`Image "
		dw 7DAh		; "Du ROM "
		dw 7E2h		; "Chargee "
		dw 51Ch		; "- "
		dw 7EBh		; "Changez De "
		dw aDisque	; "Disque"
		dw 7F7h		; " Et Appuyez Sur "
		dw 0FFFEh
		dw 1C0h
		dw 5CDh		; "<Enter> "
		dw 808h		; "Ou "
		dw 5D9h		; "<Break> "
		dw 80Ch		; "Pour Continuer"
		dw 0FFFFh	; End of message
mLoading:	dw aLoading	; Message: "Loading "
		dw aRomImage	; "ROM Image "
		dw 51Ch		; "- "
		dw 5BBh		; "Please Wait"
		dw 0FFFEh
		dw 0C0h
		dw aRomImage	; "ROM Image "
		dw 6DDh		; "Wird "
		dw 61Dh		; "Geladen "
		dw 51Ch		; "- "
		dw 6E3h		; "Bitte Warten"
		dw 0FFFEh
		dw 180h
		dw 81Bh		; "Chargement De "
		dw 7D1h		; "l`Image "
		dw 7DDh		; "ROM "
		dw 51Ch		; "- "
		dw 82Ah		; "Veuillez Patienter "
		dw 0FFFFh	; End of message

aThe:		db	"The "	; Phrases for error messages start here
		db 0		; Zero byte indicates end of phrase
aLoading:	db "Loading ", 0
aRomImage:	db "ROM "
aImage:		db "Image ",0
aWasNot:	db "Was Not ",0
aFoundOnDrive:	db "Found On Drive ",0
a0:		db "0",0
aArcnet:	db "Arcnet ",0
aBoot:		db "Boot ",0
aIsNot:		db "Is Not ",0
aAvailable:	db "Available ",0
aCanTBeLoaded:	db "Can't Be Loaded - ",0
aTooManyExtents:db "Too Many Extents",0
aFloppy:	db "Floppy ",0
aHardDisk:	db "Hard Disk ",0
aWhenYouAreRead: db "When You Are Ready ",0
aClose:		db "Close ",0
aDoorAndTryAgai: db "Door And Try Again ",0
aLostData:	db "Lost Data ",0
aCrc:		db "CRC ",0
aSeekError:	db "Seek Error",0
aComma:		db ", ",0
aOrUseAnother:	db "Or Use Another ",0
aHasBeen:	db "Has Been",0
aSwitchDisksAnd: db "Switch Disks And ",0
aPleaseWait:	db "Please Wait",0
aPressEnter:	db "Press <Enter> ",0
aOrBreak:	db "Or <Break> ",0
aId:		db "ID ",0

; German phrases
aIstNicht:	db "Ist "
aNicht:		db "Nicht ",0
aGefundenAuf:	db "Gefunden Auf ",0
aDiskettenLaufw: db "Disketten-"
aLaufwerk:	db "Laufwerk ",0
aKann:		db "Kann ",0
aIstGeladen:	db "Ist Geladen ",0
aDiskettenWechs: db "Disketten Wechseln Und Dann ",0
aDieFestplatte:	db "Die Festplatte ",0
aWerden:	db "Werden ",0
aZuVieleBereich: db "Zu Viele Bereiche",0
aBereit:	db "Bereit ",0
aSchlie:	db "Schlie"
		db    04h	; 04H is "s-zet" (double "s" character)
aEnUndErneutSta: db "en Und Erneut Starten",0
aDurda:		db "Durda",0
aFehler:	db "Fehler, ",0
aNeuStartenOder: db "Neu Starten Oder Eine Andere Platte Benutzen",0
aDrCken:	db "Dr",0ah
		db "cken ",0
aOder:		db "Oder ",0
aWird:		db "Wird ",0
aBitteWarten:	db "Bitte Warten",0
aImSystem:	db "Im System",0
aSuch:		db "Such ",0

; French phrases
aAbsent:	db "Absent",0
aEDu:		db "e Du ",0
aDansLUnit:	db " Dans l'Unit"
		db    03h	; 03H is e-acute
a0_0:		db " 0",0
aSyst:		db "Syst"
		db  0Ch		; 0CH is e-grave
aMe:		db "me ",0
aNonChargeable:	db "Non Chargeable ",0
aSegmentsTropNu: db "Segments Trop Numbreux",0
aUnit:		db "Unit"
		db   03h	; 03H is e-acute
		db " ",0
aFrank:		db "Frank ",0
aOuUtilisezUneA: db "Ou Utilisez Une Autre "
aDisque:	 db "Disque",0
aSouple:	db "-Souple ",0
aDur:		db "-Dur ",0
aNonPr:		db "Non Pr"
		db  12h	; 12H is e-circumflex
aTe:		db "te ",0
aFermezPorteDUn: db "Fermez Porte d'Unit"
		db    03h; 03H is e-acute
aEtR:		db " Et R"
		db    03h
aEssayez:	db "essayez ",0
aCrc_0:		db "CRC, ",0
aErreur:	db "Erreur ",0
aDeChercher:	db "De Chercher, ",0
aLImage:	db "l'Image ",0
aDuRom:		db "Du "
aROM:		db "ROM ",0
aCharg:		db "Charg"
		db    03h	; 03H is e-acute
aE:		db "e ",0
aChangezDe:	db "Changez De ",0
aEtAppuyezSur:	db " Et Appuyez Sur ",0
aOu:		db "Ou ",0
aPourContinuer:	db "Pour Continuer",0
aChargementDe:	db "Chargement De ",0
aVeuillezPatien: db "Veuillez Patienter ",0


loc_83E:	ld	a, (3840h)	; RST 30H -- Keyboard scan: Get	column 6
		rrca			; Is someone pressing <ENTER>?
		jr	c, loc_83E	; Loop until user lets go of <ENTER> key

loc_844:	ld	a, (3801h)	; Get col 0
		and	0FEh  	      	; Any of bits 1-7 pressed? (<A>-<G>)
		jr	z, loc_857	; Jump if none of those	were pressed...
		ld	d, 0FFh		; else start with -1...

loc_84D:	inc	d		; and keep adding 1...
		rrca			; until	we find	which keyboard row...
		jr	nc, loc_84D	; was pressed.
		ld	a, 40h ; "@"    ; Add 40H to row number to get...
		add	a, d		; the ASCII code for that key (<A>-<G>).
		ld	(401Dh), a	; Store	ASCII code in name of ROM image	file

loc_857:	ld	a, (3840h)	; Next,	get column 6 of	keyboard matrix
		bit	2, a		; Was <BREAK> pressed?
		jr	z, loc_861	; If not, skip next instruction...
		ld	(405Bh), a	; else put a non-zero value into 405BH.

loc_861:	bit	0, a		; Was <ENTER> pressed?
		jr	nz, loc_8B0	; If yes, then end of keyboard scan
		ld	a, (3802h)	; Column 1
		bit	6, a		; Was <N> pressed?
		jr	z, loc_86F	; Skip next instruction	if not...
		ld	(4064h), a	; else store non-zero value to show it was.

loc_86F:	bit	4, a		; What about the <L> key?
		jr	z, loc_876
		ld	(4066h), a	; Store	non-zero value if <L> was pressed

loc_876:	ld	d, 80h  	; 80H & greater show type of boot requested
		ld	a, (3804h)	; Column 2
		bit	6, a		; Was <V> pressed?
		jr	nz, loc_8A7	; If yes, can"t use any other options
		bit	0, a		; Was <P> pressed?
		jr	z, loc_886	; Skip next instruction	if it wasn"t...
		ld	(4065h), a	; else indicate	that it	was.

loc_886:	inc	d
		inc	d		; D is now 82H
		ld	a, (3880h)	; Column 7: L &	R <SHIFT>, <CTRL>,...
		or	a		; <CAPS>, <F1>,	<F2>, <F3>
		jr	z, loc_894	; Jump if none of those	were pressed

loc_88E:	rrca			; Add row number to D...
		jr	c, loc_8A7	; and jump once	that"s been done.
		inc	d		; So L Shift=82H, R Shift=83H, Ctrl=84H,...
		jr	loc_88E		; Caps=85H, F1=86H, F2=87H, F3=88H.
; 

loc_894:	ld	a, (3810h)	; Column 4
		ld	d, 86h 		;
		bit	1, a		; Was <1> pressed?
		jr	nz, loc_8A7	; If yes, jump with D=86H
		inc	d
		bit	2, a		; Was <2> pressed?
		jr	nz, loc_8A7	; If yes, jump with D=87H
		inc	d
		bit	3, a		; Was <3> pressed?
		jr	z, loc_8AB	; If not, then jump, else D=88H

loc_8A7:	ld	a, d		; value for boot option selected, 82H-88H
		ld	(4059h), a	; Store	it

loc_8AB:	dec	bc		; At entry to RST 30H, BC held...
		ld	a, b		; the number of	times to scan the keyboard.
		or	c		; Decrement it and see if it"s down to 0
		jr	nz, loc_844	; If not, then scan again

loc_8B0:	ld	a, (4059h)	; End of keyboard scan -- get boot option
		or	a		; Set Z	if no options chosen, else NZ.
		ret			; and that's the end of the keyboard scan.

loc_8B5:	dec	a		; RST 08H -- Disk I/O
		jp	z, loc_96F	; If A was 1, then floppy disk...
		ld	a, b		; else use hard	disk.
		cp	1		; What function	was requested?
		jr	nz, loc_8C3	; If not "initialize", skip next 2 lines
		call	sub_956		; Reset	HD controller board
		ld	b, 4		; Change function to "restore"

loc_8C3:	ld	a, c		; Sector to read
		out	(0CBh),	a	; Out to WDC sector register
		in	a, (0CBh)	; See what we get back from there...
		cp	c		; If something different, no HD	attached
		ld	a, 5		; "Hard Drive Not Available"
		jr	nz, loc_8EE	; If no	HD available, then error
		xor	a		; Drive	1, head	0, sectors 256 bytes
		out	(0CEh),	a	; Out to size/drive/head register
		ld	a, d		; Cylinder to read -- LSB
		out	(0CDh),	a	; Out to WDC cylinder low register
		ld	a, e		; Cylinder to read -- MSB
		out	(0CCh),	a	; To WDC cylinder high register
		push	bc		; Save function, sector	during next loop
		ld	d, 8		; Counter for outer loop

loc_8D9:	ld	bc, 0		; Counter for inner loop

loc_8DC:	in	a, (0CFh)	; Get WDC status register
		bit	6, a		; Is drive ready?
		jr	nz, loc_8F0	; Jump if it"s ready...
		dec	bc		; else decr counter for	inner loop...
		ld	a, b
		or	c
		jr	nz, loc_8DC	; and keep trying.
		dec	d		; Decrement counter for	outer loop...
		jr	nz, loc_8D9	; try half a million times (about 6 secs)...
		pop	bc		; before concluding that drive isn"t ready.
		ld	d, 3		; "Hard Drive Not Ready"

loc_8ED:	ld	a, d		; Error: error code into A...

loc_8EE:	or	a		; set NZ to indicate error...
		ret			; and return from RST 8.

loc_8F0:	pop	bc		; Holds	function and sector number
		ld	a, b		; Function into	A
		ld	d, 16h		; WDC "restore" command
		cp	4
		jr	z, loc_8FA	; Jump if function is "restore"...
		ld	d, 70h ; "p"    ; else change WDC command to "seek".

loc_8FA:	ld	a, d		; WDC command...
		out	(0CFh),	a	; out to command register.
		call	sub_962		; Wait until drive not busy & seek complete
		jr	nz, loc_935	; Jump if any errors
		ld	a, b		; Function into	A again...
		cp	0Ch
		jr	nz, loc_933	; if it	wasn't "read sector", then we're done.
		ld	a, 20h ; " "    ; WDC command to read single sector
		ld	e, 0		; E will be counter for	sector length
		out	(0CFh),	a	; Command out to WDC
		call	sub_962		; Wait until drive is not busy
		jr	z, loc_92D	; Jump if no errors reading sector
		ld	d, c		; Save sector number
		call	sub_94C		; Get WDC status
		bit	4, c		; If error NOT "ID not found", then real error
		jr	z, loc_938	; Otherwise, it	may be a 512-byte sector
		ld	a, 20h ; " "    ; Set size/drive/head to 512-byte sectors
		out	(0CEh),	a
		ld	a, d		; Sector number	was saved in D...
		out	(0CBh),	a	; send it to WDC sector	register again.
		ld	a, 20h ; " "    ; WDC command: read single sector
		out	(0CFh),	a
		call	sub_962		; Wait until drive not busy or seek complete
		jr	nz, loc_935	; Jump if any errors
		call	loc_92D		; Get 256 bytes	twice (512-byte	sector)

loc_92D:	ld	bc, 0C8h	; B is byte counter, C is port for input
		inir			; Get 256 bytes	from WDC sector	buffer
		inc	e		; Increment MSB	of byte	counter

loc_933:	xor	a		; Indicate no errors
		ret			; Return from RST 8

loc_935:	call	sub_94C		; Get WDC error	register

loc_938:	ld	d, 8		; "CRC Error"
		bit	6, a
		jr	nz, loc_8ED	; Jump if error	was CRC	data field error
		bit	5, a		; (Documentation says bit 5 is always 0...
		jr	nz, loc_8ED	; if it	ever SHOULD be 1, then same error)
		ld	d, 0Ch		; "ID Error"
		bit	4, a
		jr	nz, loc_8ED	; Jump if ID not found...
		ld	d, 9		; else call it a "Seek Error"
		jr	loc_8ED


sub_94C:	bit	1, a		; Is a command in progress?
		ld	b, a
		call	nz, sub_959	; If so, reset HD controller board
		in	a, (0C9h)	; Get WDC error	register...
		ld	c, a		; and save it in C.
		ret

sub_956:	xor	a		; Hard disk controller board:
		out	(0C1h),	a	; reset	control	register...

sub_959:	ld	a, 10h
		out	(0C1h),	a	; reset	controller board...
		ld	a, 0Ch
		out	(0C1h),	a	; enable controller board...
		ret			; and return.

sub_962:	in	a, (0CFh)	; Get HD status
		bit	7, a
		jr	nz, sub_962	; If HD	is busy, keep looping
		bit	4, a
		jr	z, sub_962	; If seek not complete,	keep looping
		bit	0, a		; Set NZ if error
		ret

loc_96F:	ld	a, 0D0h		; Floppy Disk Driver:
		out	(0F0h),	a	; Terminate command without interrupt
		call	LongDelay		; Delay
		ld	a, 81h		; DDEN, side 0, drive 0
		out	(0F4h),	a	; Out to drive select
		ld	a, c		; Sector
		out	(0F2h),	a	; Out to FDC Sector Register
		ld	a, b		; Function
		cp	5
		jr	nc, loc_9A2	; Jump if function > 4
		ld	a, 0Ch		; Restore -- verify track, 6 ms	step
		out	(0F0h),	a	; Out to FDC Command Register
		call	sub_A64		; Wait until not busy or not ready, get	status
		ld	b, a
		and	0DFh		; Ignore bit 5 ("head loaded")
		cp	81h		; Unless we have "not ready" and "busy"...
		jr	nz, loc_9E2	; then get error code (if any) and return.
		ld	a, c		; FDC status before "not ready"
		and	9Eh		; Did we have ready, not tk 0, no index?
		ld	d, 6		; Error	"Floppy Drive Not Available"
		jr	z, loc_99F	; If so, floppy	drive"s not available.
		ld	d, 4		; Error	"Floppy Drive Not Ready"
		cp	6		; Index	pulse? (Index because no disk there!)
		jr	z, loc_99F	; If so, "Not Ready" (no disk), else...
		ld	d, 7		; no index bec.	disk not turning: "Close Door"

loc_99F:	ld	a, d		; Put error code into A...
		or	a		; set Z/NZ (NZ indicates error)...
		ret			; and return from RST 8.
; 

loc_9A2:	cp	32h ; "2"       ; If function wasn"t "restore all"...
		jr	nz, loc_9CD	; then jump to seek/read code.
		ld	b, 3		; How many drives to restore
		ld	d, 2		; Drive	select for :1

loc_9AA:	ld	a, 0D0h		; Terminate command without interrupt...
		out	(0F0h),	a	; out to FDC.
		call	LongDelay	; Delay	after issuing FDC command
		ld	a, d
		out	(0F4h),	a	; Send out drive select	valuE
		rlca			; Rotate one bit left, to select next drive
		ld	d, a
		in	a, (0F0h)	; Get FDC status
		and	20h ; " "       ; Is head loaded on drive we selected?
		jr	z, loc_9CA	; If it	isn"t, skip to next drive
		xor	a		; Restore, no verify, 6	ms step
		out	(0F0h),	a	; Command out to FDC
		call	ShortDelay	; Delay	after issuing FDC command

loc_9C2:	in	a, (0F0h)	; Get FDC status
		xor	1		; Invert "busy" bit
		and	5		; If not track 0 and command in	progress...
		jr	z, loc_9C2	; then keep checking until it"s done.

loc_9CA:	djnz	loc_9AA		; Loop again for drives	:2 and :3...
		ret			; then return from RST 8.

loc_9CD:	ld	a, e		; Cylinder to read
		out	(0F3h),	a	; Out to FDC data reg (destination of seek)
		ld	a, 1Ch		; Seek -- verify, 6 ms step
		out	(0F0h),	a	; Out to FDC command reg
		call	sub_A64		; Delay, get FDC status
		ld	d, a		; Status into D
		and	98h		; Get not ready, seek error, CRC error bits
		jr	nz, loc_9E1	; If any of these, don"t try to read sector
		ld	a, b		; Function
		cp	0Ch
		jr	z, loc_A00	; Jump if it"s "read sector"

loc_9E1:	ld	b, d		; FDC status into B

loc_9E2:	ld	d, 9		; Code for "Seek Error"
		jr	loc_9EE		; Continue with	error routine

loc_9E6:	ld	d, 0Bh		; "Lost Data"
		bit	2, b		; Did we lose any data?
		jr	nz, loc_99F	; Jump if yes (error into A, set NZ, RET)
		ld	d, 0Ch		; "ID Error"

loc_9EE:	bit	4, b		; Record not found?
		jr	nz, loc_99F	; Jump if so (Seek:"Seek Err", Read:"ID Err")
		ld	d, 8		; "CRC Error"
		bit	3, b
		jr	nz, loc_99F	; Jump if that"s the case
		ld	d, 4		; "Floppy Drive Not Ready"
		bit	7, b
		jr	nz, loc_99F	; Jump if drive	wasn"t ready
		xor	a		; Else no errors
		ret
; 

loc_A00:	ld	a, (405Ch)	; Lowest sector	number,	0 or 1
		ld	b, a
		ld	d, 81h		; Drive 0, side 0, DDEN
		ld	c, 86h		; Read sector, compare for side 0
		in	a, (0F2h)	; FDC sector register -- sector	to read
		cp	12h		; Is sector number less	than 18?
		jr	c, loc_A14	; Jump if it is	-- must	be SS disk...
		sub	12h		; else it"s DS -- subtract 18 from sector no.
		set	4, d		; Set drive select for side 1
		set	3, c		; Set FDC cmd to compare for side 1

loc_A14:	add	a, b		; Add 1	if it"s a TRSDOS 1.3 disk
		out	(0F2h),	a	; Sector number	to FDC sector register
		ld	a, d		; Drive	select value...
		out	(0F4h),	a	; out to drive select latch.
		ld	b, 0		; B will be number of bytes read -- LSB
		ld	e, b		; E will be MSB	-- set both to 0
		ld	a, c		; FDC command into A
		ld	c, 0F3h		; C points to FDC data register (for INI)
		out	(0F0h),	a	; Send out FDC command
		ld	a, 80h		; Allow NMI on FDC interrupt request
		out	(0E4h),	a	; Out to NMI mask
		call	ShortDelay		; Delay	after issuing command to FDCK
		ld	a, d
		out	(0F4h),	a	; Send drive select value out again

loc_A2C:	in	a, (0F0h)	; FDC status
		bit	1, a		; Does FDC have	any data for us?
		jp	nz, loc_A39	; Jump if it does
		rlca			; Test bit 7 of	FDC status
		jp	nc, loc_A2C	; If drive is ready, keep waiting for DRQ...
		jr	loc_A4D		; otherwise it"s an error -- clean up & return

loc_A39:	ini			; FDC has data...get the first byte
		ld	a, d		; Drive	select value
		or	40h ; "@"       ; Set "wait"

loc_A3E:	out	(0F4h),	a	; Out to drive select
		ini			; Get byte from	FDC (decr B & set Z/NZ)
		jr	nz, loc_A3E	; If not 256 bytes yet,	get next one...
		inc	e		; else incr MSB	of byte	counter...
		jp	loc_A3E		; and loop until NMI occurs.
; 

loc_A48: pop iy		; NMI: Discard ret addr	(it pts	to loop	above)
		xor	a
		out	(0E4h),	a	; Don"t allow any NMIs

loc_A4D:	in	a, (0F0h)	; Get FDC status...
		ld	b, a		; into B and C.
		ld	c, b
		call	nullsub_1	; Call a RETN and return to next line
		jr	loc_9E6		; Set error code, if any, & return from	RST 8

nullsub_1:	retn			; Reset	interrupt flip-flops


LongDelay:	push	bc		; Delay	loop
		ld	b, 0		; The long delay is about 800 usec.
		jr	DelayLoop

ShortDelay:	push	bc		; Another delay	loop
		ld	b, 12h		; This one is about 60 usec.

DelayLoop:	djnz	$		; Repeat this instruction until	B is 0...
		pop	bc		; then restore BC and return from delay.
		ret

sub_A64:	call	ShortDelay	; Call short delay

loc_A67:	in	a, (0F0h)	; Get FDC status
		bit	0, a
		ret	z		; Return if not	busy (done with	command)
		bit	7, a
		ret	nz		; Return if not	ready
		ld	c, a		; Otherwise save FDC status...
		jr	loc_A67		; and loop until not busy or not ready.

sub_A72:	ld	a, (4067h)	; Load MODEL%/III file:	get disk error...
		or	a		; from floppy drive restore command.
		jr	nz, loc_AD8	; Jump if there	was an error
		ld	a, 1		; Sector 1...
		ld	e, 0		; cylinder 0...
		call	sub_C0B		; read it into sector buffer at	4300H.
		jr	nz, loc_AD8	; Jump if any errors
		ld	a, e		; Length of sector we just read
		cp	1		; 1 = 256 bytes, 2 = 512 bytes
		jr	nz, loc_A8E	; If not 256 then jump ("Not Found" error)
		ld	a, (4300h)	; Get first byte of sectoRr
		or	a		; Zero on 5.x/6.x disks
		jr	z, loc_A91	; If zero, then	jump, "cause 5.x/6.x is OK
		cp	0FEh 		; Is it 0FEH? (found on TRSDOS 1.3 disks)

loc_A8E:	ld	a, 0		; "ROM Image Not Found"  -- If not T1.3...
		ret	nz		; then return w/error (strange disk)

loc_A91:	ld	a, c		; FDC status from Read Sector call
		and	20h ; " "       ; Get record type (Data Address Mark)
		ld	c, 1		; Assume TRSDOS	1.3 -- first sector is 1
		ld	b, 3		; Three	sectors	per granule
		ld	hl, (4301h)	; Get bytes 01,	02 from	boot sector
		ld	a, l		; Dir cyl is byte 01 of	boot sector
		jr	nz, loc_AA2	; Jump if "deleted" DAM (T1.3 disk) or...
		ld	a, h		; adjust for 5.x/6.x --	dir cyl	is byte	02
		dec	c		; Lowest sector	number is 0
		ld	b, 6		; Six sectors per granule

loc_AA2:	ld	(405Dh), a	; Store	directory cylinder
		ld	e, a		; Directory cylinder value into	E
		ld	a, b		; Sectors per granule
		ld	(4063h), a	; Store	it
		ld	a, c		; Lowest sector	number
		ld	(405Ch), a	; Store	it
		ld	a, (405Ch)	; Get it again
		or	a
		jr	nz, loc_AC4	; Jump if TRSDOS 1.3 disk (always SS)...
		xor	a		; otherwise it"s a 5.x/6.x disk.
		call	sub_C0B		; Read sector 0	of dir cyl (GAT)
		jr	nz, loc_AD8	; Jump if any errors
		ld	a, (43CDh)	; Get configuration byte (GAT+x"CD")
		and	20h ; " "       ; Number of sides
		ld	a, 1		; Indicate single-sided
		jr	z, loc_AC4	; Jump if that"s the case...
		inc	a		; else there are two sides.

loc_AC4:	ld	(405Fh), a	; Whatever it is, store	number of sides
		ld	a, 2		; Sector 2 (of directory)
		ld	(405Eh), a	; Store	as next	sector to read

loc_ACC:	ld	hl, (405Dh)	; Dir cyl, sector number
		ld	e, l		; Cylinder into	E
		ld	a, h		; Sector into A
		call	sub_C0B		; Read it
		ld	a, 0		; "ROM Image Not Found"
		jr	z, loc_AE0	; Jump if no errors

loc_AD8:	cp	4		; Errors while loading ROM image:
		jr	nz, locret_ADF	; If not "Floppy Drive Not Ready" then jump
		ld	a, 0Ah		; Else chg to "Can`t Load Image - Not Ready"
		or	a		; Set NZ (unless error 0, "Image Not Found")

locret_ADF:	ret

loc_AE0:	ld	ix, 4300h	; Point	to first directory entry in sector

loc_AE4:	ld	a, (ix+0)	; Get first byte of entry
		bit	7, a		; Is it	an extended dir	record?
		jr	nz, loc_B11	; If so, skip it
		bit	4, a		; Is this record in use?
		jr	z, loc_B11	; If not, skip it
		push	ix
		pop	hl
		ld	de, 5
		add	hl, de		; HL points to (DIR+5) -- file name
		ld	de, 4018h	; Point	to "MODEL%   " (or MODELA - MODELG)
		ld	b, 8		; Compare eight	bytes of our filename...
		call	sub_BF6		; with filename	in directory entry.
		jr	nz, loc_B11	; If no	match, then skip to next entry
		push	ix		; Still	points to start	of entry
		pop	hl
		ld	de, 0Dh
		add	hl, de		; HL points to (DIR+13)	-- file	extension
		ld	b, 3		; Number of bytes to compare
		ld	de, 63h	; "c"   ; Point to "III"
		call	sub_BF6		; Compare them
		jr	z, loc_B36	; If they match, we"ve found it...

loc_B11:	push	ix		; otherwise try	next directory entry.
		pop	hl
		ld	de, 20h	; " "
		add	hl, de		; HL points to 20H bytes past first entry
		ld	a, (405Ch)	; Get disk type
		or	a
		jr	z, loc_B22	; Jump if it"s 5.x/6.x...
		ld	de, 10h		; else move pointer 10H	bytes more because...
		add	hl, de		; T1.3 has 30H bytes between dir entries.

loc_B22:	push	hl
		pop	ix		; Points to next dir entry
		ld	a, l
		or	a		; Is pointer at	next page of memory?
		jr	z, loc_B2D	; Jump if it is
		cp	0F0h 		; Is it at byte 0F0H (past last T1.3 entry)?
		jr	nz, loc_AE4	; Jump if not...

loc_B2D:	ld	a, (405Eh)	; else we"ve seen all entries in sector.
		inc	a		; Incr sector counter and store	it, then...
		ld	(405Eh), a	; try next dir sector. (If past	end of dir...
		jr	loc_ACC		; sect not found, get "Image Not Found" error)


loc_B36:	ld	de, 40A2h	; We found ROM image dir entry, point	to
		push	ix		; our buffer for the directory record.
		pop	hl		; HL now points	to the directory entry
		ld	bc, 30h	; "0"   ; Number of bytes to copy
		ldir			; Copy directory record	into our buffer
		ld	a, 0FEh		; Put 0FEH after directory record, so we...
		ld	(de), a		; stop there if	we"re still reading the file.
		ld	ix, 40B6h	; Point	to first extent	field and continue

loc_B48:	ld	e, 0		; RST 28H - Load Object Code: E = byte	offset

loc_B4A:	rst	20h		; Get byte from	file
		ret	nz		; Return if we can"t get a byte
		dec	a		; What did we get?
		jr	z, loc_B5A	; Jump if it was 01H (load block)
		dec	a
		jr	z, loc_B6B	; Jump if it was 02H (transfer address)
		rst	20h		; Else get another byte	-- length of block
		ld	b, a		; Use it as a counter for comment block

loc_B54:	rst	20h		; Get byte and discard it
		ret	nz		; Return if error getting byte
		djnz	loc_B54		; Loop until we"ve gotten all of block...
		jr	loc_B4A		; then get next	block in file.

loc_B5A:	rst	20h		; Load block --	get block length
		ret	nz		; Return if error
		dec	a		; Decrement by two to allow for	load address
		dec	a
		ld	b, a		; Store	number of bytes	to load
		call	sub_B6D		; Get address for loading into HL
		ret	nz		; Return if error

loc_B63:	rst	20h		; Get byte to load
		ret	nz		; Return if error...
		ld	(hl), a		; else load into memory...
		inc	hl		; point	to next	byte in	memory...
		djnz	loc_B63		; and loop until end of	block.
		jr	loc_B4A		; Get next block in file
; 

loc_B6B:	rst	20h		; Transfer addr	-- get block length & discard
		ret	nz		; Return if error

sub_B6D:	rst	20h		; Get LSB of address
		ret	nz
		ld	l, a		; Put it in L
		rst	20h		; Get MSB of address
		ret	nz
		ld	h, a		; Put it in H
		ret			; End of RST 28H - transfer address in HL

loc_B74:	push	bc		; RST 20H (initially) -- Get Byte from File
		push	hl		; Save BC, HL -- used by RST 28H
		ld	a, e		; Byte offset in current sector
		or	a
		jr	nz, loc_B82	; If not zero, we don"t need to read a sector
		call	z, sub_B88	; If it	IS zero, read the next sector of file
		jr	nz, loc_B85	; Jump if error	reading	sector...
		ld	de, 4300h	; else point to	start of sector	we just	read.

loc_B82:	xor	a		; Set Z	to indicate success
		ld	a, (de)		; Get byte from	sector buffer
		inc	de		; Point	to next	byte in	sector buffer

loc_B85:	pop	hl		; Restore registers, and return
		pop	bc
		ret

sub_B88:	ld	hl, 4060h	; Point	to number of sectors remaining...
		ld	a, (hl)		; in current extent, and get value in A.
		or	a
		jr	nz, loc_B93	; Jump if add"l sectors in current extent...
		call	sub_BB7		; else calculate next extent...
		ret	nz		; and return if	there"s a problem.

loc_B93:	dec	(hl)		; Okay...one less unread sector	in extent
		ld	b, 12h		; One more than	highest	sector number
		ld	a, (405Fh)	; Get number of	sides on disk
		dec	a
		jr	z, loc_B9E	; Jump if SS disk...
		ld	b, 24h ; "$"    ; else new limit for sector number.

loc_B9E:	ld	a, (405Eh)	; Number of last sector	we read
		cp	b		; Had we reached end of	cylinder?
		jr	nz, loc_BAC	; If not, skip next four lines...
		ld	a, (4062h)	; else get cylinder number...
		inc	a		; point	to next	cylinder...
		ld	(4062h), a	; and store new	cylinder number.
		xor	a		; Next sector read is on new cylinder

loc_BAC:	ld	c, a		; Last sector number we	read
		inc	a		; Point	to next	sector...
		ld	(405Eh), a	; and store it.
		ld	a, (4062h)	; Get cylinder number...
		ld	e, a		; and put it in	E.
		jr	loc_C0C		; Jump -- read sector and return from call

sub_BB7:	inc	ix		; Find next extent of file -- point to...
		inc	ix		; next extent of directory info.
		ld	a, (ix+0)	; Get first byte of extent field
		ld	b, a
		and	0FEh		; Drop bit 0
		cp	0FEh		; Is it 0FEH or 0FFH?
		jr	nz, loc_BC9	; Jump if it isn"t...
		ld	a, 2		; else it"s "Too Many Extents" error.
		or	a		; Set NZ to indicate error, and	return
		ret

loc_BC9:	ld	a, b		; First	byte of	extent field is	starting...
		ld	(4062h), a	; cyl of extent	-- store as current cyl.
		ld	a, (ix+1)	; Second byte of extent	field
		ld	e, a		; Store	in E
		and	0E0h 		; Bits 5,6,7 only -- relative gran in cyl
		rlca			; Get value in lowest three bits...
		rlca
		rlca
		ld	b, a		; and store it in B.
		ld	a, (4063h)	; Get sectors/gran value...
		call	sub_C02		; and multiply to get starting sector.
		ld	(405Eh), a	; Store	as last	sector we read
		ld	a, e		; Second byte of extent	field
		and	1Fh		; Get bits 0-4,	number of grans	in extent
		ld	b, a		; Store	in B
		ld	a, (405Ch)	; Get flag for disk type
		xor	1		; Change to 0 for DOS 1.x, 1 for 5.x/6.x
		add	a, b		; Add to # grans in extent, because...
		ld	b, a		; 5.x/6.x uses 0 to indicate 1 gran.
		ld	a, (4063h)	; Get sectors/gran value...
		call	sub_C02		; and multiply by # grans in extent...
		ld	(4060h), a	; to get # sectors in extent, which we store.
		xor	a		; Set Z	for success (we	found next granule)...
		ret			; and return.

sub_BF6:	ld	a, (de)		; Get character	from our filename
		cp	25h ; "%"       ; Is it "%" (wildcard)?
		jr	z, loc_BFD	; If yes, then anything	matches...
		cp	(hl)		; otherwise compare with char in dir entry.
		ret	nz		; Return if they don"t match...

loc_BFD:	inc	de		; else point to	next character of both...
		inc	hl
		djnz	sub_BF6		; and continue comparing.
		ret			; Returns with Z set if	they match

sub_C02:	ld	c, a		; Multiply -- arguments	in A & B, result in A
		ld	a, b
		or	a		; Multiplying by zero?
		ret	z		; If so, we already have the answer
		xor	a		; Start	with zero

loc_C07:	add	a, c		; Add original A to it...
		djnz	loc_C07		; B times.
		ret

sub_C0B:	ld	c, a		; Sector number	into C

loc_C0C:	ld	hl, 4300h	; Address of sector buffer in HL
		ld	b, 0Ch		; Function -- read sector
		ld	a, 1		; Floppy disk
		rst	8		; Do Disk I/O
		ret			; and Return

ArcnetError:	ld	a, 1		; Arcnet boot: No Arcnet boot ROM...
		jp	FatalError	; so it"s Error 1 (and halt).

RS232Boot:	ld	a, 4		; RS-232 boot (Network 3) starts here
		ld	(4055h), a	; Use 4	to indicate RS-232 boot

loc_C1F:	ld	a, 64h 		; Set 8/N/1, assert DTR & RTS
		out	(0EAh),	a	; Out to UART &	modem control register
		or	a		; Set NZ to get	screen address for message
		ld	hl, 0CD4h	; Point	to phrases for "Not Ready "
		rst	18h		; Display it

loc_C28:	in	a, (0E8h)	; Get modem status
		and	20h ; " "       ; Bit 5 -- Carrier Detect
		jr	nz, loc_C28	; Loop until we	have a carrier
		ld	de, 0		; Top left corner of screen
		ld	hl, 0CDCh	; Three	spaces,	to cover up "Not"
		rst	10h		; Display them,	so screen says "    Ready "
		ld	hl, 0CA4h	; Point	to "Get byte from RS-232" routine
		ld	(400Ah), hl	; Make this address our	RST 20H	vector
		ld	hl, 4057h	; Point	to baud	rate storage

loc_C3E:	ld	a, 0FFh		; Start	at 19.2K bits per second

loc_C40:	ld	(hl), a		; Store	baud rate
		out	(0E9h),	a	; Output to baud rate generator
		out	(0E8h),	a	; Reset	UART
		ld	b, 0Ah		; Number of tries to receive a byte

loc_C47:	rst	20h		; Try to receive a byte
		jr	nz, loc_C4E	; If unsuccessful, then	retry or give up
		cp	55h ; "U"       ; Did we get a "U"?
		jr	z, loc_C57	; If yes, then continue...

loc_C4E:	djnz	loc_C47		; else retry up	to our limit...
		ld	a, (hl)		; then get baud	rate...
		sub	11h		; and drop down	to next	lower rate.
		jr	nc, loc_C40	; If not below slowest rate, then try it...
		jr	loc_C3E		; else start again at 19.2K bps.

loc_C57:	ld	b, 0Ah		; Number of bytes to receive

loc_C59:	rst	20h		; Get a	byte from RS-232
		jr	nz, loc_C3E	; If error, start again
		cp	55h ; "U"       ; Did we get a "U"?
		jr	nz, loc_C3E	; If not, start	again...
		djnz	loc_C59		; else get another byte.
		ld	a, (hl)		; Okay,	we got ten "U"s...now get the...
		and	0Fh		; baud rate we used...
		call	sub_CBE		; convert to ASCII & store. (This sets NZ)
		ld	hl, 0CE0h	; "Found Baud Rate " + character
		rst	18h		; Display it
		ld	hl, 0CC4h	; "Found Baud Rate "
		call	sub_CB0		; Transmit message via RS-232
		in	a, (0EBh)	; Clear	transmit holding register

loc_C74:	rst	20h		; Receive a byte
		jr	nz, loc_C8B	; If error, go to RS-232 error routine
		cp	0FFh		; Did we receive 0FFH?
		jr	nz, loc_C74	; If not, keep trying until we do.
		ld	de, 244h	; Display at row 9, col	0
		ld	hl, aLoading	; "Loading "
		push	hl		; Save pointer to this message
		rst	10h		; Display it on	our screen
		pop	hl		; Still	points to "Loading "
		call	sub_CB0		; Send it out RS-232
		rst	28h		; Receive file from RS-232, load into memory
		jr	nz, loc_C8B	; If error, go to RS-232 error routine...
		jp	(hl)		; else start executing the file	we received.

loc_C8B:	rrca			; RS-232 error:	Rotate UART status 3 bits...
		rrca			; to the right -- bit 0	will then indicate...
		rrca			; overrun, bit 1 framing, bit 2	parity.
		call	sub_CBE		; Convert to ASCII ("A"-"H") & store (sets NZ)
		call	rst_206		; Clear	the screen (sets BC to 0)
		push	bc
		ld	hl, 0CE8h	; "Error " + character for error encountered
		rst	18h		; Display it
		ld	hl, 587h	; "Error"
		call	sub_CB0		; Transmit it
		pop	bc		; Still	0, so scan the keyboard	65536 times...
		rst	30h		; to let the other system time out and...
		jp	loc_C1F		; re-sync, then	we try the RS-232 boot again.

loc_CA4:	in	a, (0EAh)	; UART & modem control register
		bit	7, a		; Character received?
		jr	z, loc_CA4	; If not, loop until we	receive	one.
		and	38h 		; Test for overrun, framing, parity errors
		ret	nz		; Return if any	errors...
		in	a, (0EBh)	; else get character from UART...
		ret			; and return with Z set	to indicate success.

sub_CB0:	in	a, (0EAh)	; UART & modem control register
		and	40h ; "@"       ; Transmit register empty?
		jr	z, sub_CB0	; If not, loop until it	is.
		ld	a, (hl)		; Get character	to transmit
		or	a		; Is it	zero?
		ret	z		; If zero, we"re done transmitting...
		out	(0EBh),	a	; else send character to UART xmit register...
		inc	hl		; point	to next	character...
		jr	sub_CB0		; and repeat.

sub_CBE:	add	a, 41h ; Convert value 0-15 to ASCII...
		ld	(4068h), a	; store	it...
		ret			; and return.

aFoundBaudRate:	db "Found Baud Rate "
		db 0
		db 0
		dw 4D4h			; "Not "
		dw 550h			; "Ready "
		dw 0FFFFh		; End of message
		db "   "	        ; Three spaces, to erase "Not"
		db 0 
		dw 184h			; Display at row 6, col	0
		dw 0CC4h		; "Found Baud Rate "
		dw 4068h		; ASCII	character for baud rate	("A"-"P")
		dw 0FFFFh		; End of message
		dw 3C4h			; Display at bottom left corner	of screen
		dw 587h			; "Error"
		dw 58Dh			; ", "
		dw 4068h		; Character for	error
		dw 0FFFFh		; End of message
; 

loc_CF2:	ld	sp, 3FFEh	; RAM Test: Put	stack in video memory
		ld	iy, 3C80h	; Point	to row 2, col 0	of screen
		ld	a, 8		; 2 MHz	clock, disable I/O bus,...
		out	(0ECh),	a	; enable alt char set.
		xor	a		; Set 0000H-37FFH as read-only,...
		out	(84h), a	; Model	III memory map.
		ld	ix, 0D14h	; Address to go	to after clearing screen.

loc_D04: 	ld	a, 20h ; " "    ; The routine starting here...
		ld	de, 3C00h	; is called from...
		ld	(de), a		; several locations...
		ld	h, d		; in the RAM test.
		ld	l, e		; It clears the	screen...
		ld	bc, 3FFh	; and then...
		inc	de		; jumps	to the...
		ldir			; address contained in...
		jp	(ix)		; the IX register.
; 
		ld	hl, 0ED2h	; "Dynamic RAM Test"
		ld	de, 3C00h	; Top of screen
		call	sub_249		; Display it
		ld	hl, 0EE3h	; "Press <ENTER> To Begin..."
		ld	de, 3C40h	; Next row on screen
		call	sub_249		; Display it
		ld	hl, 0F0Ch	; "Errors will be displayed"
		ld	de, 0C0h 	; Next row on screen
		call	sub_249		; Display it

loc_D2F:	ld	a, (3840h)	; Column 6 of keyboard matrix
		cp	1		; Is <ENTER> pressed?
		jr	nz, loc_D2F	; Loop until it	is.
		ld	ix, 0D3Dh	; Address of next part of test
		jp	loc_D04		; Clear	screen,	then continue.
; 
		ld	hl, 0F32h	; Column headings for display
		ld	de, 3C80h	; Row 2, col 0 of screen
		call	sub_249		; Display it
		ld	hl, 0F92h	; "Stack"
		ld	de, 3F4h	; Row 15, col 52 of screen
		call	sub_249		; Display it

loc_D4F:	ld	hl, 0F56h	; "Test Pattern is 55h"
		ld	de, 3C40h	; Row 1, col 0 of screen
		call	sub_249		; Display it
		ld	hl, 4000h	; Fill memory from 4000H...
		ld	d, h		; to 0FFFFH...
		ld	e, l		; with 55H (01010101 binary).
		ld	bc, 0BFFFh
		ld	a, 55h ; "U"
		ld	(de), a
		inc	de
		ldir
		ld	hl, 4000h	; Start	checking memory	at 4000H
		ld	bc, 0BFFFh	; Number of bytes to check

loc_D6C:	ld	a, 55h ; "U"    ; Compare 55H...
		ld	e, (hl)		; with the value we find in memory.
		cp	e		; If different,	something"s wrong with memory.
		ld	ix, 0D77h	; Where	to resume if we	jump
		jp	nz, loc_E0A	; If memory error, then	display	it
		dec	bc		; One less byte	left to	check
		inc	hl		; Point	to next	byte to	check
		ld	a, c		; Have we checked all the bytes?
		or	b
		jr	nz, loc_D6C	; If not, then check the next one
		ld	hl, 0F6Ah	; Else point to	"AAh"
		ld	de, 3C50h	; Row 1, col 16	of screen
		call	sub_249		; Display it, so we see	"Test Pattern is AAh"
		ld	hl, 4000h	; The same test	again, except...
		ld	d, h		; this time...
		ld	e, l		; we fill...
		ld	bc, 0BFFFh	; 4000H	to 0FFFFH...
		ld	a, 0AAh		; with 0AAH (10101010 binary).
		ld	(de), a
		inc	de
		ldir
		ld	hl, 4000h
		ld	bc, 0BFFFh	; Compare contents of memory...
loc_D9A:	ld	a, 0AAh	; with 0AAH this time.
		ld	e, (hl)
		cp	e
		ld	ix, 0DA5h
		jp	nz, loc_E0A	; Jump if memory error,	resume at next line
		dec	bc
		inc	hl
		ld	a, c
		or	b
		jp	nz, loc_D9A	; Loop until we"ve tested all the bytes.
		ld	d, 0		; Mask for testing each	byte

loc_DAE:	ld	b, d		; Save mask in B for a moment
		ld	hl, aModifiedAddres	; "Modified Address Test  ld (hl),mask"
		ld	de, 3C40h	; Second line of screen
		call	sub_249		; Display it (overwrite	"Test Pattern...")
		ld	h, d		; Address of last character displayed...
		ld	l, e		; into HL.
		ld	d, b		; Mask back into D
		inc	hl		; Point	to next	space on screen.
		rra			; There	should be a LD A,D before this...
		rra			; to get the mask value	into A.	 See text.
		rra			; Anyway, move the upper nybble	of the...
		rra			; "mask" value into the lower nybble.
		and	0Fh		; Get lower (formerly upper) 4 bits only
		add	a, 90h 		; Convert to one ASCII character...
		daa			; using	the usual algorithm...
		adc	a, 40h ; "@"    ; so we can display the top half...
		daa			; of the "mask" value.
		ld	(hl), a		; Put character	on screen.
		inc	hl		; Point	to next	space on screen.
		ld	a, d		; Get mask value
		and	0Fh		; Lower	nybble only
		add	a, 90h 		; Convert to one ASCII character again
		daa
		adc	a, 40h ; "@"
		daa
		ld	(hl), a		; Display 2nd char of mask value
		ld	hl, 4000h	; Start	testing	at 4000H
		ld	bc, 0BFFFh	; Number of bytes to test

loc_DDA:	ld	a, d		; Get mask value...
		xor	l		; XOR with LSB of address...
		xor	h		; and with MSB of address...
		ld	(hl), a		; and put it into memory.
		inc	hl		; Loop until...
		dec	bc
		ld	a, c
		or	b
		jr	nz, loc_DDA	; we"ve tested all the bytes.
		ld	a, d		; Mask value
		and	3		; Is mask divisible by four?
		jr	nz, loc_DED	; If not, skip next two	lines...
		ld	a, 40h ; "@"    ; else run the drive motors a bit to put...
		out	(0F4h),	a	; add'l load on power supply, I suppose.

loc_DED:	ld	hl, 4000h	; Start	testing	at 4000H again
		ld	bc, 0BFFFh	; number of bytes to test

loc_DF3:	ld	a, d		; Mask...
		xor	l		; XOR lsb of address...
		xor	h		; and msb of address...
		ld	e, (hl)		; should match what we put there...
		cp	e		; a moment ago.
		ld	ix, 0DFEh	; Where	to resume if error
		jr	nz, loc_E0A	; If memory error, display it &	resume
		inc	hl		; Loop until we've...
		dec	bc
		ld	a, c
		or	b
		jr	nz, loc_DF3	; tested all the bytes...
		inc	d		; then increment mask...
		jr	nz, loc_DAE	; and test again, unless mask was up to	0FFH.
		jp	loc_D4F		; If it	was, repeat entire test, endlessly.

loc_E0A:	ex	af,af'         ; Memory Error display routine:
		exx			; Use alternate	AF, BC,	DE, HL
		ld	bc, 40h	; Number of characters on one line of screen
		add	iy, bc		; Point	to next	line on	screen
		ld	bc, 0C040h	; (0C040H = negative 3FC0H)
		add	iy, bc		; Did IY point to the last line...
		jr	nc, loc_E1C	; of the screen?  Jump if it didn"t...
		ld	iy, 0		; else keep it at the bottom line.

loc_E1C:	ld	bc, 3FC0h	; Cancels out addition of 0C040H
		add	iy, bc
		exx			; Restore original BC,DE,HL
		ld	a, h		; High byte of address of defective byte
		rra			; Move upper four bits down
		rra
		rra
		rra
		and	0Fh		; Use upper (now lower)	4 bits only
		add	a, 90h  ; Convert to one ASCII character
		daa
		adc	a, 40h ; 
		daa			; Display character under "Address" heading...
		ld	(iy+1),	a	; at col 1 of current row.
		ld	a, h		; Repeat for high byte of address...
		and	0Fh		; lower	4 bits this time.
		add	a, 90h ; Convert to ASCII
		daa
		adc	a, 40h ; 
		daa
		ld	(iy+2),	a	; Display at col 2 of current line
		ld	a, l		; Repeat for low byte of address
		rra
		rra
		rra
		rra
		and	0Fh
		add	a, 90h ; 
		daa
		adc	a, 40h ; 
		daa
		ld	(iy+3),	a
		ld	a, l
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+4),	a	; Display in cols 3 & 4	of current line
		ld	a, (hl)		; Read contents	of problem address again
		ld	i, a		; Use I	register as temporary storage
		rra			; Convert it to	ASCII
		rra
		rra
		rra
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+0Ah), a
		ld	a, i
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa			; Display under	"Retry" heading...
		ld	(iy+0Bh), a	; in cols 10,11	of current line.
		ld	a, e		; Value	originally read	from problem address
		rra			; Convert to ASCII, etc.
		rra
		rra
		rra
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+11h), a
		ld	a, e
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+12h), a	; Display under	"Read" heading, cols 17,18
		ex	af,af'         ; Restore original AF register...
		ld	i, a		; save it in I for a moment...
		rra			; this is the value we expected	to find...
		rra			; at the problem address, and didn"t.
		rra			; Convert to ASCII, etc.
		rra
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+17h), a
		ld	a, i
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa			; Display under	"Written" heading...
		ld	(iy+18h), a	; in columns 23	& 24.
		ld	a, d		; Mask value used at problem address
		rra			; Convert to ASCII, etc.
		rra
		rra
		rra
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+20h), a
		ld	a, d
		and	0Fh
		add	a, 90h ;
		daa
		adc	a, 40h ;
		daa
		ld	(iy+21h), a	; Display under	"Mask" heading, cols 32,33
		jp	(ix)		; Resume test at address in IX
; 
aDynamicRamTest:	db	"Dynamic RAM Test"
		db	0
aPressEnterToBe:	db	"Press <Enter> To Begin And RESET To Exit"
	db 0
aAnyErrorsDetec:	db	"Any Errors Detected Will Be Displayed"
	db 0
aAddressRetryRe:	db	"Address  Retry  Read  Written  Mask"
	db 0
aTestPatternIs5:	db	"Test Pattern Is 55h"
	db 0
aAah:		db	"AAh"
	db 0
aModifiedAddres:	db "Modified Address Test  ld (hl),mask"
		db	0
aStack:		db	"Stack"

 IF FREHD

	include ../frehd.inc

frehd_boot:
	ld	a,18h			; for XTRS
	out	(CONTROL),a
	call	frehd_boot1		; no return if sucess
	jp	sub_1C0			; hard drive boot (previous code path)
frehd_boot1:
	call	frehd_loader		; get loader
	ret	nz			; return if error
	jp	ROM_LOAD
frehd_loader:
	ld	a,ROM_MODEL_4P
	out	(WRITEROM),a
	in	a,(READROM)
	cp	0FEh
	ret	nz
	ld	hl,ROM_LOAD
	ld	bc,READROM
	inir
	xor	a
	ret

;;; patch loc_180 to load model III rom from FreHD (sub_A72 loads from floppy)
frehd_iii:
	call	frehd_load_iii		; try loading rom from FreHD
	ret	z			; sucess, return !
	ld	a,(4067h)		; failed, use previous code path
	or	a
	jp	nz,FatalError
	jp	loc_186
frehd_load_iii:	
	call	frehd_loader		; try to load the loader
	ret	nz			; failed => return
	ld	a, 1			; write-enable 0000-37FF
	out	(84h),a			; read boot rom, write to ram
	ld	de,fname
	ld	b,fnamlen
	call	5004h			; try to load the model III rom file
	ret				; return with Z (success) or NZ (failed)
	


fname	db	MODEL_III_FILE
fnamend	equ	$
fnamlen	equ	(fnamend-fname)
	
 ENDIF

	REPT (0FEBh - $)
	db	0
	ENDM

	ORG 0FEBh

aTandyCorp: db	"(c) 1983, Tandy Corp."
; end of "ROM"


		end

