CPPFLAGS += -I. $(shell glib-config --cflags) $(shell gtk-config --cflags)
CFLAGS += -mcpu=i686 -g -O -fstrict-aliasing -Wall  $(CPPFLAGS) # for debugging
LDFLAGS += $(shell glib-config --libs) $(shell gtk-config --libs) -lm

CPPFLAGS += -DUSE_MMX_ASM=1
asm = vlc_x86.S quant_x86.S idct_block_mmx.S

gensources=dct.c idct_248.c weighting.c quant.c vlc.c place.c parse.c bitstream.c ycrcb_to_rgb32.c
genobjects=$(gensources:.c=.o) $(asm:.S=.o)

sources = playdv.c $(gensources)
objects= playdv.o $(genobjects)
auxsources=gasmoff.c

install: all
	install -c libdv.a ../../../lib
all: $(genobjects)
	ar rv libdv.a  $(genobjects)

vlc_x86.d: asmoff.h

asmoff.h: gasmoff
	./gasmoff > asmoff.h

gasmoff: gasmoff.o bitstream.o

clean:
	rm -f libdv.a asmoff.h *.o *.depend

.depend: asmoff.h $(OBJS:.o=.cc)
	$(CC) -M $(CPPFLAGS) $(gensources) $(asm) > .depend

include .depend
