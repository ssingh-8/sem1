void parseLine(Packet *packet){

	char *token;
	int count = 0;
	char ch = '\t';
	fgets(fileBuf, 1026, fp); 


	char *temp = fileBuf;

	while(*temp!='\0'){
		while(isdigit(*temp)){
			
		}
			
			
	}

	char *start_ptr = strchr(fileBuf,'\t');
	//char *tab_ptr = start_ptr;
	while(start_ptr != NULL) {
		*start_ptr = ' ';
		*start_ptr = strchr(fileBuf,'\t');
	}
	
	token = strtok(fileBuf, ch);
	if(token != NULL)
		printf( " %s\n", token );

	
	token = strtok(NULL, ch);
	if(token != NULL)
		printf( " %s\n", token );

	token = strtok(NULL, ch);
	if(token != NULL)
		printf( " %s\n", token );



		while((*start_ptr == ' ') || (*start_ptr == '\t')){
			start_ptr++;
			flag = 1;
		}		

		start_ptr++;
		if(flag == 1){
			strcpy(str[count], start_ptr);	
			strcat(str[count], '\0');
			count++;
			flag = 0;
		}
		
	printf("\n1st = %c\n2nd = %c\n3rd = %c\n",str[0],str[1],str[2]);
/*		
			*tab_ptr++ = '\0';
			str[count] = start_ptr;			
			start_ptr = tab_ptr;
			tab_ptr = strchr(start_ptr,'\t');
			count++;
			if(count>3){
        			fprintf(stderr, "The number of fields in the transaction are more : %d\n", count+1);
        			Usage();
    			} 
		}
		if(count<3){
        		fprintf(stderr, "The number of fields in the transaction are less : %d\n", count+1);
        		Usage();
    		} 
		str[count] =  start_ptr;


	//incomplete
	char ch = '\t';
	char *token;
	fgets(fileBuf, 1026, fp); 
	
	token = strtok(fileBuf, ch);
	
	if(strcmp(token, " ") == 0)
		
      	//printf( " %s\n", token );
      	token = strtok(NULL, s);
	printf("");
   	
	printf(""); */
        //packet->interArrivalTime 
	//packet->tokensRequired
	//packet->serviceTime
}
