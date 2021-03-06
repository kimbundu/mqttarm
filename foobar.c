#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <yajl/yajl_tree.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <MQTTAsync.h>

#include "mqtt.h"
#include "serialscomm.h"
#include "config.h"
#include "deviceboat.h"
#include "msgque.h"



comm_t	   g_comm;	//com port
//connection options
#define BUFFSIZE 64

equpment_t     equipmentdev;
mqttClient_t   g_myClient;

char 		   strcfgfilename[128];
char 		   strdevfilename[128];
char		   strmqttcfgFile[128];
char *		   equipmentid=NULL;
char *  	   strreginfo=NULL;
int			   g_Exit=0;
int 		   g_reged=0;
char pubNormalTopic[64];
char subNoramlTopic[64];
const char* pubRegistTopic="huachen/client";
const char* subRegistTopic="huachen/server";
static int up2ServerInfo = 0;

static int inWhatStatus = 0; //1 --unreg  2--reg
static int send_msg_id=-1;


Opts_t OPTS;
/*=
{
	"huachenidcc11",//,"huachenid238"
	1,
	NULL,//"test", //NULL, user
	NULL,//"huachen123456", //password
	"tcp://116.62.117.121:1883",//"tcp://192.168.10.45:1883",123.57.13.129:2883
	10000L,
	20,
	"huachen/server"
};*/

void msgque_Readthread(void* param)
{
	struct msgbuffer msg;
	key_t key;
		
	key = msgque_init(MSG_FILE,MSG_BUFFER_SIZE );
			
	while(1)
	{

		if( msgque_rev(key, &msg) <= 0 )
		{	
			sleep(2);
			continue;
		}
		printf("recv:msgid=%d,text=[%s] \n",msg.mtype,msg.mtext );
		//sleep(1);

	}
}



