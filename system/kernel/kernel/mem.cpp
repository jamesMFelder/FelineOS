#include <algorithm>
#include <cstdlib>
#include <kernel/mem.h>
#include <kernel/paging.h>
#include <kernel/log.h>

inline uintptr_t offset(uintptr_t const addr) {
	return addr%PHYS_MEM_CHUNK_SIZE;
}

inline uintptr_t offset(void const * const addr) {
	return offset(reinterpret_cast<uintptr_t>(addr));
}

mem_results get_mem_from(void *phys_addr, void **new_virt_addr, uintptr_t len) {
	pmm_results phys_mem_results;
	map_results virt_mem_results;

	phys_mem_results=get_mem_area(phys_addr, len);
	switch (phys_mem_results) {
		case pmm_success:
			break;
		case pmm_nomem:
			return mem_no_physmem;
		case pmm_null:
		case pmm_invalid:
			return mem_invalid;
	}

	virt_mem_results=map_range(phys_addr, len, new_virt_addr, 0);
	switch (virt_mem_results) {
		case map_success:
			break;
		case map_no_virtmem:
			return mem_no_virtmem;
		case map_no_perm:
		case map_already_mapped:
		case map_notmapped:
		case map_err_kernel_space:
			kcritical("The VMM has a bug!\n");
			std::abort();
		case map_no_physmem:
		case map_invalid_align:
		case map_invalid_option:
			kcritical("The memory manager has a bug!\n");
			std::abort();
	}
	return mem_success;
}

mem_results get_mem_at(void *virt_addr, uintptr_t len) {
	void *phys_addr;
	pmm_results phys_mem_results;
	map_results virt_mem_results;

	phys_mem_results=get_mem_area(&phys_addr, len);
	switch (phys_mem_results) {
		case pmm_success:
			break;
		case pmm_nomem:
			return mem_no_physmem;
		case pmm_null:
		case pmm_invalid:
			kcritical("The PMM has a bug!\n");
			std::abort();
	}

	uintptr_t phys_addr_with_offset=reinterpret_cast<uintptr_t>(phys_addr);
	phys_addr_with_offset+=offset(virt_addr);
	virt_mem_results=map_range(reinterpret_cast<void*>(phys_addr_with_offset), len, virt_addr, 0);
	switch (virt_mem_results) {
		case map_success:
			break;
		case map_no_perm:
		case map_err_kernel_space:
			return mem_perm_denied;
		case map_no_virtmem:
			return mem_no_virtmem;
		case map_already_mapped:
		case map_notmapped:
			kcritical("The VMM has a bug!\n");
			std::abort();
		case map_no_physmem:
		case map_invalid_align:
		case map_invalid_option:
			kcritical("The memory manager has a bug!\n");
			std::abort();
	}
	return mem_success;
}

mem_results get_mem(void **new_virt_addr, uintptr_t len) {
	void *phys_addr;
	pmm_results phys_mem_results;
	map_results virt_mem_results;

	phys_mem_results=get_mem_area(&phys_addr, len);
	switch (phys_mem_results) {
		case pmm_success:
			break;
		case pmm_nomem:
			return mem_no_physmem;
		case pmm_null:
		case pmm_invalid:
			kcritical("The PMM has a bug!\n");
			std::abort();
	}

	virt_mem_results=map_range(phys_addr, len, new_virt_addr, 0);
	switch (virt_mem_results) {
		case map_success:
			break;
		case map_no_perm:
		case map_err_kernel_space:
			return mem_perm_denied;
		case map_no_virtmem:
			return mem_no_virtmem;
		case map_already_mapped:
		case map_notmapped:
			kcritical("The VMM has a bug!\n");
			std::abort();
		case map_no_physmem:
		case map_invalid_align:
		case map_invalid_option:
			kcritical("The memory manager has a bug!\n");
			std::abort();
	}
	//Clear the memory
	std::fill_n(static_cast<std::byte*>(*new_virt_addr), PHYS_MEM_CHUNK_SIZE, std::byte{0xCC});
	return mem_success;
}

mem_results free_mem(void *addr, uintptr_t len){
	map_results virt_mem_results;

	/* Protect against freeing an uninitialized pointer */
	if (addr == nullptr) {
		return mem_invalid;
	}

	virt_mem_results=unmap_range(addr, len, PHYS_ADDR_AUTO);
	switch (virt_mem_results) {
		case map_success:
			break;
		case map_no_perm:
		case map_err_kernel_space:
			return mem_perm_denied;
		case map_invalid_align:
		case map_already_mapped:
		case map_no_physmem:
		case map_no_virtmem:
			kcritical("The VMM has a bug!\n");
			std::abort();
		case map_invalid_option:
			kcritical("The memory manager has a bug!\n");
			std::abort();
		case map_notmapped:
			return mem_invalid;
	}
	return mem_success;
}
