#include <X11/Intrinsic.h>

Pixel *rmap, *gmap, *bmap;

void translate_build(Pixel *red,
		     Pixel *green,
		     Pixel *blue,
		     int width, int height);
void translate_box(unsigned char *red,
		   unsigned char *green,
		   unsigned char *blue, 
		   int width, int height,
		   unsigned char *outimg);
Pixel translate_pix(Pixel *pix);
void translate_init(void);
