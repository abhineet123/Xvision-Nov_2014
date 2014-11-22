// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***


/** XVision macro definitions */
#ifndef _XVMACROS_H_
#define _XVMACROS_H_

#include <assert.h>

#ifndef NULL
#define NULL 0
#endif

#if DEBUG_LEVEL > 0
#define ASSERT(x)  assert(x)
#define NDEBUG
#else
#define ASSERT(x) 
#endif

#if DEBUG_LEVEL == 2
#define DEBUG
#endif

#ifndef XV_RGB
#ifdef SCREEN_DEPTH 
#if SCREEN_DEPTH == 32
#define XV_RGB XV_RGBA32
#elif SCREEN_DEPTH == 24
#define XV_RGB XV_RGB24
#elif SCREEN_DEPTH == 16
#define XV_RGB XV_RGB16
#else
#define XV_RGB XV_RGBA32
#endif
#else
#define XV_RGB XV_RGBA32
#endif
#endif

#endif
