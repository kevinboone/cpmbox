# This is the Makefile for building cpmbox in my emulation environment.

CPM=cpm

COMS=ls.com cat.com mv.com cp.com untar.com hexdump.com cal.com du.com find.com 

LSOBJS=ls.o dirs.o getopt.o compat.o error.o term.o
CATOBJS=cat.o dirs.o getopt.o compat.o error.o term.o bdos.o
CALOBJS=cal.o getopt.o date.o 
UNTAROBJS=untar.o getopt.o compat.o error.o bdos.o 
DUOBJS=du.o dirs.o getopt.o compat.o error.o term.o bdos.o
CPOBJS=cp.o dirs.o getopt.o compat.o error.o term.o bdos.o
MVOBJS=mv.o dirs.o getopt.o compat.o error.o term.o bdos.o
HEXDUMPOBJS=hexdump.o dirs.o getopt.o compat.o error.o term.o bdos.o

all: $(COMS) 

ls.asm: ls.c defs.h dirs.h getopt.h compat.h config.h
	$(CPM) cc ls.c

cat.asm: cat.c defs.h dirs.h getopt.h compat.h config.h
	$(CPM) cc cat.c

hexdump.asm: hexdump.c defs.h dirs.h getopt.h compat.h config.h
	$(CPM) cc hexdump.c

mv.asm: mv.c defs.h dirs.h getopt.h compat.h config.h
	$(CPM) cc mv.c

cp.asm: cp.c defs.h dirs.h getopt.h compat.h config.h
	$(CPM) cc cp.c

untar.asm: untar.c defs.h getopt.h compat.h bdos.h config.h
	$(CPM) cc untar.c

cal.asm: cal.c defs.h date.h getopt.h config.h
	$(CPM) cc cal.c

du.asm: du.c defs.h date.h getopt.h config.h
	$(CPM) cc du.c

find.asm: find.c defs.h dirs.h getopt.h config.h
	$(CPM) cc find.c

date.asm: date.c date.h defs.h getopt.h config.h
	$(CPM) cc date.c

dirs.asm: dirs.c defs.h bdos.h dirs.h
	$(CPM) cc dirs.c

getopt.asm: defs.h getopt.h getopt.c
	$(CPM) cc getopt.c

compat.asm: defs.h compat.h compat.c
	$(CPM) cc compat.c

error.asm: defs.h error.h error.c
	$(CPM) cc error.c

term.asm: defs.h term.h term.c
	$(CPM) cc term.c

bdos.asm: defs.h bdos.h bdos.c
	$(CPM) cc bdos.c

%.o: %.asm
	$(CPM) as $<

ls.com: $(LSOBJS) 
	$(CPM) ln $(LSOBJS) c.lib 

cat.com: $(CATOBJS) 
	$(CPM) ln $(CATOBJS) c.lib 

hexdump.com: $(HEXDUMPOBJS)
	$(CPM) ln $(HEXDUMPOBJS) c.lib 
mv.com: $(MVOBJS) 	
	$(CPM) ln $(MVOBJS) c.lib 

cp.com: $(CPOBJS)
	$(CPM) ln $(CPOBJS) c.lib 

du.com: $(DUOBJS)
	$(CPM) ln $(DUOBJS)  c.lib 

untar.com: $(UNTAROBJS)
	$(CPM) ln $(UNTAROBJS) c.lib 

cal.com: $(CALOBJS)
	$(CPM) ln $(CALOBJS)  c.lib 

find.com: find.o dirs.o getopt.o compat.o error.o term.o bdos.o
	$(CPM) ln find.o dirs.o getopt.o compat.o error.o term.o bdos.o c.lib 

clean:
	rm -f $(COMS) *.asm *.o

