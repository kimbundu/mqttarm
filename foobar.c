#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h> 
#include <unistd.h>
#include <yajl/yajl_tree.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <MQTTAsync.h>
#include "mqtt.h"
#include "serialscomm.h"
#include "config.h"
#include "deviceboat.h"



comm_t	   g_comm;	//com port
//connection options
#define BUFFSIZE 64
static int myx1value=0;
static int myy1value=100;

equpment_t     equipmentdev;
char 		   strcfgfilename[128];
char 		   strdevfilename[128];
int 		   g_reged=0;
char *		   equipmentid=NULL;
char *  	   strreginfo=NULL;
int			   g_Exit=0;
mqttClient_t   g_myClient;


char pubNormalTopic[64];
char subNoramlTopic[64];
const char* pubRegistTopic="huachen/client";
const char* subRegistTopic="huachen/server";
static int up2ServerInfo = 0;

Opts_t OPTS=
{
	"huachenidcc",
	1,
	NULL,//"test", //NULL, user
	NULL,//"huachen123456", //password
	"tcp://123.57.13.129:2883",//"tcp://192.168.10.45:1883",
	10000L,
	20,
	"huachen/server"
};

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

// register procedure for 101
//101 server 2 client information
void sReg_Equipment_proc(char * info)
{
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
	if( myeqid != NULL ) free(myeqid);
}
//102 server 2 client information
static void sUnreg_Equipment_proc(char * info)
{
	printf(" Unregister!\n");
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
	char strreturn[256];
	const char * path[] = { "userId", (const char *) 0 };
	
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

	}
				

		
	if ( userid != NULL )
	{	

		sprintf( strreturn,
							"{\"ret\":\"0\","
							"\"desc\":\"OK\","
							"\"operateCode\":\"105\","
							"\"equipmentNo\":\"%s\","
							"\"equipmentStatus\":\"1\","
							"\"userId\":\"%s\"}", equipmentid, userid);
	
	
		send_mqttMsg(&g_myClient,pubNormalTopic,strreturn,strlen(strreturn));
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
	char strreturn[256];
	yajl_val node;
	yajl_val v;
	int len;
	int mvalue;

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
			mvalue = atoi(YAJL_GET_STRING(v));

			printf("value:%s\n",YAJL_GET_STRING(v) );
		}

		if( strcmp( childEqNo,"X1" ) == 0 )
		{
			myx1value=mvalue;
		}
		else if( strcmp( childEqNo,"Y1" ) == 0 )
		{
			myy1value=mvalue;
		}

		

	}

	

	if( childEqNo != NULL && userid != NULL )
	{
	
		sprintf( strreturn, "{\"ret\":\"0\","
							"\"desc\":\"OK\","
							"\"operateCode\":\"106\","
							"\"equipmentNo\":\"%s\","
							"\"childEquipmentNo\":\"%s\","
							"\"childEquipmentOpValue\":\"%d\","
							"\"userId\":\"%s\"}", equipmentid, childEqNo,mvalue,userid );

		send_mqttMsg(&g_myClient,pubNormalTopic,strreturn,strlen(strreturn));
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
	}
}
//102  client 2 server
static void cUnreg_Equipment()
{
	char* strunreg = "{\"operateCode\":\"102\"}";

   if( equipmentid != NULL )
   {
	   send_mqttMsg(&g_myClient,pubNormalTopic,strunreg,strlen(strunreg));

	  
   }
	else
		printf( "equipmentid is NULL\n ");
}

//0~100 random
static int r(int fanwei)
{
 	srand((unsigned)time(NULL)); 
  	return rand()%fanwei;  
}

