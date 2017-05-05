#include "deviceboat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <yajl/yajl_tree.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

static int measureSum = 5;
static char* QuoName[] = { "COD", "pH", "DO", "BOD", "TOC" };
static float QueValue[] = { 12.2, 5.6, 12.5, 23.6, 44.5 };


//0~100 random
static int r(int fanwei)
{
 	srand((unsigned)time(NULL)); 
  	return rand()%fanwei;  
}



char * getsubDevValue( equpment_t* eq, char* subdevNO )
{
	int i;
	int npos=-1;

	for( i=0;i<eq->nsubsize;i++ )
	{
		if ( strstr( eq->subdevname[i], subdevNO ) - eq->subdevname[i] == 0 )
		{
			npos = i;
			break;
		}
			
	}

	if( npos >=0 && npos < eq->nsubsize )
	{
		return eq->subdevvalue[npos];
	}


	return NULL;
}
char * setsubDevValue(  equpment_t* eq,char* subDevNO, char* subvalue )
{
	int i;
	int npos=-1;

	for( i=0;i<eq->nsubsize;i++ )
	{
		if ( strstr( eq->subdevname[i], subDevNO ) - eq->subdevname[i] == 0 )
		{
			npos = i;
			break;
		}
			
	}

	if( npos >=0 && npos < eq->nsubsize )
	{
		
		free(eq->subdevvalue[npos]);
		eq->subdevvalue[npos] = (char*)malloc(strlen(subvalue)+1 );
		strcpy(eq->subdevvalue[npos],subvalue);
		return eq->subdevvalue[npos];
	}

	return NULL;
}


typedef struct yajl_info_t  
{ 
    int depth;  
    yajl_gen gen; //保存一个generator  
} yajl_info;  


void yajl_init(yajl_info *info) 
{  
    info->depth = 0;  
    info->gen = yajl_gen_alloc(NULL);  
/*
#ifdef BEAUTIFY_OUTPUT  
    //设置输出格式  
    yajl_gen_config(info->gen, yajl_gen_beautify, 1);  
#endif*/  
}  
  
void yajl_info_free(yajl_info *info) 
{  
    info->depth = 0;  
    yajl_gen_free(info->gen);  
}  


yajl_gen_status begin_map(void *ctx)  
{  
    yajl_info *info = (yajl_info *)ctx;  
    yajl_gen_status __stat = yajl_gen_map_open(info->gen);  
    if (__stat == yajl_gen_status_ok)  {  
        info->depth++ ;  
    } else {  
        printf("yajl_gen_map_open failed\n");  
    }  
    return __stat;  
} 

yajl_gen_status end_map(void *ctx)  
{  
    yajl_info *info = (yajl_info *)ctx;  
    yajl_gen_status __stat = yajl_gen_map_close(info->gen);  
    if (__stat == yajl_gen_status_ok)  {  
        info->depth-- ;  
    } else {  
        printf("yajl_gen_map_close failed\n");  
    }  
    return __stat;  
}
  
/* 
 * begin_map和end_map会生成一个数组 
 * 在它们直接插入数据的话，会变成数组中的内存 
 */  
yajl_gen_status begin_array(void *ctx)  
{  
    yajl_info *info = (yajl_info *)ctx;  
    yajl_gen_status __stat = yajl_gen_array_open(info->gen);  
    if (__stat == yajl_gen_status_ok)  {  
        info->depth++;  
    } else {  
        printf("yajl_gen_array_open failed\n");  
    }  
    return __stat;  
}  
  
yajl_gen_status end_array(void *ctx)  
{  
    yajl_info *info = (yajl_info *)ctx;  
    yajl_gen_status __stat = yajl_gen_array_close(info->gen);  
    if (__stat == yajl_gen_status_ok) {  
        info->depth--;  
    }  
    return __stat;  
} 

/*
 * 
 * 							"{\"ret\":\"0\","
							"\"desc\":\"OK\","
							"\"operateCode\":\"105\","
							"\"equipmentNo\":\"%s\","
							"\"equipmentStatus\":\"1\","
							"\"userId\":\"%s\"}", equipmentid, userid);

*/
char * getDevCtrl2Json(  equpment_t* eq,char* eqno,char * userid )
{

	yajl_info yajlinfo;  
    yajl_init (&yajlinfo);
	unsigned char* buf=NULL;  
    size_t buflen; 
	unsigned char *key = "operateCode";  
    unsigned char *value = "105";  


	begin_map(&yajlinfo);
	
    key = "ret";  
    value = "0"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "desc";  
	value = " ";	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "operateCode";  
	value = "105"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentNo";  
	value = (unsigned char*)eqno;
	if( value == NULL ) value=" "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "equipmentStatus";  
	value = (unsigned char*)"1";
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "userId";  
	value = (unsigned char*)userid;
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	end_map(&yajlinfo);
	yajl_gen_get_buf(yajlinfo.gen, &buf, &buflen);  
    yajl_info_free(&yajlinfo);
	return buf;
}

	/*	sprintf( strreturn, "{\"ret\":\"0\","
							"\"desc\":\"OK\","
							"\"operateCode\":\"106\","
							"\"equipmentNo\":\"%s\","
							"\"childEquipmentNo\":\"%s\","
							"\"childEquipmentOpValue\":\"%d\","
							"\"userId\":\"%s\"}", equipmentid, childEqNo,mvalue,userid );
	*/

