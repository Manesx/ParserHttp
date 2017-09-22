#include <sys/types.h>

typedef struct {
    size_t len;
    char *pointer;
} string;

/* this function accept buffer, size of buffer, array key words
 * and callback function template: void *name_function(string,string),
 * where one-opt - return pointer key word in buffer, two-opt -
 * return value in buffer. Function return number lines */

extern int http_parser (char *buff, int buff_len, char **keys, void *callback);