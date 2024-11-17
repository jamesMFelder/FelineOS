/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#include <cassert>
#include <cctype>
#include <feline/logger.h>
#include <feline/settings.h>
#include <feline/shortcuts.h>
#include <feline/str.h>

static inline KStringView relative_path(std::source_location loc) {
	return &loc.file_name()[LOG_PATH_PREFIX_TRUNCATE_LEN];
}

kout::kout(log_level level, std::source_location loc, bool alloc)
	: alloc(alloc) {
	switch (level) {
	case critical:
		func = Settings::Logging::critical.get();
		add_part("FOS Critical ");
		break;
	case error:
		func = Settings::Logging::error.get();
		add_part("FOS Error ");
		break;
	case warning:
		func = Settings::Logging::warning.get();
		add_part("FOS Warning ");
		break;
	case log:
		func = Settings::Logging::log.get();
		add_part("FOS Log ");
		break;
	case debug:
		func = Settings::Logging::debug.get();
		add_part("FOS Debug ");
		break;
	}
	*this << relative_path(loc) << ':';

	if (alloc) {
		*this << dec(loc.line()) << ':' << dec(loc.column()) << '('
			  << loc.function_name() << "): ";
	} else {
		char buf[9];
		itostr(loc.line(), buf);
		*this << buf << ':';

		itostr(loc.column(), buf);
		*this << buf << '(';

		*this << loc.function_name() << "): ";
	}
};

kout::~kout() {
	add_part("\n"_kstr);
	// if we aren't allocating, everything has already been written
	if (alloc) {
		do_write();
	}
}

void kout::add_part(KStringView const str) {
	// if we aren't allocating, write it out immediately (breaks atomic writes)
	// otherwise, save it for later
	if (alloc) {
		strings.append(str);
	} else {
		func(str.data(), str.size());
	}
}

void kout::do_write() {
	// if alloc is false, func should be called directly instead of calling this
	assert(alloc);
	size_t total_length = 0;
	for (auto &str : strings) {
		total_length += str.length();
	}
	// char full_string[total_length];
	KString full_string;
	full_string.append('\0', total_length);
	size_t current_index = 0;
	for (auto &str : strings) {
		std::copy(begin(str), end(str), &full_string[current_index]);
		current_index += str.length();
	}
	func(full_string.data(), full_string.size());
	return;
}

kout &kout::operator<<(KStringView const str) {
	add_part(str);
	return *this;
}

kout &kout::operator<<(KString const str) {
	if (alloc) {
		lifetime_extender.append(std::move(str));
		/* We need to use a reference to the data in lifetime_extender, since it
		 * is moved from str (which otherwise gets destructed at the end of this
		 * function)
		 */
		add_part(KStringView(end(lifetime_extender)[-1].data(),
		                     end(lifetime_extender)[-1].size()));
	} else {
		add_part(KStringView(str.data(), str.size()));
	}
	return *this;
}

kout &kout::operator<<(char c) {
	if (!alloc) {
		add_part(KStringView(&c, 1));
	} else {
		// Force KString to make an allocation, so it doesn't get clobbered
		KString temp_string;
		temp_string.append(c);
		return *this << temp_string;
	}
	return *this;
}

KString hex(uintmax_t n) {
	KString str;
	str.append('0');
	str.append('x');
	str.append(xtostr(n));
	return str;
}

KString bin(uintmax_t n) {
	KString str;
	str.append('0');
	str.append('b');
	str.append(btostr(n));
	return str;
}

KString dec(uintmax_t n) { return itostr(n); }

KString strDebug(KStringView str) {
	KString output;
	output.append("[len="_kstr_vec);
	output.append(dec(str.length()));
	output.append(",characters=\""_kstr_vec);
	KString safe_str;
	// we will need at least this much, so reserve it in advance
	safe_str.reserve(str.length());
	std::for_each(begin(str), end(str), [&safe_str](auto &c) {
		if (isprint(c)) {
			safe_str.append(c);
		} else {
			switch (c) {
			case '\0':
				safe_str.append("\\0"_kstr_vec);
				break;
			case '\t':
				safe_str.append("\\t"_kstr_vec);
				break;
			case '\r':
				safe_str.append("\\r"_kstr_vec);
				break;
			case '\n':
				safe_str.append("\\n"_kstr_vec);
				break;
			case '\033':
				safe_str.append("\\e"_kstr_vec);
				break;
			default:
				safe_str.append("\\x"_kstr_vec);
				safe_str.append(xtostr(c, false));
			}
		}
	});
	output.append(safe_str);
	output.append("\"]"_kstr_vec);
	return output;
}
