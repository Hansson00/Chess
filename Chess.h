#pragma once
#include "SDL.h"
#include "Window.h"
#include "Engine.h"
#include "Sound.h"
#include "Chess_utility.h"
#include "future"
#include "chrono"
#include "Evaluation.h"

static std::string get_input(bool* bl);

class Chess {

public:
	Chess(Uint16 width, uint16_t height);
	uint8_t held_piece = 255;
	void main_loop();

private:
	//for test!!
	double eval_test = -6.235;



	Sound_Manager* sound_manager;
	Window* window;
	Engine* engine;
	


	void events();
	void draw();
	void command_promt();
	void compare_perft(int depth, int64_t positions);
	//void sound_effects();
	void play_sound();
	
	bool running = true;
	uint64_t held_piece_board = 0;
	void mouse_event(uint8_t button, bool mouse_down);

	//This funtion can only add or remove one piece at a time
	void chagne_bitboards(uint32_t p, uint64_t add, uint64_t remove);
};

