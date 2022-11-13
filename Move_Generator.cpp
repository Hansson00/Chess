#include "Move_Generator.h"

Move_Generator::Move_Generator(){
	init_king_attacks();
	init_knight_attacks();
	init_row_attacks();
	init_maindiagonal_attacks();
	init_antidiagonal_attacks();

	//arr[0]= &Move_Generator::generate_knight_moves;

	//(uint16_t* (Move_Generator:: * generate_knight_moves)(uint16_t *, Position *, bool))
	
	//uint16_t* (Move_Generator:: * generate_moves)(uint16_t * move_list, Position * pos, bool white)
}

Move_Generator::~Move_Generator() {

}

/*uint16_t* Move_Generator::generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos, bool white) {
	return (this->*arr[p])(move_list, pos, true);
}*/

void Move_Generator::generate_pawn_moves(Move_list* move_list, Position* pos) {
	const bool white_to_move = pos->whiteToMove;
	const uint64_t pawns = white_to_move ? pos -> pieceBoards[1] : pos->pieceBoards[7];
	const uint64_t promoting = white_to_move ? pawns & ranks[1] : pawns & ranks[6];
	const uint64_t non_promoting = pawns & ~promoting;
	const uint64_t opponent = white_to_move ? pos -> teamBoards[2] : pos->teamBoards[1];
	const uint64_t non_occupied = ~(pos->teamBoards[0]);
	const uint64_t start_pawns = white_to_move ? ranks[5] : ranks[2]; //Where pawns that are able to move twice will be after one push
	const uint64_t checker = pos->checker;
	const uint64_t block = pos->numCheckers > 0 ? pos->block_check : ~0ULL;
	const uint32_t piece_id = white_to_move ? 1ULL << 16 : 7ULL << 16;
	

	uint64_t push = shift_up(non_promoting, white_to_move) & non_occupied;
	uint64_t double_push = shift_up(push & start_pawns, white_to_move) & non_occupied & block;
	push &= block;

	const int back = white_to_move ? 8 : -8;
	while (push) {
		uint32_t dest = long_bit_scan(push);
		move_list->add_move((uint32_t)(piece_id | dest | (dest + back) << 6));
		push &= push - 1;
	}

	while (double_push) {
		uint32_t dest = long_bit_scan(double_push);
		move_list->add_move((uint32_t)(piece_id | dest | (dest + back + back) << 6 | 0x1000)); //0x1000 flag for double push
		double_push &= double_push - 1;
	}

	uint64_t pawn_left = shift_side(non_promoting & ~files[0], false, white_to_move) & opponent & checker;
	uint64_t pawn_right = shift_side(non_promoting & ~files[7], true, white_to_move) & opponent & checker;
	const uint32_t back_left = white_to_move ? 7 : -9;
	const uint32_t back_right = white_to_move ? 9 : -7;

	while (pawn_left) {
		uint32_t dest = long_bit_scan(pawn_left);
		move_list->add_move((uint32_t)(piece_id | dest | (dest + back_right) << 6 | 0x4000)); //0x4000 flag for capture
		pawn_left &= pawn_left - 1;
	}

	while (pawn_right) {
		uint32_t dest = long_bit_scan(pawn_right);
		move_list->add_move((uint32_t)(piece_id | dest | (dest + back_left) << 6 | 0x4000)); //0x4000 flag for capture
		pawn_right &= pawn_right - 1;
	}
	
	
	if (pos->enPassant != 0xFF && (block == ~0ULL || block == 0)) {
		const int pseudo = pos->enPassant & 0xFF;
		uint64_t epPos = (1ULL << pseudo);
		//Reverse pawnAttack gives potential attackers of the ep square
		uint64_t attackers = pawn_attacks(epPos, !white_to_move) & non_promoting; //ranks is redundant I think
		while (attackers != 0) {
			int pawn = long_bit_scan(attackers);
			uint32_t move = (uint32_t)(piece_id | pseudo | pawn << 6 | 0x5000); //0x5000 flag for ep capture
			move_list->add_move(move);
			attackers &= attackers - 1;
		}
	}

	if (promoting != 0) {
		uint64_t promoPush = shift_up(promoting, white_to_move) & non_occupied & block;
		uint64_t promoCapLeft = shift_side(promoting & ~files[0], false, white_to_move) & opponent & checker;
		uint64_t promoCapRight = shift_side(promoting & ~files[7], true, white_to_move) & opponent & checker;
		while (promoPush != 0) {
			int dest = long_bit_scan(promoPush);
			add_promotion(move_list, (uint32_t)(piece_id | dest | (dest + back) << 6 | 0x8000));
			promoPush &= promoPush - 1;
		}
		while (promoCapLeft != 0) {
			int dest = long_bit_scan(promoCapLeft);
			add_promotion(move_list, (uint32_t)(piece_id | dest | (dest + back_right) << 6 | 0xC000));
			promoCapLeft &= promoCapLeft - 1;
		}
		while (promoCapRight != 0) {
			int dest = long_bit_scan(promoCapRight);
			add_promotion(move_list, (uint32_t)(piece_id | dest | (dest + back_left) << 6 | 0xC000));
			promoCapRight &= promoCapRight - 1;
		}
	}
}

