/*
 *	Copyright (C) 1997 Heiko Teuber (heiko@pc10.pc.chemie.th-darmstadt.de).
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
 *
 *	I hope you enjoy using this program. Please report any bugs and useful
 *	features you would like to find in this program to me.
 *
 */

#include <tcl.h>
#include <tk.h>


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

/* #include <linux/mm.h>   */
/* The above include is unnecessary and will blow up when compiling against
   glibc2 (libc6)  */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/XShm.h>
#include <X11/keysym.h>


#include "ioctl_meteor.h"

/*
 *             Definitionen und Deklarationen
 */


/*
 *             FRAMEGRABBER-Widget
 */


typedef struct {
  int value;
} Counter;

typedef struct {

  /* geometry */
  unsigned int columns;
  unsigned int rows;
  signed   int hue;
  unsigned int chcv;   /* unsigned char!! auch die naechsten drei */
  unsigned int bright;
  unsigned int chroma;
  unsigned int contrast;
  unsigned long oformat;
  char *out_format;
  char *source;

} FGrabSetting;


typedef struct {

  /* Tcl/Tk stuff */
  Tk_Window tkwin;
  Display *display;
  Tcl_Interp *interp;
  GC gc;
  Screen *screen;
  int depth;
  XShmSegmentInfo *shminfo;
  XImage *shmimage;

  Tk_3DBorder bgBorder;
  int borderWidth;
  int relief;
  
  /* device stuff */ 
  int   meteor;
  char *DevName;
  char *mmbuf;
  char *device;
  char *format;
  char *yuv;
  int fps;
  

  /* ?stuff */
  int pixelsize;
  int bufsize;
  int fieldmode;
  int sigmode;
 
  /* geometry stuff */

  FGrabSetting video;
  FGrabSetting capture;

  /* other stuff */
  int updatePending;
  int geoChange; 

} FGrab;



/*
 *             SQUARE-Widget     
 */



typedef struct {
  Tk_Window tkwin;
  Display *display;
  Tcl_Interp *interp;
  int x, y;
  int size;
  int borderWidth;
  Tk_3DBorder bgBorder;
  Tk_3DBorder fgBorder;
  int relief;
  GC gc;
  int updatePending;
} Square;




/*
 *             SQUARE-Widget    Deklarationen
 */


int PAS_FGrab( ClientData, Tcl_Interp*, int, char** );
int PAS_FGrabObject( ClientData, Tcl_Interp*, int, char** ); 
static int FGrabCmd( ClientData, Tcl_Interp*, int, char** );
static int FGrabConfigure( Tcl_Interp*, FGrab*, int, char**, int );
static int FGrabVideoConfigure( Tcl_Interp*, FGrab*, int, char**, int );
static int FGrabCaptureConfigure( Tcl_Interp*, FGrab*, int, char**, int, int );
static int FGrabMeteorConfigure( Tcl_Interp*, FGrab*, FGrabSetting*, int );
static int FGrabWidgetCmd( ClientData, Tcl_Interp*, int, char** );
static int FGrabVideoOn( FGrab*, Tcl_Interp* );
static int FGrabVideoOff( FGrab*, Tcl_Interp* );
static int FGrabVideoHold( FGrab*, Tcl_Interp* );
static int FGrabCapture( FGrab*, Tcl_Interp*, char* );
static void FGrabEventProc( ClientData, XEvent* );
static void FGrabDisplay( ClientData );
static void FGrabDestroy( ClientData );
static void FGrabGotFrame( int );
static void FGrabCountFrame( int );

static int SquareCmd( ClientData, Tcl_Interp*, int, char** );
static int SquareConfigure( Tcl_Interp*, Square*, int, char**, int );
static int SquareWidgetCmd( ClientData, Tcl_Interp*, int, char** );
static void KeepInWindow( Square* );
static void SquareEventProc( ClientData, XEvent* );
static void SquareDisplay( ClientData );
static void SquareDestroy( ClientData );




#if defined(PAL)
#define MAXROWS 576
#define MAXCOLS 768
#define INPUT_FORMAT "pal"
#define MAXFPS 25
#elif defined(SECAM)
#define MAXROWS 576
#define MAXCOLS 768
#define INPUT_FORMAT "secam"
#define MAXFPS 25
#else /* NTSC */
#define MAXROWS 480
#define MAXCOLS 640
#define INPUT_FORMAT "ntsc"
#define MAXFPS 30
#endif

#if defined(SVIDEO)
#define INPUT_DEVICE "svideo"
#elif defined(DEV0)
#define INPUT_DEVICE "dev0"
#elif defined(DEV1)
#define INPUT_DEVICE "dev1"
#elif defined(DEV2)
#define INPUT_DEVICE "dev2"
#elif defined(RGB)
#define INPUT_DEVICE "rgb"
#else /* RCA */
#define INPUT_DEVICE "rca"
#endif

#if defined(NATOMA)
#define YUV_MODE "packed"
#else /* non-Natoma-Board */
#define YUV_MODE "planar"
#endif

#define NOFRAMES 1
#define MAXPIXELSIZE 4
#define MAXSIZE (NOFRAMES*MAXPIXELSIZE*MAXROWS*MAXCOLS)

#define ROW_MASK 0xfffe
#define COL_MASK 0xfff8

#define FPS 12
#define CHCV 44
#define BRIGHT 148
#define CHROMA 114
#define CONTRAST 74
#define HUE 0


#define COUNT         0x0001
#define ASYNCMARK     0x0002
#define AHANDLEREXIST 0x0004
#define FPSCOUNT      0x0008
#define VIDEO_ON      0x0010
#define VIDEO_HOLD    0x0020
#define PACKED        0x0040





