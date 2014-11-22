#include "config.h"
#include <string.h>
#include <float.h>
#include "XVImageIO.h"
#include "XVStereoRectify.h"

#define Sqr(a) ((a)*(a))

XVColVector 
XVStereoRectify::cross(XVColVector &v1,XVColVector &v2)
{
   XVColVector res(3);

   res[0]=v1[1]*v2[2]-v1[2]*v2[1];
   res[1]=v1[2]*v2[0]-v1[0]*v2[2];
   res[2]=v1[0]*v2[1]-v1[1]*v2[0];
   return res;
}


XVImageScalar<float>
XVStereoRectify::calc_disparity(XVImageScalar<u_char> &image_l,
		                XVImageScalar<u_char> &image_r, 
				bool left_image,int offset)
{
       align_cameras(image_l,image_r,offset);
#ifdef OPENCV_STEREO
       cv::Mat img1(config.height,config.width,CV_8U);
       cv::Mat img2(config.height,config.width,CV_8U);
       memcpy(img1.data,gray_image_l.data(),
			gray_image_l.Width()*gray_image_l.Height());
       memcpy(img2.data,gray_image_r.data(),
			gray_image_r.Width()*gray_image_r.Height());
       sgbm(img1,img2,disp);
       short *rptr=(short*)disp.data;
       float *wptr=dispLeft.lock();
       for(int i=0;i<gray_image_l.Width()*gray_image_l.Height();i++)
           *wptr++=(float)*rptr++;
       dispLeft.unlock();
#else
       stereoUpload(gray_image_l.data(),gray_image_r.data());
       stereoProcess();
       stereoDownload(dispLeft.lock(),dispRight.lock());
       dispLeft.unlock(),dispRight.unlock();
#endif
       
       return left_image? dispLeft : dispRight;
}

void XVStereoRectify::align_cameras(XVImageScalar<u_char> &image_l,
                                   XVImageScalar<u_char> &image_r,
                                   int offset,bool rectify)

{
       int width=config.width;
       int height=config.height;
       IppiSize roi={temp_image1.Width(),temp_image1.Height()};
       IppiRect roi1={0,offset,temp_image1.Width(),temp_image1.Height()};
       bzero(temp_image1.lock(),temp_image1.Width()*temp_image1.Height());
       temp_image1.unlock();
       if(rectify)
         ippiUndistortRadial_8u_C1R((Ipp8u*)image_l.data(), image_l.Width(),
				 temp_image1.lock(),temp_image1.Width(),
				 roi,config.camera_params[0].f[0], 
				 config.camera_params[0].f[1],
				 config.camera_params[0].C[0],
				 config.camera_params[0].C[1],
				 config.camera_params[0].kappa[0],
				config.camera_params[0].kappa[1], DistortBuffer);
       else
         memcpy(temp_image1.lock(),image_l.data(),
                                   image_l.Width()*image_l.Height());
       temp_image1.unlock();
       bzero(gray_image_l.lock(),gray_image_l.Width()*gray_image_l.Height());
       gray_image_l.unlock();
       ippiWarpPerspective_8u_C1R((Ipp8u*)temp_image1.data(),
     				 roi,temp_image1.Width(),roi1,
				 (Ipp8u*)gray_image_l.lock(),
				 gray_image_l.Width(),roi1,coeffs_l,
				 IPPI_INTER_NN);
       gray_image_l.unlock();
       //ippiFilterGauss_8u_C1R((const Ipp8u*)im.data(),im.Width(),
        //                     (Ipp8u*)gray_image_l.lock(),gray_image_l.Width(),
	//		     dst_roi,ippMskSize5x5);
       //gray_image_l.unlock();
       //XVWritePGM(gray_image_l,"im_left.pgm");
       bzero(gray_image_r.lock(),gray_image_r.Width()*gray_image_r.Height());
       temp_image1.unlock();
       if(rectify)
          ippiUndistortRadial_8u_C1R((Ipp8u*)image_r.data(), image_r.Width(),
				 temp_image1.lock(),temp_image1.Width(),
				 roi,config.camera_params[1].f[0], 
				 config.camera_params[1].f[1],
				 config.camera_params[1].C[0],
				 config.camera_params[1].C[1],
				 config.camera_params[1].kappa[0],
			 	 config.camera_params[1].kappa[1], DistortBuffer);

       else
         memcpy(temp_image1.lock(),image_r.data(),
                                   image_r.Width()*image_r.Height());
       temp_image1.unlock();
       //rectify rotation
       bzero(gray_image_r.lock(),gray_image_r.Width()*gray_image_r.Height());
       gray_image_r.unlock();
       ippiWarpPerspective_8u_C1R((Ipp8u*)temp_image1.data(),
      				 roi,temp_image1.Width(),roi1,
				 (Ipp8u*)gray_image_r.lock(),
				 gray_image_r.Width(),roi1,coeffs_r,
				 IPPI_INTER_NN);
       gray_image_r.unlock();
       //ippiFilterGauss_8u_C1R((const Ipp8u*)im.data(),im.Width(),
        //                     (Ipp8u*)gray_image_r.lock(),gray_image_r.Width(),
	//		     dst_roi,ippMskSize5x5);
       //gray_image_r.unlock();
}