// uploading the information of device status
void cStatus_uploading()//104  client 2 server
{
	char mystatus[512];

	if( equipmentid != NULL )
	{
	
		sprintf(mystatus,
				"{"
				"\"operateCode\":\"104\","
				"\"equipmentNo\":\"%s\","
				"\"equipmentName\":\"%s\","
				"\"equipmentType\":\"2\","
				"\"company\":\"%s\"," 
				"\"equipmentStatus\":\"1\","
				"\"equipmentViewData\":\"NONE\","
				"\"userId\":\"0000\","
				"\"childEquipmentData\":["
				"{\"childEquipmentNo\":\"B1\","
				 "\"childEquipmentOpValue\":\"1\"},"
		        "{\"childEquipmentNo\":\"X1\","
				  "\"childEquipmentOpValue\":\"%d\"},"
		        "{\"childEquipmentNo\":\"Y1\","
				 "\"childEquipmentOpValue\":\"%d\"},"
				"{\"childEquipmentNo\":\"B2\","
				"\"childEquipmentOpValue\":\"1\"}]"
				"}", 
		        equipmentid, equipmentdev.name,equipmentdev.company, 
		        myx1value,myy1value);
		
	
	
		send_mqttMsg(&g_myClient,pubNormalTopic,mystatus ,strlen(mystatus ));
	}
   else
   {
		printf( "equipmentid is NULL\n ");
   }
}

// uploading the information of measurement
static void cData_uploading()//202  client 2 server
{

	char localstr[64];	
	char mydata[512];

	double y=31.4039070000 + r(10)*0.001;
	double x=121.0287980000+ r(10)*0.001;
	
	float ph=7.5+r(4);
	float cod=20.2+r(10);
	float mdo=0.8+r(25);

	sprintf( localstr,"local%d", r(1000) );
	

	if( equipmentid != NULL )
	{
		
		sprintf(mydata,
				"{"
				"\"operateCode\":\"202\","
				"\"equipmentNo\":\"%s\","
				"\"equipmentName\":\"%s\","
				"\"temperature\":\"20\","
				"\"weather\":\"\","
				"\"remarks\":\"OK!\","
				"\"locationName\":\"%s\","
				"\"longitude\":\"%g\","
				"\"latitude\":\"%g\","
				"\"monitorData\":["
				"{\"quotaName\":\"pH\","
				 "\"position\":\"1\","
				 "\"quotaValue\":\"%.1f\"},"
		        "{\"quotaName\":\"DO\","
				 "\"position\":\"1\","
				 "\"quotaValue\":\"%g\"},"
				"{\"quotaName\":\"COD\","
				 "\"position\":\"1\","
				 "\"quotaValue\":\"%g\"}]"
				 "}", equipmentid,equipmentdev.name,
		        localstr,x,y,ph,mdo,cod);              

	
		send_mqttMsg(&g_myClient,pubNormalTopic,mydata ,strlen(mydata ));
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
	}
	else if( strcmp(subNoramlTopic,topic) == 0 )
	{
		switch(opcode)
		{
			case 102:
				sUnreg_Equipment_proc(mybuf);
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
			default:
				printf("unknown opcode:%d \n",opcode);
		}	

	}

	if( mybuf!=NULL) free(mybuf);	
	//senddata_com(&g_comm,data,len );    
	return 0;
}
// data procedure for serials port of my device
static int serials_dataproc(char* data, int len )
{           
    //send_mqttMsg(&g_myClient,pubTopic,data,len);
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
    tick.it_value.tv_sec = 600;
    tick.it_value.tv_usec = 0;
    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 600;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
        printf("Set timer failed!\n");
	}
}


int main(int argc, char* argv[])
{

	char buf_show[BUFFSIZE];
	int n;   
	char pwdbuf[80];
    getcwd(pwdbuf, sizeof(pwdbuf));
    printf("current working directory : %s\n", pwdbuf);

	sprintf( strcfgfilename, "%s/%s", pwdbuf,"device.config");
	sprintf( strdevfilename, "%s/%s", pwdbuf,"device.description");
	
	strreginfo = getRegInfo(&equipmentdev,strdevfilename);
	printf("%s\n",strreginfo);
	
//read config file and initializing parameters
	Read_cfg();
	printf( "******Isreg=%d,equipmentid=%s********\n",g_reged ,equipmentid);
	
	g_myClient.recvFunction=RecvProc;
	g_myClient.pOPTS=&OPTS;
	g_myClient.disc_finished=0;
	g_myClient.subscribed=0;
	g_myClient.finished=0;

	g_comm.devname="/dev/ttymxc1";
	g_comm.devfd=-1;
	g_comm.baut=9600;
	g_comm.status=0;
	g_comm.recvFunction = serials_dataproc;

// open serials port for device's communication
	if( open_com(&g_comm) !=0 )
	{	
	   printf("Error Opening Comm!\n");	
	}

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

		sleep(2);
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
			 cData_uploading();
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

