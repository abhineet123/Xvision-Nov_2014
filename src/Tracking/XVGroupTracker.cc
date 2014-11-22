// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVGROUPTRACKER_ICC_
#define _XVGROUPTRACKER_ICC_

template <class SRC, class FEATURE>
vector<typename FEATURE::STATE >
XVGroupTracker<SRC, FEATURE>::nextState() throw (XVTrackerException) {
  
  source.next_frame_continuous();

  vector<STATE > states(features.size());
  for(int i = 0; i < stepsPerFrame; ++i){
    for(int f = 0; f < features.size(); ++f){
      states[f] = features[f]->step(source.current_frame());
    }
  }
  
  return states;
};

template <class SRC, class FEATURE>
void XVGroupTracker<SRC, FEATURE>::track() throw(XVTrackerException) {
    
  while(1){
    
    source.next_frame_continuous();

    for(int i = 0; i < stepsPerFrame; ++i){
      for(int f = 0; f < features.size(); ++f){
	features[f]->step(source.current_frame());
      }
    }
  }
};

template <class SRC, class WIN, class FEATURE>
vector<typename FEATURE::STATE >
XVDisplayGroupTracker<SRC, WIN, FEATURE>::init(){

  source.next_frame_continuous();
  window.CopySubImage(source.current_frame());
  window.swap_buffers();
  window.flush();

  vector<typename FEATURE::STATE > states(features.size());
  for(int i = 0; i < features.size(); ++i){
    states[i] = features[i]->interactiveInit(window, source.current_frame());
  }

  window.setCOPY();
  return states;
};

template <class SRC, class WIN, class FEATURE>
vector<typename FEATURE::STATE >
XVDisplayGroupTracker<SRC, WIN, FEATURE>::init(XVSize & size) {

  source.next_frame_continuous();
  window.CopySubImage(source.current_frame());
  window.swap_buffers();
  window.flush();

  vector<typename FEATURE::STATE > states(features.size());
  for(int i = 0; i < features.size(); ++i){
    states[i] = features[i]->interactiveInit(window, size, source.current_frame());
  }

  window.setCOPY();
  return states;
};

template <class SRC, class WIN, class FEATURE>
void XVDisplayGroupTracker<SRC, WIN, FEATURE>::track() 
throw(XVTrackerException) {

  while(1){
    
    source.next_frame_continuous();

    for(int i = 0; i < stepsPerFrame; ++i){
      for(int f = 0; f < features.size(); ++f){
	features[f]->step(source.current_frame());
      }
    }

    window.CopySubImage(source.current_frame());

    for(int f = 0; f < features.size(); ++f){      
      features[f]->show(window);
    }
    
    window.swap_buffers();
    window.flush();
  }
};

template <class SRC, class WIN, class FEATURE>
vector<typename FEATURE::STATE >
XVDisplayGroupTracker<SRC, WIN, FEATURE>::nextState() 
throw (XVTrackerException) {
  
  source.next_frame_continuous();
  
  vector<STATE > states(features.size());
  for(int i = 0; i < stepsPerFrame; ++i){
    for(int f = 0; f < features.size(); ++f){
      states[f] = features[f]->step(source.current_frame());
    }
  }

  window.CopySubImage(source.current_frame());
  
  for(int f = 0; f < features.size(); ++f){    
    features[f]->show(window);
  }

  window.swap_buffers();
  window.flush();
  
  return states;
};

#endif
