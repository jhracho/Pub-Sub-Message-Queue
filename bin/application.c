// Yoyoyo this is our super cool chat application

// Libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mq/client.h"
#include "mq/queue.h"
#include "mq/thread.h"
#include "mq/request.h"
#include "mq/socket.h"
#include "mq/string.h"

// Globals
const char *TOPIC = "PBAZ";
char *PROGRAM_NAME = NULL;
FILE *fs;
Thread incoming;
Thread outgoing;

void usage(int status){
	fprintf(stderr, "Usage: %s {Host} {Port 9000-9999}\n", PROGRAM_NAME);
	exit(status);
}

void *outgoingFunc(void *arg){
	MessageQueue *mq = (MessageQueue *)arg;
	char message[BUFSIZ];
	printf("%s: ", mq->name);
	fgets(message, BUFSIZ, stdin);
	char taggedMsg[BUFSIZ];
	sprintf(taggedMsg, "%s %s\n", mq->name, message);    // Message: jhracho Hello
	
	while(!streq(message, "/quit\n")){
		mq_publish(mq, TOPIC, taggedMsg);
		
		printf("%s: ", mq->name);
		fgets(message, BUFSIZ, stdin);
		sprintf(taggedMsg, "%s %s", mq->name, message);
	}
	
	mq_stop(mq);
	return NULL;
}

void *incomingFunc(void *arg){
	MessageQueue *mq = (MessageQueue *)arg;
	char sender[BUFSIZ];
	char message[BUFSIZ];

	while (!mq_shutdown(mq)){
		char *taggedMsg = mq_retrieve(mq);
		fflush(stdout);
		if (taggedMsg){
			sscanf(taggedMsg, "%s %[^t\n]", sender, message);
			if (!streq(sender, mq->name)){
				printf("%s: %s\n", sender, message);
			}
			free(taggedMsg);
		}
	}
	return NULL;
}

void messageBoard(MessageQueue *mq){
	printf("*****Welcome to the Peter Bui Autonomous Zone (PBAZ)*****\n");
	printf("************Type /quit to leave at any time.*************\n");
	mq_subscribe(mq, TOPIC);
	mq_start(mq);
	thread_create(&incoming, NULL, incomingFunc, mq);
	thread_create(&outgoing, NULL, outgoingFunc, mq);
	thread_join(incoming, NULL);
	thread_join(outgoing, NULL);
}


int main(int argc, char *argv[]){
	PROGRAM_NAME = argv[0];
	if (argc < 3)
		usage(1);
	
	char *host = argv[1];
	char port[BUFSIZ];
	strcpy(port, argv[2]);

	int finalPort = atoi(port);
	while (finalPort < 9000 || finalPort > 9999){
		printf("Enter a port between 9000-9999: ");
		fgets(port, BUFSIZ, stdin);
		finalPort = atoi(port);
	}
	
	char *name = getenv("USER");
	MessageQueue *mq = mq_create(name, host, port);
	
	if (!mq){
		fprintf(stderr, "FATAL ERROR: MessageQueue %s creation failed", name);
		exit(1);
	}
	messageBoard(mq);

	return 0;
}
