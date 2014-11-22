// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVGeometry.h>

#define _XV2VEC_FLOAT_TO_INT_(_FLOAT_TYPE_, _INT_TYPE_) \
XV2Vec<_FLOAT_TYPE_>::operator XV2Vec<_INT_TYPE_> () const { \
  return XV2Vec<_INT_TYPE_>((int)rint(this->posx), (int)rint(this->posy)); \
};

#ifdef NEVER
_XV2VEC_FLOAT_TO_INT_(float, u_char);
_XV2VEC_FLOAT_TO_INT_(float, char);
_XV2VEC_FLOAT_TO_INT_(float, u_short);
_XV2VEC_FLOAT_TO_INT_(float, short);
_XV2VEC_FLOAT_TO_INT_(float, u_int);
_XV2VEC_FLOAT_TO_INT_(float, int);

_XV2VEC_FLOAT_TO_INT_(double, u_char);
_XV2VEC_FLOAT_TO_INT_(double, char);
_XV2VEC_FLOAT_TO_INT_(double, u_short);
_XV2VEC_FLOAT_TO_INT_(double, short);
_XV2VEC_FLOAT_TO_INT_(double, u_int);
_XV2VEC_FLOAT_TO_INT_(double, int);
#endif

template class XV2Vec<u_char>;
template class XV2Vec<char>;
template class XV2Vec<u_short>;
template class XV2Vec<short>;
template class XV2Vec<u_int>;
template class XV2Vec<int>;
template class XV2Vec<long>;
template class XV2Vec<float>;
template class XV2Vec<double>;


XVLine::XVLine(XV2Vec<double> end1, XV2Vec<double> end2) {
  XV2Vec<double> diffVec = end2 - end1;
  XV2Vec<double> halfVec = diffVec / ((double)2);
  center = end1 + halfVec;
  length = diffVec.length();
  double primAngle = acos((XV2Vec<double>(0, -1) * diffVec) 
			  / length);
  if(diffVec.x() < 0) { angle = -primAngle; }
  else                { angle = primAngle;  }
};

#define _XVLINE_OP_EQ_(_OP_) \
void XVLine::operator _OP_ (XVLine l2) { \
  center _OP_ l2.center; \
  length _OP_ l2.length; \
  angle  _OP_ l2.angle; \
};

_XVLINE_OP_EQ_(+=);
_XVLINE_OP_EQ_(-=);

void XVLine::endpoints (XV2Vec<double> ends[2]) const {
  XVAffineMatrix rotMat(angle);
  XV2Vec<double> addon = rotMat * XV2Vec<double>(0, -length / 2);    
  ends[0] = center + addon;
  ends[1] = center - addon;
};

void XVLine::rotate(const XV2Vec<double> & centerOfRotation, 
		    double rotAngle) {
  
  XVAffineMatrix rotMat(rotAngle);
  this->center = (rotMat * (this->center - centerOfRotation)) 
    + centerOfRotation;
  double sinangle = sin(this->angle);
  double cosangle = cos(this->angle);
  double tsin = rotMat.a * sinangle + rotMat.c * cosangle;
  if(tsin < 0)
    this->angle = -acos(rotMat.a * cosangle 
		       - rotMat.c * sinangle);
  else
    this->angle = acos(rotMat.a * cosangle 
		       - rotMat.c * sinangle);
};
  
#define _XVLINE_OP_(_OP_) \
XVLine operator _OP_ (XVLine l1, XVLine l2) { \
\
  XVLine retl; \
  retl.center = l1.center _OP_ l2.center; \
  retl.angle  = l1.angle  _OP_ l2.angle; \
  retl.length = l1.length _OP_ l2.length; \
  return retl; \
};

_XVLINE_OP_(+);
_XVLINE_OP_(-);

XVAffineMatrix::XVAffineMatrix(double angle) {
  a = cos(angle);
  b = -sin(angle);
  c = sin(angle);
  d = cos(angle);
};

#include <XVMatrix.h>

XVAffineMatrix::XVAffineMatrix(const XVMatrix & m){
  a = m[0][0];
  b = m[0][1];
  c = m[1][0];
  d = m[1][1];
};

XVAffineMatrix::operator XVMatrix() {
  XVMatrix m(2, 2);  
  m[0][0] = a;  
  m[0][1] = b;
  m[1][0] = c;
  m[1][1] = d;
  return m;
};

double XVAffineMatrix::operator () (int v) {
  switch (v) { case 0: return a; case 1: return b;
  case 2: return c; case 3: default: return d; } 
};

double XVAffineMatrix::operator () (int r, int c) { 
  switch (r) { case 0: switch (c) { case 0: return a; case 1: default:return b;}
  case 1: default: switch (c) { case 0: return c; case 1: default: return d;} }
};

float XVAffineMatrix::det() { return (a * d - b * c); };

XVAffineMatrix XVAffineMatrix::i() {
  float det = this->det(); 
  XVAffineMatrix mat; 
  mat.a = d / det;
  mat.b = - b / det;
  mat.c = - c / det;
  mat.d = a / det;
  return mat;
};

XVAffineMatrix XVAffineMatrix::t(){
  XVAffineMatrix m;  
  m.a = this->a;  
  m.b = this->c;
  m.c = this->b;
  m.d = this->d;
  return m ;
};
