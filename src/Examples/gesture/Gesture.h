// Should really keep around the source of this information using
// id's so that we can do non-local effects.

#include "XVImage.h"
#include "Matrix.h"

struct TrackerOutput {
  ColVector values;
  float objective_value_per_pixel;
};


template <class T, class T1>
class Gesture {

  XVImage<T> target;
  Matrix inverse_model;
  Matrix forward_model;
  ColVector x;

public:

  Gesture (const XVImage<T> &template_in);
  TrackerOutput Match_value(const XVImage<T1> &live_image);
  //  int ComputeMatch(T1 contrast,const ColVector &differences);

};


