/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_SETTINGS_H
#define _FELINE_SETTINGS_H 1

#include <feline/kernel_exceptions.h>
#include <feline/logger.h>

template <typename T, bool changeable> class Setting {
	public:
		Setting() : value(), initialized(false) {}
		explicit Setting(T &&value) : value(value), initialized(true) {}
		void initialize(T const &value) {
			if (initialized) {
				report_fatal_error("Setting already initialized!"_kstr);
			}
			this->value = value;
			initialized = true;
		};
		T const &get() {
			if (initialized) {
				return value;
			} else {
				report_fatal_error(
					"Attempt to get value for uninitialized setting!"_kstr);
			}
		}
		T &get()
			requires(changeable)
		{
			if (initialized) {
				return value;
			} else {
				report_fatal_error(
					"Attempt to get value for uninitialized setting!"_kstr);
			}
		}
		void set(T const &value)
			requires(changeable)
		{
			this->value = value;
			initialized = true;
		}
		operator bool() const { return initialized; }

	private:
		T value;
		bool initialized = false;
};

namespace Settings {
namespace Logging {
typedef void (*output_func)(const char *, size_t);
}

#define SETTINGS_LIST(_S)                                                      \
	_S(KStringView, false, Misc, commandline)                                  \
	_S(Settings::Logging::output_func, true, Logging, critical)                \
	_S(Settings::Logging::output_func, true, Logging, error)                   \
	_S(Settings::Logging::output_func, true, Logging, warning)                 \
	_S(Settings::Logging::output_func, true, Logging, log)                     \
	_S(Settings::Logging::output_func, true, Logging, debug)                   \
	_S(unsigned long long, true, Time, ms_since_boot)

#define _S(type, modifiable, ns, name)                                         \
	namespace ns {                                                             \
	extern Setting<type, modifiable> name;                                     \
	};
SETTINGS_LIST(_S)
#undef _S

}; // namespace Settings

#endif /* _FELINE_SETTINGS_H */
