	listing on
	newpage
;*=*=*
;	Model 1 NEWDOS 2.5 boot/sys patch
;   borrowed from www.trs80.nl/nhdd.htm#boot sector_
;	boot for newdos/80 harddisk for the trs-80 model 1
;	written 5-7-1993, code by Rence 
;
;	position of sys0.sys matches spg1 value. spg1 must be < 20h (32 dec)
;	revised 09-Dec-2013 dean.bear@gmail.com
;*=*=*

 
nd1_rdcmd 	equ		20h	  			; read one sector command
nd1_dct_buf	equ 	5200h			; location of pdrive DCT buffer
nd1_buffer	equ 	5300h			; location of sector buffer
nd1_dct		equ		02h				; pdrive is on sector 2 cyl 0
;display_	equ		0033h			; rom display for m1,m3
;etx		equ		03h				; etx
;lf			equ		0ah				; l/f
;cr			equ		0dh				; c/r
;cls		equ		1fh				; clear screen
;tof		equ		1ch				; top of field

m1newdos25:

;;	byte 4302h contains the starting lump of the directory,
;;	and matches PDRIVE parameter ddsl1. ddsl1 is the relative 
;;  lump whose first sector is the beginning of the volume's directory.
;;       
;;  ddsl1 actually supports values between 4 and 124 (dec);
;;	the value selected MUST be divisible by 4 to ensure
;;	correct alignment, and successful boot. 
;;
;;  ** the use of incorrect custom ddsl1 values will cause the 	***
;;  ** boot to fail with a 'hard disk error'  message          	***
;;
;;	NOTE: xtrs will update reed header byte 1f (31 dec) with the 
;;      correct ddsl1 value after a format of the 1st partition.
;;      HEADER BYTE 1f IN THE v1.0 STANDARD IS DEPRECATED BUT XTRS STILL CORRECTLY  
;;      MAINTAINS THIS BYTE - so it should continue to match the value @ 4302h, and 
;;	    position 3 in the unpatched boot/sys on the disk @ partition0, head0, sector0  
;;
;; ** now supports boot partitions with a sectors per granule (spg1) >= 5. ** 
;;

	START_PATCH 4203h	
;
; phase one
; load pdrive table into nd1_dct_buffer
;
		di							; must disable interrupts for M3
		ld		sp,41e0h			; set up a new stack 
		ld		de,nd1_dct			; sector 02 - NEWDOS PDRIVE (DCT)
		ld		hl,nd1_dct_buf		; pdrive buffer start
		push	hl					; DCT is loaded at beginning of buffer 
		pop		iy					; setup iy for indexing DCT
		exx
		ld		l,0ffh				; pdrive buffer end
		call	nd1_getbyte			; read sector 02

;
; phase two
; calculate starting location of sys0/sys using spg1 value for drive 0
;
		ld		a,(iy+03h)			; get spg1 value from pdrive DCT
		ld		d,00h				; cylinder 0
		ld		e,a					; spg1 value is also the physical sector for sys0/sys

;		
; phase three
; load SYS0/SYS into nd1_buffer then store or execute
;
		
		ld		hl,nd1_buffer		; keep DCT in nd1_dct_buffer, load sys0/sys into nd1_buffer
		exx	
		ld		hl,53ffh			; buffer start + 255 
-		call	nd1_getbyte			; read next byte
		cp		20h 				; 
		jr		nc,nd1_error_ 
		ld		c,a					; record type 
		call	nd1_getbyte 
		ld		b,a					; number of bytes 
		ld		a,c 
		cp		01h					; record type 1 (program code) 
		jr		z,+					; nd1_rectyp1 
		cp		02h					; record type 2 (program entry) 
		jr		z,++				; nd1_rectyp2		
; 
;	must be a comment (record types 0, 3 - 20h) 
; 
-		call	nd1_getbyte 
		djnz	- 
		jr		--					; all read 
; 
;	record type 01 = program code 
; 
+		call	nd1_rd_addr			; address in de 

-		call	nd1_getbyte 
		ld		(de),a				; store program data 
		inc		de					; next ram address 
		djnz	-					; nd1_rtlp01 
		jr		--- 
