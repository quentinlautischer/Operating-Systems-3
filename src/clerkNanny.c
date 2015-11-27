#include "clerkNanny.h"
#include "select_servermod.h"

static void clerkNannySendDataToClient(char *s);
static void setTimeHeader(char *currentTime);

void clerkNannyMainLoop(void){
	clerkNannyParseConfigFile();
	while(1){

	};
	clerkNannySendDataToClient("");
}

void clerkNannySetup(void){
	const char* logDir = getenv("PROCNANNYLOGS");
	fileLine = (char*)malloc(1024 * sizeof(char));
	msgData = (char*)malloc(1024 * sizeof(char));
	head = NULL;

	logFile = fopen(logDir, "w+");
	if(logFile == NULL){
		fprintf(stdout, "Error: Could Not Open Logging File");
	}
	fprintf(stdout,"Debug: Log PATH:%s\n", (logDir!=NULL)? logDir : " No Environment Variable Found For Logging.");
	fprintf(logFile,"Debug: Log PATH:%s\n", (logDir!=NULL)? logDir : " No Environment Variable Found For Logging.");
}

void cleanLinkedList(void){
	item *tmp = NULL;
	curr = head;
	while(curr){
		tmp = curr;
		curr = curr->next;
		free(tmp);
	}
	head = NULL;
}

void clerkNannySerializeConfigData(void){
	curr = head;
	while(curr){
		sprintf(fileLine, "%s %s", curr->name,curr->secs);
		clerkNannyPrint(fileLine, DEBUG);
		
		curr = curr->next;
	}
}

void clerkNannyTeardown(void){
	cleanLinkedList();

	free(msgData);
	free(configFileName);
	free(fileLine);
	if(logFile != NULL){
		fclose(logFile);
	}
}

void clerkNannyReceiveData(char* s, int lt){
	clerkNannyPrint(s, lt);
}

static void clerkNannySendDataToClient(char* s){
	// clientNannyReceiveData(s);
}

void clerkNannyParseConfigFile(void){
	clerkNannyPrint(configFileName, DEBUG);
	configFile = fopen( configFileName, "r" );
	if ( configFile == 0 ){
		clerkNannyPrint("ERROR: Could not open config file\n", BOTH);
	} else{
		//Do work
		if(head != NULL){
			cleanLinkedList();
		}
		int i = 0;
		for (; i < 128; i++){
			if (fgets(fileLine, 1024, configFile)){
				strtok(fileLine, "\n");
				clerkNannyPrint(fileLine, DEBUG);

				char seps[] = " ";
				char *token = strtok( fileLine, seps );

				curr = (item *)malloc(sizeof(item));
			    if( token != NULL ){
			    	// clerkNannyPrint(token, DEBUG);
			    	sprintf(curr->name, "%s",token);
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	// clerkNannyPrint(token, DEBUG);	
			    	sprintf(curr->secs, "%s",token);
			    }
			    curr->next = head;
			    head = curr;
			}
		}
		fclose(configFile);			
	}
	// sighupFlag = 1;
}

void clerkNannySendNewConfigToClients(void){
	serverWriteNewConfigToClients();
}

void clerkNannyPrint(char* s, int lt){
	setTimeHeader(currentTime);

	switch (lt){
		case LOGFILE:
			fprintf(logFile,"%s %s\n",currentTime,s);
			break;
		case SYSTEM:
			fprintf(stdout,"%s %s\n",currentTime,s);
			break;
		case BOTH:
			fprintf(logFile,"%s %s\n",currentTime,s);
			fprintf(stdout,"%s %s\n",currentTime,s);
			break;
		case DEBUG:
			if(DEBUGLOGGINFLAG){fprintf(stdout,"%s %s\n",currentTime,s);}
			break;
		default:
			if(DEBUGLOGGINFLAG){fprintf(stdout,"Invalid logType provided \n");}
			break;
	}
}

static void setTimeHeader(char *currentTime){
	struct tm *localTime;
	time_t now = time(NULL);
	localTime = localtime(&now);
	if (localTime == NULL){
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(currentTime, 100, "[%a %b %d %H:%M:%S %Z %Y]", localTime) == 0 ){ //had to hard code 100 cause c sucks lol
		printf("strftime failed");
		exit(EXIT_FAILURE);	
	}
}

