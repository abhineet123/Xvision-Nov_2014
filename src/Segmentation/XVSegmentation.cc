// *** BEGIN_XVISION2_COPYRIGHT_NOTICE ***
// *** END_XVISION2_COPYRIGHT_NOTICE ***

// 
// XVSegmentation.h
//
// Samuel Lang
// 9.21.00
//

#include <XVSegmentation.h>

template <class T, class Y>
XVSegmentation<T,Y>::~XVSegmentation(){

  if(histogram) delete[] histogram;
  if(lookup) delete lookup;
};

template <class T, class Y>
void XVSegmentation<T,Y>::segment(const XVImageBase<T> & src, XVImageBase<Y> & targ){

  XVImageIterator<T>  srcIter(src);
  targ.resize(src.Width(), src.Height());
  XVImageWIterator<Y> targIter(targ);

  bzero(histogram, histArraySize * sizeof(u_int));
  for(; !srcIter.end(); ++srcIter, ++targIter){
    histogram[*targIter = (*lookup)[*srcIter]]++;
  }
};

template <class T, class Y>
void XVSegmentation<T,Y>::findCentroid(const XVImageScalar<Y> & image, 
				       XVPosition & centroid,
				       int &count, bool (*pf) (const Y) ){
  int sumx, sumy;
  sumx = sumy = 0;

  if(pf == NULL){ pf = SEGFUNC; }

  XVImageIterator<Y> iter(image);
  for(; !iter.end(); ++iter){

    if(pf(*iter)){
      sumx += iter.currposx();
      sumy += iter.currposy();
      ++(count);
    }
  }
  centroid = XVPosition((int)round(sumx / count), (int)round(sumy / count));
};


template <class T, class Y>
XVRectangleBlob & 
XVSegmentation<T,Y>::findBoundingBox(const XVImageBase<T> & image, 
				     XVRectangleBlob & box, 
				     bool (*pf) (const Y) ){

  vector<XVRectangleBlob>  regions;
  XVImageScalar<Y> temp_image(image.Width(),image.Height());
  int size, tmp_size;

  segment(image,temp_image);
  regionGrow(temp_image,regions,pf);
  size=0;
  for(vector<XVRectangleBlob>::iterator it=regions.begin(); 
                                        it!=regions.end();++it)
  {
    tmp_size=it->Width()*it->Width()+it->Height()*it->Height();
    if(size<tmp_size)
    {
       box=*it; size=tmp_size;
    }
  } 
  return box;
};


template <class T, class Y>
double 
XVSegmentation<T,Y>::percentError(const XVImageBase<T> & image,
				  bool (*pf) (const Y) ){

  if(pf != NULL){ this->SEGFUNC = pf; }

  int error = 0;

  XVImageIterator<T> iter(image);

  for(; !iter.end(); ++iter){
    if(!this->SEGFUNC((*lookup)[*iter])){
      ++error;
    }
  }

  return (double)error*100.0 / (image.Width() * image.Height());
}

