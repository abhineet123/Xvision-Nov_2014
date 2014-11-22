#include <iostream>
#include "XVImageScalar.h"
#include "GeoTransform.h"
#include "Video.h"

using namespace std ;

int main() {

int n=4, i, j;
XVImageScalar<float> x(n,n), y(n,n);
cout<<"Images created. ";
cout<<"Width "<<x.Width()<<" x "<<x.Height()<<endl;

XVImageIterator<float> xiter(x), yiter(y);
cout<<"Iterators created"<<endl;
int counter=0;
for (i=0;i<n;i++) 
   for (counter=0;counter<n;counter++) {
     *xiter=(i+counter);
     *yiter=(i+counter);
     ++xiter;
     ++yiter;
   }
cout<<"Images x,y initialized"<<endl;

cout<<"In image x:"<<endl;
x.print(cout);
cout<<"In image y:"<<endl;
y.print(cout);

GeoTrans<float> rotate(0.2);   //theta in radians
GeoTrans<float> scale(3, 1); 
cout<<"GeoTransformations rotate and scale created"<<endl;

cout<<"In rotate matrix:"<<endl;
rotate.print();
cout<<"In scale matrix:"<<endl;
scale.print();

XVSize_HW rsize, ssize;
rsize = rotate.sizeNeeded (x);
ssize = scale.sizeNeeded (y);
cout<<"Size needed for rotation: "<<rsize.width
    <<" x "<<rsize.height<<endl;
cout<<"Size needed for scaling: "<<ssize.width
    <<" x "<<ssize.height<<endl;
XVImageScalar<float> z(rsize.width, rsize.height), 
                     v(ssize.width, ssize.height);

coordinate coord1, coord2;
coord1.x = coord1.y = 0;
coord2 = rotate.transform (coord1);
cout<<"Transformed rotate coordinates (0,0): ("<<coord2.x
    <<','<<coord2.y<<")\n";
coord2 = scale.transform (coord1);
cout<<"Transformed scale coordinates (0,0): ("<<coord2.x
    <<','<<coord2.y<<")\n";

z = rotate.warp(x);
v = scale.warp(y);
cout<<"Image after rotating:"<<endl;
z.print(cout);
cout<<"Image after scaling:"<<endl;
v.print(cout);
}
