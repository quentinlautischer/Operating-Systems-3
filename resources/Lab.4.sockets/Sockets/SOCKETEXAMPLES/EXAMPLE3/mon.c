#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define MY_PORT		2222	/* Master socket port number */
#define MAXCLIENTS	64	/* Maximum number of clients */
#define	HOSTNAMELENGTH	16
#define	BUFSIZE		256

struct	item	{		/* An item to be produced */

	char	hostname [HOSTNAMELENGTH];	/* The producer */
	int	stuff;				/* The produced stuff */
};

struct	item	buffer [BUFSIZE];
int	in = 0, out = 0;

isstuff () {

	return (in != out);
}

isroom () {
	return ((in+1) % BUFSIZE != out);
}

struct	item	get_item () {

	int i;

	out = ((i = out) + 1) % BUFSIZE;
	return (buffer [i]);
}

void put_item (struct item *i) {

	buffer [in] = *i;
	in = (in + 1) % BUFSIZE;
}

main() 
{
	int	nc, scn, spn, sock, fromlength, maxdesc,
		clients [MAXCLIENTS], nclients;
	char			ctype;
	struct	item		datum;
	struct	sockaddr_in	master, from;
	fd_set	producers, consumers, ready_to_read_from, ready_to_write_to;

	FD_ZERO (&producers);	    /* Initialize descriptor sets */
	FD_ZERO (&consumers);
	maxdesc = getdtablesize (); /* The maximum number of descriptors */
	nclients = 0;		    /* No clients yet */

	/* Create master socket to await connections */
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (MY_PORT);

	/* Bind the port to the master socket */
	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}

	listen (sock, 5);

	scn = spn = 0;		/* Just in case */
	while (1) {
		/* The main loop */

		if (isroom ())
		    /* Willing to get some stuff */
		    bcopy (&producers, &ready_to_read_from, sizeof (fd_set));
		else
		    FD_ZERO (&ready_to_read_from);

		if (isstuff ())
		    /* Willing to send some stuff out */
		    bcopy (&consumers, &ready_to_write_to, sizeof (fd_set));
		else
		    FD_ZERO (&ready_to_write_to);

		FD_SET (sock, &ready_to_read_from);

		nc = select (maxdesc, &ready_to_read_from, &ready_to_write_to,
			NULL, NULL);
		if (!nc) continue;	/* Just in case */

		if (FD_ISSET (sock, &ready_to_read_from)) {
			/* A new client */
			fromlength = sizeof (from);
			clients [nclients] =
				accept (sock, (struct sockaddr*) & from,
					& fromlength);
			if (clients [nclients] < 0) {
				perror ("Server: accept failed");
				exit (1);
			}
			if (recv (clients [nclients], &ctype, 1, 0) < 0) {
				perror ("Server: signature read failed");
				exit (1);
			}
			if (ctype == 'P')
			    /* This is a producer */
			    FD_SET (clients [spn = nclients], &producers);
			else
			    /* This is a receiver */
			    FD_SET (clients [scn = nclients], &consumers);
			nclients++;
			continue;
		}

		for (nc = 0; nc++ < nclients && isstuff ();
		    scn = (scn+1) % nclients) {
			/* Process consumers */
			if (FD_ISSET (clients [scn], &ready_to_write_to)) {
				datum = get_item ();
				if (send (clients [scn], &datum,
					sizeof (datum), 0) < 0) {
					perror ("Server: send failed");
					exit (1);
				}
			}
		}

		for (nc = 0; nc++ < nclients && isroom ();
		    spn = (spn+1) % nclients) {
			/* Process producers */
			if (FD_ISSET (clients [spn], &ready_to_read_from)) {
				if (recv (clients [spn], &datum,
					sizeof (datum), 0) < 0) {
					perror ("Server: recv failed");
					exit (1);
				}
				put_item (&datum);
			}
		}
	}
}