; 
;	record type 02 = program entry 
; 
+		call	nd1_rd_addr			; address in de 
		ld		a,(de)				; test byte in sys0 
		cp		0a5h				; 10100101 magic byte. must be a5h or start is aborted 'no sys'
		jr		nz,nd1_no_sys  		; no_system error message 
		inc		de					; program entry 
		push	de					; save de (entry) executed by ret
;
;   patch DCT loaded by sys0/sys, before jumping to newdos entry 
;
		ld		(iy+0dh),0			; reset byte 0d value from loaded pdrive DCT ?
		ld		de,0ff3bh			; patch sys0/sys loaded newdos pdrive table with hdd table	
		ld		hl,5200h			; source pdrive still loaded at 5200h	
		ld		bc,0010h			; copy 16 bytes to destination pdrive table for drive 0
		ldir
;		
; make a few adjustments before transferring control to newdos
; before changes are made
; 4d00:   a5 ed 56 21 45 ff 06 b9 36 00 23 10 fb 21 ff f8

; after changes
; 4d00:   a5 ed 56 21 4a ff 06 b4 36 00 23 10 fb 21 ff f8
;		
; 4d03  21 4a ff     ld   hl,ff4ah
; 4d06  06 b4        ld   b,b4h
; 4d08  36 00        ld   (hl),00h
; 4d0a  23           inc  hl
; 4d0b  10 fb        djnz 4d08h
;
		ld 		a,04ah				; adjust area to clear 
		ld		(4d04h),a			; first address to zero
		ld		a,0b4h				; byte count
		ld		(4d07h),a			; number of bytes to zero
		ret							; execute program  
; 
;	program load or entry address in de 
; 
	nd1_rd_addr:
		call	nd1_getbyte 
		dec		b					; count one byte 
		ld		e,a					; low address 
		call	nd1_getbyte 
		dec		b					; count one byte 
		ld		d,a					; high address 
		ret 
; 
;	error messages 
; 
	nd1_error_:
		ld		hl,nd1_err_msg		; address of error message 
		jr		nd1_display			; nd1_display text ->(hl) 
	nd1_no_sys:
		ld		hl,nd1_no_sysm 
	nd1_display:
		ld		a,(hl) 
		inc		hl 
		cp		etx					 
-		jr		z,-
		call	display_ 
		jr		nd1_display			; next byte of message  
	nd1_err_msg:
		defb	tof				
		defb	cls				 
		defb    "hard disk error"
		defb	etx				
	nd1_no_sysm:
		defb	tof				 
		defb	cls				 
		defb	"no system on hard disk"
		defb	etx				
	; 
	;	  read byte routine (general part...), de = sector to read, hl = buffer 
	; 
	nd1_getbyte:
		inc		l					; if 00 --> read next sector_ 
		ld		a,(hl) 
		ret		nz 
		exx 
		ld		b,05				; retry count 
-		call	nd1_hw_read			; hard disk read 
		jr		z,+ 
		djnz	- 					; time out retry again
		exx						 	
		jp		nd1_error_ 			; fail

+		call	nd1_nextsec			; set to read next sector 
		exx 
		ld		a,(hl) 
		ret 
	; 
	; 
	;	 read byte routine (western digital) 
	; 
	nd1_nextsec:
		inc 	e					; inc sector, assume sequential sectors across cylinders
		ld		a,e
		cp		20h					; overflow 20h sectors per cylinder
		ret		nz					; ok if less than 20h (32)
		inc		d					; bump cylinder
		ld		e,00h				; reset sector count
		ret 
		
	nd1_hw_read:
		ld		a,e
		out		(SECNUM),a			; send sector number to wd1002-05
		xor		a					; drive 0, head 0, sector 256 bytes
		out		(SDH), a			; send size/drive/head
		out		(CYLHI), a			; also send cyl high 0
		ld		a, d				; MSB cylinder to read
		out		(CYLLO), a
		ld		a,nd1_rdcmd			; read single SECNUM
		out		(COMMAND),a			; send command
		push	bc
		push	hl
-		in 		a, (STATUS)
		rlca						; busy bit -> C
		jr 		c, -
		in 		a, (STATUS)			; read status again
		and 	01h					; nz = error
		jr		nz,+
		ld		bc, DATA			; b = 0 == get 256 bytes
		inir						; input 256 bytes from port (c) to (hl)		
+		pop		hl
		pop		bc
		ret
		db		0,0,1ch,0
	END_PATCH

	LAST_PATCH
