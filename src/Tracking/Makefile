CUR_DEPTH	= ../../
CXXFLAGS        = -O3 -pipe -DDEBUG_LEVEL=0 -DHAVE_CONFIG_H
CPPFLAGS	= -I$(CUR_DEPTH)/include 
DIRS            = Edges Blobs SSD
OBJS		= Edges/XVEdgeFeature.o Edges/XVPattern.o \
		  Blobs/XVBlobFeature.o \
		  SSD/XVSSD.o

all: .depend $(OBJS)
#	@for i in $(DIRS); do (cd $$i; $(MAKE)); done
	ar rv libXVTrack.a $(OBJS)

clean: ;	rm -f $(OBJS)
	rm -f libXVTrack.a .depend
#	@for i in $(DIRS); do(cd $$i; $(MAKE) clean); done 

install: all
	install -c libXVTrack.a $(CUR_DEPTH)/lib

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend

