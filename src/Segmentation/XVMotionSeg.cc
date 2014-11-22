// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVMotionSeg.h>

template <class T>
XVThresholdTable<T>::XVThresholdTable(int t ) : XVScalarTable<T, T>() {
  
  thresh = t;
};

template <class T>
void XVMotionSeg<T>::init(const XVImageScalar<T> & p, int thresh, XVThresholdTable<T> * table ){

  this->setCheck(&(THRESHOLD_CHECK<T>));

  prev = XVImageScalar<T>(p.Width(), p.Height());
  XVImageIterator<T> pIter(p);
  XVImageWIterator<T> prevIter(prev);
  for(; !prevIter.end(); ++prevIter, ++pIter){
    *prevIter = *pIter;
  }

  if(table == NULL){
    
    lookup = new XVThresholdTable<T>(thresh);
    lookup->buildTable();
 }

  histSize = 0;
  histArraySize = 0;
  histogram = NULL;
};

template <class T>
XVMotionSeg<T>::XVMotionSeg(int w, int h,
			    int thresh , 
			    XVThresholdTable<T> * table ){
  
  XVImageScalar<T> p(w, h);
  p = 0;
  this->init(p, thresh, table);
};

template <class T>
XVMotionSeg<T>::XVMotionSeg(const XVImageScalar<T> & p,
			    int thresh ,
			    XVThresholdTable<T> * table ) {
  this->init(p, thresh, table);
};

template <class T>
void XVMotionSeg<T>::segment(const XVImageBase<T> & src, XVImageScalar<T> & targ){

  XVImageIterator<T> srcIter(src);
  XVImageWIterator<T> prevIter(prev);
  targ = XVImageScalar<T>(src.Width(), src.Height());
  XVImageWIterator<T> targIter(targ);
  
  for(; !srcIter.end(); ++srcIter, ++prevIter, ++targIter){
    *targIter = (*lookup)[(T)(*srcIter - *prevIter)];
    *prevIter = *srcIter;
  }
};

template class XVThresholdTable<u_char>;
template class XVThresholdTable<u_short>;
template class XVThresholdTable<int>;

template class XVMotionSeg<u_char>;
template class XVMotionSeg<u_short>;
template class XVMotionSeg<int>;
