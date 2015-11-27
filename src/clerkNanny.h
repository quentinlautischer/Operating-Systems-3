#ifndef CLERKNANNY_H 
#define CLERKNANNY_H

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "memwatch.h"

#define MAXLINE 256

enum LogType { LOGFILE, SYSTEM, BOTH, DEBUG };

#define DEBUGLOGGINFLAG 1

char *configFileName;
char currentTime[100];
char *fileLine;

char *msgData;

FILE* logFile;
FILE* configFile;

struct ConfigExtractedData {
    char name[196];
    char secs[60];
    struct ConfigExtractedData *next;
};

typedef struct ConfigExtractedData item;

item * head;
item * curr;

void clerkNannySetup(void);
void clerkNannyTeardown(void);

void clerkNannyReceiveData(char* s,  int lt);
void clerkNannyPrint(char* s, int logType);
void clerkNannyParseConfigFile(void);
void cleanLinkedList(void);
void clerkNannySerializeConfigData(void);
void clerkNannyMainLoop(void);
void receiveMonitorList(void);
void clerkNannySendNewConfigToClients(void);

#endif 