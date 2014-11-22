// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVDIG1394_H_
#define _XVDIG1394_H_

#define __user 

#include <sys/types.h>
#include <video1394.h>
#include <libraw1394/raw1394.h>
#include "libdc1394/dc1394_control.h"
#include "config.h"
#include <XVVideo.h>

// linux-2.6 kernel compatibility (kind of), Darius
#ifdef VIDEO1394_IOC_LISTEN_QUEUE_BUFFER
#define VIDEO1394_LISTEN_QUEUE_BUFFER VIDEO1394_IOC_LISTEN_QUEUE_BUFFER
#define VIDEO1394_LISTEN_WAIT_BUFFER VIDEO1394_IOC_LISTEN_WAIT_BUFFER
#define VIDEO1394_UNLISTEN_CHANNEL VIDEO1394_IOC_UNLISTEN_CHANNEL
#define VIDEO1394_LISTEN_CHANNEL VIDEO1394_IOC_LISTEN_CHANNEL
#define VIDEO1394_LISTEN_POLL_BUFFER VIDEO1394_IOC_LISTEN_POLL_BUFFER
#endif

#ifdef HAVE_VIDEO1394_DIR
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
 */

typedef enum
{
  BAYER_Not_Valid=-1,
  BAYER_PATTERN_GBRG,
  BAYER_PATTERN_BGGR,
  BAYER_PATTERN_RGGB,
  BAYER_PATTERN_GRBG,
}bayer_pattern_type;

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
   int			 fd;
   int			 raw_fd;
   int			 node_id;
   int			 camera_id;
   int			 format ;
   bool                  format7 ;
   int			 mode ;
   int                   framerate ;
   int                   ext_trigger ;
   bool                  grab_center ;
   int			 gain;
   int			 saturation;
   int                   sharpness ;
   int                   shutter ; 
   int                   gamma;
   int                   exposure;
   int			 uv[2];
   bayer_pattern_type    optical_filter;
   int			 nowait_flag;
   int			 buffer_index;
   raw1394handle_t 	 handle;
   struct video1394_mmap v_mmap;
   unsigned char	 * mm_buf[DIG_DEF_NUMFRAMES];
   const char            * device_name;
   dc1394_cameracapture  camera;
   nodeid_t              * camera_nodes;
   void                  (*convert)( const unsigned char *, IMTYPE&, int );

  public:
   using XVVideo<IMTYPE>::frame ;
   // camera_id can either be the 64-bit id of the camera, or one
   // of the DIG1394_*_CAMERA macro defined above
   		XVDig1394(const char *dev_name=DC_DEVICE_NAME,
		      const char *parm_string=NULL, unsigned long long camera_id=0);
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
                 {return dc1394_set_optical_filter(handle,node_id,f);}
   inline int  set_feature_manual(int f) 
                    {return dc1394_auto_on_off(handle,node_id,f,0);}
   inline int  set_feature_auto(int f)
                    {return dc1394_auto_on_off(handle,node_id,f,1);}
   inline int  set_brightness_manual()
                    {return set_feature_manual(FEATURE_BRIGHTNESS);}
   inline int  set_exposure_manual()
                    {return set_feature_manual(FEATURE_EXPOSURE);}
   inline int  set_sharpness_manual()
                    {return set_feature_manual(FEATURE_SHARPNESS);}
   inline int  set_whitebalance_manual()
                    {return set_feature_manual(FEATURE_WHITE_BALANCE);}
   inline int  set_saturation_manual()
                    {return set_feature_manual(FEATURE_SATURATION);}
   inline int  set_gamma_manual()
                    {return set_feature_manual(FEATURE_GAMMA);}
   inline int  set_gain_manual()
                    {return set_feature_manual(FEATURE_GAIN);}
   inline int  set_shutter_manual()
                    {return set_feature_manual(FEATURE_SHUTTER);}
   inline int  set_iris_manual()
                    {return set_feature_manual(FEATURE_IRIS);}
   inline int  set_hue_manual()
                    {return set_feature_manual(FEATURE_HUE);}
   inline int  set_capturequality_manual()
                    {return set_feature_manual(FEATURE_CAPTURE_QUALITY);}
   inline int  set_brightness_auto()
                    {return set_feature_auto(FEATURE_BRIGHTNESS);}
   inline int  set_exposure_auto()
                    {return set_feature_auto(FEATURE_EXPOSURE);}
   inline int  set_sharpness_auto()
                    {return set_feature_auto(FEATURE_SHARPNESS);}
   inline int  set_whitebalance_auto()
                    {return set_feature_auto(FEATURE_WHITE_BALANCE);}
   inline int  set_saturation_auto()
                    {return set_feature_auto(FEATURE_SATURATION);}
   inline int  set_gamma_auto()
                    {return set_feature_auto(FEATURE_GAMMA);}
   inline int  set_gain_auto()
                    {return set_feature_auto(FEATURE_GAIN);}
   inline int  set_shutter_auto()
                    {return set_feature_auto(FEATURE_SHUTTER);}
   inline int  set_iris_auto()
                    {return set_feature_auto(FEATURE_IRIS);}
   inline int  set_hue_auto()
                    {return set_feature_auto(FEATURE_HUE);}
   inline int  set_capturequality_auto()
                    {return set_feature_auto(FEATURE_CAPTURE_QUALITY);}
   inline int  set_brightness(unsigned int val)
                {return dc1394_set_brightness(handle,node_id,val);}
   inline int  set_exposure(unsigned int val)
                {return dc1394_set_exposure(handle,node_id,val);}
   inline int  set_sharpness(unsigned int val)
                {return dc1394_set_sharpness(handle,node_id,val);}
   inline int  set_whitebalance(unsigned int val,unsigned int val2)
                {return dc1394_set_white_balance(handle,node_id,val,val2);}
   inline int  set_saturation(unsigned int val)
                {return dc1394_set_saturation(handle,node_id,val);}
   inline int  set_gamma(unsigned int val)
                {return dc1394_set_gamma(handle,node_id,val);}
   inline int  set_gain(unsigned int val)
                {return dc1394_set_gain(handle,node_id,val);}
   inline int  set_shutter(unsigned int val)
                {return dc1394_set_shutter(handle,node_id,val);}
   inline int  set_iris(unsigned int val)
                {return dc1394_set_iris(handle,node_id,val);}
   inline int  set_hue( unsigned int val)
                {return dc1394_set_hue(handle,node_id,val);}
   inline int  set_capturequality( unsigned int val)
                {return dc1394_set_capture_quality(handle,node_id,val);}
   inline int  get_brightness(unsigned int *val)
                {return dc1394_get_brightness(handle,node_id,val);}
   inline int  get_exposure(unsigned int *val)
                {return dc1394_get_exposure(handle,node_id,val);}
   inline int  get_sharpness(unsigned int *val)
                {return dc1394_get_sharpness(handle,node_id,val);}
   inline int  get_whitebalance(unsigned int *val,unsigned int *val2)
                {return dc1394_get_white_balance(handle,node_id,val,val2);}
   inline int  get_saturation(unsigned int *val)
                {return dc1394_get_saturation(handle,node_id,val);}
   inline int  get_gamma(unsigned int *val)
                {return dc1394_get_gamma(handle,node_id,val);}
   inline int  get_gain(unsigned int *val)
                {return dc1394_get_gain(handle,node_id,val);}
   inline int  get_shutter(unsigned int *val)
                {return dc1394_get_shutter(handle,node_id,val);}
   inline int  get_iris(unsigned int *val)
                {return dc1394_get_iris(handle,node_id,val);}
   inline int  get_hue(unsigned int *val)
                {return dc1394_get_hue(handle,node_id,val);}
   inline int  get_capturequality(unsigned int *val)
                {return dc1394_get_capture_quality(handle,node_id,val);}
   inline int  get_node_id()
                {return node_id;}
   inline raw1394handle_t get_handle()
                {return handle;}
};

#endif
