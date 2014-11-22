// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**

  XVTools.h

  Globally useful definitions for Tools system.

  6 - 6 - 93
  Sidd Puri

  2.7.01  
  Sam Lang  -  Added modifications

*/

#ifndef _XVTOOLS_H_
#define _XVTOOLS_H_

#include <sys/types.h>
#include <math.h>

//-----------------------------------------------------------------------------
//  Global inline functions
//-----------------------------------------------------------------------------

template<class T> inline T min2 (T a, T b)
{ return a < b? a: b; }

template<class T> inline T max2 (T a, T b)
{ return a > b? a: b; }

template<class T> inline T absi (T x)
{ return x > 0? x: -x; }

#ifndef __SC__
template<class T> inline int signum (T x)
{ return x > 0? 1: -1; }

#else
#include <stdlib.h>
#define signum(x) (x >= 0)
#endif

inline bool odd (int n)
{ return (bool)(n & 1); }

template <class T>
inline int round (T x)
{ return signum(x)*int (absi(x) + .5); }

template <class T>
inline T rint (T x)
{ return (T)signum(x)*int (absi(x) + .5); }

inline float half (int l)
{ return float (l) / 2; }

inline float rad_deg (float r)
{ return r / M_PI * 180; }

inline float deg_rad (float d)
{ return d / 180 * M_PI; }

inline float radmod (float op, float div)
{ return absi(op - (((int)(op / div)) * div)); }

template<class T> inline T sqr (T x)
{ return x*x;}

template<class T> inline void copy (T *dest, T *src,int n)
{memcpy(dest,src,n*sizeof(T));}

inline void bitset(unsigned &bits, int n) {
  bits |= (0x1 << n);
}

//inline int isset(unsigned bits, int n) {
 // return ((bits & (0x1 << n)) != 0);
//}

inline void bitclear(unsigned &bits, int n) {
  bits &= ~(0x1 << n);
}

inline void bittoggle(unsigned &bits,int n, int flag) {
  if (flag)
    bitset(bits,n);
  else
    bitclear(bits,n);
}

template <class T>
inline T avg(const T * values, int num){

  T sum = 0;
  for(int i = 0; i < num; i++){

    sum += values[i];
  }
  return sum / num;
};

template <class T>
inline T maxi(const T * arr, const int size, T & max, int & index){

  max = arr[0];
  index = 0;
  for(int i = 1; i < size; i++){

    if(max < arr[i]){ index = i; max = arr[i]; }
  }
  return max;
};

template <class T>
inline T mini(const T * arr, const int size, T & min, int & index){

  min = arr[0];
  index = 0;
  for(int i = 1; i < size; i++){

    if(min > arr[i]){ index = i; min = arr[i]; }
  }
  return min;
};

#endif
