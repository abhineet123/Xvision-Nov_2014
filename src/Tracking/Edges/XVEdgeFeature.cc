#include <XVEdgeFeature.h>

template <class PIX_TYPE, class EDGETYPE>
XVImageScalar<PIX_TYPE>
XVEdgeFeature<PIX_TYPE, EDGETYPE>::warp(const XVImageScalar<PIX_TYPE> & image) {

  warped = warpRect(image, (XVPosition) currentState.state.center,
		    XVSize(searchWidth, (int)rint(currentState.state.length)), 
		    currentState.state.angle);
  warpUpdated = true;
  return warped;
};

template <class PIX_TYPE, class EDGETYPE>
const XVLineState &
XVEdgeFeature<PIX_TYPE, EDGETYPE>::step(const XVImageScalar<PIX_TYPE> & image) {

  if(!warpUpdated) this->warp(image);
  prevState = currentState;

  XVOffset delta = edge.find(warped);  

  XVAffineMatrix rotMat(currentState.state.angle);
  XV2Vec<double> dirVec = rotMat * XV2Vec<double>(0, -1);
  XV2Vec<double> perpVec(-dirVec.y(), dirVec.x());
  currentState.state.center += (perpVec * (delta.x));
  currentState.state.angle -= delta.angle;
  currentState.error = delta.val;

  warpUpdated = false;
  return currentState;
};

#include <config.h>
#include <XVWindowX.h>

template <class PIX_TYPE, class EDGETYPE>
const XVLineState &
XVEdgeFeature<PIX_TYPE, EDGETYPE>::
interactiveInit(XVInteractive & window,
		const XVImageScalar<PIX_TYPE> & image) {
  
  XVPosition ends[2];
  window.selectLine(ends[0], ends[1]);
  XV2Vec<double> diffVec = ends[1] - ends[0];
  XV2Vec<double> halfVec = diffVec / ((double)2);
  currentState.state.center = (XV2Vec<double>)(ends[0] + halfVec);
  if(diffVec.x() < 0){
    currentState.state.angle = -acos((XV2Vec<double>(0, -1) * diffVec) 
				    / diffVec.length());
  }else{
    currentState.state.angle = acos((XV2Vec<double>(0, -1) * diffVec) 
				      / diffVec.length());
  }
  currentState.error = 0;
  currentState.state.length = diffVec.length();
  prevState = currentState;
  return currentState;
};

template <class PIX_TYPE, class EDGETYPE>
void XVEdgeFeature<PIX_TYPE, EDGETYPE>::show(XVDrawable & window,float
scale) {

  XV2Vec<double> ends[2];
  currentState.state.endpoints(ends);
  window.drawLine(ends[0], ends[1]);
};

#include <XVTracker.h>
#include <XVGroupTracker.h>
#include <XVImageScalar.h>

#define _REGISTER_XVEDGEFEATURE_(_PIX_, _EDGE_) \
template class XVEdgeFeature<_PIX_, _EDGE_ >;

#define _REGISTER_XVEDGEFEATURE_SCALAR_(_EDGE_) \
_REGISTER_XVEDGEFEATURE_(int, _EDGE_<int>);

_REGISTER_XVEDGEFEATURE_SCALAR_(XVEdge);
_REGISTER_XVEDGEFEATURE_SCALAR_(XVMaxEdge);
_REGISTER_XVEDGEFEATURE_SCALAR_(XVGeneralEdge);
_REGISTER_XVEDGEFEATURE_SCALAR_(XVShortEdge);
