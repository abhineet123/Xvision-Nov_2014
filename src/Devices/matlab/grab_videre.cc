#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "XVWindowX.h"
#include "XVAffineWarp.h"
#include "XVMpeg.h"
#include "Videre.h"
#include "StereoVidere.h"
#include "XVImageBase.h"
#include "XVImageIO.h"
#include <mex.h>
#include <matrix.h>

static	StereoVidere<XVImageRGB<XV_TRGB24>,XVImageScalar<short> > 
                       * grabber=NULL;

static  bool warped_flag=false;

static int get_image(void) {


  if (!grabber) {
    grabber = 
	new StereoVidere<XVImageRGB<XV_TRGB24>,
                         XVImageScalar<short> >();
    grabber->set_stoc(true); 
    grabber->wait_for_completion(0);
    grabber->wait_for_completion(0);
    grabber->wait_for_completion(0);
  }
  if (!grabber) return 0;
  grabber->wait_for_completion(0);
  warped_flag=false;
  return 1;
}

void mexFunction(int nlhs, mxArray *plhs[], 
		                 int nrhs, const mxArray *prhs[])
{

  int i;  

  if(nrhs!=2)
  {
    mexErrMsgTxt("videre_getImage expects 2 input arguments");
  }
  if((int)mxGetScalar(prhs[0])) {   // new image requested?
    if(!get_image()) 
	  mexErrMsgTxt("videre_getImage: could not get image");
  }
  if(!grabber) 
	  mexErrMsgTxt("videre_getImage: framegrabber is not open");
  switch((int)mxGetScalar(prhs[1]))
  {
      case 0:   // 2 grayscale images
      {
	if (nlhs != 2) {
           mexErrMsgTxt("videre_getImage expects 2 output arguments");
	}
	const int dims[2]={grabber->frame(0).Width(),
			   grabber->frame(0).Height()};
	plhs[0] = mxCreateNumericArray(2,dims,mxUINT8_CLASS,mxREAL);
	plhs[1] = mxCreateNumericArray(2,dims,mxUINT8_CLASS,mxREAL);
	if(!warped_flag) {grabber->calc_warped();warped_flag=true;}
        XVImageScalar<u_char> **warped=grabber->get_warped();
	memcpy(mxGetPr(plhs[0]),warped[1]->data(),
	       grabber->frame(0).Width()*grabber->frame(0).Height());
	memcpy(mxGetPr(plhs[1]),warped[0]->data(),
	       grabber->frame(0).Width()*grabber->frame(0).Height());
	warped_flag=true;
	break;
      }
      case 1:	// color image
      {
	if (nlhs != 1) {
           mexErrMsgTxt("videre_getImage expects 1 output element");
	}
	const int dims[3]={3,grabber->frame(0).Width(),
			   grabber->frame(0).Height()};
	plhs[0] = mxCreateNumericArray(3,dims,mxUINT8_CLASS,mxREAL);
	
	memcpy(mxGetPr(plhs[0]),(char*)grabber->frame(0).data(),
	      grabber->frame(0).Width()*grabber->frame(0).Height()*3);
	break;
      }
      case 2:   // disparity image
      {
	if (nlhs != 1) {
           mexErrMsgTxt("videre_getImage expects 1 output argument");
	}
         if(warped_flag)
	 {
           mexErrMsgTxt("videre_getImage: images were already warped");
	 }
	 XVImageScalar<short>*stereo_image=grabber->calc_stereo();
	 warped_flag=true;
	 const int dims[2]={stereo_image->Width(),
		            stereo_image->Height()};
	 plhs[0] = mxCreateNumericArray(2,dims,mxUINT16_CLASS,mxREAL);
	 for(int i=0;i<stereo_image->Height();i++)
	    memcpy(((char*)mxGetPr(plhs[0]))+
			    i*stereo_image->Width()*2,
	           ((char *)stereo_image->data())+
		            i*stereo_image->SizeX()*2,
		     stereo_image->Width()*2);
	 break;
      }
      case 3:   // getSP
      {
	if (nlhs != 6) {
           mexErrMsgTxt("videre_getImage expects 6 output arguments");
	}
	svsSP params=grabber->getParams();
	plhs[0]= mxCreateDoubleMatrix(1,1,mxREAL);
	plhs[1]= mxCreateDoubleMatrix(1,1,mxREAL);
	plhs[2]= mxCreateDoubleMatrix(1,1,mxREAL);
	plhs[3]= mxCreateDoubleMatrix(1,1,mxREAL);
	plhs[4]= mxCreateDoubleMatrix(1,1,mxREAL);
	plhs[5]= mxCreateDoubleMatrix(1,1,mxREAL);
        *mxGetPr(plhs[0])=params.left.Cx;
        *mxGetPr(plhs[1])=params.left.Cy;
        *mxGetPr(plhs[2])=grabber->resleft();
        *mxGetPr(plhs[3])=grabber->restop();
        *mxGetPr(plhs[4])=params.left.f;
        *mxGetPr(plhs[5])=params.Tx;
	break;
      }
      default:  
	 mexErrMsgTxt("videre_getImage: unknown argument");
	 break;
  }
}
