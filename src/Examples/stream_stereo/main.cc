#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include <XVImageScalar.h>
#include "XVAffineWarp.h"
#include <XVVideo.h>
#include <Videre.h>
#include "XVImageBase.h"
#include <XVImageIO.h>


static  XV_Videre<XVImageRGB<XV_RGB> >         *grabber;

void sighndl(int ws) {
  cerr << "SIGINT called" << endl;
  if(grabber) delete grabber;
   exit(0);
}


int main (int argc, char **argv) {

   signal(SIGINT, sighndl);
   signal(SIGSEGV, sighndl);
   const int rate=10;
   struct timeval time1, time2;
   grabber = new XV_Videre<XVImageRGB<XV_RGB> >();
   XVWindowX<XV_RGB>      window1(640,480),window2(640,480);

   window1.map();
   window2.map();
    //grabber -> set_params ("I1N0B2");
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  grabber->wait_for_completion(0);
  XVImageScalar<u_char> **stereo_images=grabber->get_stereo();
  for (;;) {
    //Start timing
    gettimeofday (&time1, NULL);
    for (int i=0; i<rate; i++) {

      grabber->wait_for_completion(0);

      window1.CopySubImage(*(stereo_images[0]));     
      window2.CopySubImage(*(stereo_images[1]));     
 
      window1.swap_buffers();
      window1.flush();
      window2.swap_buffers();
      window2.flush();
    }
    //Produce timing results
    gettimeofday (&time2, NULL);
    cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
    (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;

  }
  return 0;
}

