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
int ttostr(const ptrdiff_t num, char str[20]);
int ztostr(const size_t num, char str[20]);
int lltostr(const long long unsigned num, char str[20]);
int ltostr(const long unsigned num, char str[20]);
int itostr(const unsigned num, char str[20]);
int stostr(const short unsigned num, char str[20]);
int ctostr(const unsigned char num, char str[20]);

//Turns ints to octal strings
int ottostr(const ptrdiff_t num, char str[20]);
int oztostr(const size_t num, char str[20]);
int olltostr(const long long unsigned num, char str[20]);
int oltostr(const long unsigned num, char str[20]);
int otostr(const unsigned num, char str[20]);
int ostostr(const short unsigned num, char str[20]);
int octostr(const unsigned char num, char str[20]);

//Turn ints to lower-hex strings
int xttostr(const ptrdiff_t num, char str[20]);
int xztostr(const size_t num, char str[20]);
int xlltostr(const long long unsigned num, char str[20]);
int xltostr(const long unsigned num, char str[20]);
int xtostr(const unsigned num, char str[20]);
int xstostr(const short unsigned num, char str[20]);
int xctostr(const unsigned char num, char str[20]);

//Turn ints to UPPER-HEX strings
int Xttostr(const ptrdiff_t num, char str[20]);
int Xztostr(const size_t num, char str[20]);
int Xlltostr(const long long unsigned num, char str[20]);
int Xltostr(const long unsigned num, char str[20]);
int Xtostr(const unsigned num, char str[20]);
int Xstostr(const short unsigned num, char str[20]);
int Xctostr(const unsigned char num, char str[20]);

//Easy name for upper hex conversion
int ptrtostr(const size_t num, char str[20]);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_STRING_H
