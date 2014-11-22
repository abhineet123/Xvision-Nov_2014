
#include <XVLookupTable.h>

template class XVRGBConversionTable<XV_RGB15, XV_YUV24>;
template class XVRGBConversionTable<XV_RGB16, XV_YUV24>;
template class XVRGBConversionTable<XV_RGB24, XV_YUV24>;
template class XVRGBConversionTable<XV_RGBA32, XV_YUV24>;
template class XVRGBConversionTable<XV_RGB15, XV_HSV24>;
template class XVRGBConversionTable<XV_RGB16, XV_HSV24>;
template class XVRGBConversionTable<XV_RGB24, XV_HSV24>;
template class XVRGBConversionTable<XV_RGBA32, XV_HSV24>;

template class XVYUVConversionTable<XV_YUV24, XV_RGB15>;
template class XVYUVConversionTable<XV_YUV24, XV_RGB16>;
template class XVYUVConversionTable<XV_YUV24, XV_RGB24>;
template class XVYUVConversionTable<XV_YUV24, XV_RGBA32>;

// Disabled, see XVLookupTable.h -- Donald
/*
template <class RGB_TYPE>
void XVYUV422ToRGBTable<RGB_TYPE>::buildTable() {

  int r, b, g1, g2;

  for(int i = 0; i < 256; i++) {
    for(int j = 0; j < 256; j++) {
      
      r  = i + (((j - 128) * 1434) / 2048);
      b  = i + (((j - 128) * 2078) / 2048);
      g1 = (((i - 128) * 406) / 2048) + (((j - 128) * 595) / 2048);
      g2 = i - j;
      r = r < 0 ? 0 : r;
      g1 = g1 < 0 ? 0 : g1;
      g2 = g2 < 0 ? 0 : g2;
      b = b < 0 ? 0 : b;
      r = r > 255 ? 255 : r;
      g1 = g1 > 255 ? 255 : g1;
      g2 = g2 > 255 ? 255 : g2;
      b = b > 255 ? 255 : b;
      redTable   [ (i << 8) | j ] = r;
      blueTable  [ (i << 8) | j ] = b;
      greenTable1[ (i << 8) | j ] = g1;
      greenTable2[ (i << 8) | j ] = g2;
    }
  }
};

template <class RGB_TYPE>
RGB_TYPE XVYUV422ToRGBTable<RGB_TYPE>::computeValue(XV_YUV422 val){ 
  throw XVLookupTableException("computeValue not defined for XVYUV422ToRGBTable");
  RGB_TYPE (ret[2]);
  return *ret;
};

template <class RGB_TYPE>
RGB_TYPE XVYUV422ToRGBTable<RGB_TYPE>::operator [] (const XV_YUV422 val){

  RGB_TYPE (ret[2]);

  u_short shiftedY0 = val.y0 << 8;
  (*ret)[0].setR( redTable[    shiftedY0 | val.v ] );
  (*ret)[0].setB( blueTable[   shiftedY0 | val.u ] );
  (*ret)[0].setG( greenTable2[ shiftedY0 | greenTable1[ (val.u << 8) | val.v ] ] );

  u_short shiftedY1 = val.y1 << 8;
  (*ret)[0].setR( redTable[    shiftedY1 | val.v ] );
  (*ret)[0].setB( blueTable[   shiftedY1 | val.u ] );
  (*ret)[0].setG( greenTable2[ shiftedY1 | greenTable1[ (val.u << 8) | val.v ] ] );
  
  return *ret;
};

template class XVYUV422ToRGBTable<XV_RGB15 *>;
template class XVYUV422ToRGBTable<XV_RGB16 *>;
template class XVYUV422ToRGBTable<XV_RGB24 *>;
template class XVYUV422ToRGBTable<XV_RGBA32 *>;
*/

