#pragma once
#include "SDL_mixer.h"
#include "SDL.h"
#include "iostream"

class Sound_Manager {
public:
	Sound_Manager();
	~Sound_Manager();
	Mix_Chunk* capture;
	Mix_Chunk* castle;
	Mix_Chunk* check;
	Mix_Chunk* checkmate;
	Mix_Chunk* move;
	void play_sound(Mix_Chunk* mc);

private:
	Mix_Chunk* load_sound(const char* file_path);
};

