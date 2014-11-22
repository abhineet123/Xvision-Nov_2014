VERSION = 0.8
FGRAB_METEOR = fgrab_meteor-$(VERSION)
FORMAT = NTSC
DEVICE = DEV0
BOARD = NON_NATOMA

all: fgrab_wish

fgrab_wish: fgrab_tkAppInit.o fgrab_meteor.o
	cc -o fgrab_wish-$(VERSION) fgrab_tkAppInit.o $(FGRAB_METEOR).o\
	-L/usr/X11R6/lib\
	-ltk8.0 -ltcl8.0 -lXext -lX11 -lm -ldl
fgrab_tkAppInit.o: fgrab_tkAppInit.c; cc -c -g -I../.. fgrab_tkAppInit.c
fgrab_meteor.o:    $(FGRAB_METEOR).c;
	 cc -D$(FORMAT) -D$(DEVICE) -D$(BOARD) -c -g -I../.. $(FGRAB_METEOR).c


clean: 
	rm -f *.o
