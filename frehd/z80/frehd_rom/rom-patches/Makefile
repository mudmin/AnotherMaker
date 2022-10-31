
ASM = asl
P2BIN = p2bin
MV = /bin/mv

M1SRC = model1rom.asm
M1LST = $(M1SRC:%.asm=%.lst)
M1BIN1 = $(M1SRC:%.asm=%.p)
M1BIN2 = $(M1SRC:%.asm=%.bin)
M1BINA = $(M1SRC:%.asm=%-A.bin)
M1BINB = $(M1SRC:%.asm=%-B.bin)
M1BINC = $(M1SRC:%.asm=%-C.bin)
M1HEX = $(M1SRC:%.asm=%.hex)
M1LST = $(M1SRC:%.asm=%.lst)

M3SRC = model3rom.asm
M3LST = $(M3SRC:%.asm=%.lst)
M3BIN1 = $(M3SRC:%.asm=%.p)
M3BIN2 = $(M3SRC:%.asm=%.bin)
M3BINA = $(M3SRC:%.asm=%-A.bin)
M3BINB = $(M3SRC:%.asm=%-B.bin)
M3BINC = $(M3SRC:%.asm=%-C.bin)
M3HEX = $(M3SRC:%.asm=%.hex)
M3LST = $(M3SRC:%.asm=%.lst)

M4SRC = model4rom.asm
M4LST = $(M4SRC:%.asm=%.lst)
M4BIN1 = $(M4SRC:%.asm=%.p)
M4BIN2 = $(M4SRC:%.asm=%.bin)
M4BINA = $(M4SRC:%.asm=%-A.bin)
M4BINB = $(M4SRC:%.asm=%-B.bin)
M4BINC = $(M4SRC:%.asm=%-C.bin)
M4BINBC = $(M4SRC:%.asm=%-BC.bin)
M4HEX = $(M4SRC:%.asm=%.hex)
M4LST = $(M4SRC:%.asm=%.lst)

M4G2SRC = model4rom.gen2.asm
M4G2LST = $(M4G2SRC:%.asm=%.lst)
M4G2BIN1 = $(M4G2SRC:%.asm=%.p)
M4G2BIN2 = $(M4G2SRC:%.asm=%.bin)
M4G2BINA = $(M4G2SRC:%.asm=%-A.bin)
M4G2BINB = $(M4G2SRC:%.asm=%-B.bin)
M4G2BINC = $(M4G2SRC:%.asm=%-C.bin)
M4G2BINBC = $(M4G2SRC:%.asm=%-BC.bin)
M4G2HEX = $(M4G2SRC:%.asm=%.hex)
M4G2LST = $(M4G2SRC:%.asm=%.lst)

M4ASRC = model4frenchrom.asm
M4ALST = $(M4ASRC:%.asm=%.lst)
M4ABIN1 = $(M4ASRC:%.asm=%.p)
M4ABIN2 = $(M4ASRC:%.asm=%.bin)
M4ABINA = $(M4ASRC:%.asm=%-A.bin)
M4ABINB = $(M4ASRC:%.asm=%-B.bin)
M4ABINC = $(M4ASRC:%.asm=%-C.bin)
M4AHEX = $(M4ASRC:%.asm=%.hex)
M4ALST = $(M4ASRC:%.asm=%.lst)

M4GSRC = model4germanrom.asm
M4GLST = $(M4GSRC:%.asm=%.lst)
M4GBIN1 = $(M4GSRC:%.asm=%.p)
M4GBIN2 = $(M4GSRC:%.asm=%.bin)
M4GBINA = $(M4GSRC:%.asm=%-A.bin)
M4GBINB = $(M4GSRC:%.asm=%-B.bin)
M4GBINC = $(M4GSRC:%.asm=%-C.bin)
M4GHEX = $(M4GSRC:%.asm=%.hex)
M4GLST = $(M4GSRC:%.asm=%.lst)

M4PSRC = model4p_rom.asm
M4PLST = $(M4PSRC:%.asm=%.lst)
M4PBIN1 = $(M4PSRC:%.asm=%.p)
M4PBIN2 = $(M4PSRC:%.asm=%.bin)
M4PLST = $(M4PSRC:%.asm=%.lst)


SVN_REV := $(shell svn info . | sed -n '/^Revision: /s/Revision: //p')

all: $(M1BIN2) $(M3BIN2) $(M4BIN2) $(M4G2BIN2) $(M4ABIN2) $(M4GBIN2) $(M4PBIN2)

