
CUR_DEPTH	= ../../
CXXFLAGS= -g -O2 -DDEBUG_LEVEL=1 -DHAVE_CONFIG_H
CPPFLAGS	= -O6 -I$(CUR_DEPTH)/include $(JPEGDEF) $(TIFFDEF)

### comment/uncomment these lines to include/exclude image support
JPEGDEF         = -DJPEG_LIB
TIFFDEF         = -DTIFF_LIB
MPEGDEF         = mpeg
###

ifdef JPEGDEF
ifdef TIFFDEF
all: jpeg tiff $(MPEGDEF)
	ar rv libjpeg.a $(JPEGLIB) $(OBJS)
	ar rv libtiff.a $(TIFFLIB) $(OBJS)
else
all: jpeg $(MPEGDEF)
	ar rv libjpeg.a $(JPEGLIB)
endif
else
ifdef TIFFDEF
all: tiff $(MPEGDEF)
	ar rv libtiff.a $(TIFFLIB)
endif
endif

clean: ;	rm -f *.o
	rm -f libjpeg.a libtiff.a .depend
	(cd mpeg_lib-1.3.1; make clean)

install: all
	install -c libjpeg.a $(CUR_DEPTH)/lib
	install -c libtiff.a $(CUR_DEPTH)/lib
	(cd mpeg_lib-1.3.1; make install)

mpeg: ;
	(cd mpeg_lib-1.3.1; make);


JPEG_DIR        = $(CUR_DEPTH)/src/drivers/jpeg-6b/
TIFF_DIR        = $(CUR_DEPTH)/src/drivers/tiff-v3.5.5/

ifdef JPEGDEF
JPEGLIB        = $(JPEG_DIR)/*.o

jpeg: ;
	(cd $(JPEG_DIR); make);
else
jpeg: ;
endif

ifdef TIFFDEF
TIFFLIB        = $(TIFF_DIR)/*.o

tiff: ;
	(cd $(TIFF_DIR); make);
else
tiff: ;
endif
