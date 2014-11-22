// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVSSD.h
 *
 * @author Greg Hager, Sam Lang
 * @version $Id: XVSSD.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 * 
 * XVSSD is a feature (XVFeature) object that uses
 * the sum of squared differences in intensity values
 * to predict motion and track any object with significant
 * intensity variation.
 *
 * For further information on SSD tracking, see:
 *
 * <i>Gregory D. Hager, Peter N. Belhumeur.</i>
 * <b>Efficient Region Tracking With Parametric 
 * Models of Geometry and Illumination.</b>
 * 
 * http://www.cs.jhu.edu/~hager/publications/
 *
 */

# ifndef _XVSSD_H_
# define _XVSSD_H_

# include <XVTools.h>
# include <XVMatrix.h>
# include <XVBlobs.h>
# include <XVFeature.h>   
# include <XVImageScalar.h>
# include <XVImageRGB.h>
# include <XVAffineWarp.h>

# include <vector>

typedef XVImageGeneric XVROI;

typedef XV2Vec<double> XVTransState;
typedef double         XVRotState;
typedef double         XVScaleState;
typedef double         XVSheerState;

class XVRotateState{
  public:
    XVRotState angle;
    XV2Vec<double> trans; 
    XVRotateState & operator += (const XVRotateState &);
    XVRotateState & operator -= (const XVRotateState &);
};
XVRotateState operator + (const XVRotateState &, const XVRotateState &);
XVRotateState operator - (const XVRotateState &, const XVRotateState &);


class XVRTState {

 public:
  XVTransState    trans;
  XVRotState      angle;
  XVRTState & operator += (const XVRTState &);
  XVRTState & operator -= (const XVRTState &);
};
XVRTState operator + (const XVRTState &, const XVRTState &);
XVRTState operator - (const XVRTState &, const XVRTState &);

class XVSE2State {

 public:

  XVTransState    trans;
  XVRotState      angle;
  XVScaleState    scale;

  XVSE2State & operator += (const XVSE2State &);
  XVSE2State & operator -= (const XVSE2State &);
};

XVSE2State operator + (const XVSE2State &, const XVSE2State &);
XVSE2State operator - (const XVSE2State &, const XVSE2State &);

class XVAffineState {

 public:

  XVTransState    trans;
  XVRotState      angle;
  XVScaleState    scale;
  XVSheerState    sheer;

  XVAffineState & operator += (const XVAffineState &);
  XVAffineState & operator -= (const XVAffineState &);
};

XVAffineState operator + (const XVAffineState &, const XVAffineState &);
XVAffineState operator - (const XVAffineState &, const XVAffineState &);


/**
 * XVSSDStepper
 *
 * @author Greg Hager, Sam Lang
 * @version $Id: XVSSD.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 * XVSSDStepper provides a class to abstract the
 * SSD computation step.  This allows for many
 * different types of steppers (just translation;
 * translation, 2D rotation, scale; any affine warping).
 * 
 * One of the essential components of the SSD stepper
 * is the initial offline computation, which allows 
 * the algorithm to be extremely efficient.
 */
template<class IM_TYPE, class ST_TYPE>
class XVSSDStepper {

 protected:

  XVImageScalar<float> target, X, Y, Dx, Dy;
  XVMatrix inverse_model;
  XVMatrix forward_model;
  XVColVector diff_intensity ;

  XVSize size;

 public:

  typedef ST_TYPE STATE_TYPE;
  typedef IM_TYPE IMAGE_TYPE;

  typedef typename IM_TYPE::PIXELTYPE PIXEL_TYPE;

  typedef XVStatePair<STATE_TYPE, double> ResultPair;

  XVSSDStepper(){}

  XVSSDStepper(const IM_TYPE & template_in );

  XVSSDStepper(const XVSSDStepper<IM_TYPE, ST_TYPE> & st) 
    : target(st.target), X(st.X), Y(st.Y), Dx(st.Dx), Dy(st.Dy),
      inverse_model(st.inverse_model), 
      forward_model(st.forward_model), 
      diff_intensity(st.diff_intensity),
      size(st.size) {}

  XVSize getSize() const { return size; };
  
  virtual void offlineInit() = 0;

  virtual ResultPair step( const XVImageScalar<float> &, const ST_TYPE &) = 0;

