#pragma once
#include "sdl.h"
#include "string"

struct Position {
	uint64_t pieceBoards[12];
	uint64_t teamBoards[3];
	uint64_t whiteAttack;
	uint64_t blackAttack;
	uint64_t pinnedPieces;
	uint64_t validSq;
	bool whiteToMove;
	int castlingRights;
	int numCheckers;
	int enPassant;
};

uint32_t bit_scan(uint32_t);
uint32_t long_bit_scan(uint64_t);
uint32_t high_bit_scan(int32_t i);
uint32_t long_high_bit_scan(uint64_t i);
void print_bit_board(uint64_t b);