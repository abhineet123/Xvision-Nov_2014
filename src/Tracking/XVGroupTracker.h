// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

/**
 * XVGroupTracker.h
 * 
 * @author Sam Lang - 4.15.01.
 * @version $Id: XVGroupTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 */

#ifndef _XVGROUPTRACKER_H_
#define _XVGROUPTRACKER_H_

#define DEFAULT_ITERATIONS 1

#include <XVInteractive.h>
#include <XVDrawable.h>
#include <XVException.h>
#include <XVFeature.h>

#include <vector>

/**
 * XVGroupTracker
 *
 * @author Sam Lang
 * @version $Id: XVGroupTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
 *
 * XVGroupTracker is just like XVTracker, except you
 * can have multiple features.
 *
 * @see XVTracker
 * @see XVVideo
 * @see XVFeature
 */
template <class SRCTYPE, class FEATURETYPE>
class XVGroupTracker {

 protected:
  
  SRCTYPE & source;
  vector<FEATURETYPE *> features;

  int stepsPerFrame;
  int frameNum;

 public:

  typedef typename FEATURETYPE::STATE STATE;

  XVGroupTracker(SRCTYPE & s, FEATURETYPE & f, int pf = DEFAULT_ITERATIONS) 
    : source(s), stepsPerFrame(pf), frameNum(0) { features.push_back(&f); };

  virtual vector<STATE > init(vector<STATE > & initStates) { 
    for(int i = 0; i < initStates.size(); ++i){
      features[i]->initState(initStates[i]);
    }
    return initStates;
  }

  virtual void track() throw (XVTrackerException);
  virtual vector<STATE > nextState() 
    throw (XVTrackerException);

  void addFeature(FEATURETYPE & f){ features.push_back(&f); };
  void removeFeature(int i){ delete features[i]; features.erase(features.begin() + i); };
  vector<FEATURETYPE *> & getFeatures(){ return features; };
};

/**
 * XVDisplayTracker
 *
 * @author Sam Lang
 * @version $Id: XVGroupTracker.h,v 1.1.1.1 2008/01/30 18:43:45 burschka Exp $
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
class XVDisplayGroupTracker : public XVGroupTracker<SRCTYPE, FEATURETYPE> {
 protected:
  using XVGroupTracker<SRCTYPE,FEATURETYPE>::source ;
  using XVGroupTracker<SRCTYPE,FEATURETYPE>::features ;
  using XVGroupTracker<SRCTYPE,FEATURETYPE>::stepsPerFrame ;


 protected:

  WINTYPE & window;

 public:

  typedef typename FEATURETYPE::STATE STATE;

  XVDisplayGroupTracker(SRCTYPE & s, WINTYPE & w, 
			FEATURETYPE & f, int pf = DEFAULT_ITERATIONS) 
    : XVGroupTracker<SRCTYPE, FEATURETYPE>(s, f, pf), window(w) {}

  virtual vector<STATE > init();
  virtual vector<STATE> init(XVSize &);
  virtual void track() throw (XVTrackerException);
  virtual vector<STATE > nextState() 
    throw (XVTrackerException);
};

#include <XVGroupTracker.icc>

#endif
