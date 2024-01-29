/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/settings.h>

namespace Settings {
#define _S(type, modifiable, ns, name)                                         \
	namespace ns {                                                             \
	Setting<type, modifiable> name;                                            \
	};
SETTINGS_LIST(_S)
#undef _S
} // namespace Settings
