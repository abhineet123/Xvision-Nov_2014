#include "config.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <XVImageScalar.h> //jcorso for monochrome operation
#include <XVImageYCbCr.h>
#include "XVPwc.h"
#include "pwc-ioctl.h"

#ifdef HAVE_IPP
#include <ippcc.h>
#endif

#include <XVMacros.h>


extern int debug;

static int conv_table[4]=
{
     V4L2_PIX_FMT_GREY,V4L2_PIX_FMT_RGB565,V4L2_PIX_FMT_BGR24,
     V4L2_PIX_FMT_BGR32};

template <class T>
int   XVPwc<T>::set_input(int channel)
{
   if(fd<0) return 0;
   if(ioctl(fd, VIDIOC_S_INPUT, &channel)==-1)
   {
     perror("ioctl VIDIOCSCHAN");
     return 0;
   }
   return 1;
}

template <class T>
int XVPwc<T>::initiate_acquire(int i_frame)
{
  if(fd<0) return 0;
  if(i_frame <0 || i_frame>n_buffers)
  {
    cerr << "scheduled invalid frame number " << i_frame<< endl;
    return 0;
  }

  vidbuf[i_frame].index=i_frame;
  vidbuf[i_frame].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vidbuf[i_frame].memory=V4L2_MEMORY_MMAP;
  if(ioctl(fd,VIDIOC_QBUF,&(vidbuf[i_frame]),0))
  {
    perror("ioctl schedule frame");
    return 0;
  }
  return 1;
}


template <class T>
int XVPwc<T>::wait_for_completion(int i_frame)
{
  fd_set rdset;
  struct timeval tv;
  struct v4l2_buffer buf;

  if(fd<0) return 0;
  if(i_frame <0 || i_frame>n_buffers)
  {
    cerr << "invalid frame number " << i_frame<< endl;
    return 0;
  }


  //memset(&(vidbuf[i_frame]),0,sizeof(struct v4l2_buffer));
  vidbuf[i_frame].index = i_frame;
  vidbuf[i_frame].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vidbuf[i_frame].memory=V4L2_MEMORY_MMAP;
  if(ioctl(fd,VIDIOC_DQBUF,&(vidbuf[i_frame])))
  {
    perror("ioctl VIDIOCSYNC");
    return 0;
  }
  return 1;
}

template <class T>
int	XVPwc<T>::get_acquisition_time(int i_frame, struct timeval &time)
{
	time = vidbuf[i_frame].timestamp;

	return 0;
}

template <class T>
int XVPwc<T>::set_shutter(int shutter)
{
   return ioctl(fd,VIDIOCPWCSSHUTTER,&shutter);
}

template <class T>
int XVPwc<T>::get_agc(int &agc)
{
   return ioctl(fd,VIDIOCPWCGAGC,&agc);
}

template <class T>
int XVPwc<T>::set_agc(int agc)
{
   return ioctl(fd,VIDIOCPWCSAGC,&agc);
}

template <class T>
int XVPwc<T>::set_params(char *paramstring)
{

   int 		num_frames=2;
   int		input=1;
   XVParser	parse_result;
   static long norms[3]={V4L2_STD_NTSC_M,V4L2_STD_PAL_D,V4L2_STD_SECAM_D};

   norm=norms[1];
   rate=15;
   while(parse_param(paramstring,parse_result)>0)
     switch(parse_result.c)
     {
       case 'I':
         input=parse_result.val;
	 break;
       case 'B': // number of buffers
	 num_frames=parse_result.val;
	 break;
       case 'r': // rate
	 rate=parse_result.val;
	 break;
       case 'N': // norm
         if(parse_result.val<0 || parse_result.val>2)
           cerr << "Unsupported norm" << endl;
         else
           norm=norms[parse_result.val];
         break;
       default:
	 cerr << parse_result.c << "=" << parse_result.val
	      << " is not supported by V4L2 (skipping)" << endl;
     }

   if(fd<0)
   {
     cerr << "Device is not open ...." << endl;
     exit(1);
   }
   req.count = num_frames;
   req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   req.memory=V4L2_MEMORY_MMAP;
   if(ioctl (fd, VIDIOC_REQBUFS,&req)==-1)
   {
     perror("REQBUFS in capture _start\n");
     throw 22;
   }
   n_buffers=num_frames;
   set_input(input);
   return 1;
}

template <class T>
void XVPwc<T>::close(void)
{
  if(fd>=0)
  {
    for(int i=0;i<req.count;i++)
      if(mm_buf[i]) munmap(mm_buf[i],vidbuf[i].length);
    ::close(fd);
  }
  fd=-1;
}

template <class T>
int XVPwc<T>::open(const char *dev_name,const char *parm_string)
{
  struct v4l2_streamparm parm;
  static char parameter[200];
  int i;

  if(parm_string)
    strcpy(parameter,parm_string);
  else
    parameter[0]=0;
  if(fd>-1) close();  // close old stuff first ???
  if((fd=::open(dev_name,O_RDWR))==-1)
  {
    cerr << "couldn't open device " << dev_name << endl;
    throw 10;
  }
  memset(&capability,0,sizeof(struct v4l2_capability));
  if((ioctl (fd, VIDIOC_QUERYCAP, &capability)))
  {
     perror ("QUERYCAP in capture_init");
     throw 11;
  }
  if (!(capability.capabilities & V4L2_CAP_STREAMING)) throw 11;
  // set video parameters
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;

  //if((ioctl (fd, VIDIOC_G_STD, &norm)))
  //  perror ("G_STD in capture_init");

  int agc=-1;                        // gain - negative is auto
  ioctl(fd, VIDIOCPWCSAGC, &agc);
  int shutter=40000;
  ioctl(fd,  VIDIOCPWCSSHUTTER, &shutter);

  if(!set_params(parameter)) return 0;
  struct video_window vwin;
  ioctl(fd, VIDIOCGWIN, &vwin);
  vwin.flags &= ~PWC_FPS_FRMASK;
  vwin.flags |= (rate << PWC_FPS_SHIFT);  // framerate
  ioctl(fd, VIDIOCSWIN, &vwin);
  memset(&fmt,0,sizeof(struct v4l2_format));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width=size.Width();
  fmt.fmt.pix.height=size.Height();
  fmt.fmt.pix.pixelformat =0x32315559;
  fmt.fmt.pix.field        = V4L2_FIELD_ANY;
  //fmt.fmt.pix.bytesperline = 480;
  if(( ioctl (fd, VIDIOC_S_FMT, &fmt))==-1)
    perror("S_FMT in capture_init");

  for(ninputs=0;ninputs<XV_MAX_INPUT_PWC;ninputs++)
  {
     inp[ninputs].index=ninputs;
     if(ioctl(fd,VIDIOC_ENUMINPUT,&(inp[ninputs]))==-1) break;
  }
  if(ioctl(fd,VIDIOC_S_STD,&norm))
    perror("S_STD error");
  // mmap buffers into user space
  for(int j=0;j<req.count;j++)
  {
    vidbuf[j].index=j;
    vidbuf[j].type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vidbuf[j].memory=V4L2_MEMORY_MMAP;
    if(ioctl (fd, VIDIOC_QUERYBUF, &(vidbuf[j]))==-1)
    {
       perror ("QUERYBUF in capture_start");
       return 0;
    }

    mm_buf[j]=(typename T::PIXELTYPE *)mmap((void *)0,vidbuf[j].length,
  			PROT_READ,MAP_SHARED,fd,vidbuf[j].m.offset);
  }
  //if(figure_out_type(*frame(0).data())==XVImage_YCbCr)
  //				size.resize(size.Width(),size.Height());
  this->init_map(size,n_buffers);
  this->remap(mm_buf,n_buffers);
  int type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl( fd, VIDIOC_STREAMON, &type );
  return 1;
}

template <class T>
int XVPwc<T>::set_brightness(int brightness)
{
   control.id =   V4L2_CID_BRIGHTNESS;
   control.value =brightness;
   return ioctl (fd, VIDIOC_S_CTRL, &control);
}

template <class T>
int XVPwc<T>::set_contrast(int contrast)
{
   control.id =   V4L2_CID_CONTRAST;
   control.value =contrast;
   return ioctl (fd, VIDIOC_S_CTRL, &control);
}

template <class T>
int XVPwc<T>::set_exposure(int exposure)
{
   control.id =   V4L2_CID_EXPOSURE;
   control.value =exposure;
   return ioctl (fd, VIDIOC_S_CTRL, &control);
}

template <class T>
int XVPwc<T>::set_gain(int gain)
{
   if(gain==-1)
   {
	   control.id =   V4L2_CID_AUTOGAIN;
	   control.value = (bool)1;
	   return ioctl (fd, VIDIOC_S_CTRL, &control);
   }
   else
   {
	   control.id =   V4L2_CID_AUTOGAIN;
	   control.value = (bool)0;
	   ioctl (fd, VIDIOC_S_CTRL, &control);
	   control.id =   V4L2_CID_GAIN;
	   control.value = gain;
	   return ioctl (fd, VIDIOC_S_CTRL, &control);
   }
}

template <class T>
XVPwc<T>::XVPwc(char const *dev_name,char const *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=XVSize(640,480);
  fd=-1;
  open(dev_name,parm_string);
}



template <class T>
XVPwc<T>::XVPwc(const XVSize & in_size, char const *dev_name,char const *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=in_size;
  fd=-1;
  open(dev_name,parm_string);
  cerr << "finished" << endl;
}


template <class T>
XVPwc<T>::~XVPwc()
{
  cerr << "closing device" << endl;
  close();
  fd=-1;
}

template class XVPwc<XVImageScalar<u_char> >;
