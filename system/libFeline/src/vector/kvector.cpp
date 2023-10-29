#include <feline/kvector.h>
#include <kernel/log.h>

void check_index(size_t index, size_t size) {
	if (index >= size) {
		kError("libfeline/") << "Index " << dec(index) << " >= size " << dec(size);
		std::abort();
	}
}
