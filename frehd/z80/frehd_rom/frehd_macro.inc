
;;;
;;; This file should be included only once !
;;;

CNT	eval	0

START_PATCH MACRO addr
tmp	eval	"\{CNT}"
	dw	addr, (eee_{tmp}-sss_{tmp})
	phase	addr
sss_{tmp}:label $
	ENDM

END_PATCH MACRO
eee_{tmp}: label $
CNT	eval	CNT+1
	dephase
	ENDM

LAST_PATCH MACRO
	dw	0000h
	ENDM


REED_PATCH MACRO addr, offset, len
	dw	addr, ((80h | len) << 8) | offset
	ENDM