  virtual XVImageScalar<float> warp(const IM_TYPE &, const ST_TYPE &) = 0;  
};

template <class IM_TYPE>
class XVTransStepper : public XVSSDStepper<IM_TYPE, XVTransState > {
 protected:
  using XVSSDStepper<IM_TYPE,XVTransState>::target ;
  using XVSSDStepper<IM_TYPE,XVTransState>::X ;
  using XVSSDStepper<IM_TYPE,XVTransState>::Y ;
  using XVSSDStepper<IM_TYPE,XVTransState>::Dx ;
  using XVSSDStepper<IM_TYPE,XVTransState>::Dy ;
  using XVSSDStepper<IM_TYPE,XVTransState>::inverse_model ;
  using XVSSDStepper<IM_TYPE,XVTransState>::forward_model ;
  using XVSSDStepper<IM_TYPE,XVTransState>::diff_intensity ;
  using XVSSDStepper<IM_TYPE,XVTransState>::size ;

 public:

  typedef typename XVSSDStepper<IM_TYPE,XVTransState>::ResultPair ResultPair ;

  XVTransStepper() : XVSSDStepper<IM_TYPE, XVTransState >() {}
  XVTransStepper(const IM_TYPE & tmpl) 
    : XVSSDStepper<IM_TYPE, XVTransState >(tmpl) {}

  virtual void offlineInit();
  virtual ResultPair step(const XVImageScalar<float> &, const XVTransState &);
  virtual XVImageScalar<float> warp(const IM_TYPE &, const XVTransState &);
};

template <class IM_TYPE>
class XVSE2Stepper : public XVSSDStepper<IM_TYPE, XVSE2State > {
 protected:
  using XVSSDStepper<IM_TYPE,XVSE2State>::target ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::X ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::Y ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::Dx ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::Dy ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::inverse_model ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::forward_model ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::diff_intensity ;
  using XVSSDStepper<IM_TYPE,XVSE2State>::size ;

 public:
  
  XVSE2Stepper() : XVSSDStepper<IM_TYPE, XVSE2State >() {}
  XVSE2Stepper(const IM_TYPE & tmpl) 
    : XVSSDStepper<IM_TYPE, XVSE2State >(tmpl) {}

  virtual void offlineInit();
  virtual XVStatePair<XVSE2State, double> step(const XVImageScalar<float> &, 
					       const XVSE2State &);
  virtual XVImageScalar<float> warp(const IM_TYPE &, const XVSE2State &);
};

template <class IM_TYPE>
class XVRTStepper : public XVSSDStepper<IM_TYPE, XVRTState > {
 protected:
  using XVSSDStepper<IM_TYPE,XVRTState>::target ;
  using XVSSDStepper<IM_TYPE,XVRTState>::X ;
  using XVSSDStepper<IM_TYPE,XVRTState>::Y ;
  using XVSSDStepper<IM_TYPE,XVRTState>::Dx ;
  using XVSSDStepper<IM_TYPE,XVRTState>::Dy ;
  using XVSSDStepper<IM_TYPE,XVRTState>::inverse_model ;
  using XVSSDStepper<IM_TYPE,XVRTState>::forward_model ;
  using XVSSDStepper<IM_TYPE,XVRTState>::diff_intensity ;
  using XVSSDStepper<IM_TYPE,XVRTState>::size ;
 public:
  XVRTStepper() : XVSSDStepper<IM_TYPE, XVRTState >() {}
  XVRTStepper(const IM_TYPE & tmpl) 
    : XVSSDStepper<IM_TYPE, XVRTState >(tmpl) {}
  virtual void offlineInit();
  virtual XVStatePair<XVRTState, double> step(const XVImageScalar<float> &, 
					       const XVRTState &);
  virtual XVImageScalar<float> warp(const IM_TYPE &, const XVRTState &);
};


template <class IM_TYPE>
class XVRotateStepper : public XVSSDStepper<IM_TYPE, XVRotateState > {
 protected:
  using XVSSDStepper<IM_TYPE,XVRotateState>::target ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::X ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::Y ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::Dx ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::Dy ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::inverse_model ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::forward_model ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::diff_intensity ;
  using XVSSDStepper<IM_TYPE,XVRotateState>::size ;

