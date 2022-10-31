;;;
;;; Copyright (C) 2013 Frederic Vecoven
;;;
;;; This file is part of trs_hard
;;;
;;; trs_hard is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or
;;; (at your option) any later version.
;;;
;;; trs_hard is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program. If not, see <http://www.gnu.org/licenses/>.
;;;

	list	p=18f4620
	include	p18f4620.inc
	include	bootloader.inc
	include trs_hard_defs.h
	include version.h

	;; extern udata
	extern	sector_buffer, extra_buffer

	;; extern udata access
	extern	state_status, state_wp, state_control
	extern	state_bytesdone, state_bytesdone2
	extern	state_command, state_command2, state_size2, state_error2
	extern	state_busy, val_1F, foo
	extern	state_error, state_cyl, state_seccnt, state_secnum
	extern	state_drive, state_head, state_present
	extern	state_secsize, state_secsize16
	extern	state_rom, state_romdone
	extern	action_type, action_flags
	

#define	TRS_ADDR	PORTA
#define TRS_DATA_IN	PORTD
#define TRS_DATA_OUT	LATD	
#define TRS_DATA_TRIS	TRISD
#define GAL_INT_IF	INTCON3,INT1IF
#define GAL_INT_IE	INTCON3,INT1IE	
#define GAL_INT		PORTB,1
#define TRS_WAIT	LATB,3
#define STAT_CS		LATC,0
#define GLED		LATB,7
#define RLED		LATB,6


int_var		UDATA_ACS
save_fsr0	res	2
		

;;;
;;; High priority interrupt code
;;;
;;; - placed at a fixed address APP_HI_INT, because the vector is in the
;;;   bootloader area.
;;;
;;; - everything is written in assembly code for speed. C is too slow,
;;;   specially when functions must be called. To achieve this, the
;;;   state variables have been placed in access memory ("near" in C).
;;;
;;; - if something takes too long, it is done in the main loop by executing
;;;   an action. During that time, the TRS must poll the status register and
;;;   wait until the action completes
;;; 
int_code	CODE APP_HI_INT	

handle_int2
	GLOBAL	handle_int2

	bcf	GAL_INT_IF	; acknowledge the interrupt

	tstfsz	state_busy	; state_busy should always by 0
	bra	handle_busy	; (unless TRS forgot to poll STATUS)
	
	movf	TRS_ADDR,w	; dispatch based on the TRS address
	andlw	0x1F		; PORTA = A3 A2 A1 A0 R/W
	cpfseq	val_1F		; is it CF ? (write command)
	bra	jj		; no, just jump
	swapf	TRS_DATA_IN,w
	andlw	0x0F		; lower nibble = command
	movwf	state_command	; save command here (save instructions later)
	clrf	state_bytesdone+0
	clrf	state_bytesdone+1
	addlw	0x1F		; add jump offset
jj	call	jump
	

	goto	trs_read_wp	    ; 	0 R C0 192
	goto	trs_read_data	    ; 	1 R C8 200
	goto	trs_read_rom	    ; 	2 R C4
	goto	trs_read_cyllo	    ; 	3 R CC 204
	goto	trs_read_data2	    ; 	4 R C2 194
	goto	trs_read_seccnt	    ; 	5 R CA 202
	goto	trs_read_uart_status ; 	6 R C6 
	goto	trs_read_sdh	    ; 	7 R CE 206
	goto	trs_read_control    ; 	8 R C1 193
	goto	trs_read_error	    ; 	9 R C9 201
	goto	trs_read_error2	    ; 	A R C5
	goto	trs_read_cylhi	    ; 	B R CD 205
	goto	trs_read_size2	    ; 	C R C3 195
	goto	trs_read_secnum	    ; 	D R CB 203
	goto	trs_read_uart	    ; 	E R C7
	goto	int_ret_rd	    ; 	F R CF 207 (74HC595)
	
	goto	int_ret_rd	     ; 	0 W C0
	goto	trs_write_data	     ; 	1 W C8 200
	goto	trs_write_command2   ; 	2 W C4 196
	goto	trs_write_cyllo	     ; 	3 W CC 204
	goto	trs_write_data2	     ; 	4 W C2 194
	goto	trs_write_seccnt     ; 	5 W CA 202
	goto	trs_write_uart_ctrl  ; 	6 W C6
	goto	trs_write_sdh	     ; 	7 W CE 206
	goto	trs_write_control    ; 	8 W C1 193
	goto	int_ret_rd	     ; 	9 W C9
	goto	trs_write_rom	     ; 	A W C5
	goto	trs_write_cylhi	     ; 	B W CD 205
	goto	trs_write_size2	     ; 	C W C3 195
	goto	trs_write_secnum     ; 	D W CB 203
	goto	trs_write_uart	     ; 	E W C7
	goto	int_ret_rd	     ; 	F W CF 207
	goto	trs_write_cmd_restore;  F W CF 207 (restore)
	goto	trs_write_cmd_read   ;  F W CF 207 (read)
	goto	trs_write_cmd_write  ;  F W CF 207 (write)
	goto	trs_write_cmd_verify ;  F W CF 207 (verify)
	goto	trs_write_cmd_format ;  F W CF 207 (format)
	goto	trs_write_cmd_init   ;  F W CF 207 (init)
	goto	trs_write_cmd_seek   ;  F W CF 207 (seek)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_test   ;  F W CF 207 (test - WD1002)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)
	goto	trs_write_cmd_nop    ;  F W CF 207 (no operation)

