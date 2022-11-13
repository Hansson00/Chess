#pragma once
#include "SDL.h"
#include "Text.h"
#include "iostream"
#include "string"

#define E_WIDTH 25

class Eval_Bar {
public:
	Eval_Bar(uint16_t x, uint16_t height);
	~Eval_Bar();
	
	SDL_Surface* create_eval_surface(double_t score);
	SDL_Rect bar_rect;

private:
	uint16_t width = E_WIDTH;
	uint16_t height = 0;


	SDL_Rect w_rect;
	SDL_Rect b_rect;

	SDL_Surface* m_eval_surface;
	Text* text;
};

