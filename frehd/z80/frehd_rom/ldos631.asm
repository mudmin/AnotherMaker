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
ldos631:
	
;; 4300  00           nop
	START_PATCH 4302h
	db	65h
;; 4301  fe 65        cp	65h		;# directory track location
	END_PATCH
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
	START_PATCH 4341h
	jr	+
-	push	hl
	ld	hl,43a0h
	ld	de,0471h		; DCT0+1
	jp	patch_loader_631
	nop
	nop
+
;; 4341  1e 00        ld	e,00h
;; 4343  cd 74 43     call	4374h
;; 4346  3a cd 12     ld	a,(12cdh)
;; 4349  e6 20        and	20h
;; 434b  21 74 04     ld	hl,0474h
;; 434e  b6           or	(hl)
;; 434f  77           ld	(hl),a
	END_PATCH
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
	START_PATCH 436fh
	ld	hl,0238h		; entry in SYS0
	jr	-
;; 436f  c3 38 02     jp	0238h
;; 4372  00           nop
;; 4373  00           nop
	END_PATCH
;; 4374  21 00 12     ld	hl,1200h
;;
;; 	;; routine to read a sector
;;	
;; 4374  21 00 12     ld	hl,1200h	; set buffer
;; 4377  06 05        ld	b,05h		; init retry counter
;; 4379  c5           push	bc		; save counter
;; 437a  e5           push	hl		; save for retries
	START_PATCH 437bh
	call	read_631
	pop	hl
	pop	bc
	nop
	nop
;; 437b  cd 96 43     call	4396h
;; 437e  e1           pop	hl
;; 437f  c1           pop	bc
;; 4380  e6 1c        and	1ch
	END_PATCH
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
;; 4396  01 f4 81     ld	bc,81f4h	; Set DDEN,DS1,d.s.port
;; 4399  ed 41        out	(c),b		; Select it
;; 439b  0d           dec	c		; Point C to data register
;; 439c  3e 18        ld	a,18h		; Seek command (6 msec)
	START_PATCH 439eh
	nop
	jp	+
	db	0ch, 90h, 00h, 0cah, 1fh, 0e3h ; zz zz zz zz zz zz

+	ld	a,09h
	sub	b
	jr	nz,+
	di
	call	read_631
	ei
	ld	a,05h
	ret	nz
	ld	a,(0479h)
	sub	d
	ld	a,06h
	jr	z,++
+	xor	a
+	and	a
	ret
read_631
	push	hl
	push	de
	ld	c,e
	ld	e,d
	ld	d,00h
	ld	a,c
	ld	b,0ch
	ld	a,02h
	call	hd_read_sector
	pop	de
	pop	hl
	ret

patch_loader_631
	ld	bc,0008h				; continue from above
	ldir
	ld	hl,loader_631
	call	patch
	ret
	nop
	nop
	nop
	END_PATCH	
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
;; 43e0               db 'Disk error'
;; 43ea               db 'No system '
;; 43f4  00           nop
	START_PATCH 43f5h
	ld	(bc),a
	inc	bc
	ex	de,hl
	add	hl,hl
	ex	de,hl
	sub	20h			; ww
	ret	c
	ld	c,a
	inc	de
	ret
;; 43f5  00           nop
;; 43f6  00           nop
;; 43f7  00           nop
;; 43f8  00           nop
;; 43f9  00           nop
;; 43fa  00           nop
;; 43fb  00           nop
;; 43fc  00           nop
;; 43fd  00           nop
;; 43fe  00           nop
;; 43ff  00           nop
	END_PATCH
	
	LAST_PATCH


;*=*=*
;	Loader patch
;*=*=*

;;                00122 ;       Boot loader routine read in by ROM, along with
;;                00123 ;       the lowcore I/O drivers.
;;                00124 ;       This section loads in SYSRES
;;                00125 ;
;; 0238  FD217004 00126 LBOOT   LD      IY,DCT$         ;Set IY for FDCDVR use
;; 023C  FD7E09   00127         LD      A,(IY+9)        ;Directory track is
;; 023F  FD7705   00128         LD      (IY+5),A        ;  the current track
;; 0242  3E04     00129         LD      A,4
;; 0244  327B00   00130         LD      (FLGTAB$+'R'-'A'),A     ;Set retries
;; 0247  3EC9     00131         LD      A,0C9H
;; 0249  320E00   00132         LD      (FDDINT$),A     ;Return for disk driver
;; 024C  3E12     00133         LD      A,18            ;5" sectors/track, dden
;; 024E  FDCB046E 00134         BIT     5,(IY+4)        ;Double sided?
;; 0252  2801     00135         JR      Z,NOTDBL
;; 0254  87       00136         ADD     A,A             ;Adjust to 36 sect/cyl
;; 0255  32A502   00137 NOTDBL  LD      (SECTRK),A
;;                00138 ;
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
	
loader_631:

	START_PATCH 24Ch
	ld	a,20h			; xx: sector per logical cylinder
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ld	(02a5h),a
	exx
	ld	c,10h			; yy: sector per granule
	END_PATCH

	START_PATCH 29eh
	call	read_631
	END_PATCH

	START_PATCH 264h
	jp	5FF0h
	END_PATCH

	START_PATCH 5FF0h
	ld	(0f95h),a
	push	hl
	ld	hl,sys0sys_631
	call	patch
	pop	hl
	jp	(hl)
	END_PATCH

	LAST_PATCH
	
	
;*=*=*
;	LDOS 6.3.1 SYS0/SYS patch
;*=*=*
sys0sys_631:

;; 1E00           02548         ORG     1E00H+START$
;;                02549 ;
;; 1E00  F3       02550         DI
;; 1E01  21E90F   02551         LD      HL,@RSTNMI      ; Reset NMI vector to
;; 1E04  226700   02552         LD      (@NMI+1),HL     ;   SYSRES's needs
;;                02553 ;
;;                02554         IF      @PCERV
;;                02555 ;
;;                02556 ;       Changes for Pete Cervasio's logo stuff - keep the code the same
;;                02557 ;       as much as possible, even though a simpler bit of code would
;;                02558 ;       work just as well...
;;                02559 ;
;; 1E07  211E04   02560         LD      HL,PAKNAM$+14   ;Point to O/S revision info
;; 1E0A  11E0F8   02561         LD      DE,VERROW*80+VERCOL+CRTBGN$
;; 1E0D  010200   02562         LD      BC,2            ; 2 chars to move
;; 1E10  EDB0     02563         LDIR                    ; Display the info...
;; 1E12  0E08     02564         LD      C,8
;; 1E14  13       02565         INC     DE
;; 1E15  13       02566         INC     DE
;; 1E16  0000     02567         DW      0               ; Take the space of the LDIR
;;                02568 ;
;;                02569         ELSE
;;                02570 ;
;;                02571         LD      HL,PAKNAM$      ; Point to disk pack name
;;                02572         LD      DE,PACKROW*80+PACKCOL+CRTBGN$
;;                02573         LD      BC,8
;;                02574         LDIR                    ; Move pack name to screen
;;                02575         LD      C,8             ; B already holds a 0
;;                02576         INC     DE              ; Leave two spaces
;;                02577         INC     DE
;;                02578         LDIR                    ; Move pack date to crt
;;                02579 ;
;;                02580         ENDIF
;;                02581 ;
;;                02582 ;---> 6.3.1 changes from version 6.2.0 in "The Source"
;;                02583 ;
;; 1E18  13       02584         INC     DE
;; 1E19  13       02585         INC     DE              ; Leave two more spaces
;; 1E1A  0E12     02586         LD      C,18            ; 18 characters
;; 1E1C  218521   02587         LD      HL,M2185        ; Point to message area
;; 1E1F  00       02588 M1E1F   NOP                     ; 6.3.0 LDIR to put the
;; 1E20  00       02589         NOP                     ;  serial # up on screen
;;                02590 ;
;;                02591 ;<--- end of 6.3.1 changes
;;                02592 ;
;;                02593 ;       Initialization routines
;;                02594 ;
;; 1E21  AF       02595         XOR     A               ;Clear out stack area
;; 1E22  218103   02596         LD      HL,STACK$+1     ;Start STACK+1
;; 1E25  2D       02597 CLRLOOP DEC     L               ;Move down a byte
;; 1E26  77       02598         LD      (HL),A          ;Now loop and fill
;; 1E27  20FC     02599         JR      NZ,CLRLOOP      ;  with zero bytes
;;                02600 ;
;; 1E29  ED56     02601         IM      1               ;Set the interrupt mode
	START_PATCH 1e2bh
	nop
	nop
	nop
