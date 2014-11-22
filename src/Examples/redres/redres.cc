#include "XVImageScalar.h"
#include <iostream>

using namespace std ;

void print(XVImageScalar<float> & im){

  XVImageIterator<float> iter(im);
  for(; !iter.end(); ++iter){
    if(iter.currposx() == 0) cout << endl << *iter << "  " << flush;
    else cout << *iter << "  " << flush;
  }
};

int main() {
	XVImageScalar<float> restest(6,6), restarg(3,3);
	int counter = 0;
	for (XVImageWIterator<float> resinit (restest);resinit.end()==false;) {
		for (int i = 0; i < restest.SizeX(); i++, ++resinit ) {
			*resinit = counter+i;
		}
		++counter;
	}
	
	print(restest);
	
	restest.reduce_resolution(2,2,restarg);
	
	print(restarg);
	
	return 0;
}	
