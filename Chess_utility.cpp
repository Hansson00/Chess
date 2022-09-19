#include "Chess_utility.h"

uint32_t bit_scan(uint32_t i) {
	i = ~i & (i - 1);
	if (i <= 0) return i & 32;
	uint32_t n = 1;
	if (i > 1 << 16) { n += 16; i >>= 16; }
	if (i > 1 << 8) { n += 8; i >>= 8; }
	if (i > 1 << 4) { n += 4; i >>= 4; }
	if (i > 1 << 2) { n += 2; i >>= 2; }
	return n + (i >> 1);
}

uint32_t long_bit_scan(uint64_t i) {
	uint32_t x = (uint32_t)i;
	return x == 0 ? 32 + bit_scan((uint32_t)(i >> 32))
		: bit_scan(x);
}