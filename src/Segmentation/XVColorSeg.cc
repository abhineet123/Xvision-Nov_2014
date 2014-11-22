// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVColorSeg.h>

template <class T, class Y>
Y XVHueSectorTable<T, Y>::computeValue(T pixel){
    
  float hv = RGBtoHue(pixel, brightPix, darkPix);
  return ((u_short)hv & (NULL_HUE | NULL_DARK | NULL_BRIGHT)) ? (Y) hv : (Y) floor( hv / 360 * numOfSectors );
};

template <class T, class Y>
XVHueSeg<T, Y>::XVHueSeg(XVHueSectorTable<T, Y> * table) : XVSegmentation<T, Y>() {

  lookup = table;

  numOfSectors = table->getSectors();
  histSize = table->getSectors();
  histArraySize = ((Y) NULL_VALUE ) + 1;
  histogram = new u_int[histArraySize];
};

template <class T, class Y>
XVHueSeg<T, Y>::XVHueSeg(int num , 
			 u_char dp , 
			 u_char bp ) : XVSegmentation<T, Y>() {

  numOfSectors = num;

  lookup = new XVHueSectorTable<T, Y>(numOfSectors, dp, bp);

  histSize = num;
  histArraySize = ((Y) NULL_VALUE ) + 1;
  histogram = new u_int[histArraySize];
};

template <class T, class Y>
float XVHueSeg<T, Y>::findBestHue(const XVImageBase<T> & src){

  XVImageScalar<Y> tmp;
  int index;
  u_int maxHue; 
  this->segment(src, tmp);
  maxi(histogram, histSize, maxHue, index);
  return (float)index;
};

template <class T>
bool XVRGBDistanceTable<T>::computeValue(T pixel){

  return (bool) ((u_short)(round(asin((length(cross(pixel, baseColor))
				       / (length(pixel) * bcLength))) 
				 * 180 / M_PI))) < distance;
};

template <class T>
XVRGBDistanceTable<T>::XVRGBDistanceTable(T bc, u_short dist) : XVRGBTable<T, bool>(), 
                                                            baseColor(bc), distance(dist) {

    bcLength = (float)length(baseColor);
};

template <class T>
XVRGBDistanceSeg<T>::XVRGBDistanceSeg(T bc, u_short dist , 
				      XVRGBDistanceTable<T> * table ) :
  XVSegmentation<T, bool>(){
    
  if(table == NULL){
    
    lookup = new XVRGBDistanceTable<T>(bc, dist);
  }else { lookup = table; }

  histSize = 2;
  histArraySize = 2;
  histogram = new u_int[histArraySize];

  this->setCheck(&RGB_DISTANCE_CHECK);
};

template <class T, class Y>
Y XVHueRangeTable<T, Y>::computeValue(T pixel){

  XVHueRangeTable<T, Y> * tmpThis = dynamic_cast<XVHueRangeTable<T, Y> *>(this);
  Y tmpVal = ((*hueTable)[pixel] == 0) ? 1 : (*hueTable)[pixel];

  if(tmpThis->hueRange.min <= 0){
    if((tmpThis->hueTable->getSectors() + tmpThis->hueRange.min) <= tmpVal &&
       tmpThis->hueTable->getSectors() > tmpVal){
      return tmpVal;
    }else{
      return (Y)0;
    }
  }else{
    if(tmpThis->hueRange.min <= tmpVal && tmpThis->hueRange.max >= tmpVal){
      return tmpVal;
    }else{
      return (Y)0;
    }
  }
};

template <class T, class Y>
void XVHueRangeTable<T, Y>::updateTable(HUERANGE range){
    
  (dynamic_cast<XVHueRangeTable<T, Y> *>(this))->hueRange = range;
  this->XVRGBTable<T, Y>::buildTable();
};

template <class T, class Y>
XVHueRangeSeg<T, Y>::XVHueRangeSeg(XVHueRangeTable<T, Y> * table) : XVSegmentation<T, Y>(){
  
  lookup = table;

  histSize = table->hueTable->getSectors() + 1;
  histArraySize = ((Y) NULL_VALUE ) + 1;
  histogram = new u_int[histArraySize];
  
  this->setCheck(RANGE_SEGFUNC<Y>);
};

