// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

# include <math.h>
# include <XVImageRGB.h>
# include <XVImageScalar.h>
# include "XVOmniWarper.h"

/*
  This omni2plane function unwarps an image obtained by omni-camera to a plane 
  image by geometric transformation. Some of the code is due to Sang-Chul Lee.
*/

// Math constants definition

#ifndef PI
#define PI 3.14159265358979323846
#endif
#define PI_2 (PI/2)

// Constants depends on the mirror-camera system. May need further calibration.

int XVOmniWarper::defCenterX = 313 ;
int XVOmniWarper::defCenterY = 252 ;
int XVOmniWarper::defHorizon = 269 ;

/* Parameters --
   source:            input image ;
   nwidth, nheight:   dimensions of desired output image width ;
   xd, yd:            coordinates of a point on the input image as direction ;
   k:                 re-sampling factor ;
   Returns --         output image ;
*/
template<class XVImage>
XVImage XVOmniWarper::omni2plane( const XVImage& source, 
  int nwidth, int nheight, int xd, int yd, double k ) const 
{
  typedef typename XVImage::PIXELTYPE Pixel ;
  XVImage dest_im(nwidth, nheight);
  const Pixel *r_ptr = source.data();
  Pixel *w_ptr = dest_im.lock(), *ptr, *end;
  
  int i, width, height, nw2, nh2 ;
  double rou, theta, phai ;
  double x,y,z, v1x,v1y,v1z, v2x,v2y,v2z, sum,dist  ;
  double p, ux,uy,uz, px,py,pz ;

  width = source.Width();
  height = source.Height();
  nw2 = nwidth/2;
  nh2 = nheight/2;

  r_ptr += centerY*width + centerX;
  xd -= centerX ;
  yd -= centerY ;

  rou = horizon ;
  if( !xd && !yd  ) {
    z = v1x = v2y = rou ;
    x = v1y = v2x = 0 ;
    y = v1z = v2z = 0 ;

  }else {
    x = xd ;
    y = yd ;
    sum = x*x+y*y ;
    dist = sqrt(sum) ;
    z = rou*cos(dist/rou*PI_2);
    dist *= k ;

    v1x = y;
    v1y = -x;
    v1z = 0;
    v1x /= dist; v1y /= dist;
  
    v2x = x*z;
    v2y = y*z;
    v2z = -sum;
    if( v2z < 0 ) {
      v2x = -v2x;
      v2y = -v2y;
      v2z = -v2z;
    }
    dist *= sqrt(sum+z*z) ;
    v2x /= dist; v2y /= dist; v2z /= dist;
  }

  ux = v1x*(-(nw2+1)) + x; 
  uy = v1y*(-(nw2+1)) + y;
  uz = v1z*(-(nw2+1)) + z;

  for( i=-nh2; i<nh2; i++ ) {

    px = ux + (v2x*i);
    py = uy + (v2y*i);
    pz = uz + (v2z*i);

    ptr = w_ptr + (nh2-i)*nwidth ;
    end = ptr + nwidth ;
    while( ptr < end ) {
      px += v1x; 
      py += v1y;
      pz += v1z;

      p = rou/(sqrt(px*px+py*py+pz*pz)+pz) ;
      *(ptr++) = r_ptr[(int)(py*p)*width + (int)(px*p)];
    }
  }
  dest_im.unlock();
  return dest_im ;
}

/* Parameters --
   source:            input image ;
   nwidth, nheight:   dimensions of desired output image width ;
   theta, phai:       direction ;
   k:                 re-sampling factor ;
   Returns --         output image ;
*/
template<class XVImage>
XVImage XVOmniWarper::omni2plane( const XVImage& source, 
  int nwidth, int nheight, double theta, double phai, double k ) const 
{
  double rou = horizon * sin(theta) ;
  int xd = (int)( rou * cos(phai) ) + centerX ;
  int yd = (int)( rou * sin(phai) ) + centerY ;
  return omni2plane( source, nwidth, nheight, xd, yd, k );
}

# define DEF_FUN1(XVImage) template XVImage XVOmniWarper::omni2plane<XVImage >(const XVImage& source, int nwidth, int nheight, int xd, int yd, double k ) const ;
# define DEF_FUN2(XVImage) template XVImage XVOmniWarper::omni2plane<XVImage >(const XVImage& source, int nwidth, int nheight, double theta, double phai, double k ) const ;
# define DEF_FUN(XVImage) DEF_FUN1(XVImage) DEF_FUN2(XVImage)
# define DEF_FUN_RGB(PIX) DEF_FUN(XVImageRGB<PIX>)
# define DEF_FUN_SCL(PIX) DEF_FUN(XVImageScalar<PIX>)

DEF_FUN_RGB(XV_RGB15)
DEF_FUN_RGB(XV_RGB16)
DEF_FUN_RGB(XV_RGB24)
DEF_FUN_RGB(XV_RGBA32)
DEF_FUN_SCL(u_char)
DEF_FUN_SCL(u_short)
DEF_FUN_SCL(u_int)
DEF_FUN_SCL(char)
DEF_FUN_SCL(short)
DEF_FUN_SCL(int)
