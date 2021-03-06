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
int g_TotalPackets = 0;
int g_TokensDropped = 0;
int g_TokensInBucket = 0;
int g_TokenNumber = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

pthread_t arrivalThread;
pthread_t tokenThread;
pthread_t serverThread;
pthread_t sigThread;

sigset_t set;
struct sigaction sig_act;
struct sigaction sig_act2;

FILE *fp;
char fileBuf[1026];

My402List q1,q2;
Parameters params;
int traceDriven = 0;
volatile int gSighandlerDone = 0;

struct timeval startTime;

double g_AvgPacketInterArrivalTime = 0.0;
double g_AvgPacketServiceTime = 0.0;
double g_AvgPacketsInQ1 = 0.0;
double g_AvgPacketsInQ2 = 0.0;
double g_AvgPacketsInS = 0.0;
double g_AvgTimePacketInSystem = 0.0;
double g_AvgTimePacketInSystem_Square = 0.0;
double g_StandardDeviation = 0.0;
double g_TokenDropProbability = 0.0;
double g_PacketDropProbability = 0.0;

void setDefaultParameters(){
	
	params.lambda = 0.5;
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

		if(strcmp(argv[i], "-lambda") == 0){
			params.lambda = atof(argv[++i]);
			if(params.lambda <= 0) {
				fprintf(stderr,"\nError: lambda must be a positive real number <%s>\n",argv[i]);
				exit(-1);
			}
			if(params.lambda < 0.1)
				params.lambda = 0.1;

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

void printEmulationParameters(){
   printf("Emulation Parameters:\n");
   printf("    number to arrive = %d\n", params.n);
   if (traceDriven) {
	        printf("    r = %g\n", params.r);
	        printf("    B = %d\n", params.B);
	        printf("    tsfile = %s\n", params.fileName);
   }
   else {
	        printf("    lambda = %g\n", params.lambda);
	        printf("    mu = %g\n", params.mu);
	        printf("    r = %g\n", params.r);
	        printf("    B = %d\n", params.B);
	        printf("    P = %d\n", params.P);
   }
   printf("\n");
}

double getTime(struct timeval start,struct timeval end){
	return (((end.tv_sec - start.tv_sec)*1000.0) + ((end.tv_usec - start.tv_usec)/1000.0));
}

void parseLine(Packet *packet){

	char *token;
	if(fgets(fileBuf, 1026, fp) != NULL) {
		token = strtok(fileBuf, " \t");
		packet->interArrivalTime = atof(token);

		token = strtok(NULL, " \t");
		packet->tokensRequired = atoi(token);

		token = strtok(NULL, " \t");
		packet->serviceTime = atof(token);
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
		
	My402ListAppend(&q2, (void *)(packet));
	gettimeofday(&currentTime, NULL);
	packet->q2EnterTime = getTime(startTime, currentTime);
	printf("%012.3fms: p%d enters Q2\n", packet->q2EnterTime, packet->packetNo);

}

void sigsubhandler(int sig)
{
	//printf("Caught signal:%d\n",sig);
	if (pthread_mutex_trylock(&m)==0)
		pthread_mutex_unlock(&m);
	pthread_exit(0);
}

void *arrival(void *arg)
{   
	struct timeval prevArrival, currentTime, leave;
	
	prevArrival = startTime;
	leave = startTime;
	int i;
	sig_act2.sa_handler = sigsubhandler;
	sigaction(SIGUSR1, &sig_act2, NULL);
	//signal(SIGUSR1,sigsubhandler);
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
			packet->interArrivalTime = 1000.0/params.lambda;
			packet->tokensRequired = params.P;
			packet->serviceTime = 1000.0/params.mu;	
		}
		
		packet->packetNo = g_TotalPackets + 1;

		gettimeofday(&leave, NULL);

		if(packet->interArrivalTime > getTime(prevArrival,leave))
			usleep(1000*(packet->interArrivalTime - getTime(prevArrival,leave)));
		
		gettimeofday(&currentTime, NULL);
		packet->arrivalTime = getTime(startTime,currentTime);


		//g_AvgPacketInterArrivalTime += packet->interArrivalTime;
		g_AvgPacketInterArrivalTime += getTime(prevArrival,currentTime);
		prevArrival = currentTime;
		
		//remove packet if tokens required is more than depth B
		if((packet->tokensRequired) > (params.B)) {
			printf("%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms, dropped\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired, packet->interArrivalTime);
                g_PacketsDropped++;
                pthread_mutex_lock(&m);
                g_TotalPackets++;
                pthread_mutex_unlock(&m);
		}

		else {
			printf ("%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired, packet->interArrivalTime);
            
			pthread_mutex_lock(&m);
			g_TotalPackets++;
			My402ListAppend(&q1, (void *)(packet));

			gettimeofday(&currentTime, NULL);
			packet->q1EnterTime = getTime(startTime, currentTime);
			printf("%012.3fms: p%d enters Q1\n", packet->q1EnterTime, packet->packetNo); 
	
			Packet *headPacket = (Packet *)(My402ListFirst(&q1)->obj);
			
			if((headPacket->tokensRequired) <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(&q2) == 1)
					pthread_cond_broadcast(&cond);				
			}
			pthread_mutex_unlock(&m);


		}
		//gettimeofday(&leave, NULL);
	}

	return NULL;
}

void *token(void *arg)
{   
	struct timeval currentTime, prevArrival, leave;
	prevArrival = startTime;
	leave = startTime;

	sig_act.sa_handler = sigsubhandler;
	sigaction(SIGUSR1, &sig_act, NULL);
	//signal(SIGUSR1,sigsubhandler);
	double tokenWaitTime = 1000.0/params.r;
	while(1) {

		pthread_mutex_lock(&m);
		if(!My402ListEmpty(&q1)) {
			Packet *headPacket = (Packet *)(My402ListFirst(&q1)->obj);

			if((headPacket->tokensRequired) <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(&q2) == 1)
					pthread_cond_broadcast(&cond);
			}
		}
		pthread_mutex_unlock(&m);

		pthread_mutex_lock(&m);

		if(g_TotalPackets == params.n && My402ListEmpty(&q1)) {
			//fprintf(stderr, "No more Packets to process \n");
			pthread_mutex_unlock(&m);
            		pthread_exit(0);
		}
		pthread_mutex_unlock(&m);

		
		gettimeofday(&leave, NULL);
		if(tokenWaitTime > getTime(prevArrival,leave))
			usleep(1000*(tokenWaitTime - getTime(prevArrival,leave)));

		gettimeofday(&currentTime, NULL);
		prevArrival = currentTime;

		pthread_mutex_lock(&m);
		g_TokenNumber++;
		pthread_mutex_unlock(&m);

		pthread_mutex_lock(&m);
		if(g_TokensInBucket >= params.B) {
			printf("%012.3fms: token t%d arrives, dropped\n", getTime(startTime,currentTime), g_TokenNumber); 
			g_TokensDropped++;
		}
		else {
			g_TokensInBucket++;
			printf("%012.3fms: token t%d arrives, token bucket now has %d tokens\n", getTime(startTime,currentTime), g_TokenNumber, g_TokensInBucket);
		}
		pthread_mutex_unlock(&m);

		//gettimeofday(&leave, NULL);
	}
	
	return NULL;
}

void *server(void *arg)
{ 
	//int error;
	/*error = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	if (error != 0) {
		fprintf(stderr, "\nCannot set DISABLE condition while canceling Server thread :[%s]", strerror(error));
		pthread_exit(0);
	}*/

	struct timeval currentTime, processTime;
	//prevArrival = startTime;
	while(1) {

			pthread_mutex_lock(&m);
			if(gSighandlerDone == 1 || (My402ListEmpty(&q2) && My402ListEmpty(&q1) && g_TotalPackets==params.n)){
				pthread_mutex_unlock(&m);
				pthread_exit(0);
				//break;
			}

			if(gSighandlerDone != 1 && (My402ListEmpty(&q2) && !My402ListEmpty(&q1) && g_TotalPackets <= params.n)){
				pthread_cond_wait(&cond, &m);
			}


			pthread_mutex_unlock(&m);
			gettimeofday(&processTime, NULL);


			pthread_mutex_lock(&m);
			if(gSighandlerDone != 1 && (!My402ListEmpty(&q2))) {


				Packet *packet = (Packet *)(My402ListFirst(&q2)->obj);
				gettimeofday(&currentTime, NULL);

				//My402ListUnlink(&q2, My402ListFirst(&q2));
				pthread_mutex_unlock(&m);

				packet->q2LeaveTime = getTime(startTime,currentTime);
				printf("%012.3fms: p%d leaves Q2, time in Q2 = %.3fms\n", packet->q2LeaveTime, packet->packetNo, (packet->q2LeaveTime)-(packet->q2EnterTime));

				/*error = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
								if (error != 0) {
									fprintf(stderr, "\nCannot set DISABLE condition while canceling Server thread :[%s]", strerror(error));
									pthread_exit(0);
				}*/
				pthread_mutex_lock(&m);
				My402ListUnlink(&q2, My402ListFirst(&q2));
				pthread_mutex_unlock(&m);

				gettimeofday(&currentTime, NULL);
				packet->serviceStartTime = getTime(startTime,currentTime);
				printf("%012.3fms: p%d begins service at S, requesting %.3fms of service\n", packet->serviceStartTime, packet->packetNo, packet->serviceTime);

				gettimeofday(&currentTime, NULL);
				if(packet->serviceTime > getTime(processTime,currentTime))
					usleep(1000*(packet->serviceTime - getTime(processTime,currentTime)));

				gettimeofday(&currentTime, NULL);
				packet->serviceEndTime = getTime(startTime,currentTime);
				printf("%012.3fms: p%d departs from S, service time = %.3fms, time in system = %.3fms\n", packet->serviceEndTime, packet->packetNo, (packet->serviceEndTime) - (packet->serviceStartTime), (packet->serviceEndTime) - (packet->arrivalTime));

				g_AvgPacketServiceTime += (packet->serviceEndTime) - (packet->serviceStartTime);

				g_AvgPacketsInQ1 += (double)(packet->q1LeaveTime-packet->q1EnterTime);
				g_AvgPacketsInQ2 += (double)(packet->q2LeaveTime-packet->q2EnterTime);
				g_AvgPacketsInS += (double)(packet->serviceEndTime - packet->serviceStartTime);

				g_AvgTimePacketInSystem += (double)(packet->serviceEndTime - packet->arrivalTime);
				g_AvgTimePacketInSystem_Square += ((packet->serviceEndTime - packet->arrivalTime))*((packet->serviceEndTime - packet->arrivalTime));
				g_PacketsCompleted++;

			}
			else
				pthread_mutex_unlock(&m);
	}
	return NULL;
}

void clearQueues(){

	struct timeval currentTime;

	//printf("Clear Queues\n\n");
	while(!My402ListEmpty(&q1)) {

		Packet *packet = (Packet *)(My402ListFirst(&q1)->obj);
		gettimeofday(&currentTime, NULL);

		printf("%012.3fms: p%d removed from Q1\n", getTime(startTime,currentTime), packet->packetNo);
		My402ListUnlink(&q1, My402ListFirst(&q1));
	}

	while(!My402ListEmpty(&q2)) {

		Packet *packet = (Packet *)(My402ListFirst(&q2)->obj);
		gettimeofday(&currentTime, NULL);

		printf("%012.3fms: p%d removed from Q2\n", getTime(startTime,currentTime), packet->packetNo);
		My402ListUnlink(&q2, My402ListFirst(&q2));
	}


	return;
}

void *sighandler (void *arg)
{
	sigset_t set;
	int s;
        sigemptyset(&set);
        sigaddset(&set,SIGINT);
	for (;;) {
	   s = sigwait(&set);
	   //if (s != 0){
	   //	   printf("Error receiving signal %d", s);
	   //	   exit(0);
	   //}
	   pthread_mutex_lock(&m);
	   gSighandlerDone = 1;
	   //pthread_cond_broadcast(&cond);
	   pthread_mutex_unlock(&m);

	   pthread_kill (arrivalThread, SIGUSR1);
	   pthread_kill (tokenThread, SIGUSR1);

	   clearQueues();
	   //pthread_cancel (serverThread);
	   //pthread_join(serverThread, 0);
	   break;
	}
	return NULL;
}

void displayStatistics(){

	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	printf("%012.3fms: emulation ends\n",(double)(getTime(startTime,currentTime)));

	printf("\nStatistics:\n\n");

	if(g_TotalPackets != 0)
		printf("    average packet inter-arrival time = %.6f seconds\n", (double)(g_AvgPacketInterArrivalTime/(double)(g_TotalPackets)/1000.0));
	else
		printf("    average packet inter-arrival time = N/A (no packet arrived at this facility)\n");

	if(g_PacketsCompleted != 0)
		printf("    average packet service time = %.6f seconds\n\n", (double)(g_AvgPacketServiceTime/(double)(g_PacketsCompleted)/1000.0));
	else
		printf("    average packet service time = N/A (no packet arrived at server)\n\n");

	printf("    average number of packets in Q1 = %.6f\n", (double)(g_AvgPacketsInQ1/getTime(startTime,currentTime)));
	printf("    average number of packets in Q2 = %.6f\n", (double)(g_AvgPacketsInQ2/getTime(startTime,currentTime)));
	printf("    average number of packets at S = %.6f\n\n", (double)(g_AvgPacketsInS/getTime(startTime,currentTime)));

	if(g_PacketsCompleted != 0){
		printf("    average time a packet spent in system = %.6f seconds\n", (double)(g_AvgTimePacketInSystem/(double)(g_PacketsCompleted)/1000.0));

		g_StandardDeviation = sqrt((g_AvgTimePacketInSystem_Square/g_PacketsCompleted/1000000.0)-((g_AvgTimePacketInSystem/g_PacketsCompleted/1000.0)*(g_AvgTimePacketInSystem/g_PacketsCompleted/1000.0)));
		if(g_StandardDeviation < 0.000000001)
			g_StandardDeviation = 0.000000;
		printf("    standard deviation for time spent in system = %.6f seconds\n\n", g_StandardDeviation);
	}
	else {
		printf("    average time a packet spent in system = N/A (no packet arrived at server)\n");
		printf("    standard deviation for time spent in system = N/A (no packet arrived at server)\n\n");
	}

	if(g_TokenNumber != 0)
		printf("    token drop probability = %.6f\n", (double)(g_TokensDropped)/(double)(g_TokenNumber));
	else
		printf("    token drop probability = N/A (no tokens arrived at token bucket filter)\n");

	if(g_TotalPackets != 0)
		printf("    packet drop probability = %.6f\n\n", (double)(g_PacketsDropped)/(double)(g_TotalPackets));
	else
		printf("    packet drop probability = N/A (no packet arrived at this facility)\n");
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

	printEmulationParameters();
	int error;
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	error = pthread_sigmask(SIG_BLOCK,&set,NULL);

	if(error != 0){
		fprintf(stderr, "\ncan't create mask :[%s]", strerror(error));
		exit(1);
	}

	gettimeofday(&startTime, NULL);
	printf("%012.3fms: emulation begins\n",getTime(startTime,startTime));

	if((error = pthread_create(&arrivalThread, 0, arrival, NULL))){
       	fprintf(stderr, "\ncan't create arrival thread :[%s]", strerror(error));
		exit(1);
	}
	if((error = pthread_create(&tokenThread, 0, token, NULL))){
      	fprintf(stderr, "\ncan't create token depositing thread :[%s]", strerror(error));
		exit(1);
	}
	if((error = pthread_create(&serverThread, 0, server, NULL))){
       	fprintf(stderr, "\ncan't create server thread :[%s]", strerror(error));
		exit(1);
	}
	if((error = pthread_create(&sigThread, 0, sighandler, (void *)&set))){
        fprintf(stderr, "\ncan't create Signal thread :[%s]", strerror(error));
		exit(1);
	}
	
	pthread_join(arrivalThread,0);
	pthread_join(tokenThread,0);
	pthread_join(serverThread,0);

	displayStatistics();
	return 0;
}

