# makefile for nesdoug example code, for Linux
# this version won't move the final files to BUILD folder
# also won't rebuild on changes to neslib/nesdoug/famitone files

CC65 = ../bin/cc65
CA65 = ../bin/ca65
LD65 = ../bin/ld65
NAME = platformer5
CFG = nrom_32k_vert.cfg


.PHONY: default clean

default: $(NAME).nes


#target: dependencies

$(NAME).nes: $(NAME).o crt0.o $(CFG)
	$(LD65) -C $(CFG) -o $(NAME).nes crt0.o $(NAME).o nes.lib -Ln labels.txt --dbgfile dbg.txt
	rm *.o
	@echo $(NAME).nes created

crt0.o: crt0.s platformer5.chr MUSIC/TestMusic3.s MUSIC/SoundFx.s
	$(CA65) crt0.s

$(NAME).o: $(NAME).s
	$(CA65) $(NAME).s -g

$(NAME).s: $(NAME).c Sprites.h platformer5.h BG/Room1.c BG/Room2.c BG/Room3.c BG/Room4.c BG/Room5.c
	$(CC65) -Oirs $(NAME).c --add-source

clean:
	rm $(NAME).nes
