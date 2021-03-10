#pragma once
#include <intrin.h>

template<typename T>
T bswap16(T x) {
	unsigned short y = _byteswap_ushort(*reinterpret_cast<unsigned short*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap32(T x) {
	unsigned long y = _byteswap_ulong(*reinterpret_cast<unsigned long*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap64(T x) {
	unsigned __int64 y = _byteswap_uint64(*reinterpret_cast<unsigned __int64*>(&x));
	return *reinterpret_cast<T*>(&y);
}
