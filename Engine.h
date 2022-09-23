#pragma once
#include "Window.h"
#include "SDL.h"
#include "Chess_utility.h"
#include "Move_Generator.h"


class Engine : private Move_Generator {

public:
	Engine();
	~Engine();
	uint64_t move_squares(uint16_t* moves, uint16_t* end);
	uint16_t* get_legal_moves(uint16_t* moves);
	Position pos;
	uint64_t generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos, uint64_t mask);

private:
	uint16_t* (Move_Generator::* arr[6])(uint16_t*, Position*, bool) = { &Move_Generator::generate_king_moves, &Move_Generator::generate_pawn_moves, &Move_Generator::generate_knight_moves };
	void fenInit(std::string);
	
	

};
