#define MRT_GET_STATUS		0
#define MRT_SET_STATUS		1
#define MRT_SET_REGISTER	2
#define MRT_SET_BT819		3
#define MRT_GET_BT819		4
#define MRT_HOME_CURSOR		5
#define MRT_NEXT_LINE		6
#define MRT_CAPTURE		7
#define MRT_VSYNC		8   /* MRT_HOME_CURSOR is called implicitly*/
#define MRT_GET_DATA		15

/* status bits */

#define CAPTURE_FLAG		(1<<7)
#define FIELD_FLAG		(1<<6)
#define MONO_BIT		(1<<5)
#define CAPTURE_BIT		(1<<4)
#define FRAME_BIT		(1<<3)

#define HOME_CURSOR		(1<<7)
#define NEXT_CURSOR		(1<<6)