char * getSubCtrl2Json(equpment_t* eq,char* eqno,char* subDevNO, char* subvalue, char * userid )
{
	yajl_info yajlinfo;  
	char *mybuffer=NULL;
	size_t buflen; 
	unsigned char *key = "ret";  
	unsigned char *value = "1"; 

	yajl_init (&yajlinfo);
	begin_map(&yajlinfo);

	key = "ret";  
    value = "0"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "desc";  
	value = " ";	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "operateCode";  
	value = "106"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentNo";  
	value = (unsigned char*)eqno;
	if( value == NULL ) value=" "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "childEquipmentNo";  
	value = (unsigned char*)subDevNO;
	if( value == NULL ) value=" "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "childEquipmentOpValue";  
	value = (unsigned char*) subvalue;
	if( value == NULL ) value=" "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "userId";  
	value = (unsigned char*)userid;
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));
	

	end_map(&yajlinfo);
	yajl_gen_get_buf(yajlinfo.gen, &mybuffer, &buflen);  
	yajl_info_free(&yajlinfo);
	return mybuffer;
}
/*
 * 
 * "{"
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
				*/


char * getMeasure2Json(equpment_t* eq,char* eqno,int opcode)
{



	double y=31.4041070000 + r(10)*0.001;
	double x=121.0297980000+ r(10)*0.001;
	
	//float ph=7.5+r(4);
	//float cod=20.2+r(10);
	//float mdo=0.8+r(25);
	char localstr[16];	
	
	yajl_info yajlinfo;  
    yajl_init (&yajlinfo);
	char sztemp[32];
	unsigned char* buf = NULL;  
    size_t buflen; 
	int i;
	unsigned char *key = "operateCode";  
    unsigned char *value = "202";  


	eq->longtitude=x;
	eq->latitude=y;


	sprintf( localstr,"local%d", r(1000) );

	
	begin_map(&yajlinfo);
	
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));
	
	key = "equipmentNo";  
    value = (unsigned char*)eqno;

	if( value == NULL ) value=" "; 
	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentName";  
    value = (unsigned char*)eq->name; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "temperature";  
    value = " "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "weather";  
    value = " ";
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "remarks";  
    value = " "; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "locationName";  
    value = (unsigned char*)localstr; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	//eq->longtitude=x;
	//eq->latitude=y;

	key = "longitude";  
	sprintf(sztemp,"%g",eq->longtitude);
    value = (unsigned char*)sztemp; 
	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "latitude";  
    sprintf(sztemp,"%g",eq->latitude);
    value = (unsigned char*)sztemp; 
	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));
	

	key="monitorData";
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	begin_array(&yajlinfo);

//static float QueValue[] = { 12.2, 5.6, 12.5, 23.6, 44.5 };
    for(i=0;i<measureSum;i++) 
	{  
       //往对象中插入数据  
	   begin_map(&yajlinfo);
		key = "quotaName";  
		value = (unsigned char*)QuoName[i]; 
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


		key = "position";  
		value = (unsigned char*)"00"; 
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


		key = "quotaValue";  
		sprintf(sztemp,"%g",QueValue[i]+r(10));
    	value = (unsigned char*)sztemp; 
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
		yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));	
		
       end_map(&yajlinfo);
    }
	end_array(&yajlinfo); 
	end_map(&yajlinfo);
	
	yajl_gen_get_buf(yajlinfo.gen, &buf, &buflen);  
    yajl_info_free(&yajlinfo);
	//printf("%s\n", buf);  
	return buf;

}


/*sprintf(mystatus,
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

*/	

char * subdevtostring( equpment_t* eq, char* eqno, int opcode )
{

	
	yajl_info yajlinfo;  
    yajl_init (&yajlinfo);
	unsigned char* buf=NULL;  
    size_t buflen; 
	int i;
	unsigned char *key = "operateCode";  
    unsigned char *value = "104";  
	char sztemp[16];

	begin_map(&yajlinfo);
	
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));
	
	key = "equipmentNo";  
    value = (unsigned char*)eqno;

	if( value == NULL ) value=" "; 
	
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentName";  
    value = (unsigned char*)eq->name; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "equipmentType";  
	sprintf( sztemp,"%d", eq->type);
    value = (unsigned char*)sztemp; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "company";  
    value = (unsigned char*)eq->company;
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentStatus";  
	sprintf( sztemp,"%d", eq->status);
    value = (unsigned char*)sztemp; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));


	key = "equipmentViewData";  
    value = (unsigned char*)"my boat"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "userId";  
    value = (unsigned char*)"0000"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));
	

	key="childEquipmentData";
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
	begin_array(&yajlinfo);
	
    for(i=0;i<eq->nsubsize;i++) 
	{  
       //往对象中插入数据  
	   begin_map(&yajlinfo);
       yajl_gen_string(yajlinfo.gen, (const unsigned char*)eq->subdevname[i], strlen(eq->subdevname[i]));  
       yajl_gen_string(yajlinfo.gen, (const unsigned char*)eq->subdevvalue[i], strlen(eq->subdevvalue[i])); 
       end_map(&yajlinfo);
    }
	end_array(&yajlinfo); 
	end_map(&yajlinfo);

	yajl_gen_get_buf(yajlinfo.gen, &buf, &buflen);  
    yajl_info_free(&yajlinfo);
	return buf;

}

