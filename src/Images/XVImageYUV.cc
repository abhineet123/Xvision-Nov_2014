// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVImageYUV.h>

#ifndef COLORRANGE
#define COLORRANGE 256
#endif

//template class XVImageYUV<XV_YUV16>;
template class XVImageYUV<XV_YUV24>;


bool bRGB2YUVTableBuilt=false;
bool bYUV2RGBTableBuilt=false;

unsigned char TABLE_YUV_TO_R[COLORRANGE][COLORRANGE]; // R = t_r[Y][V]
unsigned char TABLE_YUV_TO_B[COLORRANGE][COLORRANGE]; // B = t_b[Y][U]
unsigned char TABLE_YUV_TO_PG[COLORRANGE][COLORRANGE]; 
unsigned char TABLE_YUV_TO_G[COLORRANGE][COLORRANGE]; // G = t_g[Y][t_g[U][V]]

unsigned char TABLE_RGB_TO_PY[COLORRANGE][COLORRANGE]; // py = t_py[R][G]
unsigned char TABLE_RGB_TO_Y[COLORRANGE][COLORRANGE];  // Y  = t_y[py][B]
unsigned char TABLE_RGB_TO_U[COLORRANGE][COLORRANGE];  // U  = t_u[B][Y]
unsigned char TABLE_RGB_TO_V[COLORRANGE][COLORRANGE];  // V =  t_v[R][V]]

void buildYUV2RGBTable()
{
  if( bYUV2RGBTableBuilt) return;
  int r, b, pg, g ;
  
  for( int i = 0 ; i < COLORRANGE ; i ++ ) {
    for( int j = 0 ; j < COLORRANGE ; j ++) {

  /*  interger implemntation
      r = i + (((j-128) * 9339)  / 8192);
      b = i + (((j-128) * 16646) / 8192);
      pg = (((i-128)*3228)/8192) + (((j-128)*4760)/8192) + 128 ;
  */
      //float-point implementation
      r  = round( i +  (j-128) * 1.1402   );
      b  = round( i +  (j-128) * 2.0326   );
      pg = round( (i-128)*0.3948+ (j-128)*0.5808 + 128 );
      g = i - j + 128 ;
      
      TABLE_YUV_TO_R[i][j] = r < 0 ? 0 : r >= COLORRANGE ? COLORRANGE-1 : r ;
      TABLE_YUV_TO_B[i][j] = b < 0 ? 0 : b >= COLORRANGE ? COLORRANGE-1 : b ;
      TABLE_YUV_TO_G[i][j] = g < 0 ? 0 : g >= COLORRANGE ? COLORRANGE-1 : g ;
      TABLE_YUV_TO_PG[i][j]= pg ; // impossible to underflow or overflow
    }
  }

  bYUV2RGBTableBuilt=true;
}

void buildRGB2YUVTable()
{
  if( bRGB2YUVTableBuilt) return;
  int py, y, u, v;
  
  for( int i = 0 ; i < COLORRANGE ; i ++ ) {
    for( int j = 0 ; j < COLORRANGE ; j ++) {
     /*
      py =  (i*2449) / 8192 + (j*4809)/8192;
      y = i+(j*934)/8192;
      u = ( (i-j)*4031 )/8192 + 128;
      v = ( (i-j)*7184 )/8192 + 128;
    */
  
    
      py= round( i*0.299+ j*0.587 );
      y = round( i+j*0.114  );
      u = round( (i-j)*0.492 + 128 );
      v = round( (i-j)*0.877 + 128 );

      TABLE_RGB_TO_PY[i][j] =py;
      TABLE_RGB_TO_Y[i][j]  = y < 0 ? 0 : y >= COLORRANGE ? COLORRANGE-1 : y ;
      TABLE_RGB_TO_U[i][j]  = u < 0 ? 0 : u >= COLORRANGE ? COLORRANGE-1 : u ;
      TABLE_RGB_TO_V[i][j]  = v < 0 ? 0 : v >= COLORRANGE ? COLORRANGE-1 : v ; 
    }
  }

  bRGB2YUVTableBuilt=true;
}

