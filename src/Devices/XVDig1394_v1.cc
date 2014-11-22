#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include <config.h>
#ifdef HAVE_DV

#include "libdc1394/dc1394_control.h"
#include <XVDig1394.h>
#include <XVMacros.h>

using namespace std ;

extern int debug;

namespace {

  // manually initialize the YUB<->RGB conversion tables since we are going
  // to use these tables directly.
  void init_tab(void) { 
    if( ! bRGB2YUVTableBuilt ) {
      buildRGB2YUVTable();
    }
    if( ! bYUV2RGBTableBuilt ) {
      buildYUV2RGBTable();
    }
  }; // init_tab

  // helper class to help conversion functions
  template<class IMTYPE>
  struct Convert {
    typedef typename IMTYPE::PIXELTYPE PIXELTYPE ;

  // convert a buffer of YUV444 image into an IMTYPE (assume XVImageRGB)
  // YUV8 4:4:4 format:  U Y V, so buffer size = # pixels * 3
    static
  void yuv444_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3 ;
    unsigned char pg ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 3, ++ iTarg ) {
      pg =  TABLE_YUV_TO_PG[ ptr[0] ][ ptr[2] ] ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[1] ][ ptr[2] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[1] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[1] ][ pg ] );
    }
  } // yuv444_f

  // convert a buffer of YUV422 image into an IMTYPE (assume XVImageRGB)
  // YUV8 4:2:2 format:  U Y1 V Y2, so buffer size = # pixels * / 2 * 4
    static
  void yuv422_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*2 ;
    unsigned char pg ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 4 ) {
      pg =  TABLE_YUV_TO_PG[ ptr[0] ][ ptr[2] ] ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[1] ][ ptr[2] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[1] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[1] ][ pg ] );
      ++ iTarg ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[3] ][ ptr[2] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[3] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[3] ][ pg ] );
      ++ iTarg ;
    }
  } // yuv422_f

  // convert a buffer of YUV411 image into an IMTYPE (assume XVImageRGB)
  // YUV8 4:1:1 format:  U Y1 Y2 V Y3 Y4, so buffer size = # pixels / 4 * 6
    static 
  void yuv411_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3/2 ;
    unsigned char pg ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 6 ) {
      pg =  TABLE_YUV_TO_PG[ ptr[0] ][ ptr[3] ] ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[1] ][ ptr[3] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[1] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[1] ][ pg ] );
      ++ iTarg ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[2] ][ ptr[3] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[2] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[2] ][ pg ] );
      ++ iTarg ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[4] ][ ptr[3] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[4] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[4] ][ pg ] );
      ++ iTarg ;
      iTarg -> setR( TABLE_YUV_TO_R[ ptr[5] ][ ptr[3] ] );
      iTarg -> setB( TABLE_YUV_TO_B[ ptr[5] ][ ptr[0] ] );
      iTarg -> setG( TABLE_YUV_TO_G[ ptr[5] ][ pg ] );
      ++ iTarg ;
    }
  } // yuv411_f

  // convert a buffer of RGB image into an IMTYPE (assume XVImageRGB)
  // RGB8 format:  R G B, so buffer size = # pixels * 3
    static
  void rgb8_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ++ iTarg ) {
      iTarg -> setR( *ptr++ );
      iTarg -> setG( *ptr++ );
      iTarg -> setB( *ptr++ );
    }
  } // rgb8_f

  // convert a buffer of RGB image into an IMTYPE (assume XVImageRGB)
  // RGB16 format:  Rh Rl Gh Gl Bh Bl, so buffer size = # pixels * 6
    static
  void rgb16_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*6 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ++ iTarg ) {
      iTarg -> setR( *ptr );
      ptr += 2 ;
      iTarg -> setG( *ptr );
      ptr += 2 ;
      iTarg -> setB( *ptr );
      ptr += 2 ;
    }
  } // rgb16_f

  // convert a buffer of 8 bit mono image into an IMTYPE (assume XVImageRGB)
    static
  void mono8_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr ++, ++ iTarg ) {
      iTarg -> setR( *ptr );
      iTarg -> setG( *ptr );
      iTarg -> setB( *ptr );
    }
  } // mono8_f

  // convert a buffer of 16 bit mono image into an IMTYPE (assume XVImageRGB)
    static
  void mono16_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels * 2 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 2, ++ iTarg ) {
      iTarg -> setR( *ptr );
      iTarg -> setG( *ptr );
      iTarg -> setB( *ptr );
    }
  } // mono16_f

  }; // struct Convert

  // partial specialization of Convert to deal with XVImageYUV
  template<class PIXELTYPE>
  struct Convert<XVImageYUV<PIXELTYPE> > {
    typedef XVImageYUV<PIXELTYPE> IMTYPE ;

  // convert a buffer of YUV444 image into a XVImageYUV (independent Y,U and V)
  // YUV8 4:4:4 format:  U Y V, so buffer size = # pixels * 3
    static
  void yuv444_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ++ iTarg ) {
      iTarg -> setU( *ptr++ );
      iTarg -> setY( *ptr++ );
      iTarg -> setV( *ptr++ );
    }
  } // yuv444_f<XVImageYUV>

  // convert a buffer of YUV422 image into a XVImageYUV (independent Y,U and V)
  // YUV8 4:2:2 format:  U Y1 V Y2, so buffer size = # pixels * / 2 * 4
    static
  void yuv422_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*2 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 4 ) {
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[1] );
      iTarg -> setV( ptr[2] );
      ++ iTarg ;
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[3] );
      iTarg -> setV( ptr[2] );
      ++ iTarg ;
    }
  } // yuv422_f<XVImageYUV>

  // convert a buffer of YUV411 image into a XVImageYUV (independent Y,U and V)
  // YUV8 4:1:1 format:  U Y1 Y2 V Y3 Y4, so buffer size = # pixels / 4 * 6
    static
  void yuv411_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3/2 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 6 ) {
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[1] );
      iTarg -> setV( ptr[3] );
      ++ iTarg ;
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[2] );
      iTarg -> setV( ptr[3] );
      ++ iTarg ;
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[4] );
      iTarg -> setV( ptr[3] );
      ++ iTarg ;
      iTarg -> setU( ptr[0] );
      iTarg -> setY( ptr[5] );
      iTarg -> setV( ptr[3] );
      ++ iTarg ;
    }
  } // yuv411_f<XVImageYUV>

  // convert a buffer of RGB image into a XVImageYUV (independent Y,U and V)
  // RGB8 format:  R G B, so buffer size = # pixels * 3
    static
  void rgb8_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*3 ;
    unsigned char py, y ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for( ; ptr < ptrEnd ; ptr += 3, ++ iTarg ) {
      py = TABLE_RGB_TO_PY[ ptr[0] ][ ptr[1] ];
      y = TABLE_RGB_TO_Y[ py ][ ptr[2] ];
      iTarg -> setU( TABLE_RGB_TO_U[ ptr[2] ][ y ] );
      iTarg -> setY( y );
      iTarg -> setV( TABLE_RGB_TO_U[ ptr[0] ][ y ] );
    }
  } // rgb8_f<XVImageYUV>

  // convert a buffer of RGB image into a XVImageYUV (independent Y,U and V)
  // RGB16 format:  Rh Rl Gh Gl Bh Bl, so buffer size = # pixels * 6
    static
  void rgb16_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels*6 ;
    unsigned char py, y ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for( ; ptr < ptrEnd ; ptr += 6, ++ iTarg ) {
      py = TABLE_RGB_TO_PY[ ptr[0] ][ ptr[2] ];
      y = TABLE_RGB_TO_Y[ py ][ ptr[4] ];
      iTarg -> setU( TABLE_RGB_TO_U[ ptr[4] ][ y ] );
      iTarg -> setY( y );
      iTarg -> setV( TABLE_RGB_TO_U[ ptr[0] ][ y ] );
    }
  } // rgb16_f<XVImageYUV>

  // convert a buffer of 8 bit mono image into a XVImageYUV (i11t Y,U and V)
    static
  void mono8_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr ++, ++ iTarg ) {
      iTarg -> setY( *ptr );
      iTarg -> setU( 0 );
      iTarg -> setV( 0 );
    }
  } // mono8_f<XVImageYUV>

  // convert a buffer of 16 bit mono image into a XVImageYUV (i11t Y,U and V)
    static
  void mono16_f( const unsigned char * buf, IMTYPE& targ, int numPixels ) {
    const unsigned char * ptr = buf, * ptrEnd = buf + numPixels * 2 ;
    XVImageWIterator<PIXELTYPE>  iTarg(targ);
    for ( ; ptr < ptrEnd ; ptr += 2, ++ iTarg ) {
      iTarg -> setY( *ptr );
      iTarg -> setU( 0 );
      iTarg -> setV( 0 );
    }
  } // mono16_f<XVImageYUV>

  }; // struct Convert<XVImageYUV>

  enum Color { mono8, yuv411, yuv422, yuv444, rgb8, mono16, rgb16 } ;
  const char * strColor[] = 
    { "Mono8", "YUV8 4:1:1", "YUV8 4:2:2", "YUV8 4:4:4", 
      "RGB8", "Mono16", "RGB16" } ;

  // returns the correpsonding color-space conversion function
  template<class T>
  void (*color_f( Color c ))( const unsigned char *, T&, int ) {
    switch( c ) {
    case mono8 :
      return &Convert<T>::mono8_f ;
    case mono16:
      return &Convert<T>::mono16_f ;
    case rgb8 :
      return &Convert<T>::rgb8_f ;
    case rgb16 :
      return &Convert<T>::rgb16_f ;
    case yuv411:
      return &Convert<T>::yuv411_f ;
    case yuv422:
      return &Convert<T>::yuv422_f ;
    case yuv444:
      return &Convert<T>::yuv444_f ;
    default:
      return 0 ; // invalid or unimplemented
    }
  }

  // returns the size of a line of pixels
  int getLineSize( int width,  Color c ) {
    switch( c ) {
    case mono8 :
      return width ;
    case mono16 :
      return width * 2 ;
    case rgb8 :
      return width * 3 ;
    case rgb16 :
      return width * 6 ;
    case yuv411:
      return width * 3 / 2 ;
    case yuv422:
      return width * 2 ;
    case yuv444:
      return width * 3 ;
    default:
      return 0 ; // invalid or unimplemented
    }
  }

  // returns the size of required buffer
  inline int getBufSize( int width, int height, Color c ) {
    return getLineSize( width, c ) * height ;
  }

  // look-up tables
  const int nFormats = 3 ;
  const int nModes = 8 ;
  struct ModeInfo {  // IEEE 1394 camera formats and modes
    Color color ;
    int width ;
    int height ;
    short lines ;       // supposed at framerate 1.875, numerator
    short per_packet ;  // denominator
  } modeinfo[nFormats][nModes] = {
    {  // format 0
      { yuv444,  160,  120,  1, 32 },  // mode 0 
      { yuv422,  320,  240,  1, 16 },  // mode 1 
      { yuv411,  640,  480,  1,  8 },  // mode 2
      { yuv422,  640,  480,  1,  8 },  // mode 3
      { rgb8,    640,  480,  1,  8 },  // mode 4
      { mono8,   640,  480,  1,  8 },  // mode 5 
      { mono16,  640,  480,  1,  8 },  // mode 6 
    },
    {  // format 1
      { yuv422,  800,  600,  5, 32 },  // mode 0
      { rgb8,    800,  600,  5, 32 },  // mode 1
      { mono8,   800,  600,  5, 32 },  // mode 2
      { yuv422, 1024,  768,  3, 16 },  // mode 3
      { rgb8,   1024,  768,  3, 16 },  // mode 4
      { mono8,  1024,  768,  3, 16 },  // mode 5
      { mono16,  800,  600,  5, 32 },  // mode 6
      { mono16, 1024,  768,  3, 16 },  // mode 7
    },
    {  // format 2
      { yuv422, 1280,  960,  1,  4 },  // mode 0
      { rgb8,   1280,  960,  1,  4 },  // mode 1
      { mono8,  1280,  960,  1,  4 },  // mode 2
      { yuv422, 1600, 1200,  5, 16 },  // mode 3
      { rgb8,   1600, 1200,  5, 16 },  // mode 4
      { mono8,  1600, 1200,  5, 16 },  // mode 5
      { mono16, 1280,  960,  1,  4 },  // mode 6
      { mono16, 1600, 1200,  5, 16 },  // mode 7
    },
  };
  const char * strFrameRates[] = 
    { "1.875", "3.75", "7.5", "15", "30", "60" };

  // helper functions to look-up the table
  inline XVSize getSize( int format, int mode ) {
    return XVSize( modeinfo[format][mode].width,
		   modeinfo[format][mode].height );
  }
  inline int getBufSize( int format, int mode ) {
    return getBufSize( modeinfo[format][mode].width,
		       modeinfo[format][mode].height,
		       modeinfo[format][mode].color );
  }
  inline int getPacketSize( int format, int mode, int xFrameRate ) {
    return getLineSize( modeinfo[format][mode].width, 
			modeinfo[format][mode].color ) * 
      modeinfo[format][mode].lines * (1<<(xFrameRate-FRAMERATE_MIN)) /
      modeinfo[format][mode].per_packet ;
  }
  template<class T>
  void (*getConvert( int format, int mode ))( const unsigned char *, T&, int ){
    return color_f<T>( modeinfo[format][mode].color );
  }
  inline const char* strConvert( int format, int mode ) {
    return strColor[modeinfo[format][mode].color] ;
  }

  // for work with libdc1394_control
  inline int getFormat( int format ) {
    return FORMAT_MIN + format ;
  }
  inline int getMode( int format, int mode ) {
    static int MODE_FORMATx_MIN[nFormats] = { 
      MODE_FORMAT0_MIN, MODE_FORMAT1_MIN, MODE_FORMAT2_MIN 
    };
    return (format==7 ? MODE_FORMAT7_MIN : MODE_FORMATx_MIN[format]) + mode ;
  }
  inline int getFrameRate( int framerate ) {
    return FRAMERATE_MIN + framerate ;
  }
  inline int getSpeed() {
    return SPEED_400 ;
  }
  inline int getColor( Color c ) {
    return COLOR_FORMAT7_MIN + c ;
  }
  inline bool isAvailable( int n, quadlet_t masks ) { // n is format or mode
    return masks & (quadlet_t)1 << (8*sizeof(quadlet_t)-n-1) ;
  }
  int minMask( quadlet_t masks ) { // return -1 for nothing
    int n = 0 ;
    quadlet_t test = (quadlet_t)1 << (8*sizeof(quadlet_t)-n-1) ;
    for( ; test && !( masks & test ) ; test >>= 1, n ++ );
    return test ? n : -1 ;
  }
  int maxMask( quadlet_t masks, int n ) { // return -1 for nothing
    quadlet_t test = (quadlet_t)1 << (8*sizeof(quadlet_t)-n-1) ;
    for( ; test && !( masks & test ) ; test <<= 1, n -- );
    return n ;
  }
  inline int maxFrameRate( quadlet_t masks ) { // return -1 for error
    return maxMask( masks, NUM_FRAMERATES-1 );
  }
  void printFM( raw1394handle_t handle, nodeid_t node ) {
    quadlet_t fmasks = 0, mmasks = 0 ;
    cout << "Supported formats and modes: " ;
    dc1394_query_supported_formats( handle, node, &fmasks );
    for( int f = 0 ; f <= 7 ; f ++, mmasks = 0 ) {
      if( isAvailable( f, fmasks ) ) {
	dc1394_query_supported_modes( handle, node, getFormat(f), &mmasks );
	for( int m = 0 ; m < nModes ; m ++ ) {
	  if( isAvailable( m, mmasks ) ) {
	    cout << f << m << ' ' ;
	  }
	}
      }
    }
    cout << endl;
  }

  // for work with XVDig1394::set_param interface
  const int nRGBParam = 6 ;     // YUV422, RGB, MONO8, MONO16, YUV411, YUV444
  const int nScaleParam = 4 ;   // default, 640x480, 320x240, 160x120
  const int nHighResParam = 4 ; // 800x600, 1027x768, 1280x960, 1600x1200
  struct ParamTrans {
    int format ;
    int mode ;
  } paramtrans [nRGBParam][nScaleParam+nHighResParam] = {
    { {  0,  3 }, {  0,  3 }, {  0,  1 }, { -1, -1 },
      {  1,  0 }, {  1,  3 }, {  2,  0 }, {  2,  3 } },
    { {  0,  4 }, { -1, -1 }, { -1, -1 }, { -1, -1 },
      {  1,  1 }, {  1,  4 }, {  2,  1 }, {  2,  4 } },
    { {  0,  5 }, {  0,  5 }, { -1, -1 }, { -1, -1 },
      {  1,  2 }, {  1,  5 }, {  2,  2 }, {  2,  5 } },
    { {  0,  6 }, {  0,  6 }, { -1, -1 }, { -1, -1 },
      {  1,  6 }, {  1,  7 }, {  2,  6 }, {  2,  7 } },
    { {  0,  2 }, {  0,  2 }, { -1, -1 }, { -1, -1 },
      { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } },
    { {  0,  0 }, { -1, -1 }, { -1, -1 }, {  0,  0 },
      { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } },
  };
  /* note: the logic of the code artificially limits the selection of 
     pixel color encoding and image size (scale) in format 7 into 
     those avaible in format 0-2 because these information
     is still carried in format and mode data members from
     set_params() to the class constructor.
   */

  /* The following code is to deal with the the change of prototype of 
     dc1394_query_format7_total_bytes between different versions of 
     dc1394_control.h . The fourth parameter has been changed from 
     unsigned int * to unsigned long long int * . 
     Ideally this should be resolved in autoconf level instead of .cc level
     however I (Donald) don't have time to investigate which version is which
     so here is just a quick dirty fix using template partial specialization.
   */
  template<class F>
  struct CorrectCall {
  };
  template<>
  struct CorrectCall< int (*)( raw1394handle_t, nodeid_t,
			       unsigned int, unsigned long long int * ) > {
    typedef int (*fun_t)( raw1394handle_t, nodeid_t,
			  unsigned int, unsigned long long int * );
    inline int operator () 
      ( fun_t f, raw1394handle_t handle, nodeid_t node,
	unsigned int mode, unsigned long long int * total_bytes ) {
      return f( handle, node, mode, total_bytes );
    }
  };
  template<>
  struct CorrectCall< int (*)( raw1394handle_t, nodeid_t,
			       unsigned int, unsigned int *) > {
    typedef int (*fun_t)( raw1394handle_t, nodeid_t,
			  unsigned int, unsigned int * );
    inline int operator () 
      ( fun_t f, raw1394handle_t handle, nodeid_t node,
	unsigned int mode, unsigned long long int * total_bytes ) {
      if( total_bytes ) *total_bytes = 0 ;
      return f( handle, node, mode, (unsigned int *)total_bytes );
    }
  };

  template<class F>
  inline int correctCall( F f, raw1394handle_t handle, nodeid_t node,
		    unsigned int mode, unsigned long long int * total_bytes ) {
    return CorrectCall<F>()( f, handle, node, mode, total_bytes );
  }
  
} // unnamed namespace 

