#include "XVImageScalar.h"
#include <iostream>

using namespace std ;

void print (float*, int);
void dimensions (XVImageScalar<float>);
XVImageScalar<float> half (XVImageScalar<float>, int);

main() {
  int n=9;
  XVImageScalar<float> x(n,n), y(n,n);
  cout<<"Creating two images "<<n<<"x"<<n<<endl;

  //for (i=0;i<n;i++)
  //  for (j=0;j<n;j++)
  //    foo[i*n+j] = (i);
 
  XVImageWIterator<float> xIter (x);
  int counter = 0;
  cout<<"Writing a gradient into image x"<<endl;
  while (xIter.end() == false) {
    for (int i = 0;i<x.SizeX();i++) {
      *xIter = counter;
      ++xIter;
    }
    cout<<"Assigning values in line number "<<counter<<endl;
    ++counter;
  }

  //  dimensions(x);
  cout<<"Printing contents of x after assignment"<<endl;
  //  x.print (cout);
  cout<<endl;
  //float *goo = x.data();
  //x.print (cout);

  n=n-2;
  cout<<"Setting a subimage in x"<<endl;
  x.set_subimage(1,1,n,n);
  //  float *goo = x.data();
  // x.print (cout);

  cout<<"Finding mean value: "<<x.mean()<<endl;
  cout<<"Substracting 1 from each element in x"<<endl;
  x-=1;
  cout<<"Dividing each element of x by 2"<<endl;
  x/=2;
  //  x.print(cout);

  /*
  for (i=0;i<n;i++) {
   for (j=0;j<n;j++) {
    float *z = x[j];
    *z=19;
    cout.width(3);
    cout<<*z;
   }  cout<<endl;
  }
  */
 
  // x.print(cout);
  
  cout<<"Reducing resolution in x by 2"<<endl;
  x.reduce_resolution (2,2,x);
  //  x.print (cout);
  
  /*
  int m = n/2+n%2;
  int k = m/2+m%2;
  XVImage<float> w(m,m);
  //w = half (x,n);
  //half (half ((half (x, n)),m), k);
  */
  return(0);
}

void print (float* array,int n) {
  int i,j;
  cout<<"\nContents of the image "<<n<<'x'<<n<<endl;
  for (i=0;i<n;i++) {
    for (j=0;j<n;j++) {
      cout.width(3);
      cout<<array[i*n+j]<<' ';
    }
    cout<<endl;
  }
  //for (i=0;i<n*n;i++) cout<<array[i]<<' ';
  cout<<endl;
}









