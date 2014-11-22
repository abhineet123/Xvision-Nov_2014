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
// every function in Consoles 

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include "XVImageRGB.h"
#include "XVImageScalar.h"
#include "XVImageBase.h"
#include "XVImageIterator.h"


// One of the following two should be defined 
// depending if you are using the program for 
// testing or for timing:
//#define TESTING
#define TIMING

// Width and height of the images to be tested.
// It is recommended to set them to small values 
// when testing and large values when timing.
#ifndef WIDTH
#define WIDTH 10
#endif
#ifndef HEIGHT
#define HEIGHT 10
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
  cout<<"seconds elapsed : "<<sec_elapsed<<endl\
      <<"seconds per operation : "<<(sec_elapsed/NTIMES)<<endl;
#endif


// These are tests for various functions in Consoles 

template <class T, class T1>
void filter_scalars(T image, T1 target){
  print_type(image);
  cout<<"and";
  print_type(target);
  DO_TEST("target = Prewitt_x(image)", 
           Prewitt_x(image), MEDIUM);
  DO_TEST("target = Prewitt_y(image)", 
           Prewitt_y(image), MEDIUM);  
  DO_TEST("target = Box_Filter_x(image, 2)", 
           Box_Filter_x(image, 2), MEDIUM);
  DO_TEST("target = Box_Filter_y(image, 2)", 
           Box_Filter_y(image, 2), MEDIUM);
}

template <class T>
void print_all(T image) {         //don't use this one for timing
  print_type (image);
  DO_TEST("printing image", image, SLOW);
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

template <class T, class T1>
void resample_all(T image, T1 x) {
  XVImageBase<T1> target (1, 1);
  print_type (image);
  DO_TEST("target = image.resample(1, target)", 
           target = image.resample(1, target), MEDIUM);
  DO_TEST("target = image.resample(2, target)", 
           target = image.resample(2, target), MEDIUM);
  DO_TEST("target = image.resample(3, target)", 
           target = image.resample(3, target), MEDIUM);
  DO_TEST("target = image.resample(4, target)", 
           target = image.resample(4, target), MEDIUM);
}

template <class T>
void convolve_scalars (T image) {
  T target(100, 100), temp(2, 2);
  temp = 0.25;
  print_type (image);
  DO_TEST("target = convolve (temp, image, 0)",
	  target = convolve (temp, image, 0), SLOW);
}

template <class T, class T1, class T2>
void convert_all (T image0, T1 image1, T2 x) {
  XVImageBase<T2> target(1,1);
  print_type(image0);
  cout<<"and";
  print_type(image1);
  DO_TEST("image0 = RGBtoScalar(image1, image0)",
	  image0 = RGBtoScalar(image1, image0), MEDIUM);
  DO_TEST("target = ScalartoRGB(image0, image1)",
	  target = ScalartoRGB(image0, image1), MEDIUM);
}

template <class T, class T1>
void pgm_and_ppm (T image0, T1 image1) {
  print_type(image0);
  DO_TEST("image0.write_pgm(\"image0.pgm\")",
          image0.write_pgm("image0.pgm"), MEDIUM); 
  DO_TEST("image0.read_pgm(\"image0.pgm\")",
          image0.read_pgm("image0.pgm"), MEDIUM); 
  print_type(image1);
  DO_TEST("image1.write_ppm(\"image1.ppm\")",
          image1.write_ppm("image1.ppm"), MEDIUM); 
  DO_TEST("image1.read_ppm(\"image1.ppm\")",
          image1.read_ppm("image1.ppm"), MEDIUM); 
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

template <class T, class T1>
void operators_scalar(T image, T1 x) {
  T target(1,1);
  print_type(image);
  DO_TEST ("image.mean()", image.mean(), FAST);
  DO_TEST ("image.sum()", image.sum(), FAST);
  DO_TEST ("target = x", target = x, FAST);
  DO_TEST ("target = image + x", target = image + x, FAST);
  DO_TEST ("target = image - x", target = image - x, FAST);
  DO_TEST ("target = image * x", target = image * x, FAST);
  DO_TEST ("target = image / x", target = image / x, FAST);
  DO_TEST ("target += x", target += x, FAST);
  DO_TEST ("target -= x", target -= x, FAST);
  DO_TEST ("target *= x", target *= x, FAST);
  DO_TEST ("target /= x", target /= x, FAST);
  DO_TEST ("target = -image", target = -image, FAST);
  DO_TEST ("target + image", target + image, FAST);
  DO_TEST ("target - image", target - image, FAST);
  DO_TEST ("target * image", target * image, FAST);
  XVImageWIterator<T1> *iter;
  iter = new XVImageWIterator<T1> (image);
  for (;iter->end() == false; ++(*iter)) 
    if (**iter == 0) **iter = 1;
  delete iter;
  DO_TEST ("target / image", target / image, FAST);
  DO_TEST ("target += image", target += image, FAST);  
  DO_TEST ("target -= image", target -= image, FAST);  
  DO_TEST ("target *= image", target *= image, FAST);  
  DO_TEST ("target /= image", target /= image, FAST);  
  DO_TEST ("target = image.reduce_resolution(2, 2, target)",
	   target = image.reduce_resolution(2, 2, target), MEDIUM);
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
  
#ifdef TESTING
  print_all(image1);
  print_all(image2);
  print_all(image3);

  print_all(image16);
  print_all(image24);
  print_all(image32);
#endif

  resample_all(image1, x1);
  resample_all(image2, x2);
  resample_all(image3, x3);  

  resample_all(image16, x16);
  resample_all(image24, x24);
  resample_all(image32, x32);
  
  convolve_scalars(image1);
  convolve_scalars(image2);
  convolve_scalars(image3);

  convert_all(image1, image16, x16);
  convert_all(image2, image24, x24);
  convert_all(image3, image32, x32); 

  pgm_and_ppm(image1, image16);  
  pgm_and_ppm(image2, image24);  
  pgm_and_ppm(image3, image32);  
  
  operators_scalar (image1, x1);
  operators_scalar (image2, x2);
  operators_scalar (image3, x3);

  filter_scalars(image1, image2);
  filter_scalars(image2, image3);
  filter_scalars(image3, image1);

  return(0);
}








