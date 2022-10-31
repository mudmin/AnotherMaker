LDOS = export2.cmd import2.cmd fupdate.cmd vhdutl.cmd import3.cmd dsk.dct eupdate.cmd
CPM = vhdutl.com vhdutlkp.com

LST = export2.lst import2.lst import3.lst fupdate.lst vhdutl_ldos.lst vhdutl_trs_cpm.lst vhdutl_kpro_cpm.lst dsk.lst

ZMACFLAGS_LDOS = -h
ZMACFLAGS_CPM  =

RM = /bin/rm -f
MV = /bin/mv -f


all: $(LDOS) $(CPM)

.SUFFIXES:	.z80 .cmd .dct .man .txt .hex .bin .com

vhdutl.cmd: vhdutl_ldos.cmd
	$(MV) vhdutl_ldos.cmd vhdutl.cmd

vhdutl.com: vhdutl_trs_cpm.bin
	$(MV) vhdutl_trs_cpm.bin vhdutl.com
	
vhdutlkp.com: vhdutl_kpro_cpm.bin
	$(MV) vhdutl_kpro_cpm.bin vhdutlkp.com

.z80.cmd:
	zmac $(ZMACFLAGS_LDOS) $<
	hex2cmd $*.hex > $*.cmd
	$(RM) $*.hex
.z80.dct:
	zmac $(ZMACFLAGS_LDOS) $<
	hex2cmd $*.hex > $*.dct
	$(RM) $*.hex
.z80.bin:
	zmac $(ZMACFLAGS_CPM) $<

clean:
	$(RM) $(LDOS) $(CPM) $(LST)
