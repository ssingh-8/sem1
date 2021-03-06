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

static char gszProgName[MAXPATHLENGTH];


int g_PacketsCompleted = 0;
int g_PacketsDropped = 0;
int g_PacketsRemoved = 0;
int g_TokensDropped = 0;
int g_TokensInBucket = 0;
int g_TokenNumber = 0;
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

sigset_t set;

FILE *fp;
char fileBuf[1026];

My402List q1,q2;
Parameters params;
int traceDriven = 0;

struct timeval startTime;

void setDefaultParameters(){
	
	params.lamda = 0.5;
	params.r = 1.5;
	params.mu = 0.35;
	params.B = 10;
	params.P = 3;
	params.n = 20;
	params.fileName = NULL;
}

static
void Usage()
{
    	fprintf(stderr,
		"\nusage: %s %s\n",
            	gszProgName, "[-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
    	exit(-1);
}

void ProcessOptions(int argc, char *argv[]){

	if(argc > 15){
            	fprintf(stderr, "More command line options than expected <%d>\n", argc);
            	exit(1);
        }
	
	int i;
	for(i = 1; i < argc; i++) {

		if(strcmp(argv[i], "-lamda") == 0){
			params.lamda = atof(argv[++i]);
			if(params.lamda <= 0) {
				fprintf(stderr,"\nError: lamda must be a positive real number <%s>\n",argv[i]);
				exit(-1);
			}
			if(params.lamda < 0.1)
				params.lamda = 0.1;
			//To Do : check for values 0.gf33 etc

			//printf("\n Lamda = %f\n",params.lamda);
		}

		else if(strcmp(argv[i], "-mu") == 0){
			params.mu = atof(argv[++i]);
			if(params.mu <= 0) {
				fprintf(stderr,"\nError: mu must be a positive real number <%s>\n",argv[i]);
				exit(-1);
			}
			if(params.mu < 0.1)
				params.mu = 0.1;
		}

		else if(strcmp(argv[i], "-r") == 0){
			params.r = atof(argv[++i]);
			if(params.r <= 0) {
				fprintf(stderr,"\nError: r must be a positive real number <%s>\n",argv[i]);
				exit(-1);
			}
			if(params.r < 0.1)
				params.r = 0.1;
		}

		else if(strcmp(argv[i], "-B") == 0){
			params.B = atoi(argv[++i]);
			if(params.B <= 0 || params.B > 2147483647) {
				fprintf(stderr,
					"\nError: Token Bucket depth 'B' must be a Valid Integer between the range 1 to 2147483646 <%s>\n",
					argv[i]);
				exit(-1);
			}
		}

		else if(strcmp(argv[i], "-P") == 0){
			params.P = atoi(argv[++i]);
			printf("\n P = %d\n",params.P);
			if(params.P <= 0 || params.P > 2147483647) {
				fprintf(stderr,"\nError: Number of tokens 'P' required to process a packet must be a Valid Integer between the range 1 to 2147483646 <%s>\n",argv[i]);
				exit(-1);
			}
		}

		else if(strcmp(argv[i], "-n") == 0){
			params.n = atoi(argv[++i]);
			if(params.n <= 0 || params.n > 2147483647) {
				fprintf(stderr,"\nError: Number of packets 'n' must be a Valid Integer between the range 1 to 2147483646 <%s>\n",argv[i]);
				exit(-1);
			}
		}

		else if(strcmp(argv[i], "-t") == 0){
			params.fileName = argv[++i];
			traceDriven = 1;
		}

		else {
			fprintf(stderr, "\nError: Invalid command line argument. <%s>\n",argv[i]);
            		Usage();
		}
	}
}

static
void SetProgramName(char *s)
{
    char *c_ptr=strrchr(s, DIR_SEP);

    if (c_ptr == NULL) {
        strcpy(gszProgName, s);
    } else {
        strcpy(gszProgName, ++c_ptr);
    }
}


double getTime(struct timeval start,struct timeval end){
	return (((end.tv_sec - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)/1000));
}

void parseLine(Packet *packet){

	//INCOMPLETE

	char *token;
	int count = 0;
	//char *ch = '\t';
	if(fgets(fileBuf, 1026, fp) != NULL) {
		token = strtok(fileBuf, " \t");
		packet->interArrivalTime = atof(token);
		//printf( " %f\n", packet->interArrivalTime );

		token = strtok(NULL, " \t");
		packet->tokensRequired = atoi(token);
		//printf( " %s\n", token );

		token = strtok(NULL, " \t");
		packet->serviceTime = atof(token);
		//printf( " %s\n", token );
	} 	
}


void movePktFromQ1toQ2(){
	
	struct timeval currentTime;	

	My402ListElem *elem = My402ListFirst(&q1);
	Packet *packet = (Packet *)(elem->obj);			
	
	g_TokensInBucket -= packet->tokensRequired;
	My402ListUnlink(&q1, elem);	
	
	gettimeofday(&currentTime, NULL);
	packet->q1LeaveTime = getTime(startTime, currentTime);
	
	printf("%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d tokens\n", packet->q1LeaveTime, packet->packetNo, packet->q1LeaveTime-packet->q1EnterTime, g_TokensInBucket);
		
	My402ListAppend(&q2, elem);
	gettimeofday(&currentTime, NULL);
	packet->q2EnterTime = getTime(startTime, currentTime);
	printf("%012.3fms: p%d enters Q2\n", packet->q2EnterTime, packet->packetNo);

}


void *arrival(void *arg)
{   
	struct timeval currentTime; //prevArrival, currentTime, leave;
	
	//prevArrival = startTime;
	//leave = prevArrival;
	double timeElapsed = 0.0;
	int i;
	for(i = 0; i < params.n; i++){

		Packet *packet = (Packet *)malloc(sizeof(Packet));
		if(packet == NULL){
			fprintf(stderr, "Error while creating packet\n");
            		exit(1);
		}

		if(traceDriven){
			parseLine(packet);
		}
		else{
			packet->interArrivalTime = 1000.0/params.lamda;
			packet->tokensRequired = params.P;
			packet->serviceTime = 1000.0/params.mu;	
		}
		
		packet->packetNo = ++i;
		timeElapsed += packet->interArrivalTime;
		gettimeofday(&currentTime, NULL);
		if(timeElapsed > getTime(startTime,currentTime))
			usleep(1000*(timeElapsed - getTime(startTime,currentTime)));
		
		gettimeofday(&currentTime, NULL);
		packet->arrivalTime = timeElapsed;// getTime(startTime,currentTime);
		//prevArrival = currentTime;
		
		
		//remove packet if tokens required is more than depth B
		if(packet->tokensRequired > params.B) {
			printf("%012.3fms: p%d arrives, needs %d tokens, dropped\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired);
                g_PacketsDropped++;
		}

		else {
			printf ("%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired, packet->interArrivalTime);
            
			pthread_mutex_lock(&m);

			My402ListAppend(&q1, (void *)packet);
			gettimeofday(&currentTime, NULL);
			packet->q1EnterTime = getTime(startTime, currentTime);
			printf("%012.3fms: p%d enters Q1\n", packet->q1EnterTime, packet->packetNo); 
	
			Packet *headPacket = (Packet *)(My402ListFirst(&q1)->obj); 
			
			if(headPacket->tokensRequired <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(&q2) == 1)
					pthread_cond_broadcast(&cond);				
			}
			pthread_mutex_unlock(&m);
		
		}		
	}
	return NULL;
}

void *token(void *arg)
{   
	struct timeval currentTime; //prevArrival, leave;
	//prevArrival = startTime;
	//leave = prevArrival;
	
	double timeElapsed = 0.0;
	while(1) {
		
		
		timeElapsed += (1000.0/params.r) ;
		pthread_mutex_lock(&m);
		if(params.n <= 0 && My402ListEmpty(&q1)) {
			fprintf(stderr, "No more Packets to process \n");
            		exit(1);
		}
		pthread_mutex_unlock(&m);
		gettimeofday(&currentTime, NULL);
		if(timeElapsed > getTime(startTime,currentTime))
			usleep(1000*(timeElapsed - getTime(startTime,currentTime)));


		pthread_mutex_lock(&m);
		g_TokenNumber++;

		
		if(g_TokensInBucket >= params.B) {
			printf("%012.3fms: token t%d arrives, dropped\n", timeElapsed, g_TokenNumber); 
			g_TokensDropped++;
		}
		else {
			g_TokensInBucket++;
			printf("%012.3fms: token t%d arrives, token bucket now has %d tokens\n", timeElapsed, g_TokenNumber, g_TokensInBucket);
		}
		
		if(!My402ListEmpty(&q1)) {
			Packet *headPacket = (Packet *)(My402ListFirst(&q1)->obj); 
			
			if(headPacket->tokensRequired <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(&q2) == 1)
					pthread_cond_broadcast(&cond);				
			}
		}
		pthread_mutex_unlock(&m);
			
		//gettimeofday(&leave, NULL); 	
	}
	
	return NULL;
}

void *server(void *arg)
{ 
	
	return NULL;
}

int main(int argc, char *argv[])
{
	
	SetProgramName(*argv);
	setDefaultParameters();
	ProcessOptions(argc, argv);

	My402ListInit(&q1);
	My402ListInit(&q2);

	if(traceDriven){
		if ((fp = fopen(params.fileName, "r")) == NULL) {
            		fprintf(stderr, "Could not open the file <%s>\n", params.fileName);
            		exit(1);
        	}
        
        	fgets(fileBuf, 1026, fp); 
        	params.n = atoi(fileBuf);  
		if(params.n < 0 || params.n > 2147483647) {
			fprintf(stderr, "Invalid Number of Packets <%s>\n", fileBuf);
            		exit(1);
		}     
    	}

	int error;
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	error = pthread_sigmask(SIG_BLOCK,&set,NULL);

	gettimeofday(&startTime, NULL); 
	//printf("%d seconds\n", startTime.tv_sec); 
    	//printf("%d microseconds\n", startTime.tv_usec); 

	printf("%012.3fms: emulation begins\n",getTime(startTime,startTime));

	if(error != 0){
		fprintf(stderr, "\ncan't create mask :[%s]", strerror(error));
		exit(1);
	}


	if((error = pthread_create(&arrivalThread, 0, arrival, NULL))){
        	fprintf(stderr, "\ncan't create arrival thread :[%s]", strerror(error));
		exit(1);
	}

	if((error = pthread_create(&tokenThread, 0, token, NULL))){
            	fprintf(stderr, "\ncan't create token depositing thread :[%s]", strerror(error));
		exit(1);
	}

	if((error = pthread_create(&serverThread, 0, server, NULL))){
            	fprintf(stderr, "\nlcan't create server thread :[%s]", strerror(error));
		exit(1);
	}

	
	pthread_join(arrivalThread,0);
	pthread_join(tokenThread,0);
	pthread_join(serverThread,0);

    	//pause();
    	//sleep(1);
	return 0;
}