template <class IMTYPE>
int XVDig1394<IMTYPE>::initiate_acquire(int i_frame)
{
   struct video1394_wait w_descr;
   w_descr.channel=v_mmap.channel;
   w_descr.buffer=i_frame;
   if(ioctl(fd,VIDEO1394_LISTEN_QUEUE_BUFFER,&w_descr))
   {
     perror("XVision:");
     exit(1);
   } 
   return 1;
}


template <class PIXELTYPE> 
void    BayerNearestNeighbor(unsigned char *src, 
                 XVImageRGB<PIXELTYPE> & targ, int sx, int sy,
		 bayer_pattern_type optical_filter)
{
  PIXELTYPE  *data=targ.lock();
  register int i,j;

  switch (optical_filter) {
  case BAYER_PATTERN_GRBG: //-------------------------------------------
    // copy original RGB data to output images
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	data[(i*sx+j)].setG(src[i*sx+j]);
	data[((i+1)*sx+(j+1))].setG(src[(i+1)*sx+(j+1)]);
	data[(i*sx+j+1)].setB(src[i*sx+j+1]);
	data[((i+1)*sx+j)].setR(src[(i+1)*sx+j]);
      }
    }
    // B channel
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setB(data[(i*sx+j+1)].B());
	data[((i+1)*sx+j+1)].setB(data[(i*sx+j+1)].B());
	data[((i+1)*sx+j)].setB(data[(i*sx+j+1)].B());
      }
    }
      // R channel
    for (i=0;i<sy-1;i+=2)  { //every two lines
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setR(data[((i+1)*sx+j)].R());
	data[(i*sx+j+1)].setR(data[((i+1)*sx+j)].R());
	data[((i+1)*sx+j+1)].setR(data[((i+1)*sx+j)].R());
      }
    }
    // using lower direction for G channel
      
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=1;j<sx;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
      
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      data[((sy-1)*sx+j)].setG(data[((sy-2)*sx+j)].G());
    
    break;

  case BAYER_PATTERN_GBRG:
    // copy original RGB data to output images
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	data[(i*sx+j)].setG(src[i*sx+j]);
	data[((i+1)*sx+(j+1))].setG(src[(i+1)*sx+(j+1)]);
	data[(i*sx+j+1)].setR(src[i*sx+j+1]);
	data[((i+1)*sx+j)].setB(src[(i+1)*sx+j]);
      }
    }
    // R channel
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setR(data[(i*sx+j+1)].R());
	data[((i+1)*sx+j+1)].setR(data[(i*sx+j+1)].R());
	data[((i+1)*sx+j)].setR(data[(i*sx+j+1)].R());
      }
    }
      // B channel
    for (i=0;i<sy-1;i+=2)  { //every two lines
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setB(data[((i+1)*sx+j)].B());
	data[(i*sx+j+1)].setB(data[((i+1)*sx+j)].B());
	data[((i+1)*sx+j+1)].setB(data[((i+1)*sx+j)].B());
      }
    }
    // using lower direction for G channel
      
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=1;j<sx;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
      
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      data[((sy-1)*sx+j)].setG(data[((sy-2)*sx+j)].G());
    
    break;
  case BAYER_PATTERN_RGGB:
    // copy original data
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	data[(i*sx+j)].setR(src[i*sx+j]);
	data[((i+1)*sx+(j+1))].setB(src[(i+1)*sx+(j+1)]);
	data[(i*sx+j+1)].setG(src[i*sx+j+1]);
	data[((i+1)*sx+j)].setG(src[(i+1)*sx+j]);
      }
    }
    // B channel
    for (i=0;i<sy;i+=2){
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setB(data[((i+1)*sx+j+1)].B());
	data[(i*sx+j+1)].setB(data[((i+1)*sx+j+1)].B());
	data[((i+1)*sx+j)].setB(data[((i+1)*sx+j+1)].B());
      }
    }
    // R channel
    for (i=0;i<sy-1;i+=2) { //every two lines
      for (j=0;j<sx-1;j+=2) {
	data[((i+1)*sx+j)].setR(data[(i*sx+j)].R());
	data[(i*sx+j+1)].setR(data[(i*sx+j)].R());
	data[((i+1)*sx+j+1)].setR(data[(i*sx+j)].R());
      }
    }
    // using lower direction for G channel
    
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
    
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j+1)].setG(data[((i+1)*sx+j+1)].G());
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      data[((sy-1)*sx+j+1)].setG(data[((sy-2)*sx+j+1)].G());
    
    break;
    
  case BAYER_PATTERN_BGGR: //-------------------------------------------
    // copy original data
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	data[(i*sx+j)].setB(src[i*sx+j]);
	data[((i+1)*sx+(j+1))].setR(src[(i+1)*sx+(j+1)]);
	data[(i*sx+j+1)].setG(src[i*sx+j+1]);
	data[((i+1)*sx+j)].setG(src[(i+1)*sx+j]);
      }
    }
    // R channel
    for (i=0;i<sy;i+=2){
      for (j=0;j<sx-1;j+=2) {
	data[(i*sx+j)].setR(data[((i+1)*sx+j+1)].R());
	data[(i*sx+j+1)].setR(data[((i+1)*sx+j+1)].R());
	data[((i+1)*sx+j)].setR(data[((i+1)*sx+j+1)].R());
      }
    }
    // B channel
    for (i=0;i<sy-1;i+=2) { //every two lines
      for (j=0;j<sx-1;j+=2) {
	data[((i+1)*sx+j)].setB(data[(i*sx+j)].B());
	data[(i*sx+j+1)].setB(data[(i*sx+j)].B());
	data[((i+1)*sx+j+1)].setB(data[(i*sx+j)].B());
      }
    }
    // using lower direction for G channel
    
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j)].setG(data[((i+1)*sx+j)].G());
    
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	data[(i*sx+j+1)].setG(data[((i+1)*sx+j+1)].G());
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      data[((sy-1)*sx+j+1)].setG(data[((sy-2)*sx+j+1)].G());
    
    break;
    
  default:  //-------------------------------------------
    break;
  }
  targ.unlock();
}

