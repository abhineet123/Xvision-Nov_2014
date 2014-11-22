SYSTEM = -DPAL
#COLOR = -DGREYSCALE
DEFINES = $(SYSTEM) $(COLOR)

CFLAGS = -I../.. $(DEFINES) -g -O2

all: vu4-color

vu4-color.o: vu4-color.c 
	gcc $(CFLAGS) -c vu4-color.c

colorDisplay.o: colorDisplay.c 
	gcc $(CFLAGS) -O2 -c colorDisplay.c

translate.o: translate.c 
	gcc $(CFLAGS) -O2 -c translate.c

vu4-color: vu4-color.o colorDisplay.o translate.o
	gcc -g -O2 vu4-color.o colorDisplay.o translate.o \
	-o vu4-color -L/usr/X11/lib -lXext -lX11 -lm


