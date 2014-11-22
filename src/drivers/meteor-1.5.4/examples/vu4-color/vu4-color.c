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
 * Version 5: (22/Jul/98)
 *  Modified to compile on libc6 (glibc2) based systems.
 *  Mark Sutton <mark.sutton@laitram.com>
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
/* #include <sys/shm.h>*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/XShm.h>
#include <X11/keysym.h>

#include "ioctl_meteor.h"
#include "colorDisplay.h"
#include "translate.h"

#define MAXPIXELSIZE 4
#define MAXSIZE (MAXPIXELSIZE*MAXROWS*MAXCOLS)

extern int errno;

/* function prototypes */
static int setupDisplay();
static void eventLoop(void);
void gotframe(int x);

/* global X objects */
Display* display_;
GC gc_;
Window wtwin_;
extern struct timeval tp1;

/* device stuff */
static int   meteor;
static char *DevName;
static char *mmbuf;
static char *data;
static struct meteor_geomet geo;

static struct sigaction sigact = 
((struct sigaction){ sa_handler: gotframe,
		     sa_mask: 0
/*		     sa_flags: SA_RESTART */ });

/* geometry stuff */
static int columns = MAXCOLS,
           rows = MAXROWS,
	   window_size_x = MAXCOLS,
           window_size_y = MAXROWS,
	   pixelsize = MAXPIXELSIZE,
	   bufsize = MAXPIXELSIZE*MAXROWS*MAXCOLS,
	   fps = MAXFPS;

static XColor color_tab[125];
static int dither16x16[16][16];
static int divN[256], modN[256];
static int cdither = 0;

int Meteor_End(void)
{
  close(meteor);
  fprintf(stderr, "\nMeteor Closed successfully...\n");
  fflush(stdout);
  fflush(stderr);

  return 1;
}

static void Meteor_Abort(int sig) {
  fprintf(stderr, "\ncaught interrupt - Cleaning up...\n");
  fflush(stdout);
  fflush(stderr);
  Meteor_End();
  exit(0);
}

unsigned char R[MAXROWS * MAXCOLS];
unsigned char G[MAXROWS * MAXCOLS];
unsigned char B[MAXROWS * MAXCOLS];

void gotframe(int signum)
{
    int            i,j;
    int            color;
    int            index;
    static XImage        *ximage;
    char *tmp, *tdata;
    register int  cd2=cdither*cdither;
    unsigned char *rtmp, *gtmp, *btmp;
    
    tmp = mmbuf; rtmp = R; gtmp = G; btmp = B;
    for (i=0; i< rows; i++) 
      for (j=0; j< columns; j++) {
	 *btmp++ = *tmp++;
	 *gtmp++ = *tmp++;
	 *rtmp++ = *tmp++;
	 ++tmp;
      }
    display_image (columns, rows, R, G, B);

}


