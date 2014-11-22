#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Meteor.h"
#include <XVMacros.h>

using namespace std ;

extern int debug;
template class Meteor<XVImageRGB<XV_RGB16> >;
template class Meteor<XVImageRGB<XV_RGBA32> >;


template <class T>
int   Meteor<T>::set_input(int norm,int channel)
{
   if(fd<0) return 0;
   if (ioctl(fd, METEORSINPUT, &channel) < 0) {
       perror("ioctl(METEORSINPUT)");
       return 0;
   }
   if (ioctl(fd, METEORSFMT, &norm) < 0) {
     perror("ioctl(METEORSFMT)");
     return 0;
   }
   return 1;
}

template <class T>
int Meteor<T>::initiate_acquire(int i_frame)
{
  int c=METEOR_CAP_CONT_ONCE;

  if(fd<0) return 0;
  if(i_frame<0 || i_frame>=geo.frames)
  {
     cerr << "only " << geo.frames << " allowed" <<endl;
     return 0;
  }
  assert(frame(i_frame).lock());
  if (ioctl(fd, METEORCAPTUR, &c)) {
    perror("ioctl(METEORCAPTUR)");
    exit(1);
  }
  frame(i_frame).unlock();
  return 1;
}

template <class T>
int Meteor<T>::wait_for_completion(int i_frame)
{
  return 1;
}

template <class T>
int Meteor<T>::set_params(char *paramstring)
{
   int 		num_frames=METEOR_DEF_NUMFRAMES;
   int 		input=METEOR_DEF_INPUT,
                norm=METEOR_DEF_NORM,i,format;
   XVParser	parse_result;
   static int norms[3]={METEOR_FMT_NTSC,METEOR_FMT_PAL,
                        METEOR_FMT_SECAM};
   static int inputs[6]={METEOR_INPUT_DEV0,
     			 METEOR_INPUT_DEV1,
     			 METEOR_INPUT_DEV2,
     			 METEOR_INPUT_DEV3,
     			 METEOR_INPUT_DEV_SVIDEO,
     			 METEOR_INPUT_DEV_RGB};
   while(parse_param(paramstring,parse_result)>0)
     switch(parse_result.c)
     {
       case 'N':  // norm selection
	 assert(norm>=0 && norm<3);
	 norm=norms[parse_result.val];
	 break;
       case 'I':  // input selection
         input=inputs[parse_result.val];
	 break;
       case 'B': // number of buffers
	 num_frames=parse_result.val;
	 break;
       default:
	 cerr << parse_result.c << "=" << parse_result.val
	      << " is not supported by Meteor (skipping)" << endl;
     }

   if(fd<0)
   {
     cerr << "Device is not open ...." << endl;
     exit(1);
   }
   if(num_frames>geo.frames)
   {
      cerr << "param error - only " << geo.frames
           << " frames compiled in driver" << endl;
      exit(1);
   }
   n_buffers=num_frames;
   geo.frames=num_frames;
   set_input(norm,input);
   return 1;
}

template <class T>
void Meteor<T>::close(void)
{
  if(fd>=0) ::close(fd);
  fd=-1;
}

template <class T>
int Meteor<T>::open(const char *dev_name,const char *parm_string)
{
  static char parameter[200];
  char   *mem_ptr;
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
  geo.oformat = (sizeof(T) == 2)? METEOR_GEO_RGB16:METEOR_GEO_RGB24;
  geo.columns=size.Width();
  geo.rows=size.Height();
  geo.frames=METEOR_DEF_NUMFRAMES;
  set_params(parameter); // see Video.h for explanation
  if (ioctl(fd, METEORSETGEO, &geo) < 0) {
    perror("ioctl(METEORSETGEO)");
    return 1;
  }
  if (ioctl(fd, METEORGFROFF, &off) < 0) {
          perror("ioctl FrameOffset failed");
          return 1;
  }
  // mmap buffers into user space
  mem_ptr=(char *)mmap((void *)0, off.fb_size, PROT_READ|PROT_WRITE,
      		      MAP_FILE|MAP_PRIVATE,fd,(off_t)0);
  if(mem_ptr==(char *)~0)
  {
    cerr << "couldn't mmap buffers" << endl;
    exit(1);
  }

  for(i=0;i<geo.frames;i++)
    mm_buf[i]=(typename T::PIXELTYPE*)(mem_ptr+off.frame_offset[i]);
  this->remap(mm_buf,geo.frames);
  return 1 ;
}

template <class T>
Meteor<T>::Meteor(const char *dev_name,const char *parm_string):
				XVVideo<T>(dev_name,parm_string)
{

  size=XVSize_NTSC_full;
  fd=-1;
  open(dev_name,parm_string);
}

template <class T>
Meteor<T>::~Meteor()
{
  if(fd>-1) ::close(fd);
}

