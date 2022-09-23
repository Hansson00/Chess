#pragma once
#include "SDL.h"
#include "Window.h"
#include "Engine.h"
#include "Sound.h"
#include "Chess_utility.h"

class Chess {

public:
	Chess();
	uint8_t held_piece = 255;
	void main_loop();

private:
	Sound_Manager* sound_manager;
	Window* window;
	Engine* engine;

	void events();
	void draw();
	//void sound_effects();
	
	bool running = true;
	uint64_t held_piece_board = 0;
	void mouse_event(uint8_t button, bool mouse_down);

	//This funtion can only add or remove one piece at a time
	void chagne_bitboards(uint32_t p, uint64_t add, uint64_t remove);
};

