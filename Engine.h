#pragma once
#include "Window.h"
#include "SDL.h"
#include "Chess_utility.h"
#include "Move_Generator.h"
#include "fstream"
#include "unordered_map"
#include "random"
#include "Evaluation.h"
#include "queue"
#include "Opening_book.h"

uint64_t hash_pos(Position* pos);

class Engine : private Evaluation {

public:
	Engine();
	~Engine();

	Position pos;
	Move_list move_list;
	Position_list* p_list;
	int move_highlight = 0;

	uint64_t zobrist_hash(Position* pos);
	void init_hashtable(Position* pos);
	
	std::unordered_map<uint64_t, int> eval_map;
	std::unordered_map<uint64_t, uint64_t> perft_map;

	std::string perft_out = "";

	enum Sound {
		s_move,
		s_capture,
		s_castle,
		s_check,
		s_checkmate
	};
	Sound sound;

	Opening_book* ob;

	uint32_t hash_hits = 0;

	void get_legal_moves(Position* pos, Move_list* move_list);
	uint64_t generate_held_piece_moves(uint16_t p, Position* pos, uint64_t mask);

	void make_move(Position* pos, uint32_t move);
	void undo_move(Position* current, Position* perv);
	void update_attack(Position* pos);

	void player_make_move(const uint32_t move);
	void player_undo_move();

	double search_eval2(int depth, int alpha, int beta, Position* pos);
	int search_eval(int depth, Position* pos);
	uint32_t find_best_move(int depth, Position* pos);

	uint32_t find_best_move_th(int depth, Position* pos, bool* run);
	int search_eval2_th(int depth, int alpha, int beta, Position* pos, bool* run);



	uint64_t perft(int depth, Position* pos);
	void _perft_debug(int depth, Position* pos);
	uint64_t search(int depth, Position* pos);

private:

	void in_check_masks(Position* pos, bool white_in_check);
	void en_passant(Position* pos, int pushedPawn);
	uint64_t find_block(uint32_t king, uint32_t checker);
	void find_pins(Position* pos);
	void filter_pins(Move_list* move_list, Position* pos);
	const uint64_t pinned_ray(int king, int piece);
	void castle(Position* pos, uint64_t from, uint64_t to);

	void (Move_Generator::* arr[6])(Move_list*, Position*) = { &Move_Generator::generate_king_moves, &Move_Generator::generate_pawn_moves, &Move_Generator::generate_knight_moves,
	&Move_Generator::generate_bishop_moves, &Move_Generator::generate_rook_moves, &Move_Generator::generate_queen_moves };
	void fenInit(Position* pos, std::string);

	uint64_t hash_table[12][64];
	uint64_t black_to_move_hash;
	uint64_t move_hash;
	uint64_t ep_hash[8];
	uint64_t castle_hash[4];

	const double worst_eval = -999999;


};
