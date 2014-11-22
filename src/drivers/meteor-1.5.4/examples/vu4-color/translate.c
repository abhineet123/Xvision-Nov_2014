/*
 * Converted from Bill Ross's GIL display routines and Sun, to Xavier and X
 *
 * This is a library of routines to handle
 * translation between 24 bit and 8 bit color
 * formats.
 *
 * Original: Bill Ross		April, 1990
 * Xavier version: Joseph O'Sullivan Oct 1994
 */
#include "newcolors.h"
#include "translate.h"

/* Type declatation */
struct color {
    Pixel r;
    Pixel g;
    Pixel b;
};

struct hist_ent {
  int pos;
  struct color color;
  Pixel newcolor;
  long hits;
};

/* Local functions */
/* int qsort(char *base, int nel, int width, int (*compare)()); */

/* Local variables */
extern int cmapsize;
int Greyscale = 0;

/*
 */
static int histcmp_hits(register struct hist_ent *a,
			register struct hist_ent *b)
{
  if (a->hits > b->hits)
    return(-1);
  else if (a->hits < b->hits)
    return(1);
  return(0);
}

static int histcmp_pos(register struct hist_ent *a,
		       register struct hist_ent *b)
{
  if (a->pos > b->pos)
    return(1);
  else if (a->pos < b->pos)
    return(-1);
  return(0);
}

struct hist_ent hist[4096];

/* how can we speed this up?  I'm not sure a macro is a
 * big win... */
static int cdif(register struct color a, register struct color b)
{
  return((int)(abs(a.r-b.r)+abs(a.g-b.g)+abs(a.b-b.b)));
}

/* This builds a color map for the sun display and
 * also a translation table to convert 24 bit pixels
 * to 8 bit pixels.  Input is the three bands of an
 * image plus the dimensions of the image.
 */
void translate_build(Pixel *red,
		     Pixel *green,
		     Pixel *blue,
		     int width, int height)
{
  register long i, j, size;
  register unsigned short rim, x;
  int best;
  struct color c;
  
  size = (long)width * (long)height;
  
  if (Greyscale) {
    /* initialize our translation table/histogram */
    for (x = 0 ; x < 256 ; x++) {
      hist[x].pos = x;
      hist[x].hits = 0;
      hist[x].color.r = (Pixel) x;
      hist[x].color.g = (Pixel) x;
      hist[x].color.b = (Pixel) x;
    }
    
    /* reduce the image to 12 bits/pixel = 4 bits/band
     * and pack 3 bands for each pixel into a short */
    for (i = 0 ; i < size ; i++) {
       rim = (unsigned short) (*(red++) * .326 +
			       *(green++) * .578 +
			       *(blue++) * .096);
       hist[rim].hits++;
    }
    qsort((char *)hist,256,sizeof(struct hist_ent),
	  (int (*) (const void *, const void *))histcmp_hits);
    
    /* turn the first cmapsize elements into the colormap 
     * last elements of map hold foreground and background for suntools */
    for (i = 0 ; i < cmapsize ; i++) {
      rmap[i] = (Pixel) (hist[i].color.r);
      gmap[i] = (Pixel) (hist[i].color.g);
      bmap[i] = (Pixel) (hist[i].color.b);
      /* suntools is wierd -- if we display 0,0,0 we get the
       * BACKGROUND color -- which could be WHITE! 
       * So instead display 1,1,1 */
      if ( (rmap[i]==0) && (gmap[i]==0) && (bmap[i]==0) )
	rmap[i] = gmap[i] = bmap[i] = 1;
    }
    
    /* the first cmapsize colors will be mapped to themselves */
    for (i = 0 ; i < cmapsize ; i++)
      hist[i].newcolor = i;
    
    /* map newcolor for the rest of the colors */
    for (i = cmapsize ; i < 256 ; i++) {
      if (hist[i].hits == 0) continue;  /* maybe we should resolve these.. */
      best = 0;
      c = hist[i].color;
      for (j = 0 ; j < cmapsize ; j++) {
	if (cdif(c, hist[j].color) < cdif(c, hist[best].color))
	  best = j;
      }
      hist[i].newcolor = best;
    }
    
    /* now we re-sort the histogram so that we
     * can access things by combined pixel value.  
     * Then we map everything into the final image */
    qsort((char *)hist,256,sizeof(struct hist_ent),
	  (int (*) (const void *, const void *))histcmp_pos);
  }
  else { /* Not Greyscale */
    
    /* initialize our translation table/histogram */
    for (x = 0 ; x < 4096 ; x++) {
      hist[x].pos = x;
      hist[x].hits = 0;
      hist[x].color.r = (Pixel) (x/256);
      hist[x].color.g = (Pixel) ((x-(256*(x/256)))/16);
      hist[x].color.b = (Pixel) (x-(16*(x/16)));
    }
    
    /* reduce the image to 12 bits/pixel = 4 bits/band
     * and pack 3 bands for each pixel into a short */
    for (i = 0 ; i < size ; i++) {
      rim = (unsigned short) ((Pixel)*(blue++))/16 +
	(((Pixel)*(green++))/16)*16 +
	  (((Pixel)*(red++))/16)*256;
      hist[rim].hits++;
    }
    qsort((char *)hist,4096,sizeof(struct hist_ent),
	  (int (*) (const void *, const void *))histcmp_hits);
    
    /* turn the first cmapsize elements into the colormap 
     * last elements of map hold foreground and background for suntools */
    for (i = 0 ; i < cmapsize ; i++) {
      rmap[i] = (Pixel) (16*hist[i].color.r);
      gmap[i] = (Pixel) (16*hist[i].color.g);
      bmap[i] = (Pixel) (16*hist[i].color.b);
      /* suntools is wierd -- if we display 0,0,0 we get the
       * BACKGROUND color -- which could be WHITE! 
       * So instead display 1,1,1 */
      if ( (rmap[i]==0) && (gmap[i]==0) && (bmap[i]==0) )
	rmap[i] = gmap[i] = bmap[i] = 1;
    }
    
    /* the first cmapsize colors will be mapped to themselves */
    for (i = 0 ; i < cmapsize ; i++)
      hist[i].newcolor = i;
    
    /* map newcolor for the rest of the colors */
    for (i = cmapsize ; i < 4096 ; i++) {
      if (hist[i].hits == 0) continue;  /* maybe we should resolve these.. */
      best = 0;
      c = hist[i].color;
      for (j = 0 ; j < cmapsize ; j++) {
	if (cdif(c, hist[j].color) < cdif(c, hist[best].color))
	  best = j;
      }
      hist[i].newcolor = best;
    }
    
    /* now we re-sort the histogram so that we
     * can access things by combined pixel value.  
     * Then we map everything into the final image */
    qsort((char *)hist,4096,sizeof(struct hist_ent),
	  (int (*) (const void *, const void *))histcmp_pos);
  }
}

