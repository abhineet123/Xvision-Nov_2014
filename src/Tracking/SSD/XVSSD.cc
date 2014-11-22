// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVSSD.cc
 *
 * @author Gregory D. Hager, Sam Lang
 * @version $Id: XVSSD.cc,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 */

#include "XVSSD.h"
#include "XVImageIterator.h"

#define _XVSE2STATE_OP_EQ_(_OP_) \
XVSE2State & XVSE2State::operator _OP_ (const XVSE2State & addon) { \
  this->trans _OP_ addon.trans; \
  this->angle _OP_ addon.angle; \
  this->scale _OP_ addon.scale; \
  return *this; \
};

_XVSE2STATE_OP_EQ_(+=);
_XVSE2STATE_OP_EQ_(-=);


#define _XVSE2STATE_OP_(_OP_) \
XVSE2State operator _OP_ (const XVSE2State & s1, const XVSE2State & s2) { \
\
  XVSE2State res; \
  res.trans = s1.trans _OP_ s2.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  res.scale = s1.scale _OP_ s2.scale; \
  return res; \
};

_XVSE2STATE_OP_(+);
_XVSE2STATE_OP_(-);

#define _XVRTSTATE_OP_EQ_(_OP_) \
XVRTState & XVRTState::operator _OP_ (const XVRTState & addon) { \
  this->trans _OP_ addon.trans; \
  this->angle _OP_ addon.angle; \
  return *this; \
};

_XVRTSTATE_OP_EQ_(+=);
_XVRTSTATE_OP_EQ_(-=);

#define _XVRTSTATE_OP_(_OP_) \
XVRTState operator _OP_ (const XVRTState & s1, const XVRTState & s2) { \
\
  XVRTState res; \
  res.trans = s1.trans _OP_ s2.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  return res; \
};

_XVRTSTATE_OP_(+);
_XVRTSTATE_OP_(-);


#define _XVRotateSTATE_OP_EQ_(_OP_) \
XVRotateState & XVRotateState::operator _OP_ (const XVRotateState & addon) { \
  this->angle _OP_ addon.angle; \
  return *this; \
};

_XVRotateSTATE_OP_EQ_(+=);
_XVRotateSTATE_OP_EQ_(-=);

#define _XVRotateSTATE_OP_(_OP_) \
XVRotateState operator _OP_ (const XVRotateState & s1, const XVRotateState & s2) { \
\
  XVRotateState res; \
  res.trans = s1.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  return res; \
};

_XVRotateSTATE_OP_(+);
_XVRotateSTATE_OP_(-);


#define _XVAFFINESTATE_OP_EQ_(_OP_) \
XVAffineState & XVAffineState::operator _OP_ (const XVAffineState & addon){ \
  this->trans _OP_ addon.trans;  this->angle _OP_ addon.angle; \
  this->scale _OP_ addon.scale;  this->sheer _OP_ addon.sheer; \
  return *this; \
}

_XVAFFINESTATE_OP_EQ_(+=);
_XVAFFINESTATE_OP_EQ_(-=);

#define _XVAFFINESTATE_OP_(_OP_) \
XVAffineState operator _OP_ (const XVAffineState & s1, const XVAffineState & s2){ \
  XVAffineState res; \
  res.trans = s1.trans _OP_ s2.trans; \
  res.angle = s1.angle _OP_ s2.angle; \
  res.scale = s1.scale _OP_ s2.scale; \
  res.sheer = s1.sheer _OP_ s2.sheer; \
  return res; \
};

_XVAFFINESTATE_OP_(+);
_XVAFFINESTATE_OP_(-);

template <class IM_TYPE, class ST_TYPE>
XVSSDStepper<IM_TYPE, ST_TYPE >::XVSSDStepper(const IM_TYPE & itemplate_in ) {

  XVImageScalar<float> template_in;
  RGBtoScalar(itemplate_in,template_in);
  size = (XVSize)template_in;

  Dx = XVBoxFilterY(XVPrewittFilterX(template_in, (float)0), 
		    3, (float)6);
  Dy = XVBoxFilterX(XVPrewittFilterY(template_in, (float)0), 
		    3, (float)6);

  target = XVBoxFilter(template_in - template_in.avg(), 3, 3);
  forward_model.resize(target.Width()*target.Height(),0);

  X = XVImageScalar<float>(target.Width(), target.Height());
  Y = XVImageScalar<float>(target.Width(), target.Height());

  XVImageWIterator<float > Xiter(X);
  XVImageWIterator<float > Yiter(Y);

  for(; !Xiter.end(); ++Xiter, ++Yiter){
      
      *Xiter = (float)Xiter.currposx();
      *Yiter = (float)Yiter.currposy();
  }    
};

