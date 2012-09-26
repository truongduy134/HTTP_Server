HTTP_Server
===========

Implement a multi-threaded HTTP web server based on boss-worker thread pattern in C, using pthread library.

Note: The project is under development. Currently, server can handle simple GET request and return RESPONSE without following full HTTP specification.

* m_http.h contains the API and structures defined for HTTP requests and response.

* m_queue.h contains the API for the queue data structure needed in the server.

* m_helperlib.h contains prototypes of helper functions used in the project.

* server.c contains the main code of the server. The main function takes the port number (required)  and the number of worker threads (optional) as command line arguments.

* client.c is a program simulating multiple clients sending requests to the server. The main function takes in the IP address of the server, the port (compulsory) and the number of client threads (optional).

* makefile is also provided for easy compilation.
