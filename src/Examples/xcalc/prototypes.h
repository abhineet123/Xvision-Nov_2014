enum {RECT_OBJECT=1,CIRC_OBJECT};

#define SCALE(x)	((x))

#define KEY_PLUS	11
#define KEY_MINUS	9
#define KEY_EQUAL	15
#define KEY_MUL		3
#define KEY_DPOINT	13
#define KEY_NEG		14

typedef struct{
  int	type;
  int	posx,posy;
  int	size1,size2;
  int   color;
  int   moving, length;
}HCIObject;