template <class IM_TYPE>
void XVTransStepper<IM_TYPE >::offlineInit(){

  forward_model = add_column(forward_model, target);
  forward_model = add_column(forward_model, Dx);
  forward_model = add_column(forward_model, Dy);
  inverse_model = ((forward_model.t() * forward_model).i()) * (forward_model.t());
};

template <class IM_TYPE>
typename XVTransStepper<IM_TYPE >::ResultPair
XVTransStepper<IM_TYPE >::step(const XVImageScalar<float>      & live_image,
			       const XVTransState & old_state) {

  XVColVector result(3);
  double error ;

  result[0]  = result[1] = result[2] = 0;
  diff_intensity << (XVBoxFilter(live_image-live_image.avg(), 3, 3) - target);

  result =  inverse_model * diff_intensity;

  XVTransState deltaMu(-result[1],-result[2]);
  error = (diff_intensity - (forward_model * result)).ip()/diff_intensity.n_of_rows();

  return ResultPair(deltaMu, error);
};

template <class IM_TYPE>
XVImageScalar<float> XVTransStepper<IM_TYPE >::warp(const IM_TYPE      & image, 
				       const XVTransState & state) { 
  XVImageScalar<float> tmp;
  IM_TYPE warped_image=warpRect(image, (XVPosition)state, size, 0);
 
  RGBtoScalar(warped_image, tmp);
  return tmp; 
};

template <class IM_TYPE>
void XVSE2Stepper<IM_TYPE >::offlineInit(){

  forward_model = add_column(forward_model, Dx);
  forward_model = add_column(forward_model, Dy);

  XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);
  XVImageScalar<float> XDxYDy = (X * Dx) + (Y * Dy);

  forward_model = add_column(forward_model, XDyYDx);
  forward_model = add_column(forward_model, XDxYDy);

  forward_model = add_column(forward_model, target);

  inverse_model = (((forward_model.t()*forward_model).i())*forward_model.t());
};

static void
check_brightness(XVImageScalar<float> &im)
{
  XVImageWIterator<float> iter(im);
  for(;!iter.end();++iter)
  {
    if(*iter>200) *iter=0;
  }
}

#define OUTLIER_THRESH 35
static void
check_outliers(XVColVector &diff_vec,double & err)
{
  int         turnoffs=0;

  for(int i=0;i<diff_vec.n_of_rows();i++)
  {
    if(fabs(diff_vec[i])>OUTLIER_THRESH)
    {
       diff_vec[i]=OUTLIER_THRESH/fabs(diff_vec[i])*diff_vec[i];
       turnoffs++;
    }
  }
  err=(double)turnoffs/diff_vec.n_of_rows();
}

template <class IM_TYPE>
XVStatePair<XVSE2State, double>
XVSE2Stepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
			     const XVSE2State & oldState){

  XVColVector result(5);
  XVMatrix sigma(5, 5);
 double error=0.0;

  diff_intensity << (XVBoxFilter(live_image-live_image.avg(), 3, 3) - target);
  check_outliers(diff_intensity,error);

  XVAffineMatrix rotMatrix(oldState.angle);
  sigma = 0;
  sigma[0][0] = rotMatrix(0, 0) / oldState.scale;
  sigma[1][0] = rotMatrix(1, 0);
  sigma[0][1] = rotMatrix(0, 1);
  sigma[1][1] = rotMatrix(1, 1) / oldState.scale;
  sigma[2][2] = 1;
  sigma[3][3] = 1 / oldState.scale;
  sigma[4][4] = 1;
  result = sigma.t() * (inverse_model * diff_intensity);

  XVSE2State deltaMu;
  deltaMu.trans = XV2Vec<double>(-result[0], -result[1]);
  deltaMu.angle = result[2];
  deltaMu.scale = result[3];

  XVStatePair<XVSE2State, double> ret(deltaMu, error);

  return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVSE2Stepper<IM_TYPE >::warp(const IM_TYPE    & image, 
				     const XVSE2State & state) {

  XVImageScalar<float> tmp;
  IM_TYPE warped_image=warpRect(image, (XVPosition)state.trans, size, 
	     state.angle, state.scale, state.scale, 0);
  RGBtoScalar(warped_image, tmp);

  return( tmp );
};

template <class IM_TYPE>
void XVRTStepper<IM_TYPE >::offlineInit(){
  forward_model = add_column(forward_model, Dx);
  forward_model = add_column(forward_model, Dy);

  XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);

  forward_model = add_column(forward_model, XDyYDx);
  forward_model = add_column(forward_model, target);
  inverse_model = (((forward_model.t()*forward_model).i())*forward_model.t());

};

