/*
 *	Copyright (C) 1996  Jim Bray (http://as220.org/jb). Code provided
 *      by Jason Lango and Jim Kurien at the AI Lab, and the FreeBSD driver
 *      folks.
 *
 *	This work was supported by RWI (Real World Interface) Inc.,
 *	and the AI Lab at Brown University.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Version 2:
 *  Cleaned up some cast problems. Changed all the printf()s to perror()s.
 *  Improved signal handling.
 *
 * Version 3:
 *  Renamed from view to vu to avoid name conflict.
 *
 * Version 4: (7/May/96)
 *  Substantially modified by Ian Reid (ian@robots.ox.ac.uk) to enable geometry 
 *  changes and colour display.
 *  -- Added eventLoop to process Configure events and hence change geometry of
 *     captured image [Note that if gotframe() takes too long then no X events 
 *     are ever processed since the program spends all of its time servicing SIGUSR2.
 *     Hence I have added a deliberate skip every 25 frames to ensure events get 
 *     serviced in eventLoop().
 *  -- Added #ifdef PAL bits.  
 *  -- Added automatic detection of Visual class so it will work on:
 *       8 bit PseudoColor
 *       15 bit DirectColor
 *       24/32 bit DirectColor
 *  -- Changed to straight C since the C++ bits were largely irrelevant
 *  
 * Version 4.1: (15/May/96)
 *  Changed gotframe so that it checks for X events, hence these get serviced 
 *  potentially every 33/40 ms instead of every second or so as previously.  This
 *  means that eventLoop has been replaced by doEvent.
 *
 * Version 4.2: (4/Nov/96)
 *  The Natoma chipset does't like YUV_PLANAR.  Updated to check /proc/pci
 *  for the Natoma and use YUV_PACKED if found (Tyson D. Sawyer tyson@rwii.com)
 *
 * Version 4.3: (22/Jul/98)
 * Modified to successfully compile and run on libc6 (aka. glibc2) based
 * linux systems.  Succesfully tested to compile without errors on 
 * RedHat 5.0 and 5.1.  Also compiled and run on RedHat 4.2 (an old-style
 * libc5 based Linux system) to test for backward compatablity.  It works on
 * that system too.
 * Mark Sutton <mark.sutton@laitram.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/shm.h>

/* #include <linux/mm.h>  */
/* The above include will blow up under libc6 (glibc2)  */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/XShm.h>
#include <X11/keysym.h>

#include "ioctl_meteor.h"

#if defined(PAL)
#define MAXROWS 576
#define MAXCOLS 768
#define DEFAULT_FORMAT METEOR_FMT_PAL
#define MAXFPS 25
#elif defined(SECAM)
#define MAXROWS 576
#define MAXCOLS 768
#define DEFAULT_FORMAT METEOR_FMT_SECAM
#define MAXFPS 25
#else /* NTSC */
#define MAXROWS 480
#define MAXCOLS 640
#define DEFAULT_FORMAT METEOR_FMT_NTSC
#define MAXFPS 30
#endif

#define NOFRAMES 1
#define MAXPIXELSIZE 4
#define MAXSIZE (NOFRAMES*MAXPIXELSIZE*MAXROWS*MAXCOLS)

extern int errno;

/* function prototypes */
static int setupDisplay();
static void doEvent(void);
void gotframe(int x);

/* global X objects */
static Display* display_;
static GC gc_;
static Window wtwin_;
static XImage *shmimage;
static XShmSegmentInfo* shminfo;

/* device stuff */
static int   meteor;
static char *DevName;
static char *mmbuf;
static struct meteor_geomet geo;

static struct sigaction sigact = 
((struct sigaction){ sa_handler: gotframe,
		     sa_mask: 0
/*		     sa_flags: SA_RESTART */ });

/* geometry stuff */
static int columns = MAXCOLS,
           rows = MAXROWS,
	   window_size_x,
           window_size_y,
	   pixelsize = MAXPIXELSIZE,
           noframes = NOFRAMES,
	   bufsize = MAXPIXELSIZE,
           fieldmode = 0x0,
           sigmode = 0x0,
	   fps = MAXFPS;
struct meteor_frame_offset off;

static int usePacked;

