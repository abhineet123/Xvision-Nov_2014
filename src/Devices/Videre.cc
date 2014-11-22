#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "XVVideo.h"
#include "Videre.h"
#include "XVMacros.h"
#include <iostream>

extern int debug;
static bool complain_flag=true;

template <class T>
int XV_Videre<T>::initiate_acquire(int frame)
{
   return 1;
}

template <class T>
int XV_Videre<T>::wait_for_completion(int frame)
{
   if(!(s_image = sourceObject->GetImage(500))) {cerr <<"t";return 0;} // timeout?
   if(!s_image->haveColor || !s_image->haveColorRight)
   {
	   if(complain_flag){
		   complain_flag=false;
		   cerr << "Color is not enabled in config file" << endl;
	   }
	   return 0;
   }
   scalar_image[XVVID_LEFT]->remap((u_char*)s_image->left,false);
   scalar_image[XVVID_RIGHT]->remap((u_char*)s_image->right,false);

   if(s_image->haveColor)vid_correct_color(image_buffers[frame],s_image->color);
   if(s_image->haveColorRight)vid_correct_color(image_buffers_right,s_image->color_right);
   return 1;
}

template <class T>
XVImageScalar<u_char>       **XV_Videre<T>::get_stereo(void)
{
   scalar_image[XVVID_LEFT]->remap((u_char*)s_image->left,false);
   scalar_image[XVVID_RIGHT]->remap((u_char*)s_image->right,false);
   return scalar_image;
}



template <class T>
void XV_Videre<T>::close(void)
{
  if(sourceObject) sourceObject->Stop();
  if(sourceObject) sourceObject->Close();
  sourceObject=NULL;
}

template <class T>
int XV_Videre<T>::open(const char *dev_name)
{
  sourceObject->Open(0);
  return sourceObject->ReadParams((char *)dev_name);
  //sourceObject->SetRect(true);
}

template <class T>
XV_Videre<T>::XV_Videre(const char *dev_name,const char *parm_string):
             XVVideo<T>(dev_name,parm_string)
{
  s_image=NULL;
  sourceObject = getVideoObject();
  if(!open(dev_name))
  {
    cerr << "Couldn't parse the init file " << dev_name << endl;
    cerr << "... defaulting to standard values"<<endl;
  }
  //cerr << "color from param " << sourceObject->haveColor << " "<< sourceObject->haveColorRight<<endl;
  //if(!sourceObject->CheckParams())
         sourceObject->ReadParams((char*)parm_string);
  params=sourceObject->GetIP();
  sourceObject->SetSize(640,480); // 320x240 image
  cerr << "Videre: using " << svsVideoIdent << endl;
  scalar_image[XVVID_LEFT]=
  	new XVImageScalar<u_char>(params->width,params->height);
  scalar_image[XVVID_RIGHT]=
  	new XVImageScalar<u_char>(params->width,params->height);
  XVSize size(params->width,params->height);
  //sourceObject->SetColor(false, false); // both left and right
  //sourceObject->SetColor(true, true); // both left and right

  //std::cerr << "params: " << params->color << " " << params->color_right << std::endl;
  //cin.get();

  sourceObject->SetCapture(CAP_DUAL);
  //sourceObject->SetNDisp(64);    // 32 disparities
  sourceObject->SetCorrsize(7); // correlation window size
  //sourceObject->SetLR(false);    // no left-right check, not available
  sourceObject->SetThresh(8);   // texture filter
  sourceObject->SetUnique(2);   // uniqueness filter
  sourceObject->SetHoropter(62);  // horopter offset
  sourceObject->SetRect(false);  //
  //sourceObject->SetProcMode(parm_string? PROC_MODE_DISPARITY:PROC_MODE_RECTIFIED);
  //sourceObject->SetProcMode(PROC_MODE_RECTIFIED);
  sourceObject->SetRate(30);
  sourceObject->SetColor(true, true); // both left and right
  sourceObject->Start();

  init_map(size,1);

  image_buffers_right.resize( size.Width(), size.Height() );
}

template <class T>
XV_Videre<T>::~XV_Videre()
{
  close();
}

template class XV_Videre<XVImageRGB<XV_RGB15> >;
template class XV_Videre<XVImageRGB<XV_RGB16> >;
template class XV_Videre<XVImageRGB<XV_RGB24> >;
template class XV_Videre<XVImageRGB<XV_TRGB24> >;
template class XV_Videre<XVImageRGB<XV_RGBA32> >;
template class XV_Videre<XVImageRGB<XV_GLRGBA32> >;
