;;;
;;; FreHD ROM LOADER
;;;
;;; This is called from ROM if the FreHD is present and detected.
;;; ROM write <val> to port C4h on FreHD, then reads port C5h. If FEh
;;; is returned, the ROM code reads 256 bytes from FreHD, places them
;;; at address 5000h and calls here. These 256 bytes are this code.
;;;
;;; Note that the <val> written at port C4h is passed back in the code
;;; in the 2nd byte. This is done so that the ROM can pass one parameter
;;; to FreHD.
;;;
;;; This code tries to open frehd.rom on the SD-card. If found, the
;;; file is loaded into memory at address 6000h, and the code flow
;;; continues there. If not found, or if an error occurs, the code
;;; returns and the rom will continue to try booting from floppy.
;;; 
	
	CPU	Z80


;; Interface defines
DATA2	  	equ 0c2h
SIZE2     	equ 0c3h
COMMAND2  	equ 0c4h
ERROR2    	equ 0c5h
STATUS    	equ 0cfh
OPENFILE  	equ 03h
READFILE  	equ 04h
WRITEFILE 	equ 05h
CLOSEFILE 	equ 06h
;; FatFS flags
FA_OPEN_EXISTING equ 00h
FA_READ          equ 01h
FA_WRITE         equ 02h
FA_CREATE_NEW    equ 04h
FA_CREATE_ALWAYS equ 08h
FA_OPEN_ALWAYS   equ 10h
	

	ORG	5000h

	db	0feh

	PHASE	5000h

	cp	0ffh			; trick to pass one parameter from ROM
	jr	+			; skip the jump table

	;; 5004h : file loader
	jr	file_loader
	

+	ld	a, (3840h)		; check keyboard
	and	80h			; <SPACE> pressed ?
	ret	nz			; return if so

	ld	de,fname		; load FREHD.ROM
	ld	b,fnamlen
	call	file_loader
	ret	nz
	jp	(hl)

fname	db	"FREHD.ROM"
fnamend	equ	$
fnamlen equ	(fnamend-fname)




;;;
;;; FreHD file loader
;;;
;;; Input:
;;; - DE : pointer to filename
;;; - B  : filename length
;;;
;;; Return:
;;; - Z  : no error. HL contains transfer address
;;; - NZ : error occurred, file not or partially loaded
;;;
file_loader:
	ld	hl,0			; save SP in IX
	add	hl,sp
	push	hl
	pop	ix

	ex	hl,de
	ld	a,OPENFILE		; open file command
	out	(COMMAND2),a
	call	wait
	ld	a,b
	inc	a			; +1 for options
	inc	a			; +1 for null-terminator
	out	(SIZE2),a
	ld	c,DATA2
	ld	a,FA_OPEN_EXISTING|FA_READ
	out	(c),a			; send option
	otir				; send filename
	xor	a			; send null-terminator
	out	(c),a
	call	wait
	ret	nz			; error, return

	ld	c,0FFh			; counter
load
	call	getbyte			; get type code
	dec	a
	jr	nz,+			; bypass if not type 1
	call	getaddr			; get address
-	call	getbyte
	ld	(hl),a			; save that byte
	inc	hl
	djnz	-
	jr	load
	
+	dec	a			; test if type 2 (TRADDR)
	jr	z,+			; yes
	call	getbyte			; assume comment
	ld	b,a			; get comment length
-	call	getbyte
	djnz	-			; read and ignore
	jr	load

+	call	getaddr			; get transfer address
	ld	a,CLOSEFILE		; close file
	out	(COMMAND2),a
	call	wait
	xor	a			; set Z for success !
	ret
	
getaddr
	call	getbyte			; get block length
	ld	b,a
	call	getbyte			; get low-order load addr
	ld	l,a
	dec	b			; adjust length
	call	getbyte			; get high-order load addr
	ld	h,a
	dec	b			; adjust length
	ret
	
getbyte
	inc	c			; i++
	jr	nz,+			; every 256 bytes, need another chunk
	call	getchunk
+	in	a,(DATA2)		; get a byte
	ret

getchunk
	xor	a			; request 256 bytes
	out	(SIZE2),a
	ld	a,READFILE
	out	(COMMAND2),a
	call	wait
	jr	nz,error
	in	a,(STATUS)		; DRQ set means we got something
	and	08h
	ret	nz			; return if ok !
error
	ld	a,CLOSEFILE		; close file
	out	(COMMAND2),a
	call	wait
	ld	sp,ix
	inc	a			; set NZ (wait returns A == 0 or 1)
	ret				; return

wait	in 	a, (STATUS)
	rlca
	jr 	c, wait
	in 	a, (STATUS)		; read status again
	and 	01h			; nz = error
	ret
	

	IF $ > 50ffh
	ERROR "Code too big!"
	ENDIF
