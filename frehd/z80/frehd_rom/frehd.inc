
;;;
;;; FreHD defines
;;;

;; 
;; ROM patches
;;

;; where the extra "rom is loaded
ROM_LOAD	equ 5000h

;; TRS model : passed by the patched rom
ROM_MODEL_1	equ 1h
ROM_MODEL_3	equ 3h
ROM_MODEL_4	equ 4h
ROM_MODEL_4A	equ 4h			; model 4 azerty
ROM_MODEL_4P	equ 5h

;; 4P model 3 rom file
MODEL_III_FILE	equ "MODELA.III"

;; Rom passed parameter
ROM_MODEL	equ ROM_LOAD+1

;; FreHD Interface defines
CYLHI		equ 0CDh
CYLLO		equ 0CCh
SECCNT		equ 0CAh
SECNUM		equ 0CBh
DATA		equ 0C8h
ERROR		equ 0C9h
CONTROL		equ 0C1h
SDH		equ 0CEh
COMMAND		equ 0CFh		; write only
STATUS		equ 0CFh		; read only
COMMAND2	equ 0C4h		; write only
READROM		equ 0C4h		; read only
WRITEROM	equ 0C5h		; write only
DATA2		equ 0C2h
SIZE2		equ 0C3h
ERROR2		equ 0C5h		; read only
;; Interface commands
GET_TIME	equ 01h
OPEN_FILE	equ 03h
READ_FILE	equ 04h
CLOSE_FILE	equ 06h
OPEN_DIR	equ 08h
READ_DIR	equ 09h
MOUNT_DRIVE	equ 0Ah
INFO_DRIVE	equ 0Ch
READ_HEADER	equ 0Eh
;; status bits
STATUS_BUSY	equ 80h
STATUS_READY	equ 40h
STATUS_WRERR	equ 20h
STATUS_SEEKDONE	equ 10h
STATUS_DRQ	equ 08h
STATUS_ECC	equ 04h
STATUS_CIP	equ 02h
STATUS_ERR	equ 01h
;; get time parameter
FREHD_SEC	equ 00h
FREHD_MIN	equ 01h
FREHD_HOUR	equ 02h
FREHD_YEAR	equ 03h
FREHD_DAY	equ 04h
FREHD_MONTH	equ 05h
;; file info (returned by READ_DIR)
FIL_ATTRIB_OFFSET	equ 08h
FIL_FNAME_OFFSET	equ 09h
FIL_FNAME_LEN		equ 0ch		; 8.3
;; drive info (returned by INFO_DRIVE)
DRIVE_FNAME_OFFSET	equ 06h
;; attrib bits
FIL_ATTRIB_RO		equ 01h
FIL_ATTRIB_HID		equ 02h
FIL_ATTRIB_SYS		equ 04h
FIL_ATTRIB_VOL		equ 08h
FIL_ATTRIB_DIR		equ 10h
FIL_ATTRIB_ARCHIVE	equ 20h
;; open file options
FA_OPEN_EXISTING equ 00h
FA_READ          equ 01h
FA_WRITE         equ 02h
FA_CREATE_NEW    equ 04h
FA_CREATE_ALWAYS equ 08h
FA_OPEN_ALWAYS   equ 10h


;; Reed header  http://www.trs-80emulators.com/hdv1-format/
;;
;; byte 0-1
REED_MAGIC0	equ 56h
REED_MAGIC1	equ 0cbh
;; byte 8  : flag #2 -> bit 0 = auto-boot
;; byte 11 : disk format.
;;    0 = LDOS 6.3.1
;;    1 = LDOS 5.3.1
;;    2 = CPM
;;
REED_FLAG	equ 08h
REED_OS		equ 0Bh
OS_LDOS631	equ 0h
OS_LDOS531	equ 1h
OS_CPM		equ 2h
OS_NEWDOS25	equ 3h
OS_M1LDOS531	equ 4h
OS_M1NEWDOS25	equ 5h
MAX_OS		equ 6h

