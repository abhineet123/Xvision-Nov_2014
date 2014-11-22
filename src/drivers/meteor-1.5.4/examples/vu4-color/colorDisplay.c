/************************************************************************
 *
 *   FILENAME:         colorDisplay.c
 *                 
 *   ABSTRACT:         Adds routines for display images in an ezx window
 *                     
 * Joseph   mar:13:95 based upon extraDisplay.c modified so as not to use
 *                    private colormap, and so as to be substantially 
 *                    faster...
 *
 ************************************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include "translate.h"
#include "colorDisplay.h"

#define NSAMPLESECS 5

static Pixel GammaCorrect[255];
Colormap colormap;
Pixel pixels[256];
int cmapsize=0;
extern Display *display_;
extern Window wtwin_;
extern GC gc_;
static int screen;
struct timeval tp1, tp2;  
int time_elapsed;       

static void GammifyRgb(Pixel *r,Pixel  *g,Pixel  *b)
{
  *r = GammaCorrect[*r];
  *g = GammaCorrect[*g];
  *b = GammaCorrect[*b];
}

/* install the given colormap */
static void redo_cmap(Pixel *rmap,Pixel *gmap,Pixel *bmap)
{
   int i;
   XColor xcolor;
   
   xcolor.flags= DoRed | DoGreen | DoBlue;
   for (i = 0 ; i < cmapsize ; i++) {
      xcolor.pixel = pixels[i];
      xcolor.red   = (unsigned short) (rmap[i]<<8);
      xcolor.green = (unsigned short) (gmap[i]<<8);
      xcolor.blue  = (unsigned short) (bmap[i]<<8);
      XStoreColor(display_, colormap, &xcolor);
   }

   return;
}

static void translate_setup(int xsize, int ysize,
			    unsigned char *R,
			    unsigned char *G,
			    unsigned char *B)
{
   
   static Pixel ts_r[(int) (MAXCOLS * MAXROWS)];
   static Pixel ts_g[(int) (MAXCOLS * MAXROWS)];
   static Pixel ts_b[(int) (MAXCOLS * MAXROWS)];
   int    ts_index=0;
   int    i, j;

   for (i=0; i< xsize; i++)
     for (j=0; j< ysize; j++, ts_index++) {
	ts_r[ts_index] = (Pixel) R[ts_index];
	ts_g[ts_index] = (Pixel) G[ts_index];
	ts_b[ts_index] = (Pixel) B[ts_index];
	GammifyRgb (&ts_r[ts_index], &ts_g[ts_index], &ts_b[ts_index]);
     }
   
   translate_build(ts_r, ts_g, ts_b, ts_index, 1);
}
	
/*
 Assumes a Pseudo graphic display.
 */
void init_colormaps (Display *dpy)
{
   screen = XDefaultScreen (dpy);

   fprintf(stderr, "Number of display cells = %d\n",
	   XDisplayCells(dpy, screen));
   fprintf(stderr, "Number of display planes = %d\n",
	   XDisplayPlanes(dpy, screen));

   colormap = XDefaultColormap(dpy, screen);
   while (XAllocColorCells(display_, colormap, False, NULL, 0, 
			   &(pixels[cmapsize]), 1))
     cmapsize++;
   
   cmapsize = (cmapsize > 60) ? 60 : cmapsize;
   fprintf(stderr, "Number of colors available = %d\n", cmapsize);

   if (cmapsize<5)
     {
	fprintf(stderr, "Number of colors available = %d\n", cmapsize);
	fprintf(stderr, "This is too few to run the display software.\n");
	fprintf(stderr, "Shut down windows, or restart X server.\n");
	exit(-1);      
     }
  
   /* build a grey scale colormap as a default, so that other programs */
   /* can't grab the colors*/
   {
      XColor xcolor;
      int i;
      
      xcolor.flags= DoRed | DoGreen | DoBlue;
      for (i = 0 ; i < cmapsize; i++) {
	 xcolor.pixel = pixels[i];
	 xcolor.red = (unsigned short) (i<<8);
	 xcolor.green = (unsigned short) (i<<8);
	 xcolor.blue = (unsigned short) (i<<8);
	 XStoreColor (display_, colormap, &xcolor);
      }
   }
   
  /* Generate global gamma correction table for this display */
  {
    Pixel index;
    double gamma = 1.0/1.0;
    
    for(index =0; index < 255; index++) {
      GammaCorrect[index] = (Pixel)(0.5 + 255.0 * 
				     (double) pow((double)(index)/255.0, 
						  gamma));
    }
  }

  /* Set up the internal histogram tables*/
  translate_init();

  return;
}
	
void display_image (int xsize, int ysize,
		    Pixel *R,
		    Pixel *G,
		    Pixel *B)
{
   static char    data[(int) (MAXCOLS * MAXROWS)];
   int            i;
   static XImage        *ximage;
   static int frame_count=-1;

   frame_count++;
   if ((frame_count % (NSAMPLESECS * MAXFPS)) == 0) {
      gettimeofday(&tp2, NULL); 
      time_elapsed = (int) (((tp2.tv_sec - tp1.tv_sec) * 1000)        
			    + ((tp2.tv_usec - tp1.tv_usec) / 1000));  
      printf ("caught %d frames in %.1f sec\n", 
	      (frame_count) / NSAMPLESECS,
	      ((float) time_elapsed) / (NSAMPLESECS * 1000.0));
      tp1 = tp2;
      frame_count = 0;
      translate_setup (xsize, ysize, R, G, B);
      redo_cmap(rmap,gmap,bmap);

      return;
   }
   
   translate_box (R, G, B, xsize, ysize, data);
   for (i = 0; i < xsize * ysize; i++)
     data[i] = pixels[data[i]];
   ximage = XCreateImage(display_,
			 DefaultVisual(display_, DefaultScreen(display_)),
			 8, ZPixmap,
			 0, 
			 (char*) data,
			 MAXCOLS, MAXROWS,
			 8, MAXCOLS);
   ximage->byte_order = XImageByteOrder(display_);

   XPutImage(display_, wtwin_, gc_, ximage, 
	     0, 0, 
	     0, 0,
	     MAXCOLS, MAXROWS);
    
   ximage->data = NULL; 
   XDestroyImage(ximage);
   XFlush(display_);
}  

