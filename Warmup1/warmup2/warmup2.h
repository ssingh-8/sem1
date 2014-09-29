
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





#endif /*_WARMUP2_H_*/
