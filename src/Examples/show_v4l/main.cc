#include "config.h"
#include "XVV4L2.h"
#include "XVRemoteWindowX.h"

using namespace std;

int main(int argc, char **argv)
{
  int index=0;
  struct timeval time1, time2;
 

  XVV4L2<XVImageRGB<XV_RGB> > camera(DEVICE_NAME,"B2N1I1");
  XVRemoteWindowX<XV_RGB> win(camera.frame(0).Width(),camera.frame(0).Height());
  win.map();
  camera.initiate_acquire(index);
  while(1)
  {
   gettimeofday (&time1, NULL);
   for(int i=0;i<50;i++)
   {
   camera.initiate_acquire(index^1);
   camera.wait_for_completion(index);
   win.CopySubImage(camera.frame(index));
   win.swap_buffers();
   win.flush();
   index^=1;
   }
   gettimeofday (&time2, NULL);
   cout<<"Rate: "<<50/(time2.tv_sec-time1.tv_sec+
            (time2.tv_usec-time1.tv_usec)*1e-6) << " [Hz]" << endl;

  }
  return 0;
}

