PROG		= itest
CUR_DEPTH	= ../../../
CPPFLAGS	= -g -I$(CUR_DEPTH)/include 
OBJS		= itest.o 
LIBS		= -L$(CUR_DEPTH)/lib -lXVCons \
		   -lXext -lX11 -lpng  -lpthread 

all: .depend $(OBJS)
	$(CXX) -g -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG) .depend

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
