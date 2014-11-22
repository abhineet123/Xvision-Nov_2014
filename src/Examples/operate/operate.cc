#include <iostream>
#include "XVImageScalar.h"

using namespace std ;

int main() {
 
int n=4, i=0;

XVImageScalar<float> image(n,n), target(n,n);
float a=1, b=2, c=3, d=4;

 cout<<"Initializing image to "<<a<<':'<<endl;
 image = a;
 cout<<image;
 
 cout<<"Initializing target to image:"<<endl;
 target = image;
 cout<<target;
 
 cout<<"Calculating sum: ";
 a = image.sum();
 cout<<a<<endl; 
 
 cout<<"Calculating mean: ";
 a = image.mean();
 cout<<a<<endl;

 cout<<"Adding "<<b<<" to image:"<<endl;
 target = image + b;
 cout<<target;

 cout<<"Multiplying image by "<<d<<':'<<endl;
 target = image * d;
 cout<<target;

 cout<<"Substracting "<<c<<" from image:"<<endl;
 target = image - c;
 cout<<target;

 cout<<"Dividing image by "<<b<<':'<<endl;
 target = image / b;
 cout<<target;

 cout<<"Adding "<<b<<" to image:"<<endl;
 image += b;
 cout<<image;

 cout<<"Multiplying image by "<<d<<':'<<endl;
 image *= d;
 cout<<image;

 cout<<"Substracting "<<c<<" from image:"<<endl;
 image -= c;
 cout<<image;

 cout<<"Dividing image by "<<b<<':'<<endl;
 image /= b;
 cout<<image;
 
 cout<<"Negative of image:"<<endl;
 target = -image;
 cout<<target;

 cout<<"Substracting target from image:"<<endl;
 target = image - target;
 cout<<target;

 cout<<"Adding target to image:"<<endl;
 target = image + target;
 cout<<target;

 cout<<"Multiplying target by image:"<<endl;
 target = image * target;
 cout<<target;

 cout<<"Dividing target by image:"<<endl;
 target = target / image;
 cout<<target;

 cout<<"Substracting target from image:"<<endl;
 image -= target;
 cout<<image;

 cout<<"Adding target to image:"<<endl;
 image += target;
 cout<<image;

 cout<<"Multiplying target by image:"<<endl;
 image *= target;
 cout<<image;

 cout<<"Dividing target by image:"<<endl;
 target /= image;
 cout<<target;


return(0);
}