;; 1E2B  318003   02602         LD      SP,STACK$       ;Set the stack area
	END_PATCH
;; 1E2E  AF       02603         XOR     A
;; 1E2F  320202   02604         LD      (LBANK$),A      ;Set logical bank #
;; 1E32  D3E4     02605         OUT     (0E4H),A        ;Disable INTRQ & DRQ
;;                02606 ;
;; 1E34  213802   02607         LD      HL,S1DCB$
;; 1E37  77       02608 ZERDCB  LD      (HL),A          ;Zero spare DCB area
;; 1E38  2C       02609         INC     L
;; 1E39  20FC     02610         JR      NZ,ZERDCB
;;                02611 ;
;; 1E3B  3A7600   02612         LD      A,(MODOUT$)     ;Set high speed
;; 1E3E  D3EC     02613         OUT     (0ECH),A        ;  and external BUS
;; 1E40  3A8000   02614         LD      A,(WRINT$)
;; 1E43  D3E0     02615         OUT     (0E0H),A        ;Enable RTC interrupts
;; 1E45  3A7800   02616         LD      A,(OPREG$)      ;Set memory configuration
;; 1E48  47       02617         LD      B,A
;; 1E49  3EA7     02618         LD      A,0A7H          ;Value for AUX/RAM
;; 1E4B  0E84     02619         LD      C,@OPREG        ;Set memory mgt port
;; 1E4D  ED41     02620         OUT     (C),B           ;Bring up regular RAM
;; 1E4F  21FFFF   02621         LD      HL,-1           ;Check for extended RAM
;; 1E52  220E04   02622         LD      (HIGH$),HL
;; 1E55  221C00   02623         LD      (PHIGH$),HL
;;                02624 ;
;;                02625 ;       Check the banks
;;                02626 ;
;; 1E58  56       02627         LD      D,(HL)          ;Save what's in RAM
;; 1E59  3655     02628         LD      (HL),55H        ;Stuff in regular RAM
;; 1E5B  ED79     02629         OUT     (C),A           ;Switch in alt RAM
;; 1E5D  5E       02630         LD      E,(HL)          ;Save the byte there, too
;; 1E5E  77       02631         LD      (HL),A          ;Stuff alt RAM
;; 1E5F  ED41     02632         OUT     (C),B           ;Back to reg RAM
;; 1E61  BE       02633         CP      (HL)            ;What's there now?
;; 1E62  72       02634         LD      (HL),D          ;Put reg RAM byte back
;; 1E63  ED79     02635         OUT     (C),A           ;Back to alt RAM
;; 1E65  73       02636         LD      (HL),E          ;Restore original byte
;; 1E66  ED41     02637         OUT     (C),B           ;Back to reg RAM
;; 1E68  3EFE     02638         LD      A,0FEH          ;Init BAR$ for bank 0
;; 1E6A  2802     02639         JR      Z,MZMZ01        ;Bypass if only 64k
;; 1E6C  3EF8     02640         LD      A,0F8H          ;Init BAR$ for bank 0-2
;; 1E6E  320102   02641 MZMZ01  LD      (BAR$),A        ;Load bank available ram
;; 1E71  320002   02642         LD      (BUR$),A        ;Load bank used ram
;; 1E74  3A6F00   02643         LD      A,(FEMSK$)      ;Get port FEh mask
;; 1E77  D3FE     02644         OUT     (0FEH),A        ; and set it
;; 1E79  00       02645         DC      3,0             ;Space for a jump or call
;;       00 00 
;;                02646 ;
;;                02647 ;       Update DCT$ info for SYSTEM drive
;;                02648 ;
	START_PATCH 1e7ch
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ld	a,0d0h
	out	(0f0h),a
	ld	b,80h
	djnz	1e8dh
;; 1E7C  3A9D43   02649         LD      A,(BOOTST$)     ;Get boot step rate
;; 1E7F  E603     02650         AND     3               ;Strip all but it
;; 1E81  47       02651         LD      B,A             ;Save temporarily
;; 1E82  217304   02652         LD      HL,DCT$+3       ;Point to DCT step
;; 1E85  7E       02653         LD      A,(HL)          ;Get DCT step
;; 1E86  E6FC     02654         AND     0FCH            ;Strip step rate
;; 1E88  B0       02655         OR      B               ;Merge boot step
;; 1E89  77       02656         LD      (HL),A          ;Update DCT
;; 1E8A  DBF1     02657         IN      A,(TRKREG)      ;Update DCT with current
;; 1E8C  327504   02658         LD      (DCT$+5),A      ;  track position of head
	END_PATCH
