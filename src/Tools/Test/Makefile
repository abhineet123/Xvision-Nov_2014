CUR_DEPTH	= ../../../
CPPFLAGS	= -I$(CUR_DEPTH)/include -g -DSCREEN_DEPTH=$(SD)
OBJS		= tableConvert.o #test_tools.o crop.o 
PROGS           = tableConvert #test_tools crop
LIBS		= -L$(CUR_DEPTH)/lib -lXVDevs -lXVCons -lXVTools -lXVImages \
		   -lXext -lX11 -lpng  -lpthread 

SD = $(shell $(CUR_DEPTH)/bin/screendepth)

all: .depend $(OBJS)
	@for i in $(PROGS); do ($(CXX) -gp -o $$i $$i.o $(LIBS) -lm); done

clean: ;	rm -f *.o
		rm -f $(PROGS) .depend

install: all
	install -c $(PROGS) $(CUR_DEPTH)/bin

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend
