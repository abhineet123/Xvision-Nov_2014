#include "XVImageScalar.h"

print(XVImageScalar<float> & im){

  XVImageIterator<float> iter(im);
  for(; !iter.end(); ++iter){

    if(iter.currposx() == 0) cout << endl << *iter << "  " << flush;
    else cout << *iter << "  " << flush;
  }
};


main()

{
  XVImageScalar<float> x(5,5), y(5,5);

  XVImageWIterator<float> iter(x);
  for(; !iter.end(); ++iter){
    *iter = iter.currposx() + iter.currposy();
  }

  print(x);

  y = Box_Filter_y(x,3,(float)3);
  print(y);

  y = Box_Filter_x(x,3,(float)0);
  print(y);

  y = Box_Filter_x(Prewitt_y(x,(float)0),3,(float)6);
  print(y);

  y = Box_Filter_y(Prewitt_x(x,(float)0),3,(float)6);
  print(y);

  

}
