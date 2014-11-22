// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVPattern.h
 *
 * @author Sam Lang, Sidd Puri
 * @version $Id: XVPattern.h,v 1.1.1.1 2008/01/30 18:43:46 burschka Exp $
 *
 * This code modified from its original version (XVision) to work
 * with XVision2 
 */

#ifndef _XVPATTERN_H_
#define _XVPATTERN_H_

#define TINY  0.001

#include <XVTools.h>
#include <XVImageScalar.h>
#include <XVFeature.h>

//-----------------------------------------------------------------------------
//  Default parameters
//-----------------------------------------------------------------------------

int   const default_mwidth = 8;
float const default_alpha  = deg_rad (15);
int   const default_edge_threshold = 5;
int   const default_width = 40;
float const default_sensitivity = 1.0;
int   const default_prewitt_width = 2;

//-----------------------------------------------------------------------------
//  Structures for return values
//-----------------------------------------------------------------------------

typedef struct {
  double x, angle, val;
} XVOffset;

typedef struct {				     // returned by interpolation
  float x, val;
} XVPeak;

//----------------------------------------------------------------------------
//  Definitions of XVEdge templates
//----------------------------------------------------------------------------

template <class T> class XVPattern;
template <class T> class XVEdge;
template <class T> class XVAbsMEdge;
template <class T> class XVMaxEdge;
template <class T> class XVGeneralEdge;
template <class T> class XVGaussEdge;
template <class T> class XVShortEdge;

#include <XVException.h>

class XVEdgeException : public XVException {

 public:

  XVEdgeException() : XVException() {}
  XVEdgeException(char * str) : XVException(str) {}
};

/**
 * XVPattern<T>
 *
 * @author Sam Lang
 * 
 * XVPattern is the base class for all edge classes.
 * Specifically, the XVPattern class has the find function
 * which finds an edge from an image.  Each edge also requires
 * padding or a boundary, on the edges of the image so search in.
 *
 * @see XVEdge
 * @see XVMaxEdge
 * @see XVShortEdge
 */
template <class T>
class XVPattern {

protected:

  // Edge thresholds are defined by the expected difference in the
  // light/dark transition.  The actual threshold is computed
  // based on the size of the window and the size of the mask;
  int lthreshold, uthreshold;
  float _sensitivity;
  int mwidth;

public:

  typedef T PIXELTYPE;

  XVPattern(){};
  XVPattern(const XVPattern & cpy) : lthreshold(cpy.lthreshold),
                                     uthreshold(cpy.uthreshold),
                                     _sensitivity(cpy._sensitivity),
                                     mwidth(cpy.mwidth) {}
  
 
  virtual int xpad (int width, int height) = 0;
  virtual int ypad (int width, int height) = 0;

  virtual XVOffset find (XVImageScalar<T> &) = 0;

  float sensitivity() { return _sensitivity; };
  void set_sensitivity(float sen) { _sensitivity =sen; };

  int lower_threshold(int image_height) {
    return (mwidth/2) * image_height * lthreshold;
  }

  int upper_threshold(int image_height) {
    return (mwidth/2) * image_height * uthreshold;
  }
};

typedef enum {none, lightdark, darklight, any} linetype;

/**
 * XVEdge class declaration
 * This is a simple, fast, signed edge with a -1, 1 mask.
 */
template <class T>
class XVEdge : public XVPattern<T> {
 protected:
  using XVPattern<T>::lthreshold ;
  using XVPattern<T>::uthreshold ;
  using XVPattern<T>::_sensitivity ;
  using XVPattern<T>::mwidth ;

 private:
  float alpha;

  // By convention, a line is named by the brightness change when
  // facing the the direction of the line.  By default, a line
  // is unsigned.  By setting the transition type to "any,"
  // the line starts unsigned and changes once it locks on.
  linetype trans_type;

public:

  XVEdge (int mwidth_in = default_mwidth, float alpha_in = default_alpha) {
    mwidth = mwidth_in; alpha = alpha_in; trans_type = any;
    if (odd (mwidth)) mwidth++;		     // the algorithm only makes sense
                                             // for even mask widths
    lthreshold = default_edge_threshold;
    uthreshold = 256;
    _sensitivity = default_sensitivity;
  };
  
