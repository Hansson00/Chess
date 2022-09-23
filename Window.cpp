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
	generate_attack_square();
	generate_circle_texture();
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


void Window::generate_attack_square() {
	SDL_Surface* surface = SDL_CreateRGBSurface(0, 120, 120, 32, 0, 0, 0, 0);
	SDL_Rect rect = { 0, 0, 120, 120};
	SDL_FillRect(surface, &rect, 0xff0000);
	this->attack_square = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
}

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

void Window::draw_attack_sqaure(uint64_t squares) {
	while (squares) {
		int square = long_bit_scan(squares);
		int x = (square % 8) * piece_size + padding;
		int y = (square / 8) * piece_size + padding;
		SDL_Rect rect = { x+1, y+1,  118 , 118 };
		SDL_RenderCopy(renderer, attack_square, NULL, &rect);
		squares &= squares - 1;
	}
	

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

void Window::generate_circle_texture() {

	const int w = 1000;
	const int h = 1000;

	SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);

	//SDL_Rect rect = { 0,0,1,1 };

	SDL_LockSurface(surface);
	uint8_t* pixelArray = (uint8_t*)surface->pixels;


	for (int i = -w / 2; i < w / 2; i++) {
		for (int j = -h / 2; j < h / 2; j++) {
			if (i * i + j * j <= w * h / 16) {
				pixelArray[(j + w / 2) * surface->pitch + (i + w / 2) * surface->format->BytesPerPixel + 0] = 61;
				pixelArray[(j + w / 2) * surface->pitch + (i + w / 2) * surface->format->BytesPerPixel + 1] = 61;
				pixelArray[(j + w / 2) * surface->pitch + (i + w / 2) * surface->format->BytesPerPixel + 2] = 61;
			}
		}
	}

	SDL_UnlockSurface(surface);
	SDL_SetColorKey(surface, SDL_TRUE, 0x0);
	this->legal_circle = SDL_CreateTextureFromSurface(renderer, surface);
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