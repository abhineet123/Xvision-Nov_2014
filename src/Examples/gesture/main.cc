#include <math.h>
#include "Gesture.h"

#define TX 1
#define TY 1

main()
{
  TrackerOutput y;
  XVImage<u_short> inn(100,100);
  XVImage<double> target(100,100), reduced_target(25,25);
  int i,j;

  convert(inn,target);
  for (i=0;i<100;i++)
    for (j=0;j<100;j++)
      target[i][j] = sin(i/20.0)*sin((double) j/20.0);

  target.reduce_resolution(4,4,reduced_target);
    
  Gesture<double,double> x(reduced_target);

  for (i=0;i<100;i++)
    for (j=0;j<100;j++)
      target[i][j] = sin((i+TX)/20.0)*sin((j+TY)/20.0);

  target.reduce_resolution(4,4,reduced_target);
  
  y = x.Match_value(reduced_target);

  cout << y.values << endl;

  cout << y. objective_value_per_pixel << endl;

}