char * getRegInfo( equpment_t* eq,char* filename )
{

	yajl_val node,tmpnode;
	yajl_val v,tmpk1,tmpv1;
	
	const char * eqpath[] = { "equipmentName", (const char *) 0 };
	const char * copath[] = { "company", (const char *) 0 };
	const char * typepath[] = { "equipmentType", (const char *) 0 };
	const char * subdevpath[] = { "childEquipmentData", (const char *) 0 };
	const char * subNOpath[] = { "childEquipmentNo", (const char *) 0 };
	const char * subVpath[] =  { "childEquipmentOpValue", (const char *) 0 };
	char *key,*value;
	int key_length, value_length;
	int from_fd;
	int file_len;
	int ret;
	char * buf=NULL;
	int len,sublen,i;

	if((from_fd=open(filename, O_RDONLY))==-1) 
	{ 
		printf("Open %s Error\n",filename ); 
		return NULL; 
	}

	eq->status=1;
	eq->nsubsize=0;
	//eq->mysubDev = NULL;

	file_len  = lseek(from_fd,0L,SEEK_END);
    lseek(from_fd,0L,SEEK_SET);
	buf = (char*) malloc( file_len +1 );	
	ret= read(from_fd, buf, file_len);

	buf[file_len]=0;
	
	if( ret == file_len )
	{	
		node = yajl_tree_parse((const char *) buf, NULL, 0);
		if( node != NULL )
		{
//get name			
			v = yajl_tree_get(node, eqpath, yajl_t_string);
			if(v)
			{
				len = strlen(YAJL_GET_STRING(v));
				if( len > 1 && len <30)
				{
					strcpy(eq->name,YAJL_GET_STRING(v));
					printf("--------------%s\n",eq->name);
				}
			}
			//yajl_tree_free(v);
//get company
			v = yajl_tree_get(node, copath, yajl_t_string);
			if(v)
			{
				len = strlen(YAJL_GET_STRING(v));
				if( len > 1 && len <30)
				{
					strcpy(eq->company,YAJL_GET_STRING(v));
					printf("--------------%s\n",eq->company);
				}
			}
			//yajl_tree_free(v);
//get type
			v = yajl_tree_get(node, typepath, yajl_t_string);
			if(v)
			{
						
				len = strlen(YAJL_GET_STRING(v));
				if( len >= 1 && len <3)
				{
					eq->type = atoi(YAJL_GET_STRING(v));
					printf("--------------%d\n",eq->type );
				}
			}
			//yajl_tree_free(v);
//get device list
			v = yajl_tree_get(node, subdevpath, yajl_t_array);
			if(v)
			{
				sublen = (v)->u.array.len;
				eq->nsubsize = sublen;
				printf("--------------%d\n",sublen);
				//eq->mysubDev = dictionary_init();
				if (sublen > 0 )
				{
					eq->subdevname = (char**)malloc( sublen * sizeof( char*) );
					eq->subdevvalue = (char**)malloc( sublen * sizeof( char*) );
						
					for(i=0;i<sublen;i++)
					{

						tmpnode = (v)->u.array.values[i];
						tmpk1 = yajl_tree_get(tmpnode, subNOpath, yajl_t_string);
						tmpv1 = yajl_tree_get(tmpnode, subVpath, yajl_t_string);
						if( tmpk1 && tmpv1 )
						{
							 char* szkey = YAJL_GET_STRING(tmpk1);
							 char* szval = YAJL_GET_STRING(tmpv1);
							 eq->subdevname[i] = (char*)malloc( strlen(szkey) + 1 );
							 eq->subdevvalue[i] = (char*)malloc( strlen(szval) + 1 );
							 strcpy(eq->subdevname[i], szkey);
							 strcpy(eq->subdevvalue[i], szval);							

						}
					}	
				}
			}
		}		
		/*yajl_tree_free(v);
		yajl_tree_free(tmpnode);
		yajl_tree_free(tmpk1);
		yajl_tree_free(tmpv1);
		*/
		//yajl_tree_free(node);
		close(from_fd); 
		return buf;
	}
	else
	{
		close(from_fd); 
		free(buf);
		return NULL;
	}	

}