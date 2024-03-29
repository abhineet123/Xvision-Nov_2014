# Generated automatically from Makefile.in by configure.
#
# Makefile.in for the MPEG Library -- this has to be processed by
# `configure' to be meaningful!
#

SHELL         = /bin/sh

# Compiler and compiler/preprocessor flags
CC            = gcc
OPTIMIZE      = -O2
EXTRA_CFLAGS  = 
INCLUDE_DIRS  = -I. 
DEFINES       = 
CFLAGS        = $(OPTIMIZE) $(EXTRA_CFLAGS)
CPPFLAGS      = $(INCLUDE_DIRS) $(DEFINES)

# Installation directories (and install program)
prefix          = /usr/local
exec_prefix     = ${prefix}
INSTALL_INCLUDE = ../../../include
INSTALL_LIBRARY = ../../../lib
INSTALL         = /usr/bin/install -c

# Other miscellaneous programs
AR            = ar
ARFLAGS       = -ru
RANLIB        = ranlib


# Source for the library itself -- note that we define LIBSRC and
# LIBOBJ this way (instead of just LIBOBJ=$(LIBSRC:.c=.o) because
# HP-UX make is stupid.

DECODER_SRC   = util.c video.c parseblock.c motionvector.c decoders.c \
		jrevdct.c wrapper.c globals.c 24bit.c gdith.c
DITHER_SRC    = fs2.c fs2fast.c fs4.c hybrid.c hybriderr.c 2x2.c gray.c \
		mono.c ordered.c ordered2.c mb_ordered.c
LIBSRC        = $(DECODER_SRC) $(DITHER_SRC)
LIBOBJ        = $(DECODER_SRC:.c=.o) $(DITHER_SRC:.c=.o)


# Other files of interest

LIBRARY       = libmpeg.a
SHLIB         = libmpeg.so
HEADER        = mpeg.h
#EXTRAS        = @extras@

# Implicit targets
.c.o:
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $<


# Library/cleanup targets: 

all:		lib 
	install -c libmpeg.a ../../../lib

lib:		$(LIBRARY)

shlib:          $(SHLIB)

$(LIBRARY):	$(LIBOBJ)
		$(AR) $(ARFLAGS) $(LIBRARY) $(LIBOBJ)
		$(RANLIB) $(LIBRARY)

$(SHLIB):	$(LIBOBJ)
		$(CC) -shared -o $(SHLIB) $(LIBOBJ)

install:	$(LIBRARY)
		$(INSTALL) -d $(INSTALL_INCLUDE) $(INSTALL_LIBRARY)
		$(INSTALL) -m 0644 $(HEADER) $(INSTALL_INCLUDE)/$(HEADER)
		$(INSTALL) -m 0644 $(LIBRARY) $(INSTALL_LIBRARY)/$(LIBRARY)
		@if test -f $(SHLIB) ; then echo "warning: $(SHLIB) *not* installed (installing shared libraries is too system-specific)" ; fi

clean:
		rm -f $(LIBRARY) $(EXTRAS) $(LIBOBJ) core .depend
#		cd extras && $(MAKE) clean

distclean:	clean
		rm -f config.cache config.log config.status Makefile config.h
#		cd extras && $(MAKE) distclean


# Executable targets (mpegtest, easympeg [GL platforms only])

mpegtest:	extras/mpegtest $(LIBRARY)
		rm -f mpegtest
#		ln -s extras/mpegtest .

#extras/mpegtest:
#		(cd extras ; $(MAKE) mpegtest)

#easympeg:	extras/easympeg $(LIBRARY)
#		rm -f easympeg
#		ln -s extras/easympeg .

#extras/easympeg:
#		(cd extras ; $(MAKE) easympeg)

check:		mpegtest
		./mpegtest -checksum test.mpg


# Targets to check that configuration stuff is up-to-date

configure: configure.in
	@echo "configure is out-of-date -- run autoconf"

config.status config.cache: configure
	@echo "$@ out-of-date -- run configure"

Makefile: Makefile.in config.status
	@echo "$@ out-of-date -- run configure or config.status"

#extras/Makefile: extras/Makefile.in config.status
	@echo "$@ out-of-date -- run configure or config.status"

config.h: config.h.in config.status
	@echo "$@ out-of-date -- run configure or config.status"
.depend: $(LIBOBJ:.o=.c)
	$(CC) -M $(CFLAGS) $(CPPFLAGS) $(LIBOBJ:.o=.c) > .depend
include .depend
