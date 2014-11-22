#ifndef _XVEXCEPTION_H_
#define _XVEXCEPTION_H_

#include <exception>

using namespace std ;

/** base class for an XVision exception */
class XVException : exception{

 protected:

  char * error;

 public:

  XVException(){ error[0]=0; }
  XVException(char * err){ error = err; }

  const char * what() const throw () { return error ; }
};

#endif
