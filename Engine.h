#pragma once
#include "Window.h"
#include "SDL.h"
#include "Chess_utility.h"
#include "Move_Generator.h"


class Engine : private Move_Generator {

public:
	Engine();
	~Engine();

	Position pos;
	Move_list move_list;
	

	enum Sound {
		s_move,
		s_capture,
		s_castle,
		s_check,
		s_checkmate
	};
	Sound sound;

	uint64_t move_squares(uint16_t* moves, uint16_t* end);
	void get_legal_moves(Position * pos, Move_list* move_list);
	uint64_t generate_held_piece_moves(uint16_t p, Position* pos, uint64_t mask);
	
	void make_move(Position* pos, uint32_t move);
	void undo_move(Position_list* pos_list, Position* current);
	void update_attack(Position* pos);

	void perft(int depth, Position* pos);
	uint64_t search(int depth, Position_list* p_list);
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
