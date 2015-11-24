#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

/* ------------------------------------------------ */
/* This is a sample producer for the monitor server */
/* ------------------------------------------------ */

#define MY_PORT		2222
#define HOSTNAMELENGTH	16	/* Maximum length of a host name */

#define	SERVNAME	"sheerness"

struct	item	{		/* An item to be produced */

	char	hostname [HOSTNAMELENGTH];	/* The producer */
	int	stuff;				/* The produced stuff */
};

main() 
{
	int	s, ic;
	struct	item		datum;
	struct	sockaddr_in	server;
	struct	hostent		*host;

	host = gethostbyname (SERVNAME);

	if (host == NULL) {
		perror ("Producer: cannot get host description");
		exit (1);
	}

	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		perror ("Producer: cannot open socket");
		exit (1);
	}

	bzero (&server, sizeof (server));
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = host->h_addrtype;
	server.sin_port = htons (MY_PORT);

	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Producer: cannot connect to server");
		exit (1);
	}

	send (s, "P", 1, 0);	/* Send your signature */

	/* Get the name of your host */

	bzero (datum.hostname, HOSTNAMELENGTH);
	gethostname (datum.hostname, HOSTNAMELENGTH);

	ic = 0;			/* Item number */
	while (1) {
		sleep (5);
		datum.stuff = htonl (ic++);
		send (s, &datum, sizeof (datum), 0);
	}
}
