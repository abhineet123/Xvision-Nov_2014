// Stereo functions in Stereo.cu.
extern "C" void stereoInit( int w, int h );
extern "C" void stereoUpload( const unsigned char *left, const unsigned char *right );
extern "C" void stereoProcess();
extern "C" void stereoDownload( float *disparityLeft, float *disparityRight );

