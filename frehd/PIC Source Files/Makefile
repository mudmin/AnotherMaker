
C_FILES = $(wildcard *.c)
H_FILES = $(wildcard *.h)
INC_FILES = $(wildcard *.inc)
ASM_FILES = $(wildcard *.asm)
Z80_FILES = z80/import2.z80 z80/export2.z80 z80/fupdate.z80 z80/vhdutl.z80

EXTRA_FILES = LICENSE FatFS generate_crc.pl 

FILES = $(C_FILES) $(H_FILES) $(INC_FILES) $(ASM_FILES) $(Z80_FILES) $(EXTRA_FILES)

DIST = trs_hard.tar.gz

all:
	$(RM) $(DIST)
	tar --exclude .svn -zcvf $(DIST) $(FILES)

clean:
	$(RM) $(DIST)
