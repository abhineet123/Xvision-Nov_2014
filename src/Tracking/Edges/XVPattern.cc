// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVPattern.cc
 *
 * @author Sam Lang, Sidd Puri
 * @version $Id: XVPattern.cc,v 1.1.1.1 2008/01/30 18:43:46 burschka Exp $
 *
 * The function definitions of the XVPattern classes.
 *
 * @see XVPattern.h
 */

#include <iostream>
#include <string.h>
#include <stdio.h>

#include <XVImageScalar.h>
#include <XVImageIterator.h>
#include <XVPattern.h>


template <class T>
XVOffset
XVEdge<T>::find(XVImageScalar<T> & im){

  float tanalpha = tan (alpha);		     // set up constants
  int pad = round (half (im.Height()) * tanalpha);
  int swidth = im.Width() - 2 * pad;
  int cwidth = swidth - (mwidth - 1);

  int * sums = new int[3 * swidth + 7 * pad];	      // allocate space for sums


  int * midsum = & sums[pad];                   // at angle,
  int * possum = & sums[swidth + 3 * pad];      // at angle + alpha, and
  int * negsum = & sums[2 * swidth + 5 * pad];  // at angle - alpha

  memset(sums,0,sizeof(int)*(3*swidth + 7*pad));

  XVRowIterator<T> iter(im);

  const T * imagerow;
  for(int row = 0; row < im.Height(); ++row, iter.reset(row)) {   // compute sums
    imagerow = & (*(iter));
    int offset = round ((half (im.Height()) - row) * tanalpha);
					     // the following loop is optimized
					     // handle with care
    int loopend = swidth + abs (offset);
    for (int col = -abs (offset); col < loopend; ++col) {
      int val = (int)imagerow[pad + col];
      midsum[col]	   += val;
      possum[col + offset] += val;
      negsum[col - offset] += val;
    }
  }

  int * cors = new int[3 * cwidth];			     // allocate space for correlations

  int * midcor = & cors[0];
  int * poscor = & cors[cwidth];
  int * negcor = & cors[2 * cwidth];

  // compute correlations from sums

  sumcor (midsum, midcor, cwidth, mwidth, lineType());
  sumcor (possum, poscor, cwidth, mwidth, lineType());
  sumcor (negsum, negcor, cwidth, mwidth, lineType());

  int thresh = int(this->lower_threshold(im.Height())*_sensitivity);
		     // compute delta from correlations

  XVPeak midpeak,pospeak,negpeak;
  XVOffset delta;
  if (lineType() == none) {

    midpeak = findpeak (midcor, cwidth, thresh, lineType());
    pospeak = findpeak (poscor, cwidth, thresh, lineType());
    negpeak = findpeak (negcor, cwidth, thresh, lineType());

    float maxval = max2(max2(negpeak.val,midpeak.val),pospeak.val);
    float minval = min2(min2(negpeak.val,midpeak.val),pospeak.val);
    if (-minval < maxval)
      lineType()  = darklight;
    else
      lineType() = lightdark;
    delta.val = max2((float)(-minval),maxval);
    delta.x = 0.0;
  }
  else{

    midpeak = findpeak (midcor, cwidth, thresh);
    pospeak = findpeak (poscor, cwidth, thresh);
    negpeak = findpeak (negcor, cwidth, thresh);

    XVPeak maxpeak = interparbangle(negpeak.val, midpeak.val, pospeak.val);
    delta.val = maxpeak.val;

    float &ratio = maxpeak.x;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < -1.0) ratio = -1.0;
    delta.angle = alpha*ratio;
    if (ratio >= 0)
      delta.x =  ratio * pospeak.x + (1-ratio) * midpeak.x;
    else
      delta.x = -ratio * negpeak.x + (1+ratio) * midpeak.x;
  }

  if (delta.val < this->lower_threshold(im.Height()) )
    delta.val = 0;

  delta.val /= (im.Height() * mwidth);
  return delta;
}

template <class T>
XVOffset XVMaxEdge<T>::find (XVImageScalar<T> & im) {

  int swidth = im.Width();
  int cwidth = swidth - (mwidth - 1);

  if(cwidth < 2) throw XVEdgeException((char*)"XVMaxEdge: Image too small to find Edge");

  int * sums = new int[swidth];
  memset(sums,0,sizeof(int)*swidth);

  XVRowIterator<T> iter(im);

  for (int row = 0; row < im.Height(); ++row, iter.reset(row)) {   // compute sums
    for (int col = 0; col < swidth; ++col, ++iter) {
      sums[col] += (int)(*iter);
    }
  }

  int *cors = new int[cwidth];

  // compute correlations from sums

  sumcor (sums, cors, cwidth, mwidth, lineType());

  int thresh = int(this->lower_threshold(im.Height())*_sensitivity);
		     // compute delta from correlations

  XVPeak midpeak;
  XVOffset delta;
  delta.angle = 0;

  midpeak = findpeak (cors, cwidth, 0, lineType());
  if (midpeak.val > this->lower_threshold(im.Height())) {
    delta.val = midpeak.val;
    delta.x = midpeak.x;
  }
  else
    delta.val = 0;

  delete [] sums;
  delete [] cors;

  delta.val /= (im.Height() * mwidth);
  return delta;
}

