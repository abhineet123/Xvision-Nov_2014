/************************************************************************
 *
 *   FILENAME:         extraDisplay.h
 *               
 *   ABSTRACT:         Adds routines for display images in an ezx window
 *
 ************************************************************************/
#ifndef CDISPLAY_H_INCLUDED
#define CDISPLAY_H_INCLUDED

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
#define MAXROWS 240
#define MAXCOLS 320
#define DEFAULT_FORMAT METEOR_FMT_NTSC
#define MAXFPS 30
#endif

extern void init_colormaps (Display *);
void display_image (int x, int y,
		    Pixel *R,
		    Pixel *G,
		    Pixel *B);

#endif




