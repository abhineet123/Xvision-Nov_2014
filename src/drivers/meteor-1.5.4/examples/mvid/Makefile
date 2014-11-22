# Change to match your system PAL|SECAM|NTSC
SYSTEM = NTSC

all:  mvid

mvid: mvid.o 
	gcc -O2 mvid.o -o mvid -L/usr/X11/lib -lXext -lX11

mvid.o: mvid.c 
	gcc -D$(SYSTEM) -I../.. -O2 -c mvid.c

clean:
	/bin/rm -f *.o *~
