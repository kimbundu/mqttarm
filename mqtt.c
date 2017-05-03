#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTAsync.h>

#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif
#include "mqtt.h"


static MQTTAsync_connectOptions g_conn_opts = MQTTAsync_connectOptions_initializer;


volatile  MQTTAsync_token deliveredtoken;

void connlost(void *context, char *cause)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;

	mqttClient->connected=0;
	//MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
	//printf("Reconnecting\n");
	
	//conn_opts.keepAliveInterval = 20;
	//conn_opts.cleansession = 1;
	/*if ((rc = MQTTAsync_connect(mqttClient->gclient, &g_conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
	    mqttClient->finished = 1;
	}*/
}


int msgarrvd(void *context, char *topicName, int topicLen, 
             MQTTAsync_message *message)
{   
	mqttClient_t* mqttClient = (mqttClient_t*)context;

	if( mqttClient->recvFunction != NULL )
	{
		mqttClient->recvFunction( (char*)message->payload,topicName,message->payloadlen);
	}
	
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;
	printf("Successful disconnection\n");
	mqttClient->disc_finished = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;
	printf("Subscribe succeeded\n");
	mqttClient->subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
	mqttClient->finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	mqttClient->finished = 1;
}
void onSend(void* context, MQTTAsync_successData* response)
{
	//mqttClient_t* mqttClient = (mqttClient_t*)context;
		
/*	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;
	

	printf("Message with token value %d delivery confirmed\n", response->token);

	opts.onSuccess = onDisconnect;
	opts.context = client;

	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
*/
}

void unsubs_mqttMsg(mqttClient_t *mqtt,const char* topicname)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;
	opts.context = mqtt;

	if ((rc = MQTTAsync_unsubscribe(mqtt->gclient, topicname,&opts)) != 
	    MQTTASYNC_SUCCESS)
	{
		printf("Error on unsubscrib!\n");
	}

}

void subs_mqttMsg(mqttClient_t *mqtt,const char* topicname)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;
	opts.context = mqtt;

	if ((rc = MQTTAsync_subscribe(mqtt->gclient, topicname, 
	                              1, &opts)) != 
	    MQTTASYNC_SUCCESS)
	{
		printf("Error on subscrib!\n");
	}

}

void onConnect(void* context, MQTTAsync_successData* response)
{
	mqttClient_t* mqttClient = (mqttClient_t*)context;
	//MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	//MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	printf("Successful connection\n");

	//g_connected = true;

	//printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
    //      "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = mqttClient;

	deliveredtoken = 0;
	mqttClient->connected=1;

	if ((rc = MQTTAsync_subscribe(mqttClient->gclient, mqttClient->pOPTS->topic, 
	                              mqttClient->pOPTS->qos, &opts)) != 
	    MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		//exit(EXIT_FAILURE);
	}

	
}




void send_mqttMsg(mqttClient_t *mqtt,const char* topicname,  char* strmsg, int len )
{
	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	
	if(!MQTTAsync_isConnected(mqtt->gclient)  )
	{
		printf("Isn't connected!\n");
		return;
	}
	
	
	printf("************send msg***************\n");
	printf("     TOPIC=%s\n", topicname);
	printf("     Message:\n");
	printf("  %s\n", strmsg);
	printf("************end send msg***************\n");
	
	
	
	opts.onSuccess = onSend;
	opts.context = mqtt->gclient;
	pubmsg.payload = strmsg;
	pubmsg.payloadlen = len;//strlen(strmsg);
	pubmsg.qos = mqtt->pOPTS->qos;
	pubmsg.retained = 0;
	deliveredtoken = 0;

	
	if ((rc = MQTTAsync_sendMessage(mqtt->gclient, topicname, &pubmsg, &opts)) != 
	    	MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		//printf("client:%d\n", &mqtt.gclient);		
		//exit(EXIT_FAILURE);
	}

	
}

int init_mqtt( mqttClient_t *mqtt)
{


	//g_pOPTS = popts;

	int rc;
	//MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	
	MQTTAsync_create(&mqtt->gclient, mqtt->pOPTS->address, mqtt->pOPTS->clientid, 
	                 MQTTCLIENT_PERSISTENCE_NONE, NULL);
	
	MQTTAsync_setCallbacks(mqtt->gclient, mqtt, connlost, msgarrvd, NULL);	
	
	g_conn_opts.keepAliveInterval = mqtt->pOPTS->keepalive;
	g_conn_opts.cleansession = 1;
	g_conn_opts.onSuccess = onConnect;
	g_conn_opts.onFailure = onConnectFailure;
	g_conn_opts.context =   mqtt;
	g_conn_opts.username=mqtt->pOPTS->username; 
	g_conn_opts.password=mqtt->pOPTS->password;

	if ((rc = MQTTAsync_connect(mqtt->gclient, &g_conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		return -1;
	}


	/*while	(!mqtt->subscribed)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	if (mqtt->finished)
		return -2;

*/
	return 0;

}

int deinit_mqtt( mqttClient_t *mqtt)
{
	MQTTAsync_disconnectOptions disc_opts = 
		MQTTAsync_disconnectOptions_initializer;
	int rc;
	disc_opts.onSuccess = onDisconnect;
	disc_opts.context =   mqtt;
	
	if ((rc = MQTTAsync_disconnect(mqtt->gclient, &disc_opts)) != 
	    MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		return -1;
	}
 	while(!mqtt->disc_finished)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
    
	MQTTAsync_destroy(&mqtt->gclient);
 	return 0;
}
