// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGEYUV422_H_
#define _XVIMAGEYUV422_H_

#include <XVColorImage.h>
#include <XVImageBase.h>
#include <XVImageRGB.h>
#include <XVImageYUV.h>

/** class for YUV422 images */ 
class XVImageYUV422 : public XVImageBase<XV_YUV422>, public XVColorImage {
  
 public:

  XVImageYUV422(const int width, const int height, XVPixmap<XV_YUV422> * pixmap = NULL) :
    XVImageBase<XV_YUV422>(width, height, pixmap) {}
  XVImageYUV422(const XVSize s, XVPixmap<XV_YUV422> * p ) :
    XVImageBase<XV_YUV422>(s.Width(), s.Height(), p) {}
  XVImageYUV422(const XVImageGeneric & im, XVPixmap<XV_YUV422> * pix = NULL) : XVImageBase<XV_YUV422>(im, pix){}
  XVImageYUV422(const XVImageYUV422 & im) : XVImageBase<XV_YUV422>(im) {}

  XVImageYUV422(XVPixmap<XV_YUV422> * pixmap) : XVImageBase<XV_YUV422>(pixmap) {}
  XVImageYUV422() : XVImageBase<XV_YUV422>() {}

  XVImageBase<XV_YUV422> newImage() { return XVImageYUV422(); };
  XVImageBase<XV_YUV422> newImage(int w, int h) { return XVImageYUV422(w,h); }
  XVImageBase<XV_YUV422> newImage(XVSize s) { return XVImageYUV422(s); };
  XVImageBase<XV_YUV422> newImage(XVPixmap<XV_YUV422> * p ) { return XVImageYUV422(p); };

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
