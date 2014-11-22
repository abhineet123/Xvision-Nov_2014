#include <config.h>
#include <XVMpeg.h>
#include <XVDig1394.h>
#include <XVVideo.h>
#include <XVColorSeg.h>
#include <XVBlobFeature.h>
#include <XVTracker.h>
#include <XVWindowX.h>
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

static u_short preferredHue = 0;
static u_short range = 2;

int main(int argc, char * argv[]){

  typedef XVVideo<XVImageRGB<XV_RGB> > VID;
  typedef XVMpeg<XVImageRGB<XV_RGB> >  MPG;
  typedef XVDig1394<XVImageRGB<XV_RGB> >  FIREWIRE;
  typedef XVInteractWindowX<XV_RGB>    WIN;
  typedef XVV4L2<XVImageRGB<XV_RGB> >  V4L2;
  typedef XVBlobFeature<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > BLOB;

    VID *vid = new MPG(argv[1]);
 // VID * vid = new FIREWIRE(DC_DEVICE_NAME,"i1V1");
    //VID *vid = new V4L2(DEVICE_NAME,"B2");
  for(int i=0;i<15;i++)
    vid->next_frame_continuous();
  //cout << vid->frame(0);
  WIN win(vid->frame(0));
  win.map();

  XVHueRangeSeg<XV_RGB, u_short> seg;

  BLOB feat(seg); //, true);  // for resampling

  XVDisplayTracker<VID, WIN, BLOB> tracker(*vid, win, feat);

  XVBlobState bs = tracker.init();

  while(1){
    try {
      bs = tracker.nextState();
    }catch(XVTrackerException te){ cout << te.what() << endl; };
  }
  delete vid;
};
