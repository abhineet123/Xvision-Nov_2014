// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 *
 * XVMatrix.cc
 *
 *  @author Chien-Ping Lu, Jonathan Wang, Greg Hager, Sam Lang
 */

#include <XVMatrix.h>
#include <XVTools.h>

void _panic(char *mess)
{
  cerr << mess << endl;
  exit(1);
}

XVMatrix::XVMatrix (const XVMatrix& m)
{
  init_empty();
  resize(m.rowNum,m.colNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = m.rowPtrs[i][j];
    }
  }
}

int
XVMatrix::resize(int nrows, int ncols)
{
  // If things are the same, do nothing and return

  if ((nrows == rowNum) && (ncols == colNum)) {return 1;}
  rowNum = nrows; colNum = ncols;

  if (dataShared())
    _panic((char*)"Cannot resize a matrix which has an outstanding submatrix");

  // Check data room
  if ( (nrows*ncols > csize) || (csize == 0) ) {
    dsize = nrows*ncols; csize = dsize;
    if ( data ) delete[] data;
    data = new FrReal[csize];
  }
  // Check column room
  if ( (nrows > trsize) || (trsize == 0) ) {
    trsize = nrows;
    if ( rowPtrs ) delete[] rowPtrs;
    rowPtrs = new FrReal*[trsize];
  }
  FrReal **t;
  t = rowPtrs;
  for (int i=0, j=0; j<trsize; i+=ncols, ++j) {
    rowPtrs[j] = data + i;
  }
  return 1;
}


XVMatrix&
XVMatrix::operator=(const XVMatrix &m)
{
  //CHECKSIZE(m,"Incompatible size in =");
  resize(m.rowNum, m.colNum); // now it's same with <<
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = m.rowPtrs[i][j];
    }
  }
  return *this;
}

XVMatrix&
XVMatrix::operator<<(const XVMatrix &m)
{
  resize(m.rowNum, m.colNum);

  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = m.rowPtrs[i][j];
    }
  }
  return *this;
}

XVMatrix& XVMatrix::operator<<( FrReal *x )
{
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = *x++;
    }
  }
  return *this;
}


XVMatrix&
XVMatrix::operator=(FrReal x)
{
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = x;
    }
  }
  return *this;
}

XVMatrix
XVMatrix::operator*(const XVMatrix &mat) const
{
  XVMatrix p(rowNum,mat.colNum);
  p = 0;

  if (colNum != mat.rowNum)
    _panic((char*)"XVMatrix mismatch in matrix matrix multiply");

  for (int i=0;i<rowNum;i++)
    for (int j=0;j<mat.colNum;j++)
      for (int k=0;k<mat.rowNum;k++)
	p[i][j]+=rowPtrs[i][k] * mat.rowPtrs[k][j];
  return p;
}

XVColVector
XVMatrix::operator*(XVColVector &mat) const
{
  XVColVector p(rowNum);
  if (colNum != mat.rowNum)
    _panic((char*)"XVMatrix mismatch in matrix/vector multiply\n");

  p = 0.0;
  for (int j=0;j<colNum;j++) {
    for (int i=0;i<rowNum;i++) {
      p[i]+=rowPtrs[i][j] * mat[j];
    }
  }

  return p;
}

XVRowVector
XVMatrix::operator*(XVRowVector &mat) const
{
  XVRowVector p(colNum);
  FrReal *ptr = mat.rowPtrs[0],*ptr1;
  float pp;

  if (rowNum != mat.colNum)
    _panic((char*)"XVMatrix mismatch in matrix/vector multiply\n");

  p = 0.0;
  for (int i=0;i<rowNum;i++) {
    ptr1 = rowPtrs[i];
    pp = ptr[i];
    for (int j=0;j<colNum;j++) {
      p[j] += ptr1[j] * pp;
    }
  }
  return p;
}

XVMatrix
XVMatrix::operator/(FrReal x) const
{
  XVMatrix v(rowNum,colNum);
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      v.rowPtrs[i][j] = rowPtrs[i][j]/x;
  return v;
}

XVMatrix
XVMatrix::operator+(const XVMatrix &mat) const
{
  CHECKSIZE(mat,(char*)"Incompatible size in +");
  XVMatrix v(rowNum,colNum);
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      v.rowPtrs[i][j] = mat.rowPtrs[i][j]+rowPtrs[i][j];
  return v;
}


