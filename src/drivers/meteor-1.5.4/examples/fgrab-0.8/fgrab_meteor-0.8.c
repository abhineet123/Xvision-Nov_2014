/*
 *	Copyright (C) 1997 Heiko Teuber (heiko@pc10.pc.chemie.th-darmstadt.de).
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *	I have derived this program using Ian Reid's vu4 (ian@robots.oxford.
 *	ac.uk) and the square-widget of J. K. Ousterhost's book Programming
 *	in Tcl/Tk. Although most of the code in my program is new I
 *	used a few parts especially the shared memory routines out of vu4 with
 *	only little change. The copyright for these parts are therefore not
 *	changed by the above notice.	
 *
 *
 *	I hope you enjoy using this program. Please report any bugs and useful
 *	features you would like to find in this program to
 *	heiko@pc10.pc.chemie.th-darmstadt.de
 *
 *	Version 0.7.1
 *	Removed one Bug for single-frame capture (due to a hint given by
 *	Dan Mueth)
 *	Added yuv-packed mode (code of Ian Reid's mvid-program)
 *	Added command command line option for input_device, input_format 
 *	and yuv-mode
 *
 *	Version 0.8
 *	Removed one silly Bug from the new command line option of Version
 *	0.7.1.
 *	Added 15-bit and 8-bit single frame capture mode (code of Patrick
 *	Flynn's snap-program)
 *	Added mode for choosing the source for single captureing (meteor-card
 *	or screen)
 *
 *      Version 0.8.1
 *      Modified by Mark Sutton <mark.sutton@laitram.com> to compile and
 *      run successfully on glibc2 (aka. libc6) based Linux distributions.
 */

#include "fgrab_meteor-0.8.h"



static int FGrabReg = 0;
static int FGrabCountFps = 0;
static Tcl_AsyncHandler FGrabAsyncToken;

static struct meteor_geomet geo;
static struct meteor_frame_offset off;



static struct sigaction sigact = 
((struct sigaction){ sa_handler: FGrabGotFrame,
		     sa_mask: 0
/*		     sa_flags: SA_RESTART */ });

static struct sigaction sigalrm = 
((struct sigaction){ sa_handler: FGrabCountFrame,
		     sa_mask: SIGUSR2
/*		     sa_flags: SA_RESTART */ });


static void FGrabCountFrame( int signum ) {


  fprintf( stderr, "%d ", FGrabCountFps);
  FGrabCountFps = 0;
  if (FGrabReg & FPSCOUNT) {
    alarm(1);
  }
}

static void FGrabGotFrame( int signum ) {

  if (!(FGrabReg & ASYNCMARK)) {
    FGrabReg |= ASYNCMARK;
    Tcl_AsyncMark (FGrabAsyncToken);
  }
}


static int FGrabAsyncHandler( ClientData clientData, Tcl_Interp *interp,\
			      int code) {

  FGrab *fgrabPtr = (FGrab *) clientData;
  Tk_Window tkwin = fgrabPtr->tkwin;
  int borderWidth = fgrabPtr->borderWidth;
  char *src, *dest;
  unsigned int size;
  
  if (Tk_IsMapped (fgrabPtr->tkwin)) {
    
    src = fgrabPtr->mmbuf + off.frame_offset[0];
    dest = fgrabPtr->shmimage->data;
    size = fgrabPtr->video.rows * fgrabPtr->video.columns\
      * fgrabPtr->pixelsize;

    if (FGrabReg & PACKED) {
      for (src++; size--; src+=2)
	    *dest++ = *src;
    } else {

      /* continuous capture */
      memcpy(dest, src, size);
    }
    
    /* Draw screen onto display */
    XShmPutImage(fgrabPtr->display, Tk_WindowId\
		 (fgrabPtr->tkwin), fgrabPtr->gc,\
		 fgrabPtr->shmimage, 0, 0, borderWidth, borderWidth,\
		 Tk_Width (tkwin) - 2 * borderWidth,\
		 Tk_Height(tkwin) - 2 * borderWidth,\
		 False);
    /*  XSync(FGrabPtr->display, 0); */
  }
  FGrabReg &= ~ASYNCMARK;
  FGrabCountFps++;
  return code;
}



/*
 * FGrabConf[]
 * 
 * configure struct of Tk:	standart options
 */     

