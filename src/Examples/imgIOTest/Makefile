
PROG		= imgIOTest
CUR_DEPTH	= ../../../
CPPFLAGS	= -I$(CUR_DEPTH)/include -g -DSCREEN_DEPTH=$(SD)
OBJS		= imageIOTest.o 
LIBS		= -L$(CUR_DEPTH)/lib -lXVCons -lXVImages -ljpeg -ltiff \
		    -lXext -lX11 -lpng  -lpthread 


# if you want to set the screen depth explicitly,
# do it here.  Otherwise, the depth of the screen 
# used to compile XVision2 (the screen you type "make" in)
# will be determined and used.
SD = $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(OBJS)
	$(CXX) -gp -o $(PROG) $(OBJS) $(LIBS) -lm 

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

clean: ;	rm -f *.o
		rm -f $(PROG)

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