int main(int argc, char **argv)
{

    static char DefaultName[]="/dev/mmetfgrab0";
    int c;

    if (--argc > 0 )
	DevName = *++argv;
    else
	DevName = DefaultName;

    if ((meteor = open(DevName, O_RDONLY)) < 0) {
	perror("open failed");
	printf("device %s\n", DevName);
	exit(1);
    }

    signal(SIGTERM, &Meteor_Abort);
    signal(SIGINT,  &Meteor_Abort);

    switch (setupDisplay()) {
    case 24:
    case 32:
	pixelsize = 4;
	bufsize = 4*rows*columns;
	geo.oformat = METEOR_GEO_RGB24;
	break;
    case 15:
	pixelsize = 2;
	bufsize = 2*rows*columns;
	geo.oformat = METEOR_GEO_RGB16;
	break;
    default:
	pixelsize = 1;
	bufsize = 2*rows*columns;
	geo.oformat = METEOR_GEO_YUV_PLANER;
	break;
    }
    
    /* set up dithering tables */
    init_colormaps (display_);

    printf ("(%d %d)\n", rows, columns);
    geo.rows = rows;
    geo.columns = columns;
    geo.frames = 1;
    /* geo.oformat = METEOR_GEO_RGB24 | METEOR_GEO_ODD_ONLY;*/
    geo.oformat = METEOR_GEO_RGB24;

    if (ioctl(meteor, METEORSETGEO, &geo) < 0) {
	perror("ioctl SetGeometry failed");
	exit(1);
    }

    bufsize = 4*rows*columns;
    mmbuf=(char *) mmap ((caddr_t)0, bufsize,
			 PROT_READ,MAP_FILE|MAP_PRIVATE,
			 meteor, (off_t)0);
    if ( mmbuf==(char *)(-1) ) {
	perror("mmap failed");
	exit(1);
    }
    if ((data = (char *) malloc (rows*columns * sizeof (char *))) == NULL) {
       perror ("malloc for data failed");
       exit (1);
    }
    
    c = DEFAULT_FORMAT;
    if (ioctl(meteor, METEORSFMT, &c) < 0) {
	perror("ioctl SetFormat failed");
	exit(1);
    }

    c = METEOR_INPUT_DEV0;
    if (ioctl(meteor, METEORSINPUT, &c) < 0) {
	perror("ioctl Setinput failed");
	exit(1);
    }

    if ( sigaction(SIGUSR2, &sigact, NULL) ) {
	perror("sigaction failed");
	exit(1);
    }

    c = SIGUSR2;
    if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
	perror("ioctl SetSignal failed");
	exit(1);
    }

    /* continuous frame capture */
    c = METEOR_CAP_CONTINOUS ;
    if (ioctl(meteor, METEORCAPTUR, &c)) {
	perror("ioctl CaptContinuous failed");
	exit(1);
    }

    gettimeofday(&tp1, NULL);
    
    eventLoop();


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
    sizehints.max_aspect.x = 2;
    sizehints.max_aspect.y = 1;
    sizehints.width_inc = 2;
    sizehints.height_inc = 2;
    XSetNormalHints(display_, wtwin_, &sizehints);

    XMapWindow(display_, wtwin_);

    gc_values.graphics_exposures = False;
    gc_ = XCreateGC(display_, wtwin_, GCGraphicsExposures, &gc_values);  

    screen = DefaultScreen(display_);
    XGetWindowAttributes(display_, wtwin_, &win_attributes);
    depth = win_attributes.depth;
  
    /* SHARED MEMORY PORTION */
  
    /*
    shminfo = (XShmSegmentInfo*) malloc (sizeof(XShmSegmentInfo));
    shmimage = XShmCreateImage(display_, DefaultVisual(display_, screen),
			       depth, ZPixmap, NULL, shminfo, columns, rows);
    if ((shminfo->shmid = shmget(ftok(DevName, 'v'),
				 shmimage->bytes_per_line * shmimage->height,
				 IPC_CREAT | 0777)) == -1) {
	perror("Shared memory error (shmget)");
	exit(1);
    }
    if ((shminfo->shmaddr = (char *) shmat(shminfo->shmid, (void *) 0, 0)) == (char *)(-1)) {
	perror("Shared memory error (shmat)");
	exit(1);
    }
    shmimage->data = shminfo->shmaddr;
  
    XShmAttach(display_, shminfo);
    */
    
    /* END SHARED MEMORY PORTION */

    desired_visual_template.screen = screen;
    desired_visual_template.class = DirectColor;
    visual_info_list = XGetVisualInfo(display_, 
				      VisualScreenMask,
				      &desired_visual_template,
				      &no_visual_matched);
    if (no_visual_matched > 0)
	/* found a DirectColor visual */
	return depth;

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

    /*
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
    */
    
    return depth;
}


static void eventLoop(void)
{
    XEvent event;

    XSelectInput(display_, wtwin_, KeyPressMask|KeyReleaseMask|StructureNotifyMask);

    for (;;) {
	while (XEventsQueued(display_, QueuedAfterReading) != 0) {
	    XNextEvent(display_, &event);
	    switch (event.type) {

	    case ConfigureNotify:
		/* resize the image */
	        { 
		    int c;

		    /* first disable application signals */
		    c = 0;
		    if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
			perror("ioctl SetSignal failed");
			exit(1);
		    }

		    /* now stop continuous capture */
		    c = METEOR_CAP_STOP_CONT;
		    if (ioctl(meteor, METEORCAPTUR, &c)) {
			perror("ioctl CaptContinuous failed");
			exit(1);
		    }

		    /* reset the geometry */
		    geo.rows = rows = window_size_y = event.xconfigure.height;
		    geo.columns = columns = window_size_x = event.xconfigure.width;
		    geo.frames = 1;
		    if (ioctl(meteor, METEORSETGEO, &geo) < 0) {
			perror("ioctl SetGeometry failed");
			exit(1);
		    }

		    /* need to reallocate shmimage */
		    { 
			int screen = DefaultScreen(display_);
			int depth = DefaultDepth(display_, screen);

			/*
			free(shmimage);
			shmimage = XShmCreateImage(display_, DefaultVisual(display_, screen),
					       depth, ZPixmap, NULL, shminfo, columns, rows);
			shmimage->data = shminfo->shmaddr;
			*/
		    }

		    /* re-enable application signals */
		    c = SIGUSR2;
		    if (ioctl(meteor, METEORSSIGNAL, &c) < 0) {
			perror("ioctl SetSignal failed");
			exit(1);
		    }

		    /* start continuous capture again */
		    c = METEOR_CAP_CONTINOUS;
		    if (ioctl(meteor, METEORCAPTUR, &c)) {
			perror("ioctl CaptContinuous failed");
			exit(1);
		    }
		}
		break;

	    default:
		/* do nothing */
		break;
	    }
	}
	XFlush(display_);
	pause();
    }
 }
