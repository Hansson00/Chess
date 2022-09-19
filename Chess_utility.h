#pragma once
#include "sdl.h"

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