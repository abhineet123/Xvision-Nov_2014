/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include "XVDig1394.h"
#include "XVImageBase.h"
#include <signal.h>
# include <sys/ioctl.h>
# include <fcntl.h>
# include <termios.h>
# include <XVOmniWarper.h>

XVImageRGB<XV_RGB> mark(XVImageRGB<XV_RGB> &source, int xd, int yd) {
  int width = source.Width() ;
  int height = source.Height() ;

  XVImageRGB<XV_RGB> dest_im(width, height);
  const  XV_RGB      *r_ptr=source.data();
  XV_RGB             *w_ptr=dest_im.lock(), *ptr;  

  memcpy( w_ptr, r_ptr, sizeof(XV_RGB)*width*height );
  for( int i = -10 ; i <= 10 ; i ++ ) {
    ptr = w_ptr + width*yd + xd+i ;
    ptr->setR(255) ; ptr->setG(127) ; ptr->setB(127) ;
    ptr = w_ptr + width*(yd+i) + xd ;
    ptr->setR(255) ; ptr->setG(127) ; ptr->setB(127) ;
  }
  dest_im.unlock();
  return dest_im ;
}

// Math constants definition

#ifndef PI
#define PI 3.14159265358979323846
#endif
#define PI_2 (PI/2)

// Constants depends on the mirror-camera system. May need further calibration.

int main (int argc, char **argv) {
  XVOmniWarper ow ;
  static XVWindow<XV_RGB> *window1;
  static XVWindow<XV_RGB> *window2;
  static XVVideo<XVImageRGB<XV_RGB> > *grabber;

  int framenum=0, next_fr=0, n_buffers, i, j, rate=100,
    output=1, depth;
  struct timeval time1, time2; 

  int x, y, r ;
  if( argc >= 2 ) {
    r = atoi(argv[1]);
  }else {
    r = 150 ;
  }
  if (!grabber) {
    grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0"); //full-size
    //grabber = new Dig1394<XV_RGB>(DC_DEVICE_NAME,"S2"); //half-size
  }
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  n_buffers=grabber->buffer_count();
  //XVImageRGB<XV_RGB> image(grabber->frame(0));
  XVImageRGB<XV_RGB> image, image1, image2 ;

  //Set up the window
  //if (output == 1 || output == 3) {
  window1 = new XVWindowX<XV_RGB> ((grabber->frame(0)));
  window1 -> map();
  window2 = new XVWindowX<XV_RGB> ((grabber->frame(0)));
  window2 -> map();
  //}  

  grabber -> initiate_acquire(framenum);
  for ( j=0 ;;) {
    //Start timing
    gettimeofday (&time1, NULL);
    for ( i=0; i<rate; i++, j++ ) {
      x = (int)(ow.centerX+r*cos((double)j*PI*2/1000)) ;
      y = (int)(ow.centerY+r*sin((double)j*PI*2/1000)) ;

      //Acquire image
      grabber -> initiate_acquire((framenum+1)%n_buffers);
      grabber -> wait_for_completion(framenum);

      //image=unwarp(grabber->frame(framenum),0);
      image = grabber->frame(framenum) ;
      image1= ow.omni2plane(image,320,240,x,y);
      //image=grabber->frame(framenum) ;
      window1 ->CopySubImage(image1);
      window1 -> swap_buffers();
      window1 -> flush();

      image2 = mark( image, x, y );
      window2 ->CopySubImage(image2);
      window2 -> swap_buffers();
      window2 -> flush();

      framenum=(framenum+1)%n_buffers;
    }
    gettimeofday (&time2, NULL);
    cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
    (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;
  }
  return 0;
}
