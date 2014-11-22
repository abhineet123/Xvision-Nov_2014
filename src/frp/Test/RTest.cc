# include "Scout.h"
# include "Image.h"
# include "Event.h"
# include "Track.h"

# include <XVDig1394.h>
# include <XVOmniWarper.h>
# include <XVTracker.h>
# include <XVColorSeg.h>
# include <XVBlobFeature.h>
# include <math.h>

# ifndef PI
# define PI 3.14159265358979323846
# endif


using namespace frp ;

XVPosition center( const XVBlobState& r ) {
  return XVPosition( r.state.PosX()+r.state.Width()/2, 
		     r.state.PosY()+r.state.Height()/2 );
}

struct DrawPos : public XVPosition {
  DrawPos() {}
  DrawPos( int x, int y ) : XVPosition(x,y) {}
  DrawPos( const XVPosition& x ) : XVPosition(x) {}
  void show( XVDrawable& d ) const 
    { d.drawLine( XVOmniWarper::defCenterX, XVOmniWarper::defCenterY, 
		  PosX(), PosY(), "green" ) ;  }
};

ScoutVel go_for( const XVPosition& obj, const XVPosition& dest ) {
  const double turn_speed = 15 ;
  const double turn_scale = -10 ;
  const double straight_scale = 0.1 ;
  const ScoutVel left_turn(-1,1) ;
  const ScoutVel right_turn(1,-1) ;
  const ScoutVel go_straight(1,1) ;

  int c_x = XVOmniWarper::defCenterX ;
  int c_y = XVOmniWarper::defCenterY ;
  int o_x = obj.PosX() ;
  int o_y = obj.PosY() ;
  int d_x = dest.PosX() ;
  int d_y = dest.PosY() ;

  o_x -= c_x ; o_y -= c_y ;
  d_x -= c_x ; d_y -= c_y ;
  double o_r = sqrt( o_x*o_x + o_y*o_y );
  double d_r = sqrt( d_x*d_x + d_y*d_y );
  double o_t = atan2( o_y, o_x );
  double d_t = atan2( d_y, d_x );
  
  o_t -= d_t ;
  o_r -= d_r ;
  if( o_t < -PI ) o_t += 2*PI ;
  if( o_t > PI ) o_t += 2*PI ;
  if( o_t <= -PI/2 )  { // at the right rear, turn to right first
    return right_turn * turn_speed ;
  }else if( o_t >= PI/2 ) {  // at the left rear, turn to left first
    return left_turn * turn_speed ;
  }else {  // turn while go straight
    return left_turn * ( o_t * turn_scale ) 
         + go_straight * ( o_r * straight_scale ) ;
  }
}

int main() {
  XVDig1394<XVImageRGB<XV_RGB> > dig(DC_DEVICE_NAME,"S1R1");

  theScoutRobot.init();

  Behavior<XVImageRGB<XV_RGB> > source ;
  WindowDisplay<XV_RGB> display;

  XVHueRangeSeg<XV_RGB, u_short> seg;
  typedef XVBlobFeature<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > Blob ;
  Blob blob(seg) ;
  Tracker<Blob> feature(blob);

  XVBlobState start ;
  Behavior<XVBlobState> position ;
  Behavior<bool> lost ;

  Behavior<XVPosition> target( XVPosition(420,XVOmniWarper::defCenterY) );

  source = video(dig) ;
  start = feature.interactiveInit(source); 
  position = feature( source, delayB(start) <<= position ) ;
  lost = ( lift(&XVBlobState::error) <<= position ) <= constB(0.9) ;
  (display << feature << cast<DrawPos,XVPosition>(target) <<= source , 
    scoutDrive <<=  lift(go_for)( lift(center) <<= position, target )
              TillB  whenE( lost ) ThenConstB scoutStop )->run();

  return 0 ;
}

// int main() { return 0 ;}