;;; implement a jump table by messing with the return stack
jump
	rlncf	WREG,w		;  goto instructions are 4 bytes
	rlncf	WREG,w
	addwf	TOSL,f
	;clrf	WREG		; NOT NEEDED. jump table above doesn't
	;addwfc	TOSH,f		; cross page boundary
	return

handle_busy
	btfsc	TRS_ADDR,4	; TRS read (0) or write (1) ?
	bra	int_ret_rd	; write => ignore
	movff	state_status,TRS_DATA_OUT ; read => return status
	bra	int_ret_wr
	
trs_read_wp
	movff	state_wp,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_control
	movff	state_control,TRS_DATA_OUT
	bra	int_ret_wr
	
trs_read_data
	movff	FSR0L,save_fsr0+0 	; save FSR0
	movff	FSR0H,save_fsr0+1
	lfsr	FSR0,sector_buffer
	movf	state_bytesdone+0,w
	addwf	FSR0L, f
	movf	state_bytesdone+1,w
	addwfc	FSR0H, f
	movff	INDF0,TRS_DATA_OUT
	infsnz	state_bytesdone+0,f	; bytesdone++
	incf	state_bytesdone+1,f
	movf	state_bytesdone+0,w	; bytesdone == secsize16 ?
	subwf	state_secsize16+0,w
	bnz	int_ret_wr_FSR0		; no, provide data + restore FSR0
	movf	state_bytesdone+1,w
	subwf	state_secsize16+1,w
	bnz	int_ret_wr_FSR0		; no, provide data + restore FSR0
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE)	; done !
	movwf	SSPBUF
	movwf	state_status
	bra	int_ret_wr_FSR0_S	; provide data + restore FSR0 + status


trs_read_data2
	movff	FSR0L,save_fsr0+0 ; save FSR0
	movff	FSR0H,save_fsr0+1
	lfsr	FSR0,extra_buffer
	movff	state_bytesdone2, FSR0L
	incf	state_bytesdone2,f
	movff	INDF0,TRS_DATA_OUT
	bra	int_ret_wr_FSR0 ; provide data + restore FSR0

trs_read_size2
	movff	state_size2+0,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_error
	movff	state_error,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_error2
	movff	state_error2,TRS_DATA_OUT
	bra	int_ret_wr
		
trs_read_secnum
	movff	state_secnum,TRS_DATA_OUT
	bra	int_ret_wr
		
trs_read_seccnt
	movff	state_seccnt,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_cyllo
	movff	state_cyl+0,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_cylhi
	movff	state_cyl+1,TRS_DATA_OUT
	bra	int_ret_wr

trs_read_sdh
	rlncf	state_secsize,w
	swapf	WREG,w
	movwf	foo
	rlncf	state_drive,w
	rlncf	WREG,w
	rlncf	WREG,w
	iorwf	state_head,w
	iorwf	foo,w
	movwf	TRS_DATA_OUT
	bra	int_ret_wr


trs_read_uart_status ; 	6 R C6
trs_read_uart	    ; 	E R C7
	bra	int_ret_rd

trs_write_control
	movf	TRS_DATA_IN,w
	movwf	state_control
	btfsc	WREG,3			;  TRS_HARD_DEVICE_ENABLE = 0x8
	bra	int_ret_rd
	tstfsz	state_present
	bra	twco0
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR)
	movwf	SSPBUF
	movwf	state_status
	movlw	TRS_HARD_NFERR
	movwf	state_error
	bra	int_ret_rd_S