 public:
  XVRotateStepper() : XVSSDStepper<IM_TYPE, XVRotateState >() {}
  XVRotateStepper(const IM_TYPE & tmpl) 
    : XVSSDStepper<IM_TYPE, XVRotateState >(tmpl){} 

  virtual void offlineInit();
  virtual XVStatePair<XVRotateState, double> step(const XVImageScalar<float> &, 
					       const XVRotateState &);
  virtual XVImageScalar<float> warp(const IM_TYPE &, const XVRotateState &);
};

template <class STEPPER_TYPE>
class XVPyramidStepper {
 protected:
  typedef typename STEPPER_TYPE::STATE_TYPE ST_TYPE ;
  typedef typename STEPPER_TYPE::IMAGE_TYPE IM_TYPE ;

  typedef std::vector<STEPPER_TYPE> STEPPER_SET ;

  double scale_factor ;
  STEPPER_SET steppers ;

  IM_TYPE savedImage ;

  virtual XVImageScalar<float> upperLayer( const XVImageScalar<float>& lower );

 public:

  typedef ST_TYPE STATE_TYPE;
  typedef IM_TYPE IMAGE_TYPE;

  typedef typename IM_TYPE::PIXELTYPE PIXEL_TYPE;

  typedef XVStatePair<STATE_TYPE, double> ResultPair;

  XVPyramidStepper(){}

  XVPyramidStepper( const IM_TYPE & template_in, double scale = .5,
		    int levels = 2 );

  virtual XVSize getSize() const { return steppers[0].getSize(); };
  
  virtual void offlineInit() ;

  virtual ResultPair step( const XVImageScalar<float> &, const ST_TYPE &) ;

  virtual XVImageScalar<float> warp(const IM_TYPE &, const ST_TYPE &) ;  
};

/**
 * XVSSD
 * 
 * @author Greg Hager, Sam Lang
 * @version $Id: XVSSD.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 * The XVSSD class provides an interface between the SSD
 * tracking algorithm, and the design of the XVision2
 * generic tracking structure (XVFeature).
 *
 * @see XVSSDStepper
 */
template <class IM_TYPE, class STEPPER_TYPE>
class XVSSD : public XVFeature<IM_TYPE, XVImageScalar<float>,
                               XVStatePair<typename STEPPER_TYPE::STATE_TYPE, 
                                           double> > {
protected:
  using XVFeature<IM_TYPE, XVImageScalar<float>,
    XVStatePair<typename STEPPER_TYPE::STATE_TYPE,double> >::prevState ;
  using XVFeature<IM_TYPE, XVImageScalar<float>,
    XVStatePair<typename STEPPER_TYPE::STATE_TYPE,double> >::currentState ;
  using XVFeature<IM_TYPE, XVImageScalar<float>,
    XVStatePair<typename STEPPER_TYPE::STATE_TYPE,double> >::warped ;
  using XVFeature<IM_TYPE, XVImageScalar<float>,
    XVStatePair<typename STEPPER_TYPE::STATE_TYPE,double> >::warpUpdated ;

protected:

  STEPPER_TYPE Stepper;

public:

  typedef IM_TYPE IMAGE_TYPE;
  typedef typename STEPPER_TYPE::STATE_TYPE STATE_TYPE;
  typedef XVStatePair<STATE_TYPE, double > SP;
  
  XVSSD() {}
  XVSSD(const STATE_TYPE & pos_in,
	const IM_TYPE & tmpl_in) 
    : XVFeature<IM_TYPE, XVImageScalar<float>, SP >(SP(pos_in, 0.0)), Stepper(tmpl_in) {Stepper.offlineInit();};

  virtual XVImageScalar<float> warp(const IM_TYPE &);
  virtual const SP & step(const IM_TYPE &);
  virtual const SP & step(const IM_TYPE & im, const SP & st)
    { return XVFeature<IM_TYPE, XVImageScalar<float>, SP>::step( im, st ); }
#ifndef NOVIS
  virtual void show(XVDrawable &,float scale=1.0);
#endif
  virtual const SP & interactiveInit(XVInteractive &, const IM_TYPE &);
  virtual const SP & interactiveInit(XVInteractive &, XVSize &, const IM_TYPE &);
  virtual void setStepper( const STEPPER_TYPE& stepper_in );
};

# endif
