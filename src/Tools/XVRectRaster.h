# ifndef _XVRECTRASTER_H_
# define _XVRECTRASTER_H_

/*
  This class rasterizes a parallelogram given three of its vertices and 
  the numbers of points along the direction of both of two nearby edges: 
  n_h points are taken along the edge between (x_0,y_0) and (x_h,y_h);
  n_v points are taken along the edge between (x_0,y_0) and (x_v,y_v).
  So totally n_h * n_v points are taken in the parallelogram.
  Note that the edge between (x_0,y_0) and (x_h,y_h) and that between
  (x_0,y_0) and (x_v,y_v) are considered ``inside'' the parallelogram, 
  while the other two edges are considered ``outside''.
*/

class XVRectRaster {
 private:
  int x0, y0, xh, yh, xv, yv, h, v ;
  int dxh, dyh, dxv, dyv, fdxh, fdyh, fdxv, fdyv, s, s2 ;
  int i, j, x, y, fx, fy, rx, ry, frx, fry ;
 public:
  XVRectRaster( int x_0, int y_0, int x_h, int y_h, 
		int x_v, int y_v, int n_h, int n_v ) : 
   x0(x_0), y0(y_0), xh(x_h), yh(y_h), xv(x_v), yv(y_v), h(n_h), v(n_v),
   dxh((xh-x0)/h), dyh((yh-y0)/h), dxv((xv-x0)/v), dyv((yv-y0)/v), 
   fdxh((xh-x0)%h*v*2), fdyh((yh-y0)%h*v*2), fdxv((xv-x0)%v*h*2), 
   fdyv((yv-y0)%v*h*2), s(h*v), s2(s*2), 
   i(0), j(0), x(x0), y(y0), fx(0), fy(0), rx(x0), ry(y0), frx(0), fry(0) {}

  // return false when there is no more point to iterate, true otherwise

  bool next() {
    if( ++i >= h ) {
      return nextLine() ;
    }else {
      x += dxh ;
      fx += fdxh ;
      if( fx > s ) {
        fx -= s2 ;
        x ++ ;
      }else if( fx <= -s ) {
        fx += s2 ;
        x -- ;
      }
      y += dyh ;
      fy += fdyh ;
      if( fy > s ) {
        fy -= s2 ;
        y ++ ;
      }else if( fy <= -s ) {
        fy += s2 ;
        y -- ;
      }
      return true ;
    }
  }

  // move to the head of next scan line

  bool nextLine() {
    i = 0 ;
    if( ++j >= v ) {
      j = 0 ;
      x = rx = x0 ;
      y = ry = y0 ;
      fx = fy = frx = fry = 0 ;
      return false ;
    }else {
      rx += dxv ;
      frx += fdxv ;
      if( frx > s ) {
	frx -= s2 ;
	rx ++ ;
      }else if( frx <= -s ) {
	frx += s2 ;
	rx -- ;
      }
      ry += dyv ;
      fry += fdyv ;
      if( fry > s ) {
	fry -= s2 ;
	ry ++ ;
      }else if( fry <= -s ) {
	fry += s2 ;
	ry -- ;
      }
      x = rx ;
      y = ry ;
      fx = frx ;
      fy = fry ;
      return true ;
    }
  }

  int nearestX() const { return x ; }
  int nearestY() const { return y ; }

  // p1 and p2 are integers that reprensent the relative weight of two
  // coordinates given. the real x coordinate is (x1*p1+x2*p2)/(p1+p2)

  void nearX( int& x1, int& p1, int& x2, int& p2 ) const {
    if( fx >= 0 ) {
      x1 = x ;
      p1 = s2 - fx ;
      x2 = x + 1 ;
      p2 = fx ;
    }else {
      x1 = x - 1 ;
      p1 = s2 + fx ;
      x2 = x ;
      p2 = - fx ;
    }
  }
  void nearY( int& y1, int& p1, int& y2, int& p2 ) const {
    if( fy >= 0 ) {
      y1 = y ;
      p1 = s2 - fy ;
      y2 = y + 1 ;
      p2 = fy ;
    }else {
      y1 = y - 1 ;
      p1 = s2 + fy ;
      y2 = y ;
      p2 = - fy ;
    }
  }
};

# include <XVImageBase.h>
# include <string.h>

/*
 Transform parallelogram abcd on source image to the whole target image.
 a -> (0,0)
 b -> (w,0)
 c -> (w,h)
 d -> (0,h)
*/

template<class T>
void intAffineWarp( const XVImageBase<T>& source, XVImageBase<T>& target,
		    XVPosition a, XVPosition b, XVPosition d ) {
  if( target.Width() == 0 || target.Height() == 0 ) {
    return ;
  }
  T zero ;
  memset( &zero, 0, sizeof(zero) );
  XVRectRaster r( a.PosX(), a.PosY(), b.PosX(), b.PosY(), d.PosX(), d.PosY(),
		  target.Width(), target.Height() );
  XVImageWIterator<T> p(target);
  int w = source.Width(), h = source.Height() ;
  int x, y;
  XVPosition c = b - a + d ;
  if( source.contains(a) && source.contains(b) &&
      source.contains(c) && source.contains(d)) {
    do {
      *p = source[ r.nearestY() ][ r.nearestX() ];
      ++ p ;
    }while( r.next() );
  }else {
    do {
      x = r.nearestX() ;
      y = r.nearestY() ;
      if( x >= 0 && x < w && y >= 0 && y < h ) {
	*p = source[y][x] ;
      }else {
	*p = zero ;
      }
      ++ p ;
    }while( r.next() );
  }
};

/*
 center is the point on the source image which will be the center 
 of the target image. 
 theta is counter-clockwise from source to target.
 scale is source to target
 sheer is positive for left-sheer, negative for right-sheer
*/

template<class T>
void intAffineWarp( const XVImageBase<T>& source, XVImageBase<T>& target,
		    XVPosition center, double theta = 0.0,
		    double scaleX = 1.0, double scaleY = 1.0,
		    double sheer = 0.0 ) {
  double cx = center.PosX() ;
  double cy = center.PosY() ;
  double w = target.Width() / scaleX / 2.0 ;
  double h = target.Height() / scaleY / 2.0 ;
  double cos_theta = cos( theta );
  double sin_theta = sin( theta );
  double wc = w * cos_theta ;
  double ws = w * sin_theta ;
  double hc = h * cos_theta ;
  double hs = h * sin_theta ;
  double wss = ws * sheer ;
  double hcs = hc * sheer ;
  int ax = (int) round( cx - wc + hs + wss + hcs);
  int ay = (int) round( cy - ws - hc );
  int bx = (int) round( cx + wc + hs - wss + hcs );
  int by = (int) round( cy + ws - hc );
  int dx = (int) round( cx - wc - hs + wss - hcs );
  int dy = (int) round( cy - ws + hc );
  intAffineWarp( source, target, XVPosition( ax, ay ),
		 XVPosition( bx, by ), XVPosition( dx, dy ) );
}

# endif // _XVRECTRASTER_H_

