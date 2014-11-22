PROG		= pong
CUR_DEPTH	= ../../../
CPPFLAGS	= \
                  -I$(CUR_DEPTH)/include 
OBJS		= PongWindow.o main.o
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVColSelector\
		   -lXext -lX11 -lpng  -lpthread 

all: .depend $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG) .depend

.depend: $(OBJS:.o=.cc)
	$(CC) -M  $(CPPFLAGS) $(OBJS:.o=.cc) > .depend

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

include .depend
