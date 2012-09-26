#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "helperlib.h"
#include <errno.h>
#define MAX_NUM_THREAD 10000
#define DEFAULT_NUM_THREAD 10
#define NUM_REQUEST 30
#define MAX_LEN_RESPONSE 10000
#define NUM_FILE 7
#define MAX_LEN_REQUEST 1000

struct sockaddr_in serverAddr;
pthread_mutex_t mutex_print;
int numFailRecv = 0;
int numFailConn = 0;

char *file_name[NUM_FILE] = {"dummy/complete_hello_fake.html", "console.html", "debugger.html", "fadeIn.html", "hello.html", "complete_hello.html", "complete_hello_fake.html"};
char DEFAULT_PATH[100] = "/Repository/";

void *send_request(void *);
char *generate_request();
	
int main(int argc, char *argv[])
{
	int numThread, index, clientSocket, rtnValue;
	char message[100], *serverIP;
	pthread_t threadArr[MAX_NUM_THREAD];
	in_port_t serverPort;
	void * status;
	int pthread_id[MAX_NUM_THREAD];
	unsigned int rand_seed;

	if(argc < 3)
		print_user_error("In-line argument", "You must specify the server IP address, and the port number");
	serverIP = argv[1];
	serverPort = atoi(argv[2]);

	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	rtnValue = inet_pton(AF_INET, serverIP, &serverAddr.sin_addr.s_addr);
	if(rtnValue == 0)
		print_user_error("inet_pton() fails", "Invalid address string");
	else
		if(rtnValue < 0)
			print_system_error("inet_pton() fails");
	serverAddr.sin_port = htons(serverPort);
	
	/* Initialize seed for random number generation */
	rand_seed = (unsigned int) time(NULL);
	srand(rand_seed);
			
	/* Create threads */
	pthread_mutex_init(&mutex_print, NULL);

	if(argc >= 4)
	{
		numThread = atoi(argv[3]);
		if(numThread > MAX_NUM_THREAD)
			numThread = MAX_NUM_THREAD;
	}
	else
		numThread = DEFAULT_NUM_THREAD;
	for(index = 0; index < numThread; index++)
	{
		pthread_id[index] = index;
		rtnValue = pthread_create(&threadArr[index], NULL, send_request, &pthread_id[index]);

		if(rtnValue)
		{
			sprintf(message, "pthread_create() fails with error code %d", rtnValue);
			print_system_error(message);
		}
	}
	
	/* Join */
	for(index = 0; index < numThread; index++)
	{
		rtnValue = pthread_join(threadArr[index], &status);
		if(rtnValue)
		{
		} 
	}
	pthread_mutex_destroy(&mutex_print);
	printf("Total failed request by recv() = %d\n", numFailRecv);
	printf("Total failed request by connect() = %d\n", numFailConn);
	pthread_exit(NULL);
}

void * send_request(void * threadId)
{
	int id = *((int *) threadId);
	int clientSocket, lenBuffer;
	char *request;
	int curNumRequest, lenRequest;
	ssize_t numByte, numByteRecv;
	char buffer[MAX_LEN_RESPONSE], *relative_buffer;
	char end[100] = "END";
	int flagFailRecv;

	/* Create a socket */
	for(curNumRequest = 0; curNumRequest < NUM_REQUEST; curNumRequest++)
	{
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(clientSocket < 0)
			print_system_error("socket() fails");

		/* Establish connection to the server */
		if(connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	//		print_system_error("connect() fails");
		{
			//perror("connect() fails");
			//pthread_exit(NULL);
			pthread_mutex_lock(&mutex_print);
				numFailConn++;
			pthread_mutex_unlock(&mutex_print);
			continue;
		}

		/* Send request */
		request = generate_request();
		lenRequest = strlen(request);

		numByte = send(clientSocket, request, lenRequest, 0);
		if(numByte < 0)
			print_system_error("send() fails");
		else
			if(numByte != lenRequest)
				print_user_error("send()", "sent unexpected number of bytes");		
		shutdown(clientSocket, SHUT_WR);		

		/* Receive response */
		relative_buffer = buffer;
		lenBuffer = MAX_LEN_RESPONSE;
		flagFailRecv = 0;
		do
		{
			numByteRecv = recv(clientSocket, relative_buffer, lenBuffer - 1, 0);
			if(numByteRecv < 0)
			{
				flagFailRecv = 1;
				break;
				/*print_system_error("recv() fails");*/
			}
			else
				if(numByteRecv > 0)
				{
					relative_buffer = relative_buffer + numByteRecv;
					lenBuffer -= numByteRecv;
				}
				else
					relative_buffer[0] = '\0';
		} while(numByteRecv > 0);
		
		
	/*	send(clientSocket, end, 3, 0); */
		close(clientSocket);
		
		
		pthread_mutex_lock(&mutex_print);
		{
			if(flagFailRecv)
				numFailRecv++;
			else
			{
				printf("Client thread with id = %d sends request %d\n", id, curNumRequest + 1);
				printf("Response from the server:\n");						printf("%s", buffer);
			}
		}
		pthread_mutex_unlock(&mutex_print);
		free(request);
	}			

	pthread_exit(NULL);
}

char * generate_request()
{
	char buffer[MAX_LEN_REQUEST];
	int rand_index = random() % NUM_FILE;

	sprintf(buffer, "GET %s%s HTTP/1.1", DEFAULT_PATH, file_name[rand_index]);

	return copy_str_dynamic(buffer);
}