//read config file for mqtt connection
void Read_mqtt_Cfg()
{

	CONFIG *cfg = config_open(NULL);
    bool  succ = config_load(cfg, strmqttcfgFile);

	char* temp = config_get_value_string(cfg, "OPTIONS", "clientid", "huachenidcc11");
	OPTS.clientid = strdup(temp);
	
	temp = config_get_value_string(cfg, "OPTIONS", "qos", "1");
	OPTS.qos = atoi(temp);

	temp = config_get_value_string(cfg, "OPTIONS", "address", "tcp://116.62.117.121:1883");
	OPTS.address = strdup(temp);

	temp = config_get_value_string(cfg, "OPTIONS", "timeout", "10000");
	OPTS.timeout = atoi(temp);
	
	temp = config_get_value_string(cfg, "OPTIONS", "keepalive", "20");
	OPTS.keepalive = atoi(temp);

	temp = config_get_value_string(cfg, "OPTIONS", "topic", "huachen/server");
	OPTS.topic = strdup(temp);
	
	temp = config_get_value_string(cfg, "OPTIONS", "username", "");
	if( strlen(temp) > 0 )
		OPTS.username = strdup(temp);
	else
		OPTS.username = NULL;
	
	
	temp = config_get_value_string(cfg, "OPTIONS", "password", "");

	if( strlen(temp) > 0 )
		OPTS.password = strdup(temp);
	else
		OPTS.password = NULL;
	

	config_close(cfg);
	
	printf( "%s\n", OPTS.clientid);
	printf( "%d\n", OPTS.qos);
	printf( "%s\n", OPTS.address);
	printf( "%d\n", OPTS.timeout);
	printf( "%d\n", OPTS.keepalive);
	printf( "%s\n", OPTS.topic);

	if( OPTS.username != NULL )
		printf( "%s\n", OPTS.username);
	else
		printf("username:NULL\n");


	if( OPTS.password != NULL )
		printf( "%s\n", OPTS.password);
	else
		printf("password:NULL\n");
	
    
}
//read config file for equipmentid
void Read_cfg()
{
	CONFIG *cfg = config_open(NULL);
    bool  succ = config_load(cfg, strcfgfilename);	
	char* szreg = config_get_value_string(cfg, "OPTIONS", "IsReg", "0");
	char* eqid = config_get_value_string(cfg, "OPTIONS", "equipmentid", "");

	//printf( "---------------%d\n", strlen(eqid));
	if( eqid != NULL && strlen(eqid) > 5 )
	{
		if( equipmentid != NULL ) free( eqid);	
		equipmentid = (char*) malloc(strlen(eqid)+1);
		memcpy(equipmentid,eqid,strlen(eqid));
		equipmentid[strlen(eqid)] = 0;

	}

	g_reged = atoi(szreg);
    config_close(cfg);
}
//save config information for equipmentid
void Write_cfg()
{
	CONFIG *cfg = config_open(NULL);
	config_set_value_int(cfg, "OPTIONS", "IsReg", g_reged);
    config_set_value_string(cfg, "OPTIONS", "equipmentid", equipmentid);
	config_save(cfg, strcfgfilename);
    config_close(cfg);
	
}
void sHeart_beating_proc(char*info)
{


	char* strunreg = "{\"operateCode\":\"301\","
		             "\"equipmentNo\":" ;


	char mydata[128];
	
	if( equipmentid != NULL )
	{

		sprintf(mydata,"%s\"%s\"}",strunreg,equipmentid);
		send_mqttMsg(&g_myClient,pubNormalTopic,mydata ,strlen(mydata ));
   }
   else
   {
		printf( "equipmentid is NULL\n ");
   }
	
}
// register procedure for 101
//101 server 2 client information
void sReg_Equipment_proc(char * info)
{

	if( inWhatStatus != 2 ) return; //unreg
	
	yajl_val node,v;
	int len;
	char* myeqid=NULL;
	const char * path[] = { "equipmentNo", (const char *) 0 };
	node = yajl_tree_parse((const char *) info, NULL, 0);
	if( node != NULL )
	{
		v = yajl_tree_get(node, path, yajl_t_string);
		if(v)
		{
			len = strlen(YAJL_GET_STRING(v));
			if( len > 5 )
			{
				myeqid = malloc(len+1);
				strcpy(myeqid,YAJL_GET_STRING(v));

				if( equipmentid != NULL ) free( equipmentid );
				
				equipmentid = (char*) malloc(strlen(myeqid)+1);
				strcpy(equipmentid,myeqid);
				g_reged = 1;
				sprintf(subNoramlTopic,"huachen/server/%s", equipmentid);
				sprintf(pubNormalTopic,"huachen/client/%s", equipmentid);
				subs_mqttMsg(&g_myClient,subNoramlTopic); //test subscibe
				Write_cfg();
				
		
			}
		}	
	}
	
	//ajl_tree_free(node);
	//yajl_tree_free(v);
	inWhatStatus = 0;
	if( myeqid != NULL ) free(myeqid);
}
//102 server 2 client information
static void sUnreg_Equipment_proc(char * info)
{
	printf(" Unregister!\n");

	if( inWhatStatus != 1 ) return; //unreg
	
		
	if( x_getReturnInfo(info )  == 0 )
	{
		printf("Sucessful to Unregister!\n");
		unsubs_mqttMsg(&g_myClient,subNoramlTopic);
		free(equipmentid); equipmentid=NULL;
	    g_reged=0;
	    Write_cfg();
		
		
	}
	else
	{
		printf("Failed to Unregister!\n");
	}

	inWhatStatus = 0;
}

//103  server 2 client information

static void sStatus_uploading_proc(char * info)
{	
	cStatus_uploading();
}
//105 server 2 client information

