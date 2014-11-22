// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVImageScalar.h>

#include <XVImageRGB.h>
#include <XVImageYUV.h>
#include <XVImageHSV.h>
#include <XVImageYUV422.h>

#define CONVERT_OP_IMPL(IM_TYPE, PIXEL_TYPE) \
template <class T> \
XVImageScalar<T>::operator IM_TYPE() const { \
\
  IM_TYPE newIM(0, 0); \
\
  newIM.resize(this->Width(), this->Height()); \
  XVImageIterator<T> fromIter(*this); \
\
  XVImageWIterator<PIXEL_TYPE > toIter(newIM); \
\
  for(; !toIter.end(); ++toIter, ++fromIter) { \
    *toIter << *fromIter; \
  } \
\
  return newIM; \
};

CONVERT_OP_IMPL(XVImageRGB<XV_RGB15>,  XV_RGB15);
CONVERT_OP_IMPL(XVImageRGB<XV_RGB16>,  XV_RGB16);
CONVERT_OP_IMPL(XVImageRGB<XV_RGB24>,  XV_RGB24);  
CONVERT_OP_IMPL(XVImageRGB<XV_TRGB24>,  XV_TRGB24);  
CONVERT_OP_IMPL(XVImageRGB<XV_RGBA32>, XV_RGBA32);
CONVERT_OP_IMPL(XVImageRGB<XV_GLRGBA32>, XV_GLRGBA32);
CONVERT_OP_IMPL(XVImageYUV<XV_YUV24>,  XV_YUV24);  
CONVERT_OP_IMPL(XVImageHSV<XV_HSV24>,  XV_HSV24);

template <class T>
XVImageScalar<T>::operator XVImageYUV422() const {

  XVImageYUV422 newIM(0, 0);
  XVSize oldSize = (XVSize)(*this);
  (const_cast<XVImageScalar<T> *>(this))->resize(2 * ((int)(this->Width() / 2)), this->Height());
  newIM.resize((int)(this->Width() / 2), this->Height());
  XVImageIterator<T> fromIter(*this);
  XVImageWIterator<XV_YUV422> toIter(newIM);
  T doublePix[2];
  for(; !toIter.end(); ++toIter, ++fromIter){
    doublePix[0] = *fromIter;
    ++fromIter;
    doublePix[1] = *fromIter;
    *toIter << doublePix;
  }
  (const_cast<XVImageScalar<T> *>(this))->resize(oldSize);
  return newIM;
}

template <class T>
T XVImageScalar<T>::mean() const { 
    T     *ptr = win_addr;
      int   i,j;

      T sum = (T)0; 
      for (i=0;i<height;i++,ptr+=lineskip) 
	        for (j=0;j<width;j++)
		        sum +=  *ptr++; 
       return sum/(width*height);
}


template <class T>
T XVImageScalar<T>::avg() const {
  
  T sum = 0;

  XVImageIterator<T> iIter(*this);
  for (;iIter.end() == false; ++iIter) sum += *iIter;
  return sum / (width * height);
};

template <class T>
T XVImageScalar<T>::sum() const {

  T sum = 0;
  XVImageIterator<T> iIter(*this);
  for (;iIter.end() == false; ++iIter) sum += *iIter;
  return sum;
};
 
#define METHOD_SCALAR_OP_EQUALS(_OP_) \
template <class T> \
XVImageScalar<T> & XVImageScalar<T>::operator _OP_ (const XVImageScalar<T> & im){ \
  ASSERT((XVSize)im == (XVSize)(*this)); \
  XVImageIterator<T> iter(im); \
  XVImageWIterator<T> iterThis(*this); \
  for(; !iter.end(); ++iter, ++iterThis){ \
    *iterThis _OP_ *iter; \
  } \
  return *this; \
};

METHOD_SCALAR_OP_EQUALS(+=);
METHOD_SCALAR_OP_EQUALS(-=);
METHOD_SCALAR_OP_EQUALS(*=);
METHOD_SCALAR_OP_EQUALS(/=);

#define METHOD_SCALAR_OP_INT(_OP_) \
template <class T> \
XVImageScalar<T> & XVImageScalar<T>::operator _OP_ (T val){ \
  XVImageWIterator<T> iterThis(*this); \
  for(; !iterThis.end(); ++iterThis){ \
    *iterThis _OP_ val; \
  } \
  return *this; \
}; 

METHOD_SCALAR_OP_INT(+=);
METHOD_SCALAR_OP_INT(-=);
METHOD_SCALAR_OP_INT(*=);
METHOD_SCALAR_OP_INT(/=);
METHOD_SCALAR_OP_INT(=);

template <class T>
XVImageScalar<T> & XVImageScalar<T>::operator - () {

  XVImageWIterator<T> iterThis(*this);
  for(; !iterThis.end(); ++iterThis){
    *iterThis = - *iterThis;
  }
  return *this;
};


template class XVImageScalar<u_char>;
template class XVImageScalar<u_short>;
template class XVImageScalar<u_int>;
template class XVImageScalar<char>;
template class XVImageScalar<short>;
template class XVImageScalar<int>;
template class XVImageScalar<float>;
template class XVImageScalar<double>;
