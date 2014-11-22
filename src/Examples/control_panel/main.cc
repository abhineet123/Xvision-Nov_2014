/*                                                      -*-c++-*-
    Copyright (C) 2000 Gregory D. Hager and Darius Burschka (JHU
    Lab for Computational Interaction with Physical Systems (CIPS))

Permission is granted to any individual or institution to use, copy,
modify, and distribute this software, provided that this complete
copyright and permission notice is maintained, intact, in all copies and
supporting documentation.  Authors of papers that describe software
systems using this software package are asked to acknowledge such use by
a brief statement in the paper.

We provide this software "as is" without express or implied warranty.
*/

/*
 *  Jason Corso
 *
 *  This program can be used to test camera parameters. 
 *  It also serves as an example of how to control the parameters.
 *
 *  This code uses glut and mui as the gui libraries.  They are usually
 *   installed by default on linux distros.  If not, you must get them.
 */


#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "config.h"
#include "XVWindowX.h"
#include "XVAffineWarp.h"
#include "XVMpeg.h"
#include "Videre.h"
#include "XVDig1394.h"
#include "XVImageBase.h"
#include <signal.h>
#include <mui/mui.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>


#define CP_MANUAL 0
#define CP_AUTO 1

static int cameraIndex=0;

static  XVWindowX<XV_RGB> * window;
static	XVDig1394<XVImageRGB<XV_RGB> > * grabber;

static raw1394handle_t cp_handle;
static int cp_node_id;

static unsigned int cp_brightness;
static unsigned int cp_exposure;
static unsigned int cp_sharpness;
static unsigned int cp_white1, cp_white2;
static unsigned int cp_saturation;
static unsigned int cp_gamma;
static unsigned int cp_gain;
static unsigned int cp_shutter;
static unsigned int cp_iris;
static unsigned int cp_hue;
static unsigned int cp_capturequality;


static unsigned int cp_brightness_min;
static unsigned int cp_exposure_min;
static unsigned int cp_sharpness_min;
static unsigned int cp_white1_min,cp_white2_min;
static unsigned int cp_saturation_min;
static unsigned int cp_gamma_min;
static unsigned int cp_gain_min;
static unsigned int cp_shutter_min;
static unsigned int cp_iris_min;
static unsigned int cp_hue_min;
static unsigned int cp_capturequality_min;


static unsigned int cp_brightness_max;
static unsigned int cp_exposure_max;
static unsigned int cp_sharpness_max;
static unsigned int cp_white1_max,cp_white2_max;
static unsigned int cp_saturation_max;
static unsigned int cp_gamma_max;
static unsigned int cp_gain_max;
static unsigned int cp_shutter_max;
static unsigned int cp_iris_max;
static unsigned int cp_hue_max;
static unsigned int cp_capturequality_max;


static char cp_auto_manual_mode;

static muiObject *brightness_slider,*brightness_label,*brightness_value;
static char* brightness_text = "Brightness:";
static char brightness_valuetext[32] = "n/a";

static muiObject *exposure_slider,*exposure_label,*exposure_value;
static char* exposure_text = "Exposure:";
static char exposure_valuetext[32] = "n/a";

static muiObject *white1_slider,*white1_label,*white1_value;
static char* white1_text = "White 1:";
static char white1_valuetext[32] = "n/a";

static muiObject *white2_slider,*white2_label,*white2_value;
static char* white2_text = "White 2:";
static char white2_valuetext[32] = "n/a";

static muiObject *sharpness_slider,*sharpness_label,*sharpness_value;
static char* sharpness_text = "Sharpness:";
static char sharpness_valuetext[32] = "n/a";

static muiObject *saturation_slider,*saturation_label,*saturation_value;
static char* saturation_text = "Saturation:";
static char saturation_valuetext[32] = "n/a";

static muiObject *gamma_slider,*gamma_label,*gamma_value;
static char* gamma_text = "Gamma:";
static char gamma_valuetext[32] = "n/a";

