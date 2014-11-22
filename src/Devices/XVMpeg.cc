#include "XVMpeg.h"

#include <mpeg.h>

template <class IMTYPE>
XVMpeg<IMTYPE>::XVMpeg(const char * fname,
		  const char * parm_string) 
  : XVVideo<IMTYPE>(fname) {

  open(fname);
}

template <class IMTYPE>
int XVMpeg<IMTYPE>::open(const char * fname) {

  ImageDesc img_info;

  // initializations

  if ( (fd = fopen(fname, "rb")) == NULL) { // get MPEG FILE for OpenMPEG
    perror("Can't open MPEG file!");
  }

  SetMPEGOption(MPEG_DITHER, FULL_COLOR_DITHER); // set dither mode
  OpenMPEG(fd, &img_info);  // open MPEG stream

#ifdef DEBUG0
  cout << "MPEG info:\n";
  cout << "Height = " << img_info.Height << endl;
  cout << "Width = " << img_info.Width << endl;
  cout << "Depth = " << img_info.Depth << endl;
  cout << "PixelSize = " << img_info.PixelSize << endl;
  cout << "Size = " << img_info.Size << endl;
  cout << "BitmapPad = " << img_info.BitmapPad << endl;
  cout << "PictureRate = " << img_info.PictureRate << endl;
  cout << "BitRate = " << img_info.BitRate << endl;
  cout << "ColormapSize = " << img_info.ColormapSize << endl;
#endif

  if (img_info.PixelSize != 32) {
    cout << "MPEG info:\n";
    cout << "Height = " << img_info.Height << endl;
    cout << "Width = " << img_info.Width << endl;
    cout << "Depth = " << img_info.Depth << endl;
    cout << "PixelSize = " << img_info.PixelSize << endl;
    cout << "Size = " << img_info.Size << endl;
    cout << "BitmapPad = " << img_info.BitmapPad << endl;
    cout << "PictureRate = " << img_info.PictureRate << endl;
    cout << "BitRate = " << img_info.BitRate << endl;
    cout << "ColormapSize = " << img_info.ColormapSize << endl;
    perror("Unexpected Pixel Size in MPEG");
    return 0;
  }

  // allocate the buffer space -- for some reason MPEG returns these
  // values backwards
  
  size.resize(img_info.Height,img_info.Width);

  size.resize(img_info.Width,img_info.Height);

  // Basic MPEG decode gets just one buffer

  init_map(size,DEF_MPEG_NUM);
  return 1 ;
}

template <class IMTYPE>
void XVMpeg<IMTYPE>::close() 
{
  CloseMPEG();  // close MPEG stream
  ::fclose(fd);
}

//-----------------------------------------------------------------------------
//  Grab functions
//-----------------------------------------------------------------------------

template <class IMTYPE>
int XVMpeg<IMTYPE>::initiate_acquire(int framenum) {

  // get the next MPEG frame
  char *ptr = (char *)const_cast<typename IMTYPE::PIXELTYPE *>(frame(framenum).pixData());
  if (!GetMPEGFrame((char *) ptr)) {
    frame(framenum).unlock();
    return -1;
  }

  frame(framenum).unlock();
  return framenum;
}

template <class IMTYPE>
int XVMpeg<IMTYPE>::wait_for_completion(int framenum) { return 1; }

/* libmpeg only supports 24 bit MPEG decoding.
 * Thus, in order to use an mpeg on a display of <24 bits, you must
 * explicitly create the XVImageRGB<XV_RGB15|16> and perform the 
 * conversion this way on your own. jcorso 26August2001
 *
template class XVMpeg<XVImageRGB<XV_RGB15> >;
template class XVMpeg<XVImageRGB<XV_RGB16> >;
*/
template class XVMpeg<XVImageRGB<XV_RGB24> >;
template class XVMpeg<XVImageRGB<XV_RGBA32> >;
