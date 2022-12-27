#include "Chess_utility.h"
#include <string>
#include <iostream>

/*
namespace std {
	template<>
	struct hash<Position> {
		size_t operator()(const Position& key) {

			uint64_t temp = 0;
			for (int i = 0; i < 12; i++) {
				temp = temp ^ key.pieceBoards[i];
			}
			temp = temp ^ key.castlingRights;
			return hash<uint64_t>()(temp);
		}
	};
}*/

uint32_t bit_count(uint64_t i) {
	// HD, Figure 5-2
	i = i - ((i >> 1) & 0x5555555555555555ULL);
	i = (i & 0x3333333333333333ULL) + ((i >> 2) & 0x3333333333333333ULL);
	i = (i + (i >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
	i = i + (i >> 8);
	i = i + (i >> 16);
	i = i + (i >> 32);
	return (uint32_t)i & 0x7f;
}

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

uint32_t high_bit_scan(int32_t i) {
	if (i <= 0)
		return i == 0 ? 32 : 0;
	int32_t n = 31;
	if (i >= 1 << 16) { n -= 16; i >>= 16; }
	if (i >= 1 << 8) { n -= 8; i >>=  8; }
	if (i >= 1 << 4) { n -= 4; i >>=  4; }
	if (i >= 1 << 2) { n -= 2; i >>=  2; }
	return n - (i >> 1);
}

uint32_t long_high_bit_scan(uint64_t i) {
	int32_t x = (int32_t)(i >> 32);
	return x == 0 ? 32 + high_bit_scan((int32_t)i)
		: high_bit_scan(x);
}



uint32_t long_bit_scan(uint64_t i) {
	uint32_t x = (uint32_t)i;
	return x == 0 ? 32 + bit_scan((uint32_t)(i >> 32))
		: bit_scan(x);
}


void print_bit_board(uint64_t b) {
	std::string stb = "";
	for (int i = 0; i < 64; i++) {
		stb += (b & 1) == 1 ? 'x' : '0';
		if (i % 8 == 7){
			std::cout << stb;
			std::cout << "\n";
			stb = "";
		}
		b >>= 1;
	}
	std::cout << "\n\n";
}

void parse_move(uint16_t move) {
	int from = (move >> 6) & 0x3F;
	int to = move & 0x3F;
	std::cout << (char)(from % 8 + 'a') << (char)('8' - from / 8)
		<< (char)('a' + to % 8) << (char)('8' - to / 8) << std::endl;
}