template <class IM_TYPE>
XVStatePair<XVRTState, double>
XVRTStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
			     const XVRTState & oldState){

  XVColVector result(4);
  XVMatrix sigma(4, 4);
  double error=0.0;

  diff_intensity << (XVBoxFilter(live_image-live_image.avg(), 3, 3) - target);
  check_outliers(diff_intensity,error);

  XVAffineMatrix rotMatrix(oldState.angle);
  sigma = 0;
  sigma[0][0] = rotMatrix(0, 0); 
  sigma[1][0] = rotMatrix(1, 0);
  sigma[0][1] = rotMatrix(0, 1);
  sigma[1][1] = rotMatrix(1, 1); 
  sigma[2][2] = 1;
  sigma[3][3] = 1; 
  result = sigma.t() * (inverse_model * diff_intensity);

  XVRTState deltaMu;
  deltaMu.trans = XV2Vec<double>(-result[0], -result[1]);
  deltaMu.angle = result[2];

  XVStatePair<XVRTState, double> ret(deltaMu, error);

  return ret;

};

template <class IM_TYPE>
XVImageScalar<float> XVRTStepper<IM_TYPE >::warp(const IM_TYPE    & image, 
				     const XVRTState & state) {

  XVImageScalar<float> tmp;
  IM_TYPE warped_image=warpRect(image, (XVPosition)state.trans, size, 
	     state.angle); 
  RGBtoScalar(warped_image, tmp);

  return( tmp );
};


template <class IM_TYPE>
void XVRotateStepper<IM_TYPE >::offlineInit(){

  //copy from XVRT 
  forward_model = add_column(forward_model, Dx);
  forward_model = add_column(forward_model, Dy);
  XVImageScalar<float> XDyYDx = (X * Dy) - (Y * Dx);
  forward_model = add_column(forward_model, XDyYDx);
  forward_model = add_column(forward_model, target);
  inverse_model = (((forward_model.t()*forward_model).i())*forward_model.t());

};

template <class IM_TYPE>
XVStatePair<XVRotateState, double>
XVRotateStepper<IM_TYPE >::step(const XVImageScalar<float>    & live_image,
			     const XVRotateState & oldState){

  XVColVector result(4);
  XVMatrix sigma(4, 4);
  double error=0.0;

  diff_intensity << (XVBoxFilter(live_image-live_image.avg(), 3, 3) - target);
  check_outliers(diff_intensity,error);

  XVAffineMatrix rotMatrix(oldState.angle);
  sigma = 0;
  sigma[0][0] = rotMatrix(0, 0); 
  sigma[1][0] = rotMatrix(1, 0);
  sigma[0][1] = rotMatrix(0, 1);
  sigma[1][1] = rotMatrix(1, 1); 
  sigma[2][2] = 1;
  sigma[3][3] = 1; 
  result = sigma.t() * (inverse_model * diff_intensity);

  XVRotateState deltaMu;
  deltaMu.trans = XV2Vec<double>(-result[0], -result[1]);
  deltaMu.angle = result[2];

  XVStatePair<XVRotateState, double> ret(deltaMu, error);

  return ret;
};

template <class IM_TYPE>
XVImageScalar<float> XVRotateStepper<IM_TYPE >::warp(const IM_TYPE    & image, 
				     const XVRotateState & state) {

  XVImageScalar<float> tmp;
  IM_TYPE warped_image=warpRect(image, (XVPosition)state.trans, size, 
	     state.angle); 
  RGBtoScalar(warped_image, tmp);

  return( tmp );

};