static muiObject *gain_slider,*gain_label,*gain_value;
static char* gain_text = "Gain:";
static char gain_valuetext[32] = "n/a";

static muiObject *shutter_slider,*shutter_label,*shutter_value;
static char* shutter_text = "Shutter:";
static char shutter_valuetext[32] = "n/a";

static muiObject *iris_slider,*iris_label,*iris_value;
static char* iris_text = "Iris:";
static char iris_valuetext[32] = "n/a";

static muiObject *hue_slider,*hue_label,*hue_value;
static char* hue_text = "Hue:";
static char hue_valuetext[32] = "n/a";

static muiObject *capturequality_slider,*capturequality_label,*capturequality_value;
static char* capturequality_text = "Quality:";
static char capturequality_valuetext[32] = "n/a";


static muiObject *auto_manual_button;
static char* auto_text = "To Auto Mode";
static char* manual_text = "To Manual Mode";
static char* auto_manual_note_text = "Auto mode does not effect camera settings.";

static muiObject *save_button;
static char* save_text = "Save Params";

static muiObject *title_label;
static char* title_text = "XVDig1394 Control Panel";


// function prototypes
void init_timer(int value);


void brightness_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(brightness_slider);
  i = ( (unsigned int ) (f * ((float)cp_brightness_max-cp_brightness_min) ) )
                           + cp_brightness_min;
  if (i != cp_brightness) {
    cp_brightness = i;
    grabber->set_brightness(cp_brightness);
    sprintf(brightness_valuetext,"%d",cp_brightness);
    muiChangeLabel(brightness_value,brightness_valuetext);
  }
}

void exposure_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(exposure_slider);
  i = ( (unsigned int ) (f * ((float)cp_exposure_max-cp_exposure_min) ) )
                           + cp_exposure_min;
  if (i != cp_exposure) {
    cp_exposure = i;
    grabber->set_exposure(cp_exposure);
    sprintf(exposure_valuetext,"%d",cp_exposure);
    muiChangeLabel(exposure_value,exposure_valuetext);
  }
}
void white1_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(white1_slider);
  i = ( (unsigned int ) (f * ((float)cp_white1_max-cp_white1_min) ) )
                           + cp_white1_min;
  if (i != cp_white1) {
    cp_white1 = i;
    grabber->set_whitebalance(cp_white1, cp_white2);
    sprintf(white1_valuetext,"%d",cp_white1);
    muiChangeLabel(white1_value,white1_valuetext);
  }
}
void white2_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(white2_slider);
  i = ( (unsigned int ) (f * ((float)cp_white2_max-cp_white2_min) ) )
                           + cp_white2_min;
  if (i != cp_white2) {
    cp_white2 = i;
    grabber->set_whitebalance(cp_white1, cp_white2);
    sprintf(white2_valuetext,"%d",cp_white2);
    muiChangeLabel(white2_value,white2_valuetext);
  }
}


void sharpness_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(sharpness_slider);
  i = ( (unsigned int ) (f * ((float)cp_sharpness_max-cp_sharpness_min) ) )
                           + cp_sharpness_min;
  if (i != cp_sharpness) {
    cp_sharpness = i;
    grabber->set_sharpness(cp_sharpness);
    sprintf(sharpness_valuetext,"%d",cp_sharpness);
    muiChangeLabel(sharpness_value,sharpness_valuetext);
  }
}

void saturation_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(saturation_slider);
  i = ( (unsigned int ) (f * ((float)cp_saturation_max-cp_saturation_min) ) )
                           + cp_saturation_min;
  if (i != cp_saturation) {
    cp_saturation = i;
    grabber->set_saturation(cp_saturation);
    sprintf(saturation_valuetext,"%d",cp_saturation);
    muiChangeLabel(saturation_value,saturation_valuetext);
  }
}

