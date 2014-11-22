// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVDIG1394_H_
#define _XVDIG1394_H_
#ifdef HAVE_IPP
#include <ippi.h>
#include <ippcc.h>
#endif

#define __user 
#include <sys/types.h>
#include <video1394.h>
//#include <libraw1394/raw1394.h>
#include "dc1394/dc1394.h"
#include "config.h"
#include <XVVideo.h>
#include <list>
#include <pthread.h>

#ifdef HAVE_VIDEO1394_NEW
#define DC_DEVICE_NAME	"/dev/video1394/0"
#else
#define DC_DEVICE_NAME	"/dev/video1394"
#endif
#define DIG_DEF_NUMFRAMES 4
#define DIG_DEF_INPUT     0

#define DIG1394_FIRST_CAMERA   (~(unsigned long long)0)
#define DIG1394_SECOND_CAMERA  (~(unsigned long long)1)
#define DIG1394_THIRD_CAMERA   (~(unsigned long long)2)
#define DIG1394_FOURTH_CAMERA  (~(unsigned long long)3)
#define DIG1394_FIFTH_CAMERA   (~(unsigned long long)4)
#define DIG1394_SIXTH_CAMERA   (~(unsigned long long)5)
#define DIG1394_SEVENTH_CAMERA (~(unsigned long long)6)
#define DIG1394_EIGHTH_CAMERA  (~(unsigned long long)7)

#define DIG1394_NTH_CAMERA(n)  (~(unsigned long long)(n))

/*
  parameter format for XVDig1394::set_params is a string of "XnXn..."
  where X is the parameter character and n is the corresponding value
  valid characters and their values:
  B : number of buffers ( same as defined in XVVideo.h )
  R : pixel format from camera, 0 = YUV422, 1 = RGB, 2 = MONO8,
                                3 = MONO16, 4 = YUV411, 5 = YUV444
				default = 0 
  S : scale, 0 = any, 1 = 640x480, 2 = 320x200, 3 = 160x120,
             4 = 800x600, 5 = 1024x768, 6 = 1280x960, 7 = 1600x1200
	     default = 0
  M : scale (obsolete), M0 = S4, M1 = S5, M2 = S6, M3 = S7
  C : whether to grab center of the image or not, 0 = no, 1 = yes, default = 0
      note that C1 implies format 7
  T : external trigger, 0 = off, 1-4 = mode 0-3, or directly supply the
                        number to the camera. default = 0
  f : directly set IEEE 1394 camera format (as oppose to set R and S/M)
      if the value is 7, S values are still used.
  m : directly set IEEE 1394 camera mode   (as oppose to set R and S/M)
      note that this parameter should not to set for format 7
  r : frame rate, 0 = 1.875, 1 = 3.75, 2 = 7.5, 3 = 15, 4 = 30, 5 = 60 (fps)
                  default = fastest under selected format and mode
  g : gain
  u : u component for the white balance
  v : v component for the white balance
  s : saturation
  A : sharpness
  h : shutter
  a : gamma
  x : exposure
  o : optical filter
  i : bus init (resets the IEEE bus)
 */


typedef std::list<int> Jobs;


template <class IMTYPE>
class XVDig1394 : public XVVideo<IMTYPE> {
 protected:
  using XVVideo<IMTYPE>::size ;
  using XVVideo<IMTYPE>::image_buffers ;
  using XVVideo<IMTYPE>::n_buffers ;
  using XVVideo<IMTYPE>::current_buffer ;
  using XVVideo<IMTYPE>::init_map ;
  using XVVideo<IMTYPE>::parse_param ;

 private:
   int                   ext_trigger ;
   int                   format ;
   int                   mode ;
   int                   framerate ;
   int                   gain;
   int                   saturation;
   int                   sharpness ;
   int                   shutter ;
   int                   gamma;
   int                   exposure;
   int                   uv[2];
   int			 channel;
   int			 scale;
   int			 mode_type;
   int			 rgb_pixel;
   bool			 optical_flag;
   bool			 reset_ieee;
   bool                  grab_center ;
   bool			 format7;
   bool			 verbose;
   dc1394color_filter_t  optical_filter;
   int                   nowait_flag;
   int                   buffer_index;
   unsigned char	 * mm_buf[DIG_DEF_NUMFRAMES];
   const char            * device_name;
   dc1394camera_t        *camera_node;
   dc1394_t * 		 fd;
   pthread_t		 grabber_thread;
   pthread_mutex_t       wait_grab[DIG_DEF_NUMFRAMES];
   bool			 threadded;
   static void			 *grab_thread(void *obj);
   Jobs			  requests; 
   pthread_mutex_t job_mutex;
  public:
   using XVVideo<IMTYPE>::frame ;
   // camera_id can either be the 64-bit id of the camera, or one
   // of the DIG1394_*_CAMERA macro defined above
   		XVDig1394(const char *dev_name=DC_DEVICE_NAME,
		      const char *parm_string=NULL, unsigned long long
		      camera_id=0);
   virtual	~XVDig1394();
   // Video_h compatibility functions
   int          open(const char *dev_name);
   void		close(void);
   IMTYPE 		& current_frame_continuous();