/* This uses the maps built by build_maps() to
 * convert a 3 band image into a single band image.
 * The output band is loaded into outimg. */
void translate_box(unsigned char *red, unsigned char *green, unsigned char *blue, 
		   int width, int height,
		   unsigned char *outimg)
{
  register long x, size;
  register unsigned short rim;
  
  size = (long) width * (long) height;
  
  if (Greyscale) 
    for (x = 0 ; x < size ; x++) {
       rim = (unsigned short) (*(red++) * .326 +
			       *(green++) * .578 +
			       *(blue++) * .096);
       outimg[x] = hist[rim].newcolor;
    }
  else
    for (x = 0 ; x < size ; x++) {
       rim = (unsigned short) ((Pixel)*(blue++))/16 + 
	 (((Pixel)*(green++))/16)*16 +
	   (((Pixel)*(red++))/16)*256;
       outimg[x] = hist[rim].newcolor;
    }
}


/* Return an 8 pit value for the given 24
 * bit pixel */
Pixel translate_pix(Pixel *pix)
{
  register unsigned short rim;
  
 
  if (Greyscale) {
    return(hist[pix[0]].newcolor);
  }
  else {
    rim = (unsigned short) pix[2]/16;
    rim += (unsigned short) (pix[1]/16)*16;
    rim += (unsigned short) (pix[0]/16)*256;
    return(hist[rim].newcolor);
  }
}


/* initialize the conversion table and load
 * initial colormap.  */
void translate_init(void)
{
  register unsigned short x;
  register int i;
  int r, g, b;

  rmap = (Pixel *) malloc (256 * sizeof (Pixel));
  gmap = (Pixel *) malloc (256 * sizeof (Pixel));
  bmap = (Pixel *) malloc (256 * sizeof (Pixel));
  if (Greyscale) {
    for (i = 0 ; i < 256 ; i++) {
      rmap[i] = (Pixel) i;
      gmap[i] = (Pixel) i;
      bmap[i] = (Pixel) i;
      /* initialize the conversion table */
      hist[i].pos = i;
      hist[i].hits = 0;
      hist[i].newcolor = i;
      hist[i].color.r = (Pixel) i;
      hist[i].color.g = (Pixel) i;
      hist[i].color.b = (Pixel) i;
    }
  }
  else {
    /* initialize the conversion table */
    for (x = 0 ; x < 4096 ; x++) {
      hist[x].pos = x;
      hist[x].hits = 0;
      hist[x].newcolor = newcolors[x];
      hist[x].color.r = (Pixel) (x/256);
      hist[x].color.g = (Pixel) ((x-(256*(x/256)))/16);
      hist[x].color.b = (Pixel) (x-(16*(x/16)));
    }
    /* initialize the color map -- first some greys */
    for (i = 0 ; i < 128 ; i++) {
      rmap[i] = (Pixel) 2*i;
      gmap[i] = (Pixel) 2*i;
      bmap[i] = (Pixel) 2*i;
    }
    /* now add some colors */
    i = 128;
    for (r = 51 ; r < 256 ; r += 51) {
      for (g = 51 ; g < 256 ; g += 51) {
	for (b = 51 ; b < 256 ; b += 51) {
	  rmap[i] = (Pixel) r;
	  gmap[i] = (Pixel) g;
	  bmap[i++] = (Pixel) b;
	}
      }
    }
    rmap[254] = gmap[254] = bmap[254] = 255;
    rmap[255] = gmap[255] = bmap[255] = 0;
  }
}



/******************************************************************/
/* Xwindow specifics.                                             */
/******************************************************************/

