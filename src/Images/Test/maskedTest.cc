/*
 * maskedTest.cc
 *
 * a simple test program for the XVMaskedImage class
 * jcorso
 */

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"

#include <XVImageBase.h>
#include <XVColorImage.h>
#include <XVWindowX.h>
#include <XVVideo.h>
#ifdef HAVE_BTTV
#include <XVBt8x8.h>
#endif

//#define USE_DIRECTBUFFER

#include <XVMaskedImage.h>

//#define TEST_TYPE u_char
//#define TEST_TYPE u_short
#define TEST_TYPE float

void setupmask(XVMaskedImage<XVImageScalar<TEST_TYPE> >&);


int main (int argc, char **argv)
{
  int old_frame,new_frame,num_buffers; 
  XVImageRGB<XV_RGB> im;
  XVWindowX <XV_RGB> *win;
  XVBt8x8 <XVImageRGB<XV_RGB> > *vid;
  XVMaskedImage<XVImageScalar<TEST_TYPE> > *mim;
  struct timeval start_time,end_time;


  vid = new XVBt8x8 < XVImageRGB<XV_RGB> > ("/dev/video0");
  vid->set_params("B2I1N0");
  num_buffers = vid->buffer_count();

  win = new XVWindowX <XV_RGB> (vid->frame(0));
#ifdef USE_DIRECTBUFFER
  win->setImages(&(vid->frame(0)),num_buffers);
#endif
  win->map();

  mim = new XVMaskedImage<XVImageScalar<TEST_TYPE> >(640,480);
  setupmask(*mim);

  new_frame = 0;
  vid->initiate_acquire(new_frame); 


  while(1) {
  
    gettimeofday(&start_time,NULL);

    for(int i=0;i<50;i++) {
      old_frame = new_frame;
      new_frame = (new_frame+1)%num_buffers;
      vid->initiate_acquire(new_frame);
      vid->wait_for_completion(old_frame);

      RGBtoScalar(vid->frame(old_frame), *(mim->realimage) );
      mim->intersect();
      im = ScalartoRGB(*mim->maskimage,im);
      //im = ScalartoRGB(*mim->realimage,im);
     
#ifdef USE_DIRECTBUFFER
      win->CopyImage(old_frame);
#else
      win->CopySubImage(im);
#endif

      win->swap_buffers();
      win->flush();
    }

    gettimeofday(&end_time,NULL);
    cerr << 50/((end_time.tv_sec-start_time.tv_sec)+
               (end_time.tv_usec-start_time.tv_usec)*1e-6)
         << " [Hz]" << endl;
  }
 
  return 0;

}


void setupmask(XVMaskedImage<XVImageScalar<TEST_TYPE> > & m)
{
  int i,j;
  u_char *c;

  c = (u_char *)m.lock();

  //*
  for (i=0;i<480;i++) {
    for (j=0;j<640;j++) {
      c[i*640+j] = 0;
    }
  }
  // */


  for (i=100;i<400;i++) {
    for (j=100;j<400;j++) {
      c[i*640+j] = 0xFFFF;
    }
  }


  for (i=400;i<540;i++) {
    for (j=300;j<460;j++) {
      c[j*640+i] = 0xFFFF; 
    }
  }

  m.unlock();

}
