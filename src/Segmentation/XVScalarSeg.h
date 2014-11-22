#include "XVLookupTable.h"
#include "XVSegmentation.h"

template <class T>
struct dispLims {
   T min;
   T max;
};

template <class Y>
inline bool SCAL_RANGE_SEGFUNC(const Y val){ return (bool)val; }

/** fast lookup table for scalar range values */
template <class T, class Y>
class XVScalarRangeTable:public XVScalarTable<T,Y> {
  protected:
    using XVScalarTable<T,Y>::table;

  private:
    dispLims<T> range;
  public:
    XVScalarRangeTable(dispLims<T> & set_range):
      				XVScalarTable<T,Y>(),
				range(set_range){};
    ~XVScalarRangeTable() {};
    Y computeValue(T val) {if(val>=range.min && val<range.max)
      			    return (Y)(val-range.min+1); else return 0;};
    void buildTable(void);
};

/** segmentation based on scalar range values */
template <class T, class Y>
class XVScalarRangeSeg : public virtual XVSegmentation<T, Y>{
 protected:
  using XVSegmentation<T,Y>::histSize ;
  using XVSegmentation<T,Y>::histArraySize ;
  using XVSegmentation<T,Y>::histogram ;
  using XVSegmentation<T,Y>::lookup ;

 public:
  XVScalarRangeSeg(dispLims<T> & set_range);
  ~XVScalarRangeSeg() {};
  void update(const XVImageBase<T>&){};
};