void gotframe(int signum)
{
    static long i=0;
    struct meteor_mem *mm = (struct meteor_mem *)(mmbuf + off.mem_off);
    unsigned char *src;
    unsigned char *dest;
    unsigned int count;

    if (XEventsQueued(display_, QueuedAfterReading) != 0)
	doEvent();

    src = (noframes>1) ? mmbuf+off.frame_offset[mm->cur_frame-1] : mmbuf + off.frame_offset[0];
    dest = shmimage->data;
    count = rows*columns*pixelsize;

    if (usePacked) {
	for (src++; count--; src+=2)
	    *dest++ = *src;
    } else {
	memcpy(dest, src, count);
    }

    /* Draw screen onto display */
    XShmPutImage(display_, wtwin_, gc_, shmimage,
		 0, 0, 0, 0, window_size_x, window_size_y, False);
    XSync(display_, 0);
}


int checkChips(void) 
{
    FILE *file;
    char buffer[256];
    int count;

    file = fopen("/proc/pci", "r");
    if (file==NULL) {
	fprintf(stderr, "failed to open /proc/pci.  "
		"Assuming this is not Natoma.\n");
	return(0);
    }

    while ((count=fscanf(file, "%255s\n", buffer))>0) {
	if (!strncasecmp("Natoma", buffer, 255)) {
	    fclose(file);
	    fprintf(stderr, "Looks like a Natoma chipset.  Using YUV_PACKED.\n");
	    return(1);
	}
    }
    fprintf(stderr, "Doesn't look like a Natoma chipset.  Using YUV_PLANAR.\n");
    fclose(file);
    return(0);
}


