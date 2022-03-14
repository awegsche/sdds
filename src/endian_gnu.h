#pragma once


template<typename T>
T bswap16(T x) {
	uint16_t y = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap32(T x) {
	uint32_t y = __builtin_bswap32(*reinterpret_cast<uint32_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}

template<typename T>
T bswap64(T x) {
	 uint64_t y = __builtin_bswap64(*reinterpret_cast<uint64_t*>(&x));
	return *reinterpret_cast<T*>(&y);
}
