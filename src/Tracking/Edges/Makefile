CUR_DEPTH	= ../../..
CXXFLAGS        = -O3 -pipe -DDEBUG_LEVEL=0 -DHAVE_CONFIG_H
CPPFLAGS	= -I$(CUR_DEPTH)/include
OBJS		= XVPattern.o

all: .depend $(OBJS)

clean: ;	rm -f *.o
	        rm -f .depend

.depend: $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(OBJS:.o=.cc) > .depend 

include .depend

