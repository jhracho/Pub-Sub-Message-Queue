// Yoyoyo this is our super cool chat application

// Libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mq/client.h"
#include "mq/queue.h"
#include "mq/thread.h"
#include "mq/request.h"
#include "mq/socket.h"

// Globals
char *PROGRAM_NAME = NULL;

// Infinite loop while the input is not /quit
// fgets to grab input, streq to check command
// fputs to display the output back to the screen
void usage(int status){
	fprintf(stderr, "Usage: %s {Host} {Port}\n", PROGRAM_NAME);
	exit(status);
}

MessageQueue *startup(char *name, char *host, char *port){
	printf("DEBUG: Name: %s Host: %s Port: %s\n", name, host, port);
	
	MessageQueue *mq = mq_create(name, host, port);
	if (mq)
		return mq;
	else{
		fprintf(stderr, "FATAL ERROR: MessageQueue %s creation failed.", name);
		exit(1);
	}
}

int main(int argc, char *argv[]){
	PROGRAM_NAME = argv[0];
	if (argc == 1)
		usage(1);
	
	char *host = argv[1];
	char *port = argv[2];
	char name[BUFSIZ];
	printf("Enter your netid: ");
	fgets(name, BUFSIZ, stdin);
	
	printf("DEBUG: Enterring startup\n");
	MessageQueue *mq = startup(name, host, port);
	printf("DEBUG: Returned from startup\n");
	char buffer[BUFSIZ];
	while(fgets(buffer, stdin,)){
		if streq(buffer,)

	}
	return 0;
}