template <class PIXELTYPE> 
void BayerNearestNeighbor(unsigned char *src, 
        XVImageYUV<PIXELTYPE>& targ, int sx, int sy,bayer_pattern_type
	optical_filter){cerr << "Bayer decoder does not support YUV"<<
	endl;};

template <class IMTYPE>
int XVDig1394<IMTYPE>::wait_for_completion(int i_frame)
{
   struct video1394_wait w_descr;

   v_mmap.flags=VIDEO1394_SYNC_FRAMES;
   w_descr.channel=v_mmap.channel;
   w_descr.buffer=i_frame;
   if(ioctl(fd,VIDEO1394_LISTEN_WAIT_BUFFER,&w_descr))
   {
     perror("XVision:");
     exit(1);
   }
   if(optical_filter!=BAYER_Not_Valid)
     BayerNearestNeighbor(mm_buf[i_frame],image_buffers[i_frame],
       size.Width(),size.Height(),optical_filter);
   else
     convert(mm_buf[i_frame],image_buffers[i_frame],
       size.Width()*size.Height());
   return 1;
}

template <class IMTYPE>
int XVDig1394<IMTYPE>::set_params(char *paramstring) {

   XVParser	parse_result;  

   int scale = 0, pixel = 0 ;
   bool direct = false ;
   n_buffers = DIG_DEF_NUMFRAMES;
   while( parse_param(paramstring,parse_result) > 0 ) {
     switch(parse_result.c) {
     case 'B':
       n_buffers = parse_result.val;
       break;
     case 'R':
       pixel = parse_result.val;
       direct = false ;
       break;
     case 'S':
       scale = parse_result.val;
       direct = false ;
       break;
     case 'M':
       scale = parse_result.val + nScaleParam ;
       direct = false ;
       break;
     case 'C':
       if( ( grab_center = parse_result.val ) ) {
	 format7 = true ;
       }
       break;
     case 'T':
       if( parse_result.val > 0 && parse_result.val <= NUM_TRIGGER_MODE ) {
	 ext_trigger = parse_result.val + TRIGGER_MODE_0 - 1 ;
       }else {
	 ext_trigger = parse_result.val ;
       }
       break;
     case 'f':
       if( parse_result.val == 7 ) {
	 format7 = true ;
       }else {
	 format = parse_result.val;
	 direct = true ;
       }
       break ;
     case 'm' :
       mode = parse_result.val;
       direct = true ;
       break ;
     case 'r' :
       framerate = parse_result.val;
       break ;
     case 'g':			// gain of the camera
       gain = parse_result.val;
       break;
     case 'u':			// u component for the white balance
       uv[0] = parse_result.val;
       break;
     case 'v':			// v component for the white balance
       uv[1] = parse_result.val;
       break;
     case 's':			// saturation
       saturation = parse_result.val;
       break;
     case 'A':                        // sharpness
       sharpness = parse_result.val ;
       break;
     case 'h' :         //shutter
       shutter = parse_result.val;
       break;
     case 'a' :
       gamma = parse_result.val;
       break;
     case 'x' :
       exposure = parse_result.val;
       break;
     case 'o':   // optical filter selection
       optical_filter = (bayer_pattern_type)parse_result.val;
       break;
     default:
       cerr << parse_result.c << "=" << parse_result.val 
	    << " is not supported by XVDig1394 (skipping)" << endl;
     }
   }
   if( ! direct ) {
     format = paramtrans[pixel][scale].format ;
     mode = paramtrans[pixel][scale].mode ;
     if( format == -1 || mode == -1 ) {
       cerr << "Unsupported combination of pixel format and scale" << endl;
       exit(1);
     }
   }
   return 1;
}

