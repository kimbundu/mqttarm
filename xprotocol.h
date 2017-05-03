#ifndef _XPROTOCOL_COMM_H_
#define _XPROTOCOL_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
char* x_getRegInfo();
char* x_parseInfo(char* buf);
char* x_getUnregInfo();
char* x_getDevStatus();
*/		
void test();
int  mytest_config(const char* filename);


int x_getOpCode( char * strinfo );

//int x_getString_path( const char * strinfo,const char* path[], char * mystr );

int x_getReturnInfo(char * strinfo);
	

	
#ifdef __cplusplus
}
#endif

#endif