void gamma_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(gamma_slider);
  i = ( (unsigned int ) (f * ((float)cp_gamma_max-cp_gamma_min) ) )
                           + cp_gamma_min;
  if (i != cp_gamma) {
    cp_gamma = i;
    grabber->set_gamma(cp_gamma);
    sprintf(gamma_valuetext,"%d",cp_gamma);
    muiChangeLabel(gamma_value,gamma_valuetext);
  }
}

void gain_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(gain_slider);
  i = ( (unsigned int ) (f * ((float)cp_gain_max-cp_gain_min) ) )
                           + cp_gain_min;
  if (i != cp_gain) {
    cp_gain = i;
    grabber->set_gain(cp_gain);
    sprintf(gain_valuetext,"%d",cp_gain);
    muiChangeLabel(gain_value,gain_valuetext);
  }
}

void shutter_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(shutter_slider);
  i = ( (unsigned int ) (f * ((float)cp_shutter_max-cp_shutter_min) ) )
                           + cp_shutter_min;
  if (i != cp_shutter) {
    cp_shutter = i;
    grabber->set_shutter(cp_shutter);
    sprintf(shutter_valuetext,"%d",cp_shutter);
    muiChangeLabel(shutter_value,shutter_valuetext);
  }
}

void iris_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(iris_slider);
  i = ( (unsigned int ) (f * ((float)cp_iris_max-cp_iris_min) ) )
                           + cp_iris_min;
  if (i != cp_iris) {
    cp_iris = i;
    grabber->set_iris(cp_iris);
    sprintf(iris_valuetext,"%d",cp_iris);
    muiChangeLabel(iris_value,iris_valuetext);
  }
}

void hue_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(hue_slider);
  i = ( (unsigned int ) (f * ((float)cp_hue_max-cp_hue_min) ) )
                           + cp_hue_min;
  if (i != cp_hue) {
    cp_hue = i;
    grabber->set_hue(cp_hue);
    sprintf(hue_valuetext,"%d",cp_hue);
    muiChangeLabel(hue_value,hue_valuetext);
  }
}
void capturequality_cb(muiObject* O, muiReturnValue V)
{
  float f;
  unsigned int i;

  f = muiGetHSVal(capturequality_slider);
  i = ( (unsigned int ) (f * ((float)cp_capturequality_max-cp_capturequality_min) ) )
                           + cp_capturequality_min;
  if (i != cp_capturequality) {
    cp_capturequality = i;
    grabber->set_capturequality(cp_capturequality);
    sprintf(capturequality_valuetext,"%d",cp_capturequality);
    muiChangeLabel(capturequality_value,capturequality_valuetext);
  }
}



void auto_manual_cb(muiObject* O, muiReturnValue V)
{
  fprintf(stderr,"control_panel: %s\n",auto_manual_note_text);

  if (cp_auto_manual_mode == CP_MANUAL) {

    grabber->set_brightness_auto();
    grabber->set_exposure_auto();
    grabber->set_sharpness_auto();
    grabber->set_whitebalance_auto();
    grabber->set_saturation_auto();
    grabber->set_gamma_auto();
    grabber->set_gain_auto();
    grabber->set_shutter_auto();
    grabber->set_iris_auto();
    grabber->set_hue_auto();
    grabber->set_capturequality_auto();

    muiLoadButton(auto_manual_button,manual_text);
    cp_auto_manual_mode = CP_AUTO;


  } else {

    // The camera does not change the values of these registers 
    //  when it is in auto mode.  (The camera I tested this on!)
    grabber->get_brightness(&cp_brightness);
    grabber->get_exposure(&cp_exposure);
    grabber->get_sharpness(&cp_sharpness);
    grabber->get_whitebalance(&cp_white1,&cp_white2);
    grabber->get_saturation(&cp_saturation);
    grabber->get_gamma(&cp_gamma);
    grabber->get_gain(&cp_gain);
    grabber->get_shutter(&cp_shutter);
    grabber->get_iris(&cp_iris);
    grabber->get_hue(&cp_hue);
    grabber->get_capturequality(&cp_capturequality);
    
  
    grabber->set_brightness_manual();
    grabber->set_exposure_manual();
    grabber->set_sharpness_manual();
    grabber->set_whitebalance_manual();
    grabber->set_saturation_manual();
    grabber->set_gamma_manual();
    grabber->set_gain_manual();
    grabber->set_shutter_manual();
    grabber->set_iris_manual();
    grabber->set_hue_manual();
    grabber->set_capturequality_manual();
/*
    grabber->get_brightness(&cp_brightness);
    grabber->get_exposure(&cp_exposure);
    grabber->get_sharpness(&cp_sharpness);
    grabber->get_whitebalance(&cp_white1,&cp_white2);
    grabber->get_saturation(&cp_saturation);
    grabber->get_gamma(&cp_gamma);
    grabber->get_gain(&cp_gain);
    grabber->get_shutter(&cp_shutter);
    grabber->get_iris(&cp_iris);
*/

    init_timer(0);

    muiLoadButton(auto_manual_button,auto_text);
    cp_auto_manual_mode = CP_MANUAL;
  }
}

