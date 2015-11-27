/*
 * Copyright (c) 2008 Bob Beck <beck@obtuse.com>
 * Some changes (related to the port number) by Paul Lu, March 2011.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * select_server.c - an example of using select to implement a non-forking
 * server. In this case this is an "echo" server - it simply reads
 * input from clients, and echoes it back again to them, one line at
 * a time.
 *
 * to use, cc -DDEBUG -o select_server select_server.c
 * or cc -o select_server select_server.c after you read the code :)
 *
 * then run with select_server PORT
 * where PORT is some numeric port you want to listen on.
 * (Paul Lu:  You can also get the OS to choose the port by not specifying PORT)
 *
 * to connect to it, then use telnet or nc
 * i.e.
 * telnet localhost PORT
 * or
 * nc localhost PORT
 * 
 */
#include "clerkNanny.h"
#include "select_servermod.h"

char *msgData;

static void usage(void);

static void usage(void)
{
	extern char * __progname;
/* Paul Lu */
	fprintf(stderr, "usage: %s     (to have port assigned by OS), or\n",
                __progname);
	fprintf(stderr, "usage: %s portnumber\n", __progname);
	exit(1);
}


/*
 * get a free connection structure, to save a new connection in
 */
struct con * get_free_conn(void)
{
	fprintf(stdout, "%s", "Connection incomming \n");
	int i;
	for (i = 0; i < MAXCONN; i++) {
		if (connections[i].state == STATE_UNUSED)
			return(&connections[i]);
	}
	return(NULL); /* we're all full - indicate this to our caller */
}



/*
 * close or initialize a connection - resets a connection to the default,
 * unused state.
 */
void closecon (struct con *cp, int initflag)
{
	if (!initflag) {
		if (cp->sd != -1)
			close(cp->sd); /* close the socket */
			fprintf(stdout, "%s %d\n", "Closing the socket", cp->sd);
		free(cp->buf); /* free up our buffer */
	}
	memset(cp, 0, sizeof(struct con)); /* zero out the con struct */
	cp->buf = NULL; /* unnecessary because of memset above, but put here
			 * to remind you NULL is 0.
			 */
	cp->sd = -1;
}

/*
 * handlewrite - deal with a connection that we want to write stuff
 * to. assumes the caller has checked that cp->sd is writeable
 * by using select(). once we write everything out, change the
 * state of the connection to the reading state.
 */
void handlewrite(struct con *cp, char* strToWrite)
{
	ssize_t i;

	/*
	 * assuming before we are called, cp->sd was put into an fd_set
	 * and checked for writeability by select, we know that we can
	 * do one write() and write something. We are *NOT* guaranteed
	 * how much we can write. So while we will be able to write bytes
	 * we don't know if we will get a whole line, or even how much
	 * we will get - so we do *exactly* one write. and keep track
	 * of where we are. If we don't want to block, we can't do
	 * multiple writes to write everything out without calling
	 * select() again between writes.
	 */
	sprintf(cp->buf, "%s", strToWrite);
	fprintf(stdout, "About to write '%s' to 'socket %d'\n", cp->buf , cp->sd);
	i = write(cp->sd, cp->bp, cp->bl);
	if (i == -1) {
		if (errno != EAGAIN) {
			/* the write failed */
			closecon(cp, 0);
		}
		/*
		 * note if EAGAIN, we just return, and let our caller
		 * decide to call us again when socket is writable
		 */
		return;
	}
	/* otherwise, something ok happened */
	cp->bp += i; /* move where we are */
	cp->bl -= i; /* decrease how much we have left to write */
	if (cp->bl == 0) {
		/* we wrote it all out, hooray, so go back to reading */
		cp->state = STATE_READING;
		cp->bl = cp->bs; /* we can read up to this much */
		cp->bp = cp->buf;	    /* we'll start at the beginning */
	}
}

/*
 * handleread - deal with a connection that we want to read stuff
 * from. assumes the caller has checked that cp->sd is writeable
 * by using select(). If a newline is seen at the end of what we
 * are reading, change the state of this connection to the writing
 * state.
 */
