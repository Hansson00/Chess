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

//Wrapper to easily keep track of the move_list and some useful functions
struct Move_list {
	uint16_t* start() { return move_list; };
	uint16_t* end() { return last; };
	uint16_t size() { return end() - start(); }

	uint16_t contains(const uint16_t move) {
		uint16_t xd = size();
		for (uint16_t* i = start(); i < end(); i++) {
			if ((*i & 0xFFF) == move)
				return *i; //Return the move with its flags
		}
		return 0; //Move not found
	}
	~Move_list() {
		//delete(move_list);
	}
	void clear() {last = move_list;}
	void add_move(const uint16_t move) { *last++ = move; }
	uint16_t to_sq(const uint16_t move) const { return move & 0x3F; }
	uint16_t from_sq(const uint16_t move) const { return (move >> 6) & 0x3F; }
	uint16_t flags(const uint16_t move) const { return move >> 12; }
	uint16_t move_list[60];
	uint16_t* last = move_list; //60 should be enough space for all moves
};

struct Position_list {
	Position_list* prev_positions;
	Position curr_pos;
	Position_list(Position_list* _prev, Position* _curr_pos){
		prev_positions = _prev;
		curr_pos = *_curr_pos;
	};
};