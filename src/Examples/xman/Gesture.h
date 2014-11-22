// Should really keep around the source of this information using
// id's so that we can do non-local effects.

#include "XVImageScalar.h"
#include "XVMatrix.h"

struct TrackerOutput {
  XVColVector values;
  float objective_value_per_pixel;
};


template <class T, class T1>
class Gesture {

  XVImageScalar<T> templ_image;
  XVImageScalar<T> target;
  XVMatrix inverse_model;
  XVMatrix forward_model;
  XVColVector x;

public:

  XVImageScalar<T> &get_target_win(void) {return templ_image;};
  Gesture (XVImageScalar<T> &template_in);
  TrackerOutput Match_value(const XVImageScalar<T1> &live_image);
  //  int ComputeMatch(T1 contrast,const ColVector &differences);

};


