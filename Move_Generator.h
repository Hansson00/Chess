#pragma once
#include "sdl.h"
#include "Chess_utility.h"

typedef void(*FunctionPointer)();

class Move_Generator {
public:
	Move_Generator();
	~Move_Generator();
	//uint16_t* generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos ,uint64_t mask, bool white);
	typedef enum Piece{
		King,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen
	};


	//Wrapper to easily keep track of the move_list and some useful functions
	struct Move_list {
		uint16_t* start() { return move_list;};
		uint16_t* end() { return last; };
		uint16_t size() { return end() - start(); }
		
		uint16_t contains(const uint16_t move) {
			for (uint16_t* i = start(); i < end(); i++) {
				if ((*i & 0xFFF) == move)
					return *i; //Return the move with its flags
			}
			return 0; //Move not found
		}
		void add_move(const uint16_t move) { *last++ = move; }
		uint16_t to_sq(const uint16_t move) const { return move & 0x3F; }
		uint16_t from_sq(const uint16_t move) const { return (move >> 6) & 0x3F; }
		uint16_t flags(const uint16_t move) const { return move >> 12; }
		uint16_t move_list[60];
		uint16_t* last; //60 should be enough space for all moves
	};

	//Functions to generate each kind of piece moves
	void generate_pawn_moves(Move_list* move_list, Position* pos);
	void generate_knight_moves(Move_list* move_list, Position* pos);
	void generate_king_moves(Move_list* move_list, Position* pos);
	void generate_bishop_moves(Move_list* move_list, Position* pos);
	void generate_queen_moves(Move_list* move_list, Position* pos);
	void generate_rook_moves(Move_list* move_list, Position* pos);
	void generate_sliding_moves(Move_list* move_list, uint64_t board, uint64_t moves, uint32_t piece);

protected:
	void add_promotion(Move_list* move_list, uint16_t move);
	const uint64_t shift_up(uint64_t pawns, bool white);
	const uint64_t shift_side(uint64_t pawns, bool right, bool white);
	uint64_t piece_attacks(uint64_t board, uint64_t pieces, int piece_pos);
	const uint64_t pawn_attacks(uint64_t pawns, bool white);
	uint64_t knight_attacks(uint64_t board, int knight_pos);
	uint64_t rook_attacks(uint64_t board, int rook_pos);
	uint64_t bishop_attacks(uint64_t board, int bishop_pos);
	uint64_t queen_attacks(uint64_t board, int queen_pos);
	void init_knight_attacks();
	void init_king_attacks();
	uint64_t king_attacks[64];
	
	uint64_t file_attacks(uint64_t board, int rook_pos);
	uint64_t rank_attacks(uint64_t board, int rook_pos);
	uint64_t files[8] = { 0x0101010101010101LL, 0x0202020202020202LL, 0x0404040404040404LL,
		0x0808080808080808LL, 0x1010101010101010LL, 0x2020202020202020LL, 0x4040404040404040LL,
		0x8080808080808080LL };
	uint64_t ranks[8] = { 0xFFLL, 0xFF00LL, 0xFF0000LL, 0xFF000000LL, 0xFF00000000LL, 0xFF0000000000LL,
			0xFF000000000000LL, 0xFF00000000000000LL };
	uint64_t main_diagonals[15] = { 0x0100000000000000ULL, 0x0201000000000000ULL, 0x0402010000000000ULL, 0x0804020100000000ULL,
			0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
			0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020ULL, 0x8040ULL, 0x80ULL };
	uint64_t anti_diagonals[15] = { 0x1ULL, 0x0102ULL, 0x010204ULL, 0x01020408ULL, 0x0102040810ULL, 0x010204081020ULL, 0x01020408102040ULL,
			0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL,
			0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL };

private:

	//uint16_t* (Move_Generator::* arr[6])(uint16_t*, Position*, bool) = { &Move_Generator::generate_king_moves, &Move_Generator::generate_pawn_moves, &Move_Generator::generate_knight_moves };
	// = {&generate_king_moves, &generate_pawn_moves ,&generate_knight_moves, &generate_pawn_moves, &generate_pawn_moves, &generate_pawn_moves};
	
	uint32_t mask_bits[15] = {0,0,1,3,7,15,31,63,31,15,7,3,1,0,0};
	uint64_t shifted_anti_diagonal = (1ULL) | (1ULL << 7) | (1ULL << 14) | (1ULL << 21) | (1ULL << 28) | (1ULL << 35) | (1ULL << 42) | (1ULL << 49);
	
	uint64_t (Move_Generator::* attack_function[8])(uint64_t, int) = {&Move_Generator::knight_attacks, &Move_Generator::bishop_attacks, 
		&Move_Generator::rook_attacks, &Move_Generator::queen_attacks, &Move_Generator::maindiagonal_attacks, &Move_Generator::antidiagonal_attacks,
		&Move_Generator::rank_attacks, &Move_Generator::file_attacks};

	//ATTACK TABLES
	uint64_t rook_attack_table[8][64];
	uint64_t knight_attack[64];
	uint64_t** bishop_main[15];
	uint64_t** bishop_anti[15];
	
	//ROTATION FUNCTIONS
	uint64_t rank_to_file(uint64_t board, int rank);
	uint64_t file_to_rank(uint64_t board, int file);


	//INITIALIZATION FUNCTIONS USED AT START OF PROGRAM
	uint64_t antidiagonal_attacks(uint64_t board, int bishop_pos);
	uint64_t maindiagonal_attacks(uint64_t board, int bishop_pos);
	uint64_t calc_row_attack(uint64_t row_occupancy, int pos);
	uint64_t calc_maindiagonal_attack(uint64_t board_occupancy, int pos);
	uint64_t calc_antidiagonal_attack(uint64_t board_occupancy, int pos);
	void init_maindiagonal_attacks();
	void init_antidiagonal_attacks();
	void init_row_attacks();
	uint64_t set_maindiagonal_occupancy(uint64_t diagonal, uint32_t occupancy);
	uint64_t set_antidiagonal_occupancy(uint64_t diagonal, uint32_t occupancy);
	


};

