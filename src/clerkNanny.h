#ifndef CLERKNANNY_H 
#define CLERKNANNY_H

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <signal.h>


#include "childNanny.h"
#include "clientNanny.h"

#include "memwatch.h"

#define MAXLINE 256

void clerkNannySetup(void);
void clerkNannyTeardown(void);

void clerkNannyReceiveData(char* s,  int lt);
void clerkNannyParseConfigFile(int signum);
void cleanLinkedList(void);
void clerkNannySerializeConfigData(void);
void clerkNannyMainLoop(void);

enum LogType { LOGFILE, SYSTEM, BOTH, DEBUG };

#define DEBUGLOGGINFLAG 1

#endif 