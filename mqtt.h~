#ifndef _MQTT_COMM_H_
#define _MQTT_COMM_H_
#include <MQTTAsync.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef  int (*ProcCallback)( char*,char*, int);

typedef struct opts_struct
{
	char* clientid;
	int   qos;
	char* username;
	char* password;
	char* address;
	int   timeout;
	int   keepalive;
	char* topic;

}Opts_t;

typedef struct mqttclient_struct
{
	ProcCallback recvFunction;
	Opts_t 		 *pOPTS;
	MQTTAsync    gclient;
	int 	     disc_finished;//0
	int 	     subscribed;//0
	int 	     finished;//0
	int 		 connected;

}mqttClient_t;
	
int  deinit_mqtt( mqttClient_t *mqtt);
int  init_mqtt  ( mqttClient_t *mqtt);
void unsubs_mqttMsg(mqttClient_t *mqtt,const char* topicname);
void subs_mqttMsg(mqttClient_t *mqtt,const char* topicname);
void send_mqttMsg(mqttClient_t *mqtt,const char* topicname, char* strmsg, int len );
#ifdef __cplusplus
}
#endif
#endif
