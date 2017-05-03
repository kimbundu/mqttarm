#ifndef _SERIALS_COMM_H_
#define _SERIALS_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef  int (*SerialsRecvCallback)(char*, int );

typedef struct comm_struct
{
  SerialsRecvCallback recvFunction;
  int 	devfd;	
  int 	baut;
  char* devname;
  int   status;
  	 	
}comm_t;
	
int  open_com( comm_t* mycom);
int  close_com( comm_t* mycom);
int senddata_com(comm_t* mycom,char* strmsg,int strlen );

#ifdef __cplusplus
}
#endif

#endif
