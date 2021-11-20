// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cstring>

char *strcpy(char *dest, const char *src){
	//Do an ititial copy
	dest[0]=src[0];
	//If it is null (src==""), return
	//Because we look back 1 in the loop
	if(src[0]=='\0'){
		return dest;
	}
	//Loop upwards to avoid calling strlen
	//Check if count-1!='\0' to because we copy null anyway
	for(size_t count=1; src[count-1]!='\0'; count++){
		dest[count]=src[count];
	}
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n){
	//Loop upwards
	for(size_t count=0;count<n;count++){
		//If we are at the end of src
		if(src[count]=='\0'){
			//Fill the rest of dest with null
			memset(dest+count, '\0', n-count);
			return dest;
		}
		dest[count]=src[count];
	}
	return dest;
}

size_t strlcpy(char *dest, const char *src, size_t n){
	size_t count;
	//Count upwards
	for(count=0; count<n; count++){
		if((dest[count]=src[count])=='\0'){
			return count;
		}
	}
	return count;
}