template <class T, class Y>
void XVSegmentation<T,Y>::regionGrow(const XVImageScalar<Y> & image, 
				     vector<XVRectangleBlob> & regions, 
				     bool (*pf) (const Y) ,
				     int padding    ,
				     int maxRegions ,
				     float ratio    ){

  float mx, my, m2x, m2y, sx, sy, s2x, s2y;
  int ulX, ulY, lrX, lrY, MulX, MulY, MlrX, MlrY;
  int dest, merge, found, count = 0;
  
  if(pf == NULL){ pf = SEGFUNC; }

  regions = vector<XVRectangleBlob>();
  typedef vector<XVRectangleBlob>::iterator IT;

  XVImageIterator<Y> iter(image);

  for(; !iter.end(); ++iter){
    
    if(pf((*iter))){

      found = 0;

      for(IT regionIter = regions.begin(); regionIter != regions.end(); ++regionIter){
	
	ulX = regionIter->PosX(); ulY = regionIter->PosY();
	lrX = regionIter->PosX() + regionIter->Width(); 
	lrY = regionIter->PosY() + regionIter->Height();

	mx = ((float)(ulX + lrX)) / 2;
	my = ((float)(ulY + lrY)) / 2;
	
	if(fabs(mx-iter.currposx())<fabs(ulX - mx)+padding &&
	   fabs(my-iter.currposy())<fabs(ulY - my)+padding) {
	  
	  if(ulX > iter.currposx())
	    ulX = iter.currposx();	  
	  if(lrX < iter.currposx()) 
	    lrX = iter.currposx();
	  if(ulY > iter.currposy()) 
	    ulY = iter.currposy();
	  if(lrY < iter.currposy())
	    lrY = iter.currposy();
	  found = 1;
	  regionIter->reposition(ulX, ulY);
	  regionIter->resize(lrX - ulX, lrY - ulY);
	}
      }

      if(!found){
	if(regions.size() < maxRegions){

	  regions.push_back(XVRectangleBlob(XVSize(1,1), 
					  XVPosition(iter.currposx(), iter.currposy())));
	}else{
#ifdef DEBUG
	  cerr << " too many regions" << endl;
#endif
	}
      }
    }
  }

  // merge regions
  for(IT regionIter = regions.begin(); regionIter != regions.end(); ++regionIter){

    ulX = regionIter->PosX(); ulY = regionIter->PosY();
    lrX = regionIter->PosX() + regionIter->Width();
    lrY = regionIter->PosY() + regionIter->Height();

    m2x = ((float)(ulX + lrX)) / 2;
    m2y = ((float)(ulY + lrY)) / 2;
    s2x = ((float)regionIter->Width())  / 2;
    s2y = ((float)regionIter->Height()) / 2;

    IT mergeRegionIter = regionIter;
    for(++mergeRegionIter; mergeRegionIter != regions.end(); ++mergeRegionIter){

      MulX = mergeRegionIter->PosX(); MulY = mergeRegionIter->PosY();
      MlrX = mergeRegionIter->PosX() + mergeRegionIter->Width();
      MlrY = mergeRegionIter->PosY() + mergeRegionIter->Height();
      
      mx = ((float)(MulX + MlrX)) / 2; my = ((float)(MulY + MlrY)) / 2;
      sx = ((float)mergeRegionIter->Width()) / 2;
      sy = ((float)mergeRegionIter->Height()) / 2;
      
      if(fabs(m2x-mx)<s2x+sx && fabs(m2y-my)<s2y+sy){
	
	if(ulX > MulX) ulX = MulX;
	if(lrX < MlrX) lrX = MlrX;
	if(ulY > MulY) ulY = MulY;
	if(lrY < MlrY) lrY = MlrY;

	regionIter->reposition(ulX, ulY);
	regionIter->resize(lrX - ulX, lrY - ulY);
	regions.erase(mergeRegionIter);
	--mergeRegionIter;
      }
    }
  }
};

template class XVSegmentation<XV_RGB15,  bool>;
template class XVSegmentation<XV_RGB16,  bool>;
template class XVSegmentation<XV_RGB24,  bool>;
template class XVSegmentation<XV_RGBA32, bool>;

template class XVSegmentation<XV_RGB15,  u_char>;
template class XVSegmentation<XV_RGB16,  u_char>;
template class XVSegmentation<XV_RGB24,  u_char>;
template class XVSegmentation<XV_RGBA32, u_char>;

template class XVSegmentation<XV_RGB15,  u_short>;
template class XVSegmentation<XV_RGB16,  u_short>;
template class XVSegmentation<XV_RGB24,  u_short>;
template class XVSegmentation<XV_RGBA32, u_short>;

template class XVSegmentation<u_char,  u_char>;
template class XVSegmentation<u_short, u_char>;

template class XVSegmentation<u_char,  u_short>;
template class XVSegmentation<u_short, u_short>;

template class XVSegmentation<int, int>;
