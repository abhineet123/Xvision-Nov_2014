//# include <stdio.h>
# include "Behavior.h"
# include "Event.h"
# include "Task.h"
# include "Image.h"
# include "Track.h"
# include "Scout.h"
# include "XVDig1394.h"
# include "XVColorSeg.h"
# include "XVSSD.h"
# include "XVBlobFeature.h"

# include <XVImageIO.h>

# include <stdio.h>
# include <iostream>

using namespace std ;
using namespace frp ;

// helper classes (to draw things on screen)

class DrawButtons {
 public:
  const static XVDrawColor color ;
  const static XVImageGeneric rect1, rect2 ;
 protected:
  bool button1, button2 ;
 public:
  DrawButtons() {} 
  DrawButtons( bool b1, bool b2 ) : button1(b1), button2(b2) {} ;
  void show( XVDrawable& d ) const {
    if( button1 ) {
      d.fillRectangle( rect1, color );
    }else {
      d.drawRectangle( rect1, color );
    }
    if( button2 ) {
      d.fillRectangle( rect2, color );
    }else {
      d.drawRectangle( rect2, color );
    }
  }
};
const XVDrawColor DrawButtons::color( "blue" );
const XVImageGeneric DrawButtons::rect1( 80, 40, 20, 10 );
const XVImageGeneric DrawButtons::rect2( 80, 40, 120, 10 );

class DrawRect : public XVImageGeneric {
public:
  const static XVDrawColor color ;

  DrawRect() {}
  DrawRect( const XVImageGeneric& image ) : XVImageGeneric(image) {}
  void show( XVDrawable& d ) const 
    { d.drawRectangle( *this, color ); }
};
const XVDrawColor DrawRect::color( "green" );

class DrawCross : public XVPosition {
public:
  const static XVDrawColor color ;
  const static int length ;

  DrawCross() {}
  DrawCross( const XVPosition& x ) : XVPosition(x) {}
  void show( XVDrawable& d ) const {
    d.drawLine( PosX()-length, PosY(), PosX()+length, PosY(), color ); 
    d.drawLine( PosX(), PosY()-length, PosX(), PosY()+length, color );
  }
};
const XVDrawColor DrawCross::color( "green" );
const int DrawCross::length = 8 ;

// helper function for push button

template<class T>
Behavior<bool> flip_flop( const Event<T>& e ) {
  Behavior<bool> b ;
  b = stepB(false) <<= snapshotE( e, ! ( delayB(false) <<= b ) );
  return b ;
}

// trackers

typedef XVTransStepper<XVImageRGB<XV_RGB> > SSDStepper ;
typedef XVTransState SSDState ;
typedef XVSSD<XVImageRGB<XV_RGB>, SSDStepper> SSDFeature ;

Tracker<SSDFeature> get_ssd_tracker( XVImageRGB<XV_RGB> x ) {
  SSDFeature ssd( SSDState( x.PosX(), x.PosY() ), x );
  Tracker<SSDFeature> tracker(ssd) ;
  return tracker ;
}

typedef XVBlobFeature<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > BlobFeature;

Tracker<BlobFeature> get_blob_tracker( XVImageGeneric x ) {
  // pointer! memory leak!! but no way because BlobFeature internally uses  
  // a polymorphic pointer to XVSegmentation.
  XVHueRangeSeg<XV_RGB,u_short> *seg = new XVHueRangeSeg<XV_RGB,u_short>;
  BlobFeature blob(*seg) ;
  Tracker<BlobFeature> tracker(blob) ;
  tracker->initState( XVBlobState( XVRectangleBlob(x), 0 ) );
  return tracker ;
}

// predictors

template<class State>
Behavior<State> null_predictor( Behavior<State> x ) {
  return x ;
}

template<class State>
Behavior<State> linear_predictor( Behavior<State> x, double k ) {
  return x + ( x - delay1B(x) ) * k ;
}

// main tracker algorithm

template<class Predictor, class Tracker>
Behavior<typename Tracker::State::STATE> 
track( Predictor predictor, Tracker tracker, 
       Behavior<XVImageRGB<XV_RGB> > image ) {
  typedef typename Tracker::State StatePair ;
  typedef typename StatePair::STATE State ;
  Behavior<StatePair> state ;
  Behavior<State> pos, pred_pos ;
  State init = tracker->getCurrentState().state ;
  state = tracker ( image, liftB( ctorF2<StatePair,State,double>() )
		                 ( pred_pos, constB<double>(0) ) );
  pos = liftB( &StatePair::state ) <<= state ;
  pred_pos = delayB(init) <<= predictor( pos ) ;
  return pos ;
}


// segmenter (bogus)

XVImageGeneric box_cut( const XVImageGeneric& container, 
			const XVImageGeneric& object ) {
  int left = object.PosX() ;
  int right = left + object.Width() ;
  int top = object.PosY() ;
  int bottom = top + object.Height() ;
  int left_box = container.PosX() ;
  int right_box = left_box + container.Width() ;
  int top_box = container.PosY() ;
  int bottom_box = top_box + container.Height() ;
  if( left < left_box ) left = left_box ;
  if( right > right_box ) right = right_box ;
  if( top < top_box ) top = top_box ;
  if( bottom > bottom_box ) bottom = bottom_box ;
  return XVImageGeneric( right-left, bottom-top, left, top );
}

