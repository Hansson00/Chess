#include "Window.h"

Window::Window(uint32_t window_width, uint32_t window_height) {

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
	generate_board_texture();

}

Window::~Window() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyTexture(board);

	for (uint32_t i = 0; i < 12; i++)
		if (pieces[i] != nullptr)
			SDL_DestroyTexture(pieces[i]);
}

void Window::draw_pieces(uint64_t* bit_board) {

	SDL_Rect rect = { 0, 0, piece_size, piece_size };
	for (int i = 0; i < 12; i++) {
		uint64_t p = bit_board[i];
		while (p) {
			uint32_t index = long_bit_scan(p);
			rect.x = index % 8 * rect.w + padding;
			rect.y = index / 8 * rect.w + padding;
			SDL_RenderCopy(renderer, pieces[i], NULL, &rect);
			p &= p - 1;
		}
	}
}

uint32_t Window::bit_scan(uint32_t i) {
	i = ~i & (i - 1);
	if (i <= 0) return i & 32;
	uint32_t n = 1;
	if (i > 1 << 16) { n += 16; i >>= 16; }
	if (i > 1 << 8) { n += 8; i >>=  8; }
	if (i > 1 << 4) { n += 4; i >>=  4; }
	if (i > 1 << 2) { n += 2; i >>=  2; }
	return n + (i >> 1);
}

uint32_t Window::long_bit_scan(uint64_t i) {
	uint32_t x = (uint32_t)i;
	return x == 0 ? 32 + Window::bit_scan((uint32_t)(i >> 32))
		: Window::bit_scan(x);
}

void Window::draw_test() {}

void Window::draw_board() {
	SDL_Rect rect = { 0,0,  width , height};
	SDL_RenderCopy(renderer, board, NULL, &rect);
}

void Window::update() {
	SDL_RenderPresent(renderer);
}

void Window::mouse_grid_pos(int* x, int* y) {
	*x = *x < padding ? 8 : (int)(*x - padding) / piece_size;
	*y = *y < padding ? 8 : (int)(*y - padding) / piece_size;
}

void Window::draw_piece_at_mouse(int piece) {
	int x, y;
	SDL_GetMouseState(&x, &y);
	SDL_Rect rect = { 0, 0, piece_size, piece_size };
	rect.x = x - piece_size / 2;
	rect.y = y - piece_size / 2;
	SDL_RenderCopy(renderer, pieces[piece], NULL, &rect);

}

void Window::generate_board_texture() {
	
	uint32_t mini_padding = 1;
	SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect rect = { 0, 0, width, height };
	SDL_FillRect(surface, &rect, 0x0000ff); // draw blue background
	rect.x = padding - mini_padding;
	rect.y = padding - mini_padding;
	rect.w = 1000 - (padding - mini_padding) * 2; 
	rect.h = 1000 - (padding - mini_padding) * 2;
	SDL_FillRect(surface, &rect, 0xffffff); // draw all white squares background

	piece_size = (width - (padding) * 2) / 8;

	rect.w = piece_size;
	rect.h = piece_size;

	for (int i = 0; i < 64; i++)			// draw all green squares
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
	
	pieces = new SDL_Texture* [12];
	SDL_Surface* surface;	

	uint32_t i = 0;
	std::string path = "./pieces";
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string str = entry.path().string();
		surface = IMG_Load(str.c_str());
		switch (str[11]) {
		case 'K': pieces[0]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'P': pieces[1]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'N': pieces[2]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'B': pieces[3]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'R': pieces[4]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'Q': pieces[5]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'k': pieces[6]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'p': pieces[7]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'n': pieces[8]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'b': pieces[9]  = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'r': pieces[10] = SDL_CreateTextureFromSurface(renderer, surface); break;
		case 'q': pieces[11] = SDL_CreateTextureFromSurface(renderer, surface); break;
		default: break;
		}
		SDL_FreeSurface(surface);
		i++;
	}
}