XVColVector
XVColVector::operator+(const XVColVector &mat) const
{
  CHECKSIZE(mat,(char*)"Incompatible size in +");
  XVColVector v(rowNum);
  for (int i=0;i<rowNum;i++)
      v[i] = (*this)[i] + mat[i];
  return v;
}

XVMatrix
XVMatrix::operator-(const XVMatrix &mat) const
{
  CHECKSIZE(mat,(char*)"Incompatible size in -");
  XVMatrix v(rowNum,colNum);
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      v.rowPtrs[i][j] = rowPtrs[i][j]-mat.rowPtrs[i][j];
  return v;
}

XVColVector
XVColVector::operator-(const XVColVector &mat) const
{
  CHECKSIZE(mat,(char*)"Incompatible size in +");
  XVColVector v(rowNum);
  for (int i=0;i<rowNum;i++)
      v[i] = (*this)[i] - mat[i];
  return v;
}

XVMatrix
XVMatrix::operator-() const //negate
{
  XVMatrix A=(*this);

  for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
    	A[i][j] = - rowPtrs[i][j];
  return A;
}

XVColVector
XVColVector::operator-()
{
  XVColVector A=(*this);

  for (int i=0; i<rowNum; i++)
    	A[i]= - (*this)[i];
  return A;
}

XVMatrix&
XVMatrix::operator+=(FrReal x){
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      rowPtrs[i][j] += x;
  return *this;
}

XVMatrix&
XVMatrix::operator+=(const XVMatrix &mat)
{
  CHECKSIZE(mat,(char*)"Incompatible size in +=");
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      rowPtrs[i][j] += mat.rowPtrs[i][j];
  return *this;
}

XVMatrix&
XVMatrix::operator-=(FrReal x){
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      rowPtrs[i][j] -= x;
  return *this;
}

XVMatrix&
XVMatrix::operator-=(const XVMatrix &mat) {
  CHECKSIZE(mat,(char*)"Incompatible size in -=");
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
      rowPtrs[i][j] -= mat.rowPtrs[i][j];
  return *this;
}

XVMatrix&
XVMatrix::operator*=(FrReal x)
{
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  rowPtrs[i][j] *= x;
  return *this;
}

XVMatrix&
XVMatrix::operator/=(FrReal x)
{
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  rowPtrs[i][j] /= x;
  return *this;
}

XVMatrix
XVMatrix::operator*(FrReal x) 
{
  
  XVMatrix v(rowNum,colNum);
  int i;int j;
  for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  v.rowPtrs[i][j] =x*rowPtrs[i][j];
  return v;
}

XVColVector
XVColVector::operator*(FrReal x) const
{
  XVColVector v(rowNum);
  for (int i=0;i<rowNum;i++)
    v[i] = (*this)[i] * x;
  return v;
}

XVColVector&
XVColVector::operator=(const XVMatrix &m)
{
  //CHECKSIZE(m,"Incompatible size in =");

  resize(m.rowNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = m.rowPtrs[i][j];
    }
  }
  return *this;
}

XVColVector&
XVColVector::operator=(const XVColVector &v)
{
  //CHECKSIZE(v,"Incompatible size in =");

  resize(v.rowNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = v.rowPtrs[i][j];
    }
  }
  return *this;
}

XVColVector&
XVColVector::operator<<(const XVColVector &v)
{
  resize(v.rowNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = v.rowPtrs[i][j];
    }
  }
  return *this;
}

XVColVector&
XVColVector::operator<<( FrReal *x )
{
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = *x++;
    }
  }
  return *this;
}

XVColVector&
XVColVector::operator=(FrReal x)
{
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = x;
    }
  }
  return *this;
}

XVRowVector&
XVRowVector::operator=(const XVRowVector &v)
{
  //CHECKSIZE(v,"Incompatible size in =");

  //if (colNum==0) {resize(v.colNum);}
  resize(v.colNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = v.rowPtrs[i][j];
    }
  }
  return *this;
}

XVRowVector&
XVRowVector::operator=(const XVMatrix &m)
{
  CHECKSIZE(m,(char*)"Incompatible size in =");

  if (colNum==0) {resize(m.colNum);}
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = m.rowPtrs[i][j];
    }
  }
  return *this;
}

