// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#ifndef _XVTRACKER_ICC_
#define _XVTRACKER_ICC_

template <class SRC, class FEATURE>
typename FEATURE::STATE XVTracker<SRC, FEATURE>::nextState(bool next) throw (XVTrackerException) {
  
  if(next)source.next_frame_continuous();

  typename FEATURE::STATE st;
  for(int i = 0; i < stepsPerFrame; ++i){
    st = feature.step((typename FEATURE::IMAGE)(source.current_frame()));
  }
  
  return st;
};

template <class SRC, class FEATURE>
void XVTracker<SRC, FEATURE>::track() throw(XVTrackerException) {
    
  while(1){
    
    source.next_frame_continuous();

    for(int i = 0; i < stepsPerFrame; ++i){
      feature.step((typename FEATURE::IMAGE)(source.current_frame()));
    }
  }
};

template <class SRC, class WIN, class FEATURE>
typename FEATURE::STATE XVDisplayTracker<SRC, WIN, FEATURE>::init(){

  source.next_frame_continuous();
  window.CopySubImage(source.current_frame());
  window.swap_buffers();
  window.flush();

  typename FEATURE::STATE s = feature.interactiveInit(window, 
              (typename FEATURE::IMAGE)(source.current_frame()));
  window.setCOPY();
  return s;
};

template <class SRC, class WIN, class FEATURE>
void XVDisplayTracker<SRC, WIN, FEATURE>::track() throw(XVTrackerException) {

  while(1){
    
    source.next_frame_continuous();

    for(int i = 0; i < stepsPerFrame; ++i){
      feature.step((typename FEATURE::IMAGE)(source.current_frame()));
    }

    window.CopySubImage(source.current_frame());
    
    feature.show(window);
    
    window.swap_buffers();
    window.flush();
  }
};

template <class SRC, class WIN, class FEATURE>
typename FEATURE::STATE XVDisplayTracker<SRC, WIN, FEATURE>::nextState(bool next) throw (XVTrackerException) {
  
  if(next)source.next_frame_continuous();
  
  typename FEATURE::STATE st;
  for(int i = 0; i < stepsPerFrame; ++i){
    st = feature.step((typename FEATURE::IMAGE)(source.current_frame()));
  }

  window.CopySubImage(source.current_frame());
  
  feature.show(window);

  window.swap_buffers();
  window.flush();
  
  return st;
};

#endif
