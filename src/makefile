CFLAGS = `pkg-config --cflags opencv`
FLAGS64=-I/include/XVision2 -I${PVM_ROOT}/include -I/usr/include/dc1394/ -I/usr/include -L/usr/lib
LIBS=-L/usr/X11R6/lib -L${PVM_ROOT}/lib/${PVM_ARCH} -L/lib/ -L/usr/lib/ -lXVTrack -lXVDevs -lXVCons -lXVSeg -lXVTools \
	-lXVImages -ljpeg -ltiff -L/usr/X11R6/lib64 -lXext -lX11 -lavformat -lavcodec -lavutil -lpthread -lippi -lippcc -lipps  \
	-lpvm3 -lraw1394 -ldc1394 -lmpeg /usr/lib/x86_64-linux-gnu/libXxf86dga.so.1 /usr/lib/x86_64-linux-gnu/libXxf86vm.so.1 `pkg-config --libs opencv`

all: xvisionToOpenCV.c
	${CXX} -w xvisionToOpenCV.c ${FLAGS64} ${CFLAGS} ${LIBS} -o xvisionToOpenCV