template <class STEPPER_TYPE>
XVImageScalar<float> XVPyramidStepper<STEPPER_TYPE>::upperLayer
( const XVImageScalar<float>& lower ) {
  // need a faster version of convolve-and-resample here -- Donald
  const int ksize = 5 ;
  static float kernel[ksize] = { 0.0614, 0.2448, 0.3877, 0.2448, 0.0614 } ;
  static XVImageScalar<float> 
    kernelX( ksize, 1, new XVPixmap<float>( ksize, 1, kernel, false ) ),
    kernelY( 1, ksize, new XVPixmap<float>( 1, ksize, kernel, false ) ) ;

  XVImageScalar<float> temp1, mid, temp2, upper ;

  //convolve( lower, temp1, kernelX, 0 );
  XVBoxFilterX( lower, 3, 3.0f, temp1 );
  resample( scale_factor, 1, temp1, mid );
  //convolve( mid, temp2, kernelY, 0 );
  XVBoxFilterY( mid, 3, 3.0f, temp2 );
  resample( 1, scale_factor, temp2, upper );

  return upper ;
}

template <class STEPPER_TYPE>
XVPyramidStepper<STEPPER_TYPE>::XVPyramidStepper
( const typename STEPPER_TYPE::IMAGE_TYPE& template_in, double scale,
  int levels ) {
  // since the constructor of a stepper asks for a template image of IM_TYPE
  // instead of that of XVImageScalar<float> (and do the conversion internally)
  // here we have to implicitly convert to IM_TYPE needlessly
  scale_factor = scale ;
  steppers.push_back( STEPPER_TYPE(template_in) );
  XVImageScalar<float> temp ;
  temp = (XVImageScalar<float>) template_in ;
  for( levels -- ; levels > 0 ; levels -- ) {
    temp = upperLayer( temp );
    steppers.push_back( STEPPER_TYPE( (IM_TYPE)temp) );
  }
  savedImage.resize(1,1); // let it alloc a dummy pixmap 
}

template <class STEPPER_TYPE>
void XVPyramidStepper<STEPPER_TYPE>::offlineInit() {
  for( int i = 0 ; i < steppers.size() ; i ++ ) {
    steppers[i].offlineInit() ;
  }
}

namespace {

template<class ST_TYPE> void scaleUp( ST_TYPE& state, double x ) {
  state *= x ;
}

template<> void scaleUp( XVRotateState& state, double x ) {
  state.trans*= x ;
}

template<> void scaleUp( XVRTState& state, double x ) {
  state.trans *= x ;
}

template<> void scaleUp( XVSE2State& state, double x ) {
  state.trans *= x ;
}


template<> void scaleUp( XVAffineState& state, double x ) {
  state.trans *= x ;
}

}

template <class STEPPER_TYPE>
XVStatePair<typename STEPPER_TYPE::STATE_TYPE,double> 
XVPyramidStepper<STEPPER_TYPE>::step
( const XVImageScalar<float>& live_image, 
  const typename STEPPER_TYPE::STATE_TYPE& old_state ) {
  // a hack here -- assuming the same image as saved in warp()
  int i, j ;
  ST_TYPE new_state = old_state ;
  ResultPair delta_state ;
  XVImageScalar<float> currentImage = live_image ;

  for( i = steppers.size()-1 ; i >= 0 ; i -- ) {
    if( i != steppers.size()-1 ) {
      currentImage = steppers[0].warp( savedImage, new_state );
    }
    for( j = 0 ; j < i ; j ++ ) {
      currentImage = upperLayer( currentImage );
    }

    delta_state = steppers[i].step( currentImage, new_state );

    for( j = 0 ; j < i ; j ++ ) {
      scaleUp( delta_state.state, 1/scale_factor );
    }
    new_state += delta_state.state ;
  }
  delta_state.state = new_state - old_state ;
  return delta_state ;
}

template <class STEPPER_TYPE>
XVImageScalar<float> XVPyramidStepper<STEPPER_TYPE>::warp
( const typename STEPPER_TYPE::IMAGE_TYPE& image,
  const typename STEPPER_TYPE::STATE_TYPE& state ) {
  savedImage = image ; // a hack -- see step()
  return steppers[0].warp( image, state );
}