static Tk_ConfigSpec FGrabConf[] = {
 {TK_CONFIG_BORDER, "-background", "background", "Background", "#d9d9d9",\
   Tk_Offset (FGrab, bgBorder), TK_CONFIG_COLOR_ONLY,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_BORDER, "-background", "background", "Background", "white",\
   Tk_Offset (FGrab, bgBorder), TK_CONFIG_MONO_ONLY,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL, (char *) NULL,\
   0, 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL, (char *) NULL,\
   0, 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth", "1m",\
   Tk_Offset (FGrab, borderWidth), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_RELIEF, "-relief", "relief", "Relief", "raised",\
   Tk_Offset (FGrab, relief), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_STRING, "-yuv", "yuv", "Yuv", YUV_MODE,\
   Tk_Offset (FGrab, yuv), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_STRING, "-device", "device", "Device", INPUT_DEVICE,\
   Tk_Offset (FGrab, device), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_STRING, "-format", "format", "Format", INPUT_FORMAT,\
   Tk_Offset (FGrab, format), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL, (char *) NULL,\
   0, 0,\
   (Tk_CustomOption *) NULL}
};
 
/* 
 * FGrabVideoConf[] 
 *
 * configure struct of Tk:	video options
 */     

static Tk_ConfigSpec FGrabVideoConf[] = {
  {TK_CONFIG_INT, "-columns", "columns", "Columns", "768",\
   Tk_Offset (FGrab, video.columns), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-rows", "rows", "Rows", "576",\
   Tk_Offset (FGrab, video.rows), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-hue", "hue", "Hue", "20",\
   Tk_Offset (FGrab, video.hue), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-chcv", "chcv", "Chcv", "44",\
   Tk_Offset (FGrab, video.chcv), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-bright", "bright", "Bright", "148",\
   Tk_Offset (FGrab, video.bright), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-chroma", "chroma", "Chroma", "114",\
   Tk_Offset (FGrab, video.chroma), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-contrast", "contrast", "Contrast", "74",\
   Tk_Offset (FGrab, video.contrast), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-fps", "fps", "Fps", "12",\
   Tk_Offset (FGrab, fps), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL, (char *) NULL,\
   0, 0,\
   (Tk_CustomOption *) NULL}
};

/* 
 * FGrabCaptureConf[] 
 *
 * configure struct of Tk:	single-frame-capture options
 */     

static Tk_ConfigSpec FGrabCaptureConf[] = {
  {TK_CONFIG_INT, "-columns", "columns", "Columns", "640",\
   Tk_Offset (FGrab, capture.columns), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-rows", "rows", "Rows", "480",\
   Tk_Offset (FGrab, capture.rows), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-hue", "hue", "Hue", "20",\
   Tk_Offset (FGrab, capture.hue), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-chcv", "chcv", "Chcv", "44",\
   Tk_Offset (FGrab, capture.chcv), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-bright", "bright", "Bright", "148",\
   Tk_Offset (FGrab, capture.bright), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-chroma", "chroma", "Chroma", "114",\
   Tk_Offset (FGrab, capture.chroma), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_INT, "-contrast", "contrast", "Contrast", "74",\
   Tk_Offset (FGrab, capture.contrast), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_STRING, "-oformat", "oformat", "Oformat", "24bpp",\
   Tk_Offset (FGrab, capture.out_format), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_STRING, "-source", "source", "Source", "meteor",\
   Tk_Offset (FGrab, capture.source), 0,\
   (Tk_CustomOption *) NULL},
  {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL, (char *) NULL,\
   0, 0,\
   (Tk_CustomOption *) NULL}
};

/*
 *--------------------------------------------------------------------
 *
 * FGrabCmd --
 *
 * 	Diese Prozedur kreiert ein FGrab-Widget
 *
 * Results:
 *	Der Rückgabewert ist der Name des Widgets
 *
 * Side effects:
 *	Die Kreation eines weiteren FGrab-Widgets wird unterbunden	
 *	Der Widgetname wird als neuer Befehl registriert
 *	Ein Eventhandler wird kreiert
 *	Das Meteordevice wird geöffnet
 *	Das Widget struct wird initialisiert
 *	Das meteor_geomet struct wird initialisiert
 *	Die Standartoptions werden gesetzt
 *
 *--------------------------------------------------------------------
 */

static int FGrabCmd (ClientData clientData, Tcl_Interp *interp, int argc,\
		      char *argv[]) {
  
  int c, i;
  signed char b;
  unsigned char a;
  Tk_Window main = (Tk_Window) clientData;
  FGrab *fgrabPtr;
  Tk_Window tkwin;

  /* existiert bereits ein FGrab-Widget ? */
  if (FGrabReg & COUNT != 0) {
    Tcl_AppendResult (interp, "FGrab Object already created (just one ",\
		      "is possible!)", (char *) NULL);
    return TCL_ERROR;
  }

  /* Speicher für den Widget-Struct reservieren */
  if ((fgrabPtr = (FGrab *) malloc (sizeof(FGrab))) == NULL) {
    Tcl_AppendResult (interp, "Not enough memory for fgrab-structure",\
		      (char *) NULL);
    return TCL_ERROR;
  }

  if (argc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0], \
		      " pathName ?options?\"", (char *) NULL);
    return TCL_ERROR;
  }

  /* Widget-Objekt anlegen */
  tkwin = Tk_CreateWindowFromPath (interp, main, argv[1], (char *) NULL);
  if (tkwin == NULL) {
    return TCL_ERROR;
  }
  Tk_SetClass (tkwin, "FGrab");

  /* Widget-Struct initialisieren */
  fgrabPtr->tkwin = tkwin;
  fgrabPtr->display = Tk_Display (tkwin);
  fgrabPtr->interp = interp;
  fgrabPtr->screen = Tk_Screen (tkwin);
  fgrabPtr->depth = Tk_Depth (tkwin);

  fgrabPtr->gc = None;
  fgrabPtr->meteor = 0;
  fgrabPtr->DevName = "/dev/mmetfgrab0";
  fgrabPtr->mmbuf = NULL;
  fgrabPtr->fps = FPS;
  fgrabPtr->pixelsize = MAXPIXELSIZE;
  fgrabPtr->bufsize = MAXPIXELSIZE;
  fgrabPtr->fieldmode = 0x0;
  fgrabPtr->sigmode = 0x0;
  fgrabPtr->device = NULL;
  fgrabPtr->format = NULL;
  fgrabPtr->yuv = NULL;

  fgrabPtr->video.columns = MAXCOLS;
  fgrabPtr->video.rows = MAXROWS;
  fgrabPtr->video.hue = HUE;
  fgrabPtr->video.chcv = CHCV;
  fgrabPtr->video.bright = BRIGHT;
  fgrabPtr->video.chroma = CHROMA;
  fgrabPtr->video.contrast = CONTRAST;
  fgrabPtr->video.oformat = METEOR_GEO_RGB16;


  fgrabPtr->capture.columns = MAXCOLS;
  fgrabPtr->capture.rows = MAXROWS;
  fgrabPtr->capture.hue = HUE;
  fgrabPtr->capture.chcv = CHCV;
  fgrabPtr->capture.bright = BRIGHT;
  fgrabPtr->capture.chroma = CHROMA;
  fgrabPtr->capture.contrast = CONTRAST;
  fgrabPtr->capture.oformat = METEOR_GEO_RGB24;
  fgrabPtr->capture.out_format = NULL;
  fgrabPtr->capture.source = NULL;

  fgrabPtr->updatePending = 0;
  fgrabPtr->geoChange = 0;

  fgrabPtr->borderWidth = 0;
  fgrabPtr->bgBorder = NULL;
  fgrabPtr->relief = TK_RELIEF_FLAT;

  /* FGrab Register Variable initialisieren */
  FGrabReg = 0;

  /* EventHandler kreieren */
  Tk_CreateEventHandler (fgrabPtr->tkwin, ExposureMask|StructureNotifyMask,\
			 FGrabEventProc, (ClientData) fgrabPtr);

  /* Widget-Befehl kreieren */
  Tcl_CreateCommand (interp, Tk_PathName (fgrabPtr->tkwin), FGrabWidgetCmd,\
		    (ClientData) fgrabPtr, (Tcl_CmdDeleteProc *) NULL);


  /* Meteor-Device öffnen */
  if ((fgrabPtr->meteor = open(fgrabPtr->DevName, O_RDWR)) < 0) {
    Tcl_AppendResult (interp, "open failed for device %s",\
		      fgrabPtr->DevName, (char *) NULL);
    return TCL_ERROR;
  }    

  /* Standartoptions setzen (Kommandozeile/Defaultwerte der Tk_ConfigSpec) */
  if (FGrabConfigure (interp, fgrabPtr, argc-2, argv+2, 0) != TCL_OK) {
    Tk_DestroyWindow (fgrabPtr->tkwin);
    return TCL_ERROR;
  }

  /* Videooptions setzen (Defaultwerte der Tk_ConfigSpec) */
  if (FGrabVideoConfigure (interp, fgrabPtr, 0, NULL, 0) != TCL_OK) {
    Tk_DestroyWindow (fgrabPtr->tkwin);
    return TCL_ERROR;
  }

  /* Captureoptions setzen (Defaultwerte der Tk_ConfigSpec) */
  if (FGrabCaptureConfigure (interp, fgrabPtr, 0, NULL, 0, FALSE) != TCL_OK) {
    Tk_DestroyWindow (fgrabPtr->tkwin);
    return TCL_ERROR;
  }

  /* Setzen des Input-Formats */
  if (strcmp (fgrabPtr->format, "pal") == 0)
    c = METEOR_FMT_PAL;
  else if (strcmp (fgrabPtr->format, "secam") == 0)
    c = METEOR_FMT_SECAM;
  else if (strcmp (fgrabPtr->format, "ntsc") == 0)
    c = METEOR_FMT_NTSC;
  else {
    Tk_DestroyWindow (fgrabPtr->tkwin);
    return TCL_ERROR;
  }
  
  fprintf(stderr, "%s\n", fgrabPtr->format);
  if (ioctl(fgrabPtr->meteor, METEORSFMT, &c) < 0) {
    Tcl_AppendResult (interp, "ioctl SetFormat failed", (char *) NULL);
    return TCL_ERROR;
  }
  
  /* Setzen der Input-Source */
  if (strcmp (fgrabPtr->device, "svideo") == 0)
    c = METEOR_INPUT_DEV_SVIDEO;
  else if (strcmp (fgrabPtr->device, "dev0") == 0)
    c = METEOR_INPUT_DEV0;
  else if (strcmp (fgrabPtr->device, "dev1") == 0)
    c = METEOR_INPUT_DEV1;
  else if (strcmp (fgrabPtr->device, "dev2") == 0)
    c = METEOR_INPUT_DEV2;
  else if (strcmp (fgrabPtr->device, "rgb") == 0)
    c = METEOR_INPUT_DEV_RGB;
  else if (strcmp (fgrabPtr->device, "rca") == 0)
    c = METEOR_INPUT_DEV_RCA;
  else {
    Tk_DestroyWindow (fgrabPtr->tkwin);
    return TCL_ERROR;
  }
  fprintf(stderr, "%s\n", fgrabPtr->device);
  if (ioctl(fgrabPtr->meteor, METEORSINPUT, &c) < 0) {
    Tcl_AppendResult (interp, "ioctl Setinput failed", (char *) NULL);
    return TCL_ERROR;
  }
 
  /* setzen von pixelsize und bufsize entsprechend der Farbtiefe */
  switch (fgrabPtr->depth) {
  case 24:
  case 32:
    fgrabPtr->pixelsize = 4;
    fgrabPtr->bufsize = 4;
    geo.oformat = METEOR_GEO_RGB24;
    fgrabPtr->video.oformat = METEOR_GEO_RGB24;
    break;
  case 16:
  case 15:
    fgrabPtr->pixelsize = 2;
    fgrabPtr->bufsize = 2;
    geo.oformat = METEOR_GEO_RGB16;
    fgrabPtr->video.oformat = METEOR_GEO_RGB16;
    break;
  default:
    fgrabPtr->pixelsize = 1;
    fgrabPtr->bufsize = 2;
    if (strcmp (fgrabPtr->yuv, "packed") == 0) {
      geo.oformat = METEOR_GEO_YUV_PACKED;
      fgrabPtr->video.oformat = METEOR_GEO_YUV_PACKED;
      fprintf(stderr, "Format set to YUV_PACKED\n");
      FGrabReg |= PACKED;
    } else if (strcmp (fgrabPtr->yuv, "planar") == 0) {
      geo.oformat = METEOR_GEO_YUV_PLANAR;
      fgrabPtr->video.oformat = METEOR_GEO_YUV_PLANAR;
      fprintf(stderr, "Format set to YUV_PLANAR\n");
    }
    break;
  }

  /* Initialisieren des meteor_geomet Struct aus den Video-Daten */
  geo.rows = fgrabPtr->video.rows;
  geo.columns = fgrabPtr->video.columns;
  geo.frames = NOFRAMES;
  geo.oformat |= fgrabPtr->fieldmode;
    
  /* Setzen des COUNT-Bit in FGrab-Register */
  FGrabReg |= COUNT;

  /* Rückgabe des neuen Widget-Befehls */
  interp->result = Tk_PathName (fgrabPtr->tkwin);
  return TCL_OK;
}


/*
 *--------------------------------------------------------------------
 *
 * FGrabVideoConfigure --
 *
 * 	Diese Prozedur konfiguriert die Video-Optionen
 *
 * Results:
 *	Der Rückgabewert ist TCL_OK/TCL_ERROR
 *
 * Side effects:
 *	-Videooptionen werden im Widget-Struct gesetzt
 *	-row, column, fps, hue, chcv, contrast, brightness, chroma werden
 *	 auf der Meteor Karte gesetzt
 *	-der für row/column benötigte MAP/SHM-Speicher wird allociert
 *	-die sich aus row/column ergebenden Geometry-Daten werden dem Packer
 *	 mitgeteilt
 *
 *--------------------------------------------------------------------
 */

static int FGrabVideoConfigure( Tcl_Interp *interp, FGrab *fgrabPtr, int argc,\
			    char *argv[], int flags ) {


  int c;
  signed char b;
  unsigned char a;
  FGrabSetting *video = &fgrabPtr->video;
  int oldRows = video->rows;
  int oldColumns = video->columns;
  int helpRows, helpColumns;


  /* setzen der Video-Optionen im Widget-Struct */
  if (Tk_ConfigureWidget( interp, fgrabPtr->tkwin, FGrabVideoConf, argc, argv,\
			  (char *) fgrabPtr, flags) != TCL_OK ) {
    return TCL_ERROR;
  }  
  if (FGrabMeteorConfigure (interp, fgrabPtr, &fgrabPtr->video, TRUE)\
      != TCL_OK ) {
    return TCL_ERROR;
  }

  /* setzen und checken von FPS */
  if (fgrabPtr->fps > 25) fgrabPtr->fps = 25;
  if (fgrabPtr->fps < 1)  fgrabPtr->fps = 1;

  if (ioctl(fgrabPtr->meteor, METEORSFPS, &(fgrabPtr->fps)) < 0) {
    Tcl_AppendResult (interp, "ioctl SetFPS failed", (char *) NULL);
    return TCL_ERROR;
  }
  if (ioctl(fgrabPtr->meteor, METEORGFPS, &a) < 0) {
    Tcl_AppendResult (interp, "ioctl GetFPS failed", (char *) NULL);
    return TCL_ERROR;
  }
  fprintf( stderr, "Fps: %d\n", a);


  /*
   * Setzen einer neuen Geometry falls erforderlich
   *	-Stoppen des kontinuierlichen Grabbens
   *	-freigeben und neuallocieren des MAP-Memories
   * 	-Starten des kontinuierlichen Grabbens
   *	-freigeben und neuallocieren des SHM-Memories
   */
  if ((video->rows != oldRows)||(video->columns != oldColumns)) {


    /* reset the geometry */
    geo.rows = video->rows;
    geo.columns = video->columns;
    
    /* Video on ?: nein -> dann keine direkte Kommunikation mit der Karte */
    if (FGrabReg & VIDEO_ON) {

      helpRows = video->rows;
      helpColumns = video->columns;
      video->rows = oldRows;
      video->columns = oldColumns;

      if (FGrabVideoOff (fgrabPtr, interp) != TCL_OK) {
	Tcl_AppendResult (interp, "Video-Shutdown failed", (char *) NULL);
	return TCL_ERROR;
      }       
            
      video->rows = helpRows;
      video->columns = helpColumns;

      /* neue Geometry setzen */
      if (ioctl(fgrabPtr->meteor, METEORSETGEO, &geo) < 0) {
	perror("ioctl SetGeometry failed");
	exit(1);
      }
      
      if (ioctl(fgrabPtr->meteor, METEORGFROFF, &off) < 0) {
	perror("ioctl FrameOffset failed");
	exit(1);
      }
      fprintf(stderr, "New geometry set to %d x %d\n", geo.columns, geo.rows);
      
      /* remap memory */
      if ((fgrabPtr->mmbuf=(char *) mmap((caddr_t)0, off.fb_size,\
         			 PROT_READ|PROT_WRITE,\
	  			 MAP_FILE|MAP_PRIVATE,\
                                fgrabPtr->meteor, (off_t)0)) == (char *)(-1)) {
	perror("mmap failed");
	exit(1);
      }
      
      /* continuous frame capture */
      c = METEOR_CAP_CONTINOUS ;
      if (ioctl (fgrabPtr->meteor, METEORCAPTUR, &c)) {
	perror("ioctl CaptContinuous failed");
	exit(1);
      }
      FGrabReg |= VIDEO_ON;
    }

    /* need to reallocate shmimage
     * wird im Moment unabhängig von VIDEO_ON gemacht, um immer die richtige
     * Größe zu garantieren
     */
    free(fgrabPtr->shmimage);
    fgrabPtr->shmimage = XShmCreateImage(fgrabPtr->display,\
					 Tk_Visual(fgrabPtr->tkwin),\
					 fgrabPtr->depth, ZPixmap, NULL,\
					 fgrabPtr->shminfo,\
					 video->columns, video->rows);
    fgrabPtr->shmimage->data = fgrabPtr->shminfo->shmaddr;
    
    /* setzen des Geometry-Flags (besser in FGrabReg!), da application
     * signals erst nach der Aktualisierung des X-Servers aktiviert werden
     * dürfen: Configure-Notify-Signal
     */
    fgrabPtr->geoChange = 1;
    
  }

  /* Neue Geometry Anforderung bekanntgeben */
  Tk_GeometryRequest (fgrabPtr->tkwin, video->columns\
		      + 2 * fgrabPtr->borderWidth, video->rows\
		      + 2 * fgrabPtr->borderWidth);
  Tk_SetInternalBorder (fgrabPtr->tkwin, fgrabPtr->borderWidth);

  /* Neuaufbau des Widgets anfordern */
  if (!fgrabPtr->updatePending) {
    Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
    fgrabPtr->updatePending = 1;
  }
  return TCL_OK;
}


static int FGrabMeteorConfigure( Tcl_Interp *interp, FGrab *fgrabPtr,\
				 FGrabSetting *set, int write ) {


  int c;
  signed char b;
  unsigned char a;


  /* Test auf Gültigkeitsgrenzen der einzelnen Werte */
  if (set->rows > MAXROWS) set->rows = MAXROWS;
  if (set->columns > MAXCOLS) set->columns = MAXCOLS;
  set->rows &= ROW_MASK;
  set->columns &= COL_MASK;

  if (set->chcv > 255) set->chcv = 255;
  if (set->bright > 255) set->bright = 255;
  if (set->chroma > 255) set->chroma = 255;
  if (set->contrast > 255) set->contrast = 255;
  if (set->hue > 127) set->hue = 127;

  if (set->chcv < 0) set->chcv = 0;
  if (set->bright < 0) set->bright = 0;
  if (set->chroma < 0) set->chroma = 0;
  if (set->contrast < 0) set->contrast = 0;
  if (set->hue < -127) set->hue = -127;


  if (write) {
    
    /* setzen und checken von CHCV */
    if (ioctl(fgrabPtr->meteor, METEORSCHCV, &(set->chcv)) < 0) {
      Tcl_AppendResult (interp, "ioctl SetCHCV failed", (char *) NULL);
      return TCL_ERROR;
    }
    if (ioctl(fgrabPtr->meteor, METEORGCHCV, &a) < 0) {
      Tcl_AppendResult (interp, "ioctl GetCHCV failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf( stderr, "CHCV: %d\n", a);
    
    /* setzen und checken von BRIGHTNESS */
    if (ioctl(fgrabPtr->meteor, METEORSBRIG, &(set->bright)) < 0) {
      Tcl_AppendResult (interp, "ioctl SetBrightness failed", (char *) NULL);
      return TCL_ERROR;
    }
    if (ioctl(fgrabPtr->meteor, METEORGBRIG, &a) < 0) {
      Tcl_AppendResult (interp, "ioctl GetBrightness failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf( stderr, "Brightness: %d\n", a);
    
    /* setzen und checken von CHROMA */
    if (ioctl(fgrabPtr->meteor, METEORSCSAT, &(set->chroma)) < 0) {
      Tcl_AppendResult (interp, "ioctl SetChromaSat failed", (char *) NULL);
      return TCL_ERROR;
    }
    if (ioctl(fgrabPtr->meteor, METEORGCSAT, &a) < 0) {
      Tcl_AppendResult (interp, "ioctl GetChromaSat failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf( stderr, "Chroma: %d\n", a);
    
    /* setzen und checken von CONTRAST */
    if (ioctl(fgrabPtr->meteor, METEORSCONT, &(set->contrast)) < 0) {
      Tcl_AppendResult (interp, "ioctl SetContrast failed", (char *) NULL);
      return TCL_ERROR;
    }
    if (ioctl(fgrabPtr->meteor, METEORGCONT, &a) < 0) {
      Tcl_AppendResult (interp, "ioctl GetContrast failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf( stderr, "Contrast: %d\n", a);
    
    /* setzen und checken von HUE */
    if (ioctl(fgrabPtr->meteor, METEORSHUE, &(set->hue)) < 0) {
      Tcl_AppendResult (interp, "ioctl SetHue failed", (char *) NULL);
      return TCL_ERROR;
    }
    if (ioctl(fgrabPtr->meteor, METEORGHUE, &b) < 0) {
      Tcl_AppendResult (interp, "ioctl GetHue failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf( stderr, "Hue: %d\n", b);
  }

  return TCL_OK;
}


static int FGrabCaptureConfigure( Tcl_Interp *interp, FGrab *fgrabPtr,\
				  int argc, char *argv[], int flags,\
				  int write ) {


  int c;
  signed char b;
  unsigned char a;

  if (Tk_ConfigureWidget( interp, fgrabPtr->tkwin, FGrabCaptureConf, argc,\
			  argv, (char *) fgrabPtr, flags) != TCL_OK ) {
    return TCL_ERROR;
  }
  if (FGrabMeteorConfigure (interp, fgrabPtr, &fgrabPtr->capture, write)\
      != TCL_OK ) {
    return TCL_ERROR;
  }




    /* Setzen des Output-Formats */

  if (strcmp (fgrabPtr->capture.out_format, "24bpp") == 0) {
    fgrabPtr->capture.oformat = METEOR_GEO_RGB24;
  }
  else if (strcmp (fgrabPtr->capture.out_format, "15bpp") == 0) {
    fgrabPtr->capture.oformat = METEOR_GEO_RGB16;
  }
  else if (strcmp (fgrabPtr->capture.out_format, "b/w") == 0) {
    if (strcmp (fgrabPtr->yuv, "packed") == 0) {
      fgrabPtr->capture.oformat = METEOR_GEO_YUV_PACKED;
    } else if (strcmp (fgrabPtr->yuv, "planar") == 0) {
      fgrabPtr->capture.oformat = METEOR_GEO_YUV_PLANAR;
    }
  } else {
    fgrabPtr->capture.oformat = METEOR_GEO_RGB24;
    Tcl_AppendResult (interp, "wrong output-format: should be \"",\
		      "24bpp, 15bpp, b/w\"", (char *) NULL);
    return TCL_ERROR;
  }

  if (strcmp (fgrabPtr->capture.source, "meteor") != 0) {
    if (strcmp (fgrabPtr->capture.source, "video") != 0) {
      Tcl_AppendResult (interp, "wrong source: should be \"",\
			"meteor, video\"", (char *) NULL);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}

static int FGrabConfigure( Tcl_Interp *interp, FGrab *fgrabPtr, int argc,\
			    char *argv[], int flags ) {

  int c;
  signed char b;
  unsigned char a;

  if (Tk_ConfigureWidget( interp, fgrabPtr->tkwin, FGrabConf, argc, argv,\
			  (char *) fgrabPtr, flags) != TCL_OK ) {
    return TCL_ERROR;
  }

  Tk_SetWindowBackground( fgrabPtr->tkwin,\
			   Tk_3DBorderColor (fgrabPtr->bgBorder)->pixel );

  if (fgrabPtr->gc == None) {
    XGCValues gcValues;
    gcValues.function = GXcopy;
    gcValues.graphics_exposures = False;
    fgrabPtr->gc = Tk_GetGC (fgrabPtr->tkwin,\
			      GCFunction|GCGraphicsExposures, &gcValues);

    /* SHARED MEMORY */
      
    fgrabPtr->shminfo = (XShmSegmentInfo*) malloc (sizeof(XShmSegmentInfo));
    fgrabPtr->shmimage = XShmCreateImage(fgrabPtr->display,\
			 Tk_Visual(fgrabPtr->tkwin),\
			 fgrabPtr->depth, ZPixmap, NULL, fgrabPtr->shminfo,\
			 fgrabPtr->video.columns, fgrabPtr->video.rows);
    fprintf(stderr, "depth=%d\n", fgrabPtr->depth);
    fprintf(stderr, "size=%lx\n", fgrabPtr->shmimage->bytes_per_line *\
	    fgrabPtr->shmimage->height);
    if ((fgrabPtr->shminfo->shmid = shmget(IPC_PRIVATE,\
				 fgrabPtr->shmimage->bytes_per_line * \
				 fgrabPtr->shmimage->height,
				 IPC_CREAT | 0777)) == -1) {
      Tcl_AppendResult (interp, "Shared memory error (shmget)", (char *) NULL);
      return TCL_ERROR;
    }
    if ((fgrabPtr->shminfo->shmaddr = (char *) shmat(fgrabPtr->shminfo->shmid,\
				      (void *)0, 0)) == (char *)(-1)) {
      Tcl_AppendResult (interp, "Shared memory error (shmat)", (char *) NULL);
      return TCL_ERROR;
    }
    fgrabPtr->shmimage->data = fgrabPtr->shminfo->shmaddr;
    XShmAttach(fgrabPtr->display, fgrabPtr->shminfo);
    


    /*    fgrabPtr->shmimage = XCreateImage(fgrabPtr->display,\
			 Tk_Visual(fgrabPtr->tkwin),\
			 fgrabPtr->depth, ZPixmap, 0, NULL,\
			 fgrabPtr->video.columns, fgrabPtr->video.rows, 8, 0);
    fprintf(stderr, "depth=%d\n", fgrabPtr->depth);
    fprintf(stderr, "size=%lx\n", fgrabPtr->shmimage->bytes_per_line *\
	    fgrabPtr->shmimage->height);
    fgrabPtr->shmimage->data = (char *) malloc (fgrabPtr->\
			       shmimage->bytes_per_line * \
			       fgrabPtr->shmimage->height);*/

  }

  Tk_GeometryRequest (fgrabPtr->tkwin, fgrabPtr->video.columns\
		      + 2 * fgrabPtr->borderWidth, fgrabPtr->video.rows\
		      + 2 * fgrabPtr->borderWidth);
  Tk_SetInternalBorder (fgrabPtr->tkwin, fgrabPtr->borderWidth);

  if (!fgrabPtr->updatePending) {
    Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
    fgrabPtr->updatePending = 1;
  }
  return TCL_OK;
}


static int FGrabWidgetCmd( ClientData clientData, Tcl_Interp *interp,\
			    int argc, char *argv[] ) {
  

  FGrab *fgrabPtr = (FGrab *) clientData;
  int result = TCL_OK;


  if (argc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],\
		      " option ?arg arg ...?\"", (char *) NULL);
    return TCL_ERROR;
  }

  Tk_Preserve ((ClientData) fgrabPtr);

  if (strcmp (argv[1], "configure") == 0) {
    if (argc == 2) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabConf,\
				 (char *) fgrabPtr, (char *) NULL, 0);
    } else if (argc == 3) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabConf,\
				 (char *) fgrabPtr, argv[2], 0);
    } else {
      result = FGrabConfigure (interp, fgrabPtr, argc-2, argv+2,\
				TK_CONFIG_ARGV_ONLY);
    }
    if (!fgrabPtr->updatePending) {
      Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
      fgrabPtr->updatePending = 1;
    }

  }else if (strcmp (argv[1], "videoconfigure") == 0) {
    if (argc == 2) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabVideoConf,\
				 (char *) fgrabPtr, (char *) NULL, 0);
    } else if (argc == 3) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabVideoConf,\
				 (char *) fgrabPtr, argv[2], 0);
    } else {
      if (!(FGrabReg & VIDEO_HOLD)) {
	result = FGrabVideoConfigure (interp, fgrabPtr, argc-2, argv+2,\
				      TK_CONFIG_ARGV_ONLY);
      }
    }
    if (!fgrabPtr->updatePending) {
      Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
      fgrabPtr->updatePending = 1;
    }

  } else if (strcmp (argv[1], "captureconfigure") == 0) {
    if (argc == 2) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabCaptureConf,\
				 (char *) fgrabPtr, (char *) NULL, 0);
    } else if (argc == 3) {
      result = Tk_ConfigureInfo (interp, fgrabPtr->tkwin, FGrabCaptureConf,\
				 (char *) fgrabPtr, argv[2], 0);
    } else {
      result = FGrabCaptureConfigure (interp, fgrabPtr, argc-2, argv+2,\
				TK_CONFIG_ARGV_ONLY, FALSE);
    }

    /* 
     * VIDEO-Option
     * Ein- und Ausschalten des Livevideos
     * Keine Auswirkungen auf das bereits dardestellte Bild
     */

  } else if (strcmp (argv[1], "video") == 0) {
    if (argc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],\
			" video ?arg?\"", (char *) NULL);
      goto error;
    }
    if (strcmp (argv[2], "on") == 0) {
      result = FGrabVideoOn (fgrabPtr, interp);
    } else if (strcmp (argv[2], "off") == 0) {
      result = FGrabVideoOff (fgrabPtr, interp);
    } else if (strcmp (argv[2], "hold") == 0) {
      result = FGrabVideoHold (fgrabPtr, interp);
    } else {
      Tcl_AppendResult (interp, "bad option: must be \"", argv[0]," ",\
			argv[1], " on/off/hold\"", (char *) NULL);
    }
    /* 
     * CAPTURE-Option
     * capture of a single frame
     * 
     */

  } else if (strcmp (argv[1], "capture") == 0) {
    if (argc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],\
			" capture ?arg?\"", (char *) NULL);
      goto error;
    }
    result = FGrabCapture (fgrabPtr, interp, argv[2]);

  } else if (strcmp (argv[1], "fpscount") == 0) {
    if (argc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"", argv[0],\
			" fpscount ?arg?\"", (char *) NULL);
      goto error;
    }
    if (strcmp (argv[2], "on") == 0) {
      if (!(FGrabReg & FPSCOUNT)) { 
	FGrabReg |= FPSCOUNT;
	/* hier muß noch der alte Wert von Sigaction gespeichert werden !! */
	if (sigaction (SIGALRM, &sigalrm, NULL)) {
	  Tcl_AppendResult (interp, "alrmaction failed", (char *) NULL);
	  return TCL_ERROR;
	}
	raise (SIGALRM);
      }
    } else if (strcmp (argv[2], "off") == 0) {
      FGrabReg &= ~FPSCOUNT;
      /* hier muß noch der Sigaction zurueckgestellt werden!!, dies kann
       * nur bei ausgeschaltetem Interrupt erfolgen !!
       */
    } else {
      Tcl_AppendResult (interp, "bad option: must be \"", argv[0]," ",\
			argv[1], "on/off\"", (char *) NULL);
      goto error;
    }

  } else {
    Tcl_AppendResult (interp, "bad option \"", argv[1],\
		      "\": must be configure, videoconfigure, ",\
		      "captureconfigure, video, fpscount\"", (char *) NULL);
    goto error;
  }

  Tk_Release ((ClientData) fgrabPtr);
  return result;

			    error:
  Tk_Release ((ClientData) fgrabPtr);
  return TCL_ERROR;
}

