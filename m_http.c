#include "m_http.h"

/* parse_http_request(...)
 *
 *	Parses a HTTP request string to obtain information such as: method, file path, version, etc.
 *	
 *	Returns 1 if the request follows HTTP request format and the parsing is sucessful.
 *	Returns 0 otherwise.	
 *
 *	The response object in the parameter list will be updated accordingly (e.g. the HTTP version, status code, reason_phrase and message_body).
 *
 *
 *	Currently, we do not support getting header information
 */
int parse_http_request(struct http_request_s * request_obj, char * request, struct http_response_s * response_obj)
{
	char delims[3] = "\r\n";
	char *requestLine = NULL;
	char m_method[20], m_url[500], m_protocol[20];
	int index;
	char relative_path[510] = ".";   /* Relative path has the form ./<path> */

	printf("Going to do strtok\n");
	/* Parse and handle the first line of request */
	requestLine = strtok(request, delims);
	printf("Finish strtok the first time\n");
	printf("Result = %s\n", requestLine);

	if(requestLine == NULL)
	{
		set_status_code_error(response_obj, 400, "400 Bad Request", "Your request does not follow HTTP format");
		return 0;
	}
	sscanf(requestLine, "%s %s %s", m_method, m_url, m_protocol);
	printf("method = %s\n", m_method);
	printf("url = %s\n", m_url);
	printf("protcol = %s\n", m_protocol);
	printf("Going to check method name!\n");
	if(find_str_index(METHOD_LIST, NUM_METHOD_TYPE, m_method) < 0)
	{
		/* Exception: Method is not implemented */
		set_status_code_error(response_obj, 501, "501 Not Implemented", "Method name is invalid or it is not implemented on this server");
		return 0;
	}
	printf("Hullo 1\n");
	request_obj->method = copy_str_dynamic(m_method);
 	printf("Method name is OK\n");
	printf("Going to check protocol\n");
	if(strcasestr(m_protocol, "HTTP") == NULL)
	{
		/* Exception: The protocol is not HTTP */
		set_status_code_error(response_obj, 505, "505 HTTP Version Not Supported", "The protocol is not HTTP. This server can only handle HTTP requests");
		return 0;
	}
	printf("Hullo 2\n");
	request_obj->version = copy_str_dynamic(m_protocol);
	printf("Protocol is OK\n");
	if(strcasestr(m_url, "HTTP://") != NULL)
	{
		/* Now the url is of the format http://www.xyz.com/<relative_path> */
		index = find_occurrence_index(m_url, '/', 3);		
	}
	else
		/* Then the format of m_url is, for example, /ABC/XYZ.html */
		index = 0;
	
	strcat(relative_path, m_url + index);
	printf("Relative path = %s\n", relative_path);

	request_obj->path = copy_str_dynamic(relative_path); 
//	printf("Path = %s\n", request_obj->path);

	if(request_obj->method == NULL || request_obj->version == NULL || request_obj->path == NULL)
	{
		set_status_code_error(response_obj, 500, "500 Internal Server Error", "Memory allocation in the server failed");
		return 0;
	}

	printf("Hullo. Finish the path");
	/* Continue to parse other header information */
	while(requestLine != NULL)
	{
		requestLine = strtok(NULL, delims);

		if(requestLine != NULL)
		{
			
		}
	}

	return 1;
}

struct http_request_s * create_request_struct()
{
	struct http_request_s * new_obj = (struct http_request_s *) malloc(sizeof(struct http_request_s));
	int index;

	new_obj->method = NULL;
	new_obj->path = NULL;
	new_obj->version = NULL;
	for(index = 0; index < NUM_REQUEST_HEADER_TYPE; index++)
		new_obj->request_header[index] = NULL;

	return new_obj;	
}

void delete_request(struct http_request_s ** request_obj)
{
	int index;
	free((*request_obj)->method);
	free((*request_obj)->path);
	free((*request_obj)->version);
	for(index = 0; index < NUM_REQUEST_HEADER_TYPE; index++)
		free((*request_obj)->request_header[index]);

	free(*request_obj);
	*request_obj = NULL;	
}

int exec_http_request(struct http_request_s *request_obj, struct http_response_s *response_obj)
{
	int error_code;
	char *file_content;

	if(strcasecmp(request_obj->method, "GET") == 0)
	{
		file_content = read_whole_file(request_obj->path, &error_code);

		if(error_code == 0)
		{
			printf("Sucess in exec\n");
			/* Reading file content is successful */
			response_obj->message_body = file_content;
			response_obj->status_code = 200;
			response_obj->reason_phrase = copy_str_dynamic("200 OK");
		}
		else
		{
			if(error_code == 1)
			{
				/* Error when opening files */
				switch(errno)
				{
					case ENAMETOOLONG:
						set_status_code_error(response_obj, 414, "414 Request-URI Too Long", "Your Request URI or path name is too long to be handled on this server.");
						break;
					case EOVERFLOW:
						set_status_code_error(response_obj, 413, "413 Request Entity Too Large", "Your requested file is too large to be opened on this server.");
						break;
					case ENOENT:
						set_status_code_error(response_obj, 404, "404 Not Found", "Your requested file cannot be found on this server.");
				}
			}
			else
				/* Error when using malloc */
				set_status_code_error(response_obj, 500, "500 Internal Server Error", "Memory allocation on this server failed.");
			return 0;
		}
	}

	return 1;
}

struct http_response_s * create_response_struct()
{
	struct http_response_s * new_obj = (struct http_response_s *) malloc(sizeof(struct http_response_s));
	int index;

	new_obj->version = NULL;
	new_obj->status_code = 0;
	new_obj->reason_phrase = NULL;
	new_obj->message_body = NULL;

	for(index = 0; index < NUM_RESPONSE_HEADER_TYPE; index++)
		new_obj->response_header[index] = NULL;

	return new_obj;
}

void delete_response(struct http_response_s ** response_obj)
{
	int index;

	free((*response_obj)->version);
	free((*response_obj)->reason_phrase);
	free((*response_obj)->message_body);

	for(index = 0; index < NUM_RESPONSE_HEADER_TYPE; index++)
		free((*response_obj)->response_header[index]);

	free(*response_obj);
	*response_obj = NULL;	
}

/* 
 * Set status_code, reason_phrase, error_message of the http_response_s structure based on input parameters 
 *
 */
void set_status_code_error(struct http_response_s *response_obj, int status_code, char * reason_phrase, char * error_message)
{
	response_obj->status_code = status_code;
	response_obj->reason_phrase = copy_str_dynamic(reason_phrase);
	response_obj->message_body = generate_html_error_page(reason_phrase, error_message);	
}

/* Get a text representing the response object. This is for sending purpose */
char * get_response_text(struct http_response_s *response_obj)
{
	char buffer[MAX_LENGTH_RESPONSE];
	char *temp;

	sprintf(buffer, "%s %d %s\r\n\r\n%s", response_obj->version, response_obj->status_code, response_obj->reason_phrase, response_obj->message_body);

	return copy_str_dynamic(buffer);
}
