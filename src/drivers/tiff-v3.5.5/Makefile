# Tag Image File Format Library
#
# Copyright (c) 1988-1997 Sam Leffler
# Copyright (c) 1991-1997 Silicon Graphics, Inc.

CUR_DEPTH = ../../../
CFLAGS = 
SRCDIR = .
OPTIMIZER=-O
#
OBJS	= \
	fax3sm_winnt.o \
	tif_fax3.o \
	tif_aux.o \
	tif_close.o \
	tif_codec.o \
	tif_compress.o \
	tif_dir.o \
	tif_dirinfo.o \
	tif_dirread.o \
	tif_dirwrite.o \
	tif_dumpmode.o \
	tif_error.o \
	tif_getimage.o \
	tif_jpeg.o \
	tif_flush.o \
	tif_luv.o \
	tif_lzw.o \
	tif_next.o \
	tif_open.o \
	tif_packbits.o \
	tif_pixarlog.o \
	tif_predict.o \
	tif_print.o \
	tif_read.o \
	tif_swab.o \
	tif_strip.o \
	tif_thunder.o \
	tif_tile.o \
	tif_unix.o \
	tif_warning.o \
	tif_write.o \
	tif_zip.o \
	${NULL}

all:	.depend $(OBJS)
	

.depend:  $(OBJS:.o=.c)
	${CC} -M $(OBJS:.o=.c) > .depend

include .depend


install: all

clean:
	rm -f *.o
	rm -f  .depend
