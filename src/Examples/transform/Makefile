PROG		= assigntest
CUR_DEPTH	= ../../../
CPPFLAGS	= -I$(CUR_DEPTH)/include -g
OBJS		= assigntest.o
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools\
		   -lXext -lX11 -lpng  -lpthread 

all: .depend $(OBJS)
	$(CXX) -pg -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG)

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
