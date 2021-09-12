#include <string.h>

//Convert a number to a string
//By assuming that base<=10, we force the caller to deal with higher bases with
// `if(c>'9') c+=39` for lowercase or `c+=7` for uppercase
//TODO: dynamic str length
int ntostr(long long unsigned num, char str[20], short int base){
	//Start with the 1s digits
	unsigned long long int todivby=1;
	//Work right to left
	char *ptr=&str[16];
	//Add a null so it is a valid string
	*ptr='\0';
	//Start by decrementing ptr because it just added a null
	//Do this until it is infront of the string
	//Go backwards, and shift digits each time
	for(ptr--;ptr>=str;ptr--,todivby*=base){
		*ptr=(num/todivby)%base+'0';
	}
	return 0;
}

int ottostr(const ptrdiff_t num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int oztostr(const size_t num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int olltostr(const long long unsigned int num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int oltostr(const long unsigned int num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int otostr(const unsigned int num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int ostostr(const short unsigned num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}

int octostr(const unsigned char num, char str[20]){
	int retval=ntostr(num, str, 8);
	return retval;
}


int ttostr(const ptrdiff_t num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int ztostr(const size_t num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int lltostr(const long long unsigned int num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int ltostr(const long unsigned int num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int itostr(const unsigned int num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int stostr(const short unsigned num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}

int ctostr(const unsigned char num, char str[20]){
	int retval=ntostr(num, str, 10);
	return retval;
}


int xttostr(ptrdiff_t num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xztostr(size_t num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xlltostr(long long unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xltostr(long unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xtostr(unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xstostr(unsigned short num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}

int xctostr(unsigned char num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=39;
		}
	}
	return retval;
}


int Xttostr(ptrdiff_t num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xztostr(size_t num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xlltostr(long long unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xltostr(long unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xtostr(unsigned num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xstostr(unsigned short num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}

int Xctostr(unsigned char num, char str[20]){
	int retval=ntostr(num, str, 0x10);
	for(char *ptr=str;*ptr!='\0';ptr++){
		if(*ptr>'9'){
			*ptr+=7;
		}
	}
	return retval;
}
