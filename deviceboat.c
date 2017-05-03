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

static int compare_e ( void* left, void* right ) 
{
    return strcmp ( (const char *)left, (const char *) right );
}
void replace_subvalues(equpment_t* eq, char* key, char* value) 
{
	remove_c_map ( eq->subdev, key);
	char *keystr = clib_strdup(key);
	int key_length = (int)strlen ( keystr ) + 1;
	char* valuestr = clib_strdup( value);
	int value_length = (int)strlen ( valuestr ) + 1;
	insert_c_map ( eq->subdev , keystr, key_length, &valuestr, value_length);
	free ( keystr );
	free ( valuestr );	
}

char * subdevtostring( equpment_t* eq, char* eqno, int opcode )
{

	
	yajl_info yajlinfo;  
    yajl_init (&yajlinfo);
	unsigned char* buf=NULL;  
    size_t buflen; 
	int i;
	unsigned char *key = "operateCode";  
    unsigned char *value = "104";  

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
    value = (unsigned char*)"2"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "company";  
    value = (unsigned char*)"2"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentStatus";  
    value = (unsigned char*)"1"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentType";  
    value = (unsigned char*)"2"; 
	yajl_gen_string(yajlinfo.gen, (const unsigned char*)key, strlen(key));  
    yajl_gen_string(yajlinfo.gen, (const unsigned char*)value, strlen(value));

	key = "equipmentViewData";  
    value = (unsigned char*)"   "; 
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

		unsigned char *key1 = "operateCode";  
    	unsigned char *value1 = "104";  

       yajl_gen_string(yajlinfo.gen, (const unsigned char*)key1, strlen(key1));  
       yajl_gen_string(yajlinfo.gen, (const unsigned char*)value1, strlen(value1));  
       end_map(&yajlinfo);
    }


	//key = "equipmentNo";  
    //value = (unsigned char*)eqno;

	end_array(&yajlinfo); 
	end_map(&yajlinfo);

	yajl_gen_get_buf(yajlinfo.gen, &buf, &buflen);  
    printf("%s\n", buf);  
    yajl_info_free(&yajlinfo);
	return NULL;
	//return buf;

	
/*
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

*/	

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

	eq->subdev = new_c_map ( compare_e, NULL, NULL);

	file_len  = lseek(from_fd,0L,SEEK_END);
    lseek(from_fd,0L,SEEK_SET);
	buf = (char*) malloc( file_len  );	
	ret= read(from_fd, buf, file_len);
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
				eq->nsubsize=sublen;
				printf("--------------%d\n",sublen);
				for(i=0;i<sublen;i++)
				{

					tmpnode = (v)->u.array.values[i];
					tmpk1 = yajl_tree_get(tmpnode, subNOpath, yajl_t_string);
					tmpv1 = yajl_tree_get(tmpnode, subVpath, yajl_t_string);
					if( tmpk1 && tmpv1 )
					{
						
						char *key = clib_strdup( YAJL_GET_STRING(tmpk1));
						int key_length = (int)strlen ( key ) + 1;
						char* value = clib_strdup( YAJL_GET_STRING(tmpv1));
						int value_length = (int)strlen ( value ) + 1;						
						printf ( "Inserting [%s -> %s]\n", key, value );
						insert_c_map ( eq->subdev , key, key_length, &value, value_length); 
						free ( key );
						free ( value );
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