/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_SETTINGS_H
#define _KERN_SETTINGS_H 1

#include <cstdlib>
#include <kernel/log.h>

template <typename T, bool changeable>
class Setting {
	public:
		Setting() : value(), initialized(false) {}
		explicit Setting(T &&value) : value(value), initialized(true) {}
		void initialize(T const &value) {
			if (initialized) {
				kCriticalNoAlloc("kernel/") << "Setting already initialized!";
				std::abort();
			}
			this->value = value;
			initialized=true;
		};
		T const &get() {
			if (initialized) {
				return value;
			}
			else {
				kCriticalNoAlloc("kernel/") << "Attempt to get value for uninitialized setting!";
				std::abort();
			}
		}
		bool set(T &value) requires(changeable) { this->value=value; initialized = true; }
		operator bool() const { return initialized; }

	private:
		T value;
		bool initialized = false;
};

namespace Settings {
	namespace PMM {
		inline Setting<unsigned long long, false> totalMem;
	};
	namespace Misc {
		inline Setting<KStringView, false> commandline;
	};
	namespace Logging {
		typedef void (*output_func)(const char*, size_t);
		inline Setting<output_func, true> critical;
		inline Setting<output_func, true> error;
		inline Setting<output_func, true> warning;
		inline Setting<output_func, true> log;
		inline Setting<output_func, true> debug;
	}
};

#endif /* _KERN_SETTINGS_H */
