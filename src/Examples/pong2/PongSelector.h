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

#ifndef __PONGSELECTOR_H
#define __PONGSELECTOR_H

#include <stdio.h>

#include "XVColorSeg.h"
#include "XVImageRGB.h"

#define	 UP_HIT		(1<<0)
#define	 DOWN_HIT	(1<<1)
#define  LEFT_HIT	(1<<2)
#define  RIGHT_HIT	(1<<3)

template <class T, class Y>
class PongSelector: public XVHueSeg<T,Y>
{
  public:
    int  check_ball(XVImageRGB<T> *image,int x,int y, int size,
    		    bool (*pf) (const Y) = NULL);
         PongSelector(int num_sectors = 72,
	              int dark_pix=12,int bright_pix=200);
};

#endif
