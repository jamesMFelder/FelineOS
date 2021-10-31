// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_MEM_H
#define _KERN_MEM_H 1

//4 Kilobytes
#define PHYS_MEM_CHUNK_SIZE 4096

#ifdef __cplusplus
extern "C"{
#endif
//Get a free area
void *get_mem_area();

//Return it (TODO: keep track of who can free what)
int free_mem_area(void *loc);
#ifdef __cplusplus
}
#endif

#endif //_KERN_MEM_H
