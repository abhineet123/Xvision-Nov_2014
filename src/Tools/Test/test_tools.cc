/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permisson is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

// This a universal program for testing and timing 
// every function in Tools 

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include "XVImageRGB.h"
#include "XVImageScalar.h"
#include "XVImageBase.h"
#include "XVImageIterator.h"
#include "GeoTransform.h"

// One of the following two should be defined 
// depending if you are using the program for 
// testing or for timing:
//#define TESTING
#define TIMING

// Width and height of the images to be tested.
// It is recommended to set them to small values 
// when testing and large values when timing.
#ifndef WIDTH
#define WIDTH 1000
#endif
#ifndef HEIGHT
#define HEIGHT 1000
#endif

// Number of times certain routines should be called during timing.
#define SLOW 100
#define MEDIUM 1000
#define FAST 10000

#ifdef TESTING
#define DO_TEST(message,x,NTIMES)   cout<<endl<<(message)<<endl<<(x)<<endl;
#endif
#ifdef TIMING
  struct rusage rbuffer;
  float usec_started, sec_started,
      usec_elapsed, sec_elapsed;
#define DO_TEST(message,x,NTIMES)\
  cout<<endl<<(message)<<' '<<NTIMES<<" times"<<endl;\
\
  (void) getrusage (RUSAGE_SELF, &rbuffer);\
  usec_started = rbuffer.ru_utime.tv_usec;\
  sec_started  = rbuffer.ru_utime.tv_sec;\
  for (int i=0; i<NTIMES; i++) x;\
\
  (void) getrusage (RUSAGE_SELF, &rbuffer);\
  usec_elapsed = rbuffer.ru_utime.tv_usec - usec_started;\
  sec_elapsed  = rbuffer.ru_utime.tv_sec  - sec_started\
                  + usec_elapsed/1000000;\
  cout<<"sec_elapsed  : "<<sec_elapsed<<endl;\
  cout<<"seconds per operation : "<<(sec_elapsed/NTIMES)<<endl;
#endif


// These are tests for various functions in Tools 

template <class T, class T1>
void warp_scalar (T image0, T1 x) {
  T target0 (100,100);
  print_type (image0);
  GeoTrans<T1> all(1.5, 1.2, 1.2, 0.3);     //rotate, scale, sheer
  //image0.read_pgm("image0.pgm"); 
  DO_TEST("target0 = all.reverseWarp 
         (image0, target0, image0.Width()/2, image0.Height()/2)",
          target0 = all.reverseWarp 
	  (image0, target0, image0.Width()/2, image0.Height()/2), SLOW);
  target0.write_pgm("target0.pgm"); 
}

template <class T, class T1>
void warp_RGB (T image1, T1 x) {
  T target1 (100,100);
  print_type (image1);
  GeoTransRGB<T1> allRGB(1.5, 1.2, 1.2, 0.3);   //rotate, scale, sheer
  //image1.read_ppm("image1.ppm");
  DO_TEST("target1 = allRGB.reverseWarp 
         (image1, target1, image1.Width()/2, image1.Height()/2)",
          target1 = allRGB.reverseWarp 
	  (image1, target1, image1.Width()/2, image1.Height()/2), SLOW); 
  target1.write_ppm("target1.ppm"); 
}

template <class T>
void print_type (T image) {          //not for timing
  cout<<endl<<endl<<"  OPERATING ON IMAGE TYPE: ";
  int type = figure_out_type(*image[0,0]);
  switch (type) {
  case 0: cout<<"u_char";
    break;
  case 1: cout<<"u_short";
    break;
  case 2: cout<<"u_int";
    break;
  case 10: cout<<"char";
    break;
  case 11: cout<<"short";
    break;
  case 12: cout<<"int";
    break;
  case 13: cout<<"float";
    break;
  case 14: cout<<"double";
    break;
  case 21: cout<<"RGB15";
    break;
  case 22: cout<<"RGB16";
    break;
  case 23: cout<<"RGB24";
    break;
  case 24: cout<<"RGB32";
    break;
  default: cout<<"unknown";
  }
  cout<<endl;
}

template <class T>
void init_scalar (XVImageBase<T> &image) {     //for initialization only
  int m=image.Width(), n=image.Height(), i, j;
  XVImageWIterator<T> iter (image);
  for (i=0; i<n; ++i)
    for (j=0; j<m; ++j, ++iter) 
      *iter = i+j;
}

template <class T>
void init_RGB (XVImageBase<T> &image) {        //for initialization only
  int m=image.Width(), n=image.Height(), i, j;
  XVImageWIterator<T> iter (image);
  for (i=0; i<n; ++i)
    for (j=0; j<m; ++j, ++iter) 
      iter->r = iter->g = iter->b = i+j;
}

int main (int argc, char **argv) {

  int w = WIDTH, h = HEIGHT;
  cout<<"Operating on images of size "<<w<<'x'<<h<<endl;
 

  int x1 = 1;
  float x2 = 1;
  double x3 = 1;

  XV_RGB16 x16;
  XV_RGB24 x24;
  XV_RGBA32 x32;

  XVImageScalar<int> image1 (w,h);
  XVImageScalar<float> image2 (w,h); 
  XVImageScalar<double> image3 (w,h);
  
  XVImageRGB<XV_RGB16> image16 (w,h);
  XVImageRGB<XV_RGB24> image24 (w,h);
  XVImageRGB<XV_RGBA32> image32 (w,h); 

  init_scalar (image1);
  init_scalar (image2);
  init_scalar (image3);

  init_RGB (image16);
  init_RGB (image24);
  init_RGB (image32);  
 
  warp_scalar (image1, x1);
  warp_scalar (image2, x2);
  warp_scalar (image3, x3);

  warp_RGB (image16, x16);
  warp_RGB (image24, x24);
  warp_RGB (image32, x32);

  return(0);
}








