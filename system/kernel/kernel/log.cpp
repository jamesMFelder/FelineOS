/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cassert>
#include <cstdlib>
#include <feline/shortcuts.h>
#include <feline/str.h>
#include <kernel/log.h>
#include <kernel/settings.h>
#include <cstdarg>

static constexpr inline KStringView relative_path(std::source_location loc) {
	return &loc.file_name()[LOG_PATH_PREFIX_TRUNCATE_LEN];
}

kout::kout(log_level level, std::source_location loc, bool alloc) :
	alloc(alloc)
{
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
		*this << dec(loc.line()) << ':' << dec(loc.column()) << '(' << loc.function_name() << "): ";
	}
	else {
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
	//if we aren't allocating, everything has already been written
	if (alloc) {
		do_write();
	}
}

void kout::add_part(KStringView str) {
	//if we aren't allocating, write it out immediately (breaks atomic writes)
	//otherwise, save it for later
	if (alloc) {
		strings.append(str);
	}
	else {
		func(str.data(), str.size());
	}
}

void kout::do_write() {
	//if alloc is false, func should be called directly instead of calling this
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

kout& kout::operator<<(KStringView str) {
	add_part(str);
	return *this;
}

kout& kout::operator<<(KString const str) {
	add_part(KStringView(str.data(), str.size()));
	if (alloc) {
		lifetime_extender.append(str);
	}
	return *this;
}

kout& kout::operator<<(char c) {
	if (!alloc) {
		add_part(KStringView(&c, 1));
	}
	else {
		//Force KString to make an allocation, so it doesn't get clobbered
		KString temp_string;
		temp_string.append(c);
		return *this << temp_string;
	}
	return *this;
}

KString hex(uintmax_t n) {
	KString str;
	str.append('\0', 11);
	str[0]='0';
	str[1]='x';
	xtostr(n, str.data()+2);
	str.erase(end(str)-1, end(str));
	return str;
}

KString bin(uintmax_t n) {
	KString str;
	str.append('\0', 11);
	str[0]='0';
	str[1]='b';
	btostr(n, str.data()+2);
	str.erase(end(str)-1, end(str));
	return str;
}

KString dec(uintmax_t n) {
	KString str;
	str.append('\0', 9);
	ntostr(n, str.data(), 10);
	str.erase(end(str)-1, end(str));
	return str;
}

KString strDebug(KStringView str) {
	KString output;
	output.append("[len="_kstr_vec);
	output.append(dec(str.length()));
	output.append(",characters=\""_kstr_vec);
	KString safe_str;
	//we will need at least this much, so reserve it in advance
	safe_str.reserve(str.length());
	std::for_each(begin(str), end(str), [&safe_str](auto &c){
			if (c >= ' ' && c <= '~') {
				safe_str.append(c);
			}
			else {
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
						auto hex_str = hex(c);
						safe_str.append(begin(hex_str)+8, end(hex_str));
				}
			}
			});
	output.append(safe_str);
	output.append("\"]"_kstr_vec);
	return output;
}

/* Add a log. Logging functions should always use this so I can change the backend */
__attribute__((format (printf, 2, 0))) void __internal_log(const int level, const char *fmt, va_list data);

void __internal_log(const int level, const char *fmt, va_list data){

	if(level>3){
		kwarnf("Attempt to log at level %d.", level);
		return;
	}

	static const char *err_level[]={"CRIT", "ERR", "WARN", "LOG"};
	printf("%s %s: ", "Feline OS", err_level[level]);

	vprintf(fmt, data);

	putchar('\n');
	return;
}

/* For the moment, just write to terminal. */
__attribute__ ((format (printf, 1, 2))) void kerrorf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(1, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void kwarnf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(2, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void klogf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(3, format, data);
	va_end(data);
	return;
}

__attribute__ ((format (printf, 1, 2))) void kcriticalf(const char* format, ...){
	va_list data;
	va_start(data, format);
	__internal_log(0, format, data);
	va_end(data);
	return;
}