XVRowVector&
XVRowVector::operator<<(const XVRowVector &v)
{
  resize(v.colNum);
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = v.rowPtrs[i][j];
    }
  }
  return *this;
}

XVRowVector&
XVRowVector::operator=(FrReal x)
{
  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = x;
    }
  }
  return *this;
}

FrReal
XVMatrix::SumSquare() const
{
  FrReal sum=0.0;
  for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
      sum +=rowPtrs[i][j]*rowPtrs[i][j];

  return sum;
}

FrReal
XVMatrix::Sum() const
{
  FrReal sum=0.0;
  for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
      sum +=rowPtrs[i][j];

  return sum;
}


void
XVMatrix::init_empty(void)
{
  refPtr  = 0;
  rowNum  = 0;
  colNum  = 0;
  data    = 0;
  rowPtrs = 0;
  csize   = 0;
  dsize   = 0;
  trsize  = 0;
}

int
XVMatrix::init(XVMatrix &m,int startr, int startc, int nrows, int ncols)
{

#ifndef NO_BOUNDS_CHECK
  if ((startr < 0) || (startc < 0) || (nrows < 0) || (ncols < 0) ||
      (nrows > m.rowNum) || (ncols > m.colNum))
    _panic((char*)"Submatrix requested out of bounds");
#endif

    // Set up the XVMatrix parameters
  rowNum = nrows;
  colNum = ncols;
  rowPtrs = new FrReal*[nrows];

    // Set up the pointers
  int i;
  for (i=0;i<nrows;i++) {rowPtrs[i] = m.rowPtrs[i+startr]+startc;}

    // This is a shared structure, so reference it.
  refPtr = m.ref();
  data = m.data;
  return 1;
}

XVMatrix::~XVMatrix()
{
  delete[] rowPtrs;
  if (!dataShared()) {
    if (refPtr != NULL) delete refPtr;
    delete[] data;
  }
  else
    unref();
}

XVMatrix
XVMatrix::t() const
{
  XVMatrix tmp(colNum,rowNum);
  int i,j;
  for (i=0;i<rowNum;i++)
    for (j=0;j<colNum;j++)
      tmp[j][i] = (*this)[i][j];
  return tmp;
}

XVColVector
XVRowVector::t() const
{
  XVColVector tmp(colNum);
  for (int i=0;i<colNum;i++)
      tmp[i] = (*this)[i];
  return tmp;
}

XVRowVector
XVColVector::t() const
{
  XVRowVector tmp(rowNum);
  for (int i=0;i<rowNum;i++)
      tmp[i] = (*this)[i];
  return tmp;
}

// computes M^t M
XVMatrix
XVMatrix::ip() const
{
  XVMatrix tmp(colNum,colNum);
  FrReal sum = 0;

  int i,j,k;
  for (i=0;i<colNum;i++) {
    for (j=i;j<colNum;j++) {
      sum = 0;
      for (k=0;k<rowNum;k++)
	sum += (*this)[k][i]* (*this)[k][j];
      tmp[j][i] = tmp[i][j] = sum;
    }
  }
  return tmp;
}

FrReal
XVColVector::ip() const
{
  FrReal sum = 0;
  for (int i=0;i<rowNum;i++)
      sum += sqr((*this)[i]);

  return sum;
}

FrReal
XVColVector::ip(const XVColVector &x) const
{
  assert(n_of_rows() == x.n_of_rows());

  FrReal sum = 0;

  for (int i=0;i<rowNum;i++)
      sum += ((*this)[i])*x[i];

  return sum;
}

// computes M M^T
XVMatrix XVMatrix::op() const
{
  XVMatrix tmp(rowNum,rowNum);
  FrReal sum = 0;
  int i,j,k;
  for (i=0;i<rowNum;i++) {
    for (j=0;j<rowNum;j++) {
      sum = 0;
      for (k=0;k<colNum;k++) {
	sum += rowPtrs[i][k] * rowPtrs[j][k];
      }
      tmp[j][i] = tmp[i][j] = sum;
    }
  }
  return tmp;
}

#define TINY 1.0e-20;

