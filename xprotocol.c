#include "xprotocol.h"
#include <stdio.h>
#include <string.h>
#include <yajl/yajl_tree.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#define GEN_AND_RETURN(func)                                          \
  {                                                                   \
    yajl_gen_status __stat = func;                                    \
    if (__stat == yajl_gen_generation_complete && s_streamReformat) { \
      yajl_gen_reset(g, "\n");                                        \
      __stat = func;                                                  \
    }                                                                 \
    return __stat == yajl_gen_status_ok; }


//static unsigned char fileData[65536];


int x_getReturnInfo(char * strinfo)
{
	int ret=0;
	yajl_val node;
	yajl_val v; 
	char errbuf[1024];
	const char * retpath[] = { "ret", (const char *) 0 };
	
	node = yajl_tree_parse((const char *) strinfo, errbuf, sizeof(errbuf));
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
	//yajl_tree_free( v );
	yajl_tree_free( node);
	
	return ret;

}

int x_getOpCode( char * strinfo )
{
	int opcode = 0;
	yajl_val node, v; 
	char errbuf[1024];
	const char * path[] = { "operateCode", (const char *) 0 };
	node = yajl_tree_parse((const char *) strinfo, errbuf, sizeof(errbuf));

	if (node == NULL) 
	{
        fprintf(stderr, "parse_error \n");

		if (strlen(errbuf)) 
			fprintf(stderr, " %s", errbuf);
        else fprintf(stderr, "unknown error");
		    fprintf(stderr, "\n");
        return opcode;
    }
	v = yajl_tree_get(node, path, yajl_t_string);

	if(v)
	{
		opcode = atoi(YAJL_GET_STRING(v));
	}
	//yajl_tree_free(v);
    yajl_tree_free(node);
	
	return opcode;
}
/*int  x_getString_path( const char * strinfo,const char* path[], char * mystr )
{
	int ret=0;
	char errbuf[1024];
	yajl_val node; 
	node = yajl_tree_parse((const char *) strinfo, errbuf, sizeof(errbuf));

    if (node == NULL) 
	{
        fprintf(stderr, "parse_error: ");

		if (strlen(errbuf)) 
			fprintf(stderr, " %s", errbuf);
        else fprintf(stderr, "unknown error");
		    fprintf(stderr, "\n");
        return -1;
    }

	const char * retpath[] = { "ret", (const char *) 0 };
	yajl_val v = yajl_tree_get(node, retpath, yajl_t_string);
	if(v)
	{
		ret = atoi(YAJL_GET_STRING(v));
	}

	v = yajl_tree_get(node, path, yajl_t_string);
	
	if(v)
	{
		int len = strlen(YAJL_GET_STRING(v));
		mystr = malloc(len);
		strcpy(mystr,YAJL_GET_STRING(v));
	}	
	return  ret;
}*/



