// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVLookupTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * The XVLookupTable class provides a fast lookup for
 * different functions.  Most frequent use is pixel
 * conversion (for example, fast lookup between all types
 * of RGB pixels (16 bit) to Hue pixels).
 * 
 * Many lookup tables already exist.  See below for further
 * information.
 *
 * The class is templated on the input type (T) and the
 * output type (Y).  (for example, T = XV_RGB16, Y = HUE_VALUE).
 */

#ifndef _XVLOOKUPTABLE_H_
#define _XVLOOKUPTABLE_H_

#include <limits.h>
#include <XVMacros.h>
#include <XVTools.h>
#include <XVException.h>

class XVLookupTableException : public XVException {

 public: 
  XVLookupTableException() : XVException() {}
  XVLookupTableException(char * err) : XVException(err) {}
};

template <class T, class Y>
class XVLookupTable{

 protected:

  // holds the actual values for the table
  Y * table;

 public:

  XVLookupTable() : table(NULL) {}

  virtual ~XVLookupTable(){ delete [] table; }

  /**
   * The computeValue function performs
   * the actual conversion from T to Y.
   */
  virtual Y computeValue(T) = 0;
  
  /**
   * The buildTable function, iterates
   * through all possible values of T
   * and calls computeValue for each
   * one.  It also initializes the size
   * of the table.
   */
  virtual void buildTable() = 0;
  
  /**
   * Provides convenient access
   * to the values in the lookup 
   * table.
   */
  virtual Y operator [] (T) = 0;

  virtual Y operator () (T) = 0;
};

//  Creates a typedef'd type which is a
//  lookup table that has the syntax:
//  XVTable_<FROM>_TO_<TO>
//  where <FROM> = the type you're coming from
//  and   <TO>   = the type you're going to

#define _TYPEDEF_LOOKUP_TABLE_(_TABLE_, _FROM_TYPE_, _TO_TYPE_) \
typedef _TABLE_<_FROM_TYPE_, _TO_TYPE_> XVTable_ ## _FROM_TYPE_ ## _TO_ ## _TO_TYPE_; 

/**
 * XVScalarTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * The XVScalarTable defines a lookup table
 * for fast conversion between a grayscale
 * (char, short, etc.) pixel, and another
 * value.  T should be a built-in type,
 * nothing bigger than a short (16 bits).
 * Anything bigger is just too big.
 */
template <class T, class Y>
class XVScalarTable : public XVLookupTable<T, Y>{
protected:
  using XVLookupTable<T,Y>::table ;
  
protected:

  virtual Y computeValue(T pixel) = 0;

public:

  XVScalarTable() : XVLookupTable<T, Y>() {}

  /**
   * Iterates through each of the possible
   * values and computes the pixel value
   * of a given type.
   */
  void buildTable();

  Y operator [] (const u_char pixel){ return table[pixel]; };
  Y operator () (const u_char  pixel){ return table[pixel]; };
  Y operator [] (const u_short pixel){ return table[pixel]; };
  Y operator () (const u_short pixel){ return table[pixel]; };
  Y operator [] (const char pixel){ return table[pixel + CHAR_MAX]; };
  Y operator () (const char pixel){ return table[pixel + CHAR_MAX]; };
  Y operator [] (const short pixel){ return table[pixel + SHRT_MAX]; };
  Y operator () (const short pixel){ return table[pixel + SHRT_MAX]; };
  Y operator [] (const int pixel){ return table[pixel + INT_MAX]; };
  Y operator () (const int pixel){ return table[pixel + INT_MAX]; };
};

#include <XVPixel.h>

/**
 * XVRGBTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * Fast lookup table for RGB pixels to some
 * value (hue, YUV, etc.).  This class is
 * templated on the RGB pixel type and the
 * result type.  The RGB pixel type can
 * be any of the XV_RGB* structures specified
 * in XVPixel.h.
 *
 * Since a 24 or 32 bit lookup table would
 * be restrictively large (16 Megs, and 2.4 Gigs
 * respectively), the access ([] and ()) operators
 * bitshift the 24 and 32 bit pixels down
 * to a 16 bit value.  This means that its not
 * really worthwile to have 24 or 32 bit pixels
 * if you are just going to pass them through
 * an XVRGBTable, because the precision in
 * the larger bit pixel is lost.  For completeness
 * though, the 24 and 32 bit pixel conversions
 * exist. 
 */
template <class T, class Y>
class XVRGBTable : public XVLookupTable<T, Y>{
protected:
  using XVLookupTable<T,Y>::table ;
  
protected:

  virtual Y computeValue(T pixel) = 0;

  inline Y & tableElement(const XV_RGB15 p) { return table[*(u_short *)(&p)]; };
  inline Y & tableElement(const XV_RGB16 p) { return table[*(u_short *)(&p)]; };
  inline Y & tableElement(const XV_RGB24 p) { 
    return table[((p.r & 0xf8) << 8) | ((p.g & 0xfc) << 3) | (p.b >> 3)]; 
  };
  inline Y & tableElement(const XV_RGBA32 p) { 
    return table[((p.r & 0xf8) << 8) | ((p.g & 0xfc) << 3) | (p.b >> 3)]; 
  }; 

public:

  XVRGBTable() : XVLookupTable<T, Y>() {}

  void buildTable();

  inline Y operator [] (const XV_RGB15);
  inline Y operator () (const XV_RGB15);
  inline Y operator [] (const XV_RGB16);
  inline Y operator () (const XV_RGB16);
  inline Y operator [] (const XV_RGB24);
  inline Y operator () (const XV_RGB24);
  inline Y operator [] (const XV_RGBA32);
  inline Y operator () (const XV_RGBA32);
};


