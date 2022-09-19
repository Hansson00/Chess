#pragma once
#include "SDL.h"
#include "Window.h"
#include "Engine.h"
#include "Position.h"

class Chess {

public:
	Chess();

	Window* window;
	Engine* engine;
	uint8_t held_piece = 255;
	void main_loop();

private:
	void events();
	bool running = true;
	uint64_t held_piece_board = 0;
	void mouse_event(uint8_t button, bool mouse_down);
};

