CUR_DEPTH = ..

TOOLS        = XVMatrix.h XVMatrix.icc XVList.h XVTools.h XVException.h \
	       XVMacros.h XVAffineWarp.h XVAffineWarp.icc XVGeometry.h \
	       XVOmniWarper.h XVRectRaster.h XVLookupTable.h XVLookupTable.icc\
	       camera_config.h XVStereoRectify.h XVStereoRectifyTypes.h

IMAGES       = XVImageBase.h XVImageRGB.h XVImageRGB.icc XVImageScalar.h \
	       XVImageIterator.h XVImageFilters.icc XVPixel.h XVColorImage.h \
	       XVImageYUV.h XVImageYUV.icc XVImageYUV422.h XVImageBase.icc XVImageScalar.icc \
	       XVImageHSV.h XVImageIO.h XVImageIO.icc \
	XVImageYCbCr.h	XVMaskedImage.h

CONSOLES     = XVWindow.h XVDrawable.h XVInteractive.h XVWindowX.h \
               XVRemoteWindowX.h

DEVICES      = XVVideo.h DVFrame.h \
	       DV1394.h XVDig1394.h XVFlea2G.h \
	       VideoManager.h Meteor.h \
	       StereoVidere.h Videre.h Videre.icc XVPv.h XVAVI.h\
               XVVideo.h XVImageSeq.h XVMpeg.h XVBt8x8.h XVV4L2.h\
	       XVPwc.h video1394.h ieee1394-ioctl.h

DRIVERS      = jpeglib.h jconfig.h jmorecfg.h tiffio.h tiff.h mpeg.h \
	       ioctl_meteor.h

TRACKING     = XVFeature.h XVTracker.h XVTracker.icc \
               XVGroupTracker.h XVGroupTracker.icc \
	       XVPattern.h XVPattern.icc XVEdgeFeature.h \
	       XVBlobs.h XVBlobFeature.h XVSSD.h

SEGMENTATION = XVSegmentation.h \
	       XVColorSeg.h XVMotionSeg.h XVScalarSeg.h

FRP          = Functoid.h Lambda.h Collectable.h Behavior.h Event.h Task.h \
	       Image.h Track.h Scout.h \
	       Nclient.h

HEADERS	  = $(TOOLS) $(IMAGES) $(CONSOLES) $(DEVICES) $(DRIVERS) \
	    $(TRACKING) $(SEGMENTATION) $(FRP)

SUFFIXES := .h .icc

install: $(HEADERS)
	ln -sf ../src/drivers/libdv

clean: ;	rm -f $(HEADERS) libdv

%.h: ;	ln -s `find $(CUR_DEPTH)/src -name $@|grep -v CVS` $@
%.icc: ;	ln -s `find $(CUR_DEPTH)/src -name $@|grep -v CVS` $@

