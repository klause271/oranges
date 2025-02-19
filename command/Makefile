# commands/Makefile

#ENTRYPOINT	= 0x1000
HD		= ../80m.img

ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
#ASMFLAGS	= -I ../include/ -f elf
#CFLAGS		= -I ../include/ -c -fno-builtin -fno-stack-protector -Wall
ASMFLAGS	= -I ../include/ -I ../include/sys/ -f elf
CFLAGS		= -I ../include/ -I ../include/sys/ -c -fno-builtin -fno-stack-protector -std=gnu99 -Wall
LDFLAGS		= -Ttext 0x1000
DASMFLAGS	= -D
LIB		= ../lib/orangescrt.a
BIN		= touch rm help echo pwd game calc test1 test2 ps ls cat kill mylog attack1 attack2 attack3

# All Phony Targets
.PHONY : everything final clean realclean disasm all install ps

# Default starting position
everything : $(BIN)

install : all clean
	cp ../kernel.bin ./ -v
	tar vcf inst.tar kernel.bin $(BIN)
	dd if=inst.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk -F " " '{print $$5}'` conv=notrunc

all : realclean everything

final : all clean

clean :
	rm -f *.o

realclean :
	rm -f $(BIN) *.o

kernel.bin :
	cp ../kernel.bin ./

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: pwd.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

pwd : pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

game.o: game.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

game : game.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

calc.o: calc.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

calc : calc.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

test1.o: test1.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

test1 : test1.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

test2.o: test2.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

test2 : test2.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

help.o: help.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

help : help.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

touch.o: touch.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

touch : touch.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

rm.o: rm.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

rm : rm.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ps.o: ps.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

ps : ps.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

search.o: search.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

search : search.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ls.o: ls.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

ls : ls.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

cat.o: cat.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

cat : cat.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

kill.o: kill.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

kill : kill.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

attack1.o: attack1.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

attack1 : attack1.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

attack2.o: attack2.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

attack2 : attack2.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

attack3.o: attack3.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

attack3 : attack3.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

mylog.o: mylog.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

mylog : mylog.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?
