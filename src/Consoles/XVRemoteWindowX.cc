// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

#include <config.h>
#include <math.h>

#ifdef HAVE_LIBX11

#include <XVRemoteWindowX.h>

template<class T>
void XVRemoteWindowX<T>::init() {
  supervised = false ;
  data = new Data( worker );
  next = new Requests() ;
  pthread_cond_init (&(data->worker_signal), NULL);
  pthread_mutex_init (&(data->signal_mutex), NULL);
  fetch_size();
}

template<class T>
void XVRemoteWindowX<T>::signal_worker()
{
//#ifdef NEVER
  pthread_mutex_lock (&(data->signal_mutex));
  pthread_cond_signal (&(data->worker_signal));
  pthread_mutex_unlock (&(data->signal_mutex));
  //cerr << "(XVRemoteWindowX::signal_worker) NOTICE: sent signal!\n";
  //cin.get();
//#endif
}

template<class T>
XVRemoteWindowX<T>::XVRemoteWindowX( XVDrawWindowX<T>& xwin,
					float scaleX, float scaleY )
 : XVWindow<T>( (int)rint(xwin.Width()*scaleX),
		(int)rint(xwin.Height()*scaleY) ) {
  worker = &xwin ;
  to_free = false ;
  factorX = scaleX ;
  factorY = scaleY ;
  init();
}

template<class T>
XVRemoteWindowX<T>::XVRemoteWindowX( const XVImageRGB<T> & im,
 int px, int py, char * title,
 int event_mask, char * disp, int num_buf, int double_buf, int function )
 : XVWindow<T>( im.Width(), im.Height() ) {
  factorX = 1.0 ;
  factorY = 1.0 ;
  worker = new XVDrawWindowX<T>( (int)rint(im.Width()*factorX),
				 (int)rint(im.Height()*factorY),
				 px, py, title, event_mask, disp,
				 num_buf, double_buf, function );
  to_free = true ;
  init();
}

template<class T>
XVRemoteWindowX<T>::XVRemoteWindowX( const XVImageRGB<T> & im,
 float scaleX, float scaleY, int px, int py, char * title,
 int event_mask, char * disp, int num_buf, int double_buf, int function )
 : XVWindow<T>( (int)rint(im.Width()*scaleX),
		(int)rint(im.Height()*scaleY) ) {
  factorX = scaleX ;
  factorY = scaleY ;
  worker = new XVDrawWindowX<T>( (int)rint(im.Width()*factorX),
				 (int)rint(im.Height()*factorY),
				 px, py, title, event_mask, disp,
				 num_buf, double_buf, function );
  to_free = true ;
  init();
}

template<class T>
XVRemoteWindowX<T>::XVRemoteWindowX( int w, int h,
 int px, int py, char * title,
 int event_mask, char * disp, int num_buf, int double_buf, int function )
  : XVWindow<T>( w, h ) {
  factorX = 1.0 ;
  factorY = 1.0 ;
  worker = new XVDrawWindowX<T>( (int)rint(w*factorX),
				 (int)rint(w*factorY),
				 px, py, title, event_mask, disp,
				 num_buf, double_buf, function );
  to_free = true ;
  init();
}
template<class T>
XVRemoteWindowX<T>::XVRemoteWindowX( int w, int h,
 float scaleX, float scaleY, int px, int py, char * title,
 int event_mask, char * disp, int num_buf, int double_buf, int function )
  : XVWindow<T>( (int)rint(w*scaleX), (int)rint(h*scaleY) ) {
  factorX = scaleX ;
  factorY = scaleY ;
  worker = new XVDrawWindowX<T>( (int)rint(w*factorX),
				 (int)rint(w*factorY),
				 px, py, title, event_mask, disp,
				 num_buf, double_buf, function );
  to_free = true ;
  init();
}

template<class T>
XVRemoteWindowX<T>::~XVRemoteWindowX() {
  data->lock_next();
  data->die = true ;
  data->unlock_next();
  if( supervised ) {
    pthread_join( supervisor, 0 );
  }
  if( to_free ) {
    delete worker ;
  }
  delete data ;
}

template<class T>
void XVRemoteWindowX<T>::fetch_size() {
  width = worker->Width() ;
  height = worker->Height() ;
  posx = worker->PosX();
  posy = worker->PosY();
}

template<class T>
void XVRemoteWindowX<T>::setImages( XVImageRGB<T>* frames, int count ) {
  worker->setImages( frames, count );
}

template<class T>
void XVRemoteWindowX<T>::CopyImage( int which, u_short flip ) {
  worker->CopyImage( which, flip );
  fetch_size();
}