void save_cb(muiObject* O, muiReturnValue V)
{
  FILE *f;
  char fName[256];

  sprintf(fName,"dig1394%d.config",cameraIndex);

  f=fopen(fName,"w");

  if (!f) {
    fprintf(stderr,"Could not write to conf file\n");
  } else {
    fprintf(stderr,"Writing parameters to conf file\n");
  }
  fprintf(f,"# XVDig1394 Params:  Written by control_panel\n");

  fprintf(f,"BRIGHTNESS: %d\n",cp_brightness);
  fprintf(f,"EXPOSURE: %d\n",cp_exposure);
  fprintf(f,"SHARPNESS: %d\n",cp_sharpness);
  fprintf(f,"WHITE1: %d\n",cp_white1);
  fprintf(f,"WHITE2: %d\n",cp_white2);
  fprintf(f,"SATURATION: %d\n",cp_saturation);
  fprintf(f,"GAMMA: %d\n",cp_gamma);
  fprintf(f,"GAIN: %d\n",cp_gain);
  fprintf(f,"SHUTTER: %d\n",cp_shutter);
/*
  fprintf(f,"IRIS: %d\n",cp_iris);
  fprintf(f,"HUE: %d\n",cp_hue);
  fprintf(f,"CAPTUREQUALITY: %d\n",cp_capturequality);
*/  
  fclose(f);
}

