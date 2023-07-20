/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstdlib>
#include <kernel/devicetree.h>
#include <kernel/log.h>
#include <cinttypes>

fdt_header *devicetree;
fdt_struct_entry *structs;
char *strings;

inline void increment_fdt_struct(fdt_struct_entry **entry) {
	auto pad_len = [](size_t len){
		if (len%4==0) {
			return len;
		}
		return len+=4-(len%4);
	};
	auto strlen_with_null_and_padding = [pad_len](char const *str){
		size_t len=strlen(str);
		len += 1;
		return pad_len(len);
	};
	switch ((**entry).node_type) {
		case FDT_BEGIN_NODE:
			*entry = reinterpret_cast<fdt_struct_entry*>(
					reinterpret_cast<uintptr_t>(*entry)
					+sizeof(fdt_node_type)
					+strlen_with_null_and_padding((**entry).node_name)
					);
			break;
		case FDT_PROP:
			*entry = reinterpret_cast<fdt_struct_entry*>(
					reinterpret_cast<uintptr_t>(*entry)
					+sizeof(fdt_node_type)
					+sizeof(fdt_prop)
					+pad_len((**entry).prop.len)
					);
			break;
		case FDT_END_NODE:
		case FDT_NOP:
			*entry = reinterpret_cast<fdt_struct_entry*>(
					reinterpret_cast<uintptr_t>(*entry)
					+sizeof(fdt_node_type)
					);
			break;
		case FDT_END:
		default:
			kcritical("Attempted to move past the end of the device tree, or are working with an invalid one.");
			abort();
	}
}

static void for_each_prop_in_node(
		char const *prefix,
		fdt_struct_entry **node,
		void callback(fdt_struct_entry*, devicetree_cell_size curr_cell_size, char*, void*),
		bool usecallback,
		devicetree_cell_size curr_cell_size,
		void *state
		){
	devicetree_cell_size next_cell_size={.address_cells=2, .size_cells=1};
	while (true) {
		switch ((**node).node_type) {
			case FDT_BEGIN_NODE:
				if (strncmp(prefix, (**node).node_name, strlen(prefix))==0) {
					increment_fdt_struct(node);
					for_each_prop_in_node(prefix, node, callback, true, next_cell_size, state);
				}
				else {
					increment_fdt_struct(node);
					for_each_prop_in_node(prefix, node, callback, false, next_cell_size, state);
				}
				break;
			case FDT_PROP:
				if (strcmp("#address-cells", strings+(**node).prop.nameoff)==0) {
					next_cell_size.address_cells=(**node).prop.value[0];
				}
				else if (strcmp("#size-cells", strings+(**node).prop.nameoff)==0) {
					next_cell_size.size_cells=(**node).prop.value[0];
				}
				if (usecallback) {
					callback(*node, curr_cell_size, strings, state);
				}
				increment_fdt_struct(node);
				break;
			case FDT_NOP:
				increment_fdt_struct(node);
				break;
			case FDT_END_NODE:
				increment_fdt_struct(node);
				return;
			case FDT_END:
				return;
			default:
				kcritical("Invalid device tree! Halting.");
				abort();
		}
	}
}

void for_each_prop_in_node(
		char const *prefix,
		void callback(fdt_struct_entry *entry, devicetree_cell_size curr_cell_size, char *strings, void *user),
		void *state
		){
	fdt_struct_entry *current_struct=structs;
	return for_each_prop_in_node(prefix, &current_struct, callback, false, {.address_cells=2, .size_cells=1}, state);
}

void init_devicetree(fdt_header *header) {
	devicetree=header;
	strings=reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(header)+header->off_dt_strings);
	structs=reinterpret_cast<fdt_struct_entry*>(reinterpret_cast<uintptr_t>(header)+header->off_dt_struct);
}
