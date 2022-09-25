#include "Move_Generator.h"

Move_Generator::Move_Generator(){
	init_king_attacks();
	init_knight_attacks();
	init_row_attacks();

	//arr[0]= &Move_Generator::generate_knight_moves;

	//(uint16_t* (Move_Generator:: * generate_knight_moves)(uint16_t *, Position *, bool))
	
	//uint16_t* (Move_Generator:: * generate_moves)(uint16_t * move_list, Position * pos, bool white)
}

Move_Generator::~Move_Generator() {

}

/*uint16_t* Move_Generator::generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos, bool white) {
	return (this->*arr[p])(move_list, pos, true);
}*/

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
		*move_list++ = (uint16_t)(dest | (dest + back_right) << 6 | 0x4000); //0x4000 flag for capture
		pawn_left &= pawn_left - 1;
	}

	while (pawn_right) {
		uint32_t dest = long_bit_scan(pawn_right);
		*move_list++ = (uint16_t)(dest | (dest + back_left) << 6 | 0x4000); //0x4000 flag for capture
		pawn_right &= pawn_right - 1;
	}
	
	if (pos->enPassant != 0) {
		uint64_t epPos = 1ULL << (pos->enPassant);
		//Reverse pawnAttack gives potential attackers of the ep square
		uint64_t attackers = pawn_attacks(epPos, !white) & non_promoting & (ranks[4] | ranks[3]); //ranks is redundant I think
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
	const uint8_t castle = white ? pos->castlingRights : (pos->castlingRights >> 2);
	uint32_t k_pos = long_bit_scan(king);
	if (k_pos == 64)
		return move_list;
	//Castling moves
	if (castle & 2) {
		const uint64_t part = 1ULL << (k_pos - 1) | 1ULL << (k_pos - 2);
		const uint64_t no_attack = part | (1ULL << k_pos);
		const uint64_t no_piece = part | (1ULL << (k_pos - 3));
		const bool is_ok = (board & no_piece) == 0 && (no_attack & not_attacked) == no_attack;
		if (is_ok)
			*move_list++ = (uint16_t)(k_pos - 2 | k_pos << 6 | 0x3000); //queen side castle
	}
	if (castle & 1) {
		const uint64_t no_piece = (1ULL << (k_pos + 1)) | (1ULL << (k_pos + 2));
		const uint64_t no_attack = no_piece | (1ULL << k_pos);
		const bool is_ok = (board & no_piece) == 0 && (no_attack & not_attacked) == no_attack;
		if (is_ok)
			*move_list++ = (uint16_t)(k_pos + 2 | k_pos << 6 | 0x2000); //king side castle
	}

	//Regular moves
	uint64_t moves = king_attacks[k_pos] & not_attacked & ~team;
	while (moves) {
		uint32_t dest = long_bit_scan(moves);
		const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
		*move_list++ = (uint16_t)(dest | k_pos << 6 | cap);
		moves &= moves - 1;
	}
	return move_list;
}

uint16_t* Move_Generator::generate_rook_moves(uint16_t* move_list, Position* pos, bool white) {
	uint64_t pieces = white ? pos->pieceBoards[4] : pos->pieceBoards[10];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	while (pieces) {
		const uint32_t rook = long_bit_scan(pieces);
		const uint64_t moves = rook_attacks(board, rook) & non_team;
		move_list = Move_Generator::generate_sliding_moves(move_list, board, moves, rook);
		pieces &= pieces - 1;
	}
	return move_list;
}

