#include <iostream>
#include "XVImageRGB.h"

using namespace std ;

int main() {

int n=4, i=0;
XVImageRGB<XV_RGB16> rgb(n,n);
XVImageScalar<float> scalar(n,n);
XVImageIterator<XV_RGB16> iter1(rgb);
XVImageIterator<float> iter2(scalar);

cout<<"Initializing scalar image:"<<endl;
while (iter2.end() == false) *iter2++ = 1;
scalar.print(cout);

cout<<"Initializing rgb image:"<<endl;
while (iter1.end() == false)
  (*iter1++).assign(i%2,i%3,(i++)%5);
rgb.print(cout);

cout<<"Doing the conversion."<<endl;
convertRGB (rgb, scalar);
//convertRGB (x, y);

cout<<"In rgb:"<<endl;
rgb.print (cout);
cout<<"In scalar:"<<endl;
scalar.print (cout);

return(0);
}
