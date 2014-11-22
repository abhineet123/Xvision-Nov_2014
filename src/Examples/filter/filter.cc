#include <iostream>
#include "XVImageScalar.h"

using namespace std ;

main() {

int n=15, i, j, width_y = 5, width_x = 3;
float norm_x = 1, norm_y = 1;
XVImageScalar<float> x(n,n), y(n,n), z(n,n), w(n,n), v(n,n);
cout<<"Images created. ";
cout<<"Width "<<x.Width()<<" x "<<x.Height()<<endl;
//for(int uu=0;uu<10000;uu++) {

XVImageWIterator<float> xiter(x);
cout<<"Iterator created"<<endl;
int counter=0;
for (i=0;i<n;i++) 
   for (counter=0;counter<n;counter++) {
     *xiter=(i+counter);
     ++xiter;
   }
cout<<"Image x initialized"<<endl;

cout<<"In image x:"<<endl;
//x.print(cout);

y = Box_Filter_y (x, width_y);

cout<<"In image after Box_Filter_y:"<<endl;
//y.print(cout);

z = Box_Filter_x (x, width_x, norm_x);

cout<<"In image after Box_Filter_x:"<<endl;
//z.print(cout);

w = Box_Filter_x (y, width_x);

cout<<"In image after both filters: y then x"<<endl;
//w.print(cout);

v = Box_Filter_y (z, width_y, norm_y);

cout<<"In image after both filters: x then y"<<endl;
//v.print(cout);

//}
}
