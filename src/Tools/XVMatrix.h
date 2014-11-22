// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 *
 * XVMatrix.h
 *
 * @author Gregory D. Hager, Sam Lang
 * @version $Id: XVMatrix.h,v 1.1.1.1 2008/01/30 18:44:08 burschka Exp $  02/20/95 
 * 
 * Header file for a XVMatrix package for small systems
 * A simple matrix package for small applications.  Matrices are stored in
 * row-major order using a ragged-array data structure.  Submatrices are
 * also supported as are row and column vectors.
 *
 * Bounds checking and size checking are also included by default.
 * Defining the NO_BOUNDS_CHECK turns off bounds checking.
 * Defining check_size turns off size checking on matrix operations.
 * Check in XVMatrix.cc at the start of the file to see whether
 * NO_BOUNDS_CHECK or CHECKSIZE are defined as you would like them to be.
 *
 * The SUBSCRIPT_START_WITH_1 preprocessor variable, if defined,
 * allows you to index these structures from 1 to n, rather than from
 * 0 to n-1.
 *
 * Matrices should be templates, but for some reason templates aren't
 * working so I'll just hardcode the type for now.
 *
 * This header file contains the definitions for the classes
 * _rowvec, XVRowVector, XVColumnVector, XVMatrix, and XVXVDiagonalMatrix
 */

//#define SUBSCRIPT_START_WITH_1

#ifndef _XVMATRIX_H_
#define _XVMATRIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <XVImageScalar.h>

// Next two #defines used to be in XVMatrix.cc, but should be here.
#define NO_BOUNDS_CHECK
//#define CHECKSIZE(m,msg) /* do nothing */

#define FR_EPSFCN 1.0E-16       // FrReal is the type of the matrices
#define FrReal double           // FR_EPSFCN is the minimum available value

extern void _panic(char *);     // For simple exception handling.

typedef struct {                // Data structure for reference counts
  FrReal *data;
  int refCount;
} RefCounter;

/// Redefine the macro CHECKSIZE to turn off size checking
#ifndef CHECKSIZE
#define CHECKSIZE(m,mess) \
{if ((m.rowNum != rowNum) || (m.colNum != colNum)) _panic(mess);}
#endif

/// the class _rowvec
/// is a data structure used to implement checking on 2nd matrix index.
/// A matrix is essentially a column vector of _rowvecs
class _rowvec
{
private:
  FrReal *data;
  int colNum;

public:

  _rowvec(FrReal *datain,int ncols) {data = datain; colNum = ncols;}

  FrReal&
  operator[] (int n) {
    if ((n < 0) || (n >= colNum))
      _panic((char*)"XVMatrix second argument out of range");
    return data[n];
  }

  const FrReal&
  operator[] (int n) const {
    if ((n < 0) || (n >= colNum))
      _panic((char*)"XVMatrix second argument out of range");
    return data[n];
  }

};

class XVRowVector;
class XVColVector;
class XVMatrix;
class XVDiagonalMatrix;


// The XVMatrix class definition starts here
//---------------------------------------------------------------------
//*
//* XVMatrix.h
//*
//* 02/20/95 Gregory D. Hager
//* 
//* Header file for a XVMatrix package for small systems
//* A simple matrix package for small applications.  Matrices are stored in
//* row-major order using a ragged-array data structure.  Submatrices are
//* also supported as are row and column vectors.
//*
//* Bounds checking and size checking are also included by default.
//* Defining the NO_BOUNDS_CHECK turns off bounds checking.
//* Defining check_size turns off size checking on matrix operations.
//* Check in XVMatrix.cc at the start of the file to see whether
//* NO_BOUNDS_CHECK or CHECKSIZE are defined as you would like them to be.
//*
//* The SUBSCRIPT_START_WITH_1 preprocessor variable, if defined,
//* allows you to index these structures from 1 to n, rather than from
//* 0 to n-1.
//*
//* Matrices should be templates, but for some reason templates aren't
//* working so I'll just hardcode the type for now.
//*
//* This header file contains the definitions for the classes
//* _rowvec, XVRowVector, XVColumnVector, XVMatrix, and XVDiagonalMatrix
//*
//---------------------------------------------------------------------

class XVMatrix
{
  friend class XVRowVector;
  friend class XVColVector;
  friend class XVDiagonalMatrix;
  friend XVMatrix add_column(XVMatrix &x, XVColVector &c);

private:
    /// this creates a numerical approximation to the derivative at x
    /// using the function values yi and the horizontal step dstep
  static FrReal deriv_from_pts(FrReal y1, FrReal y2, FrReal y3,
			       FrReal dstep, FrReal x);

    /// this takes the (numerical approximation to the) derivative of
    /// the function *fn and puts it into "deriv"
  static void deriv_n(XVColVector (*fn)(const XVColVector&, const XVColVector&),
	 const XVColVector& val, int n, const XVColVector& extra, XVColVector& deriv);

    /// Used for submatrix operations
  int init(XVMatrix &m,int startr, int startc, int nrows, int ncols);