template <class IM_TYPE, class STEPPER_TYPE>
XVImageScalar<float>
XVSSD<IM_TYPE, STEPPER_TYPE>::warp(const IM_TYPE & image_in){
  warped = Stepper.warp(image_in, currentState.state);
  warpUpdated = true;
  return warped;
};

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::step(const IM_TYPE & image_in){

  if(!warpUpdated) this->warp(image_in);
  prevState = currentState;

  typename STEPPER_TYPE::ResultPair p;

  p = Stepper.step(warped, currentState.state);
  currentState.state = currentState.state + p.state;
  currentState.error = p.error;

  warpUpdated = false;
  return currentState;
};

namespace {

template <class ST_TYPE> 
struct XVSSDHelper {
};

template <>
struct XVSSDHelper<XVTransState> {
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, const IM_TYPE& im ) {
    XVROI roi;
    win.selectRectangle(roi);
    IM_TYPE tmpl = subimage(im, roi);
    Stepper = STEPPER_TYPE(tmpl);
    Stepper.offlineInit();
    XV2Vec<double> center;
    center.setX(roi.Width()  );
    center.setY(roi.Height() );
    currentState.state = (XV2Vec<double>)roi + center; 
    currentState.error = 0.0;
  }
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
    XVROI roi;
    win.selectSizedRect(roi,size);
    IM_TYPE tmpl = subimage(im, roi);
    Stepper = STEPPER_TYPE(tmpl);
    Stepper.offlineInit();
    XV2Vec<double> center;
    center.setX(roi.Width()/2  );
    center.setY(roi.Height()/2 );
    currentState.state = (XV2Vec<double>)roi + center; 
    currentState.error = 0.0;
  }
  template<class STEPPER_TYPE>
#ifndef NOVIS
  static void show
   ( XVStatePair<XVTransState,double>& currentState, STEPPER_TYPE& Stepper, 
     XVDrawable& x,float scale ) {

    XVPosition corners[4];
    XV2Vec<double> points[4];
    XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2, 
					     Stepper.getSize().Height() / 2);

    points[0] = - tmpPoint;
    points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
    points[2] = tmpPoint;
    points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

    for(int i=0; i<4; ++i)
      corners[i] = points[i] + currentState.state;

    for(int i=0;i<4;i++) corners[i].setX((int)(corners[i].x()/scale)),
    			 corners[i].setY((int)(corners[i].y()/scale));

    x.drawLine(corners[0], corners[1]);
    x.drawLine(corners[1], corners[2]);
    x.drawLine(corners[2], corners[3]);
    x.drawLine(corners[3], corners[0]);
  }
};
#endif

template<>
struct XVSSDHelper<XVSE2State> {
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, const IM_TYPE& im ) {
    XVPosition initULC;
    XVSize initSize;
    double initAngle;
    win.selectAngledRect(initULC, initSize, initAngle);
    XVAffineMatrix rotMat(initAngle);
    XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
							initSize.Height() / 2);
    currentState.state.trans = (initCenter + initULC);
    currentState.state.angle = -initAngle;
    currentState.state.scale = 1;
    Stepper = STEPPER_TYPE(warpRect(im,
				    currentState.state.trans,
				    initSize,
				    currentState.state.angle,
				    currentState.state.scale,
				    currentState.state.scale,
				    0));
    Stepper.offlineInit();
    currentState.error = 0.0;
  }
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
    cerr << "not yet defined" << endl;
    exit(-1);
  }
  template<class STEPPER_TYPE>
  static void show
   ( XVStatePair<XVSE2State,double>& currentState, STEPPER_TYPE& Stepper, 
     XVDrawable& x, float scale ) {
    XVPosition corners[4];
    XVAffineMatrix angleMat(-currentState.state.angle);
    XVAffineMatrix scaleMat( 1 / currentState.state.scale, 
			     1 / currentState.state.scale);
    XVAffineMatrix tformMat((XVMatrix)scaleMat * (XVMatrix)angleMat);

    XV2Vec<double> points[4];
    XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2, 
					     Stepper.getSize().Height() / 2);
    points[0] = - tmpPoint;
    points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
    points[2] = tmpPoint;
    points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

    corners[0] = (tformMat * points[0]) + currentState.state.trans;
    corners[1] = (tformMat * points[1]) + currentState.state.trans;
    corners[2] = (tformMat * points[2]) + currentState.state.trans;
    corners[3] = (tformMat * points[3]) + currentState.state.trans;

    for(int i=0;i<4;i++) corners[i].setX((int)(corners[i].x()/scale)),
    			 corners[i].setY((int)(corners[i].y()/scale));
    x.drawLine(corners[0], corners[1]);
    x.drawLine(corners[1], corners[2]);
    x.drawLine(corners[2], corners[3]);
    x.drawLine(corners[3], corners[0]);
  }
};

