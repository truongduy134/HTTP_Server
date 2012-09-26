#include "helperlib.h"

#define MAX_LEN 2000
void print_system_error(char *message)
{
	perror(message);
	exit(1);
}

void print_user_error(char *message, char *detail)
{
	fprintf(stderr, "%s: %s\n", message, detail);
	exit(1);
}

/* The comparison scheme is case-insensitive */
int find_str_index(char *strArr[], int numStr, char *str)
{
	int index;

	for(index = 0; index < numStr; index++)
	{
		if(strcasecmp(strArr[index], str) == 0)
			return index;
	}

	return -1;
}

/* The comparison scheme is case-sensitive */
int find_occurrence_index(char *str, char character, int occurence)
{
	int index, curr_occur;
	
	for(index = 0, curr_occur = 0; str[index] != '\0'; index++)
		if(str[index] == character)
		{
			curr_occur++;
			if(curr_occur == occurence)
				return index;
		}

	return -1;
}

/* copy_str_dynamic(...)
 *
 * 	Copies the content of the input string to new dynamically allocated memory slots.
 * 
 *	Returns a pointer to the beginning of the new memory slots.
 * 	Returns NULL if there is an exception in allocation or copying. 
 */
char * copy_str_dynamic(char * originalStr)
{
	if(originalStr == NULL)
		return NULL;

	/* NOTE: Make 1 more SLOT of '\0' !!! */
	char * newStrPointer = (char *) malloc((strlen(originalStr) + 1) * sizeof(char));

	if(newStrPointer == NULL)
	{
		printf("error with malloc\n");
		return NULL;
	}

	strcpy(newStrPointer, originalStr);

	return newStrPointer;
}

/* count_occurence(...)
 *
 *	Returns the number of occurences of the input character in the given string. 
 *
 *	The comparison scheme is case-sensitive.
 */
int count_occurence(char *str, char character)
{
	int num = 0, index;

	if(str != NULL)
	{
		for(index = 0; str[index] != '\0'; index++)
			if(str[index] == character)
				num++;
	}

	return num;
}
/* read_whole_file(...)
 *
 *	Copies the whole content of a file to a dynamically allocated memory slots.
 *
 *	Returns a pointer to the beginning of the content. Upon sucess, *error_code = 0.
 *	Returns NULL if there is an error when opening the file or allocating mmemory. If so, 
 *		*error_code = 1 if there is an error opening the file.
 *		*error_code = 2 if there is an error allocating memory 
 *
 * Note: Users have to free the memory allocated for *error_message themselves
 */
char * read_whole_file(char * relative_path, int *error_code)
{
	char *buffer;
	int numByte;

	printf("Go to function read_whole_file\n");
	/* For POSIX conforming system, "r" and "rb" are the same */
	FILE *fin = fopen(relative_path, "rb");
	if(fin == NULL)
	{
		*error_code = 1;
		return NULL;
	}
	printf("Valid path OK\n");
	/* Set the file position indicator to the end of the stream */
	fseek(fin, 0, SEEK_END);
	/* Determine the total number of bytes of the stream */
	numByte = ftell(fin);
	/* Set the file position indicator to the beginning of the stream */
	rewind(fin);
	printf("Determine size = %d: OK\n", numByte);

	buffer = (char *) malloc((numByte + 1) * sizeof(char));
	if(buffer == NULL)
	{
		*error_code = 2;
		return NULL;
	}

	fread(buffer, numByte, 1, fin);
	buffer[numByte] = '\0';
	printf("Length buffer = %d vs Byte = %d\n", strlen(buffer), numByte);
	printf("Read OK\n");
	fclose(fin);
	printf("Close file OK\n");
	/* Upon success, *error_code = 0 */
	*error_code = 0;
	printf("Totally OK?\n");
	puts(buffer);
	return buffer;
}

char * generate_html_error_page(char * header, char * error_message)
{
	char buffer[MAX_LEN + 1];
	
	sprintf(buffer, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n<title>%s</title>\n</head>\n<body><h1>%s</h1><p>%s</p></body></html>", header, header, error_message);

	return copy_str_dynamic(buffer);	
}

