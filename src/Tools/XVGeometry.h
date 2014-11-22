// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVGEOMETRY_H_
#define _XVGEOMETRY_H_

#include <sys/types.h>
#include <math.h>

/** template class for a 2d vector */
template <class T>
class XV2Vec {

protected:

  T posx;
  T posy;

public:

  inline const T & PosX() const { return posx; }
  inline const T & PosY() const { return posy; }

  inline const T & x() const { return posx; }
  inline const T & y() const { return posy; }

  inline XV2Vec(const T px, const T py) {
    posx = px;
    posy = py;
  }

  inline XV2Vec(const XV2Vec & xvp) {
    posx = xvp.posx;
    posy = xvp.posy;
  }

  inline XV2Vec() {
    posx = 0;
    posy = 0;
  }

  inline void setX(T val) { posx = val; }
  inline void setY(T val) { posy = val; }

  inline virtual void reposition(const XV2Vec<T> & xvp) {
    posx = xvp.posx;
    posy = xvp.posy;
  }

  inline virtual void reposition(const T px, const T py) {
    posx = px;
    posy = py;
  }

  inline double length() const {

    return sqrt((double)posx * posx + posy * posy);
  };
 
  inline XV2Vec<T> & operator = (const XV2Vec<T> & xvp) {
    posx = xvp.posx;
    posy = xvp.posy;
    return *this;
  }

#define MAKE_XV2VEC_OP_XV2VEC(_OP_) \
  inline XV2Vec<T> & operator _OP_ (const XV2Vec<T> & p){ \
    posx _OP_ p.PosX();  posy _OP_ p.PosY(); return *this; \
  };

MAKE_XV2VEC_OP_XV2VEC(+=);
MAKE_XV2VEC_OP_XV2VEC(-=);

#define MAKE_XV2VEC_OP_VAL(_OP_) \
  inline XV2Vec<T> & operator _OP_  (T val){ \
    posx _OP_ val;  posy _OP_ val; return *this; \
  };

MAKE_XV2VEC_OP_VAL(+=);
MAKE_XV2VEC_OP_VAL(-=);
MAKE_XV2VEC_OP_VAL(*=);
MAKE_XV2VEC_OP_VAL(/=); 

  template <class T2>
  operator XV2Vec<T2> () const { return XV2Vec<T2>((T2)(this->posx), (T2)(this->posy)); }
 virtual ~XV2Vec(){};
};


#define MAKE_COMPARE_OP_XVPOS_DIFF(OP) \
template <class T1, class T2>\
inline bool operator OP (const XV2Vec<T1> & p1, const XV2Vec<T2> & p2) { \
  return (p1.PosX() OP p2.PosX()) && (p1.PosY() OP p2.PosY()); \
}; 

MAKE_COMPARE_OP_XVPOS_DIFF(==);
MAKE_COMPARE_OP_XVPOS_DIFF(!=);
MAKE_COMPARE_OP_XVPOS_DIFF(<=);
MAKE_COMPARE_OP_XVPOS_DIFF(>=);
MAKE_COMPARE_OP_XVPOS_DIFF(<);
MAKE_COMPARE_OP_XVPOS_DIFF(>);

#define MAKE_COMPARE_OP_XVPOS(OP) \
template <class T>\
inline bool operator OP (const XV2Vec<T> & p1, const XV2Vec<T> & p2) { \
  return (p1.PosX() OP p2.PosX()) && (p1.PosY() OP p2.PosY()); \
}; 

MAKE_COMPARE_OP_XVPOS(==);
MAKE_COMPARE_OP_XVPOS(!=);
MAKE_COMPARE_OP_XVPOS(<=);
MAKE_COMPARE_OP_XVPOS(>=);
MAKE_COMPARE_OP_XVPOS(<);
MAKE_COMPARE_OP_XVPOS(>);

template <class T>
inline XV2Vec<T> operator - (const XV2Vec<T> & p){ return XV2Vec<T>(- p.PosX(), -p.PosY()); }

/**
 * The binary operators for XV2Vec
 */

#define MAKE_XVPOS_OP_XVPOS(OP) \
template <class T1, class T2>\
inline XV2Vec<T1> operator OP (const XV2Vec<T1> & p1, const XV2Vec<T2> & p2) { \
  return XV2Vec<T1>((T1)(p1.PosX() OP p2.PosX()), (T1)(p1.PosY() OP p2.PosY())); \
};

MAKE_XVPOS_OP_XVPOS(+);
MAKE_XVPOS_OP_XVPOS(-);

/**
 * The binary operators XV2Vec and int
 */

#define MAKE_XVPOS_OP_VAL(OP) \
template <class T>\
inline XV2Vec<T> operator OP (const XV2Vec<T> & p1,  T p2) { \
  return XV2Vec<T>((T)(p1.PosX() OP p2), (T)(p1.PosY() OP p2)); \
}; \
\
template <class T>\
inline XV2Vec<T> operator OP (T p2, const XV2Vec<T> & p1) { \
  return XV2Vec<T>((T)(p2 OP p1.PosX()), (T)(p2 OP p1.PosY())); \
}; 

