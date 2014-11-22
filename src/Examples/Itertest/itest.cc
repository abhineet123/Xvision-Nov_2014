
#include <XVImageScalar.h>
#include <iostream.h>

int main() {
  XVImageScalar<int> img(6,3);
  XVImageGeneric xvgt(4,1,1,1);
  assert(xvgt <= img);
  XVImageWIterator<int> imgitert (img,5,2);
  XVImageWIterator<int> imgitertx (img,xvgt);
  cerr<<"Have "<<img.get_num_locks()<<" Locks"<<endl;
  XVImageWIterator<int> imgiter (img);
  cerr<<"Have "<<img.get_num_locks()<<" Locks"<<endl;
  XVImageIterator<int> imgiterw (img);

  while(imgiter.end() == false) {
    cerr<<imgiter.currposx()<<", "<<imgiter.currposy()<<endl;
    *imgiter = imgiter.currposx() + imgiter.currposy();
    cerr<<"Wrote: "<<*imgiter<<endl;
    ++imgiter;
  }
  
  cout<<img;

  while(imgiterw < imgiter) {
    cerr<<imgiterw.currposx()<<", "<<imgiterw.currposy()<<": ";
    cerr<<*imgiterw<<endl;
    ++imgiterw;
  }
  
  for (;imgitert.end() == false; ++imgitert) {
    cerr<<imgitert.currposx()<<", "<<imgitert.currposy()<<": ";
    *imgitert = 1;
    cerr<<"Wrote: "<<*imgitert<<endl;
  }

  cout<<img;
  
  for (;imgitertx.end() == false; ++imgitertx) {
    cerr<<imgitertx.currposx()<<", "<<imgitertx.currposy()<<": ";
    *imgitertx = 8;
    cerr<<"Wrote: "<<*imgitertx<<endl;
  }

  cout<<endl<<img;

  cout<<"Building a 5 by 5 image of a ring of 5s then 4s then 3s"<<endl;
  
  XVSize xs(5,5);
  XVPosition xp(0,0);
  XVImageGeneric xvgs(xs,xp);
  XVImageBase<int> xib(xvgs);
  
  for (;
       xvgs.Width() > 0; 
       xvgs.resize(xs), xvgs.reposition(xp))
			 
    {
      XVImageWIterator<int> xibiter(xib,xvgs);
	for(;
	  xibiter.end() == false; ++xibiter)
	{
	  *xibiter = (5-(xvgs.PosX()));
	}
	
	XVPosition xvpx(0,xvgs.PosY());
      xibiter.move(xvpx);
      cerr<<"At "<<xibiter.currposx()<<", "<<xibiter.currposy();
      cerr<<": Value that lives here: "<<*xibiter<<endl;
      xs.resize(xs.Width()-2,xs.Height()-2);
      xp.reposition(xp.PosX()+1, xp.PosY()+1);
    }

  cout<<xib;

  return 0;
}



