#include <feline/kvector.h>
#include <kernel/log.h>

void check_index(size_t index, size_t size) {
	if (index >= size) {
		kerrorf("Index %zu >= size %zu!", index, size);
		std::abort();
	}
}
