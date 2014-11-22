// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <XVAffineWarp.h>

template <class T>
XVAffineWarp<T>::XVAffineWarp (float theta) {
  amatrix.a = amatrix.d = cos (theta);
  amatrix.b = -sin (theta);
  amatrix.c = sin (theta);
  ratio = amatrix.det();
}

template <class T>
XVAffineWarp<T>::XVAffineWarp (float x, float y) {
  amatrix.a = x;
  amatrix.d = y;
  amatrix.b = amatrix.c = 0;
  ratio = amatrix.det();
}

template <class T>
XVAffineWarp<T>::XVAffineWarp (float sheer, float dum1, float dum2) {
  amatrix.a = amatrix.d = 1;
  amatrix.c = 0;
  amatrix.b = sheer;
  ratio = amatrix.det();
}

template <class T>
XVAffineWarp<T>::XVAffineWarp (float theta, float x, float y, float sheer) {
  XVAffineMatrix thetaM(theta);
  XVAffineMatrix scaleM(x, y);
  XVAffineMatrix sheerM(sheer, 0, 0);
  amatrix = thetaM * scaleM * sheerM;
  ratio = amatrix.det();
};

template <class T>
XVSize XVAffineWarp<T>::sizeNeeded (const XVImageBase<T> & src) {
  return XVSize((int)((fabs(amatrix.a))*src.Width() + (fabs(amatrix.b))*src.Height()),
		(int)((fabs(amatrix.c))*src.Width() + (fabs(amatrix.d))*src.Height()));
}

template <class T>
void XVAffineWarp<T>::findBounds(const XVCoord2D & targCenter,
				 const XVSize & targSize,
				 XVCoord2D & ulc,
				 XVCoord2D & urc,
				 XVCoord2D & lrc,
				 XVCoord2D & llc) {
  
  XVCoord2D halfSize(targSize.Width() / 2, targSize.Height() / 2);
  XVCoord2D corners[4];
  corners[0] = amatrix * (-halfSize) + targCenter;
  corners[1] = amatrix * (XVCoord2D(halfSize.x(), -halfSize.y())) + targCenter;
  corners[2] = amatrix * (XVCoord2D(-halfSize.x(), halfSize.y())) + targCenter;
  corners[3] = amatrix * halfSize + targCenter;

  ulc = mini(corners, 4);
  lrc = maxi(corners, 4);
  urc = XVCoord2D(lrc.x(), ulc.y());
  llc = XVCoord2D(ulc.x(), lrc.y());
};

template <class T>
XVCoord2D XVAffineWarp<T>::transform (XVCoord2D coord1) {
  return XVCoord2D(floor((amatrix.d * coord1.x() - amatrix.b * coord1.y())/
			 (amatrix.a * amatrix.d - amatrix.b * amatrix.c)),
		   floor((amatrix.a * coord1.y() - amatrix.c * coord1.x())/
			 (amatrix.a * amatrix.d - amatrix.b * amatrix.c)));
};

template <class T>
XVImageBase<T> & XVAffineWarp<T>::reverseWarp(const XVImageBase<T> & src, XVImageBase<T> & targ,
					      const XVCoord2D & targCenter){

  return warp(src, targ, targCenter, FORWARD);
};

// this warp assumes the size of targ is already set!
template <class T> 
XVImageBase<T> & XVAffineWarp<T>::warp (const XVImageBase<T> & src, 
					XVImageBase<T> & targ,
					const XVCoord2D & targCenter,
					int DIREC){
  
  XVPosition current;
  XVCoord2D warped;

  if(DIREC == INVERSE){
    invertMatrix();
  }

  XVCoord2D halfSize(targ.Width() / 2, targ.Height() / 2);
  XVCoord2D ulc, urc, lrc, llc;

  findBounds(targCenter, (XVSize)targ, ulc, urc, lrc, llc);

  ulc.setX( round(ulc.PosX()) );
  ulc.setY( round(ulc.PosY()) );
  lrc.setX( round(lrc.PosX()) );
  lrc.setY( round(lrc.PosY()) );


  if((((XVPosition)src) + ulc) >= XVCoord2D(0, 0) && 
     (((XVPosition)src) + lrc) < XVCoord2D(src.Width(), src.Height())) {
    
    XVRowWIterator<T> riter(targ, 0);
    for (int r = 0; r < targ.Height(); ++r) {
      
      riter.reset(r);      
      float normy = riter.currposy() - halfSize.y();      
      warped = amatrix * XVCoord2D(-halfSize.x(), normy);
      
      for(; !riter.end(); ++riter){
	      current = round(warped + targCenter);
       
	      *riter = src[current.PosY()][current.PosX()];	
	      warped += XVCoord2D(amatrix.a, amatrix.c);
      }
    }
  }else{

    XVRowWIterator<T> riter(targ, 0);
    for (int r = 0; r < targ.Height(); ++r) {
      
      riter.reset(r);
      float normy = riter.currposy() - halfSize.y();
      warped = amatrix * XVCoord2D(-halfSize.x(), normy);
      
      for(; !riter.end(); ++riter){
	      current = round(warped + targCenter);
	      if(current >= XVPosition(0, 0) && 
           current < XVPosition(src.Width(), src.Height())){
	        *riter = src[current.PosY()][current.PosX()];
	      }else {
	        memset(&(*riter), 0, sizeof(*riter));
	      }

	      warped += XVCoord2D(amatrix.a, amatrix.c);
      }
    }
  } 
   
  return targ;
};

template <class T>
inline void XVAffineWarp<T>::invertMatrix(){
  
  // compute inverse using Adjoint
  amatrix = amatrix.i();
};

template <class T>
void XVAffineWarp<T>::print(){
  
  cout << amatrix.a << "  " << amatrix.b << endl
       << amatrix.c << "  " << amatrix.d << endl;
}

template class XVAffineWarp<u_char>;
template class XVAffineWarp<u_short>;
template class XVAffineWarp<u_int>;
template class XVAffineWarp<char>;
template class XVAffineWarp<short>;
template class XVAffineWarp<int>;
template class XVAffineWarp<float>;
template class XVAffineWarp<double>;

template class XVAffineWarp<XV_RGB15>;
template class XVAffineWarp<XV_RGB16>;
template class XVAffineWarp<XV_RGB24>;
template class XVAffineWarp<XV_RGBA32>;


