// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef __XVIMAGEYUV_H_
#define __XVIMAGEYUV_H_

#include <sys/types.h>
#include <assert.h>

#include <iostream>
#include <XVList.h>
#include <XVTools.h>
#include <XVImageBase.h>
#include <XVImageScalar.h>
#include <XVColorImage.h>

using namespace std ;

/** class for YUV images */
template <class T>
class XVImageYUV : virtual public XVColorBase<T> {
  
 public:

  XVImageYUV(int width, int height, XVPixmap<T> * pixmap = NULL) :
    XVColorBase<T>(width, height, pixmap) {}
  XVImageYUV(XVSize s, XVPixmap<T> * p ) :
    XVColorBase<T>(s.Width(), s.Height(), p) {}

  XVImageYUV(XVPixmap<T> * pixmap) : XVColorBase<T>(pixmap) {}
  XVImageYUV() : XVColorBase<T>() {}
};

typedef enum {XV_Y = 0,XV_U = 1, XV_V = 2} YUV_Projection;

extern bool bRGB2YUVTableBuilt;
extern bool bYUV2RGBTableBuilt;

#ifndef COLORRANGE
#define COLORRANGE 256
#endif

extern unsigned char TABLE_YUV_TO_R[COLORRANGE][COLORRANGE]; // R = t_r[Y][V]
extern unsigned char TABLE_YUV_TO_B[COLORRANGE][COLORRANGE]; // B = t_b[Y][U]
extern unsigned char TABLE_YUV_TO_PG[COLORRANGE][COLORRANGE];// pg = t_pg[U][V]
extern unsigned char TABLE_YUV_TO_G[COLORRANGE][COLORRANGE]; // G = t_g[Y][pg]

extern unsigned char TABLE_RGB_TO_PY[COLORRANGE][COLORRANGE];// py = t_py[R][G]
extern unsigned char TABLE_RGB_TO_Y[COLORRANGE][COLORRANGE]; // Y  = t_y[py][B]
extern unsigned char TABLE_RGB_TO_U[COLORRANGE][COLORRANGE]; // U  = t_u[B][Y]
extern unsigned char TABLE_RGB_TO_V[COLORRANGE][COLORRANGE]; // V =  t_v[R][Y]


void buildRGB2YUVTable();
void buildYUV2RGBTable();

#include <XVImageYUV.icc>

#endif