;;                02659 ;
;; 1E8F  110802   02660         LD      DE,KIDCB$       ;Flush type ahead, init ptrs
;; 1E92  3E03     02661         LD      A,3
;; 1E94  CD2306   02662         CALL    @CTL
;; 1E97  FB       02663         EI                      ;Interrupts on
;;                02664 ;
;;                02665 ;       Get CONFIG status & set ZERO byte
;;                02666 ;
;; 1E98  210104   02667         LD      HL,ZERO$
;; 1E9B  7E       02668         LD      A,(HL)          ;Set to NOP if SYSGENed
;; 1E9C  3600     02669         LD      (HL),0          ;Make always zero byte
;; 1E9E  F5       02670         PUSH    AF              ;Save SYSGEN flag
;;                02671 ;
;;                02672 ;       Check if date prompt is to be suppressed
;;                02673 ;
;; 1E9F  3AC204   02674         LD      A,(DTPMT$)      ;No prompt for DATE?
;; 1EA2  B7       02675         OR      A
;;                02676 ;
;;                02677 ;       Check on currency of date
;;                02678 ;
;; 1EA3  213300   02679         LD      HL,DATE$        ;Point to year
;; 1EA6  4E       02680         LD      C,(HL)          ;Save in reg C
;; 1EA7  3600     02681         LD      (HL),0          ;  while resetting to zero
;; 1EA9  23       02682         INC     HL              ;Bump to day
;; 1EAA  46       02683         LD      B,(HL)          ;  and save in reg B
;; 1EAB  3600     02684         LD      (HL),0          ;  while resetting to zero
;; 1EAD  23       02685         INC     HL              ;Bump to month
;; 1EAE  7E       02686         LD      A,(HL)          ;Save month to A
;; 1EAF  3600     02687         LD      (HL),0          ;  while resetting to zero
;; 1EB1  C2AD1F   02688         JP      NZ,TIMIN        ;Check time if DATE=OFF
;; 1EB4  2EFF     02689         LD      L,CFGFCB$+31&0FFH ; Reset pointer
;;                02690 ;
;;                02691         IF      @INTL
;;                02692         LD      (HL),B          ;Stuff day
;;                02693         DEC     HL
;;                02694         LD      (HL),A          ;Stuff month
;;                02695         ELSE
;; 1EB6  77       02696         LD      (HL),A          ;Stuff month
;; 1EB7  2B       02697         DEC     HL
;; 1EB8  70       02698         LD      (HL),B          ;Stuff day
;;                02699         ENDIF
;;                02700 ;
;; 1EB9  2B       02701         DEC     HL
;; 1EBA  71       02702         LD      (HL),C          ;Stuff year
;; 1EBB  EB       02703         EX      DE,HL           ;Point DE to CFGFCB$+29
;; 1EBC  3D       02704         DEC     A               ;Check for month range <1-12>
;; 1EBD  FE0C     02705         CP      12              ;Okay if 0-11 now
;; 1EBF  380E     02706         JR      C,DATIN1
;;                02707 ;
;; 1EC1  210005   02708 DATIN   LD      HL,DATEROW<8!DATECOL    ;Set video row, col
;; 1EC4  115C21   02709         LD      DE,DATEPR       ;DATE? question
;; 1EC7  013008   02710         LD      BC,8<+8!'0'     ;Set buff len & char
;; 1ECA  CDD620   02711         CALL    GETPARM         ;Get response
;; 1ECD  30F2     02712         JR      NC,DATIN        ;Jump on format error
;; 1ECF  1A       02713 DATIN1  LD      A,(DE)          ;Is year a leap year?
;;                02714 ;--> 6.3.1 changes start
;; 1ED0  FE0C     02715         CP      0CH
;; 1ED2  3003     02716         JR      NC,DATIN2
;; 1ED4  C664     02717         ADD     A,100
;; 1ED6  12       02718         LD      (DE),A
;;                02719 ;<-- 6.3.1 changes done
;; 1ED7  4F       02720 DATIN2  LD      C,A             ;Save year for later
;; 1ED8  D650     02721         SUB     80              ;Reduce for range test
;; 1EDA  FE20     02722         CP      20H             ; Check year range (6.2 was 8)
;; 1EDC  30E3     02723         JR      NC,DATIN
;; 1EDE  E603     02724         AND     3
;; 1EE0  3E1C     02725         LD      A,28            ;Init February
;; 1EE2  2006     02726         JR      NZ,NOTLEAP
;; 1EE4  213700   02727         LD      HL,DATE$+3+1    ;Set leap flag
;; 1EE7  CBFE     02728         SET     7,(HL)
;; 1EE9  3C       02729         INC     A               ;Feb to 29 days
;; 1EEA  210304   02730 NOTLEAP LD      HL,MAXDAY$+2    ;Set Feb max day #
;; 1EED  77       02731         LD      (HL),A
;;                02732 ;
;;                02733         IF      @INTL
;;                02734         NOP                     ;Keep same length
;;                02735         ELSE
;; 1EEE  13       02736         INC     DE              ;Bump to day
;;                02737         ENDIF
;; 1EEF  13       02738         INC     DE              ;Bump to month & get it
;; 1EF0  1A       02739         LD      A,(DE)
;; 1EF1  47       02740         LD      B,A             ;Save
;; 1EF2  3D       02741         DEC     A
;; 1EF3  FE0C     02742         CP      12              ;Range check
;; 1EF5  30CA     02743         JR      NC,DATIN        ;Go if error
;; 1EF7  2B       02744         DEC     HL              ;Point to Jan entry
;; 1EF8  85       02745         ADD     A,L             ;Index the month
;; 1EF9  6F       02746         LD      L,A
;;                02747 ;
;;                02748         IF      @INTL
;;                02749         INC     DE              ;Point to day
;;                02750         ELSE
;; 1EFA  1B       02751         DEC     DE              ;Point to day
;;                02752         ENDIF
;;                02753 ;
;; 1EFB  1A       02754         LD      A,(DE)          ;Get day entry
;; 1EFC  3D       02755         DEC     A               ;Reduce for test (0->FF)
;; 1EFD  BE       02756         CP      (HL)
;; 1EFE  30C1     02757         JR      NC,DATIN        ;Go if too large (or 0)
;;                02758 ;
;;                02759 ;       Range checks okay - move into DATE$
;;                02760 ;
;; 1F00  213500   02761         LD      HL,DATE$+2
;; 1F03  3C       02762         INC     A               ;Compensate for DEC A
;; 1F04  70       02763         LD      (HL),B          ;Stuff month
;; 1F05  2D       02764         DEC     L
;; 1F06  77       02765         LD      (HL),A          ;Stuff day
;; 1F07  2D       02766         DEC     L
;; 1F08  71       02767         LD      (HL),C          ;Stuff year
;;                02768 ;
;;                02769 ;       Date is in DATE$ - display it
;;                02770 ;
;; 1F09  79       02771         LD      A,C             ;Get year
;; 1F0A  F5       02772         PUSH    AF              ;  and save it
;; 1F0B  E603     02773         AND     3               ;Check on leap year
;; 1F0D  210304   02774         LD      HL,MAXDAY$+2    ;Init & adj Feb as
;; 1F10  361C     02775         LD      (HL),28         ;  required
;; 1F12  2001     02776         JR      NZ,$+3
;; 1F14  34       02777         INC     (HL)            ;Bump to 29
;; 1F15  3A3500   02778         LD      A,(DATE$+2)     ;Get month and xfer
;; 1F18  47       02779         LD      B,A             ;  it to B
;; 1F19  3A3400   02780         LD      A,(DATE$+1)     ;Get day of month
;;                02781 ;
;;                02782 ;       Compute day of year and day of week
;;                02783 ;
;; 1F1C  6F       02784         LD      L,A             ;Start off with days
;; 1F1D  2600     02785         LD      H,0             ;  in this month
;; 1F1F  110104   02786         LD      DE,MAXDAY$
;; 1F22  1A       02787 DAYLP   LD      A,(DE)
;; 1F23  85       02788         ADD     A,L             ;8 bit add to 16 bit
;; 1F24  6F       02789         LD      L,A
;; 1F25  8C       02790         ADC     A,H             ;Add high order & carry
;; 1F26  95       02791         SUB     L               ;Subtract off low order
;; 1F27  67       02792         LD      H,A             ;Update high order
;; 1F28  13       02793         INC     DE
;; 1F29  10F7     02794         DJNZ    DAYLP
;; 1F2B  EB       02795         EX      DE,HL           ;Move day of year to DE
;; 1F2C  213600   02796         LD      HL,DATE$+3      ;Store it
;; 1F2F  73       02797         LD      (HL),E
;; 1F30  23       02798         INC     HL
;; 1F31  7A       02799         LD      A,D             ;Get bit "8"
;; 1F32  B6       02800         OR      (HL)            ;  and OR it in
;; 1F33  77       02801         LD      (HL),A          ;Then put it back
;; 1F34  EB       02802         EX      DE,HL           ;Get DOY back in HL
;; 1F35  F1       02803         POP     AF              ;Pop the year and mask
;; 1F36  D650     02804         SUB     80              ;Compute DOW offset (6.2 was AND 7)
;; 1F38  5F       02805         LD      E,A
;; 1F39  C603     02806         ADD     A,3
;; 1F3B  0F       02807         RRCA
;; 1F3C  0F       02808         RRCA
;; 1F3D  E60F     02809         AND     0FH             ; 6.3.1 change - 6.2 was 03H
;; 1F3F  83       02810         ADD     A,E
;; 1F40  5F       02811         LD      E,A             ;And add it in
;; 1F41  1600     02812         LD      D,0             ;Add into HL
;; 1F43  19       02813         ADD     HL,DE
;; 1F44  23       02814         INC     HL              ;Start in right place
;;                02815 ;--> 6.3.1 changes
;; 1F45  3E07     02816         LD      A,7
;; 1F47  CDE306   02817         CALL    @DIV16          ;Divide by 7
;; 1F4A  3C       02818         INC     A
;;                02819 ;<-- 6.3.1 changes
;; 1F4B  47       02820         LD      B,A             ;Save in reg B
;; 1F4C  07       02821         RLCA                    ;Shift to bits 1-3
;; 1F4D  4F       02822         LD      C,A             ;Save temporarily
;; 1F4E  213700   02823         LD      HL,DATE$+3+1
;; 1F51  7E       02824         LD      A,(HL)          ;Pack into field
;; 1F52  E6F1     02825         AND     0F1H
;; 1F54  B1       02826         OR      C
;; 1F55  77       02827         LD      (HL),A
;; 1F56  C5       02828         PUSH    BC
;; 1F57  210005   02829         LD      HL,DATEROW<8!DATECOL    ;Video row/col
;; 1F5A  0603     02830         LD      B,3             ;Position cursor
;; 1F5C  CD990B   02831         CALL    @VDCTL
;; 1F5F  C1       02832         POP     BC
;; 1F60  21C704   02833         LD      HL,DAYTBL$
;; 1F63  CD3C21   02834         CALL    DSPMDY          ;Write out the DAY
;; 1F66  3E2C     02835         LD      A,','
;; 1F68  CD4206   02836         CALL    @DSP
;; 1F6B  3E20     02837         LD      A,' '
;; 1F6D  CD4206   02838         CALL    @DSP
;; 1F70  3A3500   02839         LD      A,(DATE$+2)     ;Get month number
;; 1F73  47       02840         LD      B,A
;; 1F74  2EDC     02841         LD      L,MONTBL$&0FFH  ;Set HL to month table
;;                02842 ;
;;                02843 ;       Another custom mod - month displayed slightly differently
;;                02844 ;       when @PCERV is enabled.
;;                02845 ;
;;                02846         IF      @PCERV
;; 1F76  CD3C21   02847         CALL    DSPMDY          ;Add in the spaces if @PCERV
;;                02848         ELSE
;;                02849         CALL    DSPMON          ;Write out the month name
;;                02850         ENDIF
;;                02851 ;
;; 1F79  3E20     02852         LD      A,' '           ;Space after the name
;; 1F7B  CD4206   02853         CALL    @DSP
;; 1F7E  3A3400   02854         LD      A,(DATE$+1)     ;Get day
;; 1F81  05       02855         DEC     B               ;From 0 to X'FF'
;; 1F82  04       02856 DIV10   INC     B               ;Divide by 10
;; 1F83  D60A     02857         SUB     10              ;  with quotient in B
;; 1F85  30FB     02858         JR      NC,DIV10
;; 1F87  F5       02859         PUSH    AF              ;Save remainder (-10)
;; 1F88  78       02860         LD      A,B             ;Get quotient
;; 1F89  C630     02861         ADD     A,'0'           ;Change to ASCII
;; 1F8B  FE30     02862         CP      '0'             ;Zero?
;; 1F8D  C44206   02863         CALL    NZ,@DSP         ;Display if not
;; 1F90  F1       02864         POP     AF              ;Get back remainder
;; 1F91  C63A     02865         ADD     A,'0'+10        ;Change to ASCII
;; 1F93  CD4206   02866         CALL    @DSP            ;Display it
;;                02867 ;--> 6.3.1 changes
;; 1F96  3A3300   02868         LD      A,(DATE$)       ;Get year digits (97?)
;; 1F99  216C07   02869         LD      HL,1900         ;Init to 1900
;; 1F9C  85       02870         ADD     A,L             ;Add in base year low
;; 1F9D  6F       02871         LD      L,A             ;Save back in L
;; 1F9E  8C       02872         ADC     A,H             ;Add in high byte of year
;; 1F9F  95       02873         SUB     L               ;Drop extra
;; 1FA0  67       02874         LD      H,A             ;And put in H
;; 1FA1  115521   02875         LD      DE,PARTYR+1     ;Buffer area
;; 1FA4  CDF606   02876         CALL    @HEXDEC         ;Convert to decimal
;; 1FA7  215421   02877         LD      HL,PARTYR       ;Point to buffer
;; 1FAA  CD2D05   02878         CALL    @DSPLY          ;And display it.
;;                02879 ;<-- 6.3.1 changes
;;                02880 ;
;;                02881 ;       Prompt for time
;;                02882 ;
;; 1FAD  3AC304   02883 TIMIN   LD      A,(TMPMT$)      ;Prompt for time?
;; 1FB0  B7       02884         OR      A
;; 1FB1  2037     02885         JR      NZ,M1FEA        ;Skip if not
;;                02886 ;--> 6.3.1 changes
;; 1FB3  0603     02887 TIMIN0  LD      B,3             ;3 bytes to change
;; 1FB5  21FF00   02888         LD      HL,00FFH        ;Top of first RAM page
;; 1FB8  3600     02889 M1FB8   LD      (HL),0          ;Clear those bytes out
;; 1FBA  2B       02890         DEC     HL
;; 1FBB  10FB     02891         DJNZ    M1FB8
;; 1FBD  3EFF     02892         LD      A,0FFH          ;Set byte in code for
;; 1FBF  32EF20   02893         LD      (M20EE+1),A     ;  comparison
;;                02894 ;<-- 6.3.1 changes
;; 1FC2  210006   02895         LD      HL,TIMEROW<8!TIMECOL ;Row/col set
;; 1FC5  116E21   02896         LD      DE,TIMEPR       ;Prompt message
;; 1FC8  013008   02897         LD      BC,8<+8!'0'     ;Len and separ char
;; 1FCB  CDD620   02898         CALL    GETPARM
;; 1FCE  30E3     02899         JR      NC,TIMIN0       ;Loop on format error
;; 1FD0  21FF00   02900         LD      HL,CFGFCB$+31
;; 1FD3  3E17     02901         LD      A,23            ;Max hour value
;; 1FD5  BE       02902         CP      (HL)            ;Test hour range
;; 1FD6  38DB     02903         JR      C,TIMIN0
;; 1FD8  2B       02904         DEC     HL
;; 1FD9  3E3B     02905         LD      A,59            ;Max minute value
;; 1FDB  BE       02906         CP      (HL)            ;Test minutes
;; 1FDC  38D5     02907         JR      C,TIMIN0
;; 1FDE  2B       02908         DEC     HL
;; 1FDF  BE       02909         CP      (HL)            ;Test seconds
;; 1FE0  38D1     02910         JR      C,TIMIN0
;; 1FE2  112D00   02911         LD      DE,TIME$        ;Move time value
;; 1FE5  010300   02912         LD      BC,3            ;  into the TIME$ field
;; 1FE8  EDB0     02913         LDIR
;;                02914 ;--> 6.3.1 changes
;; 1FEA  0680     02915 M1FEA   LD      B,80H           ;Add a short pause
;; 1FEC  CD8203   02916         CALL    @PAUSE
;;                02917 ;<-- 6.3.1 changes
;;                02918 ;
;;                02919 ;       Check on any auto command
;;                02920 ;
;; 1FEF  212004   02921 SELDCT  LD      HL,INBUF$
;; 1FF2  7E       02922         LD      A,(HL)          ;Get 1st byte of AUTO
;; 1FF3  FE2A     02923         CP      '*'             ;Unbreakable AUTO command?
;; 1FF5  200F     02924         JR      NZ,CKDCR
;; 1FF7  23       02925         INC     HL
;; 1FF8  3EE6     02926         LD      A,0E6H          ;Set break bit in flag by
;; 1FFA  328220   02927         LD      (STUB1+1),A     ;  changing RES 4,(SFLAG$)
;;                02928                                 ;  to SET 4,(SFLAG$)
;; 1FFD  181A     02929         JR      AUTO?
;; 1FFF  CD1708   02930 GETKB17 CALL    ENADIS_DO_RAM
;; 2002  3A41F4   02931         LD      A,(KB1!KB7)     ;Scan rows 1 and 7
;; 2005  C9       02932         RET
;; 2006  CDFF1F   02933 CKDCR   CALL    GETKB17         ;Strobe keyboard
;; 2009  CB67     02934         BIT     4,A             ;Is 'D' pressed
;; 200B  E5       02935         PUSH    HL              ;Save auto command pointer
;; 200C  21081B   02936         LD      HL,@ABORT       ;Get abort address
;; 200F  E3       02937         EX      (SP),HL         ;Swap them around
	START_PATCH 2010h
	nop
	nop
	nop
