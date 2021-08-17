#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

size_t strlen(const char*);

char* strcpy(char*, const char*);
char* strncpy(char*, const char*, size_t);
size_t strlcpy(char*, const char*, size_t);

char *strcat(char*, const char*);
char *strncat(char*, const char*, size_t);
__attribute__((deprecated("Not implemented correctly."))) size_t strlcat(char*, const char*, size_t);

//Turn ints to strings
//__attribute__((deprecated)) int lltostr(const long long unsigned num, char str[17]);
int lltostr(const long long unsigned num, char str[17]);
#define ltostr(num, str) lltostr(num, str)
#define itostr(num, str) lltostr(num, str)
#define stostr(num, str) lltostr(num, str)

//Turn ints to lower-hex strings
//__attribute__((deprecated)) int xlltostr(const long long unsigned num, char str[17]);
int xlltostr(const long long unsigned num, char str[17]);
#define xltostr(num, str) xlltostr(num, str)
#define xtostr(num, str) xlltostr(num, str)
#define xstostr(num, str) xlltostr(num, str)

//Turn ints to UPPER-HEX strings
//__attribute__((deprecated)) int Xlltostr(const long long unsigned num, char str[17]);
int Xlltostr(const long long unsigned num, char str[17]);
#define Xltostr(num, str) Xlltostr(num, str)
#define Xtostr(num, str) Xlltostr(num, str)
#define Xstostr(num, str) Xlltostr(num, str)

//Easy name for upper hex conversion
#define ptrtostr(num, str) Xlltostr(num, str)


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_STRING_H