static void sCtrl_EquipmentALL_proc(char * info)
{
	yajl_val node;
	yajl_val v;
	int len;
	char* userid = NULL;
	char* strreturn;
	
	const char * path[] = { "userId", (const char *) 0 };
	const char * path1[] = { "equipmentStatus", (const char *) 0 };
	
	node = yajl_tree_parse((const char *) info, NULL, 0);
	if( node != NULL )
	{
		v = yajl_tree_get(node, path, yajl_t_string);
		if(v)
		{
			len = strlen(YAJL_GET_STRING(v));
			if( len > 5 )
			{
				userid = malloc(len);
				strcpy(userid,YAJL_GET_STRING(v));
			}
		}

		v = yajl_tree_get(node, path1, yajl_t_string);
		if(v)
		{
			len = strlen(YAJL_GET_STRING(v));
			if( len > 0 )
			{
				equipmentdev.status = atoi( YAJL_GET_STRING(v) );
			}
		}
		

	}
				

		
	if ( userid != NULL )
	{	
	
		strreturn = getDevCtrl2Json(&equipmentdev,equipmentid,userid);
		send_mqttMsg(&g_myClient,pubNormalTopic,strreturn,strlen(strreturn));
		//free(strreturn);
	}

	//yajl_tree_free(v);
	//yajl_tree_free(node);

	if(userid != NULL)	free(userid);
}
//106 server 2 client information
static void sCtrl_EquipmentSub_proc(char * info)
{
	char* userid = NULL;
	char* childEqNo= NULL;
	char* childEqVal=NULL;
	char *strreturn;//[256];
	yajl_val node;
	yajl_val v;
	int len;

	const char * path[] = { "userId", (const char *) 0 };
	const char * path1[] = { "childEquipmentNo", (const char *) 0 };
	const char * path2[] = { "childEquipmentOpValue", (const char *) 0 };
	
	node = yajl_tree_parse((const char *) info, NULL, 0);
	
	if( node != NULL )
	{
		v = yajl_tree_get(node, path, yajl_t_string);
		if(v)
		{
			len = strlen(YAJL_GET_STRING(v));
			if( len > 5 )
			{
				userid = malloc(len);
				strcpy(userid,YAJL_GET_STRING(v));
			}
		}

		v = yajl_tree_get(node, path1, yajl_t_string);
		if(v)
		{
			childEqNo = YAJL_GET_STRING(v);
			printf("childEqNo:%s\n",childEqNo );
		}
		v= yajl_tree_get(node, path2,yajl_t_string);
		if(v)
		{	
			childEqVal = YAJL_GET_STRING(v);
		}

	}	

	if( childEqNo != NULL && userid != NULL )
	{


	  setsubDevValue( &equipmentdev,childEqNo,childEqVal );	

	  strreturn = getSubCtrl2Json(&equipmentdev,equipmentid,
	                               childEqNo,childEqVal,userid);	

		send_mqttMsg(&g_myClient,pubNormalTopic,strreturn,strlen(strreturn));
		//free(strreturn);
	}

   //yajl_tree_free(v);
   //yajl_tree_free(node);
   
   if(userid != NULL)		free(userid);
   if(childEqNo != NULL)	free(childEqNo);

}
//101 client 2 server
static void cReg_Equipment()
{
	//send register information to server
	if( strreginfo != NULL)
	{
		send_mqttMsg(&g_myClient,pubRegistTopic,strreginfo,strlen(strreginfo));
		inWhatStatus =2;
	}
}
//102  client 2 server
static void cUnreg_Equipment()
{
	char* strunreg = "{\"operateCode\":\"102\"}";

   if( equipmentid != NULL )
   {
	   send_mqttMsg(&g_myClient,pubNormalTopic,strunreg,strlen(strunreg));
	   inWhatStatus  = 1;
   }
	else
		printf( "equipmentid is NULL\n ");
}

//104  client 2 server
// uploading the information of device status
void cStatus_uploading()
{
	char *mystatus;//[512];

	if( equipmentid != NULL )
	{

		mystatus = subdevtostring(&equipmentdev, equipmentid,104);	
		send_mqttMsg(&g_myClient,pubNormalTopic,mystatus ,strlen(mystatus ));
		//free(mystatus);
		

	}
   else
   {
		printf( "equipmentid is NULL\n ");
   }
}
//202  client 2 server
// uploading the information of measurement
static void cData_uploading()
{
	char *mydata;//2];
	if( equipmentid != NULL )
	{
     
		mydata = getMeasure2Json(&equipmentdev,equipmentid,202);
		send_mqttMsg(&g_myClient,pubNormalTopic,mydata ,strlen(mydata ));
		
		
   }
   else
   {
		printf( "equipmentid is NULL\n ");
   }
		
}

