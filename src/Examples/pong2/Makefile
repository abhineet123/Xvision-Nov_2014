PROG		= pong2
CUR_DEPTH	= ../../../
CPPFLAGS	=  \
                  -I$(CUR_DEPTH)/include  -DSCREEN_DEPTH=$(SD)
OBJS		= PongWindow.o PongSelector.o main.o
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVSeg -lXVImages\
		   -lXext -lX11 -lpng  -lpthread 

SD = $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG) .depend

.depend: $(OBJS:.o=.cc) 
	$(CC) -M -g $(CPPFLAGS) $(OBJS:.o=.cc) > .depend

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

include .depend