;; 2010  C2A019   02938         JP      NZ,@DEBUG       ;DEBUG on <D>
	END_PATCH
;; 2013  D1       02939         POP     DE              ;Stack integrity
;; 2014  2F       02940         CPL
;; 2015  E601     02941         AND     1               ;No auto if <ENTER>
;; 2017  2803     02942         JR      Z,NOAUT1
;; 2019  7E       02943 AUTO?   LD      A,(HL)          ;Any auto command?
;; 201A  FE0D     02944         CP      CR              ;None if equal
;; 201C  D1       02945 NOAUT1  POP     DE              ;Get back SYSGEN flag
;; 201D  7A       02946         LD      A,D             ;  and move to reg A
;; 201E  110B1B   02947         LD      DE,@EXIT        ;Where to go after boot
;; 2021  010000   02948         LD      BC,0            ;Init BC(HL)=0 for @EXIT
;; 2024  280F     02949         JR      Z,NOAUT         ;Go if no AUTO
;; 2026  E5       02950         PUSH    HL              ;Save buffer pointer
;; 2027  21AC20   02951         LD      HL,CURSET       ;Point to cursor setting
;; 202A  34       02952         INC     (HL)            ;Bump it down a line
;; 202B  E1       02953         POP     HL              ;Recover INBUF$ pointer
;; 202C  117E19   02954         LD      DE,@CMNDI       ;Low order of @CMNDI
;; 202F  D5       02955         PUSH    DE              ;Save on stack for return
;; 2030  44       02956         LD      B,H             ;Put INBUF$ on stack
;; 2031  4D       02957         LD      C,L             ;  for @CMNDI
;; 2032  112D05   02958         LD      DE,@DSPLY       ;But do this first
;; 2035  D5       02959 NOAUT   PUSH    DE              ;Put on stack for RET
;; 2036  C5       02960         PUSH    BC              ;Either INBUF$ or 0
;; 2037  217E20   02961         LD      HL,STUB
;; 203A  115043   02962         LD      DE,MOD3BUF+80   ;Must move out of way
;; 203D  015800   02963         LD      BC,STUBLEN      ;  amout to move
;; 2040  D5       02964         PUSH    DE              ;Add RET vector to stack
;; 2041  EDB0     02965         LDIR                    ;Move stub up
;; 2043  CD7420   02966         CALL    GETKB67
;; 2046  117004   02967         LD      DE,DCT$         ;Set up to move DCTs
;; 2049  210043   02968         LD      HL,MOD3BUF      ;  from configed area
;; 204C  015000   02969         LD      BC,80           ;Count for DCTs (10*8)
;; 204F  D9       02970         EXX                     ;Keep in alternate set
;; 2050  E682     02971         AND     82H             ;Load config if zero
	START_PATCH 2052h
	nop
