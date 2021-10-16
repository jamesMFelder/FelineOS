// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <kernel/log.h>

//Add a log. Logging functions should always use this so I can change the backend
void __internal_log(const int level, const char *fmt, va_list data){

	if(level>3){
		kwarnf("Attempt to log at level %d.", level);
		return;
	}

	static const char *err_level[]={"CRIT", "ERR", "WARN", "LOG"};
	printf("%s %s: ", "Feline OS", err_level[level]);

	vprintf(fmt, data);

	putchar('\n');
	return;
}

//For the moment, just write to terminal.
__attribute__ ((format (printf, 1, 2))) void kerrorf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(1, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void kwarnf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(2, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void klogf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(3, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void kcriticalf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(0, format, data);
	va_end(data);
	return;
}
