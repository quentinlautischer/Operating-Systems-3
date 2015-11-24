/* IPC Messaging Example
 * Author: Pooria Joulani
 *
 * Example adopted from:
 * Stevens, Rago, "Advance Programming in the UNIX Environment, Second Edition",2005, (Figure 15.5)
 * Shows a header/payload approach to message passing between processes using pipes.
 */
#include "apue.h"
#include "memwatch.h"

#define TEXT_MESSAGE 1

/*The type of the message bein sent*/
enum message_type {textMsg, dataMsg};

/* The heeader, including message size and type*/
struct my_header
{
	int size;
	enum message_type type;
};

/* The message, including a header and a body (payload)*/
struct my_message
{
	struct my_header header;
	char* body;
};

/* Reads a message from the file descriptor fd*/
struct my_message* read_message(int fd)
{
	int n;
	struct my_message* msg;
	
	msg = malloc(sizeof(struct my_message));
	/*Read the header*/
	n=read(fd,&msg->header,sizeof(msg->header));
	if(n!=sizeof(msg->header))
		err_sys("bad message format");

	/*Allocate memory for and read the body*/
	msg->body = malloc((msg->header.size+1)*sizeof(char));
	memset(msg->body,0,msg->header.size+1);
	n=read(fd,msg->body,msg->header.size);
	if(n!=msg->header.size)
		err_sys("incomplete message");

	return msg;
};

/* Writes the message in msg to file descriptor fd*/
int write_message(int fd,struct my_message* msg)
{
	/* Write header*/
	write(fd,&msg->header,sizeof(msg->header));
	/*Write payload*/
	write(fd,msg->body,msg->header.size);
};

/*Creates a text message containing the text given in the argument*/
struct my_message* init_message(char* message)
{
	struct my_message* msg;
	int size;
	enum message_type type=textMsg;/* Creating a text message*/


	msg=malloc(sizeof(struct my_message));

	size = strlen(message);

	msg->body = malloc(sizeof(char)*(size+1));

	msg->header.size = size;
	msg->header.type = type;
	strcpy(msg->body,message);

	return msg;
}

int
main(void)
{
	int		n;
	int		fd[2];
	pid_t	pid;
	char	line[MAXLINE];

	struct my_message* msg;

	if (pipe(fd) < 0)
		err_sys("pipe error");
	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {		/* parent */
		close(fd[0]);

		msg = init_message("This is a simple messaging test!\n");
		write_message(fd[1],msg);

	} else {				/* child */
		close(fd[1]);
		msg = read_message(fd[0]);
		n = msg->header.size;
		write(STDOUT_FILENO, msg->body, n);
	}
	free(msg->body);
	free(msg);
	exit(0);
}