//107 add sub deive
static void cAdd_SubDevice()
{
	char *mydata;
	if( equipmentid != NULL )
	{

		//mydata = getMeasure2Json(&equipmentdev,equipmentid,202);
		//send_mqttMsg(&g_myClient,pubNormalTopic,mydata ,strlen(mydata ));
		//free(mydata );
		//opcode,eqno,childEqNo,childname,childopv,childoptype
   }
   else
   {
		printf( "equipmentid is NULL\n ");
   }
		
}
//108 delete sub deive
static void cDel_SubDevice()
{
	char *mydata;//2];
	if( equipmentid != NULL )
	{

		;
		//mydata = getMeasure2Json(&equipmentdev,equipmentid,202);
		//send_mqttMsg(&g_myClient,pubNormalTopic,mydata ,strlen(mydata ));
		//free(mydata );
		//opcode: eqno: childEqNo:
   }
   else
   {
		printf( "equipmentid is NULL\n ");
   }
		
}
//mqtt procedure
static int RecvProc( char* data,char* topic, int len )
{

	int i,ret=0;
    int opcode=0;
	char* mybuf=NULL;
	int npos=0;
	mybuf = (char*)malloc(len+10);	
	printf("---MSG---\ntopic: %s\n", topic);
	printf("message:\n");
	/*for(i=0; i<len ; i++)
    {	
		mybuf[npos] = data[i];
		npos++;
	}*/
	memcpy(mybuf,data,len);
	mybuf[len]=0;	
	printf("%s",mybuf);
    printf("\n---END MSG---\n");

	yajl_val node,v;
	char errbuf[1024];
	const char * retpath[] ={ "ret", (const char *) 0 };
	const char * oppath[]  ={"operateCode", (const char *) 0 };
	node = yajl_tree_parse((const char *)mybuf, errbuf, sizeof(errbuf));
	if (node == NULL) 
	{       
		fprintf(stderr, "parse_error \n");
		if (strlen(errbuf)) 
			fprintf(stderr, " %s", errbuf);
        else fprintf(stderr, "unknown error");
		    fprintf(stderr, "\n");
        return 0;
    }	
	v = yajl_tree_get(node, retpath, yajl_t_string);
	if(v)
	{
		ret = atoi(YAJL_GET_STRING(v));
	}

	v = yajl_tree_get(node, oppath, yajl_t_string);
	if(v)
	{
		opcode = atoi(YAJL_GET_STRING(v));
	}	
	printf("opcode=%d,ret=%d\n",opcode,ret);
	if( strcmp(subRegistTopic,topic) == 0 )
	{
		//receive a messsgae for register
		if( opcode == 101 )
		{	
			sReg_Equipment_proc(mybuf);
		}
		else if(opcode == 102  )
		{
			sUnreg_Equipment_proc(mybuf);
		}
	}
	else if( strcmp(subNoramlTopic,topic) == 0 )
	{
		switch(opcode)
		{
			case 102:
				break;
			case 103:
				sStatus_uploading_proc(mybuf);
				break;
			case 104:
				//sStatus_uploading_proc(mybuf);
				break;
			case 105:
				sCtrl_EquipmentALL_proc(mybuf );
				break;
			case 106:
				sCtrl_EquipmentSub_proc(mybuf);
				break;
			case 301: //heart beating
				sHeart_beating_proc(mybuf);		
				break;
			default:
				printf("unknown opcode:%d \n",opcode);
		}	

	}

	if( mybuf!=NULL) free(mybuf);	

	senddata_com(&g_comm,data,len );    
	return 0;
}
// data procedure for serials port of my device

