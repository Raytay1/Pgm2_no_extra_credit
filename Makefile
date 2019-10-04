#Makefile

PROGRAMS = \
	cs3743p2 \
	clean \

all: $(PROGRAMS)

#rules for creating each .o file
cs3743p2Driver.o: cs3743p2Driver.c cs3743p2.h
	gcc -g -c cs3743p2Driver.c -o cs3743p2Driver.o

cs3743p2.o: cs3743p2.c cs3743p2.h
	gcc -g -c cs3743p2.c -o cs3743p2.o

#rules for creating the executable
cs3743p2: cs3743p2Driver.o cs3743p2.o
	gcc -g -o cs3743p2 cs3743p2Driver.o cs3743p2.o

#rules for removing the .o files
clean:
	rm -f *.o