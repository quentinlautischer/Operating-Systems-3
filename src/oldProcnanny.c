#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "memwatch.h"

#define LOGFLAG 1

int killedProcesses = 0;
int numOfMonProc = 0;
int processSecondsToLive = 0;
char * line;
char * psLine;

void setTimeHeader(char *currentTime);
void forkAndExec(int argc, char* argv[]);
void forkProcMon(pid_t proc_pid, char* pid_name);
void killOldProcnannys();

int main(int argc, char* argv[]){
	line = (char *) malloc(1024); /* Size of file lines read */
	psLine = (char *) malloc(1024);
	char currentTime[500];

	const char* logDir = getenv("PROCNANNYLOGS");
	FILE* logFile;
	printf("Info: Log PATH:%s\n", (logDir!=NULL)? logDir : " No Environment Variable Found For Logging. Stdout > Console");

	FILE* fpin;
	extern FILE *popen();

	if ( argc == 2 ){ /* Ensure only one argumen provided */
		if (LOGFLAG) {
			logFile = freopen(logDir, "w+", stdout);

			if(logFile == NULL){
				printf("Error: Could Not Open Logging File");
			}
		}

		FILE* file = fopen( argv[1], "r" );
		if ( file == 0 ){
			printf("Could not open config file\n" );}
		else{
			/* Program Code Begins */
			killOldProcnannys();

			processSecondsToLive = atoi(fgets(line, sizeof(line), file));
	
			int i = 0;	
			for (; i < 128; i++){
				if ( fgets(line, sizeof(line), file) )	{
					strtok(line, "\n");
					sprintf(psLine, "ps -C %s -o pid=", line);

					if(!(fpin = popen(psLine, "r"))){
						printf("Error: Failed on popen of %s", psLine);
						exit(EXIT_FAILURE);
					}
					while(fgets(psLine, sizeof(psLine), fpin)){
						if(psLine){
							strtok(psLine, "\n");

							setTimeHeader(currentTime);
							printf("%s Info: Initializing monitoring of process '", currentTime);
							printf("%s' (PID %s).\n", line, psLine);
												
							pid_t proc_pid = (pid_t) strtol(psLine, NULL, 10);
						
							fflush(stdout);
							forkProcMon(proc_pid, line);
							fflush(stdout);
						} else {
							fflush(stdout);
							setTimeHeader(currentTime);
							printf("%s Info: No '%s' processes found.\n", currentTime, line);
							fflush(stdout);
						}
					}		
					pclose(fpin);				
				} else { 
					break; 
				}
			}
			int j = 0;
			int status;
			for (j = 0; j < numOfMonProc+1; j++){
				wait(&status);
				if (status == 0){
					killedProcesses++;
				}
			}
			setTimeHeader(currentTime);
			printf("%s Info: Exiting. %i process(es) killed. \n", currentTime, killedProcesses-1);			

			if(LOGFLAG){fclose(logFile);}
			fclose(file);
		}
	} else {
		printf("Error: Program accepts only one argument.\n");
	}	

	free(line);
	free(psLine);
	return 0;
}

void killOldProcnannys(){
	FILE* file;
	extern FILE *popen();

	char psLine[] = "ps -C procnanny -o pid=";

	if(!(file = popen(psLine, "r"))){
		printf("Error: Failed on popen of %s", psLine);
		exit(EXIT_FAILURE);
	}
	while(fgets(psLine, sizeof(psLine), file)){
							
		pid_t proc_pid = (pid_t) strtol(psLine, NULL, 10);
		if (getpid() != proc_pid){
			kill(proc_pid, SIGKILL);	
		}
	}
}

void setTimeHeader(char *currentTime){
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

void forkAndExec(int argc, char* argv[]){
	pid_t child_pid;
	child_pid = fork();
	if (child_pid >= 0){ /* fork success */
		if (child_pid == 0) { /* Child */
			char *command = argv[0];
			char *arguments[] = {NULL};
			execvp(command, arguments);						
		} else { /* Parent */}
	} else { /* Failure */
		perror("Fork Error");
		exit(0);					
	}
}

void forkProcMon(pid_t proc_pid, char* pid_name){
	pid_t child_pid;


	child_pid = fork();
	if (child_pid >= 0){ /* fork success */
		if (child_pid == 0) { /* Child */
			time_t nowMon = time(0);
			while(time(0) < nowMon + processSecondsToLive){
				sleep(1);
			}

			char currentTime[100];
			int killStatus = kill(proc_pid, SIGKILL);
			if (killStatus == 0) {
				setTimeHeader(currentTime);
				printf("%s Action: PID %ld (%s) killed after exceeding %i seconds. \n",currentTime,(long) proc_pid, pid_name, processSecondsToLive);
				
				free(line);
				free(psLine);
				exit(EXIT_SUCCESS);
			}
			free(line);
			free(psLine);
			exit(EXIT_FAILURE);						
							
		} else { /* Parent */
			numOfMonProc++;
		}
	} else { /* Failure */
		perror("Error: Fork Error");
		exit(0);
	}
}
