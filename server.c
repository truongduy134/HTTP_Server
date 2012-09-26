#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <strings.h>
#include "queue.h"
#include "helperlib.h"
#include "m_http.h"

/* Constants */
static const int DEFAULT_NUM_WORKER = 10;
static const int MAX_WORKER = 5000;
static const int MAX_WAIT = 5;
static const int MAX_LEN_REQUEST = 800;

/* For file access security checking (e.g. for NUM_SLAH = 2: ./abc.html and ./Repository/abc.html are allowed. But access to ./A/B/abc.html is denied) */ 
static const int MAX_NUM_SLASH = 2;
static char SUPPORT_HTTP[] = "HTTP/1.1";	/* The supported version of HTTP on this server */
		
/* Global variables */
struct queue_int_s *clientQueue;

/* Mutexes and conditions */
pthread_mutex_t queue_mutex;
pthread_cond_t queue_has_client;

/* Function prototypes */
void create_worker(pthread_t * workerArr, int * threadId_arr, int numWorker);
void * handle_client_request(void * workerId);
char * read_request(int clientSocket);

int main(int argc, char *argv[])
{
	int bossSocket, numWorker, clientSocket, notOverload, flagSignal;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientLen;
	in_port_t serverPort;
	pthread_t workerArr[MAX_WORKER];
	int threadId_arr[MAX_WORKER];

	if(argc < 2)
		print_user_error("In-line arguments", "You must specify the port number");
	serverPort = atoi(argv[1]);

	/* Create a socket */
	bossSocket  = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(bossSocket < 0)
		print_system_error("socket() fails");
	
	/* Construct local address structure */
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(serverPort);
	/* Bind boss socket to local address */
	if(bind(bossSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr) ) < 0)
		print_system_error("bind() fails");

	/* Initialize client queues, mutexes and conditions */
	clientQueue = create_queue();
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_cond_init(&queue_has_client, NULL);

	/* Create the pool of workers */
	if(argc < 3)
		numWorker = DEFAULT_NUM_WORKER;
	else
	{
		numWorker = atoi(argv[2]);
		if(numWorker > MAX_WORKER)
			numWorker = MAX_WORKER;
	}

	printf("Hi\n");
	create_worker(workerArr, threadId_arr, numWorker);
	printf("Hi there!\n");
	
	/* Listening */ 
	if(listen(bossSocket, MAX_WAIT) < 0)
		print_system_error("listen() fails");

	/* Handling request */
	while(1)
	{
		/* Accpet connection to a client */
		clientLen = sizeof(struct sockaddr_in);
		clientSocket = accept(bossSocket, (struct sockaddr *) &clientAddr, &clientLen);
		if(clientSocket < 0)
			print_system_error("accept() fails");

		/* Put that client to the queue, waiting for worker threads to handle it */
		pthread_mutex_lock(&queue_mutex);
		{
			flagSignal = 0;
			if(is_empty_queue(clientQueue))
				/* There must be workers waiting, so send signal */
				flagSignal = 1;
			notOverload = enqueue(clientQueue, clientSocket);
			if(flagSignal)
				pthread_cond_broadcast(&queue_has_client);
		}	
		pthread_mutex_unlock(&queue_mutex);

		if(!notOverload)
		{
			/* Kindly refuse the request of this client */
		}	
	}
}

void create_worker(pthread_t * workerArr, int * threadId_arr, int numWorker)
{
	int threadIndex;
	int returnVal;
	char message[100];

	for(threadIndex = 0; threadIndex < numWorker; threadIndex++)
	{
		threadId_arr[threadIndex] = threadIndex;

		returnVal = pthread_create(&workerArr[threadIndex], NULL, handle_client_request, &threadId_arr[threadIndex]);
		printf("Hi %d\n", threadIndex);
		if(returnVal)
		{
			sprintf(message, "pthread_create() fails with error code %d", returnVal); 
			print_system_error(message);
		}		
	}

	printf("I am finished\n");	
}

void * handle_client_request(void * workerId)
{
	int id = *((int *) workerId); 

	char * request = NULL;
	int clientSocket, parseSuccess;
	struct http_request_s * request_obj;
	struct http_response_s * response_obj;
	char *response = NULL;
	printf("Hullo in %d\n", id);
	while(1)
	{
		clientSocket = -1;
		printf("Go to loop in %d\n", id);
		pthread_mutex_lock(&queue_mutex);
		{
			if(is_empty_queue(clientQueue))
			{
				pthread_cond_wait(&queue_has_client, &queue_mutex);
			}
			else 
				/* Take 1 client out of the queue */
				dequeue(clientQueue, &clientSocket);
		}
		pthread_mutex_unlock(&queue_mutex);
		printf("Worker with id = %d handles socketId = %d\n", id, clientSocket);
		if(clientSocket >= 0)
		{
			/* Initalize for new request handling */
			request_obj = create_request_struct();
			response_obj = create_response_struct();
			response = NULL;
			
			/* Handle the request of the client */
			request = read_request(clientSocket);
/*			ssize_t numByte = recv(clientSocket, m_request, 1000, 0);
			m_request[numByte] = '\0'; */
			printf("OK reading request\n"); 
			/* Parse request */
			parseSuccess = parse_http_request(request_obj, request, response_obj);

			response_obj->version = copy_str_dynamic(SUPPORT_HTTP);

			if(parseSuccess)
			{
				/* Check HTTP version */
				if(strcasecmp(request_obj->version, SUPPORT_HTTP) != 0)
				{
					set_status_code_error(response_obj, 505, "505 HTTP Version Not Supported", "This server supports only HTTP/1.1");
				}
				else
				{
					/* Check file access security */
					if(count_occurence(request_obj->path, '/') > MAX_NUM_SLASH)
						set_status_code_error(response_obj, 401, "401 Unauthorized", "You are not authorized to access the requested file on this server");
					else
					{
						exec_http_request(request_obj, response_obj);
					}
				}
			} 
			/* Execute command and return output 
			sprintf(response, "Server worker thread with id = %d handles request: %s", id, request); */
			
			response = get_response_text(response_obj);
			send(clientSocket, response, strlen(response), 0);

			//recv(clientSocket, response, 100, 0);
			/* Close socket, free memory */
			close(clientSocket);
			free(request);
			free(response);
			delete_request(&request_obj);
			delete_response(&response_obj); 
		}
	}
}

/* Here, "\r\n" signifies the end of a request */
char * read_request(int clientSocket)
{
	char buffer[MAX_LEN_REQUEST];
	int totalByte = 0;
	ssize_t numByteRecv, curBufferLen;
	char *curBuffer;

	curBuffer = buffer;
	curBufferLen = MAX_LEN_REQUEST;
	do
	{
		numByteRecv = recv(clientSocket, curBuffer, curBufferLen - 1, 0);
		printf("numByte = %d\n", numByteRecv);		
		if(numByteRecv < 0)
			print_system_error("recv() fails");
		else
		{ 
			/* Check if we reach the end of HTTP request */
			totalByte += (int) numByteRecv;	
			if(totalByte >= 2 && buffer[totalByte - 1] == '\n' && buffer[totalByte - 2] == '\r')
			{
				buffer[totalByte] = '\0';
				break;
			}
			else
			{
				curBuffer = curBuffer + numByteRecv;
				curBufferLen -= (int) numByteRecv;
			}
		}
	} while(numByteRecv);

	buffer[totalByte] = '\0';
	
	return copy_str_dynamic(buffer);
}
