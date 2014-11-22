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
#include "XVImageBase.h"
#include <XVV4L2.h>


static  XVV4L2<XVImageRGB<XV_RGB> >         *grabber;

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
   grabber = new XVV4L2<XVImageRGB<XV_RGB> >(DEVICE_NAME,"B2");
   XVWindowX<XV_RGB>      window1(640,480);
   XVImageRGB<XV_RGB>     im;
   typedef XVInteractWindowX<XV_RGB>    WIN;
   
   window1.map();
    //grabber -> set_params ("I1N0B2");
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  grabber->next_frame_continuous();
  WIN win(grabber->next_frame_continuous());
  win.map();
 
  for (;;) {
    //Start timing
    gettimeofday (&time1, NULL);
    for (int i=0; i<rate; i++) {


      im=grabber->next_frame_continuous();   
      //cout << im;
      window1.CopySubImage(im);     
      win.CopySubImage(im);
      window1.swap_buffers();
      window1.flush();
      
    }
    //Produce timing results
    gettimeofday (&time2, NULL);
    cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
    (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;

  }
  return 0;
}