void Move_Generator::add_promotion(Move_list* move_list, uint32_t move) {
	move_list->add_move((uint32_t)(move | 0x3000));
	move_list->add_move(move);
	move_list->add_move((uint32_t)(move | 0x1000));
	move_list->add_move((uint32_t)(move | 0x2000));
}


void Move_Generator::generate_knight_moves(Move_list* move_list, Position* pos) {
	const bool whiteToMove = pos->whiteToMove;
	const uint64_t team = whiteToMove ? pos->teamBoards[1] : pos->teamBoards[2];
	const uint64_t board = pos->teamBoards[0];
	const uint32_t piece_id = whiteToMove ? 2ULL << 16 : 8ULL << 16;
	uint64_t knights = whiteToMove ? pos->pieceBoards[2] : pos->pieceBoards[8];
	
	//Checks, maybe solving checks with a template would be nice
	const uint64_t block = pos->numCheckers > 0 ? pos->block_check | pos->checker : ~0ULL;
	
	
	while (knights) {
		uint32_t knight = long_bit_scan(knights);
		uint64_t moves = knight_attack[knight] & ~team & block;
		while (moves) {
			uint32_t dest = long_bit_scan(moves);
			const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
			move_list->add_move((uint32_t)(piece_id | dest | knight << 6 | cap));
			moves &= moves - 1;
		}
		knights &= knights - 1;
	}
}

void Move_Generator::generate_king_moves(Move_list* move_list, Position* pos) {
	const bool white_to_move = pos->whiteToMove;
	const uint64_t team = white_to_move ? pos->teamBoards[1] : pos->teamBoards[2];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t not_attacked = white_to_move ? ~(pos->blackAttack) : ~(pos->whiteAttack);
	const uint64_t king = white_to_move ? pos->pieceBoards[0] : pos->pieceBoards[6];
	const uint8_t castle = white_to_move ? pos->castlingRights : (pos->castlingRights >> 2);
	const uint32_t piece_id = white_to_move ? 0 : 6ULL << 16;
	uint32_t k_pos = long_bit_scan(king);
	//Castling moves
	if (castle & 2) {
		const uint64_t part = 1ULL << (k_pos - 1) | 1ULL << (k_pos - 2);
		const uint64_t no_attack = part | (1ULL << k_pos);
		const uint64_t no_piece = part | (1ULL << (k_pos - 3));
		const bool is_ok = (board & no_piece) == 0 && (no_attack & not_attacked) == no_attack;
		if (is_ok)
			move_list->add_move((uint32_t)(piece_id | k_pos - 2 | k_pos << 6 | 0x3000)); //queen side castle
	}
	if (castle & 1) {
		const uint64_t no_piece = (1ULL << (k_pos + 1)) | (1ULL << (k_pos + 2));
		const uint64_t no_attack = no_piece | (1ULL << k_pos);
		const bool is_ok = (board & no_piece) == 0 && (no_attack & not_attacked) == no_attack;
		if (is_ok)
			move_list->add_move((uint32_t)(piece_id | k_pos + 2 | k_pos << 6 | 0x2000)); //king side castle
	}

	//Regular moves
	uint64_t moves = king_attacks[k_pos] & not_attacked & ~team;
	while (moves) {
		uint32_t dest = long_bit_scan(moves);
		const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures
		move_list->add_move((uint32_t)(piece_id | dest | k_pos << 6 | cap));
		moves &= moves - 1;
	}
}

