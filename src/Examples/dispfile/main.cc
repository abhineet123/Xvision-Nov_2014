#include "config.h"
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <XVRemoteWindowX.h>
#include "XVImageSeq.h"
#include "StereoVidere.h"

//#include <fstream>

//#define REMOVE_FLOOR
#define VIDEO   //show grayscale image


typedef StereoVidere<XVImageRGB<XV_RGB>, XVImageScalar<u_short> > VID;
typedef XVRemoteWindowX<XV_RGB>                                   WIN;

static VID* vid;

void sighndl(int ws) {
  cerr << "SIGINT called" << endl;
  if(vid) delete vid;
  exit(0);
}

void sighndl2(int ws) {
  cerr << "SIGSEGV called" << endl;
  if(vid) delete vid;
  exit(0);
}

void writeDisparityToFile( XVImageScalar<u_short> &disp_image, char filename[] )
{
	  using namespace std ;

	  XVImageIterator<u_short>  rptr(disp_image);

	  ofstream outp_file ;
	  int width = disp_image.Width() ;
	  int height = disp_image.Height() ;
	  int size = width * height ;
	  int shift = 2 ;

	  outp_file.open (filename) ;
	  // Header: bla, bla, bla.....
	  outp_file << "# Etwas 4.0 Disparity File\n" ;
	  outp_file << "\nwidth " <<  width ;
	  outp_file << "\nheight " <<  height ;
	  outp_file << "\n\n" ;

	  int w = 0 ;
	  for(int count = 0 ; count < size ; ++rptr, ++count) {
		  short value=(short)*rptr;//(*rptr<60000 && *rptr>0)? *rptr>>shift : -1;
		  outp_file << value ;
		  w++ ;
		  if (w == width) { outp_file << "\n" ; w = 0 ; }
		  else { outp_file << " " ; }
//		  if (*rptr != -1) outp_file << "-1" ;
//		  else outp_file << "*" ;
//		  cout << *rptr ; //<< endl ;
	  }

	  outp_file.close () ;

//	  for (int j = 0 ; 0 < height ; j++) {
//		  for(int i = 0 ; i < width; i++) {
//
//			  outp_file << (*rptr ) ;
//			  fistream << ++rptr ;
//		  }
//		  fistream << "\n";
//	  }

}

//void writeDisparityToFile(XVImageScalar<u_short> &disp_image, string filename)
//{
//	  ifstream infile ;
//	  int size = im.Width() * im.Height() ;
//
//	  XVImageIterator<u_short>  rptr(disp_image);
//
//	  // Header: bla, bla, bla.....
//
//	  for(int j=height; j>0; j--)
//	  {
//		  for(int j=width; j>0; j--)
//		  {
//			  fistream << ++rptr ;
//		  }
//		  fistream << "\n";
//	  }
//
//}

static void
calc_display(XVImageScalar<u_short> &disp_image,
                                XVImageRGB<XV_RGB> &im)
{
  im.resize(disp_image);
  int size=im.Width()*im.Height();
  int   shift=2;

  XVImageIterator<u_short>  rptr(disp_image);
  XV_RGB*  wptr=im.lock() ;
  unsigned char value;

  for(int count=0;count<size;++rptr,wptr++,count++)
  {
     value=(*rptr<60000)? *rptr>>shift : 0;
     //value=(*rptr<60000 && *rptr>0)? 128 : 0;
     wptr->setR(value);
     wptr->setB(value);
     wptr->setG(value);
  }
  im.unlock();
}

int main(int argc, char* argv[]) {
  signal(SIGINT, sighndl);
  signal(SIGSEGV, sighndl2);


  char * config = getenv("VIDERE_CONFIG");

  XVImageSeq<XVImageRGB<XV_RGB> > left_seq("image_left_%03d.ppm",1,2939);
  XVImageSeq<XVImageRGB<XV_RGB> > right_seq("image_right_%03d.ppm",1,2939);
  if(config) {
    vid = new VID(config);
  }
  else {
    vid = new VID();
  }

  XVImageScalar<u_short> *disp_image;
  XVImageScalar<u_char> image[2],**images;
  XVImageRGB<XV_RGB>     im;
  XVImageRGB<XV_RGB>     li,ri,dummy;
#ifdef VIDEO
  RGBtoScalar(li=left_seq.next_frame_continuous(),image[0]);
  RGBtoScalar(ri=right_seq.next_frame_continuous(),image[1]);
  vid->LoadImages(image[0],image[1],dummy,false);

  disp_image = vid->calc_stereo();
  XVWindowX<XV_RGB>   video_left(li.Width(), li.Height(), disp_image->Width()+3,0);
  XVWindowX<XV_RGB>   video_right(ri.Width(), ri.Height(), disp_image->Width()+li.Width()+3,0);
  video_left.map();
  video_right.map();
#endif
  WIN 			 win(*disp_image);
  win.map();
  struct timeval start_time, end_time;
  unsigned long frame_counter=1;
  char filename[100];
  while(1) {
    gettimeofday(&start_time, NULL);

    for(int i = 0; i < 50; i++) {
      RGBtoScalar(li=left_seq.next_frame_continuous(),image[0]);
      RGBtoScalar(ri=right_seq.next_frame_continuous(),image[1]);
      vid->LoadImages(image[0],image[1],dummy,false);
      disp_image = vid->calc_stereo();
      if(!disp_image) continue;

      sprintf(filename, "Disparity/image_disparity_%04d.pgm", frame_counter);
      //XVWritePGM(*disp_image,filename);
      writeDisparityToFile(*disp_image,filename);
      frame_counter++;


      calc_display(*disp_image,im);
      win.CopySubImage(im);
      win.swap_buffers();
      win.flush();
#ifdef VIDEO
      video_left.CopySubImage(li);
      video_left.swap_buffers();
      video_left.flush();
      video_right.CopySubImage(ri);
      video_right.swap_buffers();
      video_right.flush();
#endif
      //sleep(2);
    }

    gettimeofday(&end_time, NULL);

    cout << 50 / ((end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) * 1e-6)
	 << " [Hz]"<< endl;
  }

  delete vid;
  return 1;
}

