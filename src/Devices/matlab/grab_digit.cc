#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "XVDig1394.h"
#include <mex.h>
#include <matrix.h>

static	XVDig1394<XVImageRGB<XV_TRGB24> > * grabber=NULL;

static int get_image(void) {

  if (!grabber) {
    grabber = 
	new XVDig1394<XVImageRGB<XV_TRGB24> >(DC_DEVICE_NAME,"S1R0");
    if (!grabber) return 0;
    grabber->next_frame_continuous();
  }
  return 1;
}

void mexFunction(int nlhs, mxArray *plhs[], 
		                 int nrhs, const mxArray *prhs[])
{

  if(nrhs!=1)
  {
    mexErrMsgTxt("videre_getImage expects 1 input argument");
  }
  if((int)mxGetScalar(prhs[0])) {   // new image requested?
    if(!get_image()) 
	  mexErrMsgTxt("videre_getImage: could not get image");
  }
  if(!grabber) 
	  mexErrMsgTxt("videre_getImage: framegrabber is not open");
  if (nlhs != 1) {
           mexErrMsgTxt("videre_getImage expects 1 output element");
  }
  const int dims[3]={3,grabber->frame(0).Width(),
                       grabber->frame(0).Height()};
  plhs[0] = mxCreateNumericArray(3,dims,mxUINT8_CLASS,mxREAL);
	
  memcpy(mxGetPr(plhs[0]),
         (char*)grabber->next_frame_continuous().data(),
	 grabber->frame(0).Width()*grabber->frame(0).Height()*3);
}