  XVEdge (const XVEdge & e) {*this = e;};
  
  virtual XVPattern<T> *dup() {return new XVEdge(*this);};
  
  int xpad (int , int height)
    { return mwidth-1 + 2 * round (half (height) * tan (alpha)); }
  int ypad (int , int ) { return 0; }
  
  XVOffset find (XVImageScalar<T> &);
  
  linetype & lineType() { return trans_type; }
};

/**
 * XVMaxEdge class declaration
 * This is a simple, fast, unsigned edge that finds the max column of an image
 */
template <class T>
class XVMaxEdge : public XVPattern<T> {
  
  // By convention, a line is named by the brightness change when
  // facing the the direction of the line.  By default, a line
  // is unsigned.  By setting the transition type to "any,"
  // the line starts unsigned and changes once it locks on.
 protected:
  using XVPattern<T>::lthreshold ;
  using XVPattern<T>::uthreshold ;
  using XVPattern<T>::_sensitivity ;
  using XVPattern<T>::mwidth ;

 private:  
  linetype trans_type;
  
 public:
  XVMaxEdge (int mwidth_in = default_mwidth, int min_threshold = 0) {
    mwidth = mwidth_in; trans_type = any;
    if (odd (mwidth)) mwidth++;		     // the algorithm only makes sense
    // for even mask widths
    lthreshold = min_threshold;
    _sensitivity = default_sensitivity;
  }
  
  XVMaxEdge (const XVMaxEdge & e) {*this = e;};
  
  int xpad (int w, int h)  { return w-1; }
  int ypad (int w, int h) { return 0; }
  XVOffset find (XVImageScalar<T> &);

  linetype & lineType() { return trans_type; }
};


template <class T>
class XVGeneralEdge : public XVPattern<T> {
protected:
  using XVPattern<T>::lthreshold ;
  using XVPattern<T>::uthreshold ;
  using XVPattern<T>::_sensitivity ;
  using XVPattern<T>::mwidth ;

protected:
  int *mask;
  float alpha;				     // angle offset at which to check
 					     // for tilted line
  float maxdiffangle;			     // maximum amount of angle offset
  //  int lower_threshold,upper_threshold;

  linetype trans_type;

public:
  XVGeneralEdge (int mwidth_in = default_mwidth, linetype trans_type_in = any,
	       float alpha_in = default_alpha);

  XVGeneralEdge (int * mask_in, int mwidth_in = default_mwidth, 
		 linetype trans_type_in = any,
		 float alpha_in = default_alpha);

  XVGeneralEdge (const XVGeneralEdge &);
  ~XVGeneralEdge() { delete mask; }

  int xpad (int mwidth_in, int height)
    { return mwidth_in-1 + 2 * round (half (height) * tan (alpha)); }
  int ypad (int , int) { return 0; }
  XVOffset find (XVImageScalar<T> &);

  linetype & lineType() { return trans_type; }
};

/**
 * XVShortEdge class declaration
 * This is a simple, fast signed edge of length one.  Used primarily
 *   for contour tracking.
 */
template <class T>
class XVShortEdge : public XVPattern<T> {

  // XVShortEdges are Edges of length 1, so no angle is required.
protected:
  using XVPattern<T>::lthreshold ;
  using XVPattern<T>::uthreshold ;
  using XVPattern<T>::_sensitivity ;
  using XVPattern<T>::mwidth ;

private:
  int maxpeaks;                              // half of image width

public:
  XVShortEdge (int mwidth_in = default_mwidth);

  XVShortEdge (const XVShortEdge & s) {*this = s;};

  int xpad (int , int )
    { return mwidth-1 + 2; }
  int ypad (int , int ) { return 0; }

  // This just returns the offset of the strongest peak.
  XVOffset find (XVImageScalar<T> & im);

  // This fills  lowout  with relevant info.  An upscale find fcn.
  void getlowout_ct (XVImageScalar<T> & im); // , SE_out & lowout);
};

#include <XVPattern.icc>

#endif
