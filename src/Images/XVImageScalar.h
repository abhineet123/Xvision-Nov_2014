// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVIMAGESCALAR_H_
#define _XVIMAGESCALAR_H_

#include <sys/types.h>
#include <iostream>
#include <fstream>

#include <XVList.h>
#include <XVTools.h>
#include <XVImageBase.h>

#include <XVPixel.h>

template <class T> class XVImageScalar;
template <class T> class XVImageRGB;
template <class T> class XVImageYUV;
template <class T> class XVImageHSV;
class XVImageYUV422;

/** 
 * Defines an image with scalar operations.
 * Image class where each pixel is represented by
 * a scalar type, such as u_char, u_short, int, float or double. 
 * The class is inherited from XVImageBase, and has some additional 
 * arithmetic operators, plus write_pgm and read_pgm for input and
 * output of greyscale images in .pgm files;
 */
template <class T>
class XVImageScalar : public XVImageBase<T> {
protected:
  using XVImageBase<T>::width ;
  using XVImageBase<T>::height ;
  using XVImageBase<T>::win_addr ;
  using XVImageBase<T>::lineskip ;

public:

  XVImageScalar(int width, int height, XVPixmap<T> * pixmap = NULL) : 
    XVImageBase<T>(width,height,pixmap) {};
  XVImageScalar(const XVSize & s, XVPixmap<T> * p ) :
    XVImageBase<T>(s.Width(), s.Height(), p) {};

  XVImageScalar(XVPixmap<T> * pixmap) : XVImageBase<T>(pixmap) {};

  XVImageScalar() : XVImageBase<T>() {};

  operator XVImageRGB<XV_RGB15>()  const;
  operator XVImageRGB<XV_RGB16>()  const;
  operator XVImageRGB<XV_RGB24>()  const;
  operator XVImageRGB<XV_TRGB24>()  const;
  operator XVImageRGB<XV_RGBA32>() const;
  operator XVImageRGB<XV_GLRGBA32>() const;
  operator XVImageYUV<XV_YUV24>()  const;
  operator XVImageHSV<XV_HSV24>()  const;
  operator XVImageYUV422()         const;

  template <class T2>
    operator XVImageScalar<T2>()   const {   
    
    XVImageScalar<T2> newScalar;
    ScalartoScalar(*this, newScalar);
    return newScalar;
  }

  T mean() const;
  T avg() const;
  T sum() const;

  /** 
   * Adds to each pixel in the image the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator += (const XVImageScalar<T> &);

  /** 
   * Subtracts from each pixel in the image the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator -= (const XVImageScalar<T> &);

  /** 
   * Multiplies each pixel in the image by the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator *= (const XVImageScalar<T> &);

  /** 
   * Divides each pixel in the image by the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator /= (const XVImageScalar<T> &);

  /** 
   * Sets each pixel in the image to a specified value
   */
  XVImageScalar<T> & operator = (T);

  /** 
   * Adds to each pixel in the image the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator += (T);

  /** 
   * Subtracts from each pixel in the image the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator -= (T);

  /** 
   * Multiplies each pixel in the image by the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator *= (T);

  /** 
   * Divides each pixel in the image by the corresponding
   * pixel in the image passed in.
   */
  XVImageScalar<T> & operator /= (T);

  XVImageScalar<T> & operator - ();
};

#include <XVImageScalar.icc>
#include <XVImageFilters.icc>

#endif
