#include <config.h>
#include <XVSSD.h>
#include <XVTracker.h>
#include <XVWindowX.h>
#include <XVMpeg.h>
#include <XVImageSeq.h>
#ifdef HAVE_BTTV
#include <XVBt8x8.h>
#endif
#include <XVDig1394.h>
#include <iostream>
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

// choose your weapon

#define _EXSTEPPER_ XVTransStepper
#define _EXSTATE_ XVTransState
//#define _EXSTEPPER_ XVSE2Stepper
//#define _EXSTATE_   XVSE2State

int main(int argc, char * argv[]){

  typedef XVVideo<XVImageRGB<XV_RGB> > VID;
  typedef XVImageSeq<XVImageRGB<XV_RGB> >  MPG;
//  typedef XVBt8x8<XVImageRGB<XV_RGB> > DIG;
  typedef XVDig1394<XVImageRGB<XV_RGB> > DIG;
  typedef XVInteractWindowX<XV_RGB >    WIN;
  typedef XVV4L2<XVImageRGB<XV_RGB> >   V4L2;
//   typedef XVRemoteWindowX<XV_RGB >    WIN;

  //  typedef XVSSD<XVImageScalar<int>, _EXSTEPPER_<XVImageScalar<int> > > SSD;
  // typedef XVSSD<XVImageRGB<XV_RGB>, _EXSTEPPER_<XVImageRGB<XV_RGB> > > SSD;
  typedef XVSSD<XVImageRGB<XV_RGB>,
                XVPyramidStepper<XVTransStepper<XVImageRGB<XV_RGB> > > > SSD ;

  // pick your opponent
   //VID * vid = new MPG(argv[1],1);
  //VID * vid = new DIG( DC_DEVICE_NAME, "i1V1" );
    VID * vid = new V4L2(DEVICE_NAME,"B2");
  // flush first image
  vid->next_frame_continuous();

  WIN win(vid->frame(0));
  win.map();

  SSD ssd;

  vid->next_frame_continuous();
  vid->next_frame_continuous();
  vid->next_frame_continuous();
  vid->next_frame_continuous();
  vid->next_frame_continuous();
  win.CopySubImage(vid->current_frame());
  win.swap_buffers();
  win.flush();


  XVROI roi( 32,32 );
  win.selectRectangle(roi);
  XVImageRGB<XV_RGB> tmpl ;
  XVImageRGB<XV_RGB> tmpl1;
  // XVImageScalar<float> ftmpl;
  tmpl = subimage(vid->current_frame(), roi);

  cout << "Got the image" << endl;
  // ssd.setInit(roi,tmpl);
  // XVTransStepper<XVImageRGB<XV_RGB> > SSDStepper(tmpl);

  ssd.setStepper(subimage(vid->current_frame(),roi));
  ssd.initState( (XVTransState)roi );


  XVDisplayTracker<VID, WIN, SSD > tracker(*vid, win, ssd, 80);
  // tracker.init();


  // The SE2 (translation, rotation, and scale) SSD tracker
  // is intialized using the selectAngledRect function
  // which first expects a line to be selected by dragging the
  // mouse and releasing, and then selecting a rectangle
  // with the line as one side.  The box should be selected
  // in the downward direction from the line

  SSD::STATE st;// = tracker.init();
  XVTransState st1(50,50);// = tracker.init();

  XVWindowX<XV_RGB> warpWin(roi.Width(),roi.Height(), 400, 10);
  //  XVWindowX<XV_RGB> warpWinf(roi, 400, 10);

  warpWin.map();
  //  warpWinf.map();
  try {

    // can use simple track call, or complicated while loop

    // tracker.track();

    while(1){

      //      vid->next_frame_continuous();
      tmpl = subimage(vid->current_frame(), roi);
      for (int iii=1;iii<2;iii++) {
	st = tracker.nextState();
      ScalartoRGB(ssd.warpedImage(),tmpl1);
      warpWin.CopySubImage(tmpl1);
            warpWin.swap_buffers();
            warpWin.flush();
      }

      //SSDStepper.step(XVImageScalar<float>(tmpl),st1);

      // for a translation state
        // cout << st.state << "\t" << st.error << endl;

      // SE2 (rotation, translation, scale)
      /*      cout << st.state.angle << "\t"
	   << st.state.scale << "\t"
	   << st.state.trans.PosX() << "\t"
	   << st.state.trans.PosY() << "\t"
	   << st.error << endl;*/

      // displays the warped SSD image

	    //      RGBtoScalar(tmpl,ftmpl);
	    ScalartoRGB((XVImageScalar<float>)tmpl,tmpl1);
	    //      warpWinf.CopySubImage(tmpl1);
    }
  }catch(XVException te){ cout << te.what() << endl; };
};