bool
XVStereoRectify::calc_3Dpoints(int &num_points,Stereo_3DPoint* &Points3D)
{
  const float 		*r_ptr=dispLeft.data();
  Stereo_3DPoint	*point_ptr=PointBuffer;
  XVMatrix		CorrMat=R_l.t()*K_ideal.i()*B*K_ideal[0][0];

  num_points=config.width*config.height;
  Points3D=PointBuffer;

  memset(PointBuffer,0,config.width*config.height*sizeof(Stereo_3DPoint));
  XVColVector vec(3);

  for(int y=0;y<config.height;y++)
    for(int x=0;x<config.width;x++,r_ptr++,point_ptr++)
    {
       if (*r_ptr<=config.offset*16)
       {
         point_ptr->coord[0]=0,
	 point_ptr->coord[1]=0,
	 point_ptr->coord[2]=0;
         continue; //invalid pixel?
       }
       vec[0]=x,vec[1]=y,vec[2]=1.0;
#ifndef OPENCV_STEREO
       vec=(CorrMat*vec)/(*r_ptr+config.offset);
#else
       vec=(CorrMat*vec)/(*r_ptr/16);
       //cerr << *r_ptr << endl;
       //cerr << vec << endl<< endl;
#endif
       point_ptr->coord[0]=vec[0],
       point_ptr->coord[1]=vec[1],
       point_ptr->coord[2]=vec[2];
    }

  return true;
}

void
XVStereoRectify::calc_rectification_matrix(XVMatrix &ext,XVColVector &T, 
                                          Config &_config,bool rotate)
{
  int width=_config.width,height=_config.height;
  IppiSize roi={width,height};
  IppiRect roi_rect={0,0,width,height};

   if(rotate) cerr << "rotating" << endl;
   XVMatrix rot_90(3,3);
   rot_90=0;
   rot_90[0][1]=-1;
   rot_90[1][0]=1;
   rot_90[2][2]=1;
   //T=ext.t()*T;
   
   XVColVector v1(3),v2(3),v3(3);
   B=sqrt(Sqr(T[0])+Sqr(T[1])+Sqr(T[2]));
   //calculate new Baseline alignment
   if(rotate)
   {
     v2[0]=-T[0]/B,v2[1]=-T[1]/B,v2[2]=-T[2]/B;
     v3[0]=0,v3[1]=0,v3[2]=1;
     v1=cross(v3,v2);
     float norm=sqrt(Sqr(v1[0])+Sqr(v1[1])+Sqr(v1[2]));
     if(norm>1e-7) v1/=norm;
     v3=cross(v1,v2);
     norm=sqrt(Sqr(v3[0])+Sqr(v3[1])+Sqr(v3[2]));
     if(norm>1e-7) v3/=norm;
     R_l[0][0]=v1[0],R_l[0][1]=v1[1],R_l[0][2]=v1[2];
     R_l[1][0]=v2[0],R_l[1][1]=v2[1],R_l[1][2]=v2[2];
     R_l[2][0]=v3[0],R_l[2][1]=v3[1],R_l[2][2]=v3[2];
   }
   else
   {
     v1[0]=T[0]/B,v1[1]=T[1]/B,v1[2]=T[2]/B;
     v2[0]=0,v2[1]=0,v2[2]=1;
     v2=cross(v2,v1);
     float norm=sqrt(Sqr(v2[0])+Sqr(v2[1])+Sqr(v2[2]));
     if(norm>1e-7) v2/=norm;
     v3=cross(v1,v2);
     norm=sqrt(Sqr(v3[0])+Sqr(v3[1])+Sqr(v3[2]));
     if(norm>1e-7) v3/=norm;
     R_l[0][0]=v1[0],R_l[0][1]=v1[1],R_l[0][2]=v1[2];
     R_l[1][0]=v2[0],R_l[1][1]=v2[1],R_l[1][2]=v2[2];
     R_l[2][0]=v3[0],R_l[2][1]=v3[1],R_l[2][2]=v3[2];
   }
   R_r=R_l*ext.t();
   
   if(rotate) R_l=rot_90*R_l,R_r=rot_90*R_r;
   // rectification matrices H
   XVMatrix K(3,3),H;
   // camera matrix
   K=0.0;
   K[0][0]=_config.camera_params[0].f[0];
   K[1][1]=_config.camera_params[0].f[0];
   K[2][2]=1.0;
   // ideal stereo camera
   K_ideal=K;
   K[0][2]=_config.camera_params[0].C[0];
   K[1][2]=_config.camera_params[0].C[1];
   K[1][1]=_config.camera_params[0].f[1];
   K_ideal[0][2]=width/2;
   K_ideal[1][2]=height/2;
   H=R_l*K.i();
   double quad[4][2];
   XVColVector coord(3);
   //project the four boundary corners
   //coord[0]=0,coord[1]=-400,coord[2]=1;
   coord[0]=0,coord[1]=0,coord[2]=1;
   coord=H*coord;
   coord[0]/=coord[2],coord[1]/=coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[0][0]=coord[0],quad[0][1]=coord[1];
   //coord[0]=_config.width,coord[1]=-400,coord[2]=1;
   coord[0]=_config.width,coord[1]=0,coord[2]=1;
   coord=H*coord;
   coord[0]/=coord[2],coord[1]/=coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[1][0]=coord[0],quad[1][1]=coord[1];
   coord[0]=_config.width,coord[1]=_config.height,coord[2]=1;
   coord=H*coord;
   coord[0]/=coord[2],coord[1]/=coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[2][0]=coord[0],quad[2][1]=coord[1];
   coord[0]=0,coord[1]=_config.height,coord[2]=1;
   coord=H*coord;
   coord[0]/=coord[2],coord[1]/=coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[3][0]=coord[0],quad[3][1]=coord[1];
   ippiGetPerspectiveTransform(roi_rect,quad,coeffs_l);
   K[0][0]=_config.camera_params[1].f[0];
   K[1][1]=_config.camera_params[1].f[1];
   K[0][2]=_config.camera_params[1].C[0];
   K[1][2]=_config.camera_params[1].C[1];
   H=R_r*K.i();
   //project the four boundary corners
   //coord[0]=0,coord[1]=-400,coord[2]=1;
   coord[0]=0,coord[1]=0,coord[2]=1;
   coord=H*coord;
   coord[0]=coord[0]/coord[2],coord[1]=coord[1]/coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[0][0]=coord[0],quad[0][1]=coord[1];
   //coord[0]=_config.width,coord[1]=-400,coord[2]=1;
   coord[0]=_config.width,coord[1]=0,coord[2]=1;
   coord=H*coord;
   coord[0]=coord[0]/coord[2],coord[1]=coord[1]/coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[1][0]=coord[0],quad[1][1]=coord[1];
   coord[0]=_config.width,coord[1]=_config.height,coord[2]=1;
   coord=H*coord;
   coord[0]=coord[0]/coord[2],coord[1]=coord[1]/coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[2][0]=coord[0],quad[2][1]=coord[1];
   coord[0]=0,coord[1]=_config.height,coord[2]=1;
   coord=H*coord;
   coord[0]=coord[0]/coord[2],coord[1]=coord[1]/coord[2];coord[2]=1;
   coord=K_ideal*coord;
   quad[3][0]=coord[0],quad[3][1]=coord[1];
   ippiGetPerspectiveTransform(roi_rect,quad,coeffs_r);
}

