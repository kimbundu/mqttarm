
#include   <stdio.h> 
#include   <stdlib.h>
#include   <string.h>
#include   <unistd.h>
#include   <sys/types.h>
#include   <sys/stat.h>  
#include   <fcntl.h>  
#include   <termios.h>
#include   <errno.h> 
#include   <pthread.h> 
#include   <sys/ioctl.h> 
#include  "serialscomm.h"
#define    FALSE 1
#define    TRUE 0





int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800,
		    B2400, B1200};
int name_arr[] = {115200, 57600, 38400,  19200,  9600,  4800,  2400, 1200};

#define debugnum(data,len,prefix)  \
{ \
        unsigned int i;   \
        for (i = 0;i < len;i++) { \
                if(prefix)  \
                        printf("0x%02x ",data[i]); \
                else  \
                        printf("%02x ",data[i]); \
        } \
}

void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
	   	if (speed == name_arr[i])
	   	{
	   	    tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
			perror("tcsetattr fd1");
			return;
		 }
		tcflush(fd,TCIOFLUSH);
    }
}
int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    if  ( tcgetattr( fd,&options)  !=  0)
    {
  		perror("SetupSerial 1");
  		return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   
			options.c_iflag &= ~INPCK;   
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); 
			options.c_iflag |= INPCK;           
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;     
			break;
		case 'S':
		case 's':  
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
	
    switch (stopbits)
    {
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
		return (FALSE);
    }


	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

    /* Set input parity option */
    
    if (parity != 'n')
	options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;
    
    
	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
		perror("SetupSerial 3");
		return (FALSE);
    }
    return (TRUE);
}




void receivethread(void* pcontext)
{
  comm_t * pcomm = (comm_t*)pcontext;
  int nread;
  char buff[512];
 
  while( pcomm->status )    
  {
     
	if( ( nread = read(pcomm->devfd,buff,128) ) > 0 ) //
	{
	    printf("[RECEIVE] Len is %d,content is :\n",nread);
	    buff[nread]='\0';
	    //printf("%s\n",buff);
	    if( pcomm->recvFunction!= NULL)
	    {
		pcomm->recvFunction( buff,strlen(buff) );	
	    } 
	}
	
	usleep(1000);
   } 
 
 return;
}



int  open_com( comm_t* mycom)
{     
	pthread_t receiveid;

	mycom->devfd = open(mycom->devname, O_RDWR);
	
	

    
	if (mycom->devfd == -1)
	{
		printf("open device [%d:] [%s] faild\n",mycom->devfd,mycom->devname);
		return -1;
	}

	set_speed(mycom->devfd,mycom->baut);
	set_Parity(mycom->devfd,8,1,'N');

    mycom->status = 1;
	pthread_create(&receiveid,NULL,(void*)receivethread,mycom);
	return 0;
}
int  close_com( comm_t* mycom)
{
	if( mycom->devfd != -1)
	{	
	    close(mycom->devfd);
	}
	return 0;
}
int senddata_com(comm_t* mycom,char* strmsg,int strlen )
{
    if(  mycom->status != 1 ) return 0;
     
	int n=0;
	n = write(mycom->devfd,strmsg, strlen);
	return n;   
}



