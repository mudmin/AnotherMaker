

	CPU z80
;;;
;;; Use Model 1 L2 Version 1.3 ROM :
;;;

	BINCLUDE model1.rom
	INCLUDE ../frehd.inc
	

;;;
;;; Replace Mem Size? and R/S Level 2 Messages
;;;
;;;
;;;

;; ROM:		 ld hl,l0105h		;00b5	21 05 01 	! . . 
	ORG 00b5h
	ld	hl,message		; new address of MEM SIZE? message
	
;; ROM:			ld hl,l010eh	;00fc	21 0e 01 	! . . 

	ORG 00fch
	jp	1a19h		; return to ready routine, basic command mode

;; ROM:			call sub_28a7h	;00ff	cd a7 28 	. . ( 
;; ROM:			jp l1a19h		;0102	c3 19 1a 	. . . 
;; ROM:			ld c,l			;0105	4d 	M 
;; ROM:			ld b,l			;0106	45 	E 
;; ROM:			ld c,l			;0107	4d 	M 
;; ROM:			jr nz,l015dh	;0108	20 53 	  S 
;; ROM:			ld c,c			;010a	49 	I 
;; ROM:			ld e,d			;010b	5a 	Z 
;; ROM:			ld b,l			;010c	45 	E 
;; ROM:			nop				;010d	00 	. 
;; ROM:			ld d,d			;010e	52 	R 
;; ROM:			cpl				;010f	2f 	/ 
;; ROM:			ld d,e			;0110	53 	S 
;; ROM:			jr nz,l015fh	;0111	20 4c 	  L 
;; ROM:			ld (04220h),a	;0113	32 20 42 	2   B 
;; ROM:			ld b,c			;0116	41 	A 
;; ROM:			ld d,e			;0117	53 	S 
;; ROM:			ld c,c			;0118	49 	I 
;; ROM:			ld b,e			;0119	43 	C 
;; ROM:			dec c			;011a	0d 	. 
;; ROM:			nop				;011b	00 	. 


;; now 29 bytes free here !

	ORG 00ffh
hd_boot:	
	ld	a,ROM_MODEL_1		; 00ff  ROM parameter to FreHD. 1 for Model1.
	out	(WRITEROM),a		; 0101  2
	in	a,(READROM)		; 0103  2
	cp	0FEh			; 0105  FE is FreHD present and valid code!
	ret	nz			; 0107  return if no FreHD or old firmware
	ld	hl,ROM_LOAD		; 0108  load more "rom" from FreHD
	ld	bc,READROM		; 010b
	inir				; 010e
	jp	ROM_LOAD
message:
	db 4dh				; M 
	db 45h				; E 
	db 4dh				; M 
	db 20h
	db 53h				; S 
	db 49h				; I 
	db 5ah				; Z 
	db 45h				; E 
	db 00h				
	IF $ > 011ch
	ERROR "Code too big"
	ENDIF
	
;; ROM:		ld b,027h			;0685	06 27 	. ' 
;; ROM:		ld (de),a			;0687	12 	. 
;; ROM:		inc de				;0688	13 	. 
;; ROM:		djnz l0687h			;0689	10 fc 	. . 
;; ROM:		ld a,(03840h)		;068b	3a 40 38 	: @ 8 
;; ROM:		and 004h			;068e	e6 04 	. . 
;; ROM:		jp nz,l0075h		;0690	c2 75 00 	. u . 
;; ROM:		ld sp,0407dh		;0693	31 7d 40 	1 } @ 
;; ROM:		ld a,(037ech)		;0696	3a ec 37 	: . 7 
;; ROM:		inc a				;0699	3c 	< 
;; ROM:		cp 002h				;069a	fe 02 	. . 
;; ROM:		jp c,l0075h			;069c	da 75 00 	. u . 
;; ROM:		ld a,001h			;069f	3e 01 	> . 
;; ROM:		ld (037e1h),a		;06a1	32 e1 37 	2 . 7 
;; ROM:		ld hl,037ech		;06a4	21 ec 37 	! . 7 
;; ROM:		ld de,037efh		;06a7	11 ef 37 	. . 7 
;; ROM:		ld (hl),003h		;06aa	36 03 	6 . 
;; ROM:		ld bc,l0000h		;06ac	01 00 00 	. . . 
;; ROM:		call l0060h			;06af	cd 60 00 	. ` . 

;; 06ac - bc is already zero - see 0685-0689, 
;; save 3 bytes	ORG 0693h

	ORG 0696h
	call	hd_boot     ;0696
	ld a,(037ech)		;0699	3a ec 37 	: . 7 
	inc a				;069c	3c 	< 
	cp 002h				;069d	fe 02 	. . 
	jp c,0075h	        ;069f	da 75 00 	. u . 
	ld a,001h			;06a2	3e 01 	> . 
	ld (037e1h),a		;06a4	32 e1 37 	2 . 7 
	ld hl,037ech		;06a7	21 ec 37 	! . 7 
	ld de,037efh		;06aa	11 ef 37 	. . 7 
	ld (hl),003h		;06ad	36 03 	6 . 
	call 0060h			;06af	cd 60 00 	. ` . 
	
