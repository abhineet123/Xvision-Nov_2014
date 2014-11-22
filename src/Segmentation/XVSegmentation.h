// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

// 
// XVSegmentation.h
//
// Samuel Lang
// 9.21.00
//

#ifndef _XVSEGMENTATION_H_
#define _XVSEGMENTATION_H_

#include <XVImageBase.h>
#include <XVImageScalar.h>
#include <XVBlobs.h>
#include <vector>
#include <math.h>
#include <string.h>

#define NULL_VALUE ~0

#define MAX_REGIONS      2000
#define DEFAULT_PADDING  40
#define M_TO_S_RATIO     1.1

class XVSegException : public XVException { public:  
  XVSegException() : XVException(){}
  XVSegException(char * str) : XVException(str) {} }; 

#include <XVLookupTable.h>

/**
 * The XVSegmentation takes images and does segmentation
 * based on some user specified operation.  Examples
 * include segmentation based on color, motion, 
 * shape, etc.
 *
 * NOTE:  The XVSegmentation class is abstract (it
 * doesn't have a constructor).  So it must be
 * extended to be of any use.  It just acts as a
 * holder for functions that are general enough
 * for all types of segmentation.
 * Please see XVColorSeg.h and XVMotionSeg.h for
 * further information.
 */
template <class T, class Y>
class XVSegmentation{

  typedef T PIXELINTYPE;
  typedef Y PIXELOUTTYPE;

 protected:
  
  int             histSize;
  int             histArraySize;
  u_int *         histogram;
  XVLookupTable<T,Y> * lookup;
  
public:

  bool (*SEGFUNC) (const Y);

  XVSegmentation() : histSize(0), histArraySize(0), histogram(NULL), lookup(NULL) {}

  virtual ~XVSegmentation();

  virtual void update(const XVImageBase<T> &) = 0;
  
  /**
   * the segment function computes some value for
   * each pixel in the color image (the first parameter)
   * using the lookup table (which can be just about anything)
   * and builds the scalar image (the second parameter)
   * from those resulting values.
   */
  virtual void segment(const XVImageBase<T> &, XVImageBase<Y> &);
  virtual void findCentroid(const XVImageScalar<Y> &, XVPosition &, 
			    int &, bool (*pf) (const Y) = NULL);
  virtual XVRectangleBlob & findBoundingBox(const XVImageBase<T> &, 
					   XVRectangleBlob &,  
					   bool (*pf) (const Y) = NULL);
  virtual double percentError(const XVImageBase<T> &,
			      bool (*pf) (const Y) = NULL);
  virtual void regionGrow(const XVImageScalar<Y> &, vector<XVRectangleBlob> &, 
			  bool (*pf) (const Y) = NULL, 
			  int padding = DEFAULT_PADDING,
			  int maxRegions = MAX_REGIONS,
			  float ratio    = M_TO_S_RATIO);

  void setCheck(bool (*pf) (const Y)){ this->SEGFUNC = pf; }

  u_int * getHistogram(){ return histogram; }
};

#endif
