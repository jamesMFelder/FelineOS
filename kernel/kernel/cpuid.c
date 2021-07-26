#include "cpuid.h"

//We are assuming __get_cpuid will never fail.
int cpuid_vendor(char vendor[13]){
	unsigned int eax=0, ebx=0, ecx=0, edx=0;
	//Actually execute the function
	__get_cpuid(0, &eax, &ebx, &ecx, &edx);

	//Turn the numbers into a string
	vendor[0]=(ebx & 255); vendor[1]=((ebx & (255<<8))>>8);
	vendor[2]=((ebx & (255<<16))>>16); vendor[3]=((ebx & (255ull<<24))>>24);

	vendor[4]=(edx & 255); vendor[5]=((edx & (255<<8))>>8);
	vendor[6]=((edx & (255<<16))>>16); vendor[7]=((edx & (255ull<<24))>>24);

	vendor[8]=(ecx & 255); vendor[9]=((ecx & (255<<8))>>8);
	vendor[10]=((ecx & (255<<16))>>16); vendor[11]=((ecx & (255ull<<24))>>24);

	//Add a null so it is a string
	vendor[12]='\0';
	return eax;
}
