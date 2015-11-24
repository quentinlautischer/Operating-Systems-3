#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

/* --------------------------------------------------------------------- */
/* This is a sample client program for the number server. We assume that */
/* it runs on the same machine as the server.				 */
/* --------------------------------------------------------------------- */

main() 
{
	int	s, number;

	struct	sockaddr_un	server;

	while (1) {

		s = socket (AF_UNIX, SOCK_STREAM, 0);

		if (s < 0) {
			perror ("Client: cannot open socket");
			exit (1);
		}

		strcpy (server.sun_path, "the_number_server");
		server.sun_family = AF_UNIX;

		if (connect (s, (struct sockaddr*) & server,
			sizeof (server.sun_family) + strlen (server.sun_path)))
		{

			perror ("Client; cannot connect to server");
			exit (1);
		}

		read (s, &number, sizeof (number));
		close (s);
		fprintf (stderr, "Process %d gets number %d\n", getpid (),
			number);
		sleep (5);
	}
}