/*****************************************************
 *                                                   *
 * LUD related functions                             *
 *                                                   *
 *****************************************************/

void
XVMatrix::LUDcmp(int *perm, int& d)
{
  int n = rowNum;

  int i,imax,j,k;
  FrReal big,dum,sum,temp;
  XVColVector vv(n);

  d=1;
  for (i=0;i<n;i++) {
    big=0.0;
    for (j=0;j<n;j++)
      if ((temp=fabs(rowPtrs[i][j])) > big) big=temp;
    if (big == 0.0) _panic((char*)"Singular matrix in  LUDcmp");
    vv[i]=1.0/big;
  }
  for (j=0;j<n;j++) {
    for (i=0;i<j;i++) {
      sum=rowPtrs[i][j];
      for (k=0;k<i;k++) sum -= rowPtrs[i][k]*rowPtrs[k][j];
      rowPtrs[i][j]=sum;
    }
    big=0.0;
    for (i=j;i<n;i++) {
      sum=rowPtrs[i][j];
      for (k=0;k<j;k++)
	sum -= rowPtrs[i][k]*rowPtrs[k][j];
      rowPtrs[i][j]=sum;
      if ( (dum=vv[i]*fabs(sum)) >= big) {
	big=dum;
	imax=i;
      }
    }
    if (j != imax) {
      for (k=0;k<n;k++) {
	dum=rowPtrs[imax][k];
	rowPtrs[imax][k]=rowPtrs[j][k];
	rowPtrs[j][k]=dum;
      }
      d *= -1;
      vv[imax]=vv[j];
    }
    perm[j]=imax;
    if (rowPtrs[j][j] == 0.0) rowPtrs[j][j]=TINY;
    if (j != n) {
      dum=1.0/(rowPtrs[j][j]);
      for (i=j+1;i<n;i++) rowPtrs[i][j] *= dum;
    }
  }
}

#undef TINY

void
XVMatrix::LUBksb(int *perm, XVColVector& b)
{
  int n = rowNum;

  int i,ii=-1,ip,j;
  FrReal sum;

  for (i=0;i<n;i++) {
    ip=perm[i];
    sum=b[ip];
    b[ip]=b[i];
    if (ii != -1)
      for (j=ii;j<=i-1;j++) sum -= rowPtrs[i][j]*b[j];
    else if (sum) ii=i;
    b[i]=sum;
  }
  for (i=n-1;i>=0;i--) {
    sum=b[i];
    for (j=i+1;j<n;j++) sum -= rowPtrs[i][j]*b[j];
    b[i]=sum/rowPtrs[i][i];
  }
}

void
XVMatrix::solveByLUD(const XVColVector &B, XVColVector& X)
{
  if (colNum != rowNum)
    _panic((char*)"Solution for nonsquare matrix");

  XVMatrix A(rowNum, rowNum);
  A = *this;

  X = B;

  int *perm = new int[rowNum];
  int p;

  A.LUDcmp(perm, p);

  A.LUBksb(perm, X);

  delete[] perm;
}

XVColVector XVMatrix::LUDsolve(const XVColVector& B) {
  XVColVector X(rowNum);
  solveByLUD(B, X);
  return X;
}

void
XVMatrix::LDLtDcmp()
{
  XVMatrix* A = this;
  int n = rowNum;
  XVMatrix L(n, n);
  int i, j;

  XVColVector v(A->rowNum);
  for (j=0; j<n; j++) {
    // Compute v
    for (i=0; i<=j-1; i++) {
      v[i] = (*A)[j][i]*(*A)[i][i];
    }

    if (j==0) {
      v[j] = (*A)[j][j];
      // Store D[j] only
      (*A)[j][j] = v[j];
      (*A)(j+1,n-1,j,j) = (*A)(j+1,n-1,j,j)/v[j];
    } else {
      v[j] = (*A)[j][j]-((*A)(j,j,0,j-1)*v(0,j-1,0,0))[0][0];
      // Store D[j] and compute L(j+1:n,j)
      (*A)[j][j] = v[j];
      (*A)(j+1,n-1,j,j) = ((*A)(j+1,n-1,j,j)-
			   (*A)(j+1,n-1,0,j-1)*v(0,j-1,0,0))/
			   v[j];
    }
  }
}

