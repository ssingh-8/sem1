
void *arrival(void *arg)
{   
	struct timeval prevArrival, currentTime, leave;
	
	prevArrival = startTime;
	leave = prevArrival;
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
		gettimeofday(&currentTime, NULL);
		if(packet->interArrivalTime > getTime(prevArrival,leave))
			usleep(1000*(packet->interArrivalTime - getTime(prevArrival,leave)));
		
		packet->arrivalTime = getTime(startTime,currentTime);
		prevArrival = currentTime;
		
		
		//remove packet if tokens required is more than depth B
		if(packet->tokensRequired > params.B) {
			printf("%012.3fms: p%d arrives, needs %d tokens, dropped\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired);
                g_PacketsDropped++;
		}

		else {
			printf ("%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n", packet->arrivalTime, packet->packetNo, packet->tokensRequired, packet->interArrivalTime);
            
			pthread_mutex_lock(&m);

			My402ListAppend(q1, (void *)packet);
			packet->q1EnterTime = getTime(startTime, currentTime);
			printf("%012.3fms: p%d enters Q1\n", packet->q1EnterTime, packet->packetNo); 
	
			Packet *headPacket = (Packet *)(My402ListFirst(q1)->obj); 
			
			if(headPacket->tokensRequired <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(q2) == 1)
					pthread_cond_broadcast(&cond);				
			}
			pthread_mutex_unlock(&m);
		
			//check if head packet in q1 can be moved to q2
			//My402ListElem *elem = My402ListFirst(q1);
						
			/*if(headPacket->tokensRequired <= g_TokensInBucket) {
				movePktFromQ1toQ2();
				if(My402ListLength(q2) == 1)
					pthread_cond_broadcast(&cond);				
			}*/

		}		

		gettimeofday(&leave, NULL); 
	}
	return NULL;
}

void *token(void *arg)
{   
	struct timeval prevArrival, currentTime, leave;
	prevArrival = startTime;
	leave = prevArrival;
	while(params.n <= 0 || !My402ListEmpty(q1)) {
		
		if((1000/params.r) > getTime(prevArrival,leave))
			usleep(1000*((1000/params.r) - getTime(prevArrival,leave)));

		gettimeofday(&currentTime, NULL);
		g_TokenNumber++;

		pthread_mutex_lock(&m);
		if(g_TokensInBucket >= params.B) {
			printf("%012.3fms: token t%d arrives, dropped\n", getTime(startTime, currentTime), g_TokenNumber); 
			g_TokensDropped++;
		}
		else {
			g_TokensInBucket++;
			printf("%012.3fms: token t%d arrives, token bucket now has %d tokens\n", getTime(startTime, currentTime), g_TokenNumber, g_TokensInBucket);
		}
		
		Packet *headPacket = (Packet *)(My402ListFirst(q1)->obj); 
			
		if(headPacket->tokensRequired <= g_TokensInBucket) {
			movePktFromQ1toQ2();
			if(My402ListLength(q2) == 1)
				pthread_cond_broadcast(&cond);				
		}
		pthread_mutex_unlock(&m);
			
	gettimeofday(&leave, NULL); 	
	}
	
	return NULL;
}

