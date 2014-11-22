CUR_DEPTH  = ../../
CXXFLAGS=-O3 -pipe -DDEBUG_LEVEL=0 -DHAVE_CONFIG_H -I$(IPPROOT)/include
CPPFLAGS	=   -I$(IPPROOT)/include  \
                  -I$(CUR_DEPTH)/include \
		  -I/lib/modules/`uname -r`/build/drivers/ieee1394\
		  -I/lib/modules/`uname -r`/build/drivers/media/video
OBJS		= XVPwc.o XVV4L2.o XVBt8x8.o Meteor.o XVDig1394.o XVFlea2G.o \
                   XVImageSeq.o XVVideo.o XVMpeg.o XVAVI.o
#XVAVI.o is element of IPPOBJS if HAVE_IPP is set

all: .depend $(OBJS)
	ar rv libXVDevs.a $(OBJS)

dummy: svs_dummy.o
	ar rv libsvs.a svs_dummy.o
	install -c libsvs.a $(CUR_DEPTH)/lib

clean: ;	rm -f *.o
	rm -f libXVDevs.a .depend

install: all
	install -c libXVDevs.a $(CUR_DEPTH)/lib

XVDig1394.cc:
	rm -f XVDig1394.cc
	ln -s XVDig1394_v2.cc XVDig1394.cc

XVDig1394.h:
	rm -f XVDig1394.h
	ln -s XVDig1394_v2.h XVDig1394.h

.depend: XVDig1394.h $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
