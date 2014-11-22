CUR_DEPTH	= ../../
CXXFLAGS	= -O3 -pipe -DDEBUG_LEVEL=0
CPPFLAGS	= -I$(CUR_DEPTH)/include
OBJS		= XVSegmentation.o XVColorSeg.o XVMotionSeg.o\
		  XVScalarSeg.o

all: .depend $(OBJS)
	ar rv libXVSeg.a $(OBJS)

clean: ;	rm -f *.o
	rm -f libXVSeg.a .depend

install: all
	install -c libXVSeg.a $(CUR_DEPTH)/lib

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
