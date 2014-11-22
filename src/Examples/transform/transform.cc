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
  XVImageRGB<XV_RGB16> x(140,190), y(1000,1000);
  //XVImageIterator<XV_RGB16> iterx(x), itery(y);
  window=new XVWindow<XV_RGB16>(&x,0,0,0,NULL,4,1);  
  window->map();

  sleep(1);
  // application part (loops etc.)
  GeoTrans<u_short> rotate(1.5);   //theta in radians
  GeoTrans<u_short> scale(1,1);
  GeoTrans<u_short> sheer(0, 0, 0); 
  GeoTrans<u_short> all(0, 1, 1, 0); //rotate, scale, sheer
  GeoTransRGB<XV_RGB16> allRGB(-1.57, -0.3, 0.3, 0);
  cout<<"GeoTransformations rotate and scale created"<<endl;

  XVImageScalar<u_short> v(50, 60),w(1000,1000);
   
  //w.read_pgm("image1.pgm");
  y.read_ppm("barb2.ppm");
  x = allRGB.reverseWarp (y, x, 50, 90);
  //x = y;

  //RGBtoScalar(x, w, XV_GREENBAND);
  //for (int i=0; i<1; i++) {
  //v = all.reverseWarp(w, v, 130, 230);
  //w = v;
  //}
  //v = Box_Filter_x(w,1);
  //w = Box_Filter_y(v,1);

  //ScalartoRGB (w, x);
  //v.write_pgm("image2.pgm");
  x.write_ppm("image2.ppm");
  
  //cout<<"Image after rotating:"<<endl;
  //z.print(cout);
  //cout<<"Image after scaling:"<<endl;
  //v.print(cout);

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
 
