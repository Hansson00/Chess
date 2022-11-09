#pragma once
#include "sdl.h"
#include "string"

struct Position {
	uint64_t pieceBoards[12];
	uint64_t teamBoards[3];
	uint64_t whiteAttack;
	uint64_t blackAttack;
	uint64_t pinnedPieces;
	uint64_t block_check;
	uint64_t checker;
	bool whiteToMove;
	uint8_t castlingRights;
	int numCheckers;
	int enPassant;
	//If this method fails to find mask it is used wrong or something is wrong in the code
	int find_mask(uint64_t sq, int start, int end) const {
		for (int i = start; i < end; i++) {
			if ((pieceBoards[i] & sq) != 0) {
				return i;
			}
		}
		//This should not happen
		return -1;
	}
};

uint32_t bit_scan(uint32_t);
uint32_t long_bit_scan(uint64_t);
uint32_t high_bit_scan(int32_t i);
uint32_t long_high_bit_scan(uint64_t i);
void print_bit_board(uint64_t b);