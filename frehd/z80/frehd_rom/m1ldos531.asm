	LISTING ON
	NEWPAGE
;*=*=*
;	M1 LDOS 5.3.1  BOOT/SYS PATCH
;*=*=*

;;
;; This is remains a work in progress
;; BOOT/SYS is loaded and finds SYS0/SYS correctly 
;; SYS0/SYS is loaded at 4200 and the transfer address executes 
;; but the DCT is not correctly patched at this stage
;;

m1ldos531:

	START_PATCH 6A00h
;*=*=*
;	routine to read a sector from HD
;	D = cylinder  E = sector
;*=*=*
m1_read_sector_DE
	push	bc			;used as m1 load address
	push	hl
	push	de
	ld	h,b
	ld	l,c
	ld	c,e
	ld	e,d
	ld	d,00h
	ld	b,0ch			; not needed (4P like)
	ld	a,02h			; not needed (4P like)
	call	hd_read_sector
	pop	de
	pop	hl
	pop 	bc 			
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
m1_dct_read_hd
	ld	a,09h			; command READ SECTOR ?
	sub	b
	jr	z,m1_read_sector_DE	; yes, do it !
	xor	a			; no, return ok.
	ret

	END_PATCH


	START_PATCH 4709h
	db	0			; force directory to 0 for now - DCT 
	END_PATCH

	START_PATCH 4213h
	db 0,0,0
	END_PATCH
 
	START_PATCH 4277h	
	jr m1_pp2
	END_PATCH

	START_PATCH 4280h
	db 0,0,0
	call 42ach
	END_PATCH

	START_PATCH 428bh
	sub 12h
	END_PATCH


	START_PATCH 42ach
	call	m1_read_sector_DE
	ret

m1_pp2	
	ld 	h,a
	call	42ach	
	push	hl			; save SYS0 entry point
	ld	hl,m1_pdct		; load DCT
	ld	de,4701h
	ld	bc,0009h	
	ldir
;	ld	hl,m1_sys0_531		; patch !
;	call	patch
	pop	hl			; restore entry point
	jp	(hl)			; and jump

m1_pdct	
	dw	m1_dct_read_hd
	db	0ch, 90h, 00h, 98h, 1fh, 0e3h, 65h
	END_PATCH


m1_sys0_531:
	START_PATCH 4e7ch
	;;ld	a,(4202h)	;;m1 directory cyl
	;;ld	(4709h),a
	nop
	nop	
	nop	
	;;jr	4e84h
	END_PATCH

;	START_PATCH 4049h
;	db	0c3h
;	END_PATCH

	LAST_PATCH