static int FGrabVideoOn( FGrab *fgrabPtr, Tcl_Interp *interp) {


  int c, i;
  signed char b;
  unsigned char a;


  if (FGrabReg & VIDEO_HOLD) {
    FGrabReg &= ~VIDEO_HOLD;
  }

  if (!(FGrabReg & VIDEO_ON)) { 
    FGrabReg |= VIDEO_ON;
    

    if (FGrabVideoConfigure (interp, fgrabPtr, 0, NULL, TK_CONFIG_ARGV_ONLY)\
	!= TCL_OK) {
      return TCL_ERROR;
    }
    
    /* Initialisieren des meteor_geomet Struct aus den Video-Daten */
    geo.rows = fgrabPtr->video.rows;
    geo.columns = fgrabPtr->video.columns;
    geo.frames = NOFRAMES;
    geo.oformat = fgrabPtr->video.oformat | fgrabPtr->fieldmode;

    if (sigaction (SIGUSR2, &sigact, NULL)) {
      Tcl_AppendResult (interp, "sigaction failed", (char *) NULL);
      return TCL_ERROR;
    }
    
    FGrabAsyncToken = Tcl_AsyncCreate (FGrabAsyncHandler, (ClientData)\
				       fgrabPtr);
    FGrabReg |= AHANDLEREXIST;
    FGrabReg &= ~ASYNCMARK;
    
    if (ioctl(fgrabPtr->meteor, METEORSETGEO, &geo) < 0) {
      Tcl_AppendResult (interp, "ioctl SetVideoGeometry failed",\
			(char *) NULL);
      return TCL_ERROR;
    }
    
    if (ioctl(fgrabPtr->meteor, METEORGFROFF, &off) < 0) {
      Tcl_AppendResult (interp, "ioctl GetVideoFrameOffset failed",\
			(char *) NULL);
      return TCL_ERROR;
    }
    
    fprintf(stderr, "size:0x%lx\n", off.fb_size);
    fprintf(stderr, "mem: 0x%lx\n", off.mem_off);
    for (i=0; i < NOFRAMES; i++)
      fprintf(stderr, "frame %d: 0x%lx\n", i, off.frame_offset[i]);
    
    if ((fgrabPtr->mmbuf = (char *) mmap((caddr_t)0, off.fb_size,
			 PROT_READ|PROT_WRITE, 
			 MAP_FILE|MAP_SHARED, 
			 fgrabPtr->meteor, (off_t)0)) == (char *)(-1)) {
      Tcl_AppendResult (interp, "mmap failed", (char *) NULL);
      return TCL_ERROR;
    }
        
    if (c==METEOR_INPUT_DEV_RGB && geo.oformat==METEOR_GEO_RGB16) {
      /* set RGB section for 15 bit colour */
      u_short s = (0x4 << 4);
      if (ioctl(fgrabPtr->meteor, METEORSBT254, &s)) {
	Tcl_AppendResult (interp, "ioctl SetBt254 failed", (char *) NULL);
	return TCL_ERROR;
      }
    }
    
    c = SIGUSR2 | fgrabPtr->sigmode;
    if (ioctl(fgrabPtr->meteor, METEORSSIGNAL, &c) < 0) {
      Tcl_AppendResult (interp, "ioctl SetSignal failed", (char *) NULL);
      return TCL_ERROR;
    }
    
      
    /* continuous frame capture */
    c = METEOR_CAP_CONTINOUS ;
    if (ioctl(fgrabPtr->meteor, METEORCAPTUR, &c)) {
      Tcl_AppendResult (interp, "ioctl CaptContinous failed",\
			(char *) NULL);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}

static int FGrabVideoOff( FGrab *fgrabPtr, Tcl_Interp *interp) {

  int c, i;
  signed char b;
  unsigned char a;


  FGrabReg &= ~VIDEO_ON;
    
  /* first disable application signals */
  c = 0;
  if (ioctl(fgrabPtr->meteor, METEORSSIGNAL, &c) < 0) {
    Tcl_AppendResult (interp, "ioctl SetSignal failed", (char *) NULL);
    return TCL_ERROR;
  }
  if (FGrabReg & AHANDLEREXIST) {
    Tcl_AsyncDelete (FGrabAsyncToken);
    FGrabReg &= ~AHANDLEREXIST;
  }

  /* now stop continuous capture */
  c = METEOR_CAP_STOP_CONT;
  if (ioctl(fgrabPtr->meteor, METEORCAPTUR, &c)) {
    Tcl_AppendResult (interp, "ioctl StopCaptContinous failed",\
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  /* wait for 80ms to make sure no more frames are arriving */
  usleep(80000);
    
  /* unmap memeory */
  munmap(fgrabPtr->mmbuf, NOFRAMES * fgrabPtr->bufsize * \
	 fgrabPtr->video.rows * fgrabPtr->video.columns);
  
  if (!fgrabPtr->updatePending) {
    Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
    fgrabPtr->updatePending = 1;
  }
  return TCL_OK;
}

static int FGrabVideoHold( FGrab *fgrabPtr, Tcl_Interp *interp) {


  if (FGrabReg & VIDEO_ON) {
    if (!(FGrabReg & VIDEO_HOLD)) {
      FGrabVideoOff (fgrabPtr, interp);
      FGrabReg |= VIDEO_HOLD;
    }
  }
  return TCL_OK;
}


static int FGrabCapture( FGrab *fgrabPtr, Tcl_Interp *interp, char *name ) {


  int c,j,k,flag;
  u_long cap;
  struct meteor_counts cnt;
  int filehandler;
  char header[32];
  unsigned char *mmbuf;
  unsigned char *outbuf, *index;
  unsigned short word; 
  unsigned int size, rows, columns;
  int mono;
  unsigned long oformat;

  if (FGrabReg & VIDEO_HOLD) {
    flag = TRUE;
  } else {
    flag = FALSE;
  }

  if (FGrabVideoHold (fgrabPtr, interp) != TCL_OK) {
    return TCL_ERROR;
  }

  if (FGrabCaptureConfigure (interp, fgrabPtr, 0, NULL, TK_CONFIG_ARGV_ONLY,\
			     TRUE) != TCL_OK) {
    return TCL_ERROR;
  }

  if (strcmp (fgrabPtr->capture.source, "meteor") == 0) {
  
    /* set up the capture type and size */
    geo.rows = fgrabPtr->capture.rows;
    geo.columns = fgrabPtr->capture.columns;
    geo.frames = NOFRAMES;
    geo.oformat = fgrabPtr->capture.oformat;
    
    /* neue Geometry setzen */
    if (ioctl(fgrabPtr->meteor, METEORSETGEO, &geo) < 0) {
      Tcl_AppendResult (interp, "ioctl SetGeometry failed", (char *) NULL);
      return TCL_ERROR;
    }
    
    if (ioctl(fgrabPtr->meteor, METEORGFROFF, &off) < 0) {
      Tcl_AppendResult (interp, "ioctl GetCaptureFrameOffset failed",\
			(char *) NULL);
      return TCL_ERROR;
    }
    fprintf(stderr, "Capture geometry set to %d x %d\n", geo.columns,\
	    geo.rows);
    fprintf(stderr, "size:0x%lx\n", off.fb_size);
    fprintf(stderr, "mem: 0x%lx\n", off.frame_offset[0]);
    
    
    /* map memory */
    if ((fgrabPtr->mmbuf=(char *) mmap((caddr_t)0, off.fb_size,\
			       PROT_READ|PROT_WRITE,\
			       MAP_FILE|MAP_PRIVATE,\
			       fgrabPtr->meteor, (off_t)0)) == (char *)(-1)) {
      Tcl_AppendResult (interp, "capture mmap failed", (char *) NULL);
      return TCL_ERROR;
    }
    mmbuf = fgrabPtr->mmbuf + off.frame_offset[0];
    
    /* capture one frame */
    c = METEOR_CAP_SINGLE ;
    if (ioctl(fgrabPtr->meteor, METEORCAPTUR, &c)) {
      Tcl_AppendResult (interp, "ioctl SingleCapture failed", (char *) NULL);
      return TCL_ERROR;
    }
    
    if (ioctl(fgrabPtr->meteor, METEORGCAPT, &cap)) {
      Tcl_AppendResult (interp, "ioctl GetCapt failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf(stderr, "Capture control: 0x%lx\n", cap);
    
    /* get error counts */
    if (ioctl(fgrabPtr->meteor, METEORGCOUNT, &cnt)) {
      Tcl_AppendResult (interp, "ioctl GetCount failed", (char *) NULL);
      return TCL_ERROR;
    }
    fprintf(stderr, "Frames: %d\nEven:   %d\nOdd:    %d\n", 
	    cnt.frames_captured,
	    cnt.even_fields_captured,
	    cnt.odd_fields_captured);
    fprintf(stderr, "Fifo errors: %d\n", cnt.fifo_errors);
    fprintf(stderr, "DMA errors:  %d\n", cnt.dma_errors);
    
    fprintf(stderr, "Writing image\n");

    oformat = fgrabPtr->capture.oformat;
    rows = fgrabPtr->capture.rows;
    columns = fgrabPtr->capture.columns;

  } else {
    if (!(FGrabReg & VIDEO_HOLD) || !(Tk_IsMapped (fgrabPtr->tkwin))) {
      Tcl_AppendResult (interp, "There is no video to grab from!",\
			(char *) NULL);
      return TCL_ERROR;
    }
    oformat = fgrabPtr->video.oformat;
    rows = fgrabPtr->video.rows;
    columns = fgrabPtr->video.columns;
    mmbuf = fgrabPtr->shmimage->data;
  }

  size = rows * columns;

  switch (oformat) {

  case METEOR_GEO_RGB24:
  case METEOR_GEO_RGB16: {
    size *= 3;
    mono = FALSE;
    break;
  }

  case METEOR_GEO_YUV_PLANAR:
  case METEOR_GEO_YUV_PACKED: {
    mono = TRUE;
    break;
  }
  default: {
    fprintf(stderr, "Is there somewhere an error?");
  }
  }

  if ((outbuf = (char *) malloc (size)) == NULL) {
    Tcl_AppendResult (interp, "Allocation of Output-Buffer failed",\
		      (char *) NULL);
    return TCL_ERROR;
  }
  index = outbuf;

  switch (oformat) {
  case METEOR_GEO_RGB24: {
    
    for (j=0; j < (rows * columns); j++) {
      *(index+2) = *mmbuf++;
      *(index+1) = *mmbuf++;
      *(index) = *mmbuf++;
      index += 3;
      mmbuf++;
    }
    break;
  }
  case METEOR_GEO_RGB16: {
    for (j=0; j < (rows * columns); j++) {
      word = *mmbuf + 256 * *(mmbuf+1);
      *(index)   = (word & 0x7c00) >> 7;
      *(index+1) = (word & 0x03e0) >> 2;
      *(index+2) = (word & 0x001f) << 3;
      index += 3;
      mmbuf += 2;
    }
    break;
  }
  case METEOR_GEO_YUV_PLANAR: {

    memcpy (outbuf, mmbuf, rows * columns);
    break;
  }
  case METEOR_GEO_YUV_PACKED: {

    for (j=0; j < (rows * columns); j++) {
      *index++ = *(++mmbuf);
      mmbuf++;
    }
    break;
  }
  default: {
    fprintf(stderr, "Is there somewhere a second error?");
  }
  }

  if ((filehandler = open(name, O_WRONLY | O_CREAT, 0644)) < 0) {
    Tcl_AppendResult (interp, "open file ", name, " failed",\
		      (char *) NULL);
    return TCL_ERROR;
  }

  /* make PPM/PGM header and save to file */
  sprintf(header, "P%c\n%d %d\n255\n", (mono ? '5' : '6'),\
	  columns, rows);
  write (filehandler, &header[0], strlen(header));

  /* save the data to PPM file */
  write(filehandler, outbuf, size);

  close(filehandler);
  free (outbuf);

  /* unmap memory */
  if (strcmp (fgrabPtr->capture.source, "meteor") == 0) {
    munmap(fgrabPtr->mmbuf, off.fb_size);
  }

  if (!flag) {
    if (FGrabVideoOn (fgrabPtr, interp) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  return TCL_OK; 
}


static void FGrabEventProc( ClientData clientData, XEvent *eventPtr ) {


  u_long size;
  int c;
  FGrab *fgrabPtr = (FGrab *) clientData;


  if (eventPtr->type == Expose) {
    if (!fgrabPtr->updatePending) {
      Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
      fgrabPtr->updatePending = 1;
    }

  } else if (eventPtr->type == ConfigureNotify) {

    if (fgrabPtr->geoChange) {
      fgrabPtr->geoChange = 0;

      /* re-enable application signals */
      FGrabAsyncToken = Tcl_AsyncCreate (FGrabAsyncHandler,\
					 (ClientData) fgrabPtr);
      FGrabReg |= AHANDLEREXIST;
      FGrabReg &= ~ASYNCMARK;

      c = SIGUSR2 | fgrabPtr->sigmode;
      if (ioctl (fgrabPtr->meteor, METEORSSIGNAL, &c) < 0) {
	perror("ioctl SetSignal failed");
	exit(1);
      }
    }
    if (!fgrabPtr->updatePending) {
      Tk_DoWhenIdle (FGrabDisplay, (ClientData) fgrabPtr);
      fgrabPtr->updatePending = 1;
    }

  } else if (eventPtr->type == DestroyNotify) {
    Tcl_DeleteCommand (fgrabPtr->interp, Tk_PathName (fgrabPtr->tkwin));
    fgrabPtr->tkwin = NULL;
    if (fgrabPtr->updatePending) {
      Tk_CancelIdleCall (FGrabDisplay, (ClientData) fgrabPtr);
    }
      Tk_EventuallyFree ((ClientData) fgrabPtr,\
			 (Tk_FreeProc *) FGrabDestroy);
      /*hier muß noch der Shared-Memory-Bereich aufgeräumt werden */
  }
}
    

static void FGrabDisplay( ClientData clientData ) {
  

  FGrab *fgrabPtr = (FGrab *) clientData;
  Tk_Window tkwin = fgrabPtr->tkwin;
  
  Display *display = Tk_Display (tkwin);
  int borderWidth = fgrabPtr->borderWidth;
  Pixmap pm;
  int c;


  fgrabPtr->updatePending = 0;
  if (!Tk_IsMapped (tkwin)) {
    return;
  }

  pm = XCreatePixmap (display, Tk_WindowId (tkwin), Tk_Width (tkwin),\
		      Tk_Height (tkwin), Tk_Depth (tkwin));
  Tk_Fill3DRectangle (tkwin, pm, fgrabPtr->bgBorder, 0, 0,\
		      Tk_Width (tkwin), Tk_Height (tkwin),\
		      fgrabPtr->borderWidth, fgrabPtr->relief);

  XCopyArea (display, pm, Tk_WindowId (tkwin), fgrabPtr->gc, 0, 0,\
	     Tk_Width (tkwin), Tk_Height (tkwin), 0, 0);
  XFreePixmap (display, pm);

  /* Image neu zeichnen im Hold-Modus */
  if (FGrabReg & VIDEO_HOLD) {

    /* Draw screen onto display */
    XShmPutImage(fgrabPtr->display, Tk_WindowId\
		 (fgrabPtr->tkwin), fgrabPtr->gc,\
		 fgrabPtr->shmimage, 0, 0, borderWidth, borderWidth,\
		 Tk_Width (tkwin) - 2 * borderWidth,\
		 Tk_Height(tkwin) - 2 * borderWidth,\
		 False);
  }
}


static void FGrabDestroy( ClientData clientData ) {


  FGrab *fgrabPtr = (FGrab *) clientData;
  int c;

  FGrabReg &= ~COUNT;
  if (FGrabReg & VIDEO_ON) {
    
    /* first disable application signals */
    c = 0;
    if (ioctl(fgrabPtr->meteor, METEORSSIGNAL, &c) < 0) {
      perror("ioctl SetSignal failed");
      exit(1);
    }
    
    if (FGrabReg & AHANDLEREXIST) {
      Tcl_AsyncDelete (FGrabAsyncToken);
      FGrabReg &= ~AHANDLEREXIST;
    }
    
    /* now stop continuous capture */
    c = METEOR_CAP_STOP_CONT;
    if (ioctl(fgrabPtr->meteor, METEORCAPTUR, &c)) {
      perror("ioctl CaptContinuous failed");
      exit(1);
    }

    /* wait for 80ms to make sure no more frames are arriving */
    usleep(80000);
    
    /* unmap memeory */
    munmap(fgrabPtr->mmbuf, NOFRAMES * fgrabPtr->bufsize * \
	   fgrabPtr->video.rows * fgrabPtr->video.columns);
  }

  close(fgrabPtr->meteor);

  Tk_FreeOptions (FGrabConf, (char *) fgrabPtr, fgrabPtr->display, 0);
  if (fgrabPtr->gc != None) {
    Tk_FreeGC (fgrabPtr->display, fgrabPtr->gc);
  }

  free(fgrabPtr->shmimage);
  free ((char *) fgrabPtr);

}











int PAS_Init( Tcl_Interp *interp ){


  Tcl_CreateCommand (interp, "fgrab", FGrabCmd,\
		     (ClientData) Tk_MainWindow (interp),\
		     (Tcl_CmdDeleteProc *) NULL);
  
  return TCL_OK;							 
}





/*
 * zu erledigen:-die munmap-Befehle auf off.fb_size umrüsten!!!
 *		-falls ein Stringwert falsch eingegeben wird, wird dies
 *		 zwar mokiert, aber nicht nicht wieder korrigiert!!!
 */
