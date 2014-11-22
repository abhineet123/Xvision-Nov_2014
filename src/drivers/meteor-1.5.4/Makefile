
#Comment out if you are not running SMP. Someone take this out of here
#when the SMP stuff gets moved out of the kernel Makefile.

#SMP = 1
#SMP_PROF = 1

# Set this to NTSC | PAL | SECAM
SYSTEM = NTSC

ifeq ($(LINUXVERSION),)
 LINUXVERSION	= $(shell uname -r)
endif

ifeq ($(TOPDIR),)
 TOPDIR		= /usr/src/linux-$(LINUXVERSION)
endif


HPATH   	= $(TOPDIR)/include

CC		= gcc

OPT		= -O3 -fomit-frame-pointer -D$(SYSTEM)


MODFLAGS = -DMODULE
# MODFLAGS += -DMODVERSIONS -include $(HPATH)/linux/modversions.h
WARNFLAGS = -Wall -Wstrict-prototypes 
CFLAGS =  $(WARNFLAGS) $(OPT) -D__KERNEL__ -I$(HPATH) $(MODFLAGS)
MODLIB=/lib/modules/$(LINUXVERSION)

ifdef SMP
CFLAGS += -D__SMP__

ifdef SMP_PROF
CFLAGS += -D__SMP_PROF__
endif
endif

.PHONY : all clean

all: meteor.o

meteor.o: meteor.c meteor.h saa71x6.h ioctl_meteor.h k_compat.h kernelversioninfo.h

kernelversioninfo.h:
	@./Configure

install: meteor.o
	mkdir -p $(MODLIB)/misc
	cp -p meteor.o $(MODLIB)/misc
#	/sbin/depmod -a
	sync
	/sbin/modprobe -r meteor

clean:
	rm -f *.o