XVMatrix
XVMatrix::sqrt()
{
    // the matrix has to be symmetric and positive definite
  XVMatrix A(rowNum, colNum);

  A = *this;
  A.LDLtDcmp();
  for (int j=0; j<colNum; j++) {
    A[j][j] = ::sqrt(A[j][j]);
    for (int i=j+1; i<rowNum; i++) {
      A[i][j] *= A[j][j];
      A[j][i] = 0.0;
    }
  }
  return A;
}

XVMatrix
XVMatrix::Map(FrReal (*fn)(FrReal)) const
{
  int i, j;
  XVMatrix temp(rowNum,colNum);

  for (i=0;i<rowNum;i++)
    for (j=0;j<colNum;j++)
      temp[i][j] = fn((*this)[i][j]);

  return temp;
}

/*****************************************************
 *                                                   *
 * SVD related functions                             *
 *                                                   *
 *****************************************************/

static FrReal at,bt,ct;
#define PYTHAG(a,b) ((at=fabs(a)) > (bt=fabs(b)) ? \
(ct=bt/at,at*::sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt,bt*::sqrt(1.0+ct*ct)): 0.0))

static FrReal maxarg1,maxarg2;

#ifdef MAX
#undef MAX
#endif

#define MAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1)>(maxarg2)?(maxarg1):(maxarg2))

#ifdef SIGN
#undef SIGN
#endif

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

// singular value decomposition
//
// M = U.t * Md * V
//
// if I understand it correctly, W is the diagonal matrix stored in
// a column vector, V becomes V.t, and A becomes U.t.
// try not to use SVDcmp directly. See SVD(M, Md, U, V) -- JW.

