# Generated automatically from Makefile.in by configure.
PROG		= dispfile
CUR_DEPTH	= $(HOME)/XVision2/
CPPFLAGS	= -g -I$(CUR_DEPTH)/include  -DSCREEN_DEPTH=$(SD) \
                   -I$(SVS_PATH)/src\
		  -I/usr/lib/glib/include -DLINUX -D_REENTRANT -DUNIX
OBJS		=  main.o
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools\
		  -lXVImages -lXVSeg -lXVTrack\
		  -L$(SVS_PATH)/bin -lsvs -lsvscalc -lsvscap -lpng -ljpeg\
		  -L/usr/X11R6/lib -lXxf86vm -lXxf86dga -lXext -lX11  -lpthread 

# if you want to set the screen depth explicitly,
# do it here.  Otherwise, the depth of the screen 
# used to compile XVision2 (the screen you type "make" in)
# will be determined and used.
SD = 32

all: .depend $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG) .depend
		rm -f core

cvsclean: ;	rm -f *.o
		rm -f $(PROG) .depend
		rm -f core
		rm -f *.cc~ *.h~ *.txt

.depend: $(OBJS:.o=.cc)
	$(CC) -M  $(CPPFLAGS) $(OBJS:.o=.cc) > .depend

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

include .depend
