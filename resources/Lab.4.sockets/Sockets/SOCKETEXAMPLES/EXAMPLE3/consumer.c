#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

/* ------------------------------------------------ */
/* This is a sample consumer for the monitor server */
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
	int	s;
	struct	item		datum;
	struct	sockaddr_in	server;
	struct	hostent		*host;

	/* Insert here the name of the sun on which the server runs */
	host = gethostbyname (SERVNAME);

	if (host == NULL) {
		perror ("Consumer: cannot get host description");
		exit (1);
	}

	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		perror ("Consumer: cannot open socket");
		exit (1);
	}

	bzero (&server, sizeof (server));
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = host->h_addrtype;
	server.sin_port = htons (MY_PORT);

	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Consumer: cannot connect to server");
		exit (1);
	}

        send (s, "C", 1, 0);		/* Send your signature */

	while (1) {
		sleep (5);
		recv (s, &datum, sizeof (datum), 0);
		printf ("Consumed item: %s %d\n", datum.hostname,
			ntohl (datum.stuff));
	}
}
