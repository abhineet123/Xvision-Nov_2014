#include <config.h>
#include <XVMpeg.h>
#include <XVAVI.h>
#include <XVVideo.h>
#include <XVDig1394.h>
#include <XVEdgeFeature.h>
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

int main(int argc, char * argv[]) {

    typedef XVVideo<XVImageRGB<XV_RGB> > VID;
    typedef XVAVI<XVImageRGB<XV_RGB> > AVI_dt;
    typedef XVMpeg<XVImageRGB<XV_RGB> >  MPG;
    typedef XVInteractWindowX<XV_RGB>    WIN;
    typedef XVV4L2<XVImageRGB<XV_RGB> > V4L2;

    typedef XVEdgeFeature<int, XVEdge<int> > EDGE;

    //VID * vid = new MPG(argv[1]);
    VID * vid = new AVI_dt(argv[1]);
    //VID * vid = new V4L2(DEVICE_NAME,"B2");

    WIN win(vid->frame(0), 0, 400);
    win.map();

    XVEdge<int> edge;

    EDGE feat(edge, 50);

    XVDisplayTracker<VID, WIN, EDGE > tracker(*vid, win, feat);

    XVLineState bs = tracker.init();

    XVWindowX<XV_RGB> win2(vid->frame(0), 600, 0);
    win2.map();

    while(1) {
        try {
            tracker.nextState();
            XV2Vec<double> ends[2];
            feat.getCurrentState().state.endpoints(ends);
            bs = feat.diffState();
            XV2Vec<double> diffTop = ends[1] - ends[0];
            double val = (bs.state.center * diffTop) / diffTop.length();
            cout << val << endl;


//        cout << bs.state.center.PosX() << " : "
//  	   << bs.state.center.PosY() << " : "
//  	   << bs.state.angle << " : "
//  	   << bs.error << endl;


            win2.CopySubImage((XVImageRGB<XV_RGB>)(feat.warpedImage()));
            win2.swap_buffers();
            win2.flush();

        } catch(XVTrackerException te) {
            cout << te.what() << endl;
        };
    }
    delete vid;
};
