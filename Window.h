#pragma once
#include "SDL.h"
#include "iostream"
#include "string"
#include <filesystem>
#include "SDL_image.h"
#include "map"


class Window {
public:
	Window(int window_width, int window_height);
	~Window();

	void draw_piece(uint64_t bit_board, char piece);
	void draw_test();
	void draw_board();
	void update();

private:
	
	SDL_Window* window;
	SDL_Renderer* renderer;
	//SDL_Texture** pieces;
	std::map<char, SDL_Texture*> piece_map;

	SDL_Texture* board;

	int width;
	int height;
	int padding = 20;
	int piece_size;

	void generate_textures();
	void generate_board_texture(int width, int height);
};