void handleread(struct con *cp)
{
	ssize_t i;

	/*
	 * first, let's make sure we have enough room to do a
	 * decent sized read.
	 */

	if (cp->bl < 10) {
		char *tmp;
		tmp = realloc(cp->buf, (cp->bs + BUF_ASIZE) * sizeof(char));
		if (tmp == NULL) {
			/* we're out of memory */
			closecon(cp, 0);
			return;
		}
		cp->buf = tmp;
		cp->bs += BUF_ASIZE;
		cp->bl += BUF_ASIZE;
		cp->bp = cp->buf + (cp->bs - cp->bl);
	}

	/*
	 * assuming before we are called, cp->sd was put into an fd_set
	 * and checked for readability by select, we know that we can
	 * do one read() and get something. We are *NOT* guaranteed
	 * how much we can get. So while we will be able to read bytes
	 * we don't know if we will get a whole line, or even how much
	 * we will get - so we do *exactly* one read. and keep track
	 * of where we are. If we don't want to block, we can't do
	 * multiple reads to read in a whole line without calling
	 * select() to check for readability between each read.
	 */

	i = read(cp->sd, cp->bp, cp->bl);
	if (i == 0) {
		/* 0 byte read means the connection got closed */
		closecon(cp, 0);
		return;
	}
	if (i == -1) {
		if (errno != EAGAIN) {
			/* read failed */
			err(1, "read failed! sd %d\n", cp->sd);
			closecon(cp, 0);
		}
		/*
		 * note if EAGAIN, we just return, and let our caller
		 * decide to call us again when socket is readable
		 */
		return;
	}
	/*
	 * ok we really got something read. chage where we're
	 * pointing
	 */
	cp->bp += i;
	cp->bl -= i;
	fprintf(stdout, "%s %s %d\n", "Reading content from socket: ", cp->buf, (int) i);
	sprintf(msgData, "%s", cp->buf);
	/*
	 * now check to see if we should change state - i.e. we got
	 * a newline on the end of the buffer
	 */
	// if (cp->bl == 0 {
	fprintf(stdout, "%s\n", "Setting socket to writing");
	cp->state = STATE_WRITING;
	cp->bl = cp->bp - cp->buf; /* how much will we write */
	cp->bp = cp->buf;	   /* and we'll start from here */
	// }
}

/* Paul Lu */
/*
 * getPortNumber - given a valid file descriptor/socket, return the port number
 */
int getPortNumber( int socketNum )
{
	struct sockaddr_in addr;
	int rval;
	socklen_t addrLen;

	addrLen = (socklen_t)sizeof( addr );

	/* Use getsockname() to get the details about the socket */
	rval = getsockname( socketNum, (struct sockaddr*)&addr, &addrLen );
	if( rval != 0 )
		err(1, "getsockname() failed in getPortNumber()");

	/* Note cast and the use of ntohs() */
	return( (int) ntohs( addr.sin_port ) );
} /* getPortNumber */




