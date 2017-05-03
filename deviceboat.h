#ifndef _DEVICEBOAT_COMM_H_
#define _DEVICEBOAT_COMM_H_
#include "./cstl/inc/c_lib.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ID_SIZE		  (16)
#define MAX_LOCATION_SIZE (16)
	
typedef struct  equpment_struct
{
	char	name[32];
	char	company[32];
	int 	type;
	int		status;
	int		nsubsize;
	struct  clib_map* subdev;
	
}equpment_t;
	
char * getRegInfo( equpment_t* eq,char* filename );
//char * subdevtostring( equpment_t* eq, char* eqno, int opcode );

	
#ifdef __cplusplus
}
#endif

#endif