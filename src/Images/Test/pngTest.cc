/*
 * pngTest.cc
 *
 *  Reading and Writing and Displaying of PNG images
 * jcorso
 */

#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"

#include <XVImageBase.h>
#include <XVColorImage.h>
#include <XVImageScalar.h>
#include <XVImageRGB.h>
#include <XVImageIO.h>
#include <XVWindowX.h>


int main (int argc, char **argv)
{
  XVImageRGB<XV_RGB> im(1,1);
  XVImageRGB<XV_RGB> di(1,1);
	XVImageScalar<u_char> gm(1,1);
  XVImageScalar<float> gaussian; 
  XVWindowX <XV_RGB> *win;
	int ret;

	if (argc < 2) {
    fprintf(stderr,"usage: %s image_filename.png [1 iff image is gray]\n",argv[0]);
		exit(-1);
	}

	if (argc == 3) {
	  ret = XVReadPNG(gm,argv[1]);
	} else {
	  ret = XVReadPNG(im,argv[1]);
	}
	if (ret != 1) {
    fprintf(stderr,"could not read %s correctly [%d]\n",argv[1],ret);
		exit(-2);
	}

  win = new XVWindowX <XV_RGB> (im);
  win->map();
	if (argc == 3) {
    di = gm;
	} else {
    di = im;
	}
  win->CopySubImage(di);
  win->swap_buffers();
  win->flush();

	getchar();

	if (argc == 3) {
		printf("Writing gray.png\n");
    XVWritePNG(gm,"gray.png");
	} else {
		printf("Writing color.png\n");
    XVWritePNG(im,"color.png");
	}

  return 0;
}