   virtual int		set_params(char *param_string);
   virtual int		initiate_acquire(int frame);
   virtual int		wait_for_completion(int frame);
   int         get_buffer_index(){return buffer_index;}

   // Wrappers for online control of camera parameters
   inline int  set_optical_filter(int f) 
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_OPTICAL_FILTER,f);}
   inline int  set_feature_manual(dc1394feature_t f) 
                 {return dc1394_feature_set_mode(camera_node,
		 f,DC1394_FEATURE_MODE_MANUAL);}
   inline int  set_feature_auto(dc1394feature_t f)
                 {return dc1394_feature_set_mode(camera_node,
		 f,DC1394_FEATURE_MODE_AUTO);}
   inline int  set_brightness_manual()
                    {return set_feature_manual(DC1394_FEATURE_BRIGHTNESS);}
   inline int  set_exposure_manual()
                    {return set_feature_manual(DC1394_FEATURE_EXPOSURE);}
   inline int  set_sharpness_manual()
                    {return set_feature_manual(DC1394_FEATURE_SHARPNESS);}
   inline int  set_whitebalance_manual()
                    {return set_feature_manual(DC1394_FEATURE_WHITE_BALANCE);}
   inline int  set_saturation_manual()
                    {return set_feature_manual(DC1394_FEATURE_SATURATION);}
   inline int  set_gamma_manual()
                    {return set_feature_manual(DC1394_FEATURE_GAMMA);}
   inline int  set_gain_manual()
                    {return set_feature_manual(DC1394_FEATURE_GAIN);}
   inline int  set_shutter_manual()
                    {return set_feature_manual(DC1394_FEATURE_SHUTTER);}
   inline int  set_iris_manual()
                    {return set_feature_manual(DC1394_FEATURE_IRIS);}
   inline int  set_hue_manual()
                    {return set_feature_manual(DC1394_FEATURE_HUE);}
   inline int  set_capturequality_manual()
                    {return set_feature_manual(DC1394_FEATURE_CAPTURE_QUALITY);}
   inline int  set_brightness_auto()
                    {return set_feature_auto(DC1394_FEATURE_BRIGHTNESS);}
   inline int  set_exposure_auto()
                    {return set_feature_auto(DC1394_FEATURE_EXPOSURE);}
   inline int  set_sharpness_auto()
                    {return set_feature_auto(DC1394_FEATURE_SHARPNESS);}
   inline int  set_whitebalance_auto()
                    {return set_feature_auto(DC1394_FEATURE_WHITE_BALANCE);}
   inline int  set_saturation_auto()
                    {return set_feature_auto(DC1394_FEATURE_SATURATION);}
   inline int  set_gamma_auto()
                    {return set_feature_auto(DC1394_FEATURE_GAMMA);}
   inline int  set_gain_auto()
                    {return set_feature_auto(DC1394_FEATURE_GAIN);}
   inline int  set_shutter_auto()
                    {return set_feature_auto(DC1394_FEATURE_SHUTTER);}
   inline int  set_iris_auto()
                    {return set_feature_auto(DC1394_FEATURE_IRIS);}
   inline int  set_hue_auto()
                    {return set_feature_auto(DC1394_FEATURE_HUE);}
   inline int  set_capturequality_auto()
                    {return set_feature_auto(DC1394_FEATURE_CAPTURE_QUALITY);}
   inline int  set_brightness(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_BRIGHTNESS,val);}
   inline int  set_exposure(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_EXPOSURE,val);}
   inline int  set_sharpness(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_SHARPNESS,val);}
   inline int  set_white_balance(unsigned int val,unsigned int val2)
                 {return dc1394_feature_whitebalance_set_value
		  (camera_node,val,val2);}
   inline int  set_saturation(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_SATURATION,val);}
   inline int  set_gamma(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_GAMMA,val);}
   inline int  set_gain(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_GAIN,val);}
   inline int  set_shutter(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_SHUTTER,val);}
   inline int  set_iris(unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_IRIS,val);}
   inline int  set_hue( unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_HUE,val);}
   inline int  set_capturequality( unsigned int val)
                 {return dc1394_feature_set_value(camera_node,
		 DC1394_FEATURE_CAPTURE_QUALITY,val);}
   inline int  get_brightness(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_BRIGHTNESS,val);}
   inline int  get_exposure(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_EXPOSURE,val);}
   inline int  get_sharpness(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_SHARPNESS,val);}
   inline int  get_whitebalance(unsigned int *val,unsigned int *val2)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_WHITE_BALANCE,val);}
   inline int  get_saturation(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_SATURATION,val);}
   inline int  get_gamma(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_GAMMA,val);}
   inline int  get_gain(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_GAIN,val);}
   inline int  get_shutter(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_SHUTTER,val);}
   inline int  get_iris(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_IRIS,val);}
   inline int  get_hue(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_HUE,val);}
   inline int  get_capturequality(unsigned int *val)
                 {return dc1394_feature_get_value(camera_node,
		 DC1394_FEATURE_CAPTURE_QUALITY,val);}
};

#endif