void XVMatrix::SVDcmp(XVColVector& W, XVMatrix& V)
{
  int m = this->rowNum;
  int n = this->colNum;

  int flag,i,its,j,jj,k,l,nm;
  FrReal c,f,h,s,x,y,z;
  FrReal anorm=0.0,g=0.0,scale=0.0;

  // So that the original NRC code (using 1..n indexing) can be used
  // This should be considered as a temporary fix.
  FrReal **a = new FrReal*[m+1];
  FrReal **v = new FrReal*[n+1];
  FrReal **w = W.rowPtrs;

  w--;
  for (i=1;i<=m;i++) {
    a[i] = this->rowPtrs[i-1]-1;
  }
  for (i=1;i<=n;i++) {
    v[i] = V.rowPtrs[i-1]-1;
  }

  if (m < n)
  {
	  delete[] a;
	  delete[] v;
	  throw XVException((char*)"SVDcmp: You must augment A with extra zero rows"); //_panic((char*)"SVDcmp: You must augment A with extra zero rows");
  }
  FrReal* rv1=new FrReal[n+1];

  for (i=1;i<=n;i++) {
    l=i+1;
    rv1[i]=scale*g;
    g=s=scale=0.0;
    if (i <= m) {
      for (k=i;k<=m;k++) scale += fabs(a[k][i]);
      if (scale) {
	for (k=i;k<=m;k++) {
	  a[k][i] /= scale;
	  s += a[k][i]*a[k][i];
	}
	f=a[i][i];
	g = -SIGN(::sqrt(s),f);
	h=f*g-s;
	a[i][i]=f-g;
	if (i != n) {
	  for (j=l;j<=n;j++) {
	    for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
	    f=s/h;
	    for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
	  }
	}
	for (k=i;k<=m;k++) a[k][i] *= scale;
      }
    }
    w[i][0]=scale*g;
    g=s=scale=0.0;
    if (i <= m && i != n) {
      for (k=l;k<=n;k++) scale += fabs(a[i][k]);
      if (scale) {
	for (k=l;k<=n;k++) {
	  a[i][k] /= scale;
	  s += a[i][k]*a[i][k];
	}
	f=a[i][l];
	g = -SIGN(::sqrt(s),f);
	h=f*g-s;
	a[i][l]=f-g;
	for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
	if (i != m) {
	  for (j=l;j<=m;j++) {
	    for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
	    for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
	  }
	}
	for (k=l;k<=n;k++) a[i][k] *= scale;
      }
    }
    anorm=MAX(anorm,(fabs(w[i][0])+fabs(rv1[i])));
  }
  for (i=n;i>=1;i--) {
    if (i < n) {
      if (g) {
	for (j=l;j<=n;j++)
	  v[j][i]=(a[i][j]/a[i][l])/g;
	for (j=l;j<=n;j++) {
	  for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
	  for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
	}
      }
      for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
    }
    v[i][i]=1.0;
    g=rv1[i];
    l=i;
  }
  for (i=n;i>=1;i--) {
    l=i+1;
    g=w[i][0];
    if (i < n)
      for (j=l;j<=n;j++) a[i][j]=0.0;
    if (g) {
      g=1.0/g;
      if (i != n) {
	for (j=l;j<=n;j++) {
	  for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
	  f=(s/a[i][i])*g;
	  for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
	}
      }
      for (j=i;j<=m;j++) a[j][i] *= g;
    } else {
      for (j=i;j<=m;j++) a[j][i]=0.0;
    }
    ++a[i][i];
  }
  for (k=n;k>=1;k--) {
    for (its=1;its<=30;its++) {
      flag=1;
      for (l=k;l>=1;l--) {
	nm=l-1;
	if (fabs(rv1[l])<1e-18) {
	  flag=0;
	  break;
	}
	if (fabs(w[nm][0])<1e-18) break;
      }
      if (flag) {
	c=0.0;
	s=1.0;
	for (i=l;i<=k;i++) {
	  f=s*rv1[i];
	  if (fabs(f)+anorm != anorm) {
	    g=w[i][0];
	    h=PYTHAG(f,g);
	    w[i][0]=h;
	    h=1.0/h;
	    c=g*h;
	    s=(-f*h);
	    for (j=1;j<=m;j++) {
	      y=a[j][nm];
	      z=a[j][i];
	      a[j][nm]=y*c+z*s;
	      a[j][i]=z*c-y*s;
	    }
	  }
	}
      }
      z=w[k][0];
      if (l == k) {
	if (z < 0.0) {
	  w[k][0] = -z;
	  for (j=1;j<=n;j++) v[j][k]=(-v[j][k]);
	}
	break;
      }
      if (its == 30)
      {
    	  delete[] rv1;
    	  delete[] a;
    	  delete[] v;
    	  throw XVException((char*)"SVDcmp:no convergence in 30 iterations"); //_panic((char*)"SVDcmp:no convergence in 30 iterations");
      }
      x=w[l][0];
      nm=k-1;
      y=w[nm][0];
      g=rv1[nm];
      h=rv1[k];
      f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
      g=PYTHAG(f,1.0);
      f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
      c=s=1.0;
      for (j=l;j<=nm;j++) {
	i=j+1;
	g=rv1[i];
	y=w[i][0];
	h=s*g;
	g=c*g;
	z=PYTHAG(f,h);
	rv1[j]=z;
	c=f/z;
	s=h/z;
	f=x*c+g*s;
	g=g*c-x*s;
	h=y*s;
	y=y*c;
	for (jj=1;jj<=n;jj++) {
	  x=v[jj][j];
	  z=v[jj][i];
	  v[jj][j]=x*c+z*s;
	  v[jj][i]=z*c-x*s;
	}
	z=PYTHAG(f,h);
	w[j][0]=z;
	if (z) {
	  z=1.0/z;
	  c=f*z;
	  s=h*z;
	}
	f=(c*g)+(s*y);
	x=(c*y)-(s*g);
	for (jj=1;jj<=m;jj++) {
	  y=a[jj][j];
	  z=a[jj][i];
	  a[jj][j]=y*c+z*s;
	  a[jj][i]=z*c-y*s;
	}
      }
      rv1[l]=0.0;
      rv1[k]=f;
      w[k][0]=x;
    }
  }
  delete[] rv1;
  delete[] a;
  delete[] v;
}

#undef SIGN
#undef MAX
#undef PYTHAG

void XVMatrix::SVBksb(const XVColVector& w, const XVMatrix& v,
		    const XVColVector& b, XVColVector& x)
{
  int m = this->rowNum;
  int n = this->colNum;
  FrReal** u = rowPtrs;

  int jj,j,i;
  FrReal s,*tmp;

  tmp=new FrReal[n];
  for (j=0;j<n;j++) {
    s=0.0;
    if (w[j]) {
      for (i=0;i<m;i++) s += u[i][j]*b[i];
      s /= w[j];
    }
    tmp[j]=s;
  }
  for (j=0;j<n;j++) {
    s=0.0;
    for (jj=0;jj<n;jj++) s += v[j][jj]*tmp[jj];
    x[j]=s;
  }
  delete[] tmp;
}



