// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_MEM_H
#define _KERN_MEM_H

//4 Kilobytes
#define PHYS_MEM_CHUNK_SIZE 4096

//Get a free area
void *get_mem_area();

//Return it (TODO: keep track of who can free what)
int free_mem_area(void *loc);

#endif //_KERN_MEM_H
