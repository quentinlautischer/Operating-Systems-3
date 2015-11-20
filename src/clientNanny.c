#include "clientNanny.h"

static void clientNannySendDataToClerk(char* s, int lt);
static void clientNannySendDataToChild(void);

char *msgData;
char *psLine;
char *childPID;
char username[256];
int killedProcesses = 0;
char pidVal[150];
pid_t parent_pid;

char line[MAXLINE];
int fd = 0;
char *myfifo;

int sighupFlag;

FILE* fpin;

struct ConfigExtractedData {
	char name[1024];
	char secs[1024];
	struct ConfigExtractedData *next;
};

typedef struct ConfigExtractedData item;

item * head;
item * curr;

BoredChild *boredChild_head;
MonChild *monChild_head;

MonChild *iterCurr;
BoredChild *iterCurrr; 

void clientNannyFlow(void){
	msgData = (char*)malloc(1024 * sizeof(char));
	psLine = (char*)malloc(1024 * sizeof(char));
	childPID = (char*)malloc(1024 * sizeof(char));

	iterCurr = NULL;
	iterCurrr = NULL;

	boredChild_head = NULL;
	monChild_head = NULL;

	parent_pid = getpid();

	myfifo = (char*)malloc(1024 * sizeof(char));

	extern FILE *popen();

	sprintf(msgData,"Info: Parent process is PID %d", getpid());
	clientNannySendDataToClerk(msgData, LOGFILE);

	//GET CURRENT USER NAME
	sprintf(psLine, "id -u -n");
	if(!(fpin = popen(psLine, "r"))){
		sprintf(msgData, "Error: Failed on popen of %s", psLine);
		clientNannySendDataToClerk(msgData, BOTH);
	} else{
		while(fgets(psLine, 1024, fpin)){
			strtok(psLine, "\n");
			sprintf(username, "%s", psLine);
			// clientNannySendDataToClerk(username, DEBUG);
		}
		fclose(fpin);	
	}
	sprintf(msgData, "Info: Parent process is PID %d", getpid());
	clientNannySendDataToClerk(msgData, LOGFILE);

	killOldProcnannys();

	clerkNannyParseConfigFile(SIGHUP);//Should trigger its inital Check of processes
}

void clientNannyLoop(void){}

void unlinkFIFOandKillChildren(void){
	iterCurr = monChild_head;
	while(iterCurr != NULL){
		sprintf(myfifo, "%dp2c", iterCurr->childPID);
        if (unlink(myfifo) < 0) {
        	sprintf(msgData, "ERROR: Cannot unlink p2c, child: %d", iterCurr->childPID);
            clientNannySendDataToClerk(msgData, DEBUG);
        }
        sprintf(myfifo, "%dc2p", iterCurr->childPID);
        if (unlink(myfifo) < 0) {
        	sprintf(msgData, "ERROR: Cannot unlink cp2, child: %d", iterCurr->childPID);
            clientNannySendDataToClerk(msgData, DEBUG);
        }
        // kill(SIGKILL, iterCurr->childPID);
		iterCurr = iterCurr->next;
	}

	iterCurrr = boredChild_head;
	while(iterCurrr != NULL){
		sprintf(myfifo, "%dp2c", iterCurrr->pid);
        if (unlink(myfifo) < 0) {
        	sprintf(msgData, "ERROR: Cannot unlink p2c, child: %d", iterCurrr->pid);
            clientNannySendDataToClerk(msgData, DEBUG);
        }
        sprintf(myfifo, "%dc2p", iterCurrr->pid);
        if (unlink(myfifo) < 0) {
        	sprintf(msgData, "ERROR: Cannot unlink cp2, child: %d", iterCurrr->pid);
            clientNannySendDataToClerk(msgData, DEBUG);
        }
        // kill(SIGKILL, (pid_t) iterCurrr->pid);
		iterCurrr = iterCurrr->next;
	}
}

void clientNannyTeardown(void){
	unlinkFIFOandKillChildren();
	monLL_clear(&monChild_head);
	stack_clear(&boredChild_head);
	free(childPID);
	free(msgData);
	free(psLine);
	free(myfifo);


	free(iterCurr);
	free(iterCurrr);
	
	// if(fpin != NULL){
	// 	fclose(fpin);
	// }
}

static void clientNannySendDataToClerk(char* s, int lt){
	clerkNannyReceiveData(s, lt);
}

static void clientNannySendDataToChild(void){
	return;
}

void clientNannyReceiveData(char* s){
	return;
}


void clientNannyCheckForProcesses(int signum){
	alarm(5);
	sprintf(msgData,"Checking Processes");
	clientNannySendDataToClerk(msgData, DEBUG);
	int whileFixFlag = 0;

	curr = head;
	while(curr){
		sprintf(psLine, "pgrep -u %s %s", username, curr->name); //REMEMBER TO REPLACE TO USERNAME
		if(!(fpin = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
		} else {
			sprintf(pidVal, "%s", "0");
			whileFixFlag = 0;	
			while(fgets(pidVal, sizeof(pidVal), fpin)){
				whileFixFlag = 1;
				if(atoi(pidVal)){
					strtok(pidVal, "\n");
			
					clientNannySendDataToChild();
					//IF PID NOT IN TABLE THEN START MONITORING
					if(monLL_elem(&monChild_head, (pid_t) atoi(pidVal)) == 0){
						// clientNannySendDataToClerk("DEBUG: PID not in table.", DEBUG);
						// sprintf(msgData, "%d", stack_len(boredChild_head));
						sprintf(msgData, "Info: Initializing monitoring of process '%s' (PID %s).", curr->name, pidVal);
						clientNannySendDataToClerk(msgData, BOTH);		
						if(stack_len(boredChild_head) == 0 ){
							//THEN FORK A NEW CHILD
							clientNannySendDataToClerk("FORKING A NEW CHILD", DEBUG);
							clientNannyForkProcMon(); //LALA BIG FORK
						} else {
							clientNannySendDataToClerk("USING AN EXISTING CHILD", DEBUG);
							//USE EXISTING CHILD 
							int child_pid = stack_pop(&boredChild_head);

							sprintf(myfifo, "%dp2c", child_pid);
							mkfifo(myfifo, 0666);

							fd = open(myfifo, O_WRONLY);
							sprintf(msgData, "%s %s %s %d\n", curr->name, curr->secs, pidVal, child_pid);
							write(fd, msgData, (strlen(msgData)+1));
							close(fd);

							monLL_push(&monChild_head, (pid_t) atoi(pidVal), (pid_t) child_pid, curr->name, curr->secs);
							// TELL CHILDPID TO MON pidVal
						}
					} //ALREADY BEING MONITORED. DO NOTHING.
				}
			}
			if (!whileFixFlag && sighupFlag){
				//clientNannySendDataToClerk(psLine, DEBUG);
				sprintf(msgData, "Info: No '%s' processes found.", curr->name);
				clientNannySendDataToClerk(msgData, BOTH);

			} 
			fclose(fpin);	
			curr = curr->next; // Might need to go into next brace
		}
		
	}
	sighupFlag = 0;	
	iterCurr = monChild_head;
	while(iterCurr != NULL){
		sprintf(msgData, "ITERATING CHILD %d MONS %d",iterCurr->childPID, iterCurr->monPID);
		clientNannySendDataToClerk(msgData, DEBUG);

		sprintf(myfifo, "%dc2p", iterCurr->childPID);

		fd = open(myfifo, O_RDONLY|O_NONBLOCK);
		if(fd){
			// if(read(fd, line, MAXLINE) == -1) {clientNannySendDataToClerk("ERROR: Reading c2p iter check", DEBUG);}
			read(fd, line, MAXLINE);
			strtok(line, "\n");
			sprintf(msgData, "PARENT READS DATA: %s", line);
			clientNannySendDataToClerk(msgData, DEBUG);

			// if(close(fd)){clientNannySendDataToClerk("ERROR: Closing c2p during iter check", DEBUG);}
			if(monLL_elem(&monChild_head, (pid_t) atoi(line))) {
				if (atoi(line) <=0 ){
					// Did not kill but is gone.
				} else {
					// did kill
					killedProcesses++;
					sprintf(msgData, "Action: PID %d (%s) killed after exceeding %s seconds.", iterCurr->monPID, iterCurr->name, iterCurr->secs);
					clientNannySendDataToClerk(msgData, BOTH);
					// [Mon Oct 26 11:29:17 MST 2015] Action: PID 332 (a.out) killed after exceeding 120 seconds.
				}
				sprintf(line, "%s", "");
				stack_push(&boredChild_head, iterCurr->childPID);

				clientNannySendDataToClerk("REMOVING NODE FROM MON_LL DATA", DEBUG);
				iterCurr = monLL_remove(monChild_head, iterCurr->monPID);
				monChild_head = iterCurr;
	
				if(close(fd)){clientNannySendDataToClerk("ERROR: Closing fd after node removal", DEBUG);}//NEED TO FLUSH MAYBE ?
			continue;
			}
		} else {clientNannySendDataToClerk("ERROR: opening c2p during iter check", DEBUG);}
		if (iterCurr != NULL){
			iterCurr = iterCurr->next;
		}
	}
}

void clientNannyForkProcMon(void){
	pid_t child_pid;

	child_pid = fork();
	if (child_pid >= 0){ /* fork success */
		if (child_pid == 0) { /* Child */
			pid_t proc_pid;
			char* pid_name;
			char* pid_secs;
			char* myPid;
			sprintf(msgData, "I AM NEW CHILD %d", getpid());
			clientNannySendDataToClerk(msgData, DEBUG);

			while(1){
				sprintf(myfifo, "%dp2c", getpid());
    			fd = open(myfifo, O_RDONLY); //SHOULD BLOCK UNTIL OPEN FOR WRITTING
    			if(fd > 0){
	    			if(read(fd, line, MAXLINE) == -1) {clientNannySendDataToClerk("ERROR: Child Reading p2c", DEBUG);}
	    			strtok(line, "\n");
	    			sprintf(msgData, "CHILD READS DATA: %s", line);
					clientNannySendDataToClerk(msgData, DEBUG);	
	    			if(close(fd)){clientNannySendDataToClerk("ERROR: Child error Closing fifo p2c", DEBUG);}
	    		} else {clientNannySendDataToClerk("ERROR: Opening from p2c fifo", DEBUG);}

				char seps[] = " ";
				char *token = strtok( line, seps );

			    if( token != NULL ){
			    	pid_name = token;
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	pid_secs = token;
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	proc_pid = (pid_t) strtol(token, NULL, 10);
			    }
			    token = strtok( NULL, seps );
			    if( token != NULL ){
			    	myPid = token;
			    }

				sprintf(msgData, "NOW KNOWS PID_NAME: %s", pid_name);
				clientNannySendDataToClerk(msgData, DEBUG);	

				sprintf(msgData, "NOW KNOWS PID_Sec: %s", pid_secs);
				clientNannySendDataToClerk(msgData, DEBUG);	

				sprintf(msgData, "NOW KNOWS PID_PROC: %d", proc_pid);
				clientNannySendDataToClerk(msgData, DEBUG);	

				sprintf(msgData, "NOW KNOWS MY_PID: %s", myPid);
				clientNannySendDataToClerk(msgData, DEBUG);	


				if(proc_pid == -1){
					exit(0);
				}

				sleep(atoi(pid_secs));


				int killStatus = kill(proc_pid, SIGKILL);

				if (killStatus == 0) {
					clientNannySendDataToClerk("KILLED PROCESS", DEBUG);
					sprintf(msgData, "%d\n", proc_pid);
				} else {
					clientNannySendDataToClerk("ERROR KILLING PROCESS", DEBUG);
					sprintf(msgData, "%s\n", "-1");
				}
				sprintf(myfifo, "%sc2p", myPid);
				fd = open(myfifo, O_WRONLY|O_NONBLOCK);
				if(fd){
					if(write(fd, msgData, (strlen(msgData)+1)) == -1) {clientNannySendDataToClerk("ERROR: Writing c2p", DEBUG);}
					clientNannySendDataToClerk("CHILD RETURNED KILL DATA", DEBUG);
					clientNannySendDataToClerk(msgData, DEBUG);
					// if(close(fd)){clientNannySendDataToClerk("ERROR: closing c2p fifo", DEBUG);}
				} else {
					clientNannySendDataToClerk("ERROR: Child Error Opening fifo c2p", DEBUG);
				}						
			}

			// clerkNannyTeardown();
			// clientNannyTeardown();
			// exit(1);

		} else { /* Parent */
			sprintf(myfifo, "%dp2c", child_pid);
			if (mkfifo(myfifo, 0666)){
				clientNannySendDataToClerk("Error making p2c fifo", DEBUG);
			}

			sprintf(myfifo, "%dc2p", child_pid);
			if (mkfifo(myfifo, 0666)){
				clientNannySendDataToClerk("Error making c2p fifo", DEBUG);
			}

			sprintf(myfifo, "%dp2c", child_pid);
			fd = open(myfifo, O_WRONLY);
			if(fd){
				sprintf(msgData, "%s %s %s %d\n", curr->name, curr->secs, pidVal, child_pid);
				if(write(fd, msgData, (strlen(msgData)+1)) == -1) {clientNannySendDataToClerk("ERROR: Writing p2c", DEBUG);}
				if(close(fd)){clientNannySendDataToClerk("ERROR: closing p2c fifo", DEBUG);}
			} else {
				clientNannySendDataToClerk("ERROR: opening p2c fifo", DEBUG);
			}
			
			monLL_push(&monChild_head, (pid_t) atoi(pidVal), child_pid, curr->name, curr->secs);
			// TELL CHILDPID TO MON pidVal
		}
	} else { /* Failure */
		clientNannySendDataToClerk("ERROR: Fork Error", DEBUG);
		exit(1);
	}
}

void killOldProcnannys(){
	FILE* file;
	extern FILE *popen();

	sprintf(psLine, "%s", "ps -C procnanny -o pid="); //REMEMBER TO REPLACE TO USERNAME
		if(!(file = popen(psLine, "r"))){
			sprintf(msgData,"Error: Failed on popen of %s", psLine);
			clientNannySendDataToClerk(msgData, BOTH);
		} else {
			sprintf(pidVal, "%s", "0");
			while(fgets(pidVal, sizeof(pidVal), fpin)){
				if(atoi(pidVal)){
					strtok(pidVal, "\n");
					pid_t proc_pid = (pid_t) strtol(pidVal, NULL, 10);
					if (getpid() != proc_pid){
						if(!kill(proc_pid, SIGKILL)){
							sprintf(msgData, "Killed another Procnanny with pid: %d", proc_pid);
							clientNannySendDataToClerk(msgData, BOTH);
						}	
					}
				}
			}
		fclose(file);
		}
}