void serverBoot(int argc,  char *argv[])
{
	int max = -1, omax;      /* the biggest value sd. for select */
	readable = NULL;
	writable = NULL; /* fd_sets for select */
	u_short port;
	u_long p;
	char *ep;
	int i;

	/*
	 * first, figure out what port we will listen on - it should
	 * be our first parameter.
	 * Paul Lu:  Changed to allow no specification of port number, which
	 *	allows the OS to pick any available port
	 */

/* Paul Lu */
	if (argc != 3){ //Change to 2 according to spec.
		usage();
		errno = 0;
	}	
    	p = strtoul(argv[1], &ep, 10);
    	if (*argv[1] == '\0' || *ep != '\0') {
		/* parameter wasn't a number, or was empty */
		fprintf(stderr, "%s - not a number\n", argv[1]);
		usage();
	}
    	if ((errno == ERANGE && p == ULONG_MAX) || (p > USHRT_MAX)) {
		/* It's a number, but it either can't fit in an unsigned
		 * long, or is too big for an unsigned short
		 */
		fprintf(stderr, "%s - value out of range\n", argv[1]);
		usage();
	}
	/* now safe to do this */
	port = p;

	// port = 0;			/* Default:  Let OS choose free port */

	/* now off to the races - let's set up our listening socket */
	memset(&sockname, 0, sizeof(sockname));
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(port);
	sockname.sin_addr.s_addr = htonl(INADDR_ANY);
	sd=socket(AF_INET,SOCK_STREAM,0);
	if ( sd == -1)
		err(1, "socket failed");

	if (bind(sd, (struct sockaddr *) &sockname, sizeof(sockname)) == -1)
		err(1, "bind failed");

	if (listen(sd,3) == -1)
		err(1, "listen failed");

/* Paul Lu */
        printf("Server 'localhost' up and listening for connections on port %d\n", getPortNumber( sd ) );

	/* 
	 * We're now bound, and listening for connections on "sd".
	 * Each call to "accept" will return us a descriptor talking to
	 * a connected client.
	 */

	/*
	 * finally - the main loop.  accept connections and deal with 'em
	 */
	/* initialize all our connection structures */
	for (i = 0; i < MAXCONN; i++)
		closecon(&connections[i], 1);

	for(;;) {
		int i;
		int maxfd = -1; /* the biggest value sd we are interested in.*/

		/*
		 * first we have to initialize the fd_sets to keep
		 * track of readable and writable sockets. we have
		 * to make sure we have fd_sets that are big enough
		 * to hold our largest valued socket descriptor.
		 * so first, we find the max value by iterating through
		 * all the connections, and then we allocate fd sets
		 * that are big enough, if they aren't already.
		 */
		omax = max;
		max = sd; /* the listen socket */

		for (i = 0; i < MAXCONN; i++) {
			if (connections[i].sd > max)
				max = connections[i].sd;
		}
		if (max > omax) {
			/* we need bigger fd_sets allocated */

			/* free the old ones - does nothing if they are NULL */
			free(readable);
			free(writable);

			/*
			 * this is how to allocate fd_sets for select
			 */
			readable = (fd_set *)calloc(howmany(max + 1, NFDBITS),
			    sizeof(fd_mask));
			if (readable == NULL)
				err(1, "out of memory");
			writable = (fd_set *)calloc(howmany(max + 1, NFDBITS),
			    sizeof(fd_mask));
			if (writable == NULL)
				err(1, "out of memory");
			omax = max;
			/*
			 * note that calloc always returns 0'ed memory,
			 * (unlike malloc) so these sets are all set to 0
			 * and ready to go
			 */
		} else {
			/*
			 * our allocated sets are big enough, just make
			 * sure they are cleared to 0. 
			 */
			memset(readable, 0, howmany(max+1, NFDBITS) *
			    sizeof(fd_mask));
			memset(writable, 0, howmany(max+1, NFDBITS) *
			    sizeof(fd_mask));
		}

		/*
		 * Now, we decide which sockets we are interested
		 * in reading and writing, by setting the corresponding
		 * bit in the readable and writable fd_sets.
		 */

		/*
		 * we are always interesting in reading from the
		 * listening socket. so put it in the read set.
		 */

		FD_SET(sd, readable);
		if (maxfd < sd)
			maxfd = sd;

		/*
		 * now go through the list of connections, and if we
		 * are interested in reading from, or writing to, the
		 * connection's socket, put it in the readable, or
		 * writable fd_set - in preparation to call select
		 * to tell us which ones we can read and write to.
		 */
		for (i = 0; i<MAXCONN; i++) {
			if (connections[i].state == STATE_READING) {
				FD_SET(connections[i].sd, readable);
				if (maxfd < connections[i].sd)
					maxfd = connections[i].sd;
			}
			if (connections[i].state == STATE_WRITING) {
				FD_SET(connections[i].sd, writable);
				if (maxfd < connections[i].sd)
					maxfd = connections[i].sd;
			}
		}

		/*
		 * finally, we can call select. we have filled in "readable"
		 * and "writable" with everything we are interested in, and
		 * when select returns, it will indicate in each fd_set
		 * which sockets are readable and writable
		 */
		i = select(maxfd + 1, readable, writable, NULL,NULL);
		if (i == -1  && errno != EINTR)
			err(1, "select failed");
		if (i > 0) {

			/* something is readable or writable... */

			/*
			 * First things first.  check the listen socket.
			 * If it was readable - we have a new connection
			 * to accept.
			 */

			if (FD_ISSET(sd, readable)) {
				struct con *cp;
				int newsd;
				socklen_t slen;
				struct sockaddr_in sa;

				slen = sizeof(sa);
				newsd = accept(sd, (struct sockaddr *)&sa,
				    &slen);
				if (newsd == -1)
					err(1, "accept failed");

				fprintf(stdout, "%s\n", "Looks like there is an connection");
				cp = get_free_conn();
				if (cp == NULL) {
					fprintf(stdout, "%s\n", "failed getting connections");
					/*
					 * we have no connection structures
					 * so we close connection to our
					 * client to not leave him hanging
					 * because we are too busy to
					 * service his request
					 */
					close(newsd);
				} else {
					// fprintf(stdout, "%s\n", "success getting connections");
					clerkNannyPrint("Success getting connections", DEBUG);
					/*
					 * ok, if this worked, we now have a
					 * new connection. set him up to be
					 * READING so we do something with him
					 */
					cp->state = STATE_READING;
					cp->sd = newsd;
					cp->slen = slen;
					memcpy(&cp->sa, &sa, sizeof(sa));
				}
			}
			setReadable();
			serverRead();
			/*
			 * now, iterate through all of our connections,
			 * check to see if they are readble or writable,
			 * and if so, do a read or write accordingly 
			 */
			// for (i = 0; i<MAXCONN; i++) {
			// 	if ((connections[i].state == STATE_READING) &&
			// 	    FD_ISSET(connections[i].sd, readable))
			// 		handleread(&connections[i]);
			// 	if ((connections[i].state == STATE_WRITING) && FD_ISSET(connections[i].sd, writable)){
			// 		curr = head;
			// 		while(curr){
			// 			sprintf(msgData, "%s %s %s", "newList", curr->name, curr->secs);
			// 			clerkNannyPrint(msgData, DEBUG);
			// 			handlewrite(&connections[i], msgData);
			// 			curr = curr->next;
			// 		}
			// 		sprintf(msgData, "%s %s %s", "endList", "empty", "empty");
			// 		handlewrite(&connections[i], msgData);

					
			// 	}
			// }
		}
	}
}

