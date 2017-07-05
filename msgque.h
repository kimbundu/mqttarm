#ifndef _MSGQUE_COMM_H_
#define _MSGQUE_COMM_H_

#include <sys/msg.h>   

#define MSG_FILE 	 "/tmp"
#define MSG_BUFFER_SIZE  (512)            
#define PERM 		 (S_IRUSR|S_IWUSR)  

struct msgbuffer
{	
	int  mtype;
	char mtext[MSG_BUFFER_SIZE+1];
};
//typedef struct msgbuffer msgbuf;

#ifdef __cplusplus
extern "C" {
#endif


	


	


key_t msgque_init(char *path, int keyv);
int   msgque_getmsgid( key_t key);
int  msgque_send( int msgid,int id, char* msgbuf, int msglen );
int  msgque_rev( key_t key,struct msgbuffer * msg);
void msgque_del( int msgid );


#ifdef __cplusplus
}
#endif
#endif