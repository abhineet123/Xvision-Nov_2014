// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVCOLORSEG_H_
#define _XVCOLORSEG_H_

#include <math.h>
#include <XVSegmentation.h>
#include <XVTools.h>
#include <XVImageRGB.h>
#include <XVImageScalar.h>

#define DEFAULT_DISTANCE  20
#define DEFAULT_SECTOR_NUM 360
#define DEFAULT_HUERANGE_WIDTH 4

#define MAX_HUE_SECTOR_NUM 4000

typedef struct{

  float min;
  float max;

} HUERANGE;

static const HUERANGE DEFAULT_HUERANGE = {-1, -1};

/************************************************
 * Table and Segmentation classes for Hue based
 * Segmentation
 ************************************************/

/**
 * The HueTable class builds a table of the
 * RGB to Hue values
 */

/************************************************
 * Table and Segmentation classes for Sector
 * based Segmentation.  The hue values of the
 * pixels are divided into sectors
 * and an image is segmented based on those 
 * sectors
 ************************************************/

/**
 * The HueSectorTable class builds a table of the
 * RGB to Sector (Hue Sector) values
 */
template <class T, class Y>
class XVHueSectorTable : public virtual XVRGBTable<T, Y>{

 protected:

  u_char darkPix, brightPix;

  int numOfSectors;

  Y computeValue(T);
  
 public:
  
  XVHueSectorTable(int num, u_char dp, u_char bp) : XVRGBTable<T, Y>(), numOfSectors(num), darkPix(dp), brightPix(bp) { 
    if(numOfSectors > MAX_HUE_SECTOR_NUM) { 
      throw XVSegException((char *)"too many sectors for XVHueSectorTable"); 
    }
    this->buildTable(); 
  };

  int getSectors(){ return numOfSectors; };
};

/**
 * The XVHueSectorSeg class does Segmentation
 * based on the Hue Sectors of the image
 */
template <class T, class Y>
class XVHueSeg : public virtual XVSegmentation<T, Y>{
 protected:
  using XVSegmentation<T,Y>::histSize ;
  using XVSegmentation<T,Y>::histArraySize ;
  using XVSegmentation<T,Y>::histogram ;
  using XVSegmentation<T,Y>::lookup ;

 protected:

  int numOfSectors;

 public:

  XVHueSeg(int sec = DEFAULT_SECTOR_NUM, 
	   u_char dp = DEFAULT_DARK_PIXEL, 
	   u_char bp = DEFAULT_BRIGHT_PIXEL);
  XVHueSeg(XVHueSectorTable<T, Y> *);

  float findBestHue(const XVImageBase<T> &);

  int getNumOfSectors(){ return this->numOfSectors; }

  void update(const XVImageBase<T> & roiIM){};
};

/************************************************
 * Table and Segmentation classes for Color
 * Segmentation based on the distance from some
 * specified color.  
 ************************************************/

/**
 * The RGBDistanceTable class allows for fast
 * lookup to find the distance between any color
 * and a given color.  The distance is given
 * as the sine of the angle between the two colors,
 * or the length of their cross product divided by
 * their lengths.
 */
template <class T>
class XVRGBDistanceTable : public virtual XVRGBTable<T, bool>{

 protected:

  T baseColor;
  float bcLength;
  u_short distance;

  bool computeValue(T);
  
 public:

  XVRGBDistanceTable(T, u_short);
};

inline bool RGB_DISTANCE_CHECK(const bool val){ return val; };

/**
 * The XVRGBDistanceSeg class does Segmentation
 * based on the distance between the colors
 * in the image and some predefined color
 */
template <class T>
class XVRGBDistanceSeg : public virtual XVSegmentation<T, bool>{
 protected:
  using XVSegmentation<T,bool>::histSize ;
  using XVSegmentation<T,bool>::histArraySize ;
  using XVSegmentation<T,bool>::histogram ;
  using XVSegmentation<T,bool>::lookup ;

 public:

  XVRGBDistanceSeg(T, u_short dist = DEFAULT_DISTANCE,
		   XVRGBDistanceTable<T> * table = NULL);

  void update(const XVImageBase<T> & roiIM){};
};


/************************************************
 * Table and Segmentation classes for Color
 * Segmentation based on a range of Hue values
 ************************************************/

/**
 * The HueRangeTable class allows for fast
 * lookup to find the hue of a given RGB value
 * The table holds boolean values:  true if the hue
 * of the RGB pixel was within the desired range, false
 * otherwise.
 */
template <class T, class Y>
class XVHueRangeTable : public virtual XVRGBTable<T, Y>{

 protected:

  Y computeValue(T);
  u_char darkPix, brightPix;

 public:

  HUERANGE hueRange;

  XVHueSectorTable<T, Y> * hueTable;
  
  XVHueRangeTable(HUERANGE range, u_char dp, u_char bp) : XVRGBTable<T, Y>() { 
    hueTable = new XVHueSectorTable<T, Y>(DEFAULT_SECTOR_NUM, dp, bp);
    hueRange = range; 
    this->buildTable(); 
  }

  ~XVHueRangeTable(){ delete hueTable; };

  void updateTable(HUERANGE);
  HUERANGE getRange() { return hueRange; }
};

template <class Y>
inline bool RANGE_SEGFUNC(const Y val){ return (bool)val; }

template <class T, class Y>
class XVHueRangeSeg : public virtual XVSegmentation<T, Y>{
 protected:
  using XVSegmentation<T,Y>::histSize ;
  using XVSegmentation<T,Y>::histArraySize ;
  using XVSegmentation<T,Y>::histogram ;
  using XVSegmentation<T,Y>::lookup ;

 private:
  int rangeWidth;
  XVHueRangeTable<T, Y> * temp_Table;

 public:

  XVHueRangeSeg(HUERANGE range = DEFAULT_HUERANGE, int rW = DEFAULT_HUERANGE_WIDTH,
		int sec = DEFAULT_SECTOR_NUM, 
		u_char dp = DEFAULT_DARK_PIXEL, u_char bp = DEFAULT_BRIGHT_PIXEL);
  XVHueRangeSeg(XVHueRangeTable<T, Y> *);
  XVHueRangeSeg(XVImageRGB<T> &, int rW = DEFAULT_HUERANGE_WIDTH,
		int sec = DEFAULT_SECTOR_NUM,
		u_char dp = DEFAULT_DARK_PIXEL, u_char bp = DEFAULT_BRIGHT_PIXEL);
 
  void updateTable(HUERANGE);

  float findBestHue(const XVImageBase<T> &);

  void update(const XVImageBase<T> &);
  HUERANGE getRange(){ return dynamic_cast<XVHueRangeTable<T, Y> *>(lookup)->getRange(); }
};

#endif