twco0	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE)
	movwf	SSPBUF
	movwf	state_status
	bra	int_ret_rd_S

trs_write_data
	movf	state_command,w
	xorlw	(TRS_HARD_WRITE >> 4)
	bnz	int_ret_rd
	movff	FSR0L,save_fsr0+0
	movff	FSR0H,save_fsr0+1
	lfsr	FSR0,sector_buffer
	movf	state_bytesdone+0,w
	addwf	FSR0L,f
	movf	state_bytesdone+1,w
	addwfc	FSR0H,f
	movff	TRS_DATA_IN,INDF0
	movff	save_fsr0+0,FSR0L	; restore FSR0
	movff	save_fsr0+1,FSR0H
	infsnz	state_bytesdone+0,f	; bytesdone++
	incf	state_bytesdone+1,f
	movf	state_bytesdone+0,w	; bytesdone == secsize16 ?
	subwf	state_secsize16+0,w
	bnz	int_ret_rd
	movf	state_bytesdone+1,w
	subwf	state_secsize16+1,w
	bnz	int_ret_rd
	movlw	(TRS_HARD_BUSY | TRS_HARD_CIP)
	movwf	SSPBUF
	movwf	state_status
	movlw	ACTION_HARD_WRITE
	movwf	action_type
	bsf	action_flags,ACTION_TRS_BIT
	bra	int_ret_rd_S

trs_write_data2
	movff	FSR0L,save_fsr0+0
	movff	FSR0H,save_fsr0+1
	lfsr	FSR0,extra_buffer
	movff	state_bytesdone2,FSR0L
	movff	TRS_DATA_IN,INDF0
	movff	save_fsr0+0,FSR0L	; restore FSR0
	movff	save_fsr0+1,FSR0H
	incf	state_bytesdone2,f
	movf	state_size2,w
	cpfseq	state_bytesdone2
	bra	int_ret_rd
	movlw	ACTION_EXTRA2
	bra	action_extraN

;;;
;;; End of interrupt routine. 
;;;
;;; The code is put here so that "bra" instructions can be used above 
;;; and below us.
;;;
;;; Variants: with/without restore FSR0 and with/without update statatus
;;;
int_ret_wr_FSR0_S
	btfss	SSPSTAT,BF
	bra	$-2
	bsf	STAT_CS
	nop
	bcf	STAT_CS
int_ret_wr_FSR0
	movff	save_fsr0+0,FSR0L	; restore FSR0
	movff	save_fsr0+1,FSR0H
int_ret_wr
	clrf	TRS_DATA_TRIS		;  if TRS reads, put data on the bus
int_ret_rd	
	bsf	TRS_WAIT		; tell GAL to release WAIT*
	btfss	GAL_INT			; wait transaction completion
	bra	$-2
	setf	TRS_DATA_TRIS
	bcf	TRS_WAIT		; get ready for next transaction
	retfie	FAST

;;; with status update
int_ret_rd_S
	btfss	SSPSTAT,BF
	bra	$-2
	bsf	STAT_CS	
	bsf	TRS_WAIT		; tell GAL to release WAIT*
	btfss	GAL_INT			; wait transaction completion
	bra	$-2
	setf	TRS_DATA_TRIS
	movf	SSPBUF,w
	bcf	STAT_CS
	bcf	TRS_WAIT		; get ready for next transaction
	retfie	FAST
	
	
trs_read_rom
	movlw	0x02			; 2nd byte (going to RAM) ?
	cpfseq	state_romdone
	bra	trr1
	movff	state_rom,TRS_DATA_OUT	; yes, return state_rom
	bra	trr2
trr1	movff	state_romdone,EEADR	; no, return eeprom byte
	clrf	EEADRH
	bcf	EECON1,EEPGD
	bcf	EECON1,CFGS
	bsf	EECON1,RD
	movff	EEDATA,TRS_DATA_OUT
trr2	incf	state_romdone,f
	bra	int_ret_wr

trs_write_seccnt
	movff	TRS_DATA_IN,state_seccnt
	bra	int_ret_rd
	
trs_write_secnum
	movff	TRS_DATA_IN,state_secnum
	bra	int_ret_rd
	
trs_write_cyllo
	movff	TRS_DATA_IN,state_cyl
	bra	int_ret_rd
	