/**
 * XVRGBConversionTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * The XVRGBConversionTable class performs fast conversion
 * from RGB pixels to any of the other pixel formats supported
 * in XVPixel.h.  Specifically, the conversion operator << must
 * be defined to go from XV_RGB* pixels to that type of pixel.
 *
 * @see XVPixel.h
 */
template <class RGB_TYPE, class TO_TYPE>
class XVRGBConversionTable : public XVRGBTable<RGB_TYPE, TO_TYPE> {

 protected:

  virtual TO_TYPE computeValue(RGB_TYPE rgb){ 
    TO_TYPE retval; 
    retval << rgb;
    return retval;
  };
};

// Adds typedefs for the tables that
// convert from RGB to something else

#define _TYPEDEF_TABLE_RGB_TO_(_TO_TYPE_) \
_TYPEDEF_LOOKUP_TABLE_(XVRGBConversionTable, XV_RGB15, _TO_TYPE_); \
_TYPEDEF_LOOKUP_TABLE_(XVRGBConversionTable, XV_RGB16, _TO_TYPE_); \
_TYPEDEF_LOOKUP_TABLE_(XVRGBConversionTable, XV_RGB24, _TO_TYPE_); \
_TYPEDEF_LOOKUP_TABLE_(XVRGBConversionTable, XV_RGBA32, _TO_TYPE_);

_TYPEDEF_TABLE_RGB_TO_(XV_YUV24);
_TYPEDEF_TABLE_RGB_TO_(XV_HSV24);

/**
 * XVYUVTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 * 
 * Base class for fast table lookup conversion from
 * YUV (XV_YUV24) to some other type.  Since a 24 bit
 * lookup table (16 megs) is too big, the 24 bit YUV values
 * are converted to a Y->8, U->4, V->4 format, and that
 * value is computed from the lookup table.  This allows
 * for only a 16 bit lookup table (56k).
 *
 * @see XVPixel.h
 */
template <class YUV_TYPE, class TO_TYPE>
class XVYUVTable : public XVLookupTable<YUV_TYPE, TO_TYPE> {
protected:
  using XVLookupTable<YUV_TYPE,TO_TYPE>::table ;
  
 protected:

  virtual TO_TYPE computeValue(YUV_TYPE) = 0;

  inline TO_TYPE & tableElement(const XV_YUV24 p){ 
    return table[((p.y << 8) | (p.u << 4) | p.v)];
  };

 public:

  XVYUVTable() : XVLookupTable<YUV_TYPE, TO_TYPE>() {}
  
  void buildTable();

  inline TO_TYPE operator [] (const XV_YUV24);
  inline TO_TYPE operator () (const XV_YUV24);
};

/**
 * XVYUVConversionTable
 *
 * @author Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 * 
 * Allows for fast lookup from XV_YUV24
 * to any other pixel type
 *
 * @see XVPixel.h
 */
template <class YUV_TYPE, class TO_PIXEL_TYPE>
class XVYUVConversionTable : public XVYUVTable<YUV_TYPE, TO_PIXEL_TYPE> {

 protected:

  virtual TO_PIXEL_TYPE computeValue(const YUV_TYPE yuvVal){
    TO_PIXEL_TYPE retval;  retval << yuvVal; return retval;
  };
};

_TYPEDEF_LOOKUP_TABLE_(XVYUVConversionTable, XV_YUV24, XV_RGB15);
_TYPEDEF_LOOKUP_TABLE_(XVYUVConversionTable, XV_YUV24, XV_RGB16);
_TYPEDEF_LOOKUP_TABLE_(XVYUVConversionTable, XV_YUV24, XV_RGB24);
_TYPEDEF_LOOKUP_TABLE_(XVYUVConversionTable, XV_YUV24, XV_RGBA32);

/**
 * XVYUV422ToRGBTable
 *
 * @author Darius Burschka, Sam Lang
 * @version $Id: XVLookupTable.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * Allows for fast lookup between XV_YUV422 and any other RGB
 * pixel.
 *
 * @see XVPixel.h.
 */
// Disabled since the code is seriously bugged and nobody seems using 
// it anyway -- Donald
/* 
template <class RGB_TYPE>
class XVYUV422ToRGBTable : public XVLookupTable<XV_YUV422, RGB_TYPE> {

 protected:

  u_char redTable   [ 1 << 16 ];
  u_char blueTable  [ 1 << 16 ];
  u_char greenTable1[ 1 << 16 ];
  u_char greenTable2[ 1 << 16 ];

  virtual RGB_TYPE computeValue(const XV_YUV422);

 public:
  
  XVYUV422ToRGBTable() : XVLookupTable<XV_YUV422, RGB_TYPE>() {}

  void buildTable();

  inline RGB_TYPE operator [] (const XV_YUV422);
  inline RGB_TYPE operator () (const XV_YUV422);
};  

typedef XVYUV422ToRGBTable<XV_RGB15 *> XVTable_XV_YUV422_TO_XV_RGB15;
typedef XVYUV422ToRGBTable<XV_RGB16 *> XVTable_XV_YUV422_TO_XV_RGB16;
typedef XVYUV422ToRGBTable<XV_RGB24 *> XVTable_XV_YUV422_TO_XV_RGB24;
typedef XVYUV422ToRGBTable<XV_RGBA32 *> XVTable_XV_YUV422_TO_XV_RGBA32;
*/
#include <XVLookupTable.icc>

#endif