int findbuf( char* buf, char* submarker, int bufsize, int subbufsize )
{
	int i=0,j=0;
	for( i=0;i<bufsize-subbufsize-1;i++)
	{
		for(j=0;j<subbufsize;j++ )
		{
			if( *(buf+i+j) != *( submarker+j ) )
				break;
		}
		if( j == subbufsize )
		{
			return i;				
		}

	}

	return -1;

}

int findbuf2( char* buf, char submarker, int bufsize)
{
	int i=0;
	for( i=0;i<bufsize;i++)
	{

		if( *(buf+i) == submarker ) 
		{
			return i;				
		}
	}
	return -1;
}

static   char  my_buffer[1024];
static   int   npos=0;
#define MIN_PKG_SIZE  (10)
static int serials_dataproc(char* data, int len )
{   
	int markpos=-1;
	short pkgsize=0;
	char  pkgcmd=0;
    //send_mqttMsg(&g_myClient,pubTopic,data,len);
	//printf( "comrecv:%s\n", data );

	if( npos + len < 1023 )
	{
		memcpy( my_buffer + npos, data, len );
	}
	else
	{
		memset(my_buffer,0,sizeof(my_buffer) );
	}

	if( npos > MIN_PKG_SIZE )
	{

		markpos = findbuf2( my_buffer, 0x2E, npos );

		if( markpos >=0  && markpos +  MIN_PKG_SIZE <= npos )
		{

			//get sizeof package
			memcpy(&pkgsize,my_buffer+markpos+1,2);

			if( markpos + pkgsize + 1 < npos )
			{
				//get command of package
				pkgcmd=*(my_buffer+markpos+3);
				memmove( my_buffer, my_buffer+markpos+pkgsize, npos - markpos - pkgsize - 1    );
				npos = npos - markpos - pkgsize - 1;
			}


		}

	}

	return 0;
}

//timer for uploading the inforamtion 
static void TimerFun_sending_status(int signo)
{
	//printf    
	up2ServerInfo = 1;	
	cData_uploading();	
	up2ServerInfo = 0;
}

static void Start_ProcTimer()
{

	struct itimerval tick;	
    signal(SIGALRM, TimerFun_sending_status);
    memset(&tick, 0, sizeof(tick));
    //Timeout to run first time
    tick.it_value.tv_sec = 3000;
    tick.it_value.tv_usec = 0;
    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 3000;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
        printf("Set timer failed!\n");
	}
}


static int get_command()
{
    fd_set rfds;
    struct timeval tv;
    int ch = 0,n=0;
	char buf_show[BUFFSIZE];
	
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10; //timeout
    //any key
	
    if (select(1, &rfds, NULL, NULL, &tv) > 0)
    {
		n = read(STDIN_FILENO, buf_show, BUFFSIZE);
		buf_show[n-1]=0;		
		msgque_send(send_msg_id, 123, buf_show ,strlen(buf_show ) );
		if( strcmp(buf_show,"101" ) ==0 )
		{			
			cReg_Equipment();
		}
		else if( strcmp(buf_show,"102") == 0 )
        {
			cUnreg_Equipment();			
		}
		else if( strcmp(buf_show,"104" ) == 0)
        {
			cStatus_uploading();
		}
		else if( strcmp(buf_show,"202" ) == 0 )
        {
			 cData_uploading();
		}
		else if( strcmp(buf_show,"301" ) == 0 )
        {
			sHeart_beating_proc(NULL);		
		}
		else if( strcmp(buf_show,"exit" ) ==0 ||
		         strcmp(buf_show,"q" ) ==0)
        {
			printf( "exit\n");
			exit(1);
		}
		else
		{
			printf("Unknown Command!\n");
			
		}
	
	}

    return 0;
}

