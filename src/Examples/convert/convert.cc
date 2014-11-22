#include <iostream>
#include "XVImageRGB.h"

using namespace std ;

template <class T> print(const T p){

  cout << p << "  " << flush;
};

template <class T> print(const XV_RGB15 p){

  cout << p.R() << ", " << p.G() << ", " << p.B() << "  " << flush;
};

template <class T> print(const XV_RGB16 p){

  cout << p.R() << ", " << p.G() << ", " << p.B() << "  " << flush;
};

template <class T> print(const XV_RGB24 p){

  cout << p.R() << ", " << p.G() << ", " << p.B() << "  " << flush;
};

template <class T> print(const XV_RGBA32 p){

  cout << p.R() << ", " << p.G() << ", " << p.B() << "  " << flush;
};


template <class T> print(const XVImageBase<T> & im){

  XVImageIterator<T> iter(im);
  for(; !iter.end(); ++iter){

    if(iter.currposx() == 0) cout << endl;
    print(*iter);
  }
};

int main() {

int n=4, i=0;

XVImageRGB<XV_RGB16> rgb16(n,n);
XVImageRGB<XV_RGB24> rgb24(n,n);
XVImageRGB<XV_RGBA32>rgb32(n,n);
XVImageScalar<float> scalar4(n,n);
XVImageScalar<int>   scalar2(n,n);
XVImageScalar<double>scalar8(n,n);

XVImageWIterator<XV_RGB16> iter16(rgb16);
XVImageWIterator<XV_RGB24> iter24(rgb24);
XVImageWIterator<XV_RGBA32>iter32(rgb32);
XVImageWIterator<float>    iter4(scalar4);
XVImageWIterator<int>      iter2(scalar2);
XVImageWIterator<double>   iter8(scalar8);

cout<<"Initializing scalar images:"<<endl;
while (iter2.end() == false) *iter8++ = *iter4++ = *iter2++ = 1;

cout<<"Initializing rgb images:"<<endl;
while (iter16.end() == false) {
  (*iter16).r = (*iter24).r = (*iter32).r = i;
  (*iter16).g = (*iter24).g = (*iter32).g = i+1;
  (*iter16++).b = (*iter24++).b = (*iter32++).b = (i++)+3;
} 

cout<<"Conversion from RGB16 to integer:"<<endl;
cout<<"In scalar image before:"<<endl;
print(scalar2);
cout<<"In rgb image:"<<endl;
print(rgb16);
RGBtoScalar (rgb16, scalar2);
cout<<"In scalar image after:"<<endl;
print(scalar2);

cout<<"Conversion from RGB24 to float:"<<endl;
cout<<"In scalar image before:"<<endl;
print(scalar4);
cout<<"In rgb image:"<<endl;  
print(rgb24);
RGBtoScalar (rgb24, scalar4);
cout<<"In scalar image after:"<<endl;
print(scalar4);

cout<<"Conversion from RGB32 to double:"<<endl;
cout<<"In scalar image before:"<<endl;
print(scalar8);
cout<<"In rgb image:"<<endl;
print(rgb32);
RGBtoScalar (rgb32, scalar8);
cout<<"In scalar image after:"<<endl;
print(scalar8);

return(0);
}
