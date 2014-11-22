//# define USE_RECT_RASTER

# include <config.h>
# include <XVWindowX.h>
# include <XVImageIterator.h>
# include <XVImageIO.h>
# ifdef USE_RECT_RASTER
# include <XVRectRaster.h>
# else
# include <XVAffineWarp.h>
# endif

# include <iomanip>
# include <string>
using namespace std ;

double time_passed(void) {
  static timeval last = {0,0} ;
  timeval now ;
  gettimeofday( &now, 0 );
  double r = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec)*1e-6 ;
  if( r<0 ) { 
    cerr << r << " ! " << now.tv_sec << ":" << now.tv_usec 
         << " - " << last.tv_sec << ":" << last.tv_usec << endl ; 
    exit(-1); 
  }
  last.tv_sec = now.tv_sec ;
  last.tv_usec = now.tv_usec ;
  return r ;
}

int main( int argc, char* argv[] ) {
  const int max_count = 100 ;
  int count = 0 ;
  double t = 0, rate ;

  if( argc < 2 ) {
    cerr << "Usage: " << argv[0] << " <image_file_name>" << endl;
    return -1 ;
  }
  XVImageRGB<XV_RGB> image ;
  if( ! XVReadImage( image, argv[1] ) ) {
    cerr << "Unable to open image file " << argv[1] << endl;
  }
  int l = image.Width() >= image.Height() ? image.Width() : image.Height() ;
  XVImageRGB<XV_RGB> display_image( l, l );

  XVWindowX<XV_RGB> window( image, 0, 0 );
  window.map();
  window.CopySubImage( image );
  window.swap_buffers();
  window.flush();
  count = 0 ;
 
  const double rao = 0.1 ;
  double theta = 0 ;
  XVCoord2D center( image.PosX()+image.Width()/2, 
		    image.PosY()+image.Height()/2 );
  XVPosition center_p( image.PosX()+image.Width()/2,
		       image.PosY()+image.Height()/2 );
  while(1) {
    time_passed() ;
# ifdef USE_RECT_RASTER
    intAffineWarp( image, display_image, center_p, theta += rao, 1, 1 );
# else
    XVAffineWarp<XV_RGB> w( theta += rao );
    w.warp( image, display_image, center );
# endif
    t += time_passed() ;

    window.CopySubImage( display_image );
    window.swap_buffers();
    window.flush();

    if( ++count >= max_count ) {
      rate = t / count ;
      cout << "Rate: " << setiosflags( ios::fixed ) << setprecision(1) 
	   << rate*1e3 << " [ms/frame] " << endl ;
      count = 0 ;
      t = 0 ;
    }
  }
  return 0 ;
}
