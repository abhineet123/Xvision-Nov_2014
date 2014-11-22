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
#include <XVPwc.h>
#include <XVImageSeq.h>


static  XVPwc<XVImageScalar<u_char> >         *grabber;

void sighndl(int ws) {
  cerr << "SIGINT called" << endl;
  if(grabber) delete grabber;
   exit(0);
}


int main (int argc, char **argv) {

   int rate=10;
   int index=0;
   signal(SIGINT, sighndl);
   signal(SIGSEGV, sighndl);
   struct timeval time1, time2;
   struct timeval time_acq;
   XVSize  size(640,480);
   XVWindowX<XV_RGB>      window1(size.Width(),size.Height());
   XVImageRGB<XV_RGB> image_col(size.Width(),size.Height());
   //grabber = new XVPwc<XVImageScalar<u_char> >(window1);
   // r is the framerate... 30 seems to work only with 320x240??
   grabber = new XVPwc<XVImageScalar<u_char> >(window1,DEVICE_NAME,"r15");
   grabber->set_agc(-1);  // -1 = default is auto  0...65535
   //grabber->set_shutter(40000);
   window1.map();
  if (!grabber) {
    cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }
  grabber->initiate_acquire(index);
  for (;;) {
    //Start timing
    gettimeofday (&time1, NULL);
    for (int i=0; i<rate; i++) {


      grabber->initiate_acquire(index^1);
      grabber->wait_for_completion(index);
      grabber->get_acquisition_time(index, time_acq);
      cout << "sec: " << time_acq.tv_sec << ", usec: " << time_acq.tv_usec << endl;
      window1.CopySubImage(ScalartoRGB(grabber->frame(index),image_col));     
 
      window1.swap_buffers();
      window1.flush();
      index^=1;
    }
    //Produce timing results
    gettimeofday (&time2, NULL);
    cout<<"Rate: "<<rate/(time2.tv_sec-time1.tv_sec+
    (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;

  }
  return 0;
}

