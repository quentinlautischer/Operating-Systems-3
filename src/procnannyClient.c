#ifndef MAIN_H 
#define MAIN_H

#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>


#include "childNanny.h"
#include "clientNanny.h"

#include "memwatch.h"

#endif

void teardown(void);
void signalCallbackHandler(int signum);

char *nodeName;
int portNum;

int killedProcesses;

char *msgData;
pid_t parent_pid;

int main(int argc, char* argv[]){

	signal(SIGINT, signalCallbackHandler);
	signal(SIGALRM, clientNannyCheckForProcesses);

	clientNannyPrint("Startup");

	if ( argc == 3 ){ /* Ensure only one argument provided */
		nodeName = malloc(strlen(argv[1]) + 1);
		strcpy(nodeName, argv[1]);
		portNum = strtol(argv[2], NULL, 10);

		clientConnectToServer(nodeName, portNum):

		// clientNannyFlow(); //Main Loop

		while(1){};

	
	} else {
		clientNannyPrint("Error Startup");
	}
	clientNannyPrint("Client Exit Startup");
	return EXIT_SUCCESS;
}

void teardown(void){
	clientNannyTeardown();
}

void signalCallbackHandler(int signum){
	if(getpid() == parent_pid){
		// sprintf(msgData, "Info: Caught SIGINT. Exiting cleanly. %d process(es) killed.", killedProcesses);
		// clerkNannyReceiveData(msgData, BOTH);	
	}
	teardown();
	exit(signum);
}