;; 2052  C0       02972         RET     NZ              ;No config > go back
	END_PATCH
;; 2053  210008   02973         LD      HL,SYSGROW<8    ;Position the cursor
;; 2056  0603     02974         LD      B,3
;; 2058  CD990B   02975         CALL    @VDCTL
;; 205B  216720   02976         LD      HL,CONFIG$      ;Show sysgen message
;; 205E  CD2D05   02977         CALL    @DSPLY
;; 2061  11E000   02978         LD      DE,CFGFCB$      ;Set up to load CONFIG/SYS
;; 2064  C3381B   02979         JP      @LOAD
;;                02980 ;
;; 2067  2A       02981 CONFIG$ DB      '** SYSGEN **',03
;;       2A 20 53 59 53 47 45 4E
;;       20 2A 2A 03 
;;                02982 ;
;;                02983 ;
;;                02984 ;
;; 2074  2160F4   02985 GETKB67 LD      HL,KB67         ;Check CLEAR key
;; 2077  4F       02986         LD      C,A
;; 2078  CD1708   02987         CALL    ENADIS_DO_RAM
;; 207B  79       02988         LD      A,C
;; 207C  B6       02989         OR      (HL)            ;Key down OR not SYSGENed
;; 207D  C9       02990         RET
;;                02991 ;
;;                02992 ;       Final initialization code
;;                02993 ;
;; 207E  217C00   02994 STUB    LD      HL,SFLAG$
;; 2081  CBA6     02995 STUB1   RES     4,(HL)          ;Test or set BREAK bit
;;                02996                                 ;Without changing Z/NZ
	START_PATCH 2083h
	jr	nz,2083h
;; 2083  200C     02997         JR      NZ,NOTSG        ;Go if no SYSGEN found
	END_PATCH