void 
XVStereoRectify::calc_rectification(Config &_config)
{
  int width=_config.width,height=_config.height;
   XVMatrix ext(3,3);
   XVColVector T(3);
   ext[0][0]=_config.extrinsics[0],
   ext[0][1]=_config.extrinsics[1],
   ext[0][2]=_config.extrinsics[2],
   ext[1][0]=_config.extrinsics[4],
   ext[1][1]=_config.extrinsics[5],
   ext[1][2]=_config.extrinsics[6],
   ext[2][0]=_config.extrinsics[8],
   ext[2][1]=_config.extrinsics[9],
   ext[2][2]=_config.extrinsics[10];
   T[0]=_config.extrinsics[3],T[1]=_config.extrinsics[7],
   T[2]=_config.extrinsics[11];
   calc_rectification_matrix(ext,T,_config);
}


XVStereoRectify::XVStereoRectify(Config & _config)
{
  
   config=_config;
   int width=config.width,height=config.height;
   IppiSize roi={width,height};
   int dist_buf_size;

#ifdef OPENCV_STEREO
   sgbm.preFilterCap = 63;
   sgbm.SADWindowSize = 3;
   sgbm.P1 = 8*sgbm.SADWindowSize*sgbm.SADWindowSize;
   sgbm.P2 = 32*sgbm.SADWindowSize*sgbm.SADWindowSize;
   sgbm.minDisparity = _config.offset;
   sgbm.numberOfDisparities = config.disparity_range;
   sgbm.uniquenessRatio = 15;
   sgbm.speckleWindowSize = 100;
   sgbm.speckleRange = 32;
   sgbm.disp12MaxDiff = 3;
   sgbm.fullDP = true;
#else
   stereoInit(config.width,config.height);
#endif

   PointBuffer=new Stereo_3DPoint[config.width*config.height];
   if(!PointBuffer) throw 1;
   R_l.resize(3,3), R_r.resize(3,3);

   dispLeft.resize(config.width,config.height);
   dispRight.resize(config.width,config.height);
   gray_image_l.resize(config.width,config.height);
   gray_image_r.resize(config.width,config.height);
   temp_image1.resize(width,height);
   ippiUndistortGetSize(roi,&dist_buf_size);
   DistortBuffer=new Ipp8u[dist_buf_size];
   IppiSize roi_gauss={config.height,config.height};
   int gauss_size; 
   calc_rectification(_config);
}

XVStereoRectify::~XVStereoRectify()
{
  delete [] DistortBuffer;
  delete [] PointBuffer;
}
