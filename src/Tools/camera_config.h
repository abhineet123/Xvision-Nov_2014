#ifndef __camera_config_h
#define __camera_config_h

typedef enum{
  FILE_INTERFACE,FIREWIRE_INTERFACE,RPC_INTERFACE
}InterfaceType;

typedef struct{
  int		type;
  char	        mask[80];
  int	        start_index,end_index;
}FileStream;

typedef struct{
  char 	       device[30];
  char	       mode[80];
  int	       auto_control;
}FirewireStream;
  

typedef struct{
  float		f[2];
  float		C[2];
  float		kappa[4];
}CameraParams;

typedef struct{
  int		  width,height;
  InterfaceType   type;
  FileStream	  file[2];
  FirewireStream  camera[2];
  float		  extrinsics[12];
  int		  offset;
  int		  disparity_range;
  int		  num_points;
  int		  num_cameras;
  float		  *point;
  int		  motion;
  int		  visualization;
  CameraParams	  camera_params[3];
}Config;

#endif
