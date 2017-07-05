
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>   
#include "msgque.h"

key_t msgque_init( char *path, int keyv, int iserver)
{
	//create key
	key_t key;
	//if ((key=ftok(MSG_FILE,BUFFER_SIZE))==-1)
	if ( ( key=ftok(path,keyv)) == -1 )
	{
	 	fprintf(stderr, "ftok:%s\n", strerror(errno));
	 	//exit(1);
		return -1;
	}
	else
	{
		printf("generate a key=%d\n", key);
		return key;
	}
    
    
}

int msgque_send( key_t key, int id, char* msgbuf, int msglen )
{
	struct msgbuffer msg;	
	int retv=-1;
	int msgsize;

	int msgid = -1;
	
	//get msgid
	if( (msgid = msgget(key,IPC_EXCL )) == -1 )
	{   
	    //create message
		if ((msgid=msgget(key,PERM|IPC_CREAT))==-1)
		{
			fprintf(stderr, "msgget:%s\n", strerror(errno));
		 	//exit(1);
			return -1;
		}
	} 


	
	

	msgsize = msglen<BUFFER_SIZE?msglen:BUFFER_SIZE;
	
   // printf("len:%d\n",msgsize);           
    memset(msg.mtext,0,sizeof(msg.mtext));
    msg.mtype = id;
    memcpy( msg.mtext,msgbuf,msgsize);

	
      
    retv = msgsnd(msgid,&msg,sizeof(struct msgbuffer),IPC_NOWAIT);
    if( retv < 0 )
    {           
        fprintf(stderr, "send err!\n");
        return -1;
    }

	return 0;
}

int msgque_rev(key_t key, struct msgbuffer * msg )
{
	int ret_value;
	int msgid = -1;
	//get msgid
	msgid = msgget( key, IPC_EXCL );
	if(msgid < 0)
	{  
	 	printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));  
	 	return -1; 
	}  

	ret_value = msgrcv(msgid,msg,sizeof(struct msgbuffer),0,0);
	if( ret_value >0 )
	{
		//printf("msgid=%d,text=[%s] \n",msg->mtype,msg->mtext );
		return ret_value;
	}

	return -1;
}


void msgque_del()
{
    //delete msg
	//if( msgid > 0 )
    //	msgctl(msgid,IPC_RMID,0);
}

