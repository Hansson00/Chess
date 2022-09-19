#include "Move_Generator.h"

Move_Generator::Move_Generator(){
	king_attacks = init_king_attacks();
	knight_attacks = init_knight_attacks();
}

Move_Generator::~Move_Generator() {

}


void Move_Generator::generate_pawn_moves(uint16_t* move_list, Position* pos, bool white) {
	const uint64_t pawns = white ? pos -> pieceBoards[1] : pos->pieceBoards[7];
	const uint64_t opponent = white ? pos -> teamBoards[2] : pos->teamBoards[1];
	const uint64_t board = pos->teamBoards[0];

	uint64_t push = shift_up(pawns, white) & ~board;

	const int back = white ? 8 : -8;
	while (push) {
		uint32_t pos = long_bit_scan(push);
		*move_list++ = (uint16_t)(pos | pos + back);
		push &= push - 1;
	}


}

const uint64_t Move_Generator::shift_up(uint64_t pawns, bool white) {
	return white ? pawns >> 8 : pawns << 8;
}



const uint64_t Move_Generator::pawn_attack(uint64_t pawns, bool white) {
	return white ? ((pawns >> 9) & ~files[7] | (pawns >> 7) & ~files[0]) : 
		((pawns << 9) & ~files[7] | (pawns << 7) & ~files[0]);
}



//////////////////////////////////////////////
//Init attack arrays
//////////////////////////////////////////////
uint64_t* Move_Generator::init_knight_attacks() {
	uint64_t knight_attack[64];
	for (int i = 0; i < 64; i++) {
		uint64_t knight = 1LL << i;
		uint64_t attack = (knight >> 10) & ~(files[6] | files[7] | ranks[7]); //Two files to left and up
		attack |= (knight << 6) & ~(files[6] | files[7] | ranks[0]); //Two files to left and down
		attack |= (knight >> 17) & ~(files[7] | ranks[6] | ranks[7]); //One file to left and up
		attack |= (knight << 15) & ~(files[7] | ranks[0] | ranks[1]); //One file to left and down
		attack |= (knight >> 15) & ~(files[0] | ranks[6] | ranks[7]); //One file to right and up
		attack |= (knight << 17) & ~(files[0] | ranks[0] | ranks[1]); //One file to right and down
		attack |= (knight >> 6) & ~(files[0] | files[1] | ranks[7]); //Two files to right and up
		attack |= (knight << 10) & ~(files[0] | files[1] | ranks[0]); //Two files to right and down
		knight_attack[i] = attack;
	}
	return knight_attack;
}

uint64_t* Move_Generator::init_king_attacks() {
	uint64_t king_attacks[64];
	for (int i = 0; i < 64; i++) {
		uint64_t king = 1LL << i;
		uint64_t attack = (king >> 9) & ~(files[7] | ranks[7]); //Diagonal left up
		attack |= (king >> 1) & ~(files[7]); //One step left
		attack |= (king << 7) & ~(files[7] | ranks[0]); //Diagonal left down
		attack |= (king >> 8) & ~(ranks[7]); //One step up
		attack |= (king << 8) & ~(ranks[0]); //One step down
		attack |= (king << 1) & ~(files[0]); //One step right
		attack |= (king >> 7) & ~(files[0] | ranks[7]); //Diagonal right up
		attack |= (king << 9) & ~(files[0] | ranks[0]); //Diagonal right down
		king_attacks[i] = attack;
	}
	return king_attacks;

}