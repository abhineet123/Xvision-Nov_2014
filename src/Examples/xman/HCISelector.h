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

#include "XVBlobs.h"
#include "XVImageScalar.h"

#define	 NO_GESTURE		0
#define	 GESTURE_CHANGED    (1<<0)
#define  GESTURE_STOPPED   (1<<1)
#define  GESTURE_UNCHANGED (1<<2)
