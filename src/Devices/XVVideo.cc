#include <stdio.h>
#include <XVVideo.h>
#include <XVMacros.h>

void perror (char *x) { cout << x << endl; exit(1);}

template <class IMTYPE>
XVVideo<IMTYPE>::XVVideo(const char * dev_name ,
			 const char * parmstring ) {

  name = dev_name;
  frame_count = 0;
  current_buffer=0;
  image_buffers=NULL;
  n_buffers = 0;
  own_buffers=0;
  capture_mode = MODE_CAP_SINGLE;
}


template <class IMTYPE>
IMTYPE * XVVideo<IMTYPE>::init_map(XVSize &size, int n_frames) {
  if(own_buffers && image_buffers) delete [] image_buffers;
  //image_buffers=new IMTYPE[n_frames](size.Width(),size.Height());
  image_buffers=new IMTYPE[n_frames];
  for( int i = 0 ; i < n_frames ; i ++ ) {
    image_buffers[i].resize( size.Width(), size.Height() );
  }
  n_buffers = n_frames;
  own_buffers=1;
  return image_buffers;
}


template <class IMTYPE>
IMTYPE * XVVideo<IMTYPE>::remap(typename IMTYPE::PIXELTYPE *mm_buf[], int n_frames) {

  int     i;
  if(own_buffers && image_buffers) delete [] image_buffers;
  //image_buffers = new IMTYPE[n_frames](size.Width(),size.Height());
  image_buffers=new IMTYPE[n_frames];
  for( i = 0 ; i < n_frames ; i ++ ) {
    image_buffers[i].resize( size.Width(), size.Height() );
  }
  for(i=0;i<n_frames;i++) image_buffers[i].remap(mm_buf[i],false);
  n_buffers = n_frames;
  own_buffers=0;
  return image_buffers;
}

/*
template <class T>
XVVideo<T>::XVVideo(const char *dev_name,
		XVSize size_in)

{
  int i;

  name = dev_name;
  frame_count = 0;
  current_buffer=0;
  image_buffers=NULL;
  n_buffers = 0;
  size = size_in;
}
*/
// Better make damn sure that this doesn't delete remapped memory
// before the actualy memory host deals with it ...

template <class IMTYPE>
XVVideo<IMTYPE>::~XVVideo() {
  if(own_buffers&&image_buffers) delete [] image_buffers;
}

template <class IMTYPE>
int XVVideo<IMTYPE>::parse_param(char * &paramstring, XVParser &result) {
   char *s_ptr;
   int j;

   switch(sscanf(paramstring,"%c%d%n",&result.c,&result.val,&j))
   {
     case 2:
       paramstring+=j; // set pointer to next parameter
       return 1;
     case 1:	            // no parameters to character
       cerr << "No value parsed for " << result.c << endl;
       return -1;
     default:               // end of string
       break;
   }
   return 0;  // signal end of string
}

#include <XVImageRGB.h>
#include <XVImageScalar.h>

template class XVVideo<XVImageRGB<XV_RGB15> >;
template class XVVideo<XVImageRGB<XV_RGB16> >;
template class XVVideo<XVImageRGB<XV_TRGB24> >;
template class XVVideo<XVImageRGB<XV_RGB24> >;
template class XVVideo<XVImageRGB<XV_RGBA32> >;
template class XVVideo<XVImageRGB<XV_GLRGBA32> >;

template class XVVideo<XVImageYUV<XV_YUV24> >;
template class XVVideo<XVImageYCbCr >;

template class XVVideo<XVImageScalar<u_char> >;
template class XVVideo<XVImageScalar<char> >;
template class XVVideo<XVImageScalar<u_short> >;
template class XVVideo<XVImageScalar<short> >;
template class XVVideo<XVImageScalar<u_int> >;
template class XVVideo<XVImageScalar<int> >;
template class XVVideo<XVImageScalar<float> >;
template class XVVideo<XVImageScalar<double> >;

//template class XVVideo<XV_YUV422>;