#define TOL 1.0e-5

void
XVMatrix::solveBySVD(const XVColVector& b, XVColVector& x)
{
  int j;
  FrReal wmax,thresh;

  int ma = this->colNum;

  XVColVector w(ma);
  XVMatrix v(ma,ma);

  XVMatrix A(rowNum, colNum);
  A= *this;
  A.SVDcmp(w,v);

  wmax=0.0;
  for (j=0;j<ma;j++)
    if (w[j] > wmax) wmax=w[j];
  thresh=TOL*wmax;
  for (j=0;j<ma;j++)
    if (w[j] < thresh) w[j]=0.0;

  A.SVBksb(w,v,b,x);
}

XVColVector XVMatrix::SVDsolve(const XVColVector& B) {
  XVColVector X(colNum);
  solveBySVD(B, X);
  return X;
}

#undef TOL
#define TOL 1e-5

XVMatrix
XVMatrix::i() const
{
  int i,j;

  if ( rowNum != colNum)
    _panic((char*)"Cannot invert a non-square matrix");

  XVMatrix B(rowNum, rowNum), X(rowNum, rowNum);
  XVMatrix V(rowNum, rowNum);
  XVColVector W(rowNum);

  for (i=0; i<rowNum; i++) {
    for (j=0; j<rowNum; j++) {
      B[i][j] = (i == j) ? 1 : 0;
    }
  }

  XVMatrix A(rowNum, rowNum), C(rowNum, rowNum);
  C = A = *this;


  A.SVDcmp(W, V);

  // Zero out small W's
  FrReal maxW=0.0;
  for (j=0;j<rowNum;j++)
    if (W[j] > maxW) maxW=W[j];
  FrReal thresh=TOL*maxW;
  for (j=0;j<rowNum;j++)
    if (W[j] < thresh) W[j]=0.0;

#ifdef SUBSCRIPT_START_WITH_1
  for (j=1; j<=rowNum; j++) { // col() starts with 1
#else
  for (j=0; j<rowNum; j++) { // col() starts with 0
#endif

    // This generates a compiler warning, but it is
    // right === the .col generates an rvalue which
    // is a pointer into X which then gets modified by
    // the routine --- any ideas how to do it better
    // without copying?

    XVColVector Xcol = X.col(j);
    A.SVBksb(W, V, B.col(j), Xcol);
  }

//  cerr << "A=\n"<< A << endl
//       << "W=\n"<< W << endl
//       << "V=\n"<< V << endl;

  return X;
}

#undef TOL


ostream &operator <<(ostream &s,const XVMatrix &m)
{
  int i,j;

  for (i=0;i<m.rowNum;i++) {
    for (j=0;j<m.colNum;j++)
      s << m[i][j] << "  ";
    s << endl;
  }

  return s;
}

/************************************************************************
 *
 * For computing jacobian matrix using finite difference approximation
 *
 ************************************************************************/

/* Try to take derivatives of a function. */
FrReal
XVMatrix::deriv_from_pts(FrReal y1, FrReal y2, FrReal y3,
		       FrReal dstep, FrReal x)
{
  FrReal a,b;
  /* printf("y1 = %f, y2 = %f, y3 = %f ",y1,y2,y3); */
  y1 -= y2;
  y3 -= y2;
  a = (y1 + y3)/(2*dstep*dstep);
  b = a * (dstep - 2*x) - y1/dstep;
  /* printf("a = %f, b = %f \n",a,b);*/
  return 2*a*x + b;
}

/* Approximate dfn(i)/dval(n) for each i using some method or another. */
void
XVMatrix::deriv_n(XVColVector (*fn)(const XVColVector&, const XVColVector&),
	const XVColVector& val, int n, const XVColVector& extra, XVColVector& deriv)
{
  int i;
  XVColVector temp[3];
  XVColVector new_val;
  new_val << val;

  FrReal dstep = ::sqrt(FR_EPSFCN)*fabs(val[n]);
  new_val[n] -= dstep;

  for (i=0;i<3;i++) {
    temp[i] << fn(new_val,extra);
    new_val[n] += dstep;
  }
  int dim = temp[0].rowNum;

  for (i=0;i<dim;i++) {
    deriv[i] = deriv_from_pts(temp[0][i],temp[1][i],temp[2][i],dstep,val[n]); }
}


/* computes the jacobian of a function evaluated with parameters
   where and other inputs extra.  Form is:

   dy1/da1  dy2/da1 ....
   dy1/da2  dy2/da2 ....
   .         .
   .         .
   .         .

   where y = f(a).

   */
XVMatrix
XVMatrix::jacobian(XVColVector (*fn)(const XVColVector&, const XVColVector&),
		 int odim, const XVColVector& where, const XVColVector& extra)
{
  int idim = where.rowNum;
  XVMatrix jac(odim,idim);

#ifdef SUBSCRIPT_START_WITH_1
  for (int i=1;i<=idim;i++)     // col() starts with 1
#else
  for (int i=0;i<idim;i++) {    // col() starts with 0
#endif
    XVColVector jac_col = jac.col(i);
    deriv_n(fn,where,i,extra,jac_col);
  }
  return jac;
}


// M = U.t * Md * V
// a more intuitive version (calls  SVDcmp)  -- JW.

void SVD(XVMatrix& M, XVMatrix& Md, XVMatrix& U, XVMatrix& V) {

  XVColVector D(M.n_of_rows());
  U = M;
  U.SVDcmp(D, V); // see explanation on SVDcmp

  U = U.t();
  V = V.t();

  for (int i=0; i<M.n_of_rows(); i++)
    for (int j=0; j<M.n_of_cols(); j++)
        if (i==j)
          Md[i][i]=D[i];
        else //i!=j
          Md[i][j]=0;
}

XVMatrix
operator*(const XVDiagonalMatrix &x, const XVMatrix &y)
{
  if (x.n_of_cols() != y.n_of_rows())
    _panic((char*)"XVMatrix mismatch in matrix/vector multiply\n");

  XVMatrix temp(y);

  for (int i=0;i<y.n_of_rows();i++) {
    for (int j=0;j<y.n_of_cols();j++) {
      temp[i][j] *= x(i);
    }
  }
  return temp;
}


XVRowVector
operator*(const XVDiagonalMatrix &x, const XVRowVector &y)
{
  if (x.n_of_cols() != y.n_of_cols())
    _panic((char*)"XVMatrix mismatch in matrix/vector multiply\n");

  XVRowVector temp(y);

  for (int j=0;j<y.n_of_cols();j++) {
      temp[j] *= x(j);
    }
  return temp;
}


XVMatrix
operator*(const XVMatrix &x, const XVDiagonalMatrix &y)
{
  if (x.n_of_cols() != y.n_of_rows())
    _panic((char*)"XVMatrix mismatch in matrix/vector multiply\n");

  XVMatrix temp(x);

  for (int j=0;j<x.n_of_cols();j++) {
    for (int i=0;i<x.n_of_rows();i++) {
      temp[i][j] *= y(j);
    }
  }
  return temp;
}


XVDiagonalMatrix
operator*(const XVDiagonalMatrix &x, const XVDiagonalMatrix &y)
{
  if (x.n_of_rows() != y.n_of_rows())
    _panic((char*)"Matrix mismatch in diagonal matrix multiply\n");

  XVColVector temp(x.t);

  for (int j=0;j<y.n_of_cols();j++)
    temp[j] *= y(j);
  return XVDiagonalMatrix(temp);
}


ostream &operator << (ostream &s,const XVDiagonalMatrix &x)
{
  s << (x.t).t();
  return s;
}

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------

FrReal invs(FrReal x) { return 1 / x; }

// Add a column to x
XVMatrix add_column(XVMatrix & x, XVColVector & c)
{
  XVMatrix temp(x.rowNum,x.colNum+1);
  for (int i=0; i<x.rowNum; ++i) {
    for (int j=0; j<x.colNum; ++j) {
      temp.rowPtrs[i][j] = x.rowPtrs[i][j];
    }
  }
  temp.Column(temp.n_of_cols()-1) = c;
  return temp;
}
