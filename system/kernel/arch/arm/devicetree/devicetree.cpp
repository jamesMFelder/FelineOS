/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <kernel/devicetree.h>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/vtopmem.h>

fdt_header const *devicetree;
fdt_struct_entry *structs;
char *strings;

inline void increment_fdt_struct(fdt_struct_entry **entry) {
	auto pad_len = [](size_t len) {
		if (len % 4 == 0) {
			return len;
		}
		return len += 4 - (len % 4);
	};
	auto strlen_with_null_and_padding = [pad_len](char const *str) {
		size_t len = strlen(str);
		len += 1;
		return pad_len(len);
	};
	switch ((**entry).node_type) {
	case FDT_BEGIN_NODE:
		*entry = reinterpret_cast<fdt_struct_entry *>(
			reinterpret_cast<uintptr_t>(*entry) + sizeof(fdt_node_type) +
			strlen_with_null_and_padding((**entry).node_name));
		break;
	case FDT_PROP:
		*entry = reinterpret_cast<fdt_struct_entry *>(
			reinterpret_cast<uintptr_t>(*entry) + sizeof(fdt_node_type) +
			sizeof(fdt_prop) + pad_len((**entry).prop.len));
		break;
	case FDT_END_NODE:
	case FDT_NOP:
		*entry = reinterpret_cast<fdt_struct_entry *>(
			reinterpret_cast<uintptr_t>(*entry) + sizeof(fdt_node_type));
		break;
	case FDT_END:
#pragma GCC diagnostic push
		// We need to cover the default case because we're reading this from
		// raw memory that is only 99% likely to have the correct enum value
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#endif // __clang__
	default:
#pragma GCC diagnostic pop
		kcritical("Attempted to move past the end of the device tree, or are "
		          "working with an invalid one.");
		std::abort();
	}
}

static void for_each_prop_in_node(
	char const *prefix, fdt_struct_entry **node,
	void callback(fdt_struct_entry *, devicetree_cell_size curr_cell_size,
                  char *, void *),
	bool usecallback, devicetree_cell_size curr_cell_size, void *state) {
	devicetree_cell_size next_cell_size = {.address_cells = 2, .size_cells = 1};
	while (true) {
		switch ((**node).node_type) {
		case FDT_BEGIN_NODE:
			if (strncmp(prefix, (**node).node_name, strlen(prefix)) == 0) {
				increment_fdt_struct(node);
				for_each_prop_in_node(prefix, node, callback, true,
				                      next_cell_size, state);
			} else {
				increment_fdt_struct(node);
				for_each_prop_in_node(prefix, node, callback, false,
				                      next_cell_size, state);
			}
			break;
		case FDT_PROP:
			if (strcmp("#address-cells", strings + (**node).prop.nameoff) ==
			    0) {
				next_cell_size.address_cells = (**node).prop.value[0];
			} else if (strcmp("#size-cells", strings + (**node).prop.nameoff) ==
			           0) {
				next_cell_size.size_cells = (**node).prop.value[0];
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
#pragma GCC diagnostic push
			// We need to cover the default case because we're reading this from
			// raw memory that is only 99% likely to have the correct enum value
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#endif // __clang__
		default:
#pragma GCC diagnostic pop
			kcritical("Invalid device tree! Halting.");
			std::abort();
		}
	}
}

void for_each_prop_in_node(char const *prefix,
                           void callback(fdt_struct_entry *entry,
                                         devicetree_cell_size curr_cell_size,
                                         char *strings, void *user),
                           void *state) {
	fdt_struct_entry *current_struct = structs;
	return for_each_prop_in_node(prefix, &current_struct, callback, false,
	                             {.address_cells = 2, .size_cells = 1}, state);
}

fdt_header const *init_devicetree(PhysAddr<fdt_header const> header) {
	fdt_header dt_header = read_pmem(header);
	map_results dt_mapping =
		map_range(header, dt_header.totalsize,
	              reinterpret_cast<void const **>(&devicetree), 0);
	switch (dt_mapping) {
	// It worked!
	case map_success:
		break;
	case map_no_virtmem:
		kcritical("Out of virtual memory while booting! How is this possible? "
		          "Aborting now.");
		std::abort();
	// These should be impossible for mapping with an unspecified virt_addr
	case map_already_mapped:
	case map_notmapped:
	case map_invalid_align:
	// These can't happen with opts being 0
	case map_invalid_option:
	case map_no_physmem:
	// A. we're the kernel. B. We don't even have use these errors!
	case map_no_perm:
	case map_err_kernel_space:
		kerrorf("Unexpected error mapping the devicetree: %d. Aborting now!",
		        dt_mapping);
		std::abort();
	}
	if (devicetree->magic != FDT_MAGIC) {
		kerrorf("Device tree magic %#" PRIx32 " is not the expected %#" PRIx32
		        "!",
		        static_cast<uint32_t>(devicetree->magic),
		        static_cast<uint32_t>(FDT_MAGIC));
		abort();
	}
	strings = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(devicetree) +
	                                   devicetree->off_dt_strings);
	structs = reinterpret_cast<fdt_struct_entry *>(
		reinterpret_cast<uintptr_t>(devicetree) + devicetree->off_dt_struct);
	return devicetree;
}
