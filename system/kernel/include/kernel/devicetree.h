/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstring>
#ifndef _KERN_DEVICETREE_H
#define _KERN_DEVICETREE_H 1

#include <cstdint>
#include <feline/endian.h>

struct fdt_header {
	big_endian<uint32_t> magic;
	big_endian<uint32_t> totalsize;
	big_endian<uint32_t> off_dt_struct;
	big_endian<uint32_t> off_dt_strings;
	big_endian<uint32_t> off_mem_rsvmap;
	big_endian<uint32_t> version;
	big_endian<uint32_t> last_comp_version;
	big_endian<uint32_t> boot_cpuid_phys;
	big_endian<uint32_t> size_dt_strings;
	big_endian<uint32_t> size_dt_struct;
};

#define FDT_MAGIC 0xd00dfeed

struct fdt_reserve_entry {
	big_endian<uint64_t> address;
	big_endian<uint64_t> size;
};

enum fdt_node_type : uint32_t {
	FDT_BEGIN_NODE=0x1,
	FDT_END_NODE=0x2,
	FDT_PROP=0x3,
	FDT_NOP=0x4,
	FDT_END=0x9,
};

inline fdt_node_type reverse_endian(fdt_node_type value) {
	return static_cast<fdt_node_type>(reverse_endian(static_cast<uint32_t>(value)));
}

struct fdt_prop {
	big_endian<uint32_t> len;
	big_endian<uint32_t> nameoff;
	big_endian<uint32_t> value[];
};

struct fdt_struct_entry {
	big_endian<fdt_node_type> node_type;
	union {
		struct {
			char node_name[]; //Only for FDT_BEGIN_NODE
		};
		fdt_prop prop; //Only for FDT_PROP
	};
};

struct devicetree_cell_size {
	uint32_t address_cells;
	uint32_t size_cells;
};

void init_devicetree(fdt_header *header);

void for_each_prop_in_node(
		char const *prefix,
		void callback(fdt_struct_entry*, devicetree_cell_size curr_cell_size, char*, void*),
		void*
		);

#endif /* _KERN_DEVICETREE_H */