int main(int argc, char **argv)
{

    static char DefaultName[]="/dev/mmetfgrab0";
    u_long  size;
    u_short s;
    int c, i, inputdev = METEOR_INPUT_DEV_RCA;

    DevName = DefaultName;
    
    for (i=1; i<argc; i++) {
	if (argv[i][0]=='-') {
	    switch (argv[i][1]) {
	    case 's':
		columns = atoi(argv[++i]) & 0x3fc;
		rows = atoi(argv[++i]) & 0x7fe;
		break;
	    case 'f':
		fieldmode = METEOR_FIELD_MODE;
		sigmode = METEOR_SIG_FIELD;
		noframes = 2;
		break;
	    case 'i':
		i++;
		if (strcmp(argv[i],"RGB")==0)
		    inputdev = METEOR_INPUT_DEV_RGB;
		else if (strcmp(argv[i],"RCA")==0)
		    inputdev = METEOR_INPUT_DEV_RCA;
		else if (strcmp(argv[i],"SVIDEO")==0)
		    inputdev = METEOR_INPUT_DEV_SVIDEO;
		else if (strcmp(argv[i],"DEV0")==0)
		    inputdev = METEOR_INPUT_DEV0;
		else if (strcmp(argv[i],"DEV1")==0)
		    inputdev = METEOR_INPUT_DEV1;
		else if (strcmp(argv[i],"DEV2")==0)
		    inputdev = METEOR_INPUT_DEV2;
		else if (strcmp(argv[i],"DEV3")==0)
		    inputdev = METEOR_INPUT_DEV3;
		else {
		    fprintf(stderr, "Invalid source.  Choose from RCA, RGB, SVIDEO\n");
		    exit(1);
		}
		break;
	    case 'd':
		DevName = argv[++i];
		break;
	    case 'p':
		usePacked = 1;
		fprintf(stderr, "YUV_PACKED forced by -p option.\n");
		break;
	    }
	} else {
	    fprintf(stderr, "Usage: vu4 [-p] [-f] [-i InputDev] [-s Ncols Nrows] [-d DevName]\n");
	    exit(1);
	}
    }

    fprintf(stderr, "%d x %d\n", columns, rows);

    window_size_x = columns;
    window_size_y = rows;
    if (fieldmode==METEOR_FIELD_MODE) {
	rows /= 2;
	window_size_y = rows;
    }

    if ((meteor = open(DevName, O_RDWR)) < 0) {
	perror("open failed");
	printf("device %s\n", DevName);
	exit(1);
    }

    switch (setupDisplay()) {
    case 24:
    case 32:
        printf("Grabbing in 24 bit.\n");
	pixelsize = 4;
	bufsize = 4;
	geo.oformat = METEOR_GEO_RGB24;
	break;
    case 16:
    case 15:
        printf("Grabbing in 16 bit.\n");
	pixelsize = 2;
	bufsize = 2;
	geo.oformat = METEOR_GEO_RGB16;
	break;
    default:
        printf("Grabbing in monochrome \n");
	pixelsize = 1;
	bufsize = 2;
	if ((usePacked = checkChips())) {
	    geo.oformat = METEOR_GEO_YUV_PACKED;
	    fprintf(stderr, "Format set to YUV_PACKED\n");
	} else {
	    geo.oformat = METEOR_GEO_YUV_PLANAR;
	    fprintf(stderr, "Format set to YUV_PLANAR\n");
	}
	break;
    }

    geo.rows = rows;
    geo.columns = columns;
    geo.frames = noframes;
    geo.oformat |= fieldmode;

    if (ioctl(meteor, METEORSETGEO, &geo) < 0) {
	perror("ioctl SetGeometry failed");
	exit(1);
    }

    if (ioctl(meteor, METEORGFROFF, &off) < 0) {
	perror("ioctl FrameOffset failed");
	exit(1);
    }

    fprintf(stderr, "size: 0x%lx\n", off.fb_size);
    fprintf(stderr, "mem: 0x%lx\n", off.mem_off);
    for (i=0; i<noframes; i++)
	fprintf(stderr, "frame %d: 0x%lx\n", i, off.frame_offset[i]);

    if ((mmbuf=(char *) mmap((caddr_t)0, off.fb_size,
			     PROT_READ|PROT_WRITE, 
			     MAP_FILE|MAP_SHARED, 
			     meteor, (off_t)0)) == (char *)(-1)) {
	perror("mmap failed");
	exit(1);
    }

    c = DEFAULT_FORMAT;
    if (ioctl(meteor, METEORSFMT, &c) < 0) {
	perror("ioctl SetFormat failed");
	exit(1);
    }

    if (ioctl(meteor, METEORSINPUT, &inputdev) < 0) {
	perror("ioctl Setinput failed");
	exit(1);
    }

    if (inputdev==METEOR_INPUT_DEV_RGB && geo.oformat==METEOR_GEO_RGB16) {
	/* set RGB section for 15 bit colour */
	u_short s = (0x4 << 4);
	if (ioctl(meteor, METEORSBT254, &s)) {
	    perror("ioctl SetBt254 failed");
	    exit(1);
	}
    }

    if ( sigaction(SIGUSR2, &sigact, NULL) ) {
	perror("sigaction failed");
	exit(1);
    }

    c = SIGUSR2 | sigmode;
    if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
	perror("ioctl SetSignal failed");
	exit(1);
    }


    if (noframes < 2) {
	/* continuous frame capture */
	c = METEOR_CAP_CONTINOUS ;
	if (ioctl(meteor, METEORCAPTUR, &c)) {
	    perror("ioctl CaptContinuous failed");
	    exit(1);
	}
    } else {
	struct meteor_capframe frame;

	frame.command = METEOR_CAP_N_FRAMES;
	frame.lowat = 0;
	frame.hiwat = 0;
	if (ioctl(meteor, METEORCAPFRM, &frame)) {
	    perror("ioctl CaptFrm failed");
	    exit(1);
	}
    }

    /* process events */
    for (;;) {
	/* NOTE:  I hope that X events are reentrant since this call and the one
	   in gotframe could conceivably happen at the "same" time.  Hasn't caused
	   any probs so far, but let me know if it does ian@robots.ox.ac.uk */
	doEvent();
	XFlush(display_);
	/*pause();*/
    }
}


