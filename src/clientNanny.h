#ifndef CLIENTNANNY_H 
#define CLIENTNANNY_H

#include "childNanny.h"
#include "clerkNanny.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>


#include "memwatch.h"
#include "monLL.h"
#include "linkedList.h"


void clientNannyFlow(void);
void clientNannyReceiveData(char *s);
void clientNannyTeardown(void);
void clientNannyCheckForProcesses(int signum);
void clientNannyLoop(void);
void clientNannyForkProcMon(void);
void unlinkFIFOandKillChildren(void);
void killOldProcnannys(void);

void clientNannyPrint(char* s);
void clientConnectToServer(char* nodeName, int portNum);
void clientInit(void);
void clientNannyLoop(void);

void receiveServerData(void);


#define DEBUGLOGGINFLAG 1
#define MY_PORT  60333
#define SERVNAME "localhost"

#endif 
