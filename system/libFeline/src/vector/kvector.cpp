#include <feline/kvector.h>
#ifndef LIBFELINE_ONLY
#include <kernel/log.h>
#else
#include <iostream>
#endif

void check_index(size_t index, size_t size) {
	if (index >= size) {
#ifndef LIBFELINE_ONLY
		kError() << "Index " << dec(index) << " >= size " << dec(size);
#else
		std::cerr << "Index " << index << " >= size " << size << std::endl;
#endif
		std::abort();
	}
}