//Helper implementation for RT
template<>
struct XVSSDHelper<XVRTState> {
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, const IM_TYPE& im ) {
    XVPosition initULC;
    XVSize initSize;
    double initAngle;
    win.selectAngledRect(initULC, initSize, initAngle);
    XVAffineMatrix rotMat(initAngle);
    XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
							initSize.Height() / 2);
    currentState.state.trans = (initCenter + initULC);
    currentState.state.angle = -initAngle;
    Stepper = STEPPER_TYPE(warpRect(im,
				    currentState.state.trans,
				    initSize,
				    currentState.state.angle) );
    Stepper.offlineInit();
    currentState.error = 0.0;
  }
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
    cerr << "not yet defined" << endl;
    exit(-1);
  }
  template<class STEPPER_TYPE>
  static void show
   ( XVStatePair<XVRTState,double>& currentState, STEPPER_TYPE& Stepper, 
     XVDrawable& x, float scale ) {
    XVPosition corners[4];
    XVAffineMatrix angleMat(-currentState.state.angle);
    XVAffineMatrix tformMat((XVMatrix)angleMat);

    XV2Vec<double> points[4];
    XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2, 
					     Stepper.getSize().Height() / 2);
    points[0] = - tmpPoint;
    points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
    points[2] = tmpPoint;
    points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

    corners[0] = (tformMat * points[0]) + currentState.state.trans;
    corners[1] = (tformMat * points[1]) + currentState.state.trans;
    corners[2] = (tformMat * points[2]) + currentState.state.trans;
    corners[3] = (tformMat * points[3]) + currentState.state.trans;

    for(int i=0;i<4;i++) corners[i].setX((int)(corners[i].x()/scale)),
    			 corners[i].setY((int)(corners[i].y()/scale));
    x.drawLine(corners[0], corners[1]);
    x.drawLine(corners[1], corners[2]);
    x.drawLine(corners[2], corners[3]);
    x.drawLine(corners[3], corners[0]);
  }
};

template<>
struct XVSSDHelper<XVRotateState> {
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, const IM_TYPE& im ) {
    XVPosition initULC;
    XVSize initSize;
    double initAngle;
    win.selectAngledRect(initULC, initSize, initAngle);
    XVAffineMatrix rotMat(initAngle);
    XV2Vec<double> initCenter = rotMat * XV2Vec<double>(initSize.Width() / 2,
							initSize.Height() / 2);
    currentState.state.trans= (initCenter + initULC);
    currentState.state.angle= -initAngle;
    Stepper = STEPPER_TYPE( warpRect(im, 
              currentState.state.trans,
              initSize, 
              currentState.state.angle) );
    Stepper.offlineInit();
    currentState.error = 0.0;
  }
  template<class IM_TYPE, class STEPPER_TYPE>
  static void interactiveInit
   ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper,
     XVInteractive& win, XVSize& size, const IM_TYPE& im ) {
    cerr << "not yet defined" << endl;
    exit(-1);
  }
  template<class STEPPER_TYPE>
  static void show
   ( XVStatePair<XVRotateState,double>& currentState, STEPPER_TYPE& Stepper, 
     XVDrawable& x, float scale ) {
 
    XVPosition corners[4];
    XVAffineMatrix angleMat(-currentState.state.angle);
    XVAffineMatrix tformMat((XVMatrix)angleMat);

    XV2Vec<double> points[4];
    XV2Vec<double> tmpPoint = XV2Vec<double>(Stepper.getSize().Width() / 2, 
					     Stepper.getSize().Height() / 2);
    points[0] = - tmpPoint;
    points[1] = XV2Vec<double>(tmpPoint.PosX(), - tmpPoint.PosY());
    points[2] = tmpPoint;
    points[3] = XV2Vec<double>(- tmpPoint.PosX(), tmpPoint.PosY());

    corners[0] = (tformMat * points[0]) + currentState.state.trans;
    corners[1] = (tformMat * points[1]) + currentState.state.trans;
    corners[2] = (tformMat * points[2]) + currentState.state.trans;
    corners[3] = (tformMat * points[3]) + currentState.state.trans;

    for(int i=0;i<4;i++) corners[i].setX((int)(corners[i].x()/scale)),
    			 corners[i].setY((int)(corners[i].y()/scale));
    x.drawLine(corners[0], corners[1]);
    x.drawLine(corners[1], corners[2]);
    x.drawLine(corners[2], corners[3]);
    x.drawLine(corners[3], corners[0]);

  }
};

}

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::interactiveInit
  ( XVInteractive & win, const IM_TYPE & im ) {
  XVSSDHelper<STATE_TYPE>::interactiveInit( currentState, Stepper, win, im );
  return prevState = currentState ;
}