void make_ui()
{
    muiNewUIList(1);    /* makes an MUI display list (number 1) */

    brightness_label = muiNewLabel(20,50,brightness_text);
    brightness_value= muiNewLabel(100,50,brightness_valuetext);
    brightness_slider = muiNewHSlider(20, 20, 300, 0, 8);
    muiSetCallback(brightness_slider,brightness_cb);

    exposure_label = muiNewLabel(20,100,exposure_text);
    exposure_value= muiNewLabel(100,100,exposure_valuetext);
    exposure_slider = muiNewHSlider(20, 70, 300, 0, 8);
    muiSetCallback(exposure_slider,exposure_cb);

    white1_label = muiNewLabel(20,150,white1_text);
    white1_value= muiNewLabel(100,150,white1_valuetext);
    white1_slider = muiNewHSlider(20, 120, 300, 0, 8);
    muiSetCallback(white1_slider,white1_cb);

    white2_label = muiNewLabel(20,200,white2_text);
    white2_value= muiNewLabel(100,200,white2_valuetext);
    white2_slider = muiNewHSlider(20, 170, 300, 0, 8);
    muiSetCallback(white2_slider,white2_cb);

    sharpness_label = muiNewLabel(20,250,sharpness_text);
    sharpness_value= muiNewLabel(100,250,sharpness_valuetext);
    sharpness_slider = muiNewHSlider(20, 220, 300, 0, 8);
    muiSetCallback(sharpness_slider,sharpness_cb);

    saturation_label = muiNewLabel(20,300,saturation_text);
    saturation_value= muiNewLabel(100,300,saturation_valuetext);
    saturation_slider = muiNewHSlider(20, 270, 300, 0, 8);
    muiSetCallback(saturation_slider,saturation_cb);

    gamma_label = muiNewLabel(20,350,gamma_text);
    gamma_value= muiNewLabel(100,350,gamma_valuetext);
    gamma_slider = muiNewHSlider(20, 320, 300, 0, 8);
    muiSetCallback(gamma_slider,gamma_cb);

    gain_label = muiNewLabel(20,400,gain_text);
    gain_value= muiNewLabel(100,400,gain_valuetext);
    gain_slider = muiNewHSlider(20, 370, 300, 0, 8);
    muiSetCallback(gain_slider,gain_cb);

    shutter_label = muiNewLabel(20,450,shutter_text);
    shutter_value= muiNewLabel(100,450,shutter_valuetext);
    shutter_slider = muiNewHSlider(20, 420, 300, 0, 8);
    muiSetCallback(shutter_slider,shutter_cb);

    iris_label = muiNewLabel(20,500,iris_text);
    iris_value= muiNewLabel(100,500,iris_valuetext);
    iris_slider = muiNewHSlider(20, 470, 300, 0, 8);
    muiSetCallback(iris_slider,iris_cb);

    hue_label = muiNewLabel(20,550,hue_text);
    hue_value= muiNewLabel(100,550,hue_valuetext);
    hue_slider = muiNewHSlider(20, 520, 300, 0, 8);
    muiSetCallback(hue_slider,hue_cb);
   
    capturequality_label = muiNewLabel(20,600,capturequality_text);
    capturequality_value= muiNewLabel(100,600, capturequality_valuetext);
    capturequality_slider = muiNewHSlider(20, 570, 300, 0, 8);
    muiSetCallback( capturequality_slider, capturequality_cb);

    auto_manual_button = muiNewButton(20,150,630,660);
    muiLoadButton(auto_manual_button,auto_text);
    muiSetCallback(auto_manual_button,auto_manual_cb);

    save_button = muiNewButton(170,300,630,660);
    muiLoadButton(save_button,save_text);
    muiSetCallback(save_button,save_cb);

    title_label = muiNewBoldLabel(85,680,title_text);
}

void idle() 
{
  //Acquire image
  grabber->current_frame_continuous();
  window->CopySubImage(grabber->frame(grabber->get_buffer_index()));
  window -> swap_buffers();
  window -> flush();
}

