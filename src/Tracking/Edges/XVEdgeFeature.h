// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVEdgeFeature.h
 *
 * @author Sam Lang
 * @version $Id: XVEdgeFeature.h,v 1.1.1.1 2008/01/30 18:43:46 burschka Exp $
 * 
 * XVEdgeFeature extends the notion of features
 * to edges and lines.
 *
 * @see XVFeature
 */

# ifndef _XVEDGEFEATURE_H_
# define _XVEDGEFEATURE_H_

#include <XVGeometry.h>
#include <XVPattern.h>
#include <XVAffineWarp.h>

#define DEFAULT_SEARCH_WIDTH 100
#define DEFAULT_SEARCH_ANGLE 8
#define DEFAULT_LINE_WIDTH   10

typedef double XVLineError;

typedef XVStatePair<XVLine, XVLineError> XVLineState;

template <class PIX_TYPE, class EDGETYPE>
class XVEdgeFeature : public XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState> {
 protected:
  using XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>::prevState ;
  using XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>::currentState ;
  using XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>::warped ;
  using XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>::warpUpdated ;

 protected:

  EDGETYPE edge;
  int searchWidth;
  int searchAngle;
  int halfWidth;
  int lineWidth;

 public:

  XVEdgeFeature(XVLineState st, 
		EDGETYPE e,
		int sw = DEFAULT_SEARCH_WIDTH,
		int sa = DEFAULT_SEARCH_ANGLE,
		int lw = DEFAULT_LINE_WIDTH)
    : XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>(), 
      edge(e), searchWidth(sw), searchAngle(sa), 

    lineWidth(lw), halfWidth(sw/2) {}
  
  XVEdgeFeature(EDGETYPE e,
		int sw = DEFAULT_SEARCH_WIDTH,
		int sa = DEFAULT_SEARCH_ANGLE,
		int lw = DEFAULT_LINE_WIDTH) 
    : XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>(), 
      edge(e), searchWidth(sw), searchAngle(sa), 
      
    lineWidth(lw), halfWidth(sw/2) {}

  XVEdgeFeature(int sw = DEFAULT_SEARCH_WIDTH,
		int sa = DEFAULT_SEARCH_ANGLE,
		int lw = DEFAULT_LINE_WIDTH) 
    : XVFeature<XVImageScalar<PIX_TYPE>, XVImageScalar<PIX_TYPE>, XVLineState>(), searchWidth(sw), searchAngle(sa), 
      lineWidth(lw), halfWidth(sw/2) { edge = EDGETYPE(); }
      
  virtual XVImageScalar<PIX_TYPE> warp(const XVImageScalar<PIX_TYPE> &);
  virtual const XVLineState & step(const XVImageScalar<PIX_TYPE> &);
  virtual const XVLineState & interactiveInit(XVInteractive &, 
					const XVImageScalar<PIX_TYPE>  &);
  virtual void show(XVDrawable &,float scale=1.0);
};

#endif
