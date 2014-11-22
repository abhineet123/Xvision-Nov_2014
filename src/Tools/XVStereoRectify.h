#ifndef _XVSTEREORECTIFY_H_
#define _XVSTEREORECTIFY_H_
#include "ippi.h"
#include "ippcv.h"
#include "XVMatrix.h"
#include "XVImageScalar.h"
#include "XVStereoRectifyTypes.h"
//#include "Stereo.h"
#include "camera_config.h"


#define OPENCV_STEREO

#ifdef OPENCV_STEREO
//#include <opencv/cvaux.hpp>

//#include <opencv/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <stdio.h>
#endif


#define MAX_STEREO_WIDTH   1288
#define MAX_STEREO_HEIGHT  964

using namespace std;

class XVStereoRectify
{
   private:
     XVMatrix		   R_l,R_r; // rotation between ideal and real
     double 		   coeffs_l[3][3];
     double 		   coeffs_r[3][3];
     float       	   B;

     XVImageScalar<float>  dispLeft, dispRight;
     Stereo_3DPoint	   *PointBuffer;
     Ipp8u  		   *DistortBuffer;
     Config		   config;
     XVMatrix 	           K_ideal;
     XVColVector           cross(XVColVector &v1,XVColVector &v2);
     XVImageScalar<u_char> temp_image1;
#ifdef OPENCV_STEREO
     cv::StereoSGBM 	   sgbm;
     cv::Mat 		   disp;
#endif
   public:
     XVImageScalar<u_char> gray_image_l,gray_image_r;
     void                  align_cameras(XVImageScalar<u_char> &image_l,
                                         XVImageScalar<u_char> &image_r,
					 int offset=0,bool rectify=true);
     XVImageScalar<float>  calc_disparity(XVImageScalar<u_char> &image_l,
     					  XVImageScalar<u_char> &image_r,
					  bool left_image=true,
					  int offset=0) ;
     bool		   calc_3Dpoints(int &num_points,
     					Stereo_3DPoint * &Points3D);
     void		   calc_rectification(Config &_config);
     void		   calc_rectification_matrix(XVMatrix &ext,
                                                     XVColVector &T,
    				                     Config &config,
						     bool rotate=false);
      XVMatrix              get_camera_rotation(bool which)
                              {return which? R_r : R_l;};

     XVStereoRectify(Config & _config);
     ~XVStereoRectify();
};
#endif
