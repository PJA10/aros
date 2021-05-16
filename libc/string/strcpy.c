#include <string.h>

char *strcpy(char* destination, const char* source) {
	memcpy(destination, source, strlen(source) + 1);
    return destination;
}
