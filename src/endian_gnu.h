#pragma once


template<typename T>
T bswap16(T x) {
	unsigned short y = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap32(T x) {
	unsigned long y = __builtin_bswap32(*reinterpret_cast<uint32_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap64(T x) {
	unsigned __int64 y = __builtin_bswap64(*reinterpret_cast<uint64_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}