;; 2085  217600   02998         LD      HL,MODOUT$      ;Get pointer to port mask
;; 2088  7E       02999         LD      A,(HL)          ;Get mask byte
;; 2089  D3EC     03000         OUT     (0ECH),A        ;Speed it up
;; 208B  D9       03001         EXX                     ;Set to move DCTs
;; 208C  EDB0     03002         LDIR                    ;Move 'em
;; 208E  CD8600   03003         CALL    @ICNFG          ;Init config
;;                03004 NOTSG
;; 2091  0E07     03005         LD      C,7
;;                03006 SETCYL0
;; 2093  CD1E1A   03007         CALL    @GTDCT          ; Get drive's DCT
;; 2096  FDCB035E 03008         BIT     3,(IY+3)        ; If hard drive, don't stuff FF
;; 209A  200B     03009         JR      NZ,NOFF         ;   and don't restore
;; 209C  FD3605FF 03010         LD      (IY+5),0FFH     ; Set in case no restore
;; 20A0  3AC404   03011         LD      A,(RSTOR$)      ; Restore drives at startup?
;; 20A3  B7       03012         OR      A
;; 20A4  CCC819   03013         CALL    Z,@RSTOR        ; Restore drives 1-7
;; 20A7  0D       03014 NOFF    DEC     C               ; Next drive down
;; 20A8  20E9     03015         JR      NZ,SETCYL0      ; Loop for # of drives
;; 20AA  210008   03016         LD      HL,DINIROW<8    ; Where the cursor should wind up
;; 20AC           03017 CURSET  EQU     $-1
;; 20AD  0603     03018         LD      B,3
;; 20AF  CD990B   03019         CALL    @VDCTL          ; Set cursor position
;;                03020 ;
;;                03021 ;       Detect Model 4 or 4P and adjust TFLAG$
;;                03022 ;       Look at "MODEL" at 4018h.  If so, M4P
;;                03023 ;
;; 20B2  114D4F   03024         LD      DE,'OM'         ; Init DE to "MO"
;; 20B5  2A1840   03025         LD      HL,(4018H)      ; Get 4P rom "leftover"
;; 20B8  ED52     03026         SBC     HL,DE           ; Check if it's "MO"
;; 20BA  3E04     03027         LD      A,4             ; Init to Model 4 Reg.
;; 20BC  2002     03028         JR      NZ,MOD4REG      ;   and go if not
;; 20BE  3E05     03029         LD      A,5             ; Change to Mod 4P
;; 20C0  327D00   03030 MOD4REG LD      (TFLAG$),A
;;                03031 ;
;; 20C3  213800   03032         LD      HL,@RST38       ; Point to RST vector and
;; 20C6  36C3     03033         LD      (HL),0C3H       ;   activate task processor
;; 20C8  E1       03034         POP     HL              ; Pop INBUF$
;; 20C9  C9       03035         RET                     ; To @CMD or @DSPLY,@CMNDI
;;                03036 ;
;; 20CA  00       03037         DC      12,0            ; Space for more code
;;       00 00 00 00 00 00 00 00
;;       00 00 00 
;; 20D6           03038 STUBEND EQU     $
;; 0058           03039 STUBLEN EQU     STUBEND-STUB
;;                03040 ;
;;                03041 ;       Date and time prompting
;; 20D6  C5       03042 GETPARM PUSH    BC              ; Save separator char
;; 20D7  D5       03043         PUSH    DE              ; Save message pointer
;; 20D8  0603     03044         LD      B,3
;; 20DA  CD990B   03045         CALL    @VDCTL          ; Set cursor position
;; 20DD  E1       03046         POP     HL              ; Recover message pointer
;; 20DE  CD2D05   03047         CALL    @DSPLY          ;   and display it
;; 20E1  21001E   03048         LD      HL,OVERLAY      ; Input buffer location
;; 20E4  C1       03049         POP     BC              ; Get max length back
;; 20E5  C5       03050         PUSH    BC
;; 20E6  CD8505   03051         CALL    @KEYIN          ; Get user entry
;; 20E9  AF       03052         XOR     A
;; 20EA  B0       03053         OR      B               ; Anything entered?
;; 20EB  C1       03054         POP     BC
;;                03055 ;---> 6.3.1 changes
;; 20EC  2006     03056         JR      NZ,M20F4        ; Go if something entered
;; 20EE  3E00     03057 M20EE   LD      A,0             ;   else set return
;; 20F0  B7       03058         OR      A               ;   Z/NZ status
;; 20F1  C8       03059         RET     Z
;; 20F2  37       03060 M20F2   SCF                     ; Set carry flag
;; 20F3  C9       03061         RET                     ;   and return
;;                03062 ;<--- 6.3.1 changes
;; 20F4  C5       03063 M20F4   PUSH    BC
;; 20F5  0640     03064         LD      B,40H
;; 20F7  CD8203   03065         CALL    @PAUSE          ; To let finger off
;; 20FA  C1       03066         POP     BC
;;                03067 ;
;;                03068 ;       Routine to parse DATE entry
;;                03069 ;
;; 20FB  11FF00   03070 PARSDAT LD      DE,CFGFCB$+31   ;Point to buffer end
;; 20FE  0603     03071         LD      B,3             ;Process 3 fields
;; 2100  D5       03072 PRSD1   PUSH    DE              ;Save pointer
;;                03073 ;
;;                03074 ;       Routine to parse a digit pair
;;                03075 ;
;; 2101  CD3521   03076         CALL    PRSD3           ;Get a digit
;; 2104  300F     03077         JR      NC,PRSD2        ;Jump if bad digit
;; 2106  5F       03078         LD      E,A             ;Multiply by ten
;; 2107  07       03079         RLCA                    ;*2
;; 2108  07       03080         RLCA                    ;*4
;; 2109  83       03081         ADD     A,E             ;*5
;; 210A  07       03082         RLCA                    ;*10
;; 210B  5F       03083         LD      E,A             ;Save back in E
;; 210C  CD3521   03084         CALL    PRSD3           ;Get another digit
;; 210F  3004     03085         JR      NC,PRSD2        ;Jump on bad digit
;; 2111  83       03086         ADD     A,E             ;Accumulate new digit
;; 2112  5F       03087         LD      E,A             ;Save 2-digit value
;; 2113  37       03088         SCF                     ;Show valid
;; 2114  7B       03089         LD      A,E             ;Xfer field value
;; 2115  D1       03090 PRSD2   POP     DE              ;Recover pointer
;; 2116  D0       03091         RET     NC              ;Ret if bad digit pair
;; 2117  12       03092         LD      (DE),A          ;Else stuff the value
;; 2118  05       03093         DEC     B               ;Loop countdown
;; 2119  37       03094         SCF
;; 211A  C8       03095         RET     Z               ;Ret when through
;; 211B  1B       03096         DEC     DE              ;Backup the pointer
;; 211C  7E       03097         LD      A,(HL)          ;Ck for valid separator
;; 211D  23       03098         INC     HL              ;Bump pointer
;; 211E  FE3A     03099         CP      ':'             ;Check for colon
;; 2120  28DE     03100         JR      Z,PRSD1         ;Loop if match
;; 2122  B9       03101         CP      C               ;Separator char required
;; 2123  3014     03102         JR      NC,PRSD4        ;Exit if bad char
;;                03103 ;---> 6.3.1 changes
;; 2125  FE0D     03104         CP      0DH             ;Enter?
;; 2127  20D7     03105         JR      NZ,PRSD1        ;Loop if not
;; 2129  78       03106         LD      A,B             ;Get char count
;; 212A  3D       03107         DEC     A               ;Decrement it
;; 212B  20D3     03108         JR      NZ,PRSD1        ;Loop if not done
;; 212D  3AEF20   03109         LD      A,(M20EE+1)     ;Get value from code
;; 2130  B7       03110         OR      A               ;Is it zero?
;; 2131  28CD     03111         JR      Z,PRSD1         ;Loop if it is
;; 2133  37       03112         SCF                     ;Else set carry
;; 2134  C9       03113         RET                     ;  and return
;;                03114 ;<--- 6.3.1 changes
;; 2135  7E       03115 PRSD3   LD      A,(HL)          ;Get a digit
;; 2136  23       03116         INC     HL
;; 2137  D630     03117         SUB     30H             ;Convert to binary
;; 2139  FE0A     03118 PRSD4   CP      10
;; 213B  C9       03119         RET
;;                03120 ;
;;                03121 ;       Routine to display month or day of week
;;                03122 ;
;; 213C  E5       03123 DSPMDY  PUSH    HL              ;Print 4 spaces
;; 213D  218021   03124         LD      HL,SPACE4$      ;Point to string
;; 2140  CD2D05   03125         CALL    @DSPLY
;; 2143  E1       03126         POP     HL
;; 2144  05       03127 DSPMON  DEC     B               ;Point to Bth entry
;; 2145  7D       03128         LD      A,L             ;  in table
;; 2146  80       03129         ADD     A,B             ;Entries are 3 long
;; 2147  80       03130         ADD     A,B             ;  so have to add B
;; 2148  80       03131         ADD     A,B             ;  three times.
;; 2149  6F       03132         LD      L,A
;; 214A  0603     03133         LD      B,3             ;Print 3 chars
;; 214C  7E       03134 DSPM1   LD      A,(HL)
;; 214D  23       03135         INC     HL
;; 214E  CD4206   03136         CALL    @DSP
;; 2151  10F9     03137         DJNZ    DSPM1
;; 2153  C9       03138         RET
;; 2154  2C       03139 PARTYR  DB      ', 198 ',30,3
;;       20 31 39 38 20 1E 03 
;;                03140 ;
;;                03141         IF      @INTL
;;                03142 DATEPR  DB      30,'Date DD/MM/YY ? ',3
;;                03143         ELSE
;; 215C  1E       03144 DATEPR  DB      30,'Date MM/DD/YY ? ',3
;;       44 61 74 65 20 4D 4D 2F
;;       44 44 2F 59 59 20 3F 20
;;       03 
;;                03145         ENDIF
;;                03146 ;
;; 216E  1E       03147 TIMEPR  DB      30,'Time HH:MM:SS ? ',3
;;       54 69 6D 65 20 48 48 3A
;;       4D 4D 3A 53 53 20 3F 20
;;       03 
;; 2180  20       03148 SPACE4$ DB      '   ',3,3
;;       20 20 03 03 
;;                03149 ;
;;                03150 ;       Under 6.3.0, this was the "Serial# xxxxxxxxxx' string
;;                03151 ;
;; 2185  00       03152 M2185   DC      21,0            ;Space for message, or ???
;;       00 00 00 00 00 00 00 00
;;       00 00 00 00 00 00 00 00
;;       00 00 00 00 
;; 219A  00       03153 M2XXX   DC      32,0
;;       00 00 00 00 00 00 00 00
;;       00 00 00 00 00 00 00 00
;;       00 00 00 00 00 00 00 00
;;       00 00 00 00 00 00 00 
;; 21BA           03157 *GET    SOUND
;;                03158 ;****************************************************************
;;                03159 ;* Filename: SOUND/ASM                                          *
;;                03160 ;* Rev Date: 30 Nov 97                                          *
;;                03161 ;* Revision: 6.3.1                                              *
;;                03162 ;****************************************************************
;;                03163 ;* Misc. lowcore routines                                       *
;;                03164 ;*                                                              *
;;                03165 ;****************************************************************
;;                03166 ;
;;                03167 ;       Contains IPL, PAUSE, SOUND, and DECHEX routines
;;                03168 ;       Will be loaded into lowcore area along with SYSRES
;;                03169 ;
;;                03170 *MOD
;; 0090           03171 SNDPORT EQU     90H
;; 0380           03172         ORG     STACK$
;; 0380  0000     03173         DW      00              ;Stack guard
;;                03174 ;
;;                03175 ;       Pause routine
;;                03176 ;
;; 0382  C5       03177 @PAUSE  PUSH    BC              ;Save the count
;;                03178 ;       SRL     B               ;Adjust for wait states
;;                03179 ;       RR      C
;; 0383  3A7C00   03180         LD      A,(SFLAG$)      ;if system in FAST mode
;; 0386  CB5F     03181         BIT     3,A             ;  then double it
;; 0388  C48C03   03182         CALL    NZ,CDLOOP       ;Call if fast
;; 038B  C1       03183         POP     BC              ;Restore the count
;; 038C  0B       03184 CDLOOP  DEC     BC              ;Count down routine
;; 038D  78       03185         LD      A,B
;; 038E  B1       03186         OR      C
;; 038F  20FB     03187         JR      NZ,CDLOOP
;; 0391  C9       03188         RET
;;                03189 ;
;;                03190 ;       @SOUND SVC 104 - Operates sound generator
;;                03191 ;        B -> Sound function
;;                03192 ;         Bits 0-2 = Note # (0=highest)
;;                03193 ;         Bits 3-7 = Relative sound duration
;;                03194 ;       All regs except A left unchanged
;;                03195 ;       Z flag set on exit
;;                03196 ;       Note that interrupts disabled during generation
;;                03197 ;
;; 0392  C5       03198 @SOUND  PUSH    BC              ;Save registers
;; 0393  E5       03199         PUSH    HL
;; 0394  78       03200         LD      A,B             ;Get sound data
;; 0395  E607     03201         AND     7               ;  just want note #
;; 0397  07       03202         RLCA                    ;Adjust for 2 byte fields
;; 0398  21D103   03203         LD      HL,SNDTAB
;; 039B  4F       03204         LD      C,A
;; 039C  78       03205         LD      A,B             ;Pick up duration data
;; 039D  0600     03206         LD      B,0             ;Index into tone table
;; 039F  09       03207         ADD     HL,BC           ;  to get note on/off
;; 03A0  4E       03208         LD      C,(HL)          ;Get note on/off data
;; 03A1  23       03209         INC     HL
;; 03A2  6E       03210         LD      L,(HL)          ;Get note duration
;; 03A3  0F       03211         RRCA                    ;Rotate sound duration
;; 03A4  0F       03212         RRCA                    ;  into bits 0-4
;; 03A5  0F       03213         RRCA
;; 03A6  E61F     03214         AND     1FH             ;Strip off sound #
;; 03A8  3C       03215         INC     A               ;Adjust for offset
;; 03A9  67       03216         LD      H,A             ;Set sound counter
;; 03AA  3A7C00   03217         LD      A,(SFLAG$)      ;If fast, double values
;; 03AD  E608     03218         AND     8
;; 03AF  2806     03219         JR      Z,$A1
;; 03B1  CB24     03220         SLA     H
;; 03B3  CB25     03221         SLA     L
;; 03B5  CB21     03222         SLA     C
;; 03B7  F3       03223 $A1     DI                      ;Don't interrupt timing
;; 03B8  E5       03224 $A2     PUSH    HL              ;Save note duration
;; 03B9  41       03225 $A3     LD      B,C             ;Play tone
;; 03BA  3E01     03226         LD      A,1             ;Hold output high
;; 03BC  D390     03227         OUT     (SNDPORT),A     ;  for count of B
;; 03BE  10FE     03228         DJNZ    $
;; 03C0  41       03229         LD      B,C             ;Hold output low
;; 03C1  3C       03230         INC     A               ;  for count of B
;; 03C2  D390     03231         OUT     (SNDPORT),A
;; 03C4  10FE     03232         DJNZ    $
;; 03C6  2D       03233         DEC     L               ;Dec the duration
;; 03C7  20F0     03234         JR      NZ,$A3
;; 03C9  E1       03235         POP     HL
;; 03CA  25       03236         DEC     H
;; 03CB  20EB     03237         JR      NZ,$A2
;; 03CD  FB       03238         EI
;; 03CE  E1       03239         POP     HL
;; 03CF  C1       03240         POP     BC
;; 03D0  C9       03241         RET
;;                03242 ;
;;                03243 ;       Note Table
;;                03244 ;
;; 00B4           03245 SNDOFF  EQU     180             ;Sound duration offset
;; 001C           03246 TONER   EQU     28
;; 03D1  50       03247 SNDTAB  DB      108-TONER
;; 03D2  4C       03248         DB      0-SNDOFF
;; 03D3  56       03249         DB      114-TONER
;; 03D4  48       03250         DB      252-SNDOFF
;; 03D5  5C       03251         DB      120-TONER
;; 03D6  44       03252         DB      248-SNDOFF
;; 03D7  62       03253         DB      126-TONER
;; 03D8  40       03254         DB      244-SNDOFF
;; 03D9  6B       03255         DB      135-TONER
;; 03DA  3C       03256         DB      240-SNDOFF
;; 03DB  72       03257         DB      142-TONER
;; 03DC  38       03258         DB      236-SNDOFF
;; 03DD  79       03259         DB      149-TONER
;; 03DE  34       03260         DB      232-SNDOFF
;; 03DF  80       03261         DB      156-TONER
;; 03E0  30       03262         DB      228-SNDOFF
;; 004F           03263 SNDLEN  EQU     $-@SOUND
;;                03264 ;
;;                03265 ;       Process decimal assignment
;;                03266 ;
;; 03E1  010000   03267 @DECHEX LD      BC,0            ;Init value to zero
;; 03E4  7E       03268 DEC1    LD      A,(HL)          ;get a char
;; 03E5  D630     03269         SUB     30H
;; 03E7  D8       03270         RET     C
;; 03E8  FE0A     03271         CP      10
;; 03EA  D0       03272         RET     NC
;; 03EB  C5       03273         PUSH    BC
;; 03EC  E3       03274         EX      (SP),HL
;; 03ED  29       03275         ADD     HL,HL
;; 03EE  29       03276         ADD     HL,HL
;; 03EF  09       03277         ADD     HL,BC
;; 03F0  29       03278         ADD     HL,HL
;; 03F1  0600     03279         LD      B,0
;; 03F3  4F       03280         LD      C,A
;; 03F4  09       03281         ADD     HL,BC
;; 03F5  44       03282         LD      B,H
;; 03F6  4D       03283         LD      C,L
;; 03F7  E1       03284         POP     HL
;; 03F8  23       03285         INC     HL
;; 03F9  18E9     03286         JR      DEC1
;;                03287 ;
;;                03288 ;       Special boot code to be moved to 4300h by IPL
;;                03289 ;
;; 03FB  F3       03290 BOOTCOD DI
;; 03FC  AF       03291         XOR     A
;; 03FD  D384     03292         OUT     (@OPREG),A
;; 03FF  C7       03293         RST     0
;; 0005           03294 BOOTLEN EQU     $-BOOTCOD
;;                03297 ;
;;                03298         IF      @PCERV
;; 0400           03299 *GET    CERVLOGO
;;                03300 ; LOGO/ASM - LS-DOS 6.3.x Logo to use when booting up
;;                03301 ; Copyright (c) 1997-98 Peter W. Cervasio.
;;                03302 ; Distribute freely as long as the above copyright message is
;;                03303 ; not removed.
;;                03304 ;
;;                03305 ; This is the one that I made part of my LSDOS disks because the
;;                03306 ; combination of this and other changes keep the doggone logo
;;                03307 ; from scrolling off the screen when *SYSGEN* and the AUTO
;;                03308 ; command appear on the screen.  And it just looks cool.  :)
;;                03309 ;
;;                03310 ;
;; F800           03311         ORG     CRTBGN$
;;                03312 ;
;;                03313 ; Line 1
;;                03314 ;
;; F800  20       03315         DB      20H
;; F801  8F       03316         DB      08FH,0BFH,0BFH,08FH,020H,020H,020H,020H,0BFH,0BFH
;;       BF BF 8F 20 20 20 20 BF
;;       BF 
;; F80B  8F       03317         DB      08FH,08FH,08FH,08FH,020H,020H,020H,08FH,0BFH,0BFH
;;       8F 8F 8F 20 20 20 8F BF
;;       BF 
;; F815  8F       03318         DB      08FH,08FH,0BFH,0B0H,020H,0BFH,0BFH,08FH,08FH,0BFH
;;       8F BF B0 20 BF BF 8F 8F
;;       BF 
;; F81F  BF       03319         DB      0BFH,020H,0BFH,0BFH,08FH,08FH,08FH,08FH,020H,020H
;;       20 BF BF 8F 8F 8F 8F 20
;;       20 
;; F829  20       03320         DB      020H,0BFH,0BFH,08FH,08FH,08FH,08FH,020H,020H,020H
;;       BF BF 8F 8F 8F 8F 20 20
;;       20 
;; F833  BF       03321         DB      0BFH,08FH,08FH,08FH,0BFH,0BFH
;;       8F 8F 8F BF BF 
;; F839  20       03322         DB      '  Copyright 1986,90'
;;       20 43 6F 70 79 72 69 67
;;       68 74 20 31 39 38 36 2C
;;       39 30 
;;                03323 ;
;; F850           03324         ORG     CRTBGN$+80
;;                03325 ;
;;                03326 ; Line 2
;;                03327 ;
;; F850  20       03328         DB      20H
;; F851  20       03329         DB      020H,0BFH,0BFH,020H,020H,020H,020H,020H,08FH,08FH
;;       BF BF 20 20 20 20 20 8F
;;       8F 
;; F85B  8C       03330         DB      08CH,08CH,0BCH,0BCH,020H,08CH,08CH,020H,0BFH,0BFH
;;       8C BC BC 20 8C 8C 20 BF
;;       BF 
;; F865  20       03331         DB      020H,020H,0BFH,0BFH,020H,0BFH,0BFH,020H,020H,0BFH
;;       20 BF BF 20 BF BF 20 20
;;       BF 
;; F86F  BF       03332         DB      0BFH,020H,08FH,08FH,08CH,08CH,0BCH,0BCH,020H,020H
;;       20 8F 8F 8C 8C BC BC 20
;;       20 
;; F879  20       03333         DB      020H,0BFH,0BFH,08CH,08CH,0BCH,0BCH,020H,020H,020H
;;       BF BF 8C 8C BC BC 20 20
;;       20 
;; F883  20       03334         DB      020H,020H,08CH,08CH,0BFH,0B7H
;;       20 8C 8C BF B7 
;; F889  20       03335         DB      '  (c) MISOSYS, Inc.'
;;       20 28 63 29 20 4D 49 53
;;       4F 53 59 53 2C 20 49 6E
;;       63 2E 
;;                03336 ;
;; F8A0           03337         ORG     CRTBGN$+160
;;                03338 ;
;;                03339 ; Line 3
;;                03340 ;
;; F8A0  20       03341         DB      20H
;; F8A1  BC       03342         DB      0BCH,0BFH,0BFH,0BCH,0BCH,0BCH,0BFH,020H,0BCH,0BCH
;;       BF BF BC BC BC BF 20 BC
;;       BC 
;; F8AB  BC       03343         DB      0BCH,0BCH,0BFH,0BFH,020H,020H,020H,0BCH,0BFH,0BFH
;;       BC BF BF 20 20 20 BC BF
;;       BF 
;; F8B5  BC       03344         DB      0BCH,0BCH,0BFH,083H,020H,0BFH,0BFH,0BCH,0BCH,0BFH
;;       BC BF 83 20 BF BF BC BC
;;       BF 
;; F8BF  BF       03345         DB      0BFH,020H,0BCH,0BCH,0BCH,0BCH,0BFH,0BFH,020H,020H
;;       20 BC BC BC BC BF BF 20
;;       20 
;; F8C9  20       03346         DB      020H,0BFH,0BFH,0BCH,0BCH,0BFH,0BFH,020H,0BCH,020H
;;       BF BF BC BC BF BF 20 BC
;;       20 
;; F8D3  BF       03347         DB      0BFH,0BCH,0BCH,0BCH,0BFH,0BFH
;;       BC BC BC BF BF 
;; F8D9  20       03348         DB      '  Rel: 06.03.01H'
;;       20 52 65 6C 3A 20 30 36
;;       2E 30 33 2E 30 31 48 
;;                03349 ;
;;                03351 ;
;; F8E0           03352         ORG     CRTBGN$+160+64
;; F8E0  30       03353         DB      '06.03.01'
;;       36 2E 30 33 2E 30 31 
;;                03354 ;
;;                03355         ELSE
;;                03357         ENDIF
;;                03358 ;
;; 0036           03359         ORG     0036H
;; 0036  00       03360         DB      0
;;                03361 ;
;; 1E00           03362         END     OVERLAY

	LAST_PATCH
