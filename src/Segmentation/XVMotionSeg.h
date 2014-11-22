// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVMOTIONSEG_H_
#define _XVMOTIONSEG_H_

#include <XVSegmentation.h>

#define DEFAULT_THRESHOLD 50

template <class T>
inline bool THRESHOLD_CHECK(const T pix){ return pix < DEFAULT_THRESHOLD ? true : false; }

/**  fast lookup table for thresholds, used to make XVMotionSeg */
template <class T>
class XVThresholdTable : public XVScalarTable<T, T>{

 protected:

  int thresh;
  
  virtual T computeValue(T pixel){ 
    return (T)(pixel > thresh ? pixel : 255);
  }

 public:

  XVThresholdTable(int t = DEFAULT_THRESHOLD);
};

/**
 * The XVMotionSeg class does Segmentation
 * based on the motion of the image */
template <class T>
class XVMotionSeg : public XVSegmentation<T, T>{
 protected:
  using XVSegmentation<T,T>::histSize ;
  using XVSegmentation<T,T>::histArraySize ;
  using XVSegmentation<T,T>::histogram ;
  using XVSegmentation<T,T>::lookup ;

 protected:
  
  XVImageScalar<T> prev;

  void init(const XVImageScalar<T> &, int, XVThresholdTable<T> * table = NULL);

 public:

  XVMotionSeg(int, int, int thresh = DEFAULT_THRESHOLD,
	      XVThresholdTable<T> * table = NULL);

  XVMotionSeg(const XVImageScalar<T> &, 
	      int thresh = DEFAULT_THRESHOLD, 
	      XVThresholdTable<T> * table = NULL);

  virtual void segment(const XVImageBase<T> &, XVImageScalar<T> &);

  virtual void update(const XVImageBase<T> & im) {}
};

#endif