static int setupDisplay() 
{
    XGCValues gc_values;
    XSizeHints sizehints;
    XWindowAttributes win_attributes;
    XVisualInfo desired_visual_template;
    XVisualInfo *visual_info_list; 
    int no_visual_matched;
    XColor *palette;
    Colormap wtcolormap;
    int pixelNum, numCols;
    int depth;
    int screen;

    display_ = XOpenDisplay(NULL);
    wtwin_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
				 0, 0, window_size_x, window_size_y, 2,
				 BlackPixel(display_, DefaultScreen(display_)),
				 BlackPixel(display_, DefaultScreen(display_))); 

    sizehints.flags = PMaxSize|PAspect|PResizeInc;
    sizehints.max_width = MAXCOLS;
    sizehints.max_height = MAXROWS;
    sizehints.min_aspect.x = 1;
    sizehints.min_aspect.y = 2;
    sizehints.max_aspect.x = (fieldmode) ? 4 : 2;
    sizehints.max_aspect.y = 1;
    sizehints.width_inc = 8;
    sizehints.height_inc = 2;
    XSetNormalHints(display_, wtwin_, &sizehints);

    XMapWindow(display_, wtwin_);
    XSelectInput(display_, wtwin_, ButtonPress|StructureNotifyMask);

    gc_values.graphics_exposures = False;
    gc_ = XCreateGC(display_, wtwin_, GCGraphicsExposures, &gc_values);  

    screen = DefaultScreen(display_);
    XGetWindowAttributes(display_, wtwin_, &win_attributes);
    depth = win_attributes.depth;
  
    /* SHARED MEMORY PORTION */
  
    shminfo = (XShmSegmentInfo*) malloc (sizeof(XShmSegmentInfo));
    shmimage = XShmCreateImage(display_, DefaultVisual(display_, screen),
			       depth, ZPixmap, NULL, shminfo, columns, rows);
    fprintf(stderr, "depth=%d\n", depth);
    fprintf(stderr, "size=%lx\n", shmimage->bytes_per_line * shmimage->height);
    if ((shminfo->shmid = shmget(IPC_PRIVATE,  /*ftok(DevName, 'v'),*/
				 shmimage->bytes_per_line * shmimage->height,
				 IPC_CREAT | 0777)) == -1) {
	perror("Shared memory error (shmget)");
	exit(1);
    }
    if ((shminfo->shmaddr = (char *) shmat(shminfo->shmid, (void *)0, 0)) == (char *)(-1)) {
	perror("Shared memory error (shmat)");
	exit(1);
    }
    shmimage->data = shminfo->shmaddr;
  
    XShmAttach(display_, shminfo);

    /* END SHARED MEMORY PORTION */

    desired_visual_template.screen = screen;
    desired_visual_template.class = DirectColor;
    visual_info_list = XGetVisualInfo(display_, 
				      VisualScreenMask,
				      &desired_visual_template,
				      &no_visual_matched);
    if (no_visual_matched > 0 && depth > 8) {
	/* found a DirectColor visual */
	fprintf(stderr, "Using DirectColor\n");
	return depth;
    }

    desired_visual_template.class = PseudoColor;
    visual_info_list = XGetVisualInfo(display_, 
				      VisualScreenMask,
				      &desired_visual_template,
				      &no_visual_matched);

    if (no_visual_matched == 0) {
	fprintf(stderr, "No visual matched\n");
	exit(1);
    }

    /* make colour map */

    wtcolormap = XCreateColormap(display_, DefaultRootWindow(display_),
				 visual_info_list[0].visual, AllocAll);

    numCols = 1<<depth;
    palette = (XColor *)malloc(numCols * sizeof(XColor));
    for (pixelNum=0; pixelNum < numCols; pixelNum++) {
	palette[pixelNum].pixel=pixelNum;
	palette[pixelNum].red=pixelNum<<(16-depth);
	palette[pixelNum].green=pixelNum<<(16-depth);
	palette[pixelNum].blue=pixelNum<<(16-depth);
	palette[pixelNum].flags=DoRed|DoGreen|DoBlue;
    }

    XStoreColors(display_, wtcolormap, palette, numCols);
    XSetWindowColormap(display_, wtwin_, wtcolormap);

    fprintf(stderr, "Using PseudoColor\n");
    return depth;
}


