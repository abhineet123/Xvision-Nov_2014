#include "XVScalarSeg.h"

template <class T, class Y>
void XVScalarRangeTable<T,Y>::buildTable(void){
  int size=(1 << (sizeof(T)*8));

  // create a lookup table and set it to zero as default
  table=new Y[size];
  memset(table,0,size*sizeof(Y));
  // set values in range
  for(int i=range.min;i<range.max;i++) table[i]=(Y)(i-range.min+1);
}

template <class T, class Y>
XVScalarRangeSeg<T,Y>::XVScalarRangeSeg(dispLims<T> & range):
						XVSegmentation<T,Y>(){
  histSize=histArraySize=(range.max-range.min+1);
  histogram = new u_int[histArraySize];
  lookup=new XVScalarRangeTable<T,Y>(range);
  lookup->buildTable();
  this->setCheck(SCAL_RANGE_SEGFUNC<Y>);
}

template class XVScalarRangeSeg<u_char,u_char>;
template class XVScalarRangeSeg<u_short,u_short>;
template class XVScalarRangeTable<u_char,u_char>;
template class XVScalarRangeTable<u_short,u_short>;
