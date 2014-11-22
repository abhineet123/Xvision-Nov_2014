// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGECOLOR_H_
#define _XVIMAGECOLOR_H_

#include <XVPixel.h>
#include <XVException.h>

class XVSize;
class XVImageGeneric;

template <class T> class XVImageScalar;
template <class T> class XVImageRGB;
template <class T> class XVImageYUV;
template <class T> class XVImageHSV;
class XVImageYUV422;
class XVImageYCbCr;

/**
 * The purpose of this class is to allow implicit
 * conversions between different image types, when
 * such a conversion is required.  If no conversion
 * is required, no conversion is done.
 */
class XVColorImage {

 public:

  virtual operator XVImageRGB<XV_RGB15>()  const = 0;
  virtual operator XVImageRGB<XV_RGB16>()  const = 0;
  virtual operator XVImageRGB<XV_RGB24>()  const = 0;
  virtual operator XVImageRGB<XV_TRGB24>()  const = 0;
  virtual operator XVImageRGB<XV_RGBA32>() const = 0;
  virtual operator XVImageRGB<XV_GLRGBA32>() const = 0;
  virtual operator XVImageYUV<XV_YUV24>()  const = 0;
  virtual operator XVImageHSV<XV_HSV24>()  const = 0;
  virtual operator XVImageYUV422()         const = 0;
  virtual operator XVImageYCbCr()         const = 0;

  virtual operator XVImageScalar<u_char>()  const = 0;
  virtual operator XVImageScalar<char>()    const = 0;
  virtual operator XVImageScalar<u_short>() const = 0;
  virtual operator XVImageScalar<short>()   const = 0;
  virtual operator XVImageScalar<u_int>()   const = 0;
  virtual operator XVImageScalar<int>()     const = 0;
  virtual operator XVImageScalar<float>()   const = 0;
  virtual operator XVImageScalar<double>()  const = 0;
  virtual ~XVColorImage(){};
};

#include <XVImageBase.h>

/** base class for color images? */
template <class T>
class XVColorBase : public XVImageBase<T>, virtual public XVColorImage {

 public:

  XVColorBase(const int w, const int h, XVPixmap<T> * pix = NULL) : XVImageBase<T>(w, h, pix){}
  XVColorBase(const XVSize & s, XVPixmap<T> * pix = NULL) : XVImageBase<T>(s, pix){}
  XVColorBase(const XVImageGeneric & im, XVPixmap<T> * pix = NULL) : XVImageBase<T>(im, pix){}
  XVColorBase(const XVColorBase<T> & im) : XVImageBase<T>(im){}
  XVColorBase(XVPixmap<T> * pix) : XVImageBase<T>(pix){}
  XVColorBase() : XVImageBase<T>(){};

  virtual operator XVImageRGB<XV_RGB15>()   const; 
  virtual operator XVImageRGB<XV_RGB16>()   const;
  virtual operator XVImageRGB<XV_RGB24>()   const; 
  virtual operator XVImageRGB<XV_TRGB24>()   const; 
  virtual operator XVImageRGB<XV_RGBA32>()  const;
  virtual operator XVImageRGB<XV_GLRGBA32>()  const;
  virtual operator XVImageYUV<XV_YUV24>()   const; 
  virtual operator XVImageYUV422()          const;
  virtual operator XVImageYCbCr()          const;
  virtual operator XVImageHSV<XV_HSV24>()   const;

  virtual operator XVImageScalar<u_char>()  const;
  virtual operator XVImageScalar<char>()    const;
  virtual operator XVImageScalar<u_short>() const;
  virtual operator XVImageScalar<short>()   const;
  virtual operator XVImageScalar<u_int>()   const;
  virtual operator XVImageScalar<int>()     const;
  virtual operator XVImageScalar<float>()   const;
  virtual operator XVImageScalar<double>()  const;
};  

#endif