void setReadable(void){
	int i;
	for (i = 0; i<MAXCONN; i++) {
		if ((connections[i].state == STATE_WRITING) && FD_ISSET(connections[i].sd, writable)){
			connections[i].state = STATE_READING;
		}
	}
}

void setWritable(void){
	int i;
	for (i = 0; i<MAXCONN; i++) {
		if ((connections[i].state == STATE_READING) && FD_ISSET(connections[i].sd, readable)){
			connections[i].state = STATE_WRITING;
		}
	}
}

void serverRead(void){
	int i;
	char* header;
	// char* data;
	// char* extraData;

	for (i = 0; i<MAXCONN; i++) {
		if ((connections[i].state == STATE_READING) && FD_ISSET(connections[i].sd, readable)){
			sprintf(msgData, "%s", "");
			handleread(&connections[i]);
			clerkNannyPrint(msgData, DEBUG);
			header = strtok (msgData," "); //First line is Header
			clerkNannyPrint(header, DEBUG);
			if(!strcmp(header, "Init")){
				clerkNannyPrint("Foudn init sending config", DEBUG);
				// if ((connections[i].state == STATE_WRITING) && FD_ISSET(connections[i].sd, writable)){
					sprintf(msgData, "%s %s %s", "newList", "empty", "empty");
					handlewrite(&connections[i], msgData);
					curr = head;
					while(curr){
						sprintf(msgData, "%s %s %s", "listItem", curr->name, curr->secs);
						clerkNannyPrint(msgData, DEBUG);
						handlewrite(&connections[i], msgData);
						curr = curr->next;
					}
					sprintf(msgData, "%s %s %s", "endList", "empty", "empty");
					handlewrite(&connections[i], msgData);		
				// }
			}
			if(!strcmp(header, "Kill")){
				clerkNannyPrint("Foudn kill Documenting", DEBUG);
			}

			if(!strcmp(header, "Msg")){
				clerkNannyPrint("Msg HERE IS A MESSAGE THAT WAS SENT PRINT THE CONTENTS", BOTH);
			}
		}
	}
}

void serverWriteNewConfigToClients(void){
	// setReadable();
	// serverRead();
	// setWritable();
	int i;
	clerkNannyPrint("GOT HERE ", DEBUG);
	for (i = 0; i<MAXCONN; i++) {
		if ((connections[i].state == STATE_WRITING) && FD_ISSET(connections[i].sd, writable)){ //
			clerkNannyPrint("GOT HERE 3", DEBUG);
			sprintf(msgData, "%s %s %s", "newList", "empty", "empty");
			handlewrite(&connections[i], msgData);
			curr = head;
			while(curr){
				sprintf(msgData, "%s %s %s", "listItem", curr->name, curr->secs);
				handlewrite(&connections[i], msgData);
				curr = curr->next;
			}
			sprintf(msgData, "%s %s %s", "endList", "empty", "empty");
			handlewrite(&connections[i], msgData);		
		}
		if ((connections[i].state == STATE_WRITING)){
			clerkNannyPrint("State is equal to Write", DEBUG);
		}
		if (FD_ISSET(connections[i].sd, writable)){
			clerkNannyPrint("FD is set to writable", DEBUG);
		}
		clerkNannyPrint("Nothing good", DEBUG);
	}
}

void serverTeardown(void){
	int i;
	fprintf(stdout, "%s\n", "Tearing Down Server");
	for (i = 0; i<MAXCONN; i++) {
		sprintf(msgData, "%s %s %s", "Die", "Die", "Die");
		handlewrite(&connections[i], msgData);
		close(connections[i].sd);
	}
	close(sd);
}
