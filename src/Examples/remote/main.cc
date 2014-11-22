# include "config.h"
# include <XVWindowX.h>
# include <XVMpeg.h>
//# include <XVDig1394.h>

# include <XVRemoteWindowX.h>

int main( int argc, char* argv[] ) {
  char buf[256] ;
  int l = 0, x[2] ;
  if( argc < 2 ) {
    printf( "Missing mpeg file name\n" );
    return 1 ;
  }
  XVMpeg<XVImageRGB<XV_RGB> > grabber( argv[1] );
  //XVDig1394<XVImageRGB<XV_RGB> > grabber( DC_DEVICE_NAME, "S1R1" );
  XVImageRGB<XV_RGB> image = grabber.next_frame_continuous();
  XVRemoteWindowX<XV_RGB> remote( image, .3, .3, 600, 400, 
				  "XVRemoteWindowX", ButtonPressMask );
  remote.map();
  printf( "%d %d %d %d\n", remote.PosX(), remote.PosY(), remote.Width(),
                           remote.Height() );
  for( int i = 0 ; ; i ++ ) {
    image = grabber.next_frame_continuous();
    //image.setSubImage( 100, 100, 300, 200 );
    remote.CopySubImage( image );
    remote.drawLine( 0, 0, 300, 200 );
    remote.fillEllipse( 150, 100, 90, 60, "green" );
    if( remote.check_events( x ) & BUTTON_PRESSED ) {
      l = sprintf( buf, "%d %d", x[0], x[1] );
    }
    if( l ) {
      remote.ClearWindow();
      remote.drawString( 150, 100, buf, l, "blue" );
    }
    remote.swap_buffers();
    remote.flush();
  }
  //sleep(3);
  return 0 ;
}