template <class T>
XVGeneralEdge<T>::XVGeneralEdge (int mwidth_in, linetype line_type_in, float alpha_in) {
  mwidth = mwidth_in;
  alpha = alpha_in;
  trans_type = line_type_in;
  int i;

  mask = new int[mwidth];		     // initialize mask
  for (i = 0; i < mwidth/2 - 1; i++)
    mask[i] =  -2;
  mask[i++] =  -1;
  if (odd (mwidth))
    mask[i++] = 0;
  mask[i++] =   1;
  for (; i < mwidth; i++)
    mask[i] =   2;

  maxdiffangle = alpha;

  for (i = 0; i < mwidth/2; i++)	     // initialize simple mask
    mask[i] = -1;
  for (; i < mwidth; i++)
    mask[i] =  1;

  lthreshold = 0; //default_edge_threshold;
  uthreshold = 1000000;
  _sensitivity = default_sensitivity;
}


template <class T>
XVGeneralEdge<T>::XVGeneralEdge (int * mask_in, int mwidth_in, linetype line_type_in, float alpha_in) {

  mwidth = mwidth_in;
  alpha = alpha_in;
  trans_type = line_type_in;
  int i;

  mask = new int[mwidth_in];
  for(int mm = 0; mm < mwidth_in; ++mm) {
    mask[mm] = mask_in[mm];
  }

  maxdiffangle = alpha;

  lthreshold = 0; //default_edge_threshold;
  uthreshold = 1000000;
  _sensitivity = default_sensitivity;
};

template <class T>
XVGeneralEdge<T>::XVGeneralEdge(const XVGeneralEdge & e) {
  *this  = e;
  mask = new int[mwidth];
  memcpy(mask,e.mask,sizeof(int)*mwidth);
}

//-----------------------------------------------------------------------------

template <class T>
XVOffset XVGeneralEdge<T>::find (XVImageScalar<T> & im)
{

  float tanalpha = tan (alpha);		            // set up constants
  int pad = round (half (im.Height()) * tanalpha);
  int swidth = im.Width() - 2*pad;
  int cwidth = swidth - (mwidth - 1);

  int * cors = new int[3 * cwidth + 7 * pad];

  int * midcor = & cors[pad];                       //  at angle,
  int * poscor = & cors[cwidth + 3 * pad];          //  at angle + alpha, and
  int * negcor = & cors[2 * cwidth + 5 * pad];      //  at angle - alpha
  int i;

  memset(cors,0,sizeof(int)*(3*cwidth + 7*pad));

  XVRowIterator<T> iter(im);

  const T * imagerow;
  if ((trans_type == any ) || (trans_type == none)) {
    for (int row = 0; row < im.Height(); ++row) {   // compute correlations
      iter.reset(row);
      imagerow = & (*iter);
      int offset = round ((half (im.Height()) - row) * tanalpha);
					     // the following loop is optimized
					     // handle with care
      int loopend = cwidth + abs (offset);
      for (int col = -abs (offset); col < loopend; col++) {
	const T * imageplace = & imagerow[pad + col];
	int sum = 0;
	register int *regmask = mask;
	for (i = mwidth - 1; i >= 0; i--)
	  sum += ((int)imageplace[i]) * regmask[i];
	sum = modsum(sum,trans_type)/256;
	midcor[col]          += sum;
	poscor[col + offset] += sum;
	negcor[col - offset] += sum;
      }
    }
  }
  else {
    int * sums = new int[3 * swidth + 7 * pad];

    int * midsum = & sums[pad];                    // at angle,
    int * possum = & sums[swidth + 3 * pad];       // at angle + alpha, and
    int * negsum = & sums[2 * swidth + 5 * pad];   // at angle - alpha

    memset(sums,0,sizeof(int)*(3*swidth + 7*pad));

    for (int row = 0; row < im.Height(); ++row) {   // compute sums
      iter.reset(row);
      imagerow = & (*iter);
      int offset = round ((half (im.Height()) - row) * tanalpha);
					     // the following loop is optimized
					     // handle with care
      int loopend = swidth + abs (offset);
      for (int col = -abs (offset); col < loopend; col++) {
	int val = (int)imagerow[pad + col];
	midsum[col]	   += val;
	possum[col + offset] += val;
	negsum[col - offset] += val;
      }
    }

    int imageplace;
    for (int col = 0; col < cwidth; col++) {
      register int *regmask = mask;
      for (i = mwidth - 1; i >= 0; i--) {
	imageplace = col+i;
	midcor[col] += midsum[imageplace]* regmask[i];
	poscor[col] += possum[imageplace]* regmask[i];
	negcor[col] += negsum[imageplace]* regmask[i];
      }
      midcor[col] = modsum(midcor[col],trans_type)/256;
      poscor[col] = modsum(poscor[col],trans_type)/256;
      negcor[col] = modsum(negcor[col],trans_type)/256;
    }

    delete sums;
  }

  XVPeak midpeak = findpeak (midcor, cwidth,
			     this->lower_threshold(im.Height()), trans_type),
    pospeak = findpeak (poscor, cwidth, this->lower_threshold(im.Height()),trans_type),
    negpeak = findpeak (negcor, cwidth, this->lower_threshold(im.Height()),trans_type);

  XVOffset delta = {0,0};

  // if this is a dynamic line, set up the right type

  int minval = int(min2(midpeak.val,min2(pospeak.val,negpeak.val)));
  int maxval = int(max2(midpeak.val,max2(pospeak.val,negpeak.val)));

  if (trans_type == none) {
    if (minval * maxval > 0) {
      if (minval < 0) {
	trans_type = lightdark;
	midpeak.val = - midpeak.val;
	pospeak.val = - pospeak.val;
	negpeak.val = - negpeak.val;
	maxval = - minval;
      }
      else {
	trans_type = darklight;
      }
    }
    else {

      delta.val /= (im.Height() * mwidth);
      return delta;
    }
  }

  // Edge was ambiguous

  if (minval == 0) {
    delta.val = -1;
    return delta;
  }

  // Edge was not found

  if (maxval < this->lower_threshold(im.Height())) {
    return delta;
  }

  // Set up a threshold for next time

  //  this->lower_threshold = int(.9 * maxval);

  XVPeak maxpeak = interparbangle (negpeak.val, midpeak.val, pospeak.val);

  //  delta.angle = interbell (negpeak.val, midpeak.val, pospeak.val) * alpha;
  delta.angle = maxpeak.x * alpha;
  delta.val = maxpeak.val;
  if ((delta.angle) >  maxdiffangle) delta.angle =  maxdiffangle;
  if ((delta.angle) < -maxdiffangle) delta.angle = -maxdiffangle;
  float ratio = delta.angle / maxdiffangle;

  if (ratio >= 0)
    delta.x =  ratio * pospeak.x + (1-ratio) * midpeak.x;
  else
    delta.x = -ratio * negpeak.x + (1+ratio) * midpeak.x;

  delete [] cors;
  delta.val /= (im.Height() * mwidth);
  return delta;
}

