#include "config.h"
#include "XVBlobs.h"
#include "XVImageScalar.h"
#include "Videre.h"

#ifndef STEREO_VIDERE
#define STEREO_VIDERE

struct CALIBRATION_PARAMS {
  double height;          // height of camera
  double angle;           // angle of camera
  double d_lower;         // max disparity of lowest line of disparity map
};

template <class T, class Y>
class StereoVidere : public XV_Videre<T>
{
 protected:
  using XV_Videre<T>::get_stereo;
  using XV_Videre<T>::s_image;
  using XV_Videre<T>::sourceObject;
  using XV_Videre<T>::params;

  bool			dispInitialized;
  svsStereoProcess      *processObject;
  svsStoredImages	*file_image;
  svsDCSAcquireVideo    *dcsObject;
  Y   		        disp_image;

  // camera params
  float baseline, focalLength, pixelsize;

  // dimensions of the disparity image

  // remove floor stuff
  double getVCoordinate(int, int);
  double expectedFloorDisparity(CALIBRATION_PARAMS &, double);

 public:
  StereoVidere(const char *dev_name = VIDERE_DEVICE_NAME,
	       const char *parm_string = NULL);
  ~StereoVidere();
  Y			*calc_stereo(char *dispfile_name=NULL);
  Y			*calc_display(void);
  void                  calc_warped(void);
  XVImageScalar<u_char> **get_warped(){return get_stereo();}
  svsSP getParams(void) { return *(s_image->GetSP()); };
  void  		LoadImages( XVImageScalar<u_char> &im_l,
		                    XVImageScalar<u_char> &im_r,
				    XVImageRGB<XV_RGBA32> &col);
  XVSize size(void) { return XVSize(reswidth(), reslen()); };
  // rect=false   - means rectify the images after loading them
  // rect=true    - images are already rectified
  void     LoadImages( XVImageScalar<u_char> &im_l,
	               XVImageScalar<u_char> &im_r,
	               XVImageRGB<XV_RGBA32> &col,bool rect=false) ;

  // center's y-coordinate in left camera
  double Cy(void) { return sourceObject->GetRP()->left.Cy; };

  // calculate height/tilt of camera
  CALIBRATION_PARAMS calibrate(Y & img);

  // given a frame, remove the floor
  // if an roi is specified, take subimage and remove floor from subimage
  void removeFloor(CALIBRATION_PARAMS & params, Y & img, int y_offset, int expectedError = 25);

  // disparity image's y-offset
  int restop(void)    { return s_image->dp.dtop; };
  int reswidth(void)  { return s_image->dp.dwidth-s_image->dp.dleft; };
  int reslen(void)    { return s_image->dp.dheight;};
  int resleft(void)   { return s_image->dp.dleft;};

  //get 3D information 
  void calcPoint3D(int mx, int my, double& x, double &y, double &z); 
  bool calc3D(svs3Dpoint * &pts,int &num_pts,char *file_name=NULL);
};

#endif
