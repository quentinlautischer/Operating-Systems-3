#ifndef CLIENTNANNY_H 
#define CLIENTNANNY_H

#include "childNanny.h"
#include "clerkNanny.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

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


#define DEBUGLOGGINFLAG 1

#endif 
