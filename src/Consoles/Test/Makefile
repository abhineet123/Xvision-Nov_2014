#PROG1		= test_consoles
PROG2           = interTest
CUR_DEPTH	= ../../../
CPPFLAGS	= -I$(CUR_DEPTH)/include -g
OBJS		= interTest.o #test_consoles.o 
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools -lXVImages -lmpeg \
		   -lXext -lX11 -lpng  -lpthread 

all: .depend $(OBJS)
#	$(CXX) -pg -o $(PROG1) $(OBJS) $(LIBS) -lm
	$(CXX) -pg -o $(PROG2) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG2) # $(PROG1) 

install: all
#	install -c $(PROG1) $(CUR_DEPTH)/bin
	install -c $(PROG2) $(CUR_DEPTH)/bin

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