int main(int argc, char* argv[])
{

	char buf_show[BUFFSIZE];
	int n;   
	char pwdbuf[80];
	key_t key;
	
	pthread_t msg_threadid;	
	


	if( argc < 2 ) 
	{
		printf("usage: foobar /home/root\n");
		return 0;		
	}
	strcpy( pwdbuf,argv[1]);
	//getcwd(pwdbuf, sizeof(pwdbuf));
    printf("current working directory : %s\n", pwdbuf);

	sprintf( strcfgfilename, "%s/%s", pwdbuf,"device.config");
	sprintf( strdevfilename, "%s/%s", pwdbuf,"device.description");
	sprintf( strmqttcfgFile, "%s/%s", pwdbuf, "mqtt.config");
	
	strreginfo = getRegInfo(&equipmentdev,strdevfilename);
	printf("%s\n",strreginfo);
	
//read config file and initializing parameters
	Read_cfg();
	Read_mqtt_Cfg();
	

	printf( "******Isreg=%d,equipmentid=%s********\n",g_reged ,equipmentid);

	
	g_myClient.recvFunction=RecvProc;
	g_myClient.pOPTS=&OPTS;
	g_myClient.disc_finished=0;
	g_myClient.subscribed=0;
	g_myClient.finished=0;

	g_comm.devname="/dev/ttymxc3";
	g_comm.devfd=-1;
	g_comm.baut=115200;
	g_comm.status=0;
	g_comm.recvFunction = serials_dataproc;


// open serials port for device's communication
	if( open_com(&g_comm) !=0 )
	{	
	   printf("Error Opening Comm!\n");	
	}

	key = msgque_init(MSG_FILE,MSG_BUFFER_SIZE + 1 );
	send_msg_id = msgque_getmsgid(key);
	printf( "msg_key=%d,msg_id=%d\n",key, send_msg_id);
	sleep(1);
	pthread_create(&msg_threadid,NULL,(void*)msgque_Readthread,NULL);


	

	
//connecting to mqtt server!	
myconnection:
	
	init_mqtt(&g_myClient);
	sleep(2);
	while( !MQTTAsync_isConnected(g_myClient.gclient) )
	{
		//
		printf( "try to connecting again!\n  ");
		init_mqtt(&g_myClient);
		sleep(2);
	}
	

//subsribe the topic for registered device	
	if( equipmentid!=NULL)
	{
		sprintf(subNoramlTopic,"huachen/server/%s", equipmentid);
		sprintf(pubNormalTopic,"huachen/client/%s", equipmentid);
		subs_mqttMsg(&g_myClient,subNoramlTopic); 
		Start_ProcTimer();
	}
	//send register information always
	cReg_Equipment();
	sleep(2);
	
//try connecting when lost

	for(;;)
	{
		
		if( !MQTTAsync_isConnected(g_myClient.gclient) )
		{
			goto myconnection;
		}
		//write(STDOUT_FILENO, ":", 1);
		get_command();

		//printf("%d\n\n",a);
		
		sleep(1);
	}

//command mode for debug
	write(STDOUT_FILENO, ":", 1);
	while ((n = read(STDIN_FILENO, buf_show, BUFFSIZE)) > 0)
    {
		buf_show[n-1]=0;		
		
		if( strcmp(buf_show,"101" ) ==0 )
		{			
			cReg_Equipment();
		}
		else if( strcmp(buf_show,"102") == 0 )
        {
			cUnreg_Equipment();			
		}
		else if( strcmp(buf_show,"104" ) == 0)
        {
			cStatus_uploading();
		}
		else if( strcmp(buf_show,"202" ) == 0 )
        {
			scData_uploading();		
		}
		else if( strcmp(buf_show,"301" ) == 0 )
        {
			sHeart_beating_proc(NULL);		
		}		
		else if( strcmp(buf_show,"exit" ) ==0 ||
		         strcmp(buf_show,"q" ) ==0)
        {
			printf( "exit\n");
			 exit(1);
		}
		else
		{
			printf("Unknown Command!\n");
		}
		write(STDOUT_FILENO, ":", 1);        
	}

// destroy all
	if( strreginfo != NULL )
	{
		free(strreginfo );
	}
	close_com(&g_comm);
	deinit_mqtt(&g_myClient);
	return 0;
}


