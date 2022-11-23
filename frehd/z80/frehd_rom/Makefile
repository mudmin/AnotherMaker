
ASM = asl
P2BIN = p2bin
P2HEX = p2hex
HEX2CMD = hex2cmd
MV = /bin/mv

SRC = frehd.asm
PATCHES = ldos631.asm ldos531.asm m1ldos531.asm m1newdos25.asm newdos25.asm menu.asm time.asm menu_util.asm
LST = $(SRC:%.asm=%.lst)

BIN1 = $(SRC:%.asm=%.p)
BIN2 = $(SRC:%.asm=%.bin)
ROM = $(SRC:%.asm=%.rom)
HEX = $(SRC:%.asm=%.hex)
HEXX = $(SRC:%.asm=%.hex2)
LST = $(SRC:%.asm=%.lst)

LSRC = loader.asm
LOUT = loader_eeprom.asm
LOUT2 = loader_xtrs.h
LBIN1 = $(LSRC:%.asm=%.p)
LBIN2 = $(LSRC:%.asm=%.bin)
LHEX = $(LSRC:%.asm=%.hex)
LLST = $(LSRC:%.asm=%.lst)


SVN_REV := $(shell svn info . | sed -n '/^Revision: /s/Revision: //p')

all: $(ROM) $(LBIN2)

clean:
	$(RM) $(BIN1) $(BIN2) $(LST) $(HEX) $(HEXX) $(ROM)
	$(RM) $(LBIN1) $(LBIN2) $(LLST) $(LHEX) $(LOUT) $(LOUT2)

#
# frehd.rom
#
$(ROM): $(SRC) $(PATCHES)
	$(ASM) $(SRC) -U -L -u -D SVN_REV=$(SVN_REV) -D ALWAYS=1 -o $(BIN1)
	$(P2BIN) -r 0x6000-$$ $(BIN1) $(BIN2)
	$(P2HEX) -e 0x6000 $(BIN1) $(HEXX)
	cat $(HEXX) | awk 'NR>1{print buf}{buf = $$0}' > $(HEX)
	$(RM) $(ROM) $(HEXX)
	$(HEX2CMD) $(HEX) > $(ROM)


#
# loader
#
$(LBIN2): $(LSRC)
	$(ASM) $(LSRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(LBIN1)
	$(P2BIN) -r 0x5000-0x50FF $(LBIN1) $(LBIN2)
	./make_eeprom.pl $(LBIN2) $(LOUT) $(LOUT2)

