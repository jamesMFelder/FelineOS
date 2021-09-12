// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <string.h>

char *strcat(char *dest, const char *src){
	return strcpy(dest+strlen(dest), src);
}

char *strncat(char *dest, const char *src, size_t n){
	size_t count;
	char *dest_ptr=dest+strlen(dest);
	//Loop up
	for(count=0; count<n; count++){
		//If we copy a null byte
		if((*(dest_ptr+count)=*(src+count))=='\0'){
			//We are done
			return dest;
		}
	}
	//Since we haven't copied a null byte, append one now
	*(dest_ptr+count)='\0';
	return dest;
}

size_t strlcat(char *dest, const char *src, size_t n){
	size_t max=n-strlen(dest);
	size_t count;
	//append, don't overwrite
	char *dest_ptr=dest+strlen(dest);
	//loop up
	for(count=0;count<max;count++){
		//If we copy a null byte
		if((*(dest_ptr+count)=*(src+count))=='\0'){
			//We are done
			return (dest_ptr+count)-dest;
		}
	}
	//I don't think we append a null byte here, but I'm not sure
	//Also, is this the correct return value
	return count;
}
