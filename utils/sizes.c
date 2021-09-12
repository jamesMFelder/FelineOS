#define PRINT_SIZE(size) printf("\"%s\": \"%zu\"%c\n", #size, sizeof(size), eolc)
#define NL printf("\n")
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv){
	char eolc='\0';
	//If we have an argument
	if(argc>1){
		//strcmp returns 0 when they are the same and 0 is false
		if(!strcmp(argv[1], "--json")){
			eolc=',';
			printf("{\n");
		}
		else if(!strcmp(argv[1], "--help")){
			printf("%s: print the sizes of various types.\n", argv[0]);
			puts("\t--json: output the results in json.");
			puts("\t--help: show this help.");
			return 0;
		}
	}
	//if(argc>1){if(!strcmp(argv[1], "--json")){printf("{\n");}}
	PRINT_SIZE(void);
	NL;
	PRINT_SIZE(char);
	PRINT_SIZE(int);
	PRINT_SIZE(short int);
	PRINT_SIZE(long int);
	PRINT_SIZE(long long int);
	NL;
	PRINT_SIZE(uint16_t);
	PRINT_SIZE(uint32_t);
	PRINT_SIZE(uint64_t);
	NL;
	PRINT_SIZE(float);
	PRINT_SIZE(float_t);
	PRINT_SIZE(float _Complex);
	PRINT_SIZE(double);
	PRINT_SIZE(double_t);
	PRINT_SIZE(double _Complex);
	PRINT_SIZE(long double);
	PRINT_SIZE(long double _Complex);
	NL;
	PRINT_SIZE(bool);
	PRINT_SIZE(size_t);
	NL;
	PRINT_SIZE(FILE);
	NL;
	PRINT_SIZE(time_t);
	NL;
	PRINT_SIZE(timer_t);
	NL;
	printf("\"%s\": \"%zu\"%c\n", "void*", sizeof(void*), ','==eolc?' ':eolc);
	if(argc>1){if(!strcmp(argv[1], "--json")){printf("}\n");}}
	return 0;
}
