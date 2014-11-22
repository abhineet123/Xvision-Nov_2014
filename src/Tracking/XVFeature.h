// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVFeature.h
 *
 * @author Sam Lang
 * @version $Id: XVFeature.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 */

#ifndef _XVFEATURE_H_
#define _XVFEATURE_H_

#include <XVException.h>

class XVTrackerException : public XVException { public:
  XVTrackerException() : XVException((char*)"XVTracker Expection") {} 
  XVTrackerException( char * err ) : XVException(err) {} };  

class XVTrackerOOBException : public XVException { public:
  XVTrackerOOBException() : XVException((char*)"Tracker Out of Bounds") {}
  XVTrackerOOBException( char * err ) : XVException(err) {} };

/**
 * XVStatePair
 *
 * @author Sam Lang
 * @version $Id: XVFeature.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 * 
 * The XVStatePair represents the state of a tracked feature.
 * The class is templated on the state information, and error
 * information.  The first represents the current parameters
 * of the tracked feature, while the second gives an accuracy
 * measure of the state information.
 * 
 * Whatever types you decide to use for the STATE and ERROR of the XVStatePair,
 * the STATE type should be able to handle the operators + and - (adding or
 * subtracting two states).  This is useful in determining change of state
 * (call it delta) from two total states, or in updating a total state from
 * a delta and a previous total state.
 *
 * The ERROR type should be able to handle the logical operator &&.
 * This is useful for combining two errors together, propagating errors, etc.
 *
 * Also, You don't have to use the XVStatePair class to represent your states
 * in XVFeature and XVTracker, but if you don't, the class that you use should
 * also support the operators + and -
 *
 * Abstractly, subtacting two states should give you a change in 
 * state (delta state) or adding a change in state to a state 
 * should give you a new state.
 */

template <class ST, class ER>
class XVStatePair {

  typedef ST STATE;
  typedef ER ERROR;

 public:
  
  STATE state;
  ERROR error;

  XVStatePair(STATE s, ERROR e) : state(s), error(e) {}
  XVStatePair(STATE s) : state(s), error(0) {}
  XVStatePair() {}
};

#define _XVSTATEPAIR_BIN_OP_(_OP_) \
template <class ST, class ER> \
XVStatePair<ST, ER> operator _OP_ (const XVStatePair<ST, ER> & s1, const XVStatePair<ST, ER> & s2){ \
  XVStatePair<ST, ER> newS; \
  newS.state = s1.state _OP_ s2.state; \
  newS.error = s1.error && s2.error; \
  return newS; \
};

_XVSTATEPAIR_BIN_OP_(+);
_XVSTATEPAIR_BIN_OP_(-);

#define _XVSTATEPAIR_COMPARE_OP_(_OP_) \
template <class ST, class ER> \
bool operator _OP_ (const XVStatePair<ST, ER> & s1, const XVStatePair<ST, ER> & s2) { \
  return s1.error _OP_ s2.error; \
};
  
_XVSTATEPAIR_COMPARE_OP_(<);
_XVSTATEPAIR_COMPARE_OP_(>);
_XVSTATEPAIR_COMPARE_OP_(<=);
_XVSTATEPAIR_COMPARE_OP_(>=);
_XVSTATEPAIR_COMPARE_OP_(==);

#include <XVInteractive.h>

template <class IMTYPE, class IMTYPEINTERNAL, class STATETYPE>
class XVFeature : public XVNode {

 protected:

  STATETYPE prevState;
  STATETYPE currentState;
  IMTYPEINTERNAL warped;

  bool warpUpdated;

 public:

  typedef STATETYPE STATE;
  typedef IMTYPE IMAGE;
  typedef IMTYPEINTERNAL IMAGE_INTERNAL;

  XVFeature() : warpUpdated(false) {}
  XVFeature(STATE init) : 
    currentState(init), prevState(init), warpUpdated(false) {}

  virtual ~XVFeature() {}
  
  virtual void initState(STATE init) { 
    currentState = init; prevState = currentState;
  }

  virtual IMTYPEINTERNAL warp(const IMTYPE &) = 0;

  virtual const STATE & step(const IMTYPE &) = 0;
  
  // sets the state of this feature, then finds the next state from there
  virtual const STATE & step(const IMTYPE & im, const STATE & st){

    currentState = st;
    return this->step(im);    
  };

  virtual const STATE & getCurrentState(){ return currentState; }
  virtual const STATE & getPrevState(){ return prevState; }
  virtual STATE diffState(){ return currentState - prevState; }

  // deprecated. for backwards compatability.  
  virtual STATE getState() { return currentState; }
  virtual void setState( const STATE& s) 
    { prevState = currentState; currentState = s; }

  virtual const STATE & interactiveInit(XVInteractive &, const IMTYPE &) = 0;
  //virtual void show(XVDrawable &,float scale=1.0) = 0;

  IMTYPEINTERNAL warpedImage() { return warped; }
};

#endif
