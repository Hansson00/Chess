#pragma once
#include "SDL.h"
#include "Window.h"
#include "Position.h"

class Events{
public:


protected:
	uint8_t held_piece = 255;
	void events(Window* window, Position* pos);
	bool running = true;

private:
	
	uint64_t held_piece_board = 0;
	void mouse_event(uint8_t button, bool mouse_down, Window* window, Position* pos);

};