XVImageRGB<XV_RGB> segment( XVImageRGB<XV_RGB> image, XVPosition point ) {
  const int sizeX = 8, sizeY = 8 ;
  XVImageScalar<u_short> result ;
  vector<XVRectangleBlob> regions ;
  XVImageGeneric neighborhood( sizeX, sizeY, 
			       point.PosX()-sizeX/2, point.PosY()-sizeY/2 )  ;
  return subimage( image, neighborhood ) ;
}

/* {
  XVHueRangeSeg<XV_RGB, u_short> seg;
  seg.update( subimage( image, box_cut( image, neighborhood ) ) );
  seg.segment( image, result );
  seg.regionGrow( result, regions, 0, 4 );
  vector<XVRectangleBlob>::iterator i ;
  cout << point.PosX() << " , " << point.PosY() << endl ;
  for( i = regions.begin() ; i != regions.end() ; i ++ ) {
    cout << i->PosX() << " , " << i->PosY() << " x "
         << i->Width() << " , " << i->Height() << endl; 
    if( i->contains( point ) ) break ; 
  }
  if( i != regions.end() ) {
    return *i ;
  }else {
    return image ;
  }
} */

// put them together

XVPosition trans2pos( SSDState x ) {
  return XVPosition( (int)(x.PosX()+.5), (int)(x.PosY()+.5) );
}
XVPosition blob2pos( XVRectangleBlob x ) {
  return XVPosition( (int)((x.PosX()+x.Width())/2+.5), 
		     (int)((x.PosY()+x.Height())/2+.5) );
}

BehaviorRef<XVPosition> ssd_track( Behavior<XVImageRGB<XV_RGB> > source,
				   XVPosition point ) {
  return liftB( trans2pos ) <<=
         track( liftF( &linear_predictor<SSDState> ).bind2(0.5), 
		get_ssd_tracker( segment( source.get(), point ) ),
		source );
}
BehaviorRef<XVPosition> blob_track( Behavior<XVImageRGB<XV_RGB> > source,
				    XVPosition point ) {
  return liftB( blob2pos ) <<=
         track( &null_predictor<XVRectangleBlob>, 
		get_blob_tracker( segment( source.get(), point ) ),
		source );
}

// robot stuff

TaskT<ScoutVel> fire( XVPosition x, BehaviorRef<bool> button1state ) {
  if( button1state->get() ) {
    return mkTask( constB(ScoutVel(10.0)), neverE<TaskT<ScoutVel> >() ) ;
  }else {
    return mkTask( constB(ScoutVel(20.0)), neverE<TaskT<ScoutVel> >() ) ;    
  }
}

// main

int main() {
  XVDig1394<XVImageRGB<XV_RGB> > dig(DC_DEVICE_NAME,"S1R1");
  WindowDisplay<XV_RGB> display ;
  XVImageRGB<XV_RGB> background_image ;
  XVReadImage( background_image, "xv.ppm" );
  Behavior<XVImageRGB<XV_RGB> > source = video(dig)/*background_image*/ ;
  Event<XVPosition> button, effective_button, ssd_button, blob_button ;
  Event<char> keypress ;
  button = display.lbp() ;
  keypress = display.key() ;
  theScoutRobot.init();

  Task<ScoutVel,TaskT<ScoutVel> > action, null ;
  Event<TaskT<ScoutVel> > tasknet ;
  Behavior<bool> button1state, button2state ;
  Event<void> stopKey, button1hit, button2hit ;
  Behavior<XVPosition> target ;

  null = mkTask( scoutStop, neverE<TaskT<ScoutVel> >() );
  action = mkTaskLoopCast( null, tasknet );
  tasknet = ( effective_button ThenB liftF(fire).bind2(button1state) )
         || ( stopKey ThenConstB (TaskT<ScoutVel>)null ) ;

  /* the following line needs more thought */
  effective_button = button && ! button1hit.getRef() && ! button2hit.getRef() ;

  stopKey = whileE( keypress == constE( ' ' ) ) ;
  button1hit = filterE( liftF( &XVImageGeneric::contains, DrawButtons::rect1) )
    <<= button ;
  button2hit = filterE( liftF( &XVImageGeneric::contains, DrawButtons::rect2) )
    <<= button ;

  button1state = flip_flop( button1hit );
  button2state = flip_flop( button2hit );
  
  // Here is the tracking stuff

  ssd_button = effective_button ;
  blob_button = display.rbp() ;

  target = switchB( constB(XVPosition( 0, 0 )),
              ( ssd_button ThenB liftF(ssd_track).bind1(source) )
           || ( blob_button ThenB liftF(blob_track).bind1(source) ) );

  ( display << liftB( ctorF2<DrawButtons,bool,bool>() ) 
               ( button1state, button2state ) 
            << liftB( ctorF1<DrawCross, XVPosition>() )
               ( target )
            <<= source, 
    scoutDrive <<= action.behavior() )->run();
  return 0 ;
}
