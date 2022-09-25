#pragma once
#include "SDL_ttf.h"
#include "SDL.h"
#include "iostream"
class Text {
public:
	Text(const uint8_t size, const uint8_t r, const uint8_t g, const uint8_t b);
	Text(const uint8_t size);
	~Text();

	void draw_text(std::string text, SDL_Surface* surface, uint16_t x, uint16_t y);
	void draw_text(std::string text, SDL_Surface* surface, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
private:

	const char* font_path = "./fonts/raleway/Raleway-Bold.ttf";
	SDL_Surface* m_draw_surface = nullptr;
	TTF_Font* m_font = nullptr;
	unsigned char font_color_r = 0;
	unsigned char font_color_g = 0;
	unsigned char font_color_b = 0;
};

