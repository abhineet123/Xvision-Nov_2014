/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

#include "XVColSelector.h"

#define	 NO_GESTURE		0
#define	 GESTURE_CHANGED    (1<<0)
#define  GESTURE_STOPPED   (1<<1)
#define  GESTURE_UNCHANGED (1<<2)

template <class X,class Y,class Z>
class HCISelector: public XVColSelector<X,Y>
{
    float   *distance;   // distance from preferred angle
  public:
    XVImage<Z> *segment_distance(XVImage<Y> *image,
                                    XVImage<Z> *out_image=NULL);
    float	get_rotation(XVImage<X> *image,
                        Blob *region,X preferred);

    int  	select_gesture(Blob *regions,int count,int& sig_blob,
                        HCIObject& object);
                HCISelector(int num_bits,int num_sectors,
	               int dark_pix,int bright_pix,float preferred);
};
