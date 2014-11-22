// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGEYCBCR_H_
#define _XVIMAGEYCBCR_H_

#include <XVColorImage.h>
#include <XVImageBase.h>
#include <XVImageRGB.h>
#include <XVImageYUV.h>

/** class for YCbCr images */ 
class XVImageYCbCr : public XVImageBase<XV_YCbCr>, public XVColorImage {
  
 public:

  XVImageYCbCr(const int width, const int height, XVPixmap<XV_YCbCr> * pixmap = NULL) :
    XVImageBase<XV_YCbCr>(width, height, pixmap) {}
  XVImageYCbCr(const XVSize s, XVPixmap<XV_YCbCr> * p ) :
    XVImageBase<XV_YCbCr>(s.Width(), s.Height(), p) {}
  XVImageYCbCr(const XVImageGeneric & im, XVPixmap<XV_YCbCr> * pix = NULL) : XVImageBase<XV_YCbCr>(im, pix){}
  XVImageYCbCr(const XVImageYCbCr & im) : XVImageBase<XV_YCbCr>(im) {}

  XVImageYCbCr(XVPixmap<XV_YCbCr> * pixmap) : XVImageBase<XV_YCbCr>(pixmap) {}
  XVImageYCbCr() : XVImageBase<XV_YCbCr>() {}

  XVImageBase<XV_YCbCr> newImage() { return XVImageYCbCr(); };
  XVImageBase<XV_YCbCr> newImage(int w, int h) { return XVImageYCbCr(w,h); }
  XVImageBase<XV_YCbCr> newImage(XVSize s) { return XVImageYCbCr(s); };
  XVImageBase<XV_YCbCr> newImage(XVPixmap<XV_YCbCr> * p ) { return XVImageYCbCr(p); };

  operator XVImageRGB<XV_RGB15>()  const; 
  operator XVImageRGB<XV_RGB16>()  const;
  operator XVImageRGB<XV_RGB24>()  const; 
  operator XVImageRGB<XV_TRGB24>()  const; 
  operator XVImageRGB<XV_RGBA32>() const;
  operator XVImageRGB<XV_GLRGBA32>() const;
  operator XVImageYUV<XV_YUV24>()  const; 
  operator XVImageYUV422()         const;
  operator XVImageYCbCr()         const;
  operator XVImageHSV<XV_HSV24>()  const;

  operator XVImageScalar<u_char>()  const;
  operator XVImageScalar<char>()    const;
  operator XVImageScalar<u_short>() const;
  operator XVImageScalar<short>()   const;
  operator XVImageScalar<u_int>()   const;
  operator XVImageScalar<int>()     const;
  operator XVImageScalar<float>()   const;
  operator XVImageScalar<double>()  const;
};  

#endif