  // initialise all fields to 0
  void init_empty(void);

protected:
    /// Reference count data structure.  This structure is shared
    /// with all other matrices using this storage.
  RefCounter *refPtr;
  int rowNum;
  int colNum;

    /// These are the data pointers
  FrReal *data;
  FrReal **rowPtrs;

    /// Total storage size --  by default, we allocate double the
    /// needed storage to allow room for resizing.
  int csize;
    /// Size of the matrix now (rowNum * colNum)
  int dsize;
    /// Total Row Space
  int trsize;

    /// These are functions to reference and unreference shared data.
  RefCounter *ref();
  void unref();

    /// Return value == is my data shared with someone?
  int dataShared();

    /// submatrix constructor  
  XVMatrix(XVMatrix &m, int startr, int startc, int nrows, int ncols);


public:
    /// All of the public matrix constructors and destructors
  XVMatrix();
  XVMatrix(const XVMatrix& m);
  XVMatrix(int rr,int cc);
  XVMatrix(int rr,int cc,FrReal* x);
  virtual ~XVMatrix ();
    /// Accessor functions for the row and column number
  int n_of_rows() const;
  int n_of_cols() const;

    /// Now resize () is used only in constructors. Maybe it should be used in
    /// in destructive assignment (or <<, which is not implemented yet).
  int resize(int nrows, int ncols);

    /// All of the next 10  matrix operators return references to the calling
    /// XVMatrix, which is altered as documented here. The following ones return
    /// copies of newly constructed matrices. The operators which take 
    /// This operator= sets all of the elements to x
  XVMatrix& operator=(FrReal x);
  XVMatrix& operator=(const XVMatrix &m);
  XVMatrix& operator<<(const XVMatrix &m);
    /// This assigns the matrix elements, row-dominant, from an array
  XVMatrix& operator<<(FrReal*);
  XVMatrix& operator+=(FrReal x);
  XVMatrix& operator+=(const XVMatrix &mat);
  XVMatrix& operator-=(FrReal x);
  XVMatrix& operator-=(const XVMatrix &mat);
  XVMatrix& operator*=(FrReal x);
  XVMatrix& operator/=(FrReal x);
  XVMatrix  operator*(FrReal x);
  XVMatrix operator*(const XVMatrix &mat) const;
  XVColVector operator*(XVColVector &mat) const;
  XVRowVector operator*(XVRowVector &mat) const;
  XVMatrix operator/(FrReal x) const;
  XVMatrix operator+(const XVMatrix &mat) const;
  XVMatrix operator-(const XVMatrix &mat) const;
  XVMatrix operator-() const;
    /// returns the Sum of the Squares of the elements
  FrReal SumSquare() const;
    /// returns the Sum of the elements
  FrReal Sum() const;

    /// These are destructive solution operations.
#ifndef NO_BOUNDS_CHECK
  _rowvec operator[](int n);
#else
  FrReal* operator[](int n);
#endif

    /// These functions return copies of specified rows & cols
  FrReal* operator[](int n) const;
  XVRowVector row(int i);
  XVRowVector Row(int i);
  XVColVector col(int j);
  XVColVector Column(int j);

#ifdef SUBSCRIPT_START_WITH_1  
    /// If these functions appear, then SUBSCRIPT_START_WITH_1 is defined
  XVMatrix operator()(int sr, int lr, int sc, int lc);
  XVMatrix Rows(int first_row, int last_row);
  XVMatrix Columns(int first_col, int last_col);
        
#else
    /// If these functions appear, then SUBSCRIPT_START_WITH_1 is not defined
  XVMatrix operator()(int sr, int lr, int sc, int lc);
  XVMatrix Rows(int first_row, int last_row);
  XVMatrix Columns(int first_col, int last_col);

#endif // SUBSCRIPT_START_WITH_1
 
    /// Inverse, transpose, inner product and outer product.
  XVMatrix i() const;
  XVMatrix t() const;
  XVMatrix ip() const;
  XVMatrix op() const;

    /// The following functions are both desctuctive and nondestructive
    /// Destructive LU decomposition
  void LUDcmp(int* perm, int& d);
    /// Destructive backsubstitution
  void LUBksb(int* perm, XVColVector& b);
    /// Nondestructive solving Ax = B
  void solveByLUD(const XVColVector& b, XVColVector& x);
    /// Nondestructive x = A.LUDsolve(B), equivalent to solveByLUD
  XVColVector LUDsolve(const XVColVector& b);
    /// Does L D Ltranspose decomposition
  void LDLtDcmp();
    /// XVMatrix square root
  XVMatrix sqrt();
    /// Destructive singular value decomposition
  void SVDcmp(XVColVector& w, XVMatrix& v);
    /// Destructive singular value backsubstitution
  void SVBksb(const XVColVector& w,const XVMatrix& v,
	      const XVColVector& b, XVColVector& x);
    /// Nondestructive solving of Ax = B
  void solveBySVD(const XVColVector& b, XVColVector& x);
    /// Nondestructive equivalent to above
  XVColVector SVDsolve(const XVColVector& b);
    /// creates a jacobian matrix of the first argument at "where"
    /// The jacobian has odim (output dimension) rows and
    /// as many columns as "where"
  static XVMatrix jacobian(XVColVector (*fn)(const XVColVector&, const XVColVector&),
			 int odim, const XVColVector& where, const XVColVector& extra);
    /// prints out the elements of the matrix "m"
  friend ostream &operator << (ostream &s,const XVMatrix &m);
    /// This takes the function "*fn" and performs it to each element
    /// of the invoking matrix and puts the resulting elements into the
    /// returned matrix.
  XVMatrix Map(FrReal (*fn)(FrReal)) const;

