# include "Image.h"
# include "Track.h"

# include "XVDig1394.h"
//# include "Videre.h"
# include "XVBt8x8.h"
# include "XVColorSeg.h"
# include "XVBlobFeature.h"

namespace frp {

struct DrawPos : public XVPosition {
  DrawPos() {}
  DrawPos( int x, int y ) : XVPosition(x,y) {}
  DrawPos( const XVPosition& x ) : XVPosition(x) {}
  void show( XVDrawable& d ) const 
    { d.drawLine( 320, 240, 
		  PosX(), PosY(), "green" ) ;  }
};

struct Circle {
  int x, y, r;
  Circle() {}
  Circle( int x, int y , int r ) : x(x), y(y), r(r) {}
  void show( XVDrawable& d ) const
    { d.drawEllipse( x-r, y-r, 2*r, 2*r, "green" ); }
};

Circle makeCircle( XVPosition p ) {
  return Circle(p.PosX(),p.PosY(), 20 );
}

XVPosition operator * ( XVPosition x, double y ) {
  return XVPosition( (int)(x.PosX()*y), (int)(x.PosY()*y) );
}
XVPosition operator / ( XVPosition x, double y ) {
  return XVPosition( (int)(x.PosX()/y), (int)(x.PosY()/y) );
}

void clear( XVImageRGB<XV_RGB>& image ) {
  XVImageWIterator<XV_RGB> i(image) ;
  for( ; !i.end() ; ++i ) {
    i->setR(0);
    i->setG(0);
    i->setB(0);
  }
}

int main() {

  //XVDig1394<XVImageRGB<XV_RGB> > grabber(DC_DEVICE_NAME,"S1R1");
  //XVBt8x8<XVImageRGB<XV_RGB> > grabber;
  //XV_Videre<XVImageRGB<XV_RGB> > grabber ;
  //grabber.set_params ("B2I1N0");
  WindowDisplay<XV_RGB> display ;
  Behavior<XVImageRGB<XV_RGB> > background, grayground ;
  Behavior<Time> time, delay ;
  Event<XVPosition> button ;
  Behavior<XVPosition> current, last, target ;
  Behavior<bool> cancel ;
  //XVPosition original(320, 240);
  XVPosition original(160, 120);
  XVImageRGB<XV_RGB> back_image(320,240) ;
  clear( back_image );

  //background = video(grabber) ;
  background = constB( back_image );
  grayground = ScalartoRGB<int,XV_RGB>( RGBtoScalar<XV_RGB,int>( background ) ) ;
  delay = Time(1.0);
  time = timeB - stepTimeOfE(button) ;
  target = stepB(original) <<= button ;
  last = stepB(original) <<= snapshotE( button, current ) ;
  current = switch2B(
    (target*time+last*(delay-time))/delay, 
       (whileE(cancel) ThenConstB constB(original)) 
    || (whileE(time>=delay) ThenConstB target) );
  button = display.lbp() ;
  cancel = stepTimeOfE( display.rbp() ) > stepTimeOfE( display.lbp() ) ;

  ( display << (lift(makeCircle) <<= current) <<= grayground,
    liftS(cerr) << stepB('@')(display.key())  )->run();
  return 0 ;
}

int main_old() {
  XVHueRangeSeg<XV_RGB, u_short> seg;

  typedef XVBlobFeature<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > BLOB;
  BLOB feat(seg); //, true);  // for resampling
  LoopTracker<BLOB> trac(feat) ;

  XVDig1394<XVImageRGB<XV_RGB> > dig(DC_DEVICE_NAME,"S1R1");
  Behavior<DrawPos> pos( XVPosition( 100,100 ) ) ;
  Behavior<XVImageRGB<XV_RGB> > src ;
  src = video(dig);
  trac.interactiveInit( src );
  ( display<XV_RGB>() << pos << trac <<= src, trac <<= src )->run();
  return 0 ;
}

}

int main() {
  return frp::main() ;
}
