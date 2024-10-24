#ifndef _STRING_H
#define _STRING_H

#include <stddef.h> // Include for size_t

void *memset(void *dest, int val, size_t len);
void *memcpy(void *dest, const void *src, size_t len);
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
int my_strcmp(const char *str1, const char *str2);

#endif