void init_timer(int value) {
  muiSetHSValue(brightness_slider, (float) ((cp_brightness-cp_brightness_min)/
                                   ((float) (cp_brightness_max-cp_brightness_min))) );
  sprintf(brightness_valuetext,"%d",cp_brightness);
  muiChangeLabel(brightness_value,brightness_valuetext);
  muiSetHSValue(exposure_slider, (float) ((cp_exposure-cp_exposure_min)/
                                 ((float) (cp_exposure_max-cp_exposure_min))) );
  sprintf(exposure_valuetext,"%d",cp_exposure);
  muiChangeLabel(exposure_value,exposure_valuetext);
  muiSetHSValue(sharpness_slider, (float) ((cp_sharpness-cp_sharpness_min)/
                                  ((float) (cp_sharpness_max-cp_sharpness_min))) );
  sprintf(sharpness_valuetext,"%d",cp_sharpness);
  muiChangeLabel(sharpness_value,sharpness_valuetext);
  muiSetHSValue(saturation_slider, (float) ((cp_saturation-cp_saturation_min)/
                                   ((float) (cp_saturation_max-cp_saturation_min))) );
  sprintf(saturation_valuetext,"%d",cp_saturation);
  muiChangeLabel(saturation_value,saturation_valuetext);
  muiSetHSValue(gamma_slider, (float) ((cp_gamma-cp_gamma_min)/
                              ((float) (cp_gamma_max-cp_gamma_min))) );
  sprintf(gamma_valuetext,"%d",cp_gamma);
  muiChangeLabel(gamma_value,gamma_valuetext);
  muiSetHSValue(gain_slider, (float) ((cp_gain-cp_gain_min)/
                             ((float) (cp_gain_max-cp_gain_min))) );
  sprintf(gain_valuetext,"%d",cp_gain);
  muiChangeLabel(gain_value,gain_valuetext);
  muiSetHSValue(shutter_slider, (float) ((cp_shutter-cp_shutter_min)/
                               ((float) (cp_shutter_max-cp_shutter_min))) );
  sprintf(shutter_valuetext,"%d",cp_shutter);
  muiChangeLabel(shutter_value,shutter_valuetext);

  muiSetHSValue(iris_slider, (float) ((cp_iris-cp_iris_min)/
                            ((float) (cp_iris_max-cp_iris_min))) );
  sprintf(iris_valuetext,"%d",cp_iris);
  muiChangeLabel(iris_value,iris_valuetext);

  muiSetHSValue(white1_slider, (float) ((cp_white1-cp_white1_min)/
                            ((float) (cp_white1_max-cp_white1_min))) );
  sprintf(white1_valuetext,"%d",cp_white1);
  muiChangeLabel(white1_value,white1_valuetext);

  muiSetHSValue(white2_slider, (float) ((cp_white2-cp_white2_min)/
                            ((float) (cp_white2_max-cp_white2_min))) );
  sprintf(white2_valuetext,"%d",cp_white2);
  muiChangeLabel(white2_value,white2_valuetext);

  muiSetHSValue(hue_slider, (float) ((cp_hue-cp_hue_min)/
                            ((float) (cp_hue_max-cp_hue_min))) );
  sprintf(hue_valuetext,"%d",cp_hue);
  muiChangeLabel(hue_value,hue_valuetext);
  
  muiSetHSValue(capturequality_slider, (float) ((cp_capturequality-cp_capturequality_min)/
                            ((float) (cp_capturequality_max-cp_capturequality_min))) );
  sprintf(capturequality_valuetext,"%d",cp_capturequality);
  muiChangeLabel(capturequality_value,capturequality_valuetext);

}


//main function begins here
int main (int argc, char **argv) {

  if( argc > 1) cameraIndex=atoi( argv[1] );

  //open grabber according to camera index
/*  if( cameraIndex== 0){ 
    if (!grabber) {
      grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0",
                                                    0x081443600000022e);
      //grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R1");
    }
  }
  else{
    if(!grabber){
      grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0",
                                                    0x08144360000001c2);
    }
  }
*/
  if( cameraIndex== 0){ 
    if (!grabber) {
      grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0",
                                                   DIG1394_FIRST_CAMERA);
    }
  }
  else{
    if(!grabber){
      grabber = new XVDig1394<XVImageRGB<XV_RGB> >(DC_DEVICE_NAME,"S1R0",
                                                   DIG1394_SECOND_CAMERA);
    }
  }

  if (!grabber) {
      cerr<<"Error: no framegrabber found!"<<endl;
    exit(1);
  }

  XVImageRGB<XV_RGB> image(640,480);
  window = new XVWindowX<XV_RGB> (grabber->frame(0));
  //window->setImages(&(grabber->frame(0)),grabber->buffer_count());
  window -> map();

  cp_handle = grabber->get_handle();
  cp_node_id = grabber->get_node_id();

  grabber->set_brightness_auto();
  grabber->set_exposure_auto();
  grabber->set_sharpness_auto();
  grabber->set_whitebalance_auto();
  grabber->set_saturation_auto();
  grabber->set_gamma_auto();
  grabber->set_gain_auto();
  grabber->set_shutter_auto();
  grabber->set_iris_auto();
  grabber->set_hue_auto();
  grabber->set_capturequality_auto();
 
  for(int i=0; i<60; i++) {
    grabber->current_frame_continuous();
    window->CopySubImage(grabber->frame(grabber->current_buffer_number()));
    window -> swap_buffers();
    window -> flush();
  }
  grabber->get_brightness(&cp_brightness);
  grabber->get_exposure(&cp_exposure);
  grabber->get_sharpness(&cp_sharpness);
  grabber->get_whitebalance(&cp_white1,&cp_white2);
  grabber->get_saturation(&cp_saturation);
  grabber->get_gamma(&cp_gamma);
  grabber->get_gain(&cp_gain);
  grabber->get_shutter(&cp_shutter);
  grabber->get_iris(&cp_iris);
  grabber->get_hue(&cp_hue);
  grabber->get_capturequality(&cp_capturequality);

  printf("Values: bright: %d\t Exp: %d\t Shapr:%d\t White1: %d\t White2: %d", 
	cp_brightness, cp_exposure, cp_sharpness, cp_white1, cp_white2 );
  printf("sat: %d\t gamma: %d\t gain: %d\t shutter: %d\t iris: %d\t",
	 cp_saturation, cp_gamma, cp_gain, cp_shutter, cp_iris);
  printf(" Hue: %d\t quality: %d\t", cp_hue, cp_capturequality);


  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_BRIGHTNESS,&cp_brightness_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_BRIGHTNESS,&cp_brightness_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_EXPOSURE,&cp_exposure_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_EXPOSURE,&cp_exposure_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_SHARPNESS,&cp_sharpness_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_SHARPNESS,&cp_sharpness_max);

  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_WHITE_BALANCE,&cp_white1_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_WHITE_BALANCE,&cp_white1_max);
  cp_white2_min=cp_white1_min;
  cp_white2_max=cp_white1_max;