template <class IM_TYPE, class STEPPER_TYPE>
const XVStatePair<typename STEPPER_TYPE::STATE_TYPE, double> &
XVSSD<IM_TYPE, STEPPER_TYPE>::interactiveInit
  ( XVInteractive & win, XVSize& size, const IM_TYPE & im ) {
  XVSSDHelper<STATE_TYPE>::interactiveInit( currentState, Stepper,win,size,im);
  return prevState = currentState ;
}

template <class IM_TYPE, class STEPPER_TYPE>
void XVSSD<IM_TYPE, STEPPER_TYPE>::show( XVDrawable& x ,float scale) {
  XVSSDHelper<STATE_TYPE>::show( currentState, Stepper, x,scale );
}
template<class IM_TYPE, class STEPPER_TYPE>
void XVSSD<IM_TYPE,STEPPER_TYPE>::setStepper( 
         const STEPPER_TYPE& stepper_in){
  Stepper = stepper_in ;
  Stepper.offlineInit();
}

#include <XVFeature.h>

#define _REGISTER_STEPPERS_(_IM_TYPE_) \
template class XVTransStepper<_IM_TYPE_ >; \
template class XVRotateStepper<_IM_TYPE_ >; \
template class XVRTStepper<_IM_TYPE_ >; \
template class XVSE2Stepper<_IM_TYPE_ >; 

template class XVPyramidStepper<XVTransStepper<XVImageRGB<XV_RGB24> > >; 
template class XVPyramidStepper<XVTransStepper<XVImageRGB<XV_RGBA32> > >; 
template class XVPyramidStepper<XVRTStepper<XVImageRGB<XV_RGB24> > >; 
template class XVPyramidStepper<XVRTStepper<XVImageRGB<XV_RGBA32> > >; 
template class XVPyramidStepper<XVSE2Stepper<XVImageRGB<XV_RGB24> > >;
template class XVPyramidStepper<XVSE2Stepper<XVImageRGB<XV_RGBA32> > >;

//_REGISTER_STEPPERS_(XVImageScalar<int>);
//_REGISTER_STEPPERS_(XVImageScalar<float>);
//_REGISTER_STEPPERS_(XVImageScalar<double>);

//_REGISTER_STEPPERS_(XVImageScalar<unsigned char>);
_REGISTER_STEPPERS_(XVImageRGB<XV_RGB24>);
_REGISTER_STEPPERS_(XVImageRGB<XV_RGBA32>);

#define _SSD_PAIR_(_ST_TYPE_) XVStatePair<_ST_TYPE_, double>

#include <XVDrawable.h>

#define _REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, _IMAGE_TYPE_) \
template class XVSSD<_IMAGE_TYPE_, _STEPPER_TYPE_<_IMAGE_TYPE_ > >; \
template class XVSSD<_IMAGE_TYPE_, XVPyramidStepper<_STEPPER_TYPE_<_IMAGE_TYPE_ > > >;

/*
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<int>); \
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<float>); \
//_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageScalar<double>); \
*/

#define _REGISTER_XVSSD_SCALAR_(_STEPPER_TYPE_, _STATE_TYPE_) \
_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageRGB<XV_RGB24>);\
_REGISTER_XVSSD_(_STEPPER_TYPE_, _STATE_TYPE_, XVImageRGB<XV_RGBA32>);

_REGISTER_XVSSD_SCALAR_(XVTransStepper, XVTransState);
_REGISTER_XVSSD_SCALAR_(XVRotateStepper,    XVRotateState);
_REGISTER_XVSSD_SCALAR_(XVRTStepper,    XVRTState);
_REGISTER_XVSSD_SCALAR_(XVSE2Stepper,   XVSE2State);

