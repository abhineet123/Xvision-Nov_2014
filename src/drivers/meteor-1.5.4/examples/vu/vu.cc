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
 * Version 3:
 *  Renamed from view to vu to avoid name conflict.
 * Version 4: (22/Jul/98)
 *  Modified to successfully compile on libc6 (glibc2) based systems.
 *  Mark Sutton <mark.sutton@laitram.com>
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include "ioctl_meteor.h"

extern int errno;

static void setupDisplay();

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <X11/keysym.h>

#define ROWS 480
#define COLS 640

#define SIZE (ROWS * COLS)

#define WINDOW_SIZE_X COLS
#define WINDOW_SIZE_Y ROWS

static Display* display_;
static GC gc_;
static Window wtwin_;

static XImage *shmimage;
static XShmSegmentInfo* shminfo;

static char colvalues[256][3];
static XColor palette[256];
static int maxcols = 256;
static char *DevName;
static char *mmbuf, *fbuf;
void gotframe(int x);

static struct sigaction sigact = 
((struct sigaction){ sa_handler: gotframe,
		     sa_mask: 0,
		     sa_flags: SA_RESTART });

void gotframe(int x)
{
  /* directly access the frame buffer array data in mmbuf */

  memcpy(shmimage->data, fbuf, SIZE);
  // Draw screen onto display
  XShmPutImage(display_, wtwin_, gc_, shmimage,
	       0, 0, 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y, False);
  XSync(display_, 0);
}

void main(int argc, char **argv)
{

  struct meteor_geomet geo;
  struct meteor_frame_offset off;
  static char DefaultName[]="/dev/mmetfgrab0";
  int i,c;

  
  if ( argc > 1 )
    DevName = *++argv;
  else
    DevName = DefaultName;

  if ((i = open(DevName, O_RDONLY)) < 0) {
    perror("open failed");
    printf("device %s\n", DevName);
    exit(1);
  }

  setupDisplay();

  geo.rows = ROWS;
  geo.columns = COLS;
  geo.frames = 1;
  geo.oformat = METEOR_GEO_YUV_PLANAR;

  if (ioctl(i, METEORSETGEO, &geo) < 0) {
    perror("ioctl SetGeometry failed");
    exit(1);
  }

  if (ioctl(i, METEORGFROFF, &off) < 0) {
    perror("ioctl GetFrameOffset failed");
    exit(1);
  }

  c = METEOR_FMT_PAL;

  if (ioctl(i, METEORSFMT, &c) < 0) {
    perror("ioctl SetFormat failed");
    exit(1);
  }

  c = METEOR_INPUT_DEV0;

  if (ioctl(i, METEORSINPUT, &c) < 0) {
    perror("ioctl Setinput failed");
    exit(1);
  }

  mmbuf=(char *)mmap((caddr_t)0, SIZE, PROT_READ,MAP_FILE|MAP_SHARED, i, (off_t)0);
  if ( !mmbuf ) {
    perror("mmap failed");
    exit(1);
  }
  fbuf = mmbuf + off.frame_offset[0];

  if ( sigaction(SIGUSR2, &sigact, NULL) ) {
    perror("sigaction failed");
    exit(1);
  }

  c = SIGUSR2;
  if (ioctl(i, METEORSSIGNAL, &c) < 0) {
    perror("ioctl SetSignal failed");
    exit(1);
  }
  
  /* continuous frame capture */
  c = METEOR_CAP_CONTINOUS ;
  if (ioctl(i, METEORCAPTUR, &c)) {
    perror("ioctl CaptContinuous failed");
    exit(1);
  }
  while(1)
    pause();
}

static void setupDisplay() {
  XGCValues gc_values;
  display_ = XOpenDisplay(NULL);
  wtwin_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
			       0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y, 2,
			       BlackPixel(display_, DefaultScreen(display_)),
			       BlackPixel(display_, DefaultScreen(display_)));  
  XMapWindow(display_, wtwin_);
  XSelectInput(display_, wtwin_,KeyPressMask | KeyReleaseMask);
  gc_values.graphics_exposures = False;
  gc_ = XCreateGC(display_, wtwin_, GCGraphicsExposures, &gc_values);  

  //SHARED MEMORY PORTION
  
  int depth;
  int screen;
  XWindowAttributes win_attributes;

  screen = DefaultScreen(display_);
  XGetWindowAttributes(display_, wtwin_, &win_attributes);
  depth = win_attributes.depth;
  
  shminfo = (XShmSegmentInfo*) malloc (sizeof(XShmSegmentInfo));
  shmimage = XShmCreateImage(display_, DefaultVisual(display_, screen),
			     depth, ZPixmap, NULL, shminfo, WINDOW_SIZE_X,
			     WINDOW_SIZE_Y);
  shminfo->shmid = shmget(ftok(DevName, 'v'),
			  shmimage->bytes_per_line * shmimage->height,
			  IPC_CREAT | 0777);
  shminfo->shmaddr = (char *) shmat(shminfo->shmid, (void *) 0, 0);
  shmimage->data = shminfo->shmaddr;
  
  XShmAttach(display_, shminfo);

  //END SHARED MEMORY PORTION

  // Set palette

  XVisualInfo desired_visual_template;
  XVisualInfo *visual_info_list; 
  int no_visual_matched;
  Colormap wtcolormap;
  int pixelNum;

  desired_visual_template.screen = screen;
  desired_visual_template.c_class = PseudoColor;
  visual_info_list = XGetVisualInfo(display_, 
				    VisualScreenMask | VisualClassMask,
				    &desired_visual_template,
				    &no_visual_matched);
  if (no_visual_matched == 0)
    cerr << "No visual matched" << endl;
  
  wtcolormap = XCreateColormap(display_, DefaultRootWindow(display_),
			       visual_info_list[0].visual, AllocAll);

  for (pixelNum=0;pixelNum<maxcols;pixelNum++) {
    palette[pixelNum].pixel=pixelNum;
    palette[pixelNum].red=pixelNum*256;
    palette[pixelNum].green=pixelNum*256;
    palette[pixelNum].blue=pixelNum*256;
    palette[pixelNum].flags=DoRed|DoGreen|DoBlue;
  }

  XStoreColors(display_, wtcolormap, palette, maxcols);
  XSetWindowColormap(display_, wtwin_, wtcolormap);


}





