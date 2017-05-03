#ifndef _DEVICEBOAT_COMM_H_
#define _DEVICEBOAT_COMM_H_

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
	int 	nmeasuresize;
	char**  subdevname;
    char**  subdevvalue;
	double	longtitude;
	double  latitude;
	char**  measurename;
	char**	measurevalue;
	
}equpment_t;
	
char * getRegInfo( equpment_t* eq,char* filename );
char * subdevtostring( equpment_t* eq,char* eqno, int opcode );
char * getsubDevValue( equpment_t* eq,char* subdevNO );
char * setsubDevValue( equpment_t* eq,char* subDevNO, char* subvalue );
char * getMeasure2Json(equpment_t* eq,char* eqno,int opcode);
char * getSubCtrl2Json(equpment_t* eq,char* eqno,char* subDevNO, char* subvalue , char * userid);


	
#ifdef __cplusplus
}
#endif

#endif