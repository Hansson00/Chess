#pragma once
#include "SDL.h"
#include "iostream"
#include "string"
#include <filesystem>
#include "SDL_image.h"
#include "Chess_utility.h"


class Window {
public:
	Window(uint32_t window_width, uint32_t window_height);
	~Window();

	void draw_pieces(uint64_t bit_board[]);
	void draw_board();
	void update();
	void mouse_grid_pos(int* x, int* y);
	void draw_piece_at_mouse(int piece);
	void draw_texture_at_square(uint64_t square, SDL_Texture* texture);

	SDL_Texture* legal_circle;
	SDL_Texture* attack_square;

private:
	
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture** pieces;
	SDL_Texture* board;
	

	int width;
	int height;
	int padding = 20;
	int piece_size;

	void generate_textures();
	void generate_board_texture();
	void generate_circle_texture();
	void generate_attack_texture();
	
	

};