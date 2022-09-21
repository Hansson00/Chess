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
private:
	void fenInit(std::string);
	
	

};