template<class T>
void XVRemoteWindowX<T>::ClearWindow(void) {
  Drawing d( Drawing::ClearWindow, (char*)DEFAULT_COLOR );
  next->drawings.push_front( d );
  signal_worker();
}

template<class T>
void XVRemoteWindowX<T>::CopySubImage( const XVImageRGB<T>& image, bool flip ) {
  Drawing d( Drawing::DrawImage, (char*)DEFAULT_COLOR );
  d.x1 = flip ;
  next->drawings.push_front( d );
  resample( factorX, factorY, image, next->image );
  signal_worker();
}

template<class T>
void XVRemoteWindowX<T>::swap_buffers(void) {
  Drawing d( Drawing::SwapBuffer, (char*)DEFAULT_COLOR );
  next->drawings.push_front( d );
}

template<class T>
void XVRemoteWindowX<T>::flush(void) {
  Requests * carrier ;
  next->flush = true ;
  next->drawings.reverse();
  data->lock_next();
  carrier = data->next ;
  data->next = next ;
  next = carrier ;
  data->unlock_next();
  next->clear();
  if( ! supervised ) {
    supervised = true ;
    pthread_attr_t attr ;
    pthread_attr_init( &attr );
    //cerr << "(XVRemoteWindowX::flush) NOTICE: pthread created.\n";
    pthread_create( &supervisor, &attr, supervising, data );
    pthread_attr_destroy( &attr );
  }
  fetch_size();
  signal_worker();
}

template<class T>
XVImageRGB<T> XVRemoteWindowX<T>::getDisplayedImage( int px, int py,
						     int w, int h ) {
  return worker->getDisplayedImage();
}

template<class T>
XVImageRGB<T> XVRemoteWindowX<T>::getLastImage(void) {
  XVImageRGB<T> lastImage ;
  data->lock_last();
  resample( 1.0, 1.0, data->last->image, lastImage );
  data->unlock_last();
  return lastImage ;
}

template<class T>
XVImageRGB<T> XVRemoteWindowX<T>::getEventsImage(void) {
  XVImageRGB<T> eventsImage ;
  data->lock_last();
  resample( 1.0, 1.0, data->pre_last->image, eventsImage );
  data->unlock_last();
  return eventsImage ;
}

template<class T>
int XVRemoteWindowX<T>::check_events( int * ret_field ) {
  int type = 0 ;
  data->lock_last();
  if( ! data->events.empty() ) {
    Event& event = data->events.front();
    type = event.type ;
    if( ret_field ) {
      memcpy( ret_field, event.args, Event::num_args * sizeof(int) );
    }
    data->events.pop();
  }
  data->unlock_last();
  return type ;
}

