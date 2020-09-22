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
Mutex  lock;
//Cond   cond;


// Infinite loop while the input is not /quit
// fgets to grab input, streq to check command
// fputs to display the output back to the screen
void usage(int status){
	fprintf(stderr, "Usage: %s {Host} {Port}\n", PROGRAM_NAME);
	exit(status);
}


// Move to usage
void menu(){
	printf("\n");
	printf("/sub: Subscribe to a topic\n");
	printf("/unsub: Unsubscribe from a topic\n");
	printf("/pub: Go to topic\n");
	printf("/quit: Exit program\n\n");
	printf("Select an option: ");
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
		fflush(stdin);
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
		
		if (taggedMsg){
			sscanf(taggedMsg, "%s %[^t\n]", sender, message);
			if (!streq(sender, mq->name))
				printf("%s: %s", sender, message);
			free(taggedMsg);
		}
	}
	return NULL;
}

void messageBoard(MessageQueue *mq){
	mq_subscribe(mq, TOPIC);
	mq_start(mq);
	thread_create(&incoming, NULL, incomingFunc, mq);
	thread_create(&outgoing, NULL, outgoingFunc, mq);
	thread_join(incoming, NULL);
	thread_join(outgoing, NULL);
}


int main(int argc, char *argv[]){
	PROGRAM_NAME = argv[0];
	if (argc == 1)
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
	
	/*
	char name[BUFSIZ];
	printf("Enter your netid: ");
	fgets(name, BUFSIZ, stdin);
	*/

	char *name = getenv("USER");
	MessageQueue *mq = mq_create(name, host, port);
	mutex_init(&lock, NULL);
	//cond_init(&cond, NULL);
	if (!mq){
		fprintf(stderr, "FATAL ERROR: MessageQueue %s creation failed", name);
		exit(1);
	}
	messageBoard(mq);

	return 0;
}
