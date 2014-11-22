#include <sys/types.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include "ioctl_meteor.h"

extern int errno;
#define ROWS 480
#define COLS 640
#define SIZE (ROWS * COLS * 2)
main()
{
	struct meteor_geomet geo;
	char buf[SIZE];
	char *mmbuf;
	int i,c;

	if ((i = open("/dev/mmetfgrab0", O_RDONLY)) < 0) {
		printf("open failed\n");
		exit(1);
	}

        geo.rows = ROWS;
        geo.columns = COLS;
        geo.frames = 1;
        geo.oformat = METEOR_GEO_RGB16 ;

        if (ioctl(i, METEORSETGEO, &geo) < 0) {
		printf("ioctl failed: %d\n", errno);
		exit(1);
	}

	c = METEOR_FMT_NTSC;

        if (ioctl(i, METEORSFMT, &c) < 0) {
		printf("ioctl failed: %d\n", errno);
		exit(1);
	}

	c = METEOR_INPUT_DEV0;

        if (ioctl(i, METEORSINPUT, &c) < 0) {
		printf("ioctl failed: %d\n", errno);
		exit(1);
	}

        mmbuf=(char *)mmap((caddr_t)0, SIZE, PROT_READ,
			   MAP_FILE|MAP_SHARED, i, (off_t)0);

#ifdef SINGLE_MODE
			/* single frame capture */
	c = METEOR_CAP_SINGLE ;
        ioctl(i, METEORCAPTUR, &c);	/* wait for the frame */
  
	/* directly access the frame buffer array data in mmbuf */
#else
			/* continuous frame capture */
	c = METEOR_CAP_CONTINOUS ;
        ioctl(i, METEORCAPTUR, &c);	/* returns immediately */
  
	/* directly access the frame buffer array data in mmbuf */

	c = METEOR_CAP_STOP_CONT ;
        ioctl(i, METEORCAPTUR, &c);	/* close will also stop capture */
#endif

	close(i);
	exit(0);
}
