CUR_DEPTH = ../../../
CPPFLAGS	= -I/usr/local/share/research/src/bttv\
                  -I$(CUR_DEPTH)/include -DSCREEN_DEPTH=$(SD) -g
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVImages -lXVCons -lXVTools -lXVSeg -lmpeg -ljpeg -ltiff \
		   -lXext -lX11 -lpng  -lpthread 

OBJS            = copyTest.o rowTest.o convertTest.o maskedTest.o gaussTest.o pngTest.o
PROGS           = copyTest rowTest convertTest maskedTest gaussTest pngTest

SD 		= $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(OBJS)
	@for i in $(PROGS); do ($(CXX) -gp -o $$i $$i.o $(LIBS) -lm); done

clean: ;  rm -f *.o
	  rm -f $(PROGS) .depend

.depend:  $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend
