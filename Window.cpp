#include "Window.h"

Window::Window(int window_width, int window_height) {

	width = window_width;
	height = window_height;

	window = SDL_CreateWindow("Chess Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	if (window) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if(!renderer)
			std::cout << "Cound not init Renderer... \n";
	}
	else
		std::cout << "Cound not init Window... \n";
	
	generate_textures();
	generate_board_texture(1000, 1000);

}

Window::~Window() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyTexture(board);
	for (auto it : piece_map)
		SDL_DestroyTexture(it.second);
	piece_map.clear();

	/*for (int i = 0; i < 12; i++)
		if (pieces[i] != nullptr)
			SDL_DestroyTexture(pieces[i]);*/
}

void Window::draw_piece(uint64_t bit_board, char piece) {

	SDL_Rect rect= { 0,0,  piece_size , piece_size };

	for (int i = 0; i < 64; i++) {
		if (bit_board & 1) {
			rect.x = i % 8 * rect.w + padding;
			rect.y = i / 8 * rect.w + padding;
			SDL_RenderCopy(renderer, piece_map[piece], NULL, &rect);
		}
		bit_board = bit_board >> 1;
	}
		

}

void Window::draw_test() {
}

void Window::draw_board() {
	SDL_Rect rect = { 0,0,  width , height};
	SDL_RenderCopy(renderer, board, NULL, &rect);
}

void Window::update() {
	SDL_RenderPresent(renderer);
}

void Window::generate_board_texture(int width, int heigh) {
	
	int mini_padding = 1;
	SDL_Surface * surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect rect = { 0, 0, width, height };
	SDL_FillRect(surface, &rect, 0x0000ff); // draw blue background
	rect.x = padding - mini_padding;
	rect.y = padding - mini_padding;
	rect.w = 1000 - (padding - mini_padding) * 2; 
	rect.h = 1000 - (padding - mini_padding) * 2;
	SDL_FillRect(surface, &rect, 0xffffff);

	piece_size = (width - (padding) * 2) / 8;

	rect.w = piece_size;
	rect.h = piece_size;

	for (int i = 0; i < 64; i++)
		if (((i + i / 8) % 2) % 2 == 1) {
			rect.x = i % 8 * rect.w + padding;
			rect.y = i / 8 * rect.w + padding;
			SDL_FillRect(surface, &rect, 0x009650);
		}
	this->board = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	/*SDL_Rect dest = {0,0, 1000, 1000};
	SDL_RenderCopy(renderer, texture, NULL, &dest);
	SDL_RenderPresent(renderer);*/
}

void Window::generate_textures() {

	
	//pieces = new SDL_Texture* [12];
	SDL_Surface* surface;	

	int i = 0;
	std::string path = "./pieces";
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string str = entry.path().string();
		surface = IMG_Load(str.c_str());
		//pieces[i] = SDL_CreateTextureFromSurface(renderer, surface);
		piece_map[str[11]] = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		i++;
	}
	

}
