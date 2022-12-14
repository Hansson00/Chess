#pragma once
#include "sdl.h"
#include "string"

struct Position {
	uint64_t pieceBoards[12] = {};
	uint64_t teamBoards[3] = {};
	uint64_t whiteAttack = 0;
	uint64_t blackAttack = 0;
	uint64_t pinnedPieces = 0;
	uint64_t block_check = 0;
	uint64_t checker = 0;
	uint64_t moves = 0;
	bool whiteToMove = true;
	uint8_t castlingRights = 0;
	int numCheckers = 0;
	int enPassant = 0;
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


void parse_move(uint16_t);
uint32_t bit_scan(uint32_t);
uint32_t long_bit_scan(uint64_t);
uint32_t high_bit_scan(int32_t i);
uint32_t long_high_bit_scan(uint64_t i);
void print_bit_board(uint64_t b);
uint32_t bit_count(uint64_t i);

//Wrapper to easily keep track of the move_list and some useful functions
struct Move_list {
	uint32_t* start() { return move_list; };
	uint32_t* end() { return last; };
	uint32_t size() { return (uint32_t)(end() - start()); }

	void sort() {
		move_list;

		uint32_t* sort_left = start();
		uint32_t* sort_right = end();
		

		while (sort_left != sort_right)
		{
			while (sort_left != sort_right)
			{
				uint16_t flag = flags(*sort_left);
				if (flag & 4)
					sort_left++;
				else
					break;

			}
			while (sort_left != sort_right)
			{
				uint16_t flag = flags(*sort_right);
				if (flag & 4)
					break;
				else
					sort_right--;
			}
			if (sort_left != sort_right)
			{
				uint32_t temp = *sort_right;
				*sort_right = *sort_left;
				*sort_left = temp;
			}
		}
	}

	uint32_t contains(const uint32_t move) {
		for (uint32_t* i = start(); i < end(); i++) {
			if ((*i & 0xFFF) == move)
				return *i; //Return the move with its flags
		}
		return 0; //Move not found
	}
	const bool book_castle(const uint16_t move, const uint64_t king_board) {
		const int to_sq = move & 0x3F;
		const int from_sq = (move >> 6) & 0x3F;
		const int dist = abs(to_sq - from_sq);
		if (dist == 3 || dist == 4) {
			return king_board & (1ULL << from_sq);
		}
		return false;
	}
	~Move_list() {
		//delete(move_list);
	}
	void clear() {last = move_list;}
	void add_move(const uint32_t move) { *last++ = move; }
	uint16_t to_sq(const uint32_t move) const { return (uint32_t)(move & 0x3F); }
	uint16_t from_sq(const uint32_t move) const { return uint32_t((move >> 6) & 0x3F); }
	uint16_t flags(const uint32_t move) const { return (uint16_t)(move >> 12); }
	uint32_t move_list[100] = {};
	uint32_t* last = move_list; //60 should be enough space for all moves
};

struct Position_list {
	Position curr_pos;
	Position_list* prev_positions;
	Position_list(Position_list* _prev, Position* _curr_pos) {
		prev_positions = _prev;
		curr_pos = *_curr_pos;
	};
};


struct Best_move_ {
	uint32_t move;
	int eval;
	bool operator<(const Best_move_& other) const {
		return eval < other.eval;
	}
	bool operator>(const Best_move_& other) const {
		return eval > other.eval;
	}
	bool operator==(const Best_move_& other) const {
		return eval == other.eval;
	}
};