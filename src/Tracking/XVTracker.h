// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVTracker.h
 *
 * @author Sam Lang - 2.02.01.
 * @version $Id: XVTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 */

#ifndef _XVTRACKER_H_
#define _XVTRACKER_H_

#define DEFAULT_ITERATIONS 1

#include <XVInteractive.h>
#include <XVDrawable.h>
#include <XVException.h>
#include <XVFeature.h>

/**
 * XVTracker
 *
 * @author Sam Lang
 * @version $Id: XVTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 * XVTracker implements the basic tracker.  Each
 * tracker has its own Video source, and a feature
 * or set of features to track.  Its templated
 * on the source and feature types, allowing for
 * any feature or source to be implemented seperately
 * and simply "dropped in" to the tracker.
 *
 * @see XVVideo
 * @see XVFeature
 */
template <class SRCTYPE, class FEATURETYPE>
class XVTracker {

  typedef typename FEATURETYPE::STATE STATE;

 protected:

  SRCTYPE     & source;
  FEATURETYPE & feature;
  
  int stepsPerFrame;
  int frameNum;

 public:

  XVTracker(SRCTYPE & s, FEATURETYPE & f, int pf = DEFAULT_ITERATIONS) 
    : source(s), feature(f), stepsPerFrame(pf), frameNum(0) {};
  
  virtual STATE init(STATE & initState) { feature.initState(initState); return feature.getState(); }
  virtual void track() throw (XVTrackerException);
  virtual STATE nextState(bool next=true) throw (XVTrackerException);
};

/**
 * XVDisplayTracker
 *
 * @author Sam Lang
 * @version $Id: XVTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 * The display tracker extends the tracking
 * structure to include a window to display
 * the video sequences, and the region
 * the tracker is tracking in the images.
 * As well as the features and video source,
 * XVDisplayTracker is templated on the window
 * type.
 * 
 * @see XVTracker
 * @see XVFeature
 * @see XVVideo
 * @see XVWindow
 */
template <class SRCTYPE, class WINTYPE, class FEATURETYPE>
class XVDisplayTracker : public XVTracker<SRCTYPE, FEATURETYPE> {
 protected:
  using XVTracker<SRCTYPE,FEATURETYPE>::source ;
  using XVTracker<SRCTYPE,FEATURETYPE>::feature ;
  using XVTracker<SRCTYPE,FEATURETYPE>::stepsPerFrame ;

 protected:

  WINTYPE & window;

 public:

  XVDisplayTracker(SRCTYPE & s, WINTYPE & w, FEATURETYPE & f, int pf = DEFAULT_ITERATIONS) 
    : XVTracker<SRCTYPE, FEATURETYPE>(s, f, pf), window(w) {}

  virtual typename FEATURETYPE::STATE init();
  virtual void track() throw (XVTrackerException);
  virtual typename FEATURETYPE::STATE nextState(bool next=true) throw (XVTrackerException);
};

#include <XVTracker.icc>

#endif