  XVMatrix add_column(const XVColVector &c) const;

};



/// XVRowVector class
/// For specifics on the member functions here, see the XVMatrix class
/// from which this is derived. Basically, XVRowVector is a horizontal
/// matrix, while XVColVector is a vertical matrix.
class XVRowVector : public XVMatrix
{
  friend class XVMatrix;

protected:
  XVRowVector(XVMatrix &m, int i);

public:
  XVRowVector();
  XVRowVector(int nn);
  XVRowVector(const XVRowVector &v);
#ifdef __xvimagescalar_h
  XVRowVector(const XVImageScalar<double> &x);
#endif

  void resize(int i);
  FrReal &operator [](int n);
  const FrReal &operator [](int n) const;
  XVRowVector &operator=(const XVRowVector &v);
  XVRowVector &operator<<(const XVRowVector &v);
  XVRowVector &operator=(const XVMatrix &m);
  XVRowVector &operator=(FrReal x);
  XVColVector t() const;
};



/// XVColVector class
/// For specifics on the member functions here, see the XVMatrix class
/// from which this is derived. Basically, XVRowVector is a horizontal
/// matrix, while XVColVector is a vertical matrix.
class XVColVector : public XVMatrix
{
  friend class XVMatrix;

protected:
  XVColVector (XVMatrix &m, int j);
  XVColVector (XVColVector &m, int startr, int nrows);

public:

  XVColVector();
  XVColVector(int nn);
  XVColVector (const XVColVector &v);
  ~XVColVector();

  void resize(int i);
  FrReal &operator [](int n);
  const FrReal &operator [](int n) const;
  XVColVector &operator=(const XVColVector &v);
  XVColVector &operator<<(const XVColVector &v);
  XVColVector &operator<<(FrReal *);
  XVColVector &operator=(const XVMatrix &m);
  XVColVector &operator=(FrReal x);

  // J.W. added the below Aug 15th
  
  XVColVector operator+(const XVColVector &mat) const;
  XVColVector operator-(const XVColVector &mat) const;
  XVColVector operator*(FrReal x) const;
  XVColVector operator-();

  XVColVector Rows(int first_row, int last_row);
  XVRowVector t() const;
  FrReal ip() const;
  FrReal ip(const XVColVector &) const;
};

/// SVD(M, Md, U, V), where M=U.t * Md * V
extern void SVD(XVMatrix& , XVMatrix&, XVMatrix&, XVMatrix& );

/// For matrix operations used directly on images
//#ifdef __xvimagescalar_h
template <class T> XVColVector  &operator <<(XVColVector &x,const XVImageScalar<T> &y);
template <class T> XVRowVector  &operator <<(XVRowVector &x,const XVImageScalar<T> &y);
template <class T> XVImageScalar<T> &operator >>(const XVColVector &x,const XVImageScalar<T> &y);
template <class T> XVImageScalar<T> &operator >>(const XVRowVector &x,const XVImageScalar<T> &y);

//#endif





/// Diagonal Matrix Class
/// A diagonal matrix is basically a XVColVector 
/// with support to multiply by ordinary matrices in the appropriate
/// fashion. All of the member functions are explained in the Matrix
/// class (most are just adapted versions of those from the Matrix
/// class anyway).
class XVDiagonalMatrix
{
private:  
  XVColVector t;

public:
  XVDiagonalMatrix(int n= 0);
  XVDiagonalMatrix(const XVColVector &tin);

  int n_of_cols() const;
  int n_of_rows() const;
  XVColVector& diagonal();
  void resize(int n);

  FrReal &operator() (int n);
  FrReal operator() (int n) const;
  XVDiagonalMatrix &operator = (const XVDiagonalMatrix &m);
  XVDiagonalMatrix &operator = (FrReal x);

  friend XVMatrix operator*(const XVDiagonalMatrix &, const XVMatrix &);
  friend XVRowVector operator*(const XVDiagonalMatrix &, const XVRowVector &);
  friend XVMatrix operator*(const XVMatrix&, const XVDiagonalMatrix &);
  friend XVDiagonalMatrix operator*(const XVDiagonalMatrix &x,const XVDiagonalMatrix &y);
  friend ostream &operator << (ostream &,const XVDiagonalMatrix &);
};


//----------------------------------------------------------------
// utilities
//----------------------------------------------------------------

FrReal invs(FrReal x);

template <class T>
XVMatrix add_column(XVMatrix &x, XVImageScalar<T> &I);


#include "XVMatrix.icc"

#endif




