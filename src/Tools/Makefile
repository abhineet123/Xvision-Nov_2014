CUR_DEPTH	= ../../
CXXFLAGS	= -O3 -pipe -DDEBUG_LEVEL=0 -DHAVE_CONFIG_H 
CPPFLAGS	= -I$(CUR_DEPTH)/include  -I$(IPPROOT)/include

OBJS		= XVMatrix.o XVAffineWarp.o XVOmniWarper.o XVGeometry.o \
		  XVLookupTable.o XVStereoRectify.o

SDSRC           = sd.cc
SDBIN           = screendepth

all: .depend $(OBJS)
	ar rv libXVTools.a $(OBJS)

clean: ;	rm -f *.o
	rm -f libXVTools.a .depend $(SDBIN)

install: $(SDBIN) all 
	install -c libXVTools.a $(CUR_DEPTH)/lib

.depend: $(SDBIN) $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend

$(SDBIN): ;	$(CXX) -o $(SDBIN) $(CPPFLAGS) $(SDSRC)  -lXext -lX11 -lpng ;
	  	install -c $(SDBIN) $(CUR_DEPTH)/bin