trs_write_cylhi
	movf	TRS_DATA_IN,w		; cylinder is 10 bits
	andlw	0x03
	movwf	state_cyl+1
	bra	int_ret_rd
	
trs_write_sdh
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE)
	movwf	SSPBUF
	movwf	state_status
	clrf	state_secsize16+0
	clrf	state_secsize16+1
	rrncf	TRS_DATA_IN,w
	swapf	WREG,w
	andlw	0x3
	movwf	state_secsize		; trs_data_in[6:5]
	bnz	tws_01
	incf	state_secsize16+1,f	; 00 => 256 bytes   size16 = 0100h
	bra	tws_10
tws_01	xorlw	0x1
	bnz	tws_02
	movlw	0x2
	movwf	state_secsize16+1	; 01 => 512 bytes   size16 = 0200h
	bra	tws_10	
tws_02	xorlw	0x1 ^ 0x2
	bnz	tws_03
	movlw	0x4
	movwf	state_secsize16+1	; 10 => 1024 bytes  size16 = 0400h
	bra	tws_10
tws_03	movlw	0x80
	movwf	state_secsize16+0	; 11 => 128 bytes   size16 = 0080h
tws_10
	rlncf	TRS_DATA_IN,w
	swapf	WREG,w
	andlw	0x3
	movwf	state_drive		; trs_data_in[4:3]
	movf	TRS_DATA_IN,w
	andlw	0x7
	movwf	state_head		; trs_data_in[2:0]
	
	bra	int_ret_rd_S
	
trs_write_size2
	clrf	state_size2+1
	movf	TRS_DATA_IN,w
	movwf	state_size2+0
	bnz	int_ret_rd
	incf	state_size2+1,f	; 0 --> 0x100
	bra	int_ret_rd

trs_write_cmd_restore		; RESTORE
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE)
	movwf	SSPBUF
	movwf	state_status
	clrf	state_cyl
	clrf	state_cyl+1
	bra	int_ret_rd_S

trs_write_cmd_read		; READ
	movlw	(TRS_HARD_BUSY | TRS_HARD_CIP)
	movwf	SSPBUF
	movwf	state_status
	movlw	ACTION_HARD_READ
	movwf	action_type
	bsf	action_flags,ACTION_TRS_BIT
	bra	int_ret_rd_S

trs_write_cmd_write		; WRITE
	movf	TRS_DATA_IN,w	; we could probably skip this check
	andlw	TRS_HARD_MULTI
	bnz	twc_1
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_DRQ)
	movwf	SSPBUF
	movwf	state_status
	bra	int_ret_rd_S
twc_1	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE | TRS_HARD_ERR)
	movwf	SSPBUF
	movwf	state_status
	movlw	TRS_HARD_ABRTERR
	movwf	state_error
	bra	int_ret_rd_S
	
trs_write_cmd_verify		; VERIFY
trs_write_cmd_seek		; SEEK
	movlw	(TRS_HARD_BUSY | TRS_HARD_CIP)
	movwf	SSPBUF
	movwf	state_status
	movlw	ACTION_HARD_SEEK
	movwf	action_type
	bsf	action_flags,ACTION_TRS_BIT
	bra	int_ret_rd_S

trs_write_cmd_nop		; No Operation	
trs_write_cmd_format		; FORMAT
trs_write_cmd_init		; INIT
	movlw	(TRS_HARD_READY | TRS_HARD_SEEKDONE)
	movwf	SSPBUF
	movwf	state_status
	bra	int_ret_rd_S

trs_write_cmd_test			; TEST (WD-1002)
	clrf	state_error		; Clear error register
	movlw	TRS_HARD_READY
	movwf	SSPBUF
	movwf	state_status
	bra	int_ret_rd_S

trs_write_command2
	movff	TRS_DATA_IN,state_command2
	clrf	state_bytesdone2
	movlw	ACTION_EXTRA
action_extraN
	iorwf	state_command2,w
	movwf	action_type
	movlw	(TRS_HARD_BUSY | TRS_HARD_CIP)
	movwf	SSPBUF
	movwf	state_status
	bsf	action_flags,ACTION_TRS_BIT
	bra	int_ret_rd_S

trs_write_rom
	movff	TRS_DATA_IN,state_rom
	clrf	state_romdone
	bra	int_ret_rd

trs_write_uart_ctrl  ; 	6 W C6
trs_write_uart	     ; 	E W C7
	bra	int_ret_rd




	END
	
