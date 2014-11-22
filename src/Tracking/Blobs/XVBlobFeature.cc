// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

//-----------------------------------------------------------------------------
//
//  XVBlobFeature.cc
//
//  Sam Lang - 2.02.01
//
//-----------------------------------------------------------------------------

#include <XVBlobFeature.h>

template <class IMIN, class IMOUT>
XVRectangleBlob XVBlobFeature<IMIN, IMOUT>::pad(int padding ){

  XVRectangleBlob padded = (XVRectangleBlob)(currentState.state);
  if(padded.PosX() - padding > 0) {
    padded.posx -= padding;
    padded.width += padding;
  }
  if(padded.PosY() - padding > 0) {
    padded.posy -= padding;
    padded.height += padding;
  }
  if(padded.PosX() + padded.Width() + padding < imSize.Width()){
    padded.width += padding;
  }
  if(padded.PosY() + padded.Height() + padding < imSize.Height()){
    padded.height += padding;
  }
  return padded;
};

template <class IMIN, class IMOUT>
void XVBlobFeature<IMIN, IMOUT>::resample(const IMIN & im, IMIN & resampledIM){

  if(withResampling){
    int ratioX = round(((float)imSize.Width()) / 
		       ((float)((XVRectangleBlob)(currentState.state)).Width()));
    int ratioY = round(((float)imSize.Height()) / 
		       ((float)((XVRectangleBlob)(currentState.state)).Height()));

    int ONELESS = MAX_SAMPLING - 1;
    int sampleX = MAX_SAMPLING - (ratioX > ONELESS) ? ONELESS : ratioX;
    int sampleY = MAX_SAMPLING - (ratioY > ONELESS) ? ONELESS : ratioY;

    ::resample(sampleX, sampleY, im, resampledIM);
  }else{
    resampledIM = im;
  }
};

template <class IMIN, class IMOUT>
IMIN
XVBlobFeature<IMIN, IMOUT>::warp(const IMIN & im) {
  
  imSize = XVSize(im.Width(), im.Height());

  XVRectangleBlob paddedROI = this->pad();
  warped = subimage(im, paddedROI);

  warpUpdated = true;
  return warped;
};

template <class IMIN, class IMOUT>
const XVBlobState &
XVBlobFeature<IMIN, IMOUT>::step(const IMIN & im) {

  if(!warpUpdated) this->warp(im);
  prevState = currentState;

  XVRectangleBlob paddedROI = this->pad();

  IMIN resampledIM;
  this->resample(warped, resampledIM);

  XVRectangleBlob tmpROI;
  segmenter->findBoundingBox(resampledIM, tmpROI);

  XVSize absSize(tmpROI.Width(), tmpROI.Height());
  XVPosition absPos(tmpROI.PosX() + resampledIM.PosX(),
		    tmpROI.PosY() + resampledIM.PosY());
  XVImageGeneric absROI(absSize, absPos);
  
  IMIN newROIImage = subimage(im, absROI);

  double error = segmenter->percentError(newROIImage);

  if(tmpROI.Width() < 0 || tmpROI.Height() < 0){
    currentState.state.posx = 0; currentState.state.posy = 0;
    currentState.state.width = im.Width()-1; currentState.state.height = im.Height()-1;
    return currentState;
  } else {
    paddedROI.posx += tmpROI.PosX();  paddedROI.posy += tmpROI.PosY();
    paddedROI.width = tmpROI.Width(); paddedROI.height = tmpROI.Height();
    currentState.state = paddedROI;
  }

  currentState.error = error;
  warpUpdated = false;
  return currentState;
};

template <class IMIN, class IMOUT>
const XVBlobState &
XVBlobFeature<IMIN, IMOUT>::
interactiveInit(XVInteractive & win, const IMIN & im){
  
  XVImageGeneric roi;
  win.selectRectangle(roi);
  IMIN roiIM = subimage(im, roi);
  segmenter->update(roiIM);
  currentState.state = roi;
  currentState.error = true;
  prevState = currentState;
  return currentState;
};  

template <class IMIN, class IMOUT>
void XVBlobFeature<IMIN, IMOUT>::show(XVDrawable & win,float scale){
  
  // TODO - incorporate scale
  win.drawRectangle(currentState.state); 
};

#include <XVTracker.h>
#include <XVGroupTracker.h>
#include <XVImageRGB.h>
#include <XVImageYUV.h>
#include <XVImageYUV422.h>

#define _BLOB_F_(_SRC_IM_, _TRG_IM_) XVBlobFeature<_SRC_IM_, _TRG_IM_ >

#define _REGISTER_XVBLOBFEATURE_RGB_(_RGB_PIXEL_, _SC_PIXEL_) \
template class XVBlobFeature<XVImageRGB< _RGB_PIXEL_ >, \
                             XVImageScalar< _SC_PIXEL_ > >;

#define _REGISTER_XVBLOBFEATURE_SCALAR_(_RGB_) \
_REGISTER_XVBLOBFEATURE_RGB_(_RGB_, u_short); \
_REGISTER_XVBLOBFEATURE_RGB_(_RGB_, int);

_REGISTER_XVBLOBFEATURE_SCALAR_(XV_RGB16);
_REGISTER_XVBLOBFEATURE_SCALAR_(XV_RGB24);
_REGISTER_XVBLOBFEATURE_SCALAR_(XV_RGBA32);

#define _REGISTER_XVBLOBFEATURE_YUV_(_SC_PIXEL_) \
template class XVBlobFeature<XVImageYUV<XV_YUV24>, XVImageScalar< _SC_PIXEL_ > >; \
template class XVBlobFeature<XVImageYUV422, XVImageScalar< _SC_PIXEL_ > >; \

_REGISTER_XVBLOBFEATURE_YUV_(int);