MAKE_XVPOS_OP_VAL(+);
MAKE_XVPOS_OP_VAL(-);
MAKE_XVPOS_OP_VAL(*);
MAKE_XVPOS_OP_VAL(/);

template <class T>
inline XV2Vec<int> round(const XV2Vec<T> &x) {
  return XV2Vec<int>(round(x.x()),round(x.y()));
}

template <class T>
inline XV2Vec<T> rint(const XV2Vec<T> &x) {
  return XV2Vec<T>(rint(x.x()),rint(x.y()));
}

// dot product
template <class T>
T operator * (const XV2Vec<T> & t1, const XV2Vec<T> & t2){
  return t1.x() * t2.x() + t1.y() * t2.y();
};

template <class T>
inline XV2Vec<T> mini(const XV2Vec<T> * arr, const int size){

  XV2Vec<T> minres = arr[0];
  for(int i = 1; i < size; ++i){
    if(minres.x() > arr[i].x()) minres.setX(arr[i].x());
    if(minres.y() > arr[i].y()) minres.setY(arr[i].y());
  }
  return minres;
};

template <class T>
inline XV2Vec<T> maxi(const XV2Vec<T> * arr, const int size){

  XV2Vec<T> maxres = arr[0];
  for(int i = 1; i < size; ++i){
    if(maxres.x() < arr[i].x()) maxres.setX(arr[i].x());
    if(maxres.y() < arr[i].y()) maxres.setY(arr[i].y());
  }
  return maxres;
};


typedef XV2Vec<double> XVPoint;

/** line class for XVision */
class XVLine {
  
 public:

  XV2Vec<double> center;

  double length;
  double angle;
  
  XVLine() : center(0, 0), length(0), angle(0) {}
  XVLine(XV2Vec<double> end1, XV2Vec<double> end2);

  void operator += (XVLine);
  void operator -= (XVLine);
  
  void endpoints (XV2Vec<double> ends[2]) const;
  void rotate(const XV2Vec<double> &, double);
};

XVLine operator + (XVLine, XVLine);
XVLine operator - (XVLine, XVLine);

inline XV2Vec<double> intersect(const XVLine &, const XVLine &);

class XVMatrix;

/**
 * XVAffineMatrix
 *
 * @author Sam Lang
 * @version $Id: XVGeometry.h,v 1.1.1.1 2008/01/30 18:44:09 burschka Exp $
 *
 * An 2 x 2 affine matrix that allows for fast computation
 * of rotation, scale, and sheer.  The inverse is computed
 * using the adjoint.
 */
class XVAffineMatrix {

 public:

  float a;
  float b;
  float c;
  float d;

  XVAffineMatrix(float a0, float b0, float c0, float d0) 
    : a(a0), b(b0), c(c0), d(d0) {}
  XVAffineMatrix() : a(0),b(0),c(0),d(0) {}
  XVAffineMatrix(double angle);
  XVAffineMatrix(double sx, double sy) {
    a = sx;  d = sy; b = c = 0;
  };
  
  XVAffineMatrix(double sh, double d1, double d2){
    a = d = 1;  c = 0; b = sh;
  };

  XVAffineMatrix(const XVMatrix &);

  operator XVMatrix();
  double operator () (int v);
  double operator () (int r, int c);
  float  det();
  XVAffineMatrix i();
  XVAffineMatrix t();
};

inline XVAffineMatrix operator * (const XVAffineMatrix & m1, const 
				  XVAffineMatrix & m2) 
{
  XVAffineMatrix res;  
  res.a = m1.a * m2.a + m1.b * m2.c;
  res.b = m1.a * m2.b + m1.b * m2.d;
  res.c = m1.c * m2.a + m1.d * m2.c;
  res.d = m1.c * m2.b + m1.d * m2.d;
  return res;
};

typedef XV2Vec<float> XVCoord2D;

template <class T>
XV2Vec<T> operator * (XVAffineMatrix mat, XV2Vec<T> vec) {
  return XV2Vec<T>(mat.a * vec.x() + mat.b * vec.y(),
		   mat.c * vec.x() + mat.d * vec.y());
};

inline XV2Vec<double> intersect(const XVLine & lA, const XVLine & lB) {

  XV2Vec<double> endsA[2];
  lA.endpoints(endsA);

  XV2Vec<double> endsB[2];
  lB.endpoints(endsB);

  double lambda = 
    (((endsB[1].x() - endsB[0].x()) * 
      (endsA[0].y() - endsB[0].y()))
     - ((endsB[1].y() - endsB[0].y()) * 
	(endsA[0].x() - endsB[0].x()))) / 
    (((endsB[1].y() - endsB[0].y()) * 
      (endsA[1].x() - endsA[0].x()))
     - ((endsB[1].x() - endsB[0].x()) * 
	(endsA[1].y() - endsA[0].y())));
  
  return endsA[0] + (lambda * (endsA[1] - endsA[0]));
};

#endif