/*
int  mytest_config(const char* filename)
{
    size_t rd;
    yajl_val node;
    char errbuf[1024];
	FILE *file; 
	int i;

	file = fopen(filename, "r");
    //file = fopen("/home/kim/foobar/sample.config", "r");

	//file = fopen("/home/root/sample.config", "r");
	if( file == 0 )
	{
		fprintf(stderr, "error read on file read\n");
        return 1;
	}

	//fileData = (unsigned char *) malloc(bufSize);
    fileData[0] = errbuf[0] = 0;
    rd = fread((void *) fileData, 1, sizeof(fileData) - 1, file);

    if (rd == 0 && !feof(stdin)) 
	{	
        fprintf(stderr, "error encountered on file read\n");
        return 1;
    } else if (rd >= sizeof(fileData) - 1) 
	{
        fprintf(stderr, "config file too big\n");
        return 1;
    }

  
    node = yajl_tree_parse((const char *) fileData, errbuf, sizeof(errbuf));


    if (node == NULL) 
	{
        fprintf(stderr, "parse_error: ");

		if (strlen(errbuf)) 
			fprintf(stderr, " %s", errbuf);
        else fprintf(stderr, "unknown error");
		    fprintf(stderr, "\n");
        //return 1;
    }

    
    const char * path[] = { "Logging", "timeFormat", (const char *) 0 };
	
    yajl_val v = yajl_tree_get(node, path, yajl_t_string);

	
	
    if (v) 
		printf("%s/%s: %s\n", path[0], path[1], YAJL_GET_STRING(v));
    else   
		printf("no such node: %s/%s\n", path[0], path[1]);


	 yajl_t_string 	
yajl_t_number 	
yajl_t_object 	
yajl_t_array 	
yajl_t_true 	
yajl_t_false 	
yajl_t_null 	
yajl_t_any


	const char * path2[] = { "childEquipmentData", (const char *) 0 };
	
    yajl_val v2 = yajl_tree_get(node, path2,  yajl_t_array);

	const char *key[] = { "childEquipmentOpType",  (const char*) 0}; 

	if(v2 && YAJL_IS_ARRAY(v2)) 
	{
        int array_num = v2->u.array.len;
        printf("Top level of json string is array, has %d elements\n", array_num);

		for( i=0;i<array_num;i++)
		{
			yajl_val obj = v2->u.array.values[ i ];
            if(!YAJL_IS_OBJECT(obj)) //获取数组中的元素 
		    {
                if(YAJL_IS_STRING(obj)) 
				{
                    printf("%d, value = %s\n", i, obj->u.string);
                }
            } 
		    else //获取object中的元素
		    {                        
                printf("%d elements is object\n", i);
                yajl_val v = yajl_tree_get(obj, key, yajl_t_string);
                if (v) 
				{
                    printf("%s : %s\n", key[0], YAJL_GET_STRING(v));
                }
				else 
				{
                    printf("no such node: %s \n", key[0]);
                }
            }
        }
    }



	
   
    yajl_tree_free(node);

	//free(fileData);
    fclose(file);
    return 0;
}

void test2()
{
	yajl_gen g;
    yajl_status stat;
	
	g = yajl_gen_alloc(NULL);
    yajl_gen_config(g, yajl_gen_beautify, 1);
    yajl_gen_config(g, yajl_gen_validate_utf8, 1);


	//yajl_gen_bool(g, boolean);
	// yajl_gen_string

	//yajl_gen_null
	//yajl_gen_integer
	//yajl_gen_double
	//yajl_gen_number
	//yajl_gen_string
	//yajl_gen_bool
	//yajl_gen_map_open
	//yajl_gen_map_close
	//yajl_gen_arry_open
	//yajl_gen_arry_close
	//yajl_gen_reset
		
	 

	const unsigned char * buf;
	int len;
	yajl_gen_get_buf(g, &buf, &len);
    fwrite(buf, 1, len, stdout);
    yajl_gen_clear(g);
	yajl_gen_free(g);
	
}

void  test()
{
	size_t rd;
	yajl_status stat;   
    yajl_handle hand;


	//yajl_handle hand = yajl_alloc(NULL, NULL, NULL);
    // yajl_status stat;        
    //const char ** d;

    //yajl_config(hand, yajl_dont_validate_strings, validate_utf8 ? 0 : 1);
	
    yajl_config(hand, yajl_allow_comments, 1);
    yajl_config(hand, yajl_dont_validate_strings, 1);
    yajl_config(hand, yajl_allow_multiple_values, 1);
	stat = yajl_parse(hand, fileData, rd);
	
    //    if (stat != yajl_status_ok) break;
	stat = yajl_complete_parse(hand);
    if (stat != yajl_status_ok)
    {
       /*if (!quiet) {
            unsigned char * str = yajl_get_error(hand, 1, fileData, rd);
            fprintf(stderr, "%s", (const char *) str);
            yajl_free_error(hand, str);
        }*//*
        //retval = 1;
    }
    yajl_free(hand);
}
*/