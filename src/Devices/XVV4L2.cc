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
#include "XVV4L2.h"

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
int   XVV4L2<T>::set_input(int channel)
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
int XVV4L2<T>::initiate_acquire(int i_frame)
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
int	XVV4L2<T>::get_acquisition_time(int i_frame, struct timeval &time)
{
	time = vidbuf[i_frame].timestamp;

	return 0;
}

static void copy_pixels(XVImageYCbCr &frame,char *mm_buf)
{
   //frame.remap((XV_YCbCr *)mm_buf,false);
}

template <class T>
void copy_pixels(T &frame,char *mm_buf)
{
       XV_YCbCr *ptr=(XV_YCbCr*)mm_buf;
       XVImageWIterator<typename T::PIXELTYPE> wIter(frame);

      for(;!wIter.end();++wIter,ptr++)
       {
         wIter->r=(u_char)(YUV2R(ptr->y0,ptr->u,ptr->v));
         wIter->g=(u_char)(YUV2G(ptr->y0,ptr->u,ptr->v));
         wIter->b=(u_char)(YUV2B(ptr->y0,ptr->u,ptr->v));
         ++wIter;
         wIter->r=(u_char)(YUV2R(ptr->y1,ptr->u,ptr->v));
         wIter->g=(u_char)(YUV2G(ptr->y1,ptr->u,ptr->v));
         wIter->b=(u_char)(YUV2B(ptr->y1,ptr->u,ptr->v));
       }
}

template <class T>
int XVV4L2<T>::wait_for_completion(int i_frame)
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
#ifdef HAVE_IPP
  if ( figure_out_type(*frame(0).data())==XVImage_RGB32) // XV_RGBA32?
  {
    IppiSize roi={size.Width(),size.Height()};
    ippiYCbCr422ToBGR_8u_C2C4R((const Ipp8u *)mm_buf[i_frame],
  		size.Width()*2,(Ipp8u*)(frame(i_frame).lock()),
		size.Width()*4,roi,0);
    frame(i_frame).unlock();
  }
  else
#endif
  {
      //if(figure_out_type(*frame(i_frame).data())==XVImage_YCbCr)
          copy_pixels(frame(i_frame),(char *)mm_buf[i_frame]);
  }
  return 1;
}

template <class T>
int XVV4L2<T>::set_params(char *paramstring)
{
   int 		num_frames=2;
   int		input=1;
   XVParser	parse_result;
   static long norms[3]={V4L2_STD_NTSC_M,V4L2_STD_PAL_D,V4L2_STD_SECAM_D};

   norm=norms[1];
   while(parse_param(paramstring,parse_result)>0)
     switch(parse_result.c)
     {
       case 'I':
         input=parse_result.val;
	 break;
       case 'B': // number of buffers
	 num_frames=parse_result.val;
	 break;
       case 'r': // raw mode
	 raw_mode=parse_result.val;
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
void XVV4L2<T>::close(void)
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
int XVV4L2<T>::open(const char *dev_name,const char *parm_string)
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

  struct v4l2_streamparm setfps;
  memset(&setfps,0,sizeof(struct v4l2_streamparm));
  setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps.parm.capture.timeperframe.numerator=1;
  setfps.parm.capture.timeperframe.denominator=30;
  if((ioctl(fd,VIDIOC_S_PARM,&setfps))==-1)
    perror("SET_FPS failed");
  if(!set_params(parameter)) return 0;
  memset(&fmt,0,sizeof(struct v4l2_format));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width=size.Width();
  fmt.fmt.pix.height=size.Height();
  //fmt.fmt.pix.pixelformat =conv_table[sizeof(typename T::PIXELTYPE)-1];
  fmt.fmt.pix.pixelformat =raw_mode? V4L2_PIX_FMT_MJPEG:V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field        = V4L2_FIELD_ANY;
  //fmt.fmt.pix.bytesperline = 0;
  if(( ioctl (fd, VIDIOC_S_FMT, &fmt))==-1)
    perror("S_FMT in capture_init");

  for(ninputs=0;ninputs<XV_MAX_INPUT_V4L2;ninputs++)
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
  if(figure_out_type(*frame(0).data())==XVImage_YCbCr)
  				size.resize(size.Width()/2,size.Height());
  this->init_map(size,n_buffers);
  if(figure_out_type(*frame(0).data())==XVImage_YCbCr)
  			this->remap(mm_buf,n_buffers);
  int type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl( fd, VIDIOC_STREAMON, &type );
  return 1;
}

template <class T>
XVV4L2<T>::XVV4L2(char const *dev_name,char const *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=XVSize(640,480);
  fd=-1;raw_mode=false;
  open(dev_name,parm_string);
}



template <class T>
XVV4L2<T>::XVV4L2(const XVSize & in_size, char const *dev_name,char const *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=in_size;
  fd=-1; raw_mode=false;
  open(dev_name,parm_string);
  cerr << "finished" << endl;
}


template <class T>
XVV4L2<T>::~XVV4L2()
{
  cerr << "closing device" << endl;
  close();
  fd=-1;
}

template class XVV4L2<XVImageRGB<XV_RGBA32> >;
template class XVV4L2<XVImageRGB<XV_RGB24> >;
template class XVV4L2<XVImageRGB<XV_RGB16> >;
template class XVV4L2<XVImageRGB<XV_RGB15> >;
template class XVV4L2<XVImageYCbCr>;
