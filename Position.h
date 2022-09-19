#pragma once
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