void Move_Generator::generate_rook_moves(Move_list* move_list, Position* pos) {
	const bool white_to_move = pos->whiteToMove;
	uint64_t pieces = white_to_move ? pos->pieceBoards[4] : pos->pieceBoards[10];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white_to_move ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	const uint32_t piece_id = white_to_move ? 4ULL << 10 : 10ULL << 10;
	//Checks, maybe solving checks with a template would be nice
	const uint64_t block = pos->numCheckers > 0 ? pos->block_check | pos->checker : ~0ULL;
	
	while (pieces) {
		const uint32_t rook = long_bit_scan(pieces);
		const uint64_t moves = rook_attacks(board, rook) & non_team & block;
		generate_sliding_moves(move_list, board, moves, rook | piece_id);
		pieces &= pieces - 1;
	}
}

void Move_Generator::generate_bishop_moves(Move_list* move_list, Position* pos) {
	const bool white_to_move = pos->whiteToMove;
	uint64_t pieces = white_to_move ? pos->pieceBoards[3] : pos->pieceBoards[9];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white_to_move ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	const uint32_t piece_id = white_to_move ? 3ULL << 10 : 9ULL << 10;
	
	//Checks, maybe solving checks with a template would be nice
	const uint64_t block = pos->numCheckers > 0 ? pos->block_check | pos->checker : ~0ULL;
	
	while (pieces) {
		const uint32_t bishop = long_bit_scan(pieces);
		const uint64_t moves = bishop_attacks(board, bishop) & non_team & block;
		generate_sliding_moves(move_list, board, moves, bishop | piece_id);
		pieces &= pieces - 1;
	}
}

void Move_Generator::generate_queen_moves(Move_list* move_list, Position* pos) {
	const bool white_to_move = pos->whiteToMove;
	uint64_t pieces = white_to_move ? pos->pieceBoards[5] : pos->pieceBoards[11];
	const uint64_t board = pos->teamBoards[0];
	const uint64_t non_team = white_to_move ? ~(pos->teamBoards[1]) : ~(pos->teamBoards[2]);
	const uint32_t piece_id = white_to_move ? 5ULL << 10 : 11ULL << 10;
	
	//Checks, maybe solving checks with a template would be nice
	const uint64_t block = pos->numCheckers > 0 ? pos->block_check | pos->checker : ~0ULL;
	
	while (pieces) {
		const uint32_t queen = long_bit_scan(pieces);
		const uint64_t moves = queen_attacks(board, queen) & non_team & block;
		generate_sliding_moves(move_list, board, moves, queen | piece_id);
		pieces &= pieces - 1;
	}
}

//Since sliding move generation is so similar this help method is used
void Move_Generator::generate_sliding_moves(Move_list* move_list, uint64_t board, uint64_t moves, uint32_t piece) {
	//Piece contains moving piece_id aswell
	while (moves) {
		uint32_t dest = long_bit_scan(moves);
		const uint16_t cap = ((board & (1ULL << dest)) == 0) ? 0 : 0x4000; //intersection with board gives captures

		move_list->add_move((uint32_t)(dest | piece << 6 | cap));
		moves &= moves - 1;
	}
}





uint64_t Move_Generator::knight_attacks(uint64_t board, int pos) {
	return knight_attack[pos];
}

uint64_t Move_Generator::rook_attacks(uint64_t board, int rook_pos) {
	return rank_attacks(board, rook_pos) | file_attacks(board, rook_pos);
}

uint64_t Move_Generator::bishop_attacks(uint64_t board, int bishop_pos) {
	return maindiagonal_attacks(board, bishop_pos) | antidiagonal_attacks(board, bishop_pos);
}

uint64_t Move_Generator::queen_attacks(uint64_t board, int queen_pos) {
	return Move_Generator::rook_attacks(board, queen_pos) | Move_Generator::bishop_attacks(board, queen_pos);
}

uint64_t Move_Generator::piece_attacks(uint64_t board, uint64_t pieces, int piece_type) {
	uint64_t attack = 0ULL;
	while (pieces) {
		int pos = long_bit_scan(pieces);
		attack |= (this->*attack_function[piece_type])(board, pos);
		pieces &= pieces - 1;
	}
	return attack;
}

uint64_t Move_Generator::maindiagonal_attacks(uint64_t board, int bishop_pos) {
	int offset = (bishop_pos % 8 - bishop_pos / 8);
	int index = 7 + offset;
	uint64_t diagonal = main_diagonals[index] & board;
	//Mask relevant bits
	int shift = index < 8 ? 2 : index - 5;
	//Shift down to get occupancy bits as LSBs
	int occupancy = (int)((diagonal * files[1]) >> (56 + shift));
	//Mask edge bits for corresponding diagonal
	occupancy = occupancy & mask_bits[index];
	//Fetch attack based on diagonal, position (column) and occupancy
	return bishop_main[index][(offset > 0 ? bishop_pos % 8 - offset : bishop_pos % 8)][occupancy];
}

uint64_t Move_Generator::antidiagonal_attacks(uint64_t board, int bishop_pos) {
	//On a given anti diagonal, sum of column and row is equal for all squares => can be indexed by the sum
	int index = bishop_pos / 8 + bishop_pos % 8;
	//Occupied bits of relevant diagonal
	uint64_t diagonal = anti_diagonals[index] & board;
	//To get occupancy bits we will need to know how much to shift
	int shift = index < 8 ? 1 : index - 6;
	//Rotate diagonal to rank
	int occupancy = (int)((diagonal * files[0]) >> (56 + shift));
	//Get inner bits
	occupancy = occupancy & mask_bits[index];
	//Attack based on which diagonal, which position on the diagonal, and occupancy of the diagonal
	return bishop_anti[index][(index < 8 ? bishop_pos % 8 : bishop_pos % 8 - shift + 1)][occupancy];
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
		((pawns << 9) & ~files[0] | (pawns << 7) & ~files[7]);
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
		knight_attack[i] = attack;
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
	for (uint32_t i = 0; i < 8; i++) {
		for (uint32_t j = 0; j < 64; j++) {
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
//Conventional looping algo to calculate attacks an maindiagonal
uint64_t Move_Generator::calc_maindiagonal_attack(uint64_t board_occupancy, int pos) {
	uint64_t attack = 0ULL;
	uint64_t sq;
	int prev = pos % 8;
	int temp_pos = pos + 9;
	int next = temp_pos % 8;
	while (abs(next - prev) < 2 && (temp_pos & ~63) == 0) {
		sq = 1ULL << temp_pos;
		attack |= sq;
		if ((sq & board_occupancy) != 0)
			break;
		temp_pos += 9;
		prev = next;
		next = temp_pos % 8;
	}
	temp_pos = pos - 9;
	prev = pos % 8;
	next = temp_pos % 8;
	while (abs(next - prev) < 2 && (temp_pos & ~63) == 0) {
		sq = 1ULL << temp_pos;
		attack |= sq;
		if ((sq & board_occupancy) != 0)
			break;
		temp_pos -= 9;
		prev = next;
		next = temp_pos % 8;
	}
	return attack;
}
//Conventional looping algo to calculate attacks an antidiagonal
uint64_t Move_Generator::calc_antidiagonal_attack(uint64_t board_occupancy, int pos) {
	uint64_t attack = 0ULL;
	uint64_t sq;
	int prev = pos % 8;
	int temp_pos = pos + 7;
	int next = temp_pos % 8;
	while (abs(next - prev) < 2 && (temp_pos & ~63) == 0) {
		sq = 1ULL << temp_pos;
		attack |= sq;
		if ((sq & board_occupancy) != 0)
			break;
		temp_pos += 7;
		prev = next;
		next = temp_pos % 8;
	}
	temp_pos = pos - 7;
	prev = pos % 8;
	next = temp_pos % 8;
	while (abs(next - prev) < 2 && (temp_pos & ~63) == 0) {
		sq = 1ULL << temp_pos;
		attack |= sq;
		if ((sq & board_occupancy) != 0)
			break;
		temp_pos -= 7;
		prev = next;
		next = temp_pos % 8;
	}
	return attack;
}

void Move_Generator::init_maindiagonal_attacks() {
	//First two diagonals the occupancy doesn't matter
	bishop_main[0] = new uint64_t * [1]{new uint64_t[1]{ 0ULL }};
	bishop_main[1] = new uint64_t * [2]{ new uint64_t[1]{1ULL << 57}, new uint64_t[1]{1ULL << 48}};
	for (int d = 2; d < 13; d++) {
		uint64_t diagonal = main_diagonals[d];
		uint64_t tempDiagonal = diagonal;
		int numOccupancies = d < 8 ? (1 << (d - 1)) : 1 << (13 - d);
		int numPos = d < 8 ? d + 1 : 15 - d;
		//Initialize a dynamic matrix
		uint64_t** diagonal_attacks = new uint64_t* [numPos];
		for (int i = 0; i < numPos; i++)
			diagonal_attacks[i] = new uint64_t[numOccupancies];
		
		for (int occupancy = 0; occupancy < numOccupancies; occupancy++) {
			uint64_t board = set_maindiagonal_occupancy(diagonal, occupancy);
			int array_pos = 0;
			while (tempDiagonal != 0) {
				//First bit on diagonal
				int pos = long_bit_scan(tempDiagonal);
				//Store attack for current bishop position with given occupancy
				diagonal_attacks[array_pos++][occupancy] = calc_maindiagonal_attack(board, pos);
				//Clear bit to get next bit in next iteration
				tempDiagonal &= tempDiagonal - 1;
			}
			tempDiagonal = diagonal;
		}
		bishop_main[d] = diagonal_attacks;

	}
	bishop_main[13] = new uint64_t * [2] {new uint64_t[1]{ 1ULL << 15 }, new uint64_t[1]{ 1ULL << 6 }};
	bishop_main[14] = new uint64_t * [1] {new uint64_t[1]{ 0ULL }};
}


void Move_Generator::init_antidiagonal_attacks() {
	//First two diagonals the occupancy doesn't matter
	bishop_anti[0] = new uint64_t * [1] {new uint64_t[1]{ 0ULL }};
	bishop_anti[1] = new uint64_t * [2] { new uint64_t[1]{ 1ULL << 1 }, new uint64_t[1]{ 1ULL << 8 }};
	for (int d = 2; d < 13; d++) {
		uint64_t diagonal = anti_diagonals[d];
		uint64_t tempDiagonal = diagonal;
		int numOccupancies = d < 8 ? (1 << (d - 1)) : 1 << (13 - d);
		int numPos = d < 8 ? d + 1 : 15 - d;
		//Initialize a dynamic matrix
		uint64_t** diagonal_attacks = new uint64_t * [numPos];
		for (int i = 0; i < numPos; i++)
			diagonal_attacks[i] = new uint64_t[numOccupancies];

		for (int occupancy = 0; occupancy < numOccupancies; occupancy++) {
			uint64_t board = set_antidiagonal_occupancy(diagonal, occupancy);
			int array_pos = 0;
			while (tempDiagonal != 0) {
				//First bit on diagonal
				int pos = 63 - long_high_bit_scan(tempDiagonal);
				//Store attack for current bishop position with given occupancy
				diagonal_attacks[array_pos++][occupancy] = calc_antidiagonal_attack(board, pos);
				//Clear bit to get next bit in next iteration
				tempDiagonal &= ~(1ULL << pos);
			}
			tempDiagonal = diagonal;
		}
		bishop_anti[d] = diagonal_attacks;

	}
	bishop_anti[13] = new uint64_t * [2] {new uint64_t[1]{ 1ULL << 55 }, new uint64_t[1]{ 1ULL << 62}};
	bishop_anti[14] = new uint64_t * [1] {new uint64_t[1]{ 0ULL }};
}

uint64_t Move_Generator::set_maindiagonal_occupancy(uint64_t diagonal, uint32_t occupancy) {
	uint32_t* bits = new uint32_t[8];
	uint64_t res = 0L;
	int i;
	for (i = 0; diagonal != 0; i++) {
		int bPos = long_bit_scan(diagonal);
		bits[i] = bPos;
		diagonal &= diagonal - 1; //Reset rightmost set bit
	}
	for (int j = 1; j < i - 1; j++) {
		if ((occupancy & 1) == 1)
			res |= 1ULL << bits[j];
		occupancy >>= 1;
	}
	return res;
}

uint64_t Move_Generator::set_antidiagonal_occupancy(uint64_t diagonal, uint32_t occupancy) {
	uint32_t* bits = new uint32_t[8];
	uint64_t res = 0L;
	int i;
	for (i = 0; diagonal != 0; i++) {
		int bPos = 63 - long_high_bit_scan(diagonal);
		bits[i] = bPos;
		diagonal &= ~(1ULL << bPos); //Reset rightmost set bit
	}
	for (int j = 1; j < i - 1; j++) {
		if ((occupancy & 1) == 1)
			res |= 1ULL << bits[j];
		occupancy >>= 1;
	}
	return res;
}