template <class IMTYPE>
void XVDig1394<IMTYPE>::close(void)
{
  ioctl(fd,VIDEO1394_UNLISTEN_CHANNEL,&v_mmap);
  if(fd>=0) ::close(fd);
  fd=-1;
}

template <class IMTYPE> int XVDig1394<IMTYPE>::open(const char *dev_name) {

  device_name=dev_name;
  if(fd>=0) close(); // close old stuff first ???
  if((fd=::open(dev_name,O_RDONLY))<0)
  {
    perror("XVision:");
    exit(1);
  }
  ioctl(fd,VIDEO1394_LISTEN_CHANNEL,&v_mmap);
  mm_buf[0]=(unsigned char *)mmap((void *)0,v_mmap.buf_size*v_mmap.nb_buffers,
                        PROT_READ,MAP_SHARED,fd,0);
  if(mm_buf[0]==(unsigned char *)~0)
  {
        cerr << "couldn't mmap buffers" << endl;
	perror("XVision:");
        exit(1);
  }
  //mm_buf[0]+=12;
  for(int i=1;i<v_mmap.nb_buffers;i++)
      mm_buf[i]=mm_buf[0]+i*v_mmap.buf_size;
  return 1;
}

template <class IMTYPE>
XVDig1394<IMTYPE>::XVDig1394( const char *dev_name, const char *parm_string,
			      unsigned long long camera_id)
  : XVVideo<IMTYPE>(dev_name,parm_string) {

  int numCameras;
  int channel,camera_handle;
  dc1394_camerainfo camera_info;
  init_tab();

  // parsing parameters
  format = 0 ; mode = 3 ; framerate = -1 ; 
  format7 = false ; grab_center = false ; ext_trigger = 0 ;
  gain = -1 ; saturation = -1 ; uv[0] = uv[1] = -1 ; 
  sharpness = -1 ; shutter = -1 ; gamma = -1 ; exposure = -1 ;
  optical_filter=BAYER_Not_Valid;
  if(parm_string) set_params((char *)parm_string);

  // connecting to the camera
  switch(dev_name[strlen(dev_name)-1])
  {
     case '1':
        camera_handle=1;
        break;
     case '0':
     default:
        camera_handle=0;
        break;
  }
  if(!(handle = dc1394_create_handle(camera_handle))) {
     cerr << "Couldn't get raw1394 handle" << endl;
     exit(1);
  }
  camera_nodes = dc1394_get_sorted_camera_nodes(handle, 0, 0, &numCameras,1);
  if(numCameras<1) {
     cerr << "I don't think you connected the camera? " << numCameras 
          << " " << camera_nodes<< endl;
     raw1394_destroy_handle(handle);
     exit(1);
  }
  node_id=-1;
  if( !camera_id ) {
        node_id = camera_nodes[0] ;
        channel = 0 ;
  }else if( ~camera_id < numCameras ) {
        node_id = camera_nodes[~camera_id] ;
        channel = ~camera_id ;
  }else {
     for(int i=0;i<numCameras;i++) {
	dc1394_get_camera_info(handle,camera_nodes[i],&camera_info);
	cout << "Found camera ID 0x"<< hex  
	       << camera_info.euid_64 << endl;
	if(camera_id && camera_id==camera_info.euid_64)	{
	       node_id=camera_nodes[i];
	       channel=i;
	}
     }
  }
  if(node_id <0) {
    if(camera_id) {
      cerr << "Camera with ID 0x" << hex << camera_id << " not found" << endl;
    }else {
       cerr << "No cameras found!" << endl;
    }
    exit(1);
  }
  dc1394_get_camera_info(handle,node_id,&camera_info);
  cout << "Assigned to ID 0x" << hex << camera_info.euid_64 << dec << endl;
  dc1394_init_camera(handle,node_id);
  printFM( handle, node_id );

  // setting the format and mode
  size = getSize( format, mode );
  quadlet_t masks = 0 ;
  if( format7 ) {
    format = 7 ;
    unsigned int xFormat = getFormat(format) ;
    dc1394_query_supported_formats( handle, node_id, &masks );
    if( ! isAvailable( format, masks ) ) {
      cerr<<" format 7 not available "<<endl;
      raw1394_destroy_handle(handle);
      exit(1);
    }
    dc1394_query_supported_modes( handle, node_id, xFormat, &masks );
    mode = minMask( masks ) ;
    unsigned int xMode = getMode(format,mode) ;
    dc1394_query_format7_color_coding( handle, node_id, xMode, &masks );
    Color c = (Color) minMask( masks );
    // potential bug: is the camera really using this color encoding?
    unsigned int horizontal_size,vertical_size;
    dc1394_query_format7_max_image_size( handle, node_id, xMode, 
					 &horizontal_size, &vertical_size );
    unsigned int xpos = 0, ypos = 0 ;
    if( grab_center ) {
      xpos = horizontal_size - size.Width() ;
      ypos = vertical_size - size.Height() ;
    }
    dc1394_set_format7_image_position( handle, node_id, xMode, xpos, ypos );
    dc1394_set_format7_image_size( handle, node_id, xMode, 
				   size.Width(), size.Height() );
    unsigned int bpp;
    dc1394_query_format7_recommended_byte_per_packet( handle, node_id,
						      xMode, &bpp );
    unsigned long long int total;
    correctCall( &dc1394_query_format7_total_bytes,
		 handle, node_id, xMode, &total );
    if( dc1394_setup_format7_capture
	( handle, node_id, channel, xMode, getSpeed(), bpp, xpos, ypos, 
	  size.Width(), size.Height(), &camera) != DC1394_SUCCESS ) {
      cerr << "unable to setup camera" << endl;
      dc1394_release_camera(handle,&camera);
      raw1394_destroy_handle(handle);
      exit(1);
    }
    v_mmap.packet_size = bpp ;
    v_mmap.buf_size    = total ;
    convert            = color_f<IMTYPE>(c) ;

  }else { // not format 7
    unsigned int xFormat = getFormat(format) ;
    unsigned int xMode = getMode(format,mode) ;
    if( framerate == -1 ) {
      dc1394_query_supported_framerates( handle, node_id,xFormat,xMode,&masks);
      framerate = maxFrameRate( masks );
    }
    if( framerate == -1 ) {
      cerr << "Unable to find a possible frame rate for format " << format
	   << " and mode " << mode << endl ;
      raw1394_destroy_handle(handle);
      exit(1);
    }
    unsigned int xFrameRate = getFrameRate( framerate );
    if (dc1394_setup_capture(handle,node_id,channel,xFormat,xMode,
			     getSpeed(),xFrameRate,&camera)!= DC1394_SUCCESS) {
      cerr << "unable to setup camera" << endl;
      dc1394_release_camera(handle,&camera);
      raw1394_destroy_handle(handle);
      exit(1);
    }
    v_mmap.packet_size = getPacketSize( format,mode, xFrameRate );
    v_mmap.buf_size    = getBufSize( format, mode );
    convert            = getConvert<IMTYPE>( format, mode );
  } // end of not format 7 

  v_mmap.channel     = channel;
  v_mmap.sync_tag    = 1;
  v_mmap.nb_buffers  = n_buffers;
  v_mmap.flags       = VIDEO1394_SYNC_FRAMES;
  cout << "Listening on channel " << v_mmap.channel << endl;
  cout << "Using format " << format << " mode " << mode ;
  if( !format7 ) {
    cout << " (" << strConvert( format, mode ) << " " 
	 << size.Width() << "x" << size.Height() << ") " 
	 << " frame rate " << framerate 
	 << " (" << strFrameRates[framerate] << " fps)." ;
  }
  cout << endl;

  fd = -1 ;
  open(dev_name);
  init_map(size,n_buffers);

  // setting the gain 
  if( gain != -1 ) {
    unsigned int min_gain_val, max_gain_val;
    dc1394_get_min_value(handle,node_id,FEATURE_GAIN,&min_gain_val);
    dc1394_get_max_value(handle,node_id,FEATURE_GAIN,&max_gain_val);
    if(gain <= max_gain_val && gain >= min_gain_val) {
      set_gain(gain);
    }else {
      cerr<< " the gain value is beyond limits[" << min_gain_val
          << ","<< max_gain_val<<"]..exiting"<<endl;
      exit(1);
    }
  }
  if(optical_filter!=BAYER_Not_Valid)
    if(dc1394_set_optical_filter(handle,node_id,optical_filter)
          !=DC1394_SUCCESS)
    {
      cerr << "Unable to set optical Bayer filter selected " << 
             optical_filter<< endl;
      cerr<< "switching function off." << endl;
      optical_filter=BAYER_Not_Valid;
    }
  
  // set the saturation
  if( saturation != -1 ) {
    unsigned int min_sat_val, max_sat_val;
    dc1394_get_min_value(handle,node_id,FEATURE_SATURATION,&min_sat_val);
    dc1394_get_max_value(handle,node_id,FEATURE_SATURATION,&max_sat_val);
    if(saturation <= max_sat_val && saturation >= min_sat_val) {
      set_saturation(saturation);
    }else {
      cerr<< " the saturation value is beyond limits..exiting"<<endl;
      exit(1);
    }
  }

  // setting the sharpness
  if( sharpness != -1 ) {
    unsigned int min_sharp_val, max_sharp_val;
    dc1394_get_min_value(handle,node_id,FEATURE_SHARPNESS,&min_sharp_val);
    dc1394_get_max_value(handle,node_id,FEATURE_SHARPNESS,&max_sharp_val);
    if(sharpness <= max_sharp_val && sharpness >= min_sharp_val) {
      set_sharpness(sharpness);
    }else {
      cerr<< " the sharpness value is beyond limits..exiting"<<endl;
      exit(1);
    }
  }

  // setting the shutter
  if( shutter != -1 ) {
    unsigned int min_shutter_val, max_shutter_val;
    dc1394_get_min_value(handle,node_id,FEATURE_SHUTTER,&min_shutter_val);
    dc1394_get_max_value(handle,node_id,FEATURE_SHUTTER,&max_shutter_val);
    if(shutter <= max_shutter_val && shutter >= min_shutter_val) {
      set_shutter(shutter);
    }else {
      cerr<< " the shutter value is beyond limits [" << min_shutter_val
          << "," << max_shutter_val << "]..exiting"<<endl;
      exit(1);
    }
  }

  // setting the gamma
  if( gamma != -1 ) {
    unsigned int min_gamma_val, max_gamma_val;
    dc1394_get_min_value(handle,node_id,FEATURE_GAMMA,&min_gamma_val);
    dc1394_get_max_value(handle,node_id,FEATURE_GAMMA,&max_gamma_val);
    if(gamma <= max_gamma_val && gamma >= min_gamma_val) {
      set_gamma(gamma);
    }else {
      cerr<< " the gamma value "<< gamma <<" is beyond limits..exiting"<<endl;
      exit(1);
    }
  }

  // setting the exposure
  if( exposure != -1 ) {
    unsigned int min_exposure_val, max_exposure_val;
    dc1394_get_min_value(handle,node_id,FEATURE_EXPOSURE,&min_exposure_val);
    dc1394_get_max_value(handle,node_id,FEATURE_EXPOSURE,&max_exposure_val);
    if(exposure <= max_exposure_val && exposure >= min_exposure_val) {
      set_exposure(exposure);
    }else {
      cerr<< " the exposure value is beyond limits..exiting"<<endl;
      exit(1);
    }
  }

  // setting u and v components
  if(uv[0] >-1 || uv[1] > -1)
  {
    if(uv[0]*uv[1]<0) 
    {
      cerr << "just one of the white balance components set!!" << endl;
      exit(1);
    }
    dc1394_set_white_balance(handle,node_id,uv[0],uv[1]);
  }

  // start iso trasmission
  dc1394_start_iso_transmission(handle,node_id);
  buffer_index=0;
  nowait_flag=false;

  // setting the external trigger
  dc1394_set_trigger_on_off( handle, node_id,
			     ext_trigger ? DC1394_TRUE : DC1394_FALSE );
  if( ext_trigger ) {
    dc1394_set_feature_value( handle, node_id, FEATURE_TRIGGER, ext_trigger );
  }
}

