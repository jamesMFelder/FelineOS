/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#ifndef _ATOMIC_TPP
#define _ATOMIC_TPP 1

#include <atomic>

/* Construct the atomic object. These are not atomic!! */
/* Initialize to val */
template<typename T>
constexpr std::atomic<T>::atomic(T val) noexcept :value(val){}

/* Set to value. This is atomic. */
template<typename T>
T std::atomic<T>::operator=(T val) noexcept{
	__atomic_store_n(&value, val, __ATOMIC_SEQ_CST);
	return val;
}
template<typename T>
T std::atomic<T>::operator=(T val) volatile noexcept{
	__atomic_store_n(&value, val, __ATOMIC_SEQ_CST);
	return val;
}

/* Set a value */
template<typename T>
void std::atomic<T>::store(T val, memory_order sync) noexcept{
	__atomic_store_n(&value, val, sync);
}
template<typename T>
void std::atomic<T>::store(T val, memory_order sync) volatile noexcept{
	__atomic_store_n(&value, val, sync);
}

/* Read the value */
template<typename T>
T std::atomic<T>::load(memory_order sync) noexcept{
	return __atomic_load_n(&value, sync);
}
template<typename T>
T std::atomic<T>::load(memory_order sync) volatile noexcept{
	return __atomic_load_n(&value, sync);
}
template<typename T>
std::atomic<T>::operator T() const noexcept{
	return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
}
template<typename T>
std::atomic<T>::operator T() const volatile noexcept{
	return __atomic_load_n(&value, __ATOMIC_SEQ_CST);
}

/* Set and return the value atomically */
template<typename T>
T std::atomic<T>::exchange(T val, memory_order sync) noexcept{
	return __atomic_exchange_n(&value, val, sync);
}
template<typename T>
T std::atomic<T>::exchange(T val, memory_order sync) volatile noexcept{
	return __atomic_exchange_n(&value, val, sync);
}

/* Set to val if it equals expected, otherwise, set val to it */
/* These do a bitwise compare (not using operator==()) */
/* These next four may return false without modifying expected even if it equals val (may increase preformance) */
template<typename T>
bool std::atomic<T>::compare_exchange_weak(T& expected, T val, memory_order sync) noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, true, sync, sync);
}
template<typename T>
bool std::atomic<T>::compare_exchange_weak(T& expected, T val, memory_order sync) volatile noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, true, sync, sync);
}
template<typename T>
bool std::atomic<T>::compare_exchange_weak(T& expected, T val, memory_order success, memory_order failure) noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, true, success, failure);
}
template<typename T>
bool std::atomic<T>::compare_exchange_weak(T& expected, T val, memory_order success, memory_order failure) volatile noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, true, success, failure);
}
/* These must return true and modify expected if it equals val */
template<typename T>
bool std::atomic<T>::compare_exchange_strong(T& expected, T val, memory_order sync) noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, false, sync, sync);
}
template<typename T>
bool std::atomic<T>::compare_exchange_strong(T& expected, T val, memory_order sync) volatile noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, false, sync, sync);
}
template<typename T>
bool std::atomic<T>::compare_exchange_strong(T& expected, T val, memory_order success, memory_order failure) noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, false, success, failure);
}
template<typename T>
bool std::atomic<T>::compare_exchange_strong(T& expected, T val, memory_order success, memory_order failure) volatile noexcept{
	return __atomic_compare_exchange_n(&value, &expected, val, false, success, failure);
}

/* Is the object lock free? */
template<typename T>
bool std::atomic<T>::is_lock_free() const noexcept{
	return __atomic_is_lock_free(sizeof(T), 0);
}
template<typename T>
bool std::atomic<T>::is_lock_free() const volatile noexcept{
	return __atomic_is_lock_free(sizeof(T), 0);
}

#endif /* _ATOMIC_TPP */
