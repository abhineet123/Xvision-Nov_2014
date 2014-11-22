# Generated automatically from Makefile.in by configure.
PROG		= xgest
CUR_DEPTH	= ../../../
CPPFLAGS	=  \
                  -I$(CUR_DEPTH)/include 
OBJS		=  Gesture.o HCIWindow.o main.o
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVSeg -lXVImages\
		  -lXVTools  -lXext -lX11 -lpng  -lpthread 

all: .depend $(OBJS)
	$(CXX) -o $(PROG) $(OBJS) $(LIBS) -lm

clean: ;	rm -f *.o
		rm -f $(PROG)

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

install: all
	install -c $(PROG) $(CUR_DEPTH)/bin

include .depend
