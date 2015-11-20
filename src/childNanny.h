#ifndef CHILDNANNY_H 
#define CHILDNANNY_H

#include "clerkNanny.h"
#include "clientNanny.h"
#include "memwatch.h"

void childNannyFlow(void);
void childNannySendDataToParent(void);
void childNannyreceiveDataFromParent(void);

#endif