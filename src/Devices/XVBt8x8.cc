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
#include <XVBt8x8.h>
#include <XVMacros.h>

using namespace std ;

extern int debug;
template <class T>
int   XVBt8x8<T>::set_input(int norm,int channel)
{
   channels[channel].norm=norm;
   if(fd<0) return 0;
   if(ioctl(fd, VIDIOCSCHAN, &(channels[channel]))==-1)
   {
     perror("ioctl VIDIOCSCHAN");
     return 0;
   }
   return 1;
}

template <class T>
int XVBt8x8<T>::initiate_acquire(int i_frame)
{
  if(fd<0) return 0;
  if(i_frame <0 || i_frame>n_buffers)
  {
    cerr << "invalid frame number" << endl;
    return 0;
  }
  assert(frame(i_frame).lock());
  frames[i_frame].format=set_format;
  if(ioctl(fd,VIDIOCMCAPTURE,&(frames[i_frame]))==-1)
  {
    assert(frame(i_frame).unlock());
    perror("ioctl VIDIOCMCAPTURE");
    return 0;
  }
  return 1;
}

template <class T>
int XVBt8x8<T>::wait_for_completion(int i_frame)
{
  if(fd<0) return 0;
  if(i_frame <0 || i_frame>n_buffers)
  {
    cerr << "invalid frame number" << endl;
    return 0;
  }
  if(ioctl(fd,VIDIOCSYNC,&(frames[i_frame].frame))==-1)
  {
    perror("ioctl VIDIOCSYNC");
    return 0;
  }
  assert(frame(i_frame).unlock());
  return 1;
}

template <class T>
int XVBt8x8<T>::set_params(char *paramstring)
{
   int 		num_frames=gb_buffers.frames;
   int 		input=BT_DEF_INPUT,
                norm=BT_DEF_NORM,i,format;
   XVParser	parse_result;
   static int norms[3]={VIDEO_MODE_NTSC,VIDEO_MODE_PAL,
                        VIDEO_MODE_SECAM};
   static int conv_table[4]=
   {VIDEO_PALETTE_GREY,VIDEO_PALETTE_RGB565,VIDEO_PALETTE_RGB24,
         VIDEO_PALETTE_RGB32};

   brightness=-1;hue=-1;contrast=-1;
   while(parse_param(paramstring,parse_result)>0)
     switch(parse_result.c)
     {
       case 'h':
         hue=parse_result.val;
	 break;
       case 'c':
         contrast=parse_result.val;
	 break;
       case 'b':
         brightness=parse_result.val;
	 break;
       case 'N':  // norm selection
	 assert(norm>=0 && norm<3);
	 norm=norms[parse_result.val];
	 break;
       case 'I':  // input selection
         input=parse_result.val;
	 break;
       case 'B': // number of buffers
	 num_frames=parse_result.val;
	 break;
       default:
	 cerr << parse_result.c << "=" << parse_result.val
	      << " is not supported by Bt8x8 (skipping)" << endl;
     }

   if(fd<0)
   {
     cerr << "Device is not open ...." << endl;
     exit(1);
   }
   format=conv_table[sizeof(typename T::PIXELTYPE)-1];
   if(num_frames>gb_buffers.frames)
   {
      cerr << "param error - only " << gb_buffers.frames
           << " frames compiled in driver" << endl;
      exit(1);
   }
   if(frames) delete[] frames;
   frames=new struct video_mmap[num_frames];
   set_format=format;
   for(i=0;i<num_frames;i++)
   {
      frames[i].format=format;
      frames[i].frame=i;
      frames[i].width=size.Width();
      frames[i].height=size.Height();
   }
   n_buffers=num_frames;
   set_input(norm,input);
   return 1;
}

template <class T>
void XVBt8x8<T>::close(void)
{
  if(fd>=0)
  {
    munmap(mm_buf[0],gb_buffers.size);
    ::close(fd);
  }
  fd=-1;
}

template <class T>
int XVBt8x8<T>::open(const char *dev_name,const char *parm_string)
{
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
    exit(1);
  }
  if(ioctl(fd,VIDIOCGCAP,&capability)==-1)
  {
    cerr << "couldn't read capability" << endl;
    ::close(fd);fd=-1;
    exit(1);
  }
  if(size.Width()>capability.maxwidth)
  {
    size.resize(capability.maxwidth,size.Height());
    cerr<< "width corrected to max. allowed " << size.Width()
         << endl;
  }
  if(size.Height()>capability.maxheight)
  {
    size.resize(size.Width(),capability.maxheight);
    cerr<< "height corrected to max. allowed " << size.Height()
        << endl;
  }
  // input sources
  channels=new struct video_channel[capability.channels];
  memset(channels,0,sizeof(struct video_channel)*capability.channels);
  inputs=new struct STRTAB[capability.channels+1];
  memset(inputs,0,sizeof(struct STRTAB)*(capability.channels+1));
  for(i=0;i<capability.channels; i++)
  {
    channels[i].channel = i;
    if(ioctl(fd,VIDIOCGCHAN,&channels[i]))
    		perror("ioctl VIDIOCGCHAN"),exit(0);
    inputs[i].nr  = i;
    inputs[i].str = channels[i].name;
  }
  inputs[i].nr  = -1;
  inputs[i].str = NULL;
  // switch to input 0
  if(ioctl(fd,VIDIOCSCHAN,&channels[0])==-1)
  		perror("ioctl VIDIOCSCHAN");
  if(ioctl(fd,VIDIOCGPICT,&pict)==-1)
  		perror("ioctl VIDIOCGPICT");
  //get buffer information
  if(ioctl(fd,VIDIOCGMBUF,&gb_buffers)==-1)
  		perror("ioctl gb_buffers"),exit(1);
  // mmap buffers into user space
  mm_buf[0]=(typename T::PIXELTYPE *)mmap((void *)0,gb_buffers.size,
  			PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  if(mm_buf[0]==(typename T::PIXELTYPE *)~0)
  {
    cerr << "couldn't mmap buffers" << endl;
    exit(1);
  }
  for(i=1;i<gb_buffers.frames;i++)
    mm_buf[i]=mm_buf[0]+gb_buffers.offsets[i]/sizeof(typename T::PIXELTYPE);
  set_params(parameter); // see Video.h for explanation
  if(brightness>-1) pict.brightness=brightness;
  if(contrast>-1) pict.contrast=contrast;
  if(hue>-1) pict.hue=hue;
  if(ioctl(fd,VIDIOCSPICT,&pict)==-1)
  	perror("ioctl VIDIOCSPICT");
  this->remap(mm_buf,n_buffers);
  return 1;
}

template <class T>
XVBt8x8<T>::XVBt8x8(const char *dev_name,const char *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=XVSize(640,480);
  frames=NULL;
  fd=-1;
  open(dev_name,parm_string);
}



template <class T>
XVBt8x8<T>::XVBt8x8(const XVSize & in_size, const char *dev_name,const char *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=in_size;
  frames=NULL;
  fd=-1;
  open(dev_name,parm_string);
}


template <class T>
XVBt8x8<T>::~XVBt8x8()
{
  cerr << "closing device" << endl;
  close();
  fd=-1;
}

template class XVBt8x8<XVImageRGB<XV_RGB15> >;
template class XVBt8x8<XVImageRGB<XV_RGB16> >;
template class XVBt8x8<XVImageRGB<XV_RGB24> >;
template class XVBt8x8<XVImageRGB<XV_RGBA32> >;
template class XVBt8x8<XVImageRGB<XV_GLRGBA32> >;
template class XVBt8x8<XVImageScalar<u_char> >;

