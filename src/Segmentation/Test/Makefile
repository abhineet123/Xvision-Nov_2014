CUR_DEPTH = ../../../
PROG1     = rangeTest
PROG2     = hueTest
OBJS      = rangeTest.o hueTest.o
CPPFLAGS	= -I/usr/local/share/research/src/bttv\
                  -I$(CUR_DEPTH)/include -DSCREEN_DEPTH=$(SD) -g
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools -lXVSeg -lmpeg -ljpeg -ltiff -lXVImages \
		   -lXext -lX11 -lpng  -lpthread 
SD 		= $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(PROG1) $(PROG2)


$(PROG1): $(PROG1).o
	$(CXX) -o $(PROG1) $(PROG1).o $(LIBS) -lm

$(PROG2): $(PROG2).o
	$(CXX) -o $(PROG2) $(PROG2).o $(LIBS) -lm

clean: ;  rm -f *.o

.depend:  $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend
