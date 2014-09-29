
#ifndef _WARMUP2_H_
#define _WARMUP2_H_

#include "cs402.h"


typedef struct tagPacket {

	int packetNo;
	int tokensRequired;
	double interArrivalTime;
	double serviceTime;
	double arrivalTime;

	double q1EnterTime;
	double q1LeaveTime;
	double q2EnterTime;
	double q2LeaveTime;
	double serviceStartTime;
	double serviceEndTime;
	
} Packet;

typedef struct tagParameters {

	double lamda;
	double r;
	double mu;
	int B;
	int P;
	int n;
	char *fileName;

} Parameters;

static void Usage();
void ProcessOptions(int argc, char *argv[]);
void setDefaultParameters();
Packet *createPacket();

void *arrival(void *);
void *token(void *);
void *server(void *);



#endif /*_WARMUP2_H_*/
