#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include "helperlib.h"
/* Declare an array containing HTTP request / response header fields */
#define NUM_REQUEST_HEADER_TYPE 19
#define NUM_RESPONSE_HEADER_TYPE 9
#define NUM_METHOD_TYPE 8
#define MAX_LENGTH_RESPONSE 3000

static const char * REQUEST_HEADER_LIST[] = {"ACCEPT", "ACCEPT-CHARSET", "ACCEPT-ENCODING",
		"ACCEPT-LANGUAGE", "AUTHORIZATION", "EXPECT", "FROM", "HOST",
		"IF-MATCH", "IF-MODIFIED-SINCE", "IF-NONE-MATCH", "IF-RANGE",
		"IF-UNMODIFIED-SINCE", "MAX-FORWARDS", "RANGE", "REFERER",
		"PROXY-AUTHORIZATION", "TE", "USER-AGENT"};
static char * METHOD_LIST[] = {"GET", "POST", "OPTIONS", "HEAD", "PUT", "DELETE", "TRACE", "CONNECT"};
static const char * RESPONSE_HEADER_LIST[] = {"ACCEPT-RANGES", "AGE", "ETAG", "LOCATION", "PROXY-AUTHENTICATE", "RETRY-AFTER", "SERVER", "VARY", "WWW-AUTHENTICATE"};

/* Define a structure of HTTP request and response */
struct http_request_s
{
	char * method;
	char * path;
	char * version;		/* e.g. version = "HTTP/1.1", etc. */
	char * request_header[NUM_REQUEST_HEADER_TYPE];
};

struct http_response_s
{
	char * version;
	int status_code;
	char * reason_phrase;
	char * response_header[NUM_RESPONSE_HEADER_TYPE];
	char * message_body;	
};

/* Useful functions when dealing with HTTP requests and responses */
struct http_request_s * create_request_struct();
struct http_response_s * create_response_struct();

int parse_http_request(struct http_request_s * request_obj, char *request, struct http_response_s * response_obj);

int exec_http_request(struct http_request_s * request_obj, struct http_response_s * response_obj);

void delete_request(struct http_request_s ** request_obj);
void delete_response(struct http_response_s ** response_obj);

void set_status_code_error(struct http_response_s * response_obj, int status_code, char *reason_phrase, char *error_message);

char * get_response_text(struct http_response_s *response_obj); 
