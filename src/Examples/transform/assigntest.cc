/*                                                     -*-c++-*-
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

/**---------------------------------------------------------------------
 *  A program for testing geometric transformations: transform
 *
 *  1) Creates an object of class GeoTrans and GeoTransRGB
 *  2) Reads in data into an XVImageScalar or XVImageRGB from
 *      an image.pgm or an image.ppm file
 *  3) Calls GeoTrans.warp or GeoTrans.reverseWarp passing it the image
 *      and (optional) calls Box_Filter_x and Box_Filter_y on the image
 *  4) Writes resulting image into image2.pgm or image2.ppm file
 *  5) Displays a window with the resulting image
 *----------------------------------------------------------------------*/

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"
#include "XVWindow.h"
#include "XVImageRGB.h"
#include "GeoTransform.h"
#include "Video.h"

int main(int argc,char **argv)
{
  XVWindow<XV_RGB16> *window;
  XVImageRGB<XV_RGB16> x(140,190), *y, *z;
  XVImageScalar<u_short> v(50,60), w(1000,1000);

  window = new XVWindow<XV_RGB16>(&x,0,0,0,NULL,4,1);
  z = new XVImageRGB<XV_RGB16>(140, 190);
  y = new XVImageRGB<XV_RGB16>(140, 190);
  window->map();
  sleep(1);

  GeoTrans<u_short> all(0, 1, 1, 0); //rotate, scale, sheer
  GeoTrans<XV_RGB16> allRGB(-1.57, -0.3, 0.3, 0);
  cout<<"GeoTransformation classes created"<<endl;
 
  z->readImage("ppmtest.ppm");

  coord zCenter = {50, 90};
  coord yCenter = {y->Width() / 2, y->Height() / 2};
  allRGB.warp(*z, *y, zCenter, yCenter);
  x = *y;

  y->writeImage("out.ppm");
  
  delete y;
  delete z;  

  cout<<"In scalar matrix:"<<endl;
  all.print();
  cout<<"In RGB matrix:"<<endl;
  allRGB.print();

  // initialize XVImages for buffers available
  //window->setImages(&x,1);
  // map window on screen
  cout<<" copy image into window"<<endl;
  // Initialization
  window->CopySubImage(&x);
  //window->CopyImage(0);
  cout<<"Copied image"<<endl;
  // double buffer? then swap buffers
  window->swap_buffers();
  // refresh display
  window->flush();
  sleep(100000);

  return 0;
}
 
