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
#include "cs402.h"
#include "my402list.h"
#include "warmup2.h"


int g_PacketsCompleted = 0;
int g_PacketsDropped = 0;
int g_PacketsRemoved = 0;
int g_TokensDropped = 0;
double g_TotalInterArrivalTime = 0.0;
double g_TotalServiceTime = 0.0;
double g_TimeSpentQ1 = 0.0;
double g_TimeSpentQ2 = 0.0;
double g_TimeSpentS = 0.0;
double g_TotalTimeSpentSystem = 0.0;


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

pthread_t arrivalThread;
pthread_t tokenThread;
pthread_t serverThread;


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
	//SetProgramName(*argv);
	//ProcessOptions(argc, argv);

	int error;

	if((error = pthread_create(&arrivalThread, 0, arrival, NULL))){
        	printf("\ncan't create arrival thread :[%s]", strerror(error));
		exit(1);
	}

	if((error = pthread_create(&tokenThread, 0, token, NULL))){
            	printf("\ncan't create token depositing thread :[%s]", strerror(error));
		exit(1);
	}

	if((error = pthread_create(&serverThread, 0, server, NULL))){
            	printf("\nlcan't create server thread :[%s]", strerror(error));
		exit(1);
	}

	
	pthread_join(arrivalThread,0);
	pthread_join(tokenThread,0);
	pthread_join(serverThread,0);

    	//pause();
    	//sleep(1);
	return 0;
}