template <class IMTYPE>
XVDig1394<IMTYPE>::~XVDig1394() {
  dc1394_stop_iso_transmission(handle,node_id);
  raw1394_destroy_handle(handle);
  if(fd>=0) close();
}

template <class IMTYPE>
IMTYPE & XVDig1394<IMTYPE>::current_frame_continuous(void)
{
   struct video1394_wait w_descr;

   if (!nowait_flag) {// Start the cycle
        nowait_flag = true;
        initiate_acquire(buffer_index);
        initiate_acquire(buffer_index+1);
	wait_for_completion(buffer_index);
	convert(mm_buf[0],image_buffers[0],size.Width()*size.Height());
   }

   v_mmap.flags=VIDEO1394_SYNC_FRAMES;
   w_descr.channel=v_mmap.channel;
   w_descr.buffer=(buffer_index+1)%n_buffers;
   if(!ioctl(fd,VIDEO1394_LISTEN_POLL_BUFFER,&w_descr))
   {
      buffer_index=(buffer_index+1)%n_buffers;     
      initiate_acquire((buffer_index+1)%n_buffers);     
      convert(mm_buf[buffer_index],image_buffers[buffer_index],
	      size.Width()*size.Height());
   }
   return frame(buffer_index);
}
template class XVDig1394<XVImageRGB<XV_RGB15> >;
template class XVDig1394<XVImageRGB<XV_RGB16> >;
template class XVDig1394<XVImageRGB<XV_RGB24> >;
template class XVDig1394<XVImageRGB<XV_TRGB24> >;
template class XVDig1394<XVImageRGB<XV_RGBA32> >;
template class XVDig1394<XVImageRGB<XV_GLRGBA32> >;
template class XVDig1394<XVImageYUV<XV_YUV24> >;

#endif