template <class T, class Y>
XVHueRangeSeg<T, Y>::XVHueRangeSeg(HUERANGE range , 
				   int rW  ,
				   int sec ,
				   u_char dp ,
				   u_char bp ) : XVSegmentation<T, Y>(){
  
  if(range.min == DEFAULT_HUERANGE.min && 
     range.max == DEFAULT_HUERANGE.max) 
    range.min = 0; range.max = sec;

  rangeWidth = rW;

  temp_Table = new XVHueRangeTable<T, Y>(range, dp, bp);
  lookup=temp_Table;

  histSize = sec + 1;
  histArraySize = ((Y) NULL_VALUE ) + 1;
  histogram = new u_int[histArraySize];
  
  this->setCheck(RANGE_SEGFUNC<Y>);
};

template <class T, class Y>
XVHueRangeSeg<T, Y>::XVHueRangeSeg(XVImageRGB<T> & im, 
				   int rW , 
				   int sec ,
				   u_char dp ,
				   u_char bp ) : XVSegmentation<T, Y>() {

  rangeWidth = rW;
  HUERANGE tmpRange = {0, 0};
  temp_Table = new XVHueRangeTable<T, Y>(tmpRange, dp, bp);
  lookup=temp_Table;
  
  histSize = sec + 1;
  histArraySize = ((Y) NULL_VALUE ) + 1;
  histogram = new u_int[histArraySize];
  
  this->setCheck(RANGE_SEGFUNC<Y>);
  this->update(im);
}

template <class T, class Y>
float XVHueRangeSeg<T, Y>::findBestHue(const XVImageBase<T> & src){

  XVImageScalar<Y> tmp(src.Width(), src.Height());
  int index;
  u_int maxHue; 
  lookup = temp_Table->hueTable;

  this->segment(src, tmp);
  lookup = temp_Table;
  maxi(histogram, histSize, maxHue, index);
  return (float)index;
};

template <class T, class Y>
void XVHueRangeSeg<T, Y>::updateTable(HUERANGE range){
  bool (* TMPFUNC) (const Y) = this->SEGFUNC;
  temp_Table->updateTable(range); 
  this->SEGFUNC = TMPFUNC;
};

template <class T, class Y>
void XVHueRangeSeg<T, Y>::update(const XVImageBase<T> & im){

  float best = this->findBestHue(im);
  HUERANGE r; 
  r.min = (int)(best - (rangeWidth / 2));
  r.max = (int)(best + (rangeWidth / 2));
  this->updateTable(r);
};

template class XVHueSectorTable<XV_RGB15,  u_short>;
template class XVHueSectorTable<XV_RGB16,  u_short>;
template class XVHueSectorTable<XV_RGB24,  u_short>;
template class XVHueSectorTable<XV_RGBA32, u_short>;

template class XVHueSeg<XV_RGB15,  u_short>;
template class XVHueSeg<XV_RGB16,  u_short>;
template class XVHueSeg<XV_RGB24,  u_short>;
template class XVHueSeg<XV_RGBA32, u_short>;

template class XVRGBDistanceTable<XV_RGB15>;
template class XVRGBDistanceTable<XV_RGB16>;
template class XVRGBDistanceTable<XV_RGB24>;
template class XVRGBDistanceTable<XV_RGBA32>;

template class XVRGBDistanceSeg<XV_RGB15>;
template class XVRGBDistanceSeg<XV_RGB16>;
template class XVRGBDistanceSeg<XV_RGB24>;
template class XVRGBDistanceSeg<XV_RGBA32>;

template class XVHueRangeTable<XV_RGB15,  u_short>;
template class XVHueRangeTable<XV_RGB16,  u_short>;
template class XVHueRangeTable<XV_RGB24,  u_short>;
template class XVHueRangeTable<XV_RGBA32, u_short>;

template class XVHueRangeSeg<XV_RGB15,  u_short>;
template class XVHueRangeSeg<XV_RGB16,  u_short>;
template class XVHueRangeSeg<XV_RGB24,  u_short>;
template class XVHueRangeSeg<XV_RGBA32, u_short>;
