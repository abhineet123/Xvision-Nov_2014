#include "Gesture.h"

template <class T, class T1>
Gesture<T,T1>::Gesture(const XVImage<T> &target_in)
{
  int i,j;
  XVImage<T> Dx,Dy;

  // Careful of type here ... we're loosing precision

  
  target = Box_Filter(target_in - target_in.mean(),3,3);

  forward_model.resize(target.Width()*target.Height(),0);

  Dx = Box_Filter_y(Prewitt_x(target_in,(T)0),3,(T)6);
  Dy = Box_Filter_x(Prewitt_y(target_in,(T)0),3,(T)6);

  // Now, set up the photometric stage;

  forward_model = add_column(forward_model,target);

  forward_model = add_column(forward_model,Dx);
  forward_model = add_column(forward_model,Dy);

  {
    XVImage<double> temp(Dx.Height(),Dx.Width());

    // Generate the info needed to compute scale

    double midx = (Dx.Width()-1)/2.0;
    double midy = (Dx.Height()-1)/2.0;

    for (i=0;i<Dx.Width();i++)
      for (j=0;j<Dx.Height();j++)
	temp[j][i] = Dx[j][i]*(j-midy) - 
	             Dy[j][i]*(i-midx); 

    forward_model = add_column(forward_model,temp);

  }

  // Now, set up scale

  {

    XVImage<double> temp(Dx.Height(),Dx.Width());

    // Generate the info needed to compute scale

    double midx = (Dx.Width()-1)/2.0;
    double midy = (Dx.Height()-1)/2.0;

    for (i=0;i<Dx.Width();i++)
      for (j=0;j<Dx.Height();j++)
	temp[j][i] = Dx[j][i]*(i-midx) + 
	             Dy[j][i]*(j-midy); 

    forward_model = add_column(forward_model,temp);
  }

  inverse_model = (((forward_model.t()*forward_model).i())*forward_model.t());
    
  }
  

template <class T, class T1>
TrackerOutput Gesture<T,T1>::Match_value(const XVImage<T1> &live_image)
{
  ColVector res(5);
  TrackerOutput y;

  x << Box_Filter(live_image-live_image.mean(),3,3);

  res =  inverse_model * x;

  y.values = res;//.Rows(2,5);
  y.objective_value_per_pixel =
    (x - (forward_model * res)).ip()/x.n_of_rows();

  return y;
		       
							
}
//template <class T, class T1>
//int Gesture<T,T1>::ComputeMatch(T1 contrast,const ColVector &differences)
//{ return 1;}

template class Gesture<double,double>;

