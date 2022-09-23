#pragma once
#include "sdl.h"
#include "Chess_utility.h"

typedef void(*FunctionPointer)();

class Move_Generator {
public:
	Move_Generator();
	~Move_Generator();
	//uint16_t* generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos ,uint64_t mask, bool white);


	uint16_t* generate_pawn_moves(uint16_t* move_list, Position* pos, bool white);
	uint16_t* generate_knight_moves(uint16_t* move_list, Position* pos, bool white);
	uint16_t* generate_king_moves(uint16_t* move_list, Position* pos, bool white);
protected:
	uint16_t* add_promotion(uint16_t* move_list, uint16_t move);
	const uint64_t shift_up(uint64_t pawns, bool white);
	const uint64_t shift_side(uint64_t pawns, bool right, bool white);
	const uint64_t pawn_attacks(uint64_t pawns, bool white);
	void init_knight_attacks();
	void init_king_attacks();
	uint64_t king_attacks[64];

private:

	//uint16_t* (Move_Generator::* arr[6])(uint16_t*, Position*, bool) = { &Move_Generator::generate_king_moves, &Move_Generator::generate_pawn_moves, &Move_Generator::generate_knight_moves };
	// = {&generate_king_moves, &generate_pawn_moves ,&generate_knight_moves, &generate_pawn_moves, &generate_pawn_moves, &generate_pawn_moves};

	uint64_t knight_attacks[64];
	uint64_t files[8] = { 0x0101010101010101LL, 0x0202020202020202LL, 0x0404040404040404LL,
		0x0808080808080808LL, 0x1010101010101010LL, 0x2020202020202020LL, 0x4040404040404040LL,
		0x8080808080808080LL };
	uint64_t ranks[8] = { 0xFFLL, 0xFF00LL, 0xFF0000LL, 0xFF000000LL, 0xFF00000000LL, 0xFF0000000000LL,
			0xFF000000000000LL, 0xFF00000000000000LL };


};

