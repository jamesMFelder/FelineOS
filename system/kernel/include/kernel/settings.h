/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_SETTINGS_H
#define _KERN_SETTINGS_H 1

#include <kernel/log.h>

class SettingError {
	public:
		SettingError(KStringView message) : msg(message) {}
		KStringView what() const { return msg; }

	private:
		KStringView msg;
};

template <typename T, bool changeable> class Setting {
	public:
		Setting() : value(), initialized(false) {}
		explicit Setting(T &&value) : value(value), initialized(true) {}
		void initialize(T const &value) {
			if (initialized) {
				throw SettingError("Setting already initialized!"_kstr);
			}
			this->value = value;
			initialized = true;
		};
		T const &get() {
			if (initialized) {
				return value;
			} else {
				throw SettingError(
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
	_S(Settings::Logging::output_func, true, Logging, debug)

#define _S(type, modifiable, ns, name)                                         \
	namespace ns {                                                             \
	extern Setting<type, modifiable> name;                                     \
	};
SETTINGS_LIST(_S)
#undef _S

}; // namespace Settings

#endif /* _KERN_SETTINGS_H */