//  printf("White Balance: %d %d %d\n", cp_white1_min, cp_white1_max, cp_white1);


  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_SATURATION,&cp_saturation_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_SATURATION,&cp_saturation_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_GAMMA,&cp_gamma_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_GAMMA,&cp_gamma_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_GAIN,&cp_gain_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_GAIN,&cp_gain_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_SHUTTER,&cp_shutter_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_SHUTTER,&cp_shutter_max);
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_IRIS,&cp_iris_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_IRIS,&cp_iris_max);

  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_HUE,&cp_hue_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_HUE,&cp_hue_max);
  
  dc1394_get_min_value(cp_handle,cp_node_id,FEATURE_CAPTURE_QUALITY,&cp_capturequality_min);
  dc1394_get_max_value(cp_handle,cp_node_id,FEATURE_CAPTURE_QUALITY,&cp_capturequality_max);

  printf("Iris: %d %d Hue %d %d Quality: %d %d\n", cp_iris_min, cp_iris_max, cp_hue_min,
	cp_hue_max, cp_capturequality_min, cp_capturequality_max);

  grabber->set_brightness_manual();
  grabber->set_exposure_manual();
  grabber->set_sharpness_manual();
  grabber->set_whitebalance_manual();
  grabber->set_saturation_manual();
  grabber->set_gamma_manual();
  grabber->set_gain_manual();
  grabber->set_shutter_manual();
  grabber->set_iris_manual();
  grabber->set_hue_manual();
  grabber->set_capturequality_manual(); 

  cp_auto_manual_mode = CP_MANUAL;

  grabber->set_brightness(cp_brightness);
  grabber->set_exposure(cp_exposure);
  grabber->set_sharpness(cp_sharpness);
  grabber->set_whitebalance(cp_white1,cp_white2);
  grabber->set_saturation(cp_saturation);
  grabber->set_gamma(cp_gamma);
  grabber->set_gain(cp_gain);
  grabber->set_shutter(cp_shutter);
  grabber->set_iris(cp_iris);
  grabber->set_hue(cp_hue);
  grabber->set_capturequality(cp_capturequality);

  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(340, 720);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
  glutCreateWindow("XVision2(CIRL) - Control Panel");
  make_ui();
  muiInit();
  glutIdleFunc(idle);
  glutTimerFunc(15,init_timer,0);
  glutMainLoop();

  return 0;
}
