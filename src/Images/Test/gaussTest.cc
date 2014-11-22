/*
 * gaussTest.cc
 *
 * a simple test program for the gaussian decimation of images
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
#include <XVImageScalar.h>
#include <XVWindowX.h>
#include <XVVideo.h>
#ifdef HAVE_BTTV
#include <XVBt8x8.h>
#endif


void make_gaussian(XVImageScalar<float>& gaussian)
{
  gaussian.resize(5,5);

  float *f = gaussian.lock();

  f[0]=0.0030;   f[1]=0.0133;  f[2]=0.0219;  f[3]=0.0133;  f[4]=0.0030;
  f[5]=0.0133;   f[6]=0.0596;  f[7]=0.0983;  f[8]=0.0596;  f[9]=0.0133;
  f[10]=0.0219;  f[11]=0.0983; f[12]=0.1621; f[13]=0.0983; f[14]=0.0219;
  f[15]=0.0133;  f[16]=0.0596; f[17]=0.0983; f[18]=0.0596; f[19]=0.0133;
  f[20]=0.0030;  f[21]=0.0133; f[22]=0.0219; f[23]=0.0133; f[24]=0.0030;

  gaussian.unlock();
}


int main (int argc, char **argv)
{
  int old_frame,new_frame,num_buffers; 
  XVImageRGB<XV_RGB> im(640,480);
  XVImageScalar<float> gim(640,480);
  XVImageScalar<float> gim_p(640,480);
  XVImageScalar<u_char> gim_i(640,480);
  XVImageScalar<u_char> gim_j(640,480);
  XVImageScalar<float> gaussian; 
  XVWindowX <XV_RGB> *win,*win2;
  XVBt8x8 <XVImageRGB<XV_RGB> > *vid;
  struct timeval start_time,end_time;


  vid = new XVBt8x8 < XVImageRGB<XV_RGB> > ("/dev/video0");
  vid->set_params("B2I1N0");
  num_buffers = vid->buffer_count();

  win = new XVWindowX <XV_RGB> (vid->frame(0));
  win->map();
  win2 = new XVWindowX <XV_RGB> (vid->frame(0));
  win2->map();

  make_gaussian(gaussian);
  cout << "built the gaussian mask" << endl;

  new_frame = 0;
  vid->initiate_acquire(new_frame); 

  while(1) {
  
    gettimeofday(&start_time,NULL);

    for(int i=0;i<50;i++) {
      old_frame = new_frame;
      new_frame = (new_frame+1)%num_buffers;
      vid->initiate_acquire(new_frame);
      vid->wait_for_completion(old_frame);

      //*
      gim_j = RGBtoScalar(vid->frame(old_frame),gim_j);
      im = ScalartoRGB(gim_j,im);
      win2->CopySubImage(im);

      //gim_p = convolve(gim,gim_p,gaussian,0);
      gim_i = XVPrewittFilterX(gim_j,(u_char)2,3);
      im = ScalartoRGB(gim_i,im);
      win->CopySubImage(im);
      // */

      //win->CopySubImage(vid->frame(old_frame));

      win->swap_buffers();
      win->flush();
      win2->swap_buffers();
      win2->flush();
    }

    gettimeofday(&end_time,NULL);
    cerr << 50/((end_time.tv_sec-start_time.tv_sec)+
               (end_time.tv_usec-start_time.tv_usec)*1e-6)
         << " [Hz]" << endl;
  }
 
  return 0;

}