static void doEvent(void)
{
    XEvent event;

    while (XEventsQueued(display_, QueuedAfterReading) != 0) {
	XNextEvent(display_, &event);

	switch (event.type) {

	case ConfigureNotify:

	    /* resize the image */
	    { 
		u_long size;
		int c;

		/* first disable application signals */
		c = 0;
		if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
		    perror("ioctl SetSignal failed");
		    exit(1);
		}

		/* now stop continuous capture */
		if (noframes < 2) {
		    c = METEOR_CAP_STOP_CONT;
		    if (ioctl(meteor, METEORCAPTUR, &c)) {
			perror("ioctl CaptContinuous failed");
			exit(1);
		    }
		} else {
		    struct meteor_capframe frame;

		    frame.command = METEOR_CAP_STOP_FRAMES;
		    if (ioctl(meteor, METEORCAPFRM, &frame)) {
			perror("ioctl CaptFrm failed");
			exit(1);
		    }
		}


		usleep(80000);  /* wait for 80ms to make sure no more frames are arriving */

		/* unmap memeory */
		munmap(mmbuf, noframes*bufsize*rows*columns);

		/* reset the geometry */
		geo.rows = rows = window_size_y = event.xconfigure.height;
		geo.columns = columns = window_size_x = event.xconfigure.width;
		if (ioctl(meteor, METEORSETGEO, &geo) < 0) {
		    perror("ioctl SetGeometry failed");
		    exit(1);
		}

		if (ioctl(meteor, METEORGFROFF, &off) < 0) {
		    perror("ioctl FrameOffset failed");
		    exit(1);
		}

		fprintf(stderr, "New geometry set to %d x %d\n", geo.columns, geo.rows);

		/* remap memory */
		if ((mmbuf=(char *) mmap((caddr_t)0, off.fb_size,
					 PROT_READ|PROT_WRITE, 
					 MAP_FILE|MAP_PRIVATE, 
					 meteor, (off_t)0)) == (char *)(-1)) {
		    perror("mmap failed");
		    exit(1);
		}

		/* need to reallocate shmimage */
	        { 
		    int screen = DefaultScreen(display_);
		    int depth = DefaultDepth(display_, screen);

		    free(shmimage);
		    shmimage = XShmCreateImage(display_, DefaultVisual(display_, screen),
					       depth, ZPixmap, NULL, shminfo, columns, rows);
		    shmimage->data = shminfo->shmaddr;

		}

		/* re-enable application signals */
		c = SIGUSR2 | sigmode;
		if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
		    perror("ioctl SetSignal failed");
		    exit(1);
		}

		if (noframes < 2) {
		    /* continuous frame capture */
		    c = METEOR_CAP_CONTINOUS ;
		    if (ioctl(meteor, METEORCAPTUR, &c)) {
			perror("ioctl CaptContinuous failed");
			exit(1);
		    }
		} else {
		    struct meteor_capframe frame;

		    frame.command = METEOR_CAP_N_FRAMES;
		    frame.lowat = 0;
		    frame.hiwat = 0;
		    if (ioctl(meteor, METEORCAPFRM, &frame)) {
			perror("ioctl CaptFrm failed");
			exit(1);
		    }
		}
	    }
	    break;

	case ButtonPress:
	    if (event.xbutton.button==1) {
		/* LEFT: get error counts */

		struct meteor_counts cnt;

		if (ioctl(meteor, METEORGCOUNT, &cnt)) {
		    perror("ioctl GetCount failed");
		    exit(1);
		}
		fprintf(stderr, "Frames: %d\nEven:   %d\nOdd:    %d\n", 
			cnt.frames_captured,
			cnt.even_fields_captured,
			cnt.odd_fields_captured);
		fprintf(stderr, "Even: %d\nOdd:  %d\n", 
			cnt.even_fields_captured,
			cnt.odd_fields_captured);
		fprintf(stderr, "Fifo errors: %d\n", cnt.fifo_errors);
		fprintf(stderr, "DMA errors:  %d\n", cnt.dma_errors);

	    } else if (event.xbutton.button==2) {
		/* MIDDLE: see what's in there in case driver stops signalling */

		gotframe(12);

	    } else if (event.xbutton.button==3) {
		/* RIGHT: check the capture control register */

		u_long cap;

		if (ioctl(meteor, METEORGCAPT, &cap)) {
		    perror("ioctl GetCapt failed");
		    exit(1);
		}
		fprintf(stderr, "Capture control: 0x%lx\n", cap);

	    }
 	    break;

	default:
	    /* do nothing */
	    break;
	}
    }

    return;
}