//-----------------------------------------------------------------------------
//  Member functions for XVShortEdge
//-----------------------------------------------------------------------------

template <class T>
XVShortEdge<T>::XVShortEdge (int mwidth_in) {

  mwidth = mwidth_in;
  if (odd (mwidth)) mwidth++;		     // the algorithm only makes sense
                                             // for even mask widths
  lthreshold = default_edge_threshold;
  uthreshold = 256;
  lthreshold = int(default_sensitivity);
}

template <class T>
XVOffset XVShortEdge<T>::find (XVImageScalar<T> & im) {

  int cwidth = im.Width() - mwidth - 1;

  // compute sums

  int *sums = new int[im.Width()];

  XVImageIterator<T> iter(im);

  memset(sums,0,sizeof(int)*(im.Width()));

  for (int col = 0; col < im.Width(); col++, ++iter) {
    sums[col] += (int)(*iter);
  }

  // compute correlations from sums.
  int * cors = new int[cwidth];
  sumcor (sums, cors, cwidth, mwidth);

  int thresh = int(this->lower_threshold(im.Height())*_sensitivity);

  // find maximum peak.
  XVPeak res = findpeak (cors, cwidth, thresh);

  delete [] sums;
  delete [] cors;

  XVOffset delta;
  delta.x = res.x;
  delta.angle = 0.0;
  delta.val = res.val;

  delta.val /= (im.Height() * mwidth);
  return delta;                                      // dummy return value
}

//  template <class T>
//  void XVShortEdge<T>::getlowout_ct (XVImageScalar<T> & im, SE_out & lowout) {

//    int cwidth = im.Width() - mwidth - 1;

//    // compute sums

//    int * sums = new int[im.Width()];
//    memset(sums,0,sizeof(int)*(im.Width()));

//    XVImageIterator<T> iter(im);

//    for (int col = 0; col < im.Width(); col++, ++iter) {
//      sums[col] += (int)(*iter);
//    }

//    // compute correlations from sums.
//    int *cors = new int[cwidth];
//    sumcor (sums, cors, cwidth, mwidth);

//    int thresh = int(this->lower_threshold(im.Height())*_sensitivity); // better threshold?

//    // find peaks, set output values.
//    lowout->numpks = findpeaks (cors, image, cwidth, im.Width(), thresh, lowout);

//    iter.move(-iter.currposx(), -iter.currposy());
//    memcpy(lowout->imval, &(*iter), im.Width() * sizeof(T));

//    delete [] sums;
//    delete [] cors;
//  }

#define _MAN_INST_EDGE_(_EDGE_TYPE_) \
template class _EDGE_TYPE_<int>; \
template class _EDGE_TYPE_<float>; \
template class _EDGE_TYPE_<double>;

_MAN_INST_EDGE_(XVMaxEdge);
_MAN_INST_EDGE_(XVEdge);
_MAN_INST_EDGE_(XVGeneralEdge);
_MAN_INST_EDGE_(XVShortEdge);
