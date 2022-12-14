#pragma once
#include "Chess_utility.h"
#include "Move_Generator.h"

class Evaluation : protected Move_Generator {
public:
	int Evaluate(Position* pos);




private:
	double eval_pawn_structure(Position* pos);
	int eval_piece(uint64_t piece_board, const int* heatmap);

	const int piece_value[6] = { 0, 1, 3, 3, 5, 10 };
	
	const uint64_t central_white_squares = 0x0004040400000000ULL |
		0x0008080800000000ULL | 0x0010101000000000ULL | 0x0020202000000000ULL;
	const uint64_t central_black_squares = 0x000000004040400ULL |
		0x000000008080800ULL | 0x0000000010101000ULL | 0x0000000020202000ULL;

	const int king_late_game[64] = 
	{
		-10, -10, -10, -10, -10, -10, -10, -10,
		-10, -5,  -5,  -5,   -5,  -5,  -5, -10,
		-10, -5,   5,   5,    5,   5,  -5, -10,
		-10, -5,   5,   5,    5,   5,  -5, -10,
		-10, -5,   5,   5,    5,   5,  -5, -10,
		-10, -5,   5,   5,    5,   5,  -5, -10,
		-10, -5,  -5,  -5,   -5,  -5,  -5, -10,
		-10, -10, -10, -10, -10, -10, -10, -10
	};

	const int empty[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

	const int w_king_heatmap[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		-2, -2, -2, -2, -2, -2, -2, -2,
		-2, -2, -2, -2, -2, -2, -2, -2,
		6, 9, 5, 0, 4, 0, 9, 6
	};

	const int b_king_heatmap[64] =
	{
		6, 9, 5, 0, 4, 0, 9, 6,
		-2, -2, -2, -2, -2, -2, -2, -2,
		-2, -2, -2, -2, -2, -2, -2, -2,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	const int w_pawn_heatmap[64] =
	{
		20, 20, 20, 20, 20, 20, 20, 20,
		10, 10, 10, 10, 10, 10, 10, 10,
		0,  3, -1,  5,  5,  -1,  3,  0,
		0,  1,  4,  6,  6,  4,  1,  0,
		0,  1,  5,  7,  7,  5,  1,  0,
		1,  3,  2,  3,  3,  2,  3,  1,
		2,  1,  0,  0,  0,  0,  1,  2,
		0,  0,  0,  0,  0,  0,  0,  0,
	};

	const int b_pawn_heatmap[64] =
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		2,  1,  0,  0,  0,  0,  1,  2,
		2,  3,  2,  3,  3,  2,  3,  2,
		0,  1,  5,  7,  7,  5,  1,  0,
		0,  1,  4,  6,  6,  4,  1,  0,
		0,  0,  3,  5,  5,  3,  0,  0,
		10, 10, 10, 10, 10, 10, 10, 10,
		20, 20, 20, 20, 20, 20, 20, 20,
	};

	const int w_rook_heatmap[64] =
	{
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  0,   0,  0,  0,  0,  0,  0,  0,
	  -2,  0,  4,  5,  5,  4,  0, -2,
	};

	const int b_rook_heatmap[64] =
	{
	  -2,  0,  4,  5,  5,  4,  0,  -2,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	   0,  0,  0,  0,  0,  0,  0,   0,
	};

	const int rook_late_game[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 5, 5, 5, 5, 5, 5, 0,
		0, 5, 0, 0, 0, 0, 5, 0,
		0, 5, 0, 0, 0, 0, 5, 0,
		0, 5, 0, 0, 0, 0, 5, 0,
		0, 5, 0, 0, 0, 0, 5, 0,
		0, 5, 5, 5, 5, 5, 5, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

	const int w_knight_heatmap[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

	const int b_knight_heatmap[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 5, 5, 5, 5, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
};


