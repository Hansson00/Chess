#pragma once
#include "Window.h"
#include "SDL.h"
#include "Chess_utility.h"
#include "Move_Generator.h"
#include "fstream"
#include "unordered_map"
#include "map"

uint64_t hash_pos(Position* pos);

class Engine : private Move_Generator {

public:
	Engine();
	~Engine();

	Position pos;
	Move_list move_list;
	Position_list* p_list;


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

	uint32_t hash_hits = 0;

	uint64_t move_squares(uint16_t* moves, uint16_t* end);
	void get_legal_moves(Position * pos, Move_list* move_list);
	uint64_t generate_held_piece_moves(uint16_t p, Position* pos, uint64_t mask);
	
	void make_move(Position* pos, uint32_t move);
	void undo_move(Position* current, Position* perv);
	void update_attack(Position* pos);
	
	void player_make_move(const uint32_t move);
	void player_undo_move();

	uint64_t perft(int depth, Position* pos);
	void _perft_debug(int depth, Position* pos);
	uint64_t search(int depth, Position* pos);
	void parse_move(uint16_t move);
	


private:

	void in_check_masks(Position* pos, bool white_in_check);
	void en_passant(Position* pos, int pushedPawn);
	uint64_t find_block(uint32_t king, uint32_t checker);
	void find_pins(Position* pos);
	void filter_pins(Move_list* move_list, Position* pos);
	const uint64_t pinned_ray(int king, int piece);
	void castle(Position* pos, uint64_t from, uint64_t to);

	void (Move_Generator::* arr[6])(Move_list*, Position*) = { &Move_Generator::generate_king_moves, &Move_Generator::generate_pawn_moves, &Move_Generator::generate_knight_moves,
	&Move_Generator::generate_bishop_moves, &Move_Generator::generate_rook_moves, &Move_Generator::generate_queen_moves};
	void fenInit(Position* pos, std::string);
	
	
	

};
