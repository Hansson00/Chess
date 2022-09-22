#include "Move_Generator.h"

Move_Generator::Move_Generator(){
	init_king_attacks();
	init_knight_attacks();
}

Move_Generator::~Move_Generator() {

}


uint16_t* Move_Generator::generate_pawn_moves(uint16_t* move_list, Position* pos, bool white) {
	const uint64_t pawns = white ? pos -> pieceBoards[1] : pos->pieceBoards[7];
	const uint64_t promoting = white ? pawns & ranks[1] : pawns & ranks[6];
	const uint64_t non_promoting = pawns & ~promoting;
	const uint64_t opponent = white ? pos -> teamBoards[2] : pos->teamBoards[1];
	const uint64_t non_occupied = ~(pos->teamBoards[0]);
	const uint64_t start_pawns = white ? ranks[5] : ranks[2]; //Where pawns that are able to move twice will be after one push

	uint64_t push = shift_up(non_promoting, white) & non_occupied;
	uint64_t double_push = shift_up(push & start_pawns, white) & non_occupied;

	const int back = white ? 8 : -8;
	while (push) {
		uint32_t dest = long_bit_scan(push);
		*move_list++ = (uint16_t)(dest | (dest + back) << 6);
		push &= push - 1;
	}

	while (double_push) {
		uint32_t dest = long_bit_scan(double_push);
		*move_list++ = (uint16_t)(dest | (dest + back + back) << 6 | 0x1000); //0x1000 flag for double push
		double_push &= double_push - 1;
	}

	uint64_t pawn_left = shift_side(non_promoting & ~files[0], false, white) & opponent;
	uint64_t pawn_right = shift_side(non_promoting & ~files[7], true, white) & opponent;
	const uint32_t back_left = white ? 7 : -9;
	const uint32_t back_right = white ? 9 : -7;

	while (pawn_left) {
		uint32_t dest = long_bit_scan(pawn_left);
		*move_list++ = (uint16_t)(dest | (dest + back_left) << 6 | 0x4000); //0x4000 flag for capture
		pawn_left &= pawn_left - 1;
	}

	while (pawn_right) {
		uint32_t dest = long_bit_scan(pawn_right);
		*move_list++ = (uint16_t)(dest | (dest + back_right) << 6 | 0x4000); //0x4000 flag for capture
		pawn_right &= pawn_right - 1;
	}
	
	if (pos->enPassant != 0) {
		uint64_t epPos = 1ULL << (pos->enPassant);
		//Reverse pawnAttack gives potential attackers of the ep square
		uint64_t attackers = pawn_attack(epPos, !white) & non_promoting & (ranks[4] | ranks[3]); //ranks is redundant I think
		while (attackers != 0) {
			int pawn = long_bit_scan(attackers);
			uint16_t move = (uint16_t)(pos -> enPassant | pawn << 6 | 0x5000); //0x5000 flag for ep capture
			*move_list++ = move;
			attackers &= attackers - 1;
		}
	}

	if (promoting != 0) {
		uint64_t promoPush = shift_up(promoting, white) & non_occupied;
		uint64_t promoCapLeft = shift_side(promoting & ~files[0], false, white) & opponent;
		uint64_t promoCapRight = shift_side(promoting & ~files[7], true, white) & opponent;
		while (promoPush != 0) {
			int dest = long_bit_scan(promoPush);
			move_list = add_promotion(move_list, (uint16_t)(dest | (dest - back) << 6 | 0x8000));
			promoPush &= promoPush - 1;
		}
		while (promoCapLeft != 0) {
			int dest = long_bit_scan(promoCapLeft);
			move_list = add_promotion(move_list, (uint16_t)(dest | (dest - back_left) << 6 | 0xC000));
			promoCapLeft &= promoCapLeft - 1;
		}
		while (promoCapRight != 0) {
			int dest = long_bit_scan(promoCapRight);
			move_list = add_promotion(move_list, (uint16_t)(dest | (dest - back_right) << 6 | 0xC000));
			promoCapRight &= promoCapRight - 1;
		}
	}
	return move_list;
}

uint16_t* Move_Generator::add_promotion(uint16_t* move_list, uint16_t move) {
	*move_list++ = move;
	*move_list++ = (uint16_t)(move | 0x1000);
	*move_list++ = (uint16_t)(move | 0x2000);
	*move_list++ = (uint16_t)(move | 0x3000);
	return move_list;
}

uint16_t* Move_Generator::generate_knight_moves(uint16_t* move_list, Position* pos, bool white) {
	const uint64_t team = white ? pos->teamBoards[1] : pos->teamBoards[2];
	const uint64_t board = pos->teamBoards[0];
	uint64_t knights = white ? pos->pieceBoards[2] : pos->pieceBoards[8];
	while (knights) {
		uint32_t knight = long_bit_scan(knights);
		uint64_t moves = knight_attacks[knight] & ~team;
		while (moves) {
			uint32_t dest = long_bit_scan(moves);
			const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
			*move_list++ = (uint16_t)(dest | knight << 6 | cap);
			moves &= moves - 1;
		}
		knights &= knights - 1;
	}
	return move_list;
}


uint16_t* Move_Generator::generate_king_moves(uint16_t* move_list, Position* pos, bool white) {
	const uint64_t team = white ? pos->teamBoards[1] : pos->teamBoards[2];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t not_attacked = white ? ~(pos->blackAttack) : ~(pos->whiteAttack);
	const uint64_t king = white ? pos->pieceBoards[0] : pos->pieceBoards[6];
	uint32_t k_pos = long_bit_scan(king);
	uint64_t moves = king_attacks[k_pos] & not_attacked & ~team;
	while (moves) {
		uint32_t dest = long_bit_scan(moves);
		const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
		*move_list++ = (uint16_t)(dest | k_pos << 6 | cap);
		moves &= moves - 1;
	}
	return move_list;
}

const uint64_t Move_Generator::shift_up(uint64_t pawns, bool white) {
	return white ? pawns >> 8 : pawns << 8;
}

const uint64_t Move_Generator::shift_side(uint64_t pawns, bool right, bool white) {
	return white ? (right ? pawns >> 7 : pawns >> 9) : (right ? pawns << 9 : pawns << 7);
}



const uint64_t Move_Generator::pawn_attack(uint64_t pawns, bool white) {
	return white ? ((pawns >> 9) & ~files[7] | (pawns >> 7) & ~files[0]) : 
		((pawns << 9) & ~files[7] | (pawns << 7) & ~files[0]);
}



//////////////////////////////////////////////
//Init attack arrays
//////////////////////////////////////////////
void Move_Generator::init_knight_attacks() {
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
		knight_attacks[i] = attack;
	}
}

void Move_Generator::init_king_attacks() {
	for (int i = 0; i < 64; i++) {
		uint64_t king = 1ULL << i;
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
}