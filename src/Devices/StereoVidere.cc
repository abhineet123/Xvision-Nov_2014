#include <math.h>
#include <string.h>
#include "StereoVidere.h"
#include <iostream>

#define CALIB_RELWIDTH     0.5
#define CALIB_RELHEIGHT    0.04
#define DISP_INTERPOLATION 16

template <class T,class Y>  
Y *StereoVidere<T,Y>::calc_display(void)
{
  int size=disp_image.Width()*disp_image.Height();
  int   shift=(s_image->GetSP()->ndisp*4);

  cerr << shift;
  XVImageIterator<short> image_r(disp_image);
  XVImageWIterator<short> image_w(disp_image);
  for(;!image_w.end();++image_w,++image_r)
     *image_w=*image_r>0? *image_r>>shift : 0;
  return &disp_image;
}

template <class T,class Y> 
bool StereoVidere<T,Y>::calc3D(svs3Dpoint * &pts,int &num_pts,char *file_name)
{
   if(!s_image || !s_image->haveDisparity) return false;
   processObject->Calc3D(s_image);
   if(file_name) s_image->Save3DPointArray(file_name);
   pts=s_image->pts3D;
   num_pts=s_image->numPoints;
   return true;
}

template <class T,class Y> 
Y *StereoVidere<T,Y>::calc_stereo(char *dispfile_name)
{
  static Y image;
  calc_warped();

  if(!s_image)
  {
	  cerr << "image timed out" << endl;
          return NULL;
  }
  disp_image.resize (params->width, params->height);
  processObject->CalcStereo(s_image);
  if(dispfile_name) s_image->SaveDisparity(dispfile_name);
  if(!s_image->haveDisparity) {cerr << "no disparity??"<< endl;return NULL;}
  if(!dispInitialized)
  {
    dispInitialized=true;
  }
  disp_image.remap((short*)s_image->disparity,false);
  //disp_image=image;
  //cerr << "cropping " << s_image->dp.dleft << " " << s_image->dp.dtop <<endl;
  //disp_image.setSubImage(s_image->dp.dleft,s_image->dp.dtop,
  //  s_image->dp.dwidth,s_image->dp.dheight);
  return &disp_image;
}

template <class T,class Y> 
void StereoVidere<T,Y>::calc_warped(void)
{
}


template <class T, class Y>
void
StereoVidere<T, Y> :: removeFloor(CALIBRATION_PARAMS & params, 
				  Y & img,
				  int y_offset,
				  int expectedError) {

  // zap the pixels with floor values
  XVImageWIterator<typename Y :: PIXELTYPE> imageIter(img); 

  double v_coordinate = 0.0, floorDisparity = 0.0;

  for(; !imageIter.end(); ++imageIter) {
    // compute expected floor disparity for each line (as opposed to every pixel)
    if(imageIter.currposx() == 0) {
      v_coordinate = getVCoordinate(imageIter.currposy(), y_offset);
      //cerr << "vcoord " << v_coordinate << endl;
      floorDisparity = expectedFloorDisparity(params, v_coordinate);
      //cerr << "expected " << floorDisparity << " " << *imageIter <<endl;
    }

    // if pixel disparity falls within dynamically sized measure +/- expected floor 
    // disparity, consider it floor and zero it out.
    if(abs((int)(*imageIter - floorDisparity)) < expectedError) {
      *imageIter = 0;
    }
  }

  imageIter.close();
};

template <class T, class Y>
CALIBRATION_PARAMS 
StereoVidere<T, Y> :: calibrate(Y & img) {

  double x1, y1, z1;
  double x2, y2, z2, norm;
  int kx1,kx2;

  // shift iterators to correct position in image
  XVImageIterator<typename Y :: PIXELTYPE> imageIterLower(img), imageIterUpper(img);

  imageIterLower.moveBy(0, img.Height() - 3);

  double d_lower = 1.0, d_upper = 1.0;
  int maxLength = (int) (img.Width());

  // initialize histogram
  typename Y :: PIXELTYPE * histogram = 
    new typename Y :: PIXELTYPE[1 << (sizeof(typename Y :: PIXELTYPE) * 8)];

  // clear histogram
  memset(histogram, 0, (1 << (sizeof(typename Y :: PIXELTYPE) * 8)) * sizeof(typename Y :: PIXELTYPE));

  // find max for lower line
  for(int i = 0; i < maxLength-1; ++i, ++imageIterLower) {
    histogram[*imageIterLower]++;

    if(histogram[*imageIterLower] > histogram[(typename Y :: PIXELTYPE) d_lower] 
       && *imageIterLower < 60000 && *imageIterLower != 0) {
      d_lower = *imageIterLower;kx1=i;
    }
  }

  imageIterUpper.moveBy(0, img.Height() - 20);
  // clear histogram
  memset(histogram, 0, (1 << (sizeof(typename Y :: PIXELTYPE) * 8)) * sizeof(typename Y :: PIXELTYPE));

  // find max for upper line
  for(int i = 0; i < maxLength-1; ++i, ++imageIterUpper) {
    histogram[*imageIterUpper]++;

    if(histogram[*imageIterUpper] > histogram[(typename Y :: PIXELTYPE) d_upper] 
       && *imageIterUpper < 60000 && *imageIterUpper != 0) {
      d_upper = *imageIterUpper;kx2=i;
    }
  }

#define Sqr(a) ((a) * (a))

  processObject->CalcPoint3D(
		   kx1+s_image->dp.dleft,img.Height() - 3+s_image->dp.dtop,
		   s_image,
		   &x1, &y1, &z1);

  processObject->CalcPoint3D(
		   kx1+s_image->dp.dleft,img.Height() - 20+s_image->dp.dtop,
		   s_image,
		   &x2, &y2 ,&z2);

  CALIBRATION_PARAMS params;

  norm = sqrt(Sqr(x1 - x2) + Sqr(y1 - y2) + Sqr(z1 - z2));
  params.angle = acos((z2 - z1) / norm);
  params.height = y1 * cos(params.angle) + z1 * sin(params.angle);
  params.angle = M_PI / 2 - params.angle;
  params.d_lower = d_lower;

  delete [] histogram;
  return params;
};

template <class T, class Y>
void StereoVidere<T, Y>:: calcPoint3D(int mx, int my, double& x, double &y, double &z)
{
   processObject->CalcPoint3D(mx, my, s_image, &x, &y, &z);
}

template <class T, class Y>
double
StereoVidere<T, Y> :: getVCoordinate(int currposy,
				     int y_offset) {

  return (double)(Cy()/(getVideoObject()->binning*
                        getVideoObject()->decimation)-
		       (currposy+y_offset+restop()));
};

template <class T, class Y>
double
StereoVidere<T, Y> :: expectedFloorDisparity(CALIBRATION_PARAMS & params,
					     double v_coordinate) {

  // compute gamma from v_coordinate
  double gamma = (v_coordinate*4/ focalLength);
  // compute guessed disparity at particular v
  double d_floor = (baseline / params.height) * 
    (cos(params.angle) - sin(params.angle) * (gamma)) * (focalLength / pixelsize)*DISP_INTERPOLATION/4;

  return d_floor;
};


template <class T,class Y>
void  StereoVidere<T,Y>::LoadImages( XVImageScalar<u_char> &im_l,
	                    XVImageScalar<u_char> &im_r,
			    XVImageRGB<XV_RGBA32> &col,bool rect){ 
     file_image->Load(im_l.Width(),
                      im_l.Height(),
                      (u_char*)im_l.data(),
                      (u_char*)im_r.data(),
                      (u_char*)col.data(),NULL,rect,true);
     s_image=file_image->GetImage(500);

};

template <class T,class Y>
StereoVidere<T,Y>::StereoVidere(const char *dev_name,
				const char *parm_string) : XV_Videre<T>(dev_name,parm_string)
{
  T mode;
  file_image=NULL;
  processObject=new svsStereoProcess();
  file_image=new svsStoredImages;
  //XV_Videre<T>::set_stoc(true); //switch STOC on
  file_image->ReadParams((char *)dev_name);
  file_image->SetRect(true);
  dispInitialized=false;
  baseline = fabs(sourceObject->GetRP()->Tx);
  focalLength = sourceObject->GetRP()->left.f;
  pixelsize = sourceObject->GetRP()->left.dpx;
  cerr << "baseline " << baseline << endl;
  cerr << "foc " << focalLength << endl;
  cerr << "pixelsize " << pixelsize << endl;
  //delete image;
};

template <class T,class Y>
StereoVidere<T,Y>::~StereoVidere()
{
  /*
  if(disp_image) { 
    delete disp_image;
    disp_image = NULL;
    cout << "deleted disp_image" << endl;
  }

  if(resizedDisparityImg) {
    delete resizedDisparityImg;
    resizedDisparityImg = NULL;
    cout << "deleted resizedDisparityImg" << endl;
  }

  if(warped_image[XVVID_RIGHT]) {
    delete warped_image[XVVID_RIGHT];
    warped_image[XVVID_RIGHT] = NULL;
    cout << "deleted warped_image[XVVID_RIGHT]" << endl;
  }

  if(warped_image[XVVID_LEFT]) {
    delete warped_image[XVVID_LEFT];
    warped_image[XVVID_LEFT] = NULL;
    cout << "deleted warped_image[XVVID_LEFT]" << endl;
  }
  */
};

template class StereoVidere<XVImageRGB<XV_RGB16>,XVImageScalar<short>  >;
template class StereoVidere<XVImageRGB<XV_RGB15> ,XVImageScalar<short> >;
template class StereoVidere<XVImageRGB<XV_RGB24> ,XVImageScalar<short> >;
template class StereoVidere<XVImageRGB<XV_TRGB24> ,XVImageScalar<short> >;
template class StereoVidere<XVImageRGB<XV_RGBA32> ,XVImageScalar<short> >;
template class StereoVidere<XVImageRGB<XV_GLRGBA32> ,XVImageScalar<short> >;
