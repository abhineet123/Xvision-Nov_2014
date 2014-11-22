enum {RECT_OBJECT=1,CIRC_OBJECT};

#define SCALE(x) ((x))

typedef struct{
  int	type;
  int	posx,posy;
  int	size1,size2;
  int   color;
  int   moving, length;
}HCIObject;