uint16_t* Move_Generator::generate_bishop_moves(uint16_t* move_list, Position* pos, bool white) {
	uint64_t pieces = white ? pos->pieceBoards[3] : pos->pieceBoards[9];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	while (pieces) {
		const uint32_t bishop = long_bit_scan(pieces);
		const uint64_t moves = 0ULL & non_team; //TODO implement bishop moves
		move_list = Move_Generator::generate_sliding_moves(move_list, board, moves, bishop);
		pieces &= pieces - 1;
	}
	return move_list;
}
uint16_t* Move_Generator::generate_queen_moves(uint16_t* move_list, Position* pos, bool white) {
	uint64_t pieces = white ? pos->pieceBoards[5] : pos->pieceBoards[11];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	while (pieces) {
		const uint32_t queen = long_bit_scan(pieces);
		const uint64_t moves = (rook_attacks(board, queen) | 0ULL) & non_team; //Needs bishop moves to work
		move_list = Move_Generator::generate_sliding_moves(move_list, board, moves, queen);
		pieces &= pieces - 1;
	}
	return move_list;
}

//Currently returns rook moves
uint16_t* Move_Generator::generate_sliding_moves(uint16_t* move_list, uint64_t board, uint64_t moves, uint32_t piece) {
	while (moves) {
		uint32_t dest = long_bit_scan(moves);
		const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
		*move_list++ = (uint16_t)(dest | piece << 6 | cap);
		moves &= moves - 1;
	}
	return move_list;
}



uint64_t Move_Generator::rook_attacks(uint64_t board, int rook_pos) {
	return rank_attacks(board, rook_pos) | file_attacks(board, rook_pos);
}


uint64_t Move_Generator::file_attacks(uint64_t board, int rook_pos) {
	int file = rook_pos % 8;
	int occupancy = (int)((file_to_rank(board, file) >> 1) & 0b111111);
	uint64_t fileAttack = rook_attack_table[rook_pos / 8][occupancy];
	fileAttack = rank_to_file(fileAttack, 0);
	return fileAttack >> (7 - file);
}

uint64_t Move_Generator::rank_attacks(uint64_t board, int rook_pos) {
	int column = rook_pos % 8;
	int shift = (rook_pos / 8) * 8;
	uint64_t occupancy = (board >> (shift + 1)) & 0x3F;
	return rook_attack_table[column][occupancy] << shift;
}

uint64_t Move_Generator::rank_to_file(uint64_t bitBoard, int rank) {
	bitBoard = (bitBoard >> rank * 8) & ranks[0];
	bitBoard = (((bitBoard * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101010101ULL) >> 56; //Mirror rank0
	bitBoard = bitBoard * main_diagonals[7]; //Flip to H-file
	return bitBoard & files[7]; //Remove junk
}

uint64_t Move_Generator::file_to_rank(uint64_t bitBoard, int file) {
	bitBoard = (bitBoard << (7 - file)) & files[7]; //Move specified file to H - file and mask junk
	bitBoard = bitBoard * shifted_anti_diagonal; //Multiply with shifted anti diagonal
	return bitBoard >> 56; //Move answer to rank0
}



const uint64_t Move_Generator::shift_up(uint64_t pawns, bool white) {
	return white ? pawns >> 8 : pawns << 8;
}

const uint64_t Move_Generator::shift_side(uint64_t pawns, bool right, bool white) {
	return white ? (right ? pawns >> 7 : pawns >> 9) : (right ? pawns << 9 : pawns << 7);
}



const uint64_t Move_Generator::pawn_attacks(uint64_t pawns, bool white) {
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


void Move_Generator::init_row_attacks() {
	//All possible positions on a rank
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 64; j++) {
			//Outer bits don't matter, therefore boardmask i.e occupancy only need 2^6 bits
			rook_attack_table[i][j] = calc_row_attack(j << 1, i);
		}
	}
}


//Used to initiate row attack matrix, is only based on first row since result can be shifted to desired row
uint64_t Move_Generator::calc_row_attack(uint64_t row_occupancy, int pos) {
	uint64_t attack = 0ULL;
	uint64_t sq;
	int iter = pos - 1;
	while (iter >= 0) {
		sq = 1ULL << iter;
		attack |= sq;
		if ((sq & row_occupancy) == sq)
			break;
		iter--;
	}
	iter = pos + 1;
	while (iter <= 7) {
		sq = 1ULL << iter;
		attack |= sq;
		if ((sq & row_occupancy) == sq)
			break;
		iter++;
	}
	return attack;
}