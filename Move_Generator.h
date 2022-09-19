#pragma once
#include "sdl.h"
#include "Chess_utility.h"
class Move_Generator {
public:
	Move_Generator();
	~Move_Generator();

protected:
	uint64_t pawn_attacks(uint64_t);
	void generate_pawn_moves(uint16_t* move_list, Position* pos, bool white);
	const uint64_t shift_up(uint64_t pawns, bool white);
	const uint64_t pawn_attack(uint64_t pawns, bool white);
	uint64_t* init_knight_attacks();
	uint64_t* init_king_attacks();

private:
	uint64_t* knight_attacks;
	uint64_t* king_attacks;
	uint64_t files[8] = { 0x0101010101010101LL, 0x0202020202020202LL, 0x0404040404040404LL,
		0x0808080808080808LL, 0x1010101010101010LL, 0x2020202020202020LL, 0x4040404040404040LL,
		0x8080808080808080LL };
	uint64_t ranks[8] = { 0xFFLL, 0xFF00LL, 0xFF0000LL, 0xFF000000LL, 0xFF00000000LL, 0xFF0000000000LL,
			0xFF000000000000LL, 0xFF00000000000000LL };


};