template<class T>
int XVRemoteWindowX<T>::drawPoint( int x, int y, XVDrawColor c ) {
  Drawing d( Drawing::DrawPoint, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::drawLine( int x1, int y1, int x2, int y2,
				  XVDrawColor c,int line_width ) {
  Drawing d( Drawing::DrawLine, c );
  d.x1 = (int)rint( x1 * factorX );
  d.y1 = (int)rint( y1 * factorY );
  d.x2 = (int)rint( x2 * factorX );
  d.y2 = (int)rint( y2 * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::drawRectangle( int x, int y, int w, int h,
				       XVDrawColor c ) {
  Drawing d( Drawing::DrawRectangle, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  d.x2 = (int)rint( w * factorX );
  d.y2 = (int)rint( h * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::fillRectangle( int x, int y, int w, int h,
				       XVDrawColor c ) {
  Drawing d( Drawing::FillRectangle, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  d.x2 = (int)rint( w * factorX );
  d.y2 = (int)rint( h * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::drawEllipse( int x, int y, int w, int h,
				     XVDrawColor c ) {
  Drawing d( Drawing::DrawEllipse, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  d.x2 = (int)rint( w * factorX );
  d.y2 = (int)rint( h * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::fillEllipse( int x, int y, int w, int h,
				     XVDrawColor c ) {
  Drawing d( Drawing::FillEllipse, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  d.x2 = (int)rint( w * factorX );
  d.y2 = (int)rint( h * factorY );
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}
template<class T>
int XVRemoteWindowX<T>::drawString( int x, int y, char* string, int length,
				    XVDrawColor c ) {
  Drawing d( Drawing::DrawString, c );
  d.x1 = (int)rint( x * factorX );
  d.y1 = (int)rint( y * factorY );
  d.x2 = length ;
  d.data = new char[length+1];
  memcpy( d.data, string, length );
  d.data[length] = '\0' ;
  next->drawings.push_front( d );
  signal_worker();
  return 0 ;
}

template<class T>
void XVRemoteWindowX<T>::Requests::clear() {
  flush = false ;
  for( OneDrawing aDrawing = drawings.begin() ;
       aDrawing != drawings.end(); ++ aDrawing ) {
    delete aDrawing->data ;
  }
  drawings.clear();
}

template<class T>
XVRemoteWindowX<T>::Requests::Requests() : image(1,1) {
  clear();
}
template<class T>
XVRemoteWindowX<T>::Requests::~Requests() {
  clear();
}

template<class T>
XVRemoteWindowX<T>::Data::Data( XVDrawWindowX<T>* w ) {
  worker = w ;
  pthread_mutexattr_t attr ;
  pthread_mutexattr_init( &attr );
  //pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_ERRORCHECK_NP );
  pthread_mutex_init( &access_last, &attr );
  pthread_mutex_init( &access_next, &attr );
  pthread_mutexattr_destroy( &attr );
  last = new Requests();
  current = new Requests();
  next = new Requests();
  pre_last = new Requests();
  die = false ;
}
template<class T>
XVRemoteWindowX<T>::Data::~Data() {
  pthread_mutex_destroy( &access_last );
  pthread_mutex_destroy( &access_next );
  delete last ;
  delete current ;
  delete next ;
}

template<class T>
void * XVRemoteWindowX<T>::supervising( void * obj ) {
  Data* data = reinterpret_cast<Data*>(obj) ;
  Requests * carrier ;
  OneDrawing aDrawing ;
  Event event ;
  for(;;) {
    // test next for ending
    data->lock_next();
    if( data->die ) {
      data->unlock_next();
      break;
    }
    // move next to current
    carrier = data->current ;
    data->current = data->next ;
    data->next = carrier ;
    data->next->clear();
    data->unlock_next();
    // sending current
    for( aDrawing = data->current->drawings.begin() ;
	 aDrawing != data->current->drawings.end() ; ++ aDrawing ) {
      switch( aDrawing->type ) {
      case Drawing::SwapBuffer:
	data->worker->swap_buffers();
	break ;
      case Drawing::ClearWindow:
	data->worker->ClearWindow();
	break ;
      case Drawing::DrawImage:
	data->worker->CopySubImage( data->current->image, aDrawing->x1 );
	break ;
      case Drawing::DrawPoint:
	data->worker->drawPoint( aDrawing->x1, aDrawing->y1, aDrawing->color );
	break ;
      case Drawing::DrawLine:
	data->worker->drawLine( aDrawing->x1, aDrawing->y1,
				aDrawing->x2, aDrawing->y2, aDrawing->color );
	break ;
      case Drawing::DrawRectangle:
	data->worker->drawRectangle( aDrawing->x1, aDrawing->y1,
				aDrawing->x2, aDrawing->y2, aDrawing->color );
	break ;
      case Drawing::FillRectangle:
	data->worker->fillRectangle( aDrawing->x1, aDrawing->y1,
				aDrawing->x2, aDrawing->y2, aDrawing->color );
	break ;
      case Drawing::DrawEllipse:
	data->worker->drawEllipse( aDrawing->x1, aDrawing->y1,
				aDrawing->x2, aDrawing->y2, aDrawing->color );
	break ;
      case Drawing::FillEllipse:
	data->worker->fillEllipse( aDrawing->x1, aDrawing->y1,
				aDrawing->x2, aDrawing->y2, aDrawing->color );
	break ;
      case Drawing::DrawString:
	data->worker->drawString( aDrawing->x1, aDrawing->y1,
			      aDrawing->data, aDrawing->x2, aDrawing->color );
	break ;
      default:
	break ;
      }
    }
    if( data->current->flush ) {
      data->worker->flush();
    }
    // check events
    data->lock_last();
    while( event.type = data->worker->check_events( event.args ) ) {
      data->events.push( event );
    }
    // copy current to last ;
    if( data->current->flush ) {
      carrier = data->pre_last ;
      data->pre_last = data->last ;
      data->last = data->current ;
      data->current = carrier ;
    }
    data->unlock_last();

    data->lock_next();
    if( data->next->drawings.empty() && !data->next->flush/*&& data->next->image.SizeX()==0*/) {
      data->unlock_next();
      pthread_mutex_lock (&(data->signal_mutex));
      pthread_cond_wait (&(data->worker_signal), &(data->signal_mutex));
      pthread_mutex_unlock (&(data->signal_mutex));
    }
    else
    	data->unlock_next();

  }
  return 0 ;
}

template class XVRemoteWindowX<XV_RGB15>;
template class XVRemoteWindowX<XV_RGB16>;
template class XVRemoteWindowX<XV_RGB24>;
template class XVRemoteWindowX<XV_RGBA32>;

#endif //HAVE_LIBX11
