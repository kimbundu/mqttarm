#ifndef _MSGQUE_COMM_H_
#define _MSGQUE_COMM_H_

#include <sys/msg.h>   

#define BUFFER_SIZE  (512)            
#define PERM 		 (S_IRUSR|S_IWUSR)  

struct msgbuffer
{	
	int  mtype;
	char mtext[BUFFER_SIZE+1];
};
//typedef struct msgbuffer msgbuf;

#ifdef __cplusplus
extern "C" {
#endif

//#define MSG_FILE 	 "/tmp"
	


	


key_t msgque_init(char *path, int keyv, int iserver);
int  msgque_send( key_t key,int msgid, char* msgbuf, int msglen );
int  msgque_rev( key_t key,struct msgbuffer * msg);
void msgque_del();


#ifdef __cplusplus
}
#endif
#endif