#ifndef SELECTSERVER_H 
#define SELECTSERVER_H


#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* we use this structure to keep track of each connection to us */
struct con {
    int sd;     /* the socket for this connection */
    int state;  /* the state of the connection */
    struct sockaddr_in sa; /* the sockaddr of the connection */
    size_t  slen;   /* the sockaddr length of the connection */
    char *buf;  /* a buffer to store the characters read in */
    char *bp;   /* where we are in the buffer */
    size_t bs;  /* total size of the buffer */
    size_t bl;  /* how much we have left to read/write */
};

/*
 * we will accept a maximum of 256 simultaneous connections to us.
 * While you could make this a dynamically allocated array, and
 * use a variable for maxconn instead of a #define, that is left
 * as an exercise to the reader. The necessity of doing this
 * in the real world is debatable. Even the most monsterous of
 * daemons on real unix machines can typically only deal with several
 * thousand simultaeous connections, due to limitations of the
 * operating system and process limits. so it might not be worth it
 * in general to make this part fully dynamic, depending on your
 * application. For example, there is no point in allowing for
 * more connections than the kernel will allow your process to
 * have open files - so run 'ulimit -a' to see what that is as
 * an example of a typical reasonable number, and bear in mind
 * you may have a few more files open than just your sockets
 * in order to do anything really useful
 */
#define MAXCONN 256
struct con connections[MAXCONN];

#define BUF_ASIZE 256 /* how much buf will we allocate at a time. */

/* states used in struct con. */
#define STATE_UNUSED 0
#define STATE_READING 1
#define STATE_WRITING 2

struct sockaddr_in sockname;
int sd;              /* our listen socket */
fd_set *readable;
fd_set *writable;

void closecon (struct con *cp, int initflag);
void handlewrite(struct con *cp, char* strToWrite);
void handleread(struct con *cp);
int getPortNumber(int socketNum);
void serverBoot(int argc,  char *argv[]);

void serverTeardown(void);

void serverRead(void);
void serverWriteNewConfigToClients(void);

void setWritable(void);
void setReadable(void);


#endif 