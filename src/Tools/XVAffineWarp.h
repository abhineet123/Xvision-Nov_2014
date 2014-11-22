// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 *  XVAffineWarp
 *
 *  @author Sam Lang, Svetlana Minina
 *  @version $Id: XVAffineWarp.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 *  Definition of geometric transformations, such as rotation, sheer and 
 *  scaling. Information about the transformations is stored in a 2 x 2
 *  matrix. Transformations are performed when method 'warp' is called
 *  with a scalar image to be transformed passed as an argument.
 *
 *  To rotate image1 by 3.2 radians, scale by 2.1 in both directions, and 
 *  sheer by 0.1, recording the result in image2:
 *
 *  XVAffineWarp<T> trans(3.2, 2.1, 2.1, 0.1);
 *  image2 = trans.warp(image1);  
 */

#ifndef _XVAFFINEWARP_H_
#define _XVAFFINEWARP_H_

#include <iostream>
#include <math.h>
#include <string.h>

#include <XVMacros.h>
#include <XVGeometry.h>
#include <XVTools.h>
#include <XVImageBase.h>
#include <XVImageIterator.h>

#define INVERSE 1
#define FORWARD 0

//------------------------------------------------------------------------
//  XVAffineWarp Class Declaration
//------------------------------------------------------------------------

/** class for with 2x2 affine matrix */
template <class T>
class XVAffineWarp {
private:

  /** a 2x2 matrix which multiplied by an image produces a warped image */
  XVAffineMatrix amatrix;
  /** a constant computed in constructor, frequently needed in warping */
  float ratio;

  void invertMatrix();

public:
  /** constructor used for rotation only, theta in radians */
  XVAffineWarp (float);      
  /** constructor used for scaling in x and y directions */
  XVAffineWarp (float, float);  
  /** constructor used for sheer only, two other variables are dummies */
  XVAffineWarp (float, float, float);  
  /** constructor used for simultaneous rotation, sheer and scaling */
  XVAffineWarp (float, float, float, float); 
  /** constructor used with an affine matrix **/
  XVAffineWarp (XVAffineMatrix warp_matrix){ amatrix = warp_matrix; };

  /** determines the minimum size of the source image needed for warping */
  XVSize sizeNeeded (const XVImageBase<T> &); 
  void findBounds(const XVCoord2D &, const XVSize &, 
		  XVCoord2D &, XVCoord2D &, XVCoord2D &, XVCoord2D &);
  /** transforms the source image, resulting in target image */
  XVImageBase<T> & warp (const XVImageBase<T> &, XVImageBase<T> &, 
			 const XVCoord2D &, int DIREC = INVERSE);

  /** transforms the source image in reverse, resulting in target image */
  XVImageBase<T> & reverseWarp (const XVImageBase<T> &, XVImageBase<T> &, 
				const XVCoord2D &);

  /** determines matching coordinates in source image and in target image */
  XVCoord2D transform (XVCoord2D);
  /** prints the contents of AMatrix to the screen for testing purposes */
  void print();       
};

#include <XVAffineWarp.icc>
#endif
