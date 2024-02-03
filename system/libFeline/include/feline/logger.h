/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _FELINE_LOGGER_H
#define _FELINE_LOGGER_H 1

#include <cstdint>
#include <cstdlib>
#include <feline/kstring.h>
#include <source_location>
#include <type_traits>

enum log_level {
	critical,
	error,
	warning,
	log,
	debug,
};

/* Basic logging class styled like qDebug()
 * Not generally used, just a base for wrapper functions */
class kout {
	public:
		kout(log_level,
		     std::source_location loc = std::source_location::current(),
		     bool alloc = true);
		~kout();

		// output items
		kout &operator<<(KString const str);
		kout &operator<<(KStringView const str);
		kout &operator<<(char c);

	private:
		KVector<KStringView, KGeneralAllocator<KStringView>> strings;
		KVector<KString, KGeneralAllocator<KString>> lifetime_extender;
		void add_part(KStringView const str);
		void do_write();
		void (*func)(const char *, size_t);
		bool alloc;
};

/* Helper formatting functions intended to be used with kout */
KString hex(uintmax_t);
KString bin(uintmax_t);
KString dec(uintmax_t);

/* There is an operator<<(kout&, void*),
 * but it can be verbose with non-void pointers (especially char*) */
inline KString ptr(void const *ptr) {
	return hex(reinterpret_cast<uintptr_t>(ptr));
}

/* A macro for generating templates for formatting signed numbers
 * This works because it prints the minus sign before calling to the
 * unsigned version */
#define SIGNED_CONV(conv)                                                      \
	template <typename T>                                                      \
		requires std::is_signed_v<T>                                           \
	KString conv(T num) {                                                      \
		KString str;                                                           \
		if (num < 0)                                                           \
			str.append('-');                                                   \
		str += conv(static_cast<uintmax_t>(std::abs(num)));                    \
		return str;                                                            \
	}

SIGNED_CONV(hex);
SIGNED_CONV(dec);
SIGNED_CONV(bin);

#undef SIGNED_CONV

/* Make it easy to take an rvalue and lvalue with one definition.
 * It just calls the lvalue version. */
#define RVALUE_OVERLOAD(type)                                                  \
	inline kout &operator<<(kout &&out, type ptr) { return out << ptr; }

/* Various helper functions for formatting to a kout */
inline kout &operator<<(kout &out, char const *str) {
	return out << KStringView(str, strlen(str));
}
RVALUE_OVERLOAD(char const *)

inline kout &operator<<(kout &out, void *ptr) {
	return out << hex(reinterpret_cast<uintptr_t>(ptr));
}
RVALUE_OVERLOAD(void *)

#undef RVALUE_OVERLOAD

KString strDebug(KStringView);

/* A macro for generating inline wrappers for creating a kout object
 * The first compile-time function call must have the source_location
 * stuff, but it's a lot to type out for every variation.
 * Don't call this macro directly! */
#define GEN_LOG_INLINE(name, level, alloc)                                     \
	inline kout name(std::source_location location =                           \
	                     std::source_location::current()) {                    \
		return kout(log_level::level, location, alloc);                        \
	}

/* Basic wrappers over creating a kout object */
GEN_LOG_INLINE(kCritical, critical, true);
GEN_LOG_INLINE(kError, error, true);
GEN_LOG_INLINE(kWarning, warning, true);
GEN_LOG_INLINE(kLog, log, true);
GEN_LOG_INLINE(kDbg, debug, true);

/* Same as above, except without doing any allocations!
 * This was meant for (de)allocation code, but you still
 * to format addresses and numbers. */
GEN_LOG_INLINE(kCriticalNoAlloc, critical, false);
GEN_LOG_INLINE(kDbgNoAlloc, debug, false);

#undef GEN_LOG_INLINE

#endif /* _FELINE_LOGGER_H */
