#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* Note that after printing errors, the server is terminated */
void print_user_error(char *message, char *detail);
void print_system_error(char *message);

/* Find the index of the input string in the input string array. The comparison scheme is case-insensitive */
int find_str_index(char *strArr[], int numStr, char *str);

/* Find the index of the n-th occurence of the input character in the given string. The comparison scheme is case-sensitive */
int find_occurence_index(char *str, char character, int occurence);

/* Count the number of occurences of the input character in the given string. The comparison scheme is case-sensitive */
int count_occurence(char *str, char character);

/* Allocate memory for new string in the heap. Copy the content of the old string to the new memory slots. Return the pointer to that memory slots.
 *
 * Note: Users have to free the memory themselves after use.
 */
char * copy_str_dynamic(char * originalStr);

/* Read the whole content of a file whose relative path is given as input and return a pointer to the content which is allocated in heap space */
char * read_whole_file(char * relative_path, int *error_code);

/* Generate a HTML document displaying an error message */
char * generate_html_error_page(char *header, char *error_message);
 