clean:
	$(RM) $(M1BIN1) $(M1BIN2) $(M1BINA) $(M1BINB) $(M1BINC) $(M1LST) $(M1HEX) 
	$(RM) $(M3BIN1) $(M3BIN2) $(M3BINA) $(M3BINB) $(M3BINC) $(M3LST) $(M3HEX) 
	$(RM) $(M4BIN1) $(M4BIN2) $(M4BINA) $(M4BINB) $(M4BINC) $(M4BINBC) $(M4LST) $(M4HEX) 
	$(RM) $(M4G2BIN1) $(M4G2BIN2) $(M4G2BINA) $(M4G2BINB) $(M4G2BINC) $(M4G2BINBC)  $(M4G2LST) $(M4G2HEX) 
	$(RM) $(M4ABIN1) $(M4ABIN2) $(M4ABINA) $(M4ABINB) $(M4ABINC) $(M4ALST) $(M4AHEX) 
	$(RM) $(M4PBIN1) $(M4PBIN2) $(M4PLST)

$(M1BIN2): $(M1SRC)
	$(ASM) $(M1SRC) -U -L -u -D SVN_REV=1 -o $(M1BIN1)
	$(P2BIN) -r 0x0000-0x2fff $(M1BIN1) $(M1BIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M1BIN1) $(M1BINA)
	$(P2BIN) -r 0x2000-0x27ff $(M1BIN1) $(M1BINB)
	$(P2BIN) -r 0x2800-0x2fff $(M1BIN1) $(M1BINC)

$(M3BIN2): $(M3SRC)
	$(ASM) $(M3SRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M3BIN1)
	$(P2BIN) -r 0x0000-0x37ff $(M3BIN1) $(M3BIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M3BIN1) $(M3BINA)
	$(P2BIN) -r 0x2000-0x2fff $(M3BIN1) $(M3BINB)
	$(P2BIN) -r 0x3000-0x37ff $(M3BIN1) $(M3BINC)

$(M4BIN2): $(M4SRC)
	$(ASM) $(M4SRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M4BIN1)
	$(P2BIN) -r 0x0000-0x37ff $(M4BIN1) $(M4BIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M4BIN1) $(M4BINA)
	$(P2BIN) -r 0x2000-0x2fff $(M4BIN1) $(M4BINB)
	$(P2BIN) -r 0x3000-0x37ff $(M4BIN1) $(M4BINC)
	$(P2BIN) -r 0x2000-0x37ff $(M4BIN1) $(M4BINBC)

$(M4G2BIN2): $(M4G2SRC)
	$(ASM) $(M4G2SRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M4G2BIN1)
	$(P2BIN) -r 0x0000-0x37ff $(M4G2BIN1) $(M4G2BIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M4G2BIN1) $(M4G2BINA)
	$(P2BIN) -r 0x2000-0x2fff $(M4G2BIN1) $(M4G2BINB)
	$(P2BIN) -r 0x3000-0x37ff $(M4G2BIN1) $(M4G2BINC)
	$(P2BIN) -r 0x2000-0x37ff $(M4G2BIN1) $(M4G2BINBC)


$(M4ABIN2): $(M4ASRC)
	$(ASM) $(M4ASRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M4ABIN1)
	$(P2BIN) -r 0x0000-0x37ff $(M4ABIN1) $(M4ABIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M4ABIN1) $(M4ABINA)
	$(P2BIN) -r 0x2000-0x2fff $(M4ABIN1) $(M4ABINB)
	$(P2BIN) -r 0x3000-0x37ff $(M4ABIN1) $(M4ABINC)

$(M4GBIN2): $(M4GSRC)
	$(ASM) $(M4GSRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M4GBIN1)
	$(P2BIN) -r 0x0000-0x37ff $(M4GBIN1) $(M4GBIN2)
	$(P2BIN) -r 0x0000-0x1fff $(M4GBIN1) $(M4GBINA)
	$(P2BIN) -r 0x2000-0x2fff $(M4GBIN1) $(M4GBINB)
	$(P2BIN) -r 0x3000-0x37ff $(M4GBIN1) $(M4GBINC)

$(M4PBIN2): $(M4PSRC)
	$(ASM) $(M4PSRC) -U -L -u -D SVN_REV=$(SVN_REV) -o $(M4PBIN1)
	$(P2BIN) -r 0x0000-0x0fff $(M4PBIN1) $(M4PBIN2)



