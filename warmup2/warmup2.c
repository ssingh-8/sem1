#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

typedef struct tagPacket {


} Packet;


pthread_t thread[3];
//int ret1,ret2;

void* arrival(void *arg)
{   
	return NULL;
}

void* token(void *arg)
{   
	return NULL;
}

void* server(void *arg)
{   
	return NULL;
}

int main(void)
{
	int i;
	int error;

	if(err = pthread_create(&(thread[0]), NULL, arrival, NULL)){
        	printf("\ncan't create arrival thread :[%s]", strerror(error));
		exit(1);
	}

	if(error = pthread_create(&(thread[1]), NULL, token, NULL)){
            	printf("\ncan't create token depositing thread :[%s]", strerror(error));
		exit(1);
	}

	if(error = pthread_create(&(thread[2]), NULL, server, NULL)){
            	printf("\ncan't create server thread :[%s]", strerror(error));
		exit(1);
	}

	for(i=0;i<3;i++){
		pthread_join(thread[i],0);
	}
    	//pause();
    	//sleep(1);
	return 0;
}
