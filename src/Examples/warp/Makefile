PROGS		= omniwarp planewarp affinewarp
CUR_DEPTH	= ../../../
CPPFLAGS	= -I$(CUR_DEPTH)/include -g -DSCREEN_DEPTH=$(SD) \
		  -I/usr/lib/glib/include -I../../drivers/libdv\
		  -I/usr/src/linux-2.4/drivers/ieee1394

LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools -lXVImages\
		  -ldv -ldc1394_control -lraw1394  -lglib\
		   -lXext -lX11 -lpng  -lpthread 

# if you want to set the screen depth explicitly,
# do it here.  Otherwise, the depth of the screen 
# used to compile XVision2 (the screen you type "make" in)
# will be determined and used.
SD = $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(PROGS)

$(PROGS): %: %.o
	$(CXX) -g -o $@ $< $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROGS) .depend

.depend: $(PROGS:=.cc)
	$(CC) -M  $(CPPFLAGS) $(PROGS:=.cc) > .depend

install: all
	install -c $(PROGS) $(CUR_DEPTH)/bin

include .depend
