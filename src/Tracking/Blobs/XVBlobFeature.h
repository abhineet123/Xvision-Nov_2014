// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 *
 * XVBlobFeature.h
 *
 * @author Sam Lang
 * @version $Id: XVBlobFeature.h,v 1.1.1.1 2008/01/30 18:43:46 burschka Exp $
 *
 */

#ifndef _XVBLOBFEATURE_H_
#define _XVBLOBFEATURE_H_

#define DEFAULT_SAMPLING 1
#define MAX_SAMPLING     10
#define DEFAULT_PADSIZE  10

#include <XVFeature.h>
#include <XVSegmentation.h>
#include <XVBlobs.h>

typedef XVStatePair<XVRectangleBlob, double> XVBlobState;

template <class IMTYPEIN, class IMTYPEOUT>
class XVBlobFeature : public XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState > {

  typedef XVSegmentation<typename IMTYPEIN::PIXELTYPE, 
                         typename IMTYPEOUT::PIXELTYPE> SEGTYPE;
 protected:
  using XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>::prevState ;
  using XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>::currentState ;
  using XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>::warped ;
  using XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>::warpUpdated ;

 protected:

  SEGTYPE * segmenter;
  bool withResampling;
  XVSize imSize;

  virtual XVRectangleBlob pad(int padding = DEFAULT_PADSIZE);
  
  virtual void resample(const IMTYPEIN &, IMTYPEIN &);

 public:

  typedef typename XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>::STATE STATE;

  XVBlobFeature() : XVFeature<IMTYPEIN,IMTYPEIN,XVBlobState>(),withResampling(false), segmenter(NULL) {}

  XVBlobFeature(SEGTYPE & seg, 
		bool res = false) : XVFeature<IMTYPEIN,IMTYPEIN,XVBlobState>(),segmenter(&seg), withResampling(res) {}

  XVBlobFeature(SEGTYPE & seg, 
		STATE init, bool res = false) : XVFeature<IMTYPEIN, IMTYPEIN, XVBlobState>(init), segmenter(&seg), withResampling(res) {}

  virtual IMTYPEIN warp(const IMTYPEIN &);
  virtual const XVBlobState & step(const IMTYPEIN &);

  virtual const XVBlobState & interactiveInit(XVInteractive &, const IMTYPEIN &);
  virtual void show(XVDrawable &,float scale=1.0);
};

#include <XVImageRGB.h>
#include <XVImageScalar.h>
#include <XVImageYUV.h>

#endif
