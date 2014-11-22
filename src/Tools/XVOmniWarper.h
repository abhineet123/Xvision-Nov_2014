// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

# ifndef _XVOMNIWARPER_H_
# define _XVOMNIWARPER_H_

/** unwarps images obtained by omni-camera */
struct XVOmniWarper {
  static int defCenterX ;
  static int defCenterY ;
  static int defHorizon ;

  int centerX ;     // x co-ordinate of mirror center on image
  int centerY ;     // y co-ordinate of mirror center on image
  int horizon ;     // distance on image from center to horizon

  XVOmniWarper() : centerX(defCenterX), centerY(defCenterY), 
    horizon(defHorizon) {}

/*
  This omni2plane function unwarps an image obtained by omni-camera to a plane 
  image by geometric transformation. Some of the code is due to Sang-Chul Lee.
*/

/* Parameters --
   source:            input image ;
   nwidth, nheight:   dimensions of desired output image width ;
   xd, yd:            coordinates of a point on the input image as direction ;
   k:                 re-sampling factor ;
   Returns --         output image ;
*/

  template<class XVImage>
  XVImage omni2plane( const XVImage& source, int nwidth, int nheight, 
		      int xd, int yd, double k = 1.0 ) const ;

/* Parameters --
   source:            input image ;
   nwidth, nheight:   dimensions of desired output image width ;
   theta, phai:       direction ;
   k:                 re-sampling factor ;
   Returns --         output image ;
*/

  template<class XVImage>
  XVImage omni2plane( const XVImage& source, int nwidth, int nheight, 
		      double theta, double phai, double k = 1.0 ) const ;

};

# endif //_XVOMNIWARPER_